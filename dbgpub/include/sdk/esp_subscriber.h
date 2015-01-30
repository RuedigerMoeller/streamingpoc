/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_SUBSCRIBER_H
#define __ESP_SUBSCRIBER_H

#include <sdk/esp_types.h>
#include <sdk/esp_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspSubscriberGroup Subscribing
 * @{
 * Methods, macros, objects to support subscribing to data from running projects.
 * An EspSubscriber is used to subscribe to data from one or more streams in a server. Each
 * subscriber establishes a socket connection to the server it is subscribing from. A single
 * subscriber can be used to subscribe to data from multiple streams.
 * <p>
 * Like other entities, EspSubscriber can be created in direct, callback or select access modes.
 * A subscriber can be retrieved from the \ref EspProject instance that needs to be subscribed to or
 * directly from the global SDK. In the latter case, the SDK internally creates an \ref EspServer, \ref EspProject
 * hierarchy as the ancestors of the new Subscriber. After an EspSubscriber is retrieved it needs to connect
 * to the project - this establishes the physical socket connection with the backend project. To start receiving
 * data on the subscription, users need to issue one or more subscription requests. A single EspSubscriber
 * can be used to subscribe to multiple streams. While the subscription requests can be made either before
 * or after the connect call, the actual request to the backend engine is not made until after
 * the physical connection is established. Information received over the subscription is made available
 * to the user using \ref EspSubscriberEvent objects. In direct access modes, these events are retrieved directly
 * by user code using esp_subscriber_get_next_event() calls. The event so retrieved needs to be freed using
 * esp_subscriber_event_free(). In callback access, the events are sent to the registered
 * callback handlers. In selection access the events are returned in the list from esp_selector_select()
 * call. These events are freed by the SDK. Typical way to create a subscription is direct access mode would be
 *
 * \code
 * // Create the error object, this is used for all SDK calls
 * // to retrieve error information in any
 * EspError * error = esp_error_create();
 * 
 * esp_sdk_start(error);
 * 
 * // Credentials
 * EspCredentials * creds = esp_credentials_create(ESP_CREDENTIALS_USER_PASSWORD, error);
 * esp_credentials_set_user(creds, "user", error);
 * esp_credentials_set_password(creds, "password", error);
 * 
 * EspUri * uri = esp_uri_create_string("esp://clusterhost:19011/default/testproject", error);
 * 
 * // Get the project. As mentioned earlier subscribers can be retrieved without
 * // first getting the project
 * EspProject * project = esp_project_get(uri, creds, NULL, error);
 * int rc = esp_project_connect(project, error);
 * 
 * // Create a subscriber with default options
 * EspSubscriber * subscriber = esp_project_create_subscriber(project, NULL, error);
 * 
 * rc = esp_subscriber_connect(subscriber, error);
 * rc = esp_subscriber_subscribe(subscriber, stream, error);
 *
 * // Next retrieve events and process them
 * \endcode
 *
 * In whatever way an EspSubscriberEvent is obtained (direct, callback or select), subsequent processing is identical.
 * There are various subscription event types returned by the SDK, most of which signify a state change or an 
 * event in the engine. The \ref ESP_SUBSCRIBER_EVENT_DATA event denotes that there is data available
 * in the Subscriber. The SDK makes the data available using the classes \ref EspMessageReader and
 * \ref EspRowReader. Each data event contains a \ref EspMessageReader which can be retrieved using
 * esp_subscriber_event_get_message_reader(). EspMessageReader denotes a data block from a single stream.
 * There will one or more data rows depending on how the engine has packaged them. Users iterate over all the
 * contained \ref EspRowRowReader objects and retrieve the data for the columns they are interested in. Details
 * about the schema can be obtained from the \ref EspStream and \ref EspSchema objects. Typical usage example ...
 * <p>
 * 
 * \code
 * EspSubscriberEvent * event = NULL;
 * 
 * // Get the stream object corresponding to the stream we will be subscribing to
 * const EspStream * stream = esp_project_get_stream(project, "Stream", error);
 * const EspSchema * schema = esp_stream_get_schema(stream, error);
 * 
 * // Will need this structure to read money values
 * EspMoney * moneyValue = esp_money_create(error);
 * 
 * // Determine number of columns in the schema
 * int32_t numcols;
 * rc = esp_schema_get_numcolumns(schema, &numcols, error);
 * // Will need to provide application appropriate termination logic
 * while (true)
 * {
 *     event = esp_subscriber_get_next_event(subscriber, error);
 *     uint32_t type;
 *     rc = esp_subscriber_event_get_type(event, &type, error);
 *     switch (type) {
 *     case ESP_SUBSCRIBER_EVENT_SYNC_START:
 *     case ESP_SUBSCRIBER_EVENT_SYNC_END:
 *     case ESP_SUBSCRIBER_EVENT_DISCONNECTED:
 *     case ESP_SUBSCRIBER_EVENT_ERROR:
 *         break;
 *         // we have data    
 *     case ESP_SUBSCRIBER_EVENT_DATA:
 *         EspMessageReader * reader = esp_subscriber_event_get_reader(event, error);
 *         // Loop until we have read all rows
 *         while ((row_reader = esp_message_reader_next_row(reader, error)) != NULL) {
 *             for (int i = 0; i < numcolumns; ++i) {
 *                 int flag;
 *                 rc = esp_row_reader_is_null(row_reader, i, &flag, error);
 *                 if ( flag ) 
 *                     continue;
 *                 rc = esp_schema_get_column_type(schema, i, &type, error);
 *             
 *                 switch ( type ) {
 *                 case ESP_DATATYPE_INTEGER:
 *                     rc = esp_row_reader_get_integer(row_reader, i, &int_value, error);
 *                     break;
 *                 case ESP_DATATYPE_LONG:
 *                     rc = esp_row_reader_get_long(row_reader, i, &long_value, error);
 *                     break;
 *                 case ESP_DATATYPE_MONEY02:
 *                    rc = esp_row_reader_get_money(row_reader, i, money_value, error);
 *                    rc = esp_money_get_long(money_value, &long_value, error);
 *                    rc = esp_money_get_precision(money_value, &prec, error);
 *                    break;
 *                 }
 *             }
 *         }
 *     }
 *     esp_subscriber_event_free(event, error);
 * }
 * esp_money_free(money_value, error);
 * rc = esp_subscriber_close(subscriber, error);
 * 
 * \endcode
 * 
 * \see EspSubscriberOptions
 * \see EspSubscriberEvent
 * \see EspMessageReader
 * \see EspRowReader
 * \see EspSchema
 */

/*!
 * \addtogroup EspSubscriberEventGroup
 * @{
 */
#define ESP_SUBSCRIBER_EVENT_CONNECTED          0x04000001      /*!< Connection to the server established. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_SUBSCRIBED         0x04000002      /*!< A subscription request completed. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_UNSUBSCRIBED       0x04000004      /*!< An unsubscription request has completed. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_DISCONNECTED       0x04000008      /*!< Connection closed @hideinitializer. */
#define ESP_SUBSCRIBER_EVENT_CLOSED             0x0400000A      /*!< Subscriber closed, will not generate events or accept commands. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_DATA               0x04000010      /*!< There is data on the line. Stream and MessageReader properties are valid. @hideinitializer */ 
#define ESP_SUBSCRIBER_EVENT_SYNC_START         0x04000020      /*!< Sync start / start of image. Stream property is valid. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_SYNC_END           0x04000040      /*!< Sync end / end of image. Stream property is valid. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_DATA_INVALID       0x04000080      /*!< The data for the stream is no longer valid. Stream property is valid. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_STREAM_EXIT        0x040000A0      /*!< A stream in the project has closed. Stream property is valid. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_DATA_LOST          0x04000100      /*!< Engine has detected data loss. Stream property is valid. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_ERROR              0x04000200      /*!< An error occured while subscribing. @hideinitializer */
#define ESP_SUBSCRIBER_EVENT_ALL                0x040003FF      /*!< Utility define indicating all EspSubscriber events. @hideinitializer */
#define ESP_SUBSCRIBER_CHECKPOINT               0x04000400      /*!< This indicates checkpoint event occured in the server. @hideinitializer */

/*! @} */

/*!
 * \defgroup SubFlags Subscription Flags
 * @{
 * Enumerations to control how a subscription works
 */


/*!
 * Setting this to true, puts the ESP Project in "data shedding mode", where the oldest
 * data is dropped when a client cannot keep up with the pace of the data coming out of the ESP Project.
 * This ensures that when the client does read gateway data, it is always the most recent data that the 
 * ESP Project has delivered to the output gateway.
 * @hideinitializer
 */
#define ESP_SUBSCRIBER_FLAG_LOSSY           0x01
/*!    
 * Specifying this flag, tells ESP Project that it should not send the 'base' data. Base data is sent by
 * default as a complete "snapshot" of each stream of the subscription request before sending subsequent updates to each 
 * stream. The complete snapshot or "state" of the stream is a group of "insert" records sent from the ESP Project between
 * the \ref ESP_SUBSCRIBER_EVENT_SYNC_START and \ref ESP_SUBSCRIBER_EVENT_SYNC_END events. 
 * @hideinitializer
 */
#define ESP_SUBSCRIBER_FLAG_NOBASE          0x02
/*!
 * Setting this to true tells the ESP Project that if the client (i.e., the application using this setting)
 * can't keep up with the data it is sending and its internal buffer is filled then drop the connection to
 * the client application. This protects the ESP Project from getting into a situation
 * where it has to stop processing incoming data because the its clients can't keep up with the data it is
 * producing.  Default is false.
 * @hideinitializer
 */
#define ESP_SUBSCRIBER_FLAG_DROPPABLE       0x08
/*!
 * If set to true, ESP Project preserves the block structure of the data it sends to clients. It will
 * match the block structure of the data ESP Project receives.  Default is false.
 * @hideinitializer
 */
#define ESP_SUBSCRIBER_FLAG_PRESERVE_BLOCKS 0x20

/*!
 * If this flag is set to true, when updates do not contain new data for certain fields, those fields will
 * be set to shinethough. Such fields can be detected using the \ref esp_row_reader_is_shinethrough()
 * call.
 * @hideinitializer
 */
#define ESP_SUBSCRIBER_FLAG_SHINETHROUGH    0x40

/*! @} */

/*!
 * \defgroup EspSubscriberOptionsGroup Subscription Options
 * @{
 * Methods that control options for creation of subscription objects
 */

/*!
 * Creates a new EspSubscriberOptions object.
 * 
 * @return NULL on error.
 */
ESPAPICALL EspSubscriberOptions * esp_subscriber_options_create(
        EspError * error);

/*!
 * Frees an EspSubscriberOptions object.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_free(
        EspSubscriberOptions * options,
        EspError * error);

/*!
 * Set the access mode for the Subscriber that will be created using this options object.
 * Default is \ref DIRECT_ACCESS
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_access_mode(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        ESP_ACCESS_MODE_T mode,                         /*!< The access mode to use */
        EspError * error);                              /*!< Error details on error */

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_subscriber_options_set_dispatcher(
        EspSubscriberOptions * options, 
        const EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_subscriber_options_set_updater(
        EspSubscriberOptions * options,
        const EspUpdater * updater,
        EspError * error);

/*!
 * Set subscription flags.
 * \see SubFlags
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_flags(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        uint32_t flags,                                 /*!< A bitwise OR'd ESP_SUBSCRIBER_FLAG_xxx values */
        EspError * error);                              /*!< Error details on error */

/*!
 * Set the time limit, in milliseconds, that this client has to read all base
 * data from the subscribed stream before being dropped. When this parameter
 * is not specified, the default value is 8,000 milliseconds or 8 seconds.
 * Values less than 1000 milliseconds have no effect. 
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_base_drain_timeout(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        int32_t timeout,                                /*!< Maximum time in milliseconds */
        EspError * error);                              /*!< Error details on error */

/*!
 * Sets a pulse interval. If set, ESP Project will enter pulsed subcribe mode. In this mode
 * instead of sending the data as soon as it is available, ESP Project consolidates the data
 * for the specified interval, and sends once the interval expires. This can be useful, if
 * a client does not need each individual records but is instead is interested in the latest
 * state of a stream/window.  By default pulsing is not enabled.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_pulsed(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        int32_t interval,                               /*!< Pulse interval in seconds. */
        EspError * error);                              /*!< Error details on error */

/*!
 * This is used to set the internal buffer size in the ESP Project for this subscription.
 * ESP Project uses this buffer to queue up messages if the subscriber is slow in retrieving them.
 * The buffer prevents it from blocking and slowing down. The setting is made when the subscription is started. 
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_queuesize(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        int32_t size,                                   /*!< Queue size to set */
        EspError * error);                              /*!< Error details on error */


/*!
 * This is used for setting the GD session for a subscriber. A GD session to a subscriber is
 * uniquly identified by the GD session name for the user. So it is important to set unique 
 * GD session name per user.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_options_set_gd_session(
        EspSubscriberOptions * options,                 /*!< Options object to modify */
        char * session_name,                      /*!< Session name to be set. It should unique to the user. */
        EspError * error);                              /*!< Error details on error */


/*! @} */


/*!
 * \defgroup EspSubscriberEventGroup Subscription Events
 * @{
 * Methods and enumerations to support subscription events
 */

/*!
 * Retrieve the event type for the EspSubscriberEvent.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_event_get_type(
        const EspSubscriberEvent * event,
        uint32_t * type,
        EspError * error);                              /*!< Error details on error */

/*!
 * Retrieve the EspSubscriber object that raised this event.
 *
 * @return EspSubscriber, NULL on error.
 */
ESPAPICALL EspSubscriber * esp_subscriber_event_get_subscriber(
        const EspSubscriberEvent * event,
        EspError * error);

/*!
 * Retrieve the EspStream for which this event is intended.
 *
 * @return EspStream, NULL on error.
 */
ESPAPICALL const EspStream * esp_subscriber_event_get_stream(
        const EspSubscriberEvent * event,
        EspError * error);

/*!
 * Retrieve a message reader for the event. This will return a valid \ref EspMessagerReader
 * only for ESP_SUBSCRIBER_EVENT_DATA events.
 *
 * @return EspMessageReader, NULL on error.
 */
ESPAPICALL EspMessageReader * esp_subscriber_event_get_reader(
        const EspSubscriberEvent * event,
        EspError * error);

/*!
 * Retrieve the checkpoint sequence number from the checkpoint event which is received from the server.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_subscriber_event_get_checkpoint_sequence_number(
                const EspSubscriberEvent * event,
                int64_t * seq_val,
                EspError * error);

/*!
 * For ESP_SUBSCRIBER_EVENT_ERROR events retrieve the associated EspError object.
 *
 * @return EspError, NULL on error.
 */
ESPAPICALL EspError * esp_subscriber_event_get_error(
        const EspSubscriberEvent * event,
        EspError * error);

/*!
 * Frees the \ref EspSubscriberEvent object that has been retrieved in DIRECT mode using
 * esp_subscriber_get_next_event();
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_subscriber_event_free(
        EspSubscriberEvent * event,             /*!< EspSubscriberEvent to free */
        EspError * error);

/*! @} */


/*!
 * Creates an EspSubscriber object. The created object must be freed using \ref esp_subscriber_free().
 *
 * @return EspSubscriber, NULL on error.
 */
ESPAPICALL EspSubscriber * esp_subscriber_create(
        const EspUri * uri,                         /*!< URI of the project to which subscription is to be established */
        const EspCredentials * credentials,         /*!< EspCredentials object for authentication */
        const EspSubscriberOptions * options,       /*!< Options to use for the EspSubscriber */
        EspError * error);

/*!
 * Frees an EspSubscriber created using \ref esp_subscriber_create().
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_free(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to free */
        EspError * error);

/*!
 * Retrieve the parent EspProject.
 *
 * @return EspProject, NULL on error.
 */
ESPAPICALL EspProject * esp_subscriber_get_project(
        EspSubscriber * subscriber,
        EspError * error);


/*!
 * Returns 0 if subscriber is GD enabled
 *
 * @return 0 if GD enabled.
 */
ESPAPICALL int32_t esp_subscriber_is_gd_enabled(
        EspSubscriber * subscriber,
        EspError * error);                          /*!< Error details on error */


/*!
 * Connect to the backend project. In DIRECT access mode returns when connected.
 * In CALLBACK/SELECT will generate an ESP_SUBSCRIBER_EVENT_CONNECTED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_connect(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to connect */
        EspError * error);

/*!
 * Disconnect from the project. In DIRECT access mode returns when disconnected.
 * In CALLBACK/SELECT will generate an ESP_SUBSCRIBER_EVENT_DISCONNECTED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_disconnect(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to disconnect */
        EspError * error);

/*!
 * Closes this instance of EspSubscriber. In DIRECT access mode returns when the entity is closed.
 * In CALLBACK/SELECT will generate an ESP_SUBSCRIBER_EVENT_CLOSED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_close(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to close */
        EspError * error);

/*!
 * Subscribe to the specified stream. This call must be made after the subscriber
 * has connected.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_subscribe(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to use for the subscription */
        const EspStream * stream,                   /*!< Stream to which to subscribe */
        EspError * error);

/*!
 * Reserved for future use
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_subscriber_subscribe_streamname(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to use for the subscription */
        const char * stream_name,                   /*!< Name of the stream to subscribe to */
        EspError * error);

/*!
 * Subscribes using the provided SQL statement. 
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_subscribe_sql(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to use for the subscription */
        const char * sql,                           /*!< SQL statement to use as a projection for the data */
        EspError * error);

/*!
 * Retrieves a list of streams currently subscribed to.
 * If this is an SQL subscription, the list will contain a single EspStream
 * with the type SQL_QUERY and a schema that corresponds to the SQL statement
 * that was used to subscribe.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_get_subscribed_streams(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to query */
        EspList * stream_list,                      /*!< Pointer to EspList in which to return the stream list */
        EspError * error);

/*!
 * Registers a callback handler.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_set_callback(
        EspSubscriber * subscriber,                 /*!< a valid EspSubscriber object */
        uint32_t events,                            /*!< bitwise OR'd values of ESP_SUBSCRIBER_EVENT_xxx values */
        SUBSCRIBER_CALLBACK_T callback,             /*!< callback pointer to the handler function */
        void * user_data,                           /*!< pointer to user data that will be passed back by the SDK */
        EspError * error);


/*!
 * Use the specified \ref EspSelector to monitor events for this EspSubscriber.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_subscriber_select_with(
        EspSubscriber * subscriber,                 /*!< EspSubscriber to monitor */
        EspSelector * selector,                     /*!< EspSelector with which to monitor */
        uint32_t events,                            /*!< events to monitor for - bitwise OR'd values of ESP_SUBSCRIBER_EVENT_xxx values */
        EspError * error);

/*!
 * Determine the next event pending at the subscriber. This call should be made
 * in DIRECT ACCESS mode only. In other access modes, the events are retrieved by the
 * SDK and are either passed to the callback handler or returned from \ref esp_selector_select()
 * call.
 *
 * The event retrieved by this call must be freed using \ref esp_subscriber_event_free(). This is the only instance 
 * where an event needs to be explicitly freed by user code.
 *
 * @return EspSubscriberEvent, NULL on error.
 */
ESPAPICALL EspSubscriberEvent * esp_subscriber_get_next_event(
        EspSubscriber * subscriber,             /*!< EspSubscriber to query */
        EspError * error);                      /*!< Error details on error */

/*!
 * \defgroup EspMessageReaderGroup Data Access
 * @{
 * Object and methods to support reading data fields from data blocks returned by subscribers
 */

/*!
 * Check if the data is a transaction or an envelope block.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_message_reader_is_block(
        const EspMessageReader * reader,        /*!< EspMessageReader to query */
        int * is_block,                         /*!< Pointer to contain block indication. */
        EspError * error);                      /*!< Error details on error */

/*!
 * Return an EspRowReader for the next row. If there are no more rows returns NULL.
 *
 * @return EspRowReader.
 */
ESPAPICALL EspRowReader * esp_message_reader_next_row(
        const EspMessageReader * reader,        /*!< EspMessageReader to read the next row from */
        EspError * error);

/*!
 * Retrieve the EspStream for this message reader.
 *
 * @return EspStream, NULL on error.
 */
ESPAPICALL EspStream * esp_message_reader_get_stream(
        const EspMessageReader * reader,        /*!< EspMessageReader to query */
        EspError * error);

/*!
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_messge_reader_get_numoptions(
        const EspMessageReader * reader,
        int * num_options,
        EspError * error);

/*!
 * Retrieve the raw data buffer contents for this message. The method checks the
 * passed buffer length. If sufficient space is not available, returns an error
 * and the required buffer size in buffsize. If data is successfully copied
 * the copied data length is returned in buffsize.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_message_reader_get_buffer(
        const EspMessageReader * reader,        /*!< EspMessageReader to query */
        char * buffer,                          /*!< Pointer to buffer to receive the data */
        uint32_t * buffsize,                    /*!< On input size of the buffer being passed, on output size of the data */
        EspError * error);


/*!
 * Get the opcode in this data row.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_opcode(
        const EspRowReader * reader,            /*!< EspRowReader to query */
        ESP_OPERATION_T * opcode,               /*!< pointer to an ESP_OPERATION_T to return the opcode */
        EspError * error);

/*!
 * Return the flags values present in the data row.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_flags(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        int32_t * flags,                            /*!< Variable to hold the flags */
        EspError * error);

/*!
 * Is the column null.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_is_null(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int * is_null,                              /*!< Variable to hold the return value */
        EspError * error);

/*!
 * Is the column shinethrough?
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_is_shinethrough(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int * is_shinethrough,                      /*!< Variable to hold the return value */
        EspError * error);

/*!
 * Retrieve an integer value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_integer(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int32_t * int_value,                        /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Retrieve a long value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_long(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int64_t * long_value,                       /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Retrieve a float value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_float(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        double * double_value,                      /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Return the string value at column 'column'. Expects a buffer large
 * enough to hold the value. If it is not, the function returns an error
 * and the required buffer size in parameter 'buffsize'
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_row_reader_get_string(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        char * string_value,                        /*!< Pointer in which to return the value */
        uint32_t * buffsize,                        /*!< On input size of the passed buffer, on output the actual string size */
        EspError * error);

/*!
 * Returns the date value at column 'column' if not null or shinethrough.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_date(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        time_t * date_value,                        /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Retrieve a time data value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_time(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        EspTime * time_value,                        /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Retrieve a fixed decimal data value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_fixeddecimal(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        EspFixedDecimal * fixeddecimal_value,       /*!< Pointer in which to return the value */
        EspError * error);



/*!
 * Retrieve a timestamp data value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_timestamp(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int64_t * timestamp_value,                  /*!< Pointer in which to return the value */
        EspError * error);

/*!
 * Retrieve a legacy money data type value in an \ref EspMoney object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_legacy_money(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        EspMoney * money_value,                     /*!< Pointer to EspMoney to fill in the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieves a legacy money data type value at the specified column as an int64_t.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_legacy_money_as_long(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int64_t * long_value,                       /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve EspMoney value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_money(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        EspMoney * money_value,                     /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve money value as an int64_t. Avoids the creation of an \ref EspMoney
 * object to store the value. Potentially faster.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_money_as_long(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int64_t * long_value,                       /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve EspBigDatetime value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_bigdatetime(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        EspBigDatetime * bigdatetime_value,         /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve a binary value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_binary(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        void * binary_value,                        /*!< Pointer in which to return the value */
        uint32_t * buffsize,                        /*!< On input size of buffer, on return size of the binary value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve boolean value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_boolean(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int32_t * boolean_value,                    /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*!
 * Retrieve an interval value at the specified column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_row_reader_get_interval(
        const EspRowReader * reader,                /*!< EspRowReader to query */
        const int32_t column,                       /*!< Column number to query */
        int64_t * interval_value,                   /*!< Pointer in which to return the value */
        EspError * error);                          /*!< Error details on error */

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* __ESP_SUBSCRIBER_H */
