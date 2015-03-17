/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_URI_H
#define __ESP_URI_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspUriGroup Uri
 * @{
 * EspUri objects are used to access or refer to a cluster and projects running in a cluster. From the SDK's point of view
 * a cluster is identified by a list of host/port values. EspUris form the basis for uniquely identifying components within ESP.
 * <p>
 * A Uri is resolved or interpreted in the global context. Based on its type, a Uri can
 * have the following components specified
 * <ul>
 * <li>Server identification</li>
 * <li>Workspace identification</li>
 * <li>Project/Project Container identification</li>
 * <li>Stream identification</li>
 * </ul>
 * String representation accepted and produced by the EspUri object conforms to the following syntax
 * <pre>esp[s]://h1:p1[[;h2:p2;h3:p3]/workspace/project/stream]</pre>
 * where
 * <ul>
 * <li><code>esp[s]</code> - is the protocol to use. Adding an 's' indicates SSL connection</li>
 * <li><code>h1:p1[[;h2:p2;h3:p3]</code> - host and port tuples denoting a list of one or more cluster managers.</li>
 * <li><code>workspace</code> - workspace name</li>
 * <li><code>project</code> - project name in the above workspace</li>
 * <li><code>stream</code> - stream name</li>
 * </ul>
 * 
 * <p>
 * Uris can be used to locate or access components from the global
 * SDK context. It is also possible to locate or access components from
 * their immediate parent in the ESP component hierarchy. Thus an
 * \ref EspProject can be retrieved from its parent \ref EspServer using
 * the project name or an \ref EspStream from an \ref EspProject using
 * the stream name (and possibly the stream id). The SDK ensures Uri consistency
 * between these two methods.
 * 
 * <h4>Connecting to standalone projects</h4>
 * 
 * Though available in the current version of the SDK, its use is strongly discouraged as this mechanism
 * is not officially supported and may be dropped in future versions.
 * <p>  
 * Connecting to standalone projects is not done using Uris but by explicitly specifying
 * the host and command control ports using esp_project_get_standalone().
 * <p>
 * Even though connection is not made using uris, to ensure consistency in usage of the SDK, project
 * objects for standalone instances do have an effective uri generated internally by the SDK.
 * The format for these uris is <code>esp://esp_virtual_server:0000/default/&lt;host&gt;:&lt;port&gt;</code>
 * where host and port are the original values specified by the user for the gateway host and gateway port. 
 */

/*!
 * Creates an EspUri object from a string. SDK parses the string and determines
 * the type of EspUri to create.  The new EspUri object should be freed using \ref esp_uri_free().
 * The string needs to conform to the following format
 * \verbatim esp[s]://host1:port1;host2:port2[/workspace/project/stream] \endverbatim
 * where
 * - \c hostN:portN are the list of known cluster managers
 * - \c workspace is the workspace being connected to
 * - \c project is the project in the above workspace
 * - \c stream is the stream in the project
 *
 * @return NULL in case of error
 */
ESPAPICALL EspUri * esp_uri_create_string(
        const char * const uri_string,          /*!< string in valid ESP uri format */
        EspError * error);

/*!
 * Create an empty uri object of type 'type'. The URI object should
 * be freed using esp_uri_free().
 *
 * @return an unitialized URI object or NULL in case of error.
 */
ESPAPICALL EspUri * esp_uri_create(
        ESP_URI_T type,             /*!< The type of uri to create.*/
        EspError * error);


/*!
 * Adds the connection details (host and port) for a manager. This call can be made multiple times
 * to add multiple managers. Typically this will be used to add manager details to an empty \ref EspUri
 * that was created using esp_uri_create(), but it can also be used to add more managers to an \ref EspUri
 * created using esp_uri_create_string().
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_add_manager(
        EspUri * uri,                   /*!< EspUri to modify */
        const char * manager_host,      /*!< host where manager is running */
        const int32_t manager_port,     /*!< port number for manager xmlrpc */
        EspError * error);

/*!
 * Adds the connection details for a manager as a preformatted string "[http[s]://]host:port"
 * If multiple manager URIs are added and one of them has an https protocol, all manager
 * connections are assumed to be https.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_add_manager_string(
        EspUri * uri,                   /*!< EspUri to modify */
        const char * manager_string,    /*!< Manager connection string using http/https protocol specification */
        EspError * error);

/*!
 * Sets all manager connections in this \ref EspUri to use or not use SSL.
 * Note that SSL can be turned on implicitly by specifying https protocol in the 
 * \ref esp_uri_add_manager_string() call. In that case, the last change made before the \ref EspUri
 * object is used in the SDK will be in effect.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_set_ssl(
        EspUri * uri,               /*!< EspUri to modify */
        const int32_t ssl,          /*!< non-zero to enable SSL */
        EspError * error);

/*!
 * Set the name of the server component. Will overwrite existing name if any.
 * A server specification string consists of semicolon delimited managers
 * specs (for example, host1:port1;host2:port2).
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_set_server(
        EspUri * uri,               /*!< EspUri to modify */
        const char * server,        /*!< Comma delimited server specs */
        EspError * error);

/*!
 * Set the name of workspace component. Will overwrite existing name if any.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_set_workspace(
        EspUri * uri,               /*!< EspUri to modify */
        const char * workspace,     /*!< Project name */
        EspError * error);

/*!
 * Set the name of project component. Will overwrite existing name if any.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_set_project(
        EspUri * uri,               /*!< EspUri to modify */
        const char * project,       /*!< Project name */
        EspError * error);

/*!
 * Set the name of stream component. Will overwrite existing name if any.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_set_stream(
        EspUri * uri,               /*!< EspUri to modify */
        const char * stream,        /*!< Stream name */
        EspError * error);

/*!
 * Frees the resources used by the EspUri object.
 */
ESPAPICALL int32_t esp_uri_free(
        EspUri * uri,               /*!< EspUri to free */
        EspError * error);


/*!
 * Query for the type of this EspUri.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_type(
        const EspUri * uri,     /*!< EspUri to query */
        ESP_URI_T * type,       /*!< Pointer to an ESP_URI_T enum that will contain the type on return */
        EspError * error);

/*!
 * Query the server name in the EspUri.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_server(
        const EspUri * uri,     /*!< EspUri to query */
        char * server,          /*!< Pointer to a char * that will be filled in successful on return */
        int32_t * size,         /*!< on entry size of the buffer being passed. On successful return size of the formatted string */
        EspError * error);

/*!
 * Query for the list of managers defined by this \ref EspUri.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_managers(
        const EspUri * uri,     /*!< EspUri to query */
        EspList * managers,     /*!< EspList that will be filled on return with the managers in the EspUri */
        EspError * error);

/*!
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_workspace(
        const EspUri * uri,     /*!< EspUri to query */
        char * workspace,       /*!< Buffer to contain the workspace name */
        int32_t * size,         /*!< on entry size of the buffer being passed. On successful return size of the formatted string */
        EspError * error);

/*!
 * Query for the project name defined in the EspUri.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_project(
        const EspUri * uri,     /*!< EspUri to query */
        char * project,         /*!< Buffer to contain the project name */
        int32_t * size,         /*!< on entry size of the buffer being passed. On successful return size of the formatted string */
        EspError * error);

/*!
 * Query for the project name defined in the EspUri.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_stream(
        const EspUri * uri,     /*!< EspUri to query */
        char * stream,          /*!< Buffer to contain the stream name */
        int32_t * size,         /*!< on entry size of the buffer being passed. On successful return size of the formatted string */
        EspError * error);

/*!
 * Returns the formatted string representation for the EspUri.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_uri_get_string(
        const EspUri * uri,     /*!< EspUri to format */
        char * uri_string,      /*!< buffer to contain the formatted string */
        int32_t * size,         /*!< on entry, the size of the buffer being passed. On successful return, the size of the formatted string */
        EspError * error);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_URI_H */
