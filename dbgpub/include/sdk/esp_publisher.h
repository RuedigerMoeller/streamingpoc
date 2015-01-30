/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_PUBLISHER_H
#define __ESP_PUBLISHER_H

#include <sdk/esp_types.h>
#include <sdk/esp_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspPublisherGroup Publishing
 * @{
 * Provides methods and entities to create publishing connections to a project, format data rows and blocks and send the 
 * data to the project.
 * Publishing is done using \ref EspPublisher instance. An instance of a EspPublisher creates a 
 * single socket connection to the backend project. A single publisher can be used to publish to multiple streams.
 * <p>
 * Like other entities an \ref EspPublisher can be created in direct, callback or select access modes. A \ref EspPublisher
 * can be retrieved directly from the global SDK context using \ref esp_publisher_create() or from the parent \ref EspProject that needs 
 * publishing to using \ref esp_project_create_publisher().
 * In the former case, SDK internally creates the \ref EspServer, \ref EspProject hierarchy for the new EspPublisher.
 * The first thing after a new EspPublisher is created is to connect to the project running on the cluster. This
 * creates the physical socket connection.
 * \code
 * EspUri * uri = esp_uri_create_string("esp://clusterhost:clusterport/default_workspace/project", error);
 * // Assume we have valid EspCredentials
 * EspProject * project = esp_project_get(uri, creds, NULL, error);
 * 
 * int rc = esp_project_connect(project, error);
 * 
 * // Create publisher with default options
 * EspPublisher * publisher = esp_project_create_publisher(project, NULL, error);
 * 
 * rc = esp_publisher_connect(publisher, error);
 * // publish data
 * \endcode
 *
 * Publishing data in the SDK is done using the two classes \ref EspMessageWriter and \ref EspRelativeRowWriter.
 * \ref EspRelativeRowWriter is used to format a single data row for a particular stream. It provides methods
 * to populate a row with data of various types and set the row operation. One or more data rows are packed
 * into a \ref EspMessageWriter to be published. These two classes are not thread safe, so care must be taken
 * to serialize access if using multiple threads.
 * That means when accessing a single instance of \ref EspMessageWriter or \ref EspRelativeRowWriter from multiple threads, access needs 
 * to be protected by a serializing mechanism.  \ref EspPublisher is thread safe and can support
 * multiple \ref EspMessageWriter objects. So if each thread has its own instance, no serialization is required.
 * Typical usage would consist of first creating a \ref EspMessageWriter for
 * the stream to publish to. \ref EspMessageWriter are specific for the stream they are created for and can be
 * use to query for stream specific properties such as schema, etc. To build a data rows, users retrieve a 
 * \ref EspRelativeRowWriter. This is used to set the the row operation and the data fields. Once all data has
 * been formatted, the \ref EspMessageWriter is published.  Below is an example of publishing a single row
 * \code
 * const EspStream * stream = esp_project_get_stream(project, "Stream", error);
 * const EspSchema * schema = esp_stream_get_schema(stream, error);
 * 
 * EspMessageWriter * writer = esp_publisher_get_writer(publisher, stream, error);
 * EspRelativeRowWriter * row_writer = esp_message_writer_get_relative_rowwriter(writer, error);
 * 
 * EspMoney * money_value = esp_money_create_string("1283.74", error);
 * 
 * esp_relative_rowwriter_start_row(row_writer, error);
 * esp_relative_rowwriter_set_operation(row_writer, ESP_STREAOP_INSERT, error);
 * 
 * // Data fields need to be populated in order. All data fields need
 * // to be specified, including null and shinethrough ones
 * int32_t numcols;
 * esp_schema_get_numcolumns(schema, &numcols, error);
 * int32_t coltype;
 * for (int i = 0; i < numcols; ++i) {
 *     esp_schema_get_column_type(schema, i, &coltype, error);
 *     switch (coltype) {
 *     case ESP_DATATYPE_INTEGER:
 *         esp_relative_rowwriter_set_integer(row_writer, 10, error);
 *         break;
 *     case ESP_DATATYPE_FLOAT:
 *         esp_relative_rowwriter_set_float(row_writer, 10.44, error);
 *         break;
 *     case ESP_DATATYPE_STRING:
 *         esp_relative_rowwriter_set_string(row_writer, "some string", error);
 *         break;
 *     case ESP_DATATYPE_MONEY02:
 *         rc = esp_relative_rowwriter_set_money(row_writer, money_value, error);
 *         break;
 *     default:
 *         esp_relative_rowwriter_set_null(row_writer, error);
 *         break;
 *     }
 * }
 * 
 * rc = esp_relative_rowwriter_end_row(row_writer, error);
 * rc = esp_publisher_publish(publisher, writer, error);
 *
 * // we need to free resources used by EspMoney
 * esp_money_free(money_value);
 * \endcode
 *
 * <h3>Publishing blocks</h3>
 * The SDK provides a mechanism to batch multiple data rows for higher throughput and processing efficiency
 * in the engine. There are two types of blocks - transaction blocks and envelope blocks. Envelope blocks batch
 * rows for transmission to the engine. Once it reaches the engine the rows are processed individually. Transaction
 * blocks are also batched together for transmission, but unlike envelopes they are processed as a group in the
 * engine. If any of the rows fails, all the rows in the group are discarded. There are a number of ways blocking
 * can be accomplished in the SDK. The easiest way to block data is to the use the \ref esp_message_writer_start_transaction()
 * and \ref esp_message_writer_end_block() calls. For example
 * \code
 * const EspStream * stream = esp_project_get_stream(project, "Stream", error);
 * const EspSchema * schema = esp_stream_get_schema(stream, error);
 * 
 * EspMessageWriter * writer = esp_publisher_get_writer(publisher, stream, error);
 * EspRelativeRowWriter * row_writer = esp_message_writer_get_relative_rowwriter(writer, error);
 * 
 * // start a transaction block. Could have used esp_message_writer_start_envelope() to start
 * // an envelope block
 * esp_message_writer_start_transaction(writer, 0, error);
 *
 * // loop to fill required number of rows
 * for (int i = 0; i < NUMROWS; i++) {
 *     esp_relative_rowwriter_start_row(row_writer, error);
 *     esp_relative_rowwriter_set_operation(row_writer, ESP_STREAOP_INSERT, error);
 *     esp_relative_rowwriter_set_integer(row_writer, i, error);
 *     esp_relative_rowwriter_set_float(row_writer, 10.44, error);
 *     esp_relative_rowwriter_set_string(row_writer, "some string", error);
 *     esp_relative_rowwriter_end_row(row_writer, error);
 * }
 * 
 * esp_message_writer_end_block(writer, error);
 * 
 * rc = esp_publisher_publish(publisher, writer, error);
 * \endcode
 *
 * This method will work with an \ref EspPublisher created with default options, that is no buffering. When buffering is
 * enabled (see below) there are some more options which can be set using \ref esp_publisher_options_set_blockmode()
 *
 * If set to \ref NO_BLOCKING, start block calls are ignored and the rows are sent
 * singly. If set to \ref AUTO_BLOCKING start block calls are also ignored. The rows
 * are batched up by the publishing thread automatically. In \ref EXPLICIT_BLOCKING mode,
 * the SDK batches records based on explicit esp_message_writer_start_transaction() / esp_message_writer_start_envelope() calls.
 * This mode, potentially provides the best throughput. 
 * 
 * <h3>Buffered publishing</h3>
 * An \ref EspPublisher created with the default options does not have internal queues. Data is sent to the engine in the thread
 * that calls the \ref esp_publisher_publish(). A succesful return from the call indicates that all the data
 * has been written to the socket. Note - this does not necessarily mean that all data has been received by the engine.
 * That can be ensured by setting asynchronous mode to false. The SDK allows users to specify an internal queue size
 * when creating a publisher using \ref esp_publisher_options_set_buffersize(). If set to a positive value,
 * the resulting \ref EspPublisher creates an internal queue and starts an internal publishing thread. All data is queued
 * automatically whenever a data row or a data block is completed using \ref esp_relative_rowwriter_end_row() or
 * \ref esp_message_writer_end_block() as the case may be. This is picked up by the publishing thread and written to the
 * socket.
 * <p>
 * In buffered mode the call \ref esp_publisher_publish() is optional. As mentioned above the data is queued
 * and sent automatically. Calling \ref esp_publisher_publish() in buffered mode, acts as a 'sync point'. The
 * calling thread blocks until all queued data is written and then returns.
 * <p>
 * There are some considerations to be kept in mind for buffered publishing to work. Buffered publishing cannot
 * work with asynchronous mode set to false. It is not supported in select or callback access modes.  
 * 
 * <h3>Publishing in callback/select access modes.</h3>
 * Like other entities in the SDK, \ref EspPublisher too supports direct, callback and select access modes. But there are
 * some special considerations in using a publisher in callback/select modes. First select and access modes cannot
 * be used with buffered publishing. Like others, an EspPublisher has the common lifecycle events such as 
 * \ref ESP_PUBLISHER_EVENT_CONNECTED, \ref ESP_PUBLISHER_EVENT_DISCONNECTED and \ref ESP_PUBLISHER_EVENT_CLOSED.
 * But the actual publishing is driven by the \ref ESP_PUBLISHER_EVENT_READY event event.
 * This indicates that the publisher is ready to accept data. Only a single \ref esp_publisher_publish() 
 * or a \ref esp_publiser_commit() call may be made in the event handler in response to a ready event. 
 * <p>
 * Refer to \ref EspPublisherOptions for more options that a publisher supports.
 * 
 * \see esp_publisher_create()
 * \see esp_project_create_publisher()
 * \see EspPublisherOptions
 */

/*!
 * \addtogroup EspPublisherEventGroup
 * @{
 */
#define ESP_PUBLISHER_EVENT_CONNECTED           0x05000002          /*!< EspPublisher has connected @hideinitializer. */
#define ESP_PUBLISHER_EVENT_DISCONNECTED        0x05000004          /*!< EspPublisher has disconnected @hideinitializer. */
#define ESP_PUBLISHER_EVENT_CLOSED              0x05000008          /*!< EspPublisher has closed @hideinitializer. */
#define ESP_PUBLISHER_EVENT_READY               0x0500000A          /*!< EspPublisher is ready to publish next set of data @hideinitializer. */
#define ESP_PUBLISHER_EVENT_ERROR               0x05000010          /*!< EspPublisher has encountered an error @hideinitializer. */
#define ESP_PUBLISHER_EVENT_ALL                 0x050000FF          /*!< Utility define for all publisher events @hideinitializer. */

/*! @} */

/*!
 * Create a publisher. The new EspPublisher will need to be freed using \ref esp_publisher_free().
 *
 * @return NULL on error.
 */
ESPAPICALL EspPublisher * esp_publisher_create(
        const EspUri * uri,                         /*!< URI of the project this publisher will publish to */
        const EspCredentials * credentials,         /*!< Credentials to use to authenticate */
        const EspPublisherOptions * options,        /*!< Publishing options to use for the new EspPublisher */
        EspError * error);

/*!
 * Frees an EspPublisher created using \ref esp_publisher_create().
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_free(
        EspPublisher * publisher,                   /*!< EspPublisher to free - must have been created using \ref esp_publisher_create() */
        EspError * error);

/*!
 * Retrieve the parent EspProject for this publisher.
 *
 * @return NULL on error.
 */
ESPAPICALL EspProject * esp_publisher_get_project(
        EspPublisher * publisher,                   /*!< EspProject to which this EspPublisher is publishing */
        EspError * error);

/*!
 * Connect to the project the publisher will be publishing to. In DIRECT access the
 * call will return when the publisher has connected. In CALLBACK/SELECT the call returns
 * immediately. Completion is indicated by the generation of an \ref ESP_PUBLISHER_CONNECTED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_connect(
        EspPublisher * publisher,           /*!< EspPublisher to connect */
        EspError * error);

/*!
 * Set the callback handler if the publisher was created using CALLBACK access mode.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_set_callback(
        EspPublisher * publisher,               /*!< EspPublisher to monitor */
        uint32_t events,                        /*!< bitwised OR'd value of events to monitor */
        PUBLISHER_CALLBACK_T callback,          /*!< The callback handler function */
        void * user_data,                       /*!< This pointer is passed back to the handler function */
        EspError * error);

/*!
 * Register the EspPublisher with the specified EspSelector object. Use for SELECTION mode access.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_select_with(
        EspPublisher * publisher,               /*!< EspPublisher to monitor */
        EspSelector * selector,                 /*!< The EspSelector to use for monitoring */
        uint32_t events,                        /*!< bitwise OR'd EspPublisherEvents to monitor for */
        EspError * error);

/*!
 * Disconnect the publisher from the project. In DIRECT mode call returns when the
 * publisher has disconnected. In CALLBACK/SELECT will generate an \ref ESP_PUBLISHER_DISCONNECTED
 * event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_disconnect(
        EspPublisher * publisher,               /*!< EspPublisher to disconnect */
        EspError * error);

/*!
 * Closes this instance of the publsiher. Once closed, the Publisher instance cannot
 * be used anymore and a fresh one will need to be created. If the Publisher is still connected
 * to the backend, disconnects internally. If there are any pending messages, the publisher 
 * waits for the messages to be sent before disconnecting.
 * 
 * In DIRECT access mode, this call returns after successfully closing the instance. In CALLBACK or SELECT
 * access modes, \ref ESP_PUBLISHER_EVENT_CLOSED will be generated. If as a result of this call, the publisher also 
 * disconnected \ref ESP_PUBLISHER_EVENT_DISCONNECTED will be sent first.
 *
 * @return 0 on success.
 */ 
ESPAPICALL int32_t esp_publisher_close(
        EspPublisher * publisher,               /*!< EspPublisher to close */
        EspError * error);

/*!
 * Query the publisher for the buffer sizes of the underlying socket. This call can be
 * made only after publisher is connected. Theoritically some of the socket parameters
 * may change during the lifetime of a connection, so for uptodate information
 * this call should be made periodically. The call is thread safe but does lock the
 * access to the underlying transport, which may impact performance if concurrent publishing
 * is taking place
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_get_socket_details(
        const EspPublisher * publisher,         /*!< EspPublisher to query */
        EspSocketDetails * socket_details,      /*!< EspSocketDetails structure to fill in */
        EspError * error);


/*!
 * Return the size in bytes of a block header. A block header is used if data
 * is being published in transaction or envelope blocks. The value is constant
 * for the lifetime of the EspPublisher object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_get_blockheader_size(
        const EspPublisher * publisher,         /*!< EspPublisher to query */
        int32_t * size,                         /*!< variable to put the size in */
        EspError * error);

/*!
 * Retrieve an EspMessageWriter that can be used to build single row or blocked messages
 * to send to the project.
 *
 * @return NULL on error.
 */
ESPAPICALL EspMessageWriter * esp_publisher_get_writer(
        EspPublisher * publisher,               /*!< EspPublisher to use to publish the data */
        const EspStream * stream,               /*!< EspStream the message writer will be formatting data for */
        EspError * error);

/*!
 * Retrieve an EspRawMessageWriter that can be used to publisher preformatted
 * binary data buffers to the project.
 *
 * @return NULL on error.
 */
ESPAPICALL EspRawMessageWriter * esp_publisher_get_rawwriter(
        EspPublisher * publisher,               /*!< EspPublisher to use to publish the data */
        EspError * error);

/*!
 * Publishes the data currently in the passed EspMessageWriter. The actual behavior of this function depends on a number of
 * factors: the access mode, buffering if any and whether synchronous publishing is in effect.
 *
 * If there is no internal publishing thread running, this call publishes all data queued in the
 * MessageWriter to the socket. If running in synchronous publishing mode this 
 * call also waits for the response from the server before returning. On succesfull completion
 * all data queued up in MessageWriter is cleared. If there is an error in publishing throws an
 * exception.
 * <p>
 * If there is an internal publishing thread, as with buffered publishing, data is sent on the wire in the background 
 * by the publishing thread. In this case, this call acts a sync point, it waits until the data
 * currently queued up is flushed out to the socket. Users should make sure that there is no
 * other thread that is queuing up data for this EspMessageWriter. In that case it is possible
 * this call will not return as expected. In general, with an internal publishing thread running
 * this call should be made in the same thread as the one writing the data out.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_publisher_publish(
        EspPublisher * publisher,
        EspMessageWriter * writer,
        EspError * error);


/*!
 * Publishes the data that has been filled in the EspRawMessageWriter. The semantics of the call that depend on modes
 * and buffering are similar to that of \ref esp_publisher_publish()
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_publisher_publish_raw(
        EspPublisher * publisher,
        EspRawMessageWriter * writer,
        EspError * error);

/*!
 * Issues a commit call on the publisher which causes the platform to process all its input queues and commit the data
 * to its log stores. If buffering is enabled this call waits until all queued up data is sent.
 * 
 * In CALLBACK or SELECT access mode, this call returns immediately. A subsequent receipt of a ESP_PUBLISHER_EVENT_READY event
 * will indicate that the commit has been successful.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_commit(
        EspPublisher * publisher,               /*!< The publisher whose queues to commit */
        EspError * error);


/*!
 * Issues a gd_commit call on the publisher which causes the server to GD commit all the buffered
 * messages in to its log stores. If buffering is enabled this call waits until all queued up data is sent.
 * 
 * In CALLBACK or SELECT access mode, this call returns immediately. A subsequent receipt of a ESP_PUBLISHER_EVENT_READY event
 * will indicate that the commit has been successful.
 *
 * The parameter gd_name is the unique name that is used to create GD session. The int32_t stream_ids[] holds ids of the GD streams 
 * (stream id can be accessed using esp_stream_get_id(...) API). The int64_t seq_no[] holds the sequence number that are being 
 * committed. First column of the event a GD subscriber receives is the sequence number. The len parameter tells number of stream id 
 * and sequence number that are passed in to this API. The len and the size of the stream_ids and seq_nos must be always equal. The 
 * API will take stream id and sequence number pair from each index and send commit to the server. Committing already committed 
 * sequence number or sequence number which does not exist in the server would not have any affect. 
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_commit_gd(
        EspPublisher * publisher,               
        char * gd_name,
        int32_t stream_ids[],
        int64_t seq_nos[],
		int32_t len,
        EspError * error);


/*!
 * \defgroup EspPublisherOptionsGroup Publisher Options
 * @{
 */


/*!
 * Specifies the way data rows will be batched when sent to the server. This setting has effect only
 * if the publishing is buffered, that is if there is an internal publishing thread that picks up the formatted
 * data and sends it to the project.
 */
typedef enum
{
    /*!
     * Records are sent singly. */
    NO_BLOCKING = 0,
    /*!
     * Transaction/envelop blocks need to be set explicitly by the user using \ref esp_message_writer_start_transaction()
     * or \ref esp_message_writer_start_envelope() calls. Potentially this option provides the best throughput.
     * \sa esp_message_writer_end_block()
     */
    EXPLICIT_BLOCKING,
    /*!
     * In this blocking mode, the internal publishing thread picks up all queued data and batches them together using transaction
     * blocks. The size of the blocks may vary depending on the amount of data that has accumulated. Potentially this provides the
     * best latency, especially if the data is not available at a constant rate.
     */
    AUTO_BLOCKING
}
ESP_BLOCKING_MODE_T;

/*!
 * Create an EspPublisherOptions object. This will need to be freed using \ref esp_publisher_options_free()\.
 */
ESPAPICALL EspPublisherOptions * esp_publisher_options_create(
        EspError * error);

/*!
 * Free an EspPublisherOptions object.
 */
ESPAPICALL int32_t esp_publisher_options_free(
        EspPublisherOptions * options,          /*!< EspPublisherOptions object to free */
        EspError * error);

/*!
 * Set the access mode with which the EspPublisher will be accessed.
 * Default is \ref DIRECT_ACCESS.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_options_set_access_mode(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const ESP_ACCESS_MODE_T mode,           /*!< The access mode to use */
        EspError * error);
/*!
 * Set the blocking mode to use for this publisher.
 * Default is NO_BLOCKING
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_options_set_blockmode(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const ESP_BLOCKING_MODE_T mode,         /*!< The blocking mode to use */
        EspError * error);

/*!
 * Sets the internal buffer size. If set to a positive value, all data published is queued to the buffer. EspPublisher
 * creates an internal thread that picks up the data from the buffer and publishes it. Maximum buffer size allowed in 4096
 * <p>
 * Buffering is enabled only in DIRECT_ACCESS modes. In other access modes this flag is ignored. Note that synchronous publishing 
 * cannot be done when buffering is enabled.
 * By default buffering is not enabled.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_publisher_options_set_buffersize(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const uint32_t size,                    /*!< Maximum number of data records/blocks to queue up */
        EspError * error);

/*!
 * If set to true, the project will exit if the publisher drops its connection.
 * Default is false.
 */
ESPAPICALL int32_t esp_publisher_options_set_exit_on_close(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const int32_t close,                    /*!< Non-zero to have the project exit */
        EspError * error);

/*!
 * Set the time in milliseconds that a project will wait to exit if no data
 * is being published.
 * By default timeout is disabled.
 */
ESPAPICALL int32_t esp_publisher_options_set_exit_on_timeout(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const int32_t timeout,                  /*!< value in milliseconds (0 or less means no timeout) */
        EspError * error);

/*!
 * Sets the command the project will execute if the publisher does not publish data for the
 * specified amount of time.
 * By default timeout is disabled.
 */
ESPAPICALL int32_t esp_publisher_options_set_timeout_finalizer(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const int32_t timeout,                  /*!< Time in milliseconds to wait before running the finalizer */
        const char * finalizer,                 /*!< Finalize command to run */
        EspError * error);

/*!
 * If set to false, EspPublisher publishes to the platform in 'synchronous' mode. In this mode, each time data
 * is written to the platform, EspPublisher waits for an acknowledgement from the platform before the next
 * data is sent.
 * <p>
 * This slows down throughput but ensures that the data being written has reached the platform. Note - the 
 * acknowledgement only guarantees that the data has physically reached the platform.
 * <p>
 * By default asynchronous publishing is set to true.
 *
 * @return 0 on success.
 */

ESPAPICALL int32_t esp_publisher_options_set_async(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const int32_t async,                    /*!< Non-zero - enables async mode publishing */
        EspError * error);

/*!
 * Not used.
 * Sets a custom EspDispatcher object to use.
 */
ESPAPICALL int32_t esp_publisher_options_set_dispatcher(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Not used.
 * Sets a custom EspUpdater object to use.
 */
ESPAPICALL int32_t esp_publisher_options_set_updater(
        EspPublisherOptions * options,          /*!< The options object to modify */
        const EspUpdater * updater,
        EspError * error);

/*!
 * @}
 */

/*!
 * \defgroup EspPublisherEventGroup Publisher Events
 * @{
 */

/*!
 * Retrieve the type of the event. The type will be one of the ESP_PUBLISHER_EVENT_xxx constants.
 *
 * @return 0 on success.
 */
ESPAPICALL uint32_t esp_publisher_event_get_type(
        const EspPublisherEvent * event,            /*!< Event to query */
        uint32_t * type,                            /*!< Pointer in which to store the type */
        EspError * error);

/*!
 * Retrieve the EspPublisher which generated this event.
 *
 * @return NULL on error.
 */
ESPAPICALL EspPublisher * esp_publisher_event_get_publisher(
        const EspPublisherEvent * event,            /*!< Event to query */
        EspError * error);

/*!
 * For an ESP_PUBLISHER_EVENT_ERROR type event, retrieve the associated error object.
 *
 * @return NULL on error.
 */
ESPAPICALL EspError * esp_publisher_event_get_error(
        const EspPublisherEvent * event,            /*!< Event to query */
        EspError * error);

/*! @} */

/*!
 *\defgroup EspMessageWriterGroup Data Message Handling
 * @{
 */

/*!
 * Retrieve the size in bytes of the row header for the data that will be
 * formatted by this EspMessageWriter. The row header size depends
 * on the schema of the stream being published to. The value is constant
 * for the lifetime of the EspMessageWriter object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_writer_get_rowheader_size(
        EspMessageWriter * writer,
        int32_t * header_size,
        EspError * error);

/*!
 * Start a transaction block for this EspMessageWriter.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_writer_start_transaction(
        EspMessageWriter * writer,                  /*!< EspMessageWriter on which to start a transaction */
        const uint32_t numoptions,                  /*!< Ignored */
        EspError * error);

/*!
 * Start an envelope block for this EspMessageWriter.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_writer_start_envelope(
        EspMessageWriter * writer,                  /*!< EspMessageWriter on which to start an envelope */
        const uint32_t numoptions,                  /*!< Ignored */
        EspError * error);


/*!
 * Returns the length in bytes of the binary data buffer that has been built up
 * so far. The length includes the block header and all the rows that have been
 * added to the block using \ref esp_relative_rowwriter_end_row(). This call is valid
 * only if a block has been started and is active. A call outside the context of a block
 * will always return 0.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_writer_get_current_block_size(
        EspMessageWriter * writer,
        int32_t * size,
        EspError * error);

/*!
 * Ends a block previously started with either \ref esp_message_writer_start_transaction() or
 * \ref esp_message_writer_start_envelope(). This call does not change the length of the binary
 * data buffer.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_writer_end_block(
        EspMessageWriter * writer,                  /*!< EspMessageWriter for which to end the block */
        EspError * error);

/*!
 * Retrieve EspRelativeRowWriter for which to format data. The row writer is initialized to format data
 * for the stream for which the EspMessageWriter was created.
 *
 * @return 0 on success.
 */
ESPAPICALL EspRelativeRowWriter * esp_message_writer_get_relative_rowwriter(
        EspMessageWriter * writer,              /*!< EspMessageWriter to use to for formatting data */
        EspError * error);

/*!
 * Retrieve the EspStream for which this EspMessageWriter was created.
 *
 * @return NULL on error.
 */
ESPAPICALL const EspStream * esp_message_writer_get_stream(
        EspMessageWriter * writer,
        EspError * error);


/*!
 * Starts a new row definition. Must be the first call when starting a row.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_start_row(
        EspRelativeRowWriter * writer,
        EspError * error);

/*!
 * Ends a row definition. Must the be the last call made after all data items have been populated.
 * If publishing in buffered mode and no blocking is in effect, this call queues the row to the internal
 * buffer to be picked up by the publishing thread.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_end_row(
        EspRelativeRowWriter * writer,
        EspError * error);

/*!
 * Set the operation for the row. Default is INSERT.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_operation(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const ESP_OPERATION_T opcode,
        EspError * error);

/*!
 * Set the next column value to be NULL.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_null(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        EspError * error);

/*!
 * Set the next column value to be shinethrough.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_shinethrough(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        EspError * error);

/*!
 * Set the next integer column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_integer(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int32_t int_value,
        EspError * error);

/*!
 * Set the next long column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_long(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int64_t long_value,
        EspError * error);

/*!
 * Set the next real column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_float(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const double double_value,
        EspError * error);

/*!
 * Set the next string column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_string(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const char * string_value,
        EspError * error);

/*!
 * Set the next date column value.
 * <p>
 * The DATE data type is provided for compatibility with legacy types.
 * In the engine values of DATE types are stored as seconds since the epoch
 * so using this type will loose precision.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_date(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int64_t date_value,
        EspError * error);
        
 /*!
 * Set the next time column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_time(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const EspTime * time_value,
        EspError * error); 
        
 /*!
 * Set the next fixed decimal column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_fixeddecimal(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        EspFixedDecimal * fixeddecimal_value,
        EspError * error);       

/*!
 * Set the next timestamp column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_timestamp(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int64_t data_value,
        EspError * error);

/*!
 * Set the next money column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_money(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const EspMoney * money_value,
        EspError * error);

/*!
 * Set a legacy money value. This corresponds to the CCL MONEY data type. The precision of this data type
 * is controlled by an engine runtime flag. The precision in effect in a particular instance of a project
 * can be retrieved using \ref esp_deployment_get_money_precision()
*
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_legacy_money(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const EspMoney * money_value,
        EspError * error);

/*!
 * Set money column value.
 * <p>
 * This call allows users to bypasss creating an \ref EspMoney object. The value will be interpreted
 * using the precision of the underlying field or if the field is of type \ref ESP_DATATYPE_MONEY
 * the precision setting in the connected project.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_money_as_long(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int64_t long_value,
        EspError * error);

/*!
 * Set next \ref EspBigDatetime column value
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_bigdatetime(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const EspBigDatetime * bigdatetime_value,
        EspError * error);

/*!
 * Set next binary column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_binary(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const void * binary_value,
        const uint32_t size,
        EspError * error);

/*!
 * Set the next boolean column value.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_boolean(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int32_t boolean_value,                /*!< Non zero for true */
        EspError * error);

/*!
 * Set the next interval column value.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_relative_rowwriter_set_interval(
        EspRelativeRowWriter * writer,              /*!< EspRelativeRowWriter to modify */
        const int64_t interval_value,               /*!< interval in microseconds */
        EspError * error);


/*!
 * Start a transaction block. The block will need to be ended with an \ref esp_rawwriter_end_block() call.
 * The EspRawMessageWriter can be populated by binary row buffers once a transaction block has been started.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_rawwriter_start_transaction(
        EspRawMessageWriter * raw_writer,
        EspError * error);

/*!
 * Start an envelope block. The block will need to be ended with an \ref esp_rawwriter_end_block() call.
 * The EspRawMessageWriter can be populated by binary row buffers once an envelope block has been started.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_rawwriter_start_envelope(
        EspRawMessageWriter * raw_writer,
        EspError * error);


/*!
 * End a transaction or an envelope block.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_rawwriter_end_block(
        EspRawMessageWriter * raw_writer,
        EspError * error);


/*!
 * Add a binary buffer to the EspRawMessageBuffer to publish. If no transaction/envelope
 * block is active the buffer can be a single row buffer or a block buffer. If a
 * transaction/envelope block is active, the buffer must be a single row buffer. The
 * call does not validate the buffer contents.
 * <p>
 * If publishing is not buffered, then the data will be sent to project once \ref esp_publisher_publish_raw()
 * is called. Otherwise the data is queued to be picked up and published by the internal publishing
 * thread.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_rawwriter_add_buffer(
        EspRawMessageWriter * raw_writer,
        const char * buffer,
        EspError * error);


/*!
 * Utility method that allows users to override the target stream the data is intended for.
 * The new stream must have identical schema to the stream for which the data was originally
 * formatted.
 * <p>
 * This call is directly modifying the binary data so care must be taken when using this.
 * If passing in a block buffer, modifies the target stream for each row in the block. Depending
 * on the number of rows in the block this call may potentially slow down a client. 
 * 
 * @return 0 on success
 */
ESPAPICALL int32_t esp_rawwriter_override_stream(
        EspRawMessageWriter * raw_writer,
        char * buffer,
        const EspStream * stream,
        EspError * error);


/*!
 * Returns the length in bytes of the binary data buffer that has been built up
 * so far. The length includes the block header and all the data rows that have been
 * added to the block using \ref esp_rawwriter_add_buffer(). This call is valid
 * only if a block has been started and is active. A call outside the context of a block
 * will always return 0.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_rawwriter_get_current_block_size(
        EspRawMessageWriter * raw_writer,
        int32_t * size,
        EspError * error);

/*! @} */



/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_PUBLISHER_H */
