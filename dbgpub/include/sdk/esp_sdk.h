/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_SDK_H
#define __ESP_SDK_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspSdk General
 * @{
 */

/*!
 * \defgroup DataStructures Data Structures
 * The EspList and EspMap data structures are used to retrieve multiple
 * pieces of information from the SDK atomically. For example, retrieving the number of
 * streams defined in a project.
 *
 * @{
 */

/*!
 * Creates an EspList data structure. This must be freed using \ref esp_list_free
 * once done.
 *
 * @return NULL on error
 */
ESPAPICALL EspList * esp_list_create(
        const ESP_LIST_ITEM_T type,     /*!< The type of data this EspList will contain */
        EspError * error);

/*!
 * Returns number of items held in the EspList.
 * @return -1 on error
 */
ESPAPICALL int32_t esp_list_get_count(
        const EspList * list,
        EspError * error);

/*!
 * Clears all items in the list. The EspList can be re used in a fresh query.
 * @return -1 on error
 */
ESPAPICALL int32_t esp_list_clear(
        EspList * list,
        EspError * error);

/*!
 * Frees an EspList object previously retrieved using \ref esp_list_create.
 * @return 0 on success
 */
ESPAPICALL int32_t esp_list_free(
        EspList * list,
        EspError * error);

/*!
 * Return the EspStream object as the specified index.
 * Will generate an error if the list is not of the correct type.
 * @return NULL in case of error.
 */
ESPAPICALL const EspStream * esp_list_get_stream(
        const EspList * list,       /*!< EspList from which to retrieve */
        const int32_t index,        /*!< Index at which to retrieve the item */
        EspError * error);

/*!
 * Return a string at the specified index in the list.
 * Will generate an error if the list is not of the correct type.
 * @return NULL in case of error.
 */
ESPAPICALL const char * esp_list_get_string(
        const EspList * list,       /*!< EspList from which to retrieve */
        const int32_t index,        /*!< Index at which to retrieve the item */
        EspError * error);


/*!
 * Returns a reference to one of the supported event types. The returned void pointer
 * needs to be cast to the appropriate category. Use esp_event_get_category() to
 * determine the category of the event.
 * @return NULL in case of error.
 */
ESPAPICALL const void * esp_list_get_event(
        const EspList * list,       /*!< EspList from which to retrieve */
        const int32_t index,        /*!< Index at which to retrieve the item */
        EspError * error);


/*!
 * Creates an EspMap structure that can be used to store key value pairs. The new EspMap
 * must be freed using \ref esp_map_free once done.
 * @return NULL in case of error.
 */
ESPAPICALL EspMap * esp_map_create(
        EspError * error);

/*!
 * Insert a key value pair into the map.
 * @return non-zero in case of error.
 */
ESPAPICALL int32_t esp_map_set(
        EspMap * map,               /*!< EspMap into which to insert */
        const char * key,           /*!< Key of the mapping */
        const char * value,         /*!< Value for the mapping */
        EspError * error);

/*!
 * Retrieve the value for the specified key.
 * @return NULL in case of error.
 */
ESPAPICALL const char * esp_map_get(
        const EspMap * map,
        const char * key,
        EspError * error);

/*!
 * Return number of items stored in the map.
 * This function does not correspond to the standard return values scheme used in the SDK.
 * @return -1 in case of error.
 */
ESPAPICALL int32_t esp_map_count(
        const EspMap * map,
        EspError * error);

/*!
 */
ESPAPICALL int32_t esp_map_item_at(
        const EspMap * map,
        const int32_t index,
        const char ** key,
        const char ** value,
        EspError * error);

/*!
 */
ESPAPICALL int32_t esp_map_free(
        EspMap * map,
        EspError * error);

/*!
 * Create an empty EspSocketDetails structure that can
 * be used to retrieve socket level details of a connected
 * EspPublisher.
 *
 * @return NULL on error.
 */
ESPAPICALL EspSocketDetails * esp_socket_details_create(
        EspError * error);


/*!
 * Free EspSocketDetails that has previously been created using
 * \ref esp_socket_details_create()
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_socket_details_free(
        EspSocketDetails * socket_details,
        EspError * error);

/*!
 * Retrieve the maximum tcp segment size that can be sent
 * to peer without fragmentation. This call is valid only on
 * Unix systems. On windows it will return an error.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_socket_details_get_mss(
        const EspSocketDetails * sock_details,
        int32_t * mss,
        EspError * error);

/*!
 * What is the send buffer size of the underlying socket
 * @return 0 on success
 */
ESPAPICALL int32_t esp_socket_details_get_send_buffer_size(
        const EspSocketDetails * sock_details,
        int32_t * send_buffer_size,
        EspError * error);

/*!
 * What is the receive buffer size of the underlying socket
 * @return 0 on success
 */
ESPAPICALL int32_t esp_socket_details_get_recv_buffer_size(
        const EspSocketDetails * sock_details,
        int32_t * recv_buffer_size,
        EspError * error);



/*! @} */

/*!
 * Returns the category of the event this void * represents.
 * void * pointer must have been retrieved using \ref esp_list_get_event
 * method. This allows code to cast the void * to the appropriate event
 * object such as EspServerEvent, EspProjectEvent, EspPublisherEvent or
 * EspSubscriberEvent.
 * @return \ref ESP_EVENT_CATEGORY_T enum. In case of error returns ESP_EVENT_UNKNOWN.
 */
ESPAPICALL int32_t esp_event_get_category(
        const void * event,
        EspError * error);

/*!
 * \defgroup EspSdk SDK Methods
 * @{
 */

/*!
 * Initializes the SDK. Must be the first call made to use the API.
 * Multiple start calls may be made, but they must be matched by the same
 * number of stop calls.
 */
ESPAPICALL int32_t esp_sdk_start(
        EspError * error);

/*!
 * Shuts down the SDK. On the stop call matching the first start, closes all open 
 * entities, stops internal threads if any and releases any held resources.
 */
ESPAPICALL int32_t esp_sdk_stop(
        EspError * error);

/*!
 * Turns on internal tracing for debugging.
 */
ESPAPICALL int32_t esp_trace_on(
        EspError * error);

/*!
 * Turns off internal tracing.
 */
ESPAPICALL int32_t esp_trace_off(
        EspError * error);

/*! @} */

/*!
 * \defgroup EspErrorGroup Error Handling 
 * @{
 */


/*! @} */


/*! @} */

#ifdef __cplusplus
}
#endif

#endif  /* __ESP_SDK_H */
