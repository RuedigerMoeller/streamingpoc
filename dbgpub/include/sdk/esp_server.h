/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_SERVER_H
#define __ESP_SERVER_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspServerGroup Server
 * @{
 */

/*!
 * \defgroup EspServerEventGroup Server Events
 * @{
 * Events that can occur on an EspServer object.
 */
/*!
 * Connection to server established
 * @hideinitializer.
 */
#define ESP_SERVER_EVENT_CONNECTED                  0x01000001
/*!
 * Change in managers on the server. Will be sent at least once.
 * @hideinitializer.
 */  
#define ESP_SERVER_EVENT_MANAGER_LIST_CHANGE        0x01000002
/*!
 * Change in controllers on the server. Will be sent at least once.
 * @hideinitializer.
 */
#define ESP_SERVER_EVENT_CONTROLLER_LIST_CHANGE     0x01000004
/*!
 * Change in workspaces on the server. Will be sent at least once.
 * @hideinitializer.
 */
#define ESP_SERVER_EVENT_WORKSPACE_LIST_CHANGE      0x01000008
/*!
 * This event is sent if there is a change in application membership of this server, ie. 
 * new application(s) have been added or existing application(s) have dropped. 
 * This is also sent if there is a change in the status of one of the existing applications.
 * Will always be sent at least once after the client connects to server.
 * @hideinitializer
 */
#define ESP_SERVER_EVENT_APPLICATION_LIST_CHANGE    0x01000010
/*!
 * Connection to the server manager has closed; no reconnection will happen.
 * All runtime properties are invalidated.
 * @hideinitializer.
 */
#define ESP_SERVER_EVENT_DISCONNECTED               0x01000020
/*!
 * The \ref EspServer object is closed and cannot be used anymore
 * @hideinitializer.
 */
#define ESP_SERVER_EVENT_CLOSED                     0x01000040
/*!
 * Error processing an update or a request
 * @hideinitializer. 
 */
#define ESP_SERVER_EVENT_ERROR                      0x01000080
/*! 
 * This event is sent in \ref CALLBACK_ACCESS and \ref SELECTION_ACCESS access modes,
 * if connection retries are enabled using esp_server_options_set_retrycount().
 * This is sent when Server first detects loss of connection with all known managers.
 * @hideinitializer
 */
#define ESP_SERVER_EVENT_STALE                      0x01000100
/*!
 * This event is sent in \ref CALLBACK_ACCESS and \ref SELECTION_ACCESS access modes.
 * This is sent when a server in stale state succeeds in reconnecting to a manager.
 * @hideinitializer
 */
#define ESP_SERVER_EVENT_UPTODATE                   0x01000200
/*!
 * Utility define denoting all EspServer events
 * @hideinitializer. 
 */
#define ESP_SERVER_EVENT_ALL                        0x01000FFF

#define ESP_SERVER_EVENT_PROJECT_LIST_CHANGE        0x01000010  /*!< Temporary define until code is changed @hideinitializer. */

/*! @} */

/*!
 * \defgroup EspServerOptionsGroup Server Options
 * @{
 */

/*!
 * Creates a server options object with default values.
 */
ESPAPICALL EspServerOptions * esp_server_options_create(
        EspError * error);

/*!
 * Sets the mode the entity will be accessed with - DIRECT_ACCESS, CALLBACK_ACCESS or SELECTION_ACCESS.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_options_set_access_mode(
        EspServerOptions * server_options,
        ESP_ACCESS_MODE_T mode,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_server_options_set_dispatcher(
        EspServerOptions * server_options,
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_server_options_set_updater(
        EspServerOptions * server_options,
        EspUpdater * updater,
        EspError * error);

/*!
 * This parameter has relevance only in \ref CALLBACK_ACCESS and \ref SELECTION_ACCESS
 * access modes. If set to nonzero, when \ref EspServer loses connection with all known managers, it will
 * not generate an \ref ESP_SERVER_EVENT_DISCONNECTED event right away. It will generate an 
 * \ref ESP_SERVER_EVENT_STALE event and then will poll for managers for the configured number of retries.
 * If a connection is reestablished an \ref ESP_SERVER_EVENT_UPTODATE is sent. If the connection is not
 * established, an \ref ESP_SERVER_EVENT_DISCONNECTED is sent.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_options_set_retrycount(
        EspServerOptions * server_options,      /*!< EspServerOptions to modify */
        const uint32_t count,                   /*!< Number of times to try to reconnect */
        EspError * error);

/*!
 * Free a server options object previously obtained using esp_server_options_create().
 */
ESPAPICALL int32_t esp_server_options_free(
        EspServerOptions * server_options,
        EspError * error);

/*! @} */

/*!
 * Retrieve an EspServer object corresponding to the specified URI.
 *
 * @return EspServer, NULL on error
 */
ESPAPICALL EspServer * esp_server_get(
        const EspUri * uri,                         /*!< Cluster URI */
        const EspCredentials * cred,                /*!< credentials for authentication */
        const EspServerOptions * server_options,    /*!< Options to create EspServer with */
        EspError * error);

/*!
 * This must be the first call made to a newly retrieved server. A connection
 * must be established for all other functionality to work.
 * <p>
 * In \ref DIRECT_ACCESS access mode this tries to establish a connection and returns an error
 * if it fails.  In CALLBACK_ACCESS and SELECTION_ACCESS mode, the connect request is queued and the call returns
 * immediately. If the connection is successful, registered handlers will receive an
 * \ref ESP_SERVER_EVENT_CONNECTED event, otherwise they will receive an \ref ESP_SERVER_EVENT_ERROR.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_connect(
        EspServer * server,                     /*!< EspServer to connect */
        EspError * error);
        
/*!
 * This must be the first call made to a newly retrieved server. A connection
 * must be established for all other functionality to work. 
 * it will return a session id
 * <p>
 * In \ref only support DIRECT_ACCESS  access mode, it tries to establish a connection and returns an error
 * if it fails.  
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_connect_with_sessionid(
        EspServer * server,                     /*!< EspServer to connect */
        char * session_id,                     /*!< session id buff */
        int32_t size,                          /*!< session id buff size */ 
        EspError * error);        
        

/*!
 * Retrieves the \ref EspProject object corresponding to the "project_name".
 * A single engine process is always represented by a single instance of an \ref EspProject. Request 
 * for the same "project_name" return references to the same instance. This instance remains valid until an 
 * esp_project_close() has been called.
 * <p>
 * In DIRECT_ACCESS mode, the first time this call is made, the SDK initiates a call to the back end cluster.
 * The retrieved instance is cached and all subsequent calls return the cached instance.
 * <p>
 * In CALLBACK_ACCESS or SELECTION_ACCESS modes, this call is valid only after the \ref EspServer has received
 * an \ref ESP_SERVER_EVENT_APPLICATION_LIST_CHANGE event. Calls made before that may erroneously return an error.
 *
 * @return NULL on error
 */
ESPAPICALL EspProject * esp_server_get_project(
        EspServer * server,                         /*!< EspServer the project is running in */
        const char * workspace,                     /*!< workspace name to query */
        const char * project_name,                  /*!< name of the project */
        EspProjectOptions * project_options,        /*!< optional options to access the EspProject object with */
        EspError * error);

/*!
 * Registers this \ref EspServer with the specified \ref EspSelector. Events to wait for
 * are specifying by bitwise OR'in ESP_SERVER_EVENT_XXXX values.
 *
 * @see EspServerEventGroup
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_select_with(
        EspServer * server,             /*!< The EspServer object to monitor for events */
        EspSelector * selector,         /*!< The EspSelector to use for monitoring */
        uint32_t events,                /*!< The events to monitor for */
        EspError * error);              /*!< Details in case of error */


/*!
 * Disconnects the \ref EspServer object from the backend cluster. In direct mode,
 * EspServer is disconnected on return from the call. In callback/select access modes,
 * the call returns immediate. An \ref ESP_SERVER_EVENT_DISCONNECTED event is generated when the
 * object is actually disconnected from the backend.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_disconnect(
        EspServer * server,             /*!< EspServer to disconnect */
        EspError * error);

/*!
 * Closes the EspServer object. This object instance can no longer be used.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_close(
        EspServer * server,
        EspError * error);

/*!
 * Registers a callback handler function for the server.
 *
 * @see EspServerEventGroup
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_set_callback(
        EspServer * server,             /*!< EspServer for which to register callbacks */
        uint32_t events,                /*!< events to monitor - OR'd ESP_SERVER_EVENT_XXX values */
        SERVER_CALLBACK_T callback,     /*!< callback handler function */
        void * user_data,               /*!< pointer to data that will be passed back to the registered function */
        EspError * error);

/*!
 * Deregisters a callback function on the server.
 * 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_server_remove_callback(
        EspServer * server,             /*!< EpsServer from which to remove the callback */
        SERVER_CALLBACK_T callback,     /*!< Previously registered callback handler */
        EspError * error);

/*!
 * \addtogroup EspServerEventGroup
 * @{
 */
/*!
 * Retrieve the type of the server event
 * @see EspServerEventGroup
 * @return 0 on error
 */
ESPAPICALL int32_t esp_server_event_get_type(
        const EspServerEvent * event,   /*!< Server event to query */
        uint32_t * type,                /*!< Pointer to variable to return the type in */
        EspError * error);

/*!
 * Retrieve the \ref EspServer instance which generated this event
 * @return 0 on error
 */
ESPAPICALL EspServer * esp_server_event_get_server(
        const EspServerEvent * event,   /*!< Server event to query */
        EspError * error);

/*!
 * Retrieve error details if this is an \ref ESP_SERVER_EVENT_ERROR event
 * @return 0 on error
 */
ESPAPICALL const EspError * esp_server_event_get_error(
        const EspServerEvent * event,   /*!< Server event to query */
        EspError * error);

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif  /* __ESP_SERVER_H */
