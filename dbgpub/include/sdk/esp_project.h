/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_PROJECT_H
#define __ESP_PROJECT_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspProjectGroup Project
 * @{
 * Objects, methods and enumerations to support interaction with a project.
 * An EspProject represents a running instance of a compiled project in a cluster.
 * A EspProject object can be retrieved either from its parent server or from the global
 * SDK object directly. Once a connection to a running instance of a project has been established, 
 * the Project object can be queried for the constituent streams,
 * subscribed to and published from.
 */

/*!
 * \addtogroup EspProjectEventGroup
 * @{
 */

#define ESP_PROJECT_EVENT_CONNECTED         0x02000001  /*!< Project has connected to the backend @hideinitializer. */
#define ESP_PROJECT_EVENT_DISCONNECTED      0x02000002  /*!< Project has disconnected from the backend @hideinitializer. */
#define ESP_PROJECT_EVENT_CLOSED            0x02000004  /*!< Project entity is closed @hideinitializer. */
#define ESP_PROJECT_EVENT_ERROR             0x02000008  /*!< Project has encountered an error @hideinitializer. */
#define ESP_PROJECT_EVENT_STALE             0x02000010  /*!< Reconnect retries are enabled and the project has stopped @hideinitializer. */
#define ESP_PROJECT_EVENT_UPTODATE          0x02000020  /*!< Reconnect retries are enabled and the SDK has reconnected to the project @hideinitializer. */
#define ESP_PROJECT_EVENT_ALL               0x0200003F  /*!< Utility enum to represent all project events @hideinitializer. */

/*! @} */

/*!
 * \defgroup EspProjectOptionsGroup Project Options
 * @{
 */

/*!
 * Creates a project options object with default values. This must be freed using \ref esp_project_options_free().
 */
ESPAPICALL EspProjectOptions * esp_project_options_create(
        EspError * error);

/*!
 * Sets the mode project will be accessed with - DIRECT, CALLBACK or SELECT.
 * \sa AccessModes
 */
ESPAPICALL int32_t esp_project_options_set_access_mode(
        EspProjectOptions * project_options,            /*!< EspProjectOptions to modify */
        ESP_ACCESS_MODE_T mode,                         /*!< access mode to use */
        EspError * error);

/*!
 * Not in R5.
 * Sets a custom EspDispatcher to handle events for this EspProject.
 */
ESPAPICALL int32_t esp_project_options_set_dispatcher(
        EspProjectOptions * project_options,        /*!< EspProjectOptions to modify */
        EspDispatcher * dispatcher,                 /*!< EspDispatcher to use */
        EspError * error);

/*!
 * Not in R5.
 * Sets a custom EspUpdater to handle events for this EspProject.
 */
ESPAPICALL int32_t esp_project_options_set_updater(
        EspProjectOptions * project_options,        /*!< EspProjectOptions to modify */
        EspUpdater * updater,                       /*!< EspUpdater to use */
        EspError * error);

/*!
 * Setting this to a positive value, turns on automatic reconnect to a project. This
 * works only in CALLBACK and SELECT access modes.
 * 
 * Once a connection loss is detected, the SDK will generate an ESP_PROJECT_EVENT_STALE
 * event. It will then try specified number of times to reconnect. If reconnection is
 * successful, an ESP_PROJECT_EVENT_UPTODATE will be generated. If the reconnection fails
 * after all retries are exhausted, it will generate an ESP_PROJECT_EVENT_DISCONNECTED
 * event.
 * 
 * Reconnection attempts are made at the update interval, which is 2 seconds.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_project_options_set_retrycount(
        EspProjectOptions * project_options,        /*!< EspProjectOptions to change */
        int32_t count,                              /*!< number of retries to make */
        EspError * error);                          /*!< details in case of error */

/*!
 * Must be called to free the resources used by EspProjectOptions that
 * was created using esp_project_options_create().
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_project_options_free(
        EspProjectOptions * project_options,        /*!< EspProjectOptions to free */
        EspError * error);

/*! @} */

/*!
 * Retrieves a project object corresponding to the URI.
 *
 * @return NULL in case of error.
 */
ESPAPICALL EspProject * esp_project_get(
        const EspUri * uri,                         /*!< uri for the project to retrieve */
        const EspCredentials * creds,               /*!< valid EspCredentials for authorisation */
        const EspProjectOptions * project_options,  /*!< valid EspProjectOption or NULL for defaults */
        EspError * error);

/*!
 * Retrieves a project object to connect to a standalone project started outside 
 * the server framework.
 *
 * @returns NULL if error.
 */
ESPAPICALL EspProject * esp_project_get_standalone(
        const char * host,                          /*!< Host name where the standalone project is running */
        const int32_t port,                         /*!< Command and control port for the standalone project */
        const EspCredentials * creds,               /*!< valid EspCredentials for authorization */
        const EspProjectOptions * project_options,  /*!< valid EspProjectOption or NULL for defaults */
        EspError * error);

/*!
 * Establishes a connection to the project specified in the URI when creating the object.
 * In DIRECT access mode, call returns when the connection is established. In CALLBACK/SELECT
 * it returns immediately. Completion is indicated by an ESP_PROJECT_EVENT_CONNECTED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_connect(
        EspProject * project,
        EspError * error);

/*!
 * Disconnects from a connected project.
 * In DIRECT access mode, call returns when EspProject disconnects. In CALLBACK/SELECT
 * returns immediately. Completion is indicated by an ESP_PROJECT_EVENT_DISCONNECTED event.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_project_disconnect(
        EspProject * project,
        EspError * error);

/*!
 * Once closed this project object is no longer available for interaction
 * In DIRECT access mode, call returns when EspProject closes. In CALLBACK/SELECT
 * it returns immediately. Completion is indicated by an ESP_PROJECT_EVENT_CLOSED event.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_close(
        EspProject * project,
        EspError * error);

/*!
 * Retrieve the parent EspServer for this project.
 *
 * @return NULL on error.
 */
ESPAPICALL EspServer * esp_project_get_server(
        EspProject * project,
        EspError * error);

/*!
 * Creates an EspSubscriber for this project. The returned EspSubscriber <b>should not</b>
 * be freed explicitly. The lifetime is managed by the SDK.
 *
 * @return NULL on error.
 */
ESPAPICALL EspSubscriber * esp_project_create_subscriber(
        EspProject * project,                       /*!< EspProject to create a subscriber on */
        const EspSubscriberOptions * options,       /*!< EspSubscriberOptions or NULL for defaults */
        EspError * error);

/*!
 * Creates a publisher for this project. The returned EspPublisher <b>should not</b>
 * be freed explicitly. The lifetime is managed by the SDK.
 *
 * @return NULL on error.
 */
ESPAPICALL EspPublisher * esp_project_create_publisher(
        EspProject * project,                       /*!< EspProject to create a publisher on */
        const EspPublisherOptions * options,        /*!< EspPublisher options or NULL for defaults */
        EspError * error);

/*!
 * Retrieves a list of streams that are publicly accessible in the project. ESP_ARRAY_FREE
 * should be called to free the memory used by the ESP_STRING_ARRAY structure.
 * <p>
 * NOTE: This call has been deprecated. Use \ref esp_project_get_model_stream_names()
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_stream_names(
        EspProject * project,
        ESP_STRING_ARRAY * names,
        EspError * error);

/*!
 * Retrieve the list of streams that are explicitly modeled in the current project.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_model_stream_names(
        EspProject * project,                   /*!< EspProject to query */
        EspList * stream_list,                  /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);

/*!
 * Retrieve the list of streams that have been generated by the compiler.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_intermediate_stream_names(
        EspProject * project,                   /*!< EspProject to query */
        EspList * stream_list,                  /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);

/*!
 * Retrieve the list of system streams names.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_system_stream_names(
        EspProject * project,                   /*!< EspProject to query */
        EspList * stream_list,                  /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);

/*!
 * Retrieve the list of errror streams names.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_error_stream_names(
        EspProject * project,              /*!< EspProject to query */
        EspList * stream_list,             /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);


/*!
 * Retrieve an EspStream object for the corresponding stream name.
 *
 * @return EspStream object, NULL in case of error.
 */
ESPAPICALL const EspStream * esp_project_get_stream(
        EspProject * project,              /*!< EspProject to query */
        const char * stream,               /*!< Stream name to retrieve */
        EspError * error);

/*!
 * Check if this EspProject is connected to the backend.
 *
 * @returns 0 on success.
 */
ESPAPICALL int32_t esp_project_is_connected(
        EspProject * project,              /*!< EspProject to query */
        int32_t * connected,               /*!< Pointer to store the state, non-zero if still connected */
        EspError * error);


/*!
 * Check if the backed project is quiesced. A project is quiesced when all its
 * input queues have drained and data is committed to logstores (if any).
 *
 * @returns 0 on success
 */
ESPAPICALL int32_t esp_project_is_quiesced(
        EspProject * project,              /*!< EspProject to query */
        int32_t * quiesced,                /*!< Pointer to store quiesced state, non-zero if quiesced */
        EspError * error);

/*!
 * Block caller until the backend project is quiesced. A project is quiesced when all its
 * input queues have drained and data is committed to logstores (if any). If 'include_clients'
 * parameter is non-zero, a publisher is considered to be an active input queue and engine
 * will not quiesce while a publisher(s) is connected.
 *
 * @returns 0 on success
 */
ESPAPICALL int32_t esp_project_wait_quiesced(
        EspProject * project,              /*!< EspProject to query */
        int32_t include_clients,           /*!< Should connected clients affect the quiesce state */
        EspError * error);

/*!
 * Registers a callback handler for the project in CALLBACK access mode.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_set_callback(
        EspProject * project,               /*!< Pointer to a valid EspProject object */
        uint32_t events,                    /*!< Bitwise OR'd value of ESP_PROJECT_EVENT_XXX types of interest */
        PROJECT_CALLBACK_T callback,        /*!< Pointer to the callback handler function */
        void * data,                        /*!< Pointer to arbitrary data which will be passed back to callback handler */
        EspError * error);


/*!
 * Retrieves a list of GD sessions which are active for this user.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_active_gd_sessions(
        EspProject * project,
        EspList * gd_sessions,                  /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);


/*!
 * Retrieves a list of GD sessions which are inactive for this user.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_get_inactive_gd_sessions(
        EspProject * project,
        EspList * gd_sessions,                  /*!< Pointer to an EspList that will contain the streams on return */
        EspError * error);

/*!
 * Cancel GD subscriber session for this user.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_project_cancel_gd_subscriber_session(
        EspProject * project,
        char * gd_session,                  
        EspError * error);


/*!
 * Registers this EspProject with the specified EspSelector. Waiting on the selector
 * will return when the events registered for are triggered.
 */
ESPAPICALL int32_t esp_project_select_with(
        EspProject * project,               /*!< The EspProject object to monitor for events */
        EspSelector * selector,             /*!< The EspSelector to use for monitoring */
        uint32_t events,                    /*!< The events to monitor for */
        EspError * error);                  /*!< Details in case of error */

/*!
 * \defgroup EspDeployment Project Deployment
 * @{
 */

/*!
 * Returns the runtime deployment information for the connected project. 
 * If there is no error, the returned information is valid at the time the
 * information is returned and remains valid until the project remains connected.
 * No connectivity check is performed when retrieving the data components of an
 * EspDeployment. User code for monitoring connectivity should retrieve
 * the EspDeployment object every time it needs updated information.
 *
 * NOTE - not all members of EspDeployment are valid for standalone projects.
 */
ESPAPICALL const EspDeployment * esp_project_get_deployment(
        const EspProject * project,         /*!< EspProject for which to retrieve the deployment */
        EspError * error);

/*!
 * Is the project running in active-active mode?
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_active_active(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        char * aa,
        int32_t * size,
        EspError * error);

/*!
 * Retrieve a string denoting the project role (primary or secondary) in active-active mode. 
 * The 'size' parameter should contain the size of the 'role' buffer on input. On output
 * contains the actual size of the string copied into 'role'. If the there is insufficient space
 * returns an error.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_role(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        char * role,                                /*!< Buffer to hold the return value */
        int32_t * size,                             /*!< size of 'role' buffer */
        EspError * error);

/*!
 * Get the string ID assigned to this instance
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_instance_id(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        char * id,                                  /*!< Buffer to hold the name */
        int32_t * size,                             /*!< On input size of buffer passed, on output size of string returned */
        EspError * error);


/*!
 * Retrieve the index assigned to this project instance.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_instance_index(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * index,                            /*!< Variable to hold the index */
        EspError * error);


/*!
 * Retrieve the name of the host this project instance had been deployed on
 * by the cluster.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_host(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        char * host,                                /*!< Buffer to hold host name */
        int32_t * size,                             /*!< On input size of buffer, on output size of string returned */
        EspError * error);

/*!
 * Retrieve the command and control port number being used by the project instance.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_port(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * port,                             /*!< Variable to hold the port number */
        EspError * error);

/*!
 * Retrieve the sql port number the deployed instance is listening on. This call is not
 * valid for standalone projects.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_sql_port(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * sql_port,                         /*!< Variable to hold the port number */
        EspError * error);
        
/*!
 * Retrieve the sql ssl the deployed instance is listening on. This call is not
 * valid for standalone projects.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_sql_ssl(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * sql_ssl,                         /*!< Variable to hold the sql ssl setting */
        EspError * error);        

/*!
 * Retrieve the host of the data gateway. SDK connects to the data gateway to publish to
 * and subscribe from the project.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_data_host(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        char * data_host,                           /*!< Buffer to hold the host name */
        int32_t * size,                             /*!< On input size of buffer passed, on output size of string returned */
        EspError * error);

/*!
 * Retrieve the port number of the data gateway. SDK connects to the data gateway to publish
 * to and subscribe from the project.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_data_port(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * data_port,                        /*!< Variable to hold the port number */
        EspError * error);

/*!
 * Not valid for standalone projects.
 */
ESPAPICALL int32_t esp_deployment_get_pid(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * pid,                              /*!< Variable to hold the process id */
        EspError * error);

/*!
 * Is the project instance running on a big endian architecture?
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_is_bigendian(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * big_endian,                       /*!< 0 if little endian, non zero otherwise */
        EspError * error);

/*!
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_address_size(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * address_size,                     /*!< Variable to hold the address size */
        EspError * error);

/*!
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_date_size(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * date_size,                        /*!< Variable to hold the date_size */
        EspError * error);

/*!
 * Return the default precision the project is using. This is a legacy parameter and applies
 * to ESP_DATATYPE_MONEY data types.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_deployment_get_money_precision(
        const EspDeployment * deployment,           /*!< EspDeployment to query */
        int32_t * precision,                        /*!< Variable to hold the precision */
        EspError * error);

/*! @} */

/*!
 * \defgroup EspProjectEventGroup Project Events
 * @{
 */

/*!
 * Retrieve the event type
 */
ESPAPICALL int32_t esp_project_event_get_type(
        const EspProjectEvent * event,
        uint32_t * type,
        EspError * error);
/*!
 * Retrieve the associated EspProject event instance
 */
ESPAPICALL EspProject * esp_project_event_get_project(
        const EspProjectEvent * event,
        EspError * error);

/*!
 * Retrieve EspError instance if the event is an ERROR event
 */
ESPAPICALL const EspError * esp_project_event_get_error(
        const EspProjectEvent * event,
        EspError * error);

/*! @} */

/*!
 * \defgroup EspClockGroup Clock Interface
 * @{
 * The EspClock interface allows users to query and control the logical clock in the connected
 * project. The retrieved clock instance is updated with the timing state of the project when
 * the clock is first retrieved. The parameters that can be queried are rate, time, whether real
 * or in logical mode, pause count and max sleep time.
 * <p>
 * A separate update call \ref esp_clock_get_pause_on_debug() can be made to determine how the clock
 * will behave when the project is switched to debug mode.
 */

/*!
 * This call returns the EspClock object associated with the clock functionality
 * for the connected project. Mmultiple calls to the method will return the same EspClock instance.
 * <p>
 * The clock instance information is updated to the latest timing state in the connected project.
 *
 * @return NULL on error
 */
ESPAPICALL EspClock * esp_project_get_clock(
        EspProject * project,           /*!< EspProject to query */
        EspError * error);

/*!
 * Retrieve the clock rate at the time of last update. Values greater than 1 mean a faster clock.
 * Values between 0 and 1.0 mean a slower clock.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_rate(
        EspClock * clock,               /*!< EspClock instance to query */
        double * rate,                  /*!< Project clock rate */
        EspError * error);

/*!
 * Retrieve the project time when the clock instance was last updated.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_time(
        EspClock * clock,               /*!< EspClock instance to query */
        double * time,                  /*!< Project time as seconds since epoch */
        EspError * error);

/*!
 * Retrieve the state of the clock at the time of last update. A value of 1 indicates the project clock
 * matches the system clock. A value of 0 indicates the project clock has been modified.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_real(
        EspClock * clock,               /*!< EspClock instance to query */
        int32_t * real,
        EspError * error);

/*!
 * Retrieve the pause count of the project clock at the time of last update. A value of 0 means the clock
 * is running. If pause count is greater than 0, corresponding number of calls to \ref esp_clock_resume()
 * will need to be made to restart the clock.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_pause_count(
        EspClock * clock,               /*!< EspClock instance to query */
        int32_t * count,                /*!< Pause count at time of last update */
        EspError * error);

/*!
 * Retrieve the maximum sleep time. This is the maximum time for the clock state
 * changes to be discovered by all affected components in the project. All calls
 * that change clock state (such as \ref esp_clock_set_rate() or \ref esp_clock_set_time())
 * wait for this interval before returning.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_max_sleep(
        EspClock * clock,               /*!< EspClock instance to query */
        int32_t * sleep,                /*!< Maximum time in milliseconds for clock state change to propagate */
        EspError * error);

/*!
 * Update the EspClock instance with the latest timing state in the connected project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_update(
        EspClock * clock,               /*!< EspClock instance to update */
        EspError * error);

/*!
 * Controls the clock behavior when the project is put into debug mode. If 'pause'
 * is 1, the logical clock will pause if the project is switched to debug.
 * <p>
 * Other clock parameters are not modified and remain the same as at the time of the
 * last update.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_set_pause_on_debug(
        EspClock * clock,               /*!< EspClock instance to change */
        int32_t pause,                  /*!< 1 if clock should pause on debug, 0 otherwise */
        EspError * error);

/*!
 * Retrieve the clock behavior when project is switched to debug. This method queries
 * the engine everytime the call is made, so the latest state is returned.
 * <p>
 * Other clock parameters are not modified and remain the same as at the time of the
 * last update.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_get_pause_on_debug(
        EspClock * clock,               /*!< EspClock instance to query */
        int32_t * pause,                /*!< 1 if clock will pause on debug */
        EspError * error);

/*!
 * Pause the clock in the connected project. If the clock is running the clock is paused. If it was
 * already paused increments the pause count.
 * The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_pause(
        EspClock * clock,               /*!< EspClock instance to pause */
        EspError * error);

/*!
 * Resume the clock in the connected project. Decrements the pause count. If the pause count reaches
 * 0 the clock restarts. The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_resume(
        EspClock * clock,               /*!< EspClock instance to resume */
        EspError * error);

/*!
 * Change the clock rate in the connected project. A value greater than 1 causes the cause to run faster, e.g.
 * a value of 10.0 will cause to clock to run 10 times as fast. A value less than 1, slows the clock. For example
 * a value of 0.5 will cause the clock to run twice as slow.
 * The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_set_rate(
        EspClock * clock,               /*!< EspClock instance to update */
        double rate,                    /*!< New clock rate */
        EspError * error);

/*!
 * Update the clock time in the connected project. Time parameter is interpreted as seconds since epoch.
 * The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_set_time(
        EspClock * clock,               /*!< EspClock instance to update */
        double time,                    /*!< New lock time as seconds since epoch */
        EspError * error);

/*!
 * Update the clock rate and time in the connected project.
 * The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_set_rate_time(
        EspClock * clock,               /*!< EspClock instance to update */
        double rate,                    /*!< New clock rate */
        double time,                    /*!< New clock time as seconds sinc epoch */
        EspError * error);

/*!
 * Resets the clock to real time, i.e. the project clock matches the system clock.
 * The clock instance is updated with the latest timing state in the project.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_clock_reset_to_real(
        EspClock * clock,               /*!< EspClock instance to update */
        EspError * error);


/*! @} */


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_PROJECT_H */
