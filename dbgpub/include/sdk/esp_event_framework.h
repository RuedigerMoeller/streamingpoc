/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_EVENT_FRAMEWORK_H
#define __ESP_EVENT_FRAMEWORK_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup EspEventFramework Event Framework
 * @{
 * For an introduction to the event framework supported in the SDK please refer to the "SDK Users Guide".
 * <p>
 * An event is the mechanism the SDK uses to notify users of anything of interest in CALLBACK_ACCESS or SELECTION_ACCESS access modes.
 * In CALLBACK_ACCESS access mode, registered events handlers receive an \ref EspEvent object 
 * category. Users need to determine the type of the event and take action accordingly. In SELECTION_ACCESS access mode,
 * events are returned from \ref esp_selector_select() call. The list of returned events can be from multiple
 * entities so users must determine the category of the events before processing them.
 * The life cycles of these event objects are managed by the SDK.
 *
 * <p>
 * NOTE: In the one case where an event is returned in DIRECT access mode, EspSubscriberEvent from \ref esp_subscriber_get_next_event()
 * users will need to free the resources held by that event using \ref esp_subscriber_event_free().
 */

/*!
 * Reserved for future use
 */
ESPAPICALL EspDispatcher * esp_dispatcher_create(
        const char * name,
        EspError * error);

/*!
 * Retrieves the SDK default dispatcher. This should never be freed by the user.
 */
ESPAPICALL EspDispatcher * esp_dispatcher_get_default(
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_dispatcher_start(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_dispatcher_dispatch(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_dispatcher_stop(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Reserved for future use
 */
ESPAPICALL int32_t esp_dispatcher_free(
        EspDispatcher * dispatcher,
        EspError * error);


/*!
 * Creates an \ref EspSelector object. The created object must be freed using \ref esp_selector_free()
 * after use
 * @return NULL on error
 */
ESPAPICALL EspSelector * esp_selector_create(
        const char * name,                      /*!< string to indentify the new \ref EspSelector */
        EspError * error);

/*!
 * This call blocks until one or more events of interest have triggered for any of the registered entities.
 * Since events for different types may be returned from the call depending on what entities were registered,
 * it may be necessary to distinguish the event categories. Here is a typical example of how this can be done
 * assuming entities of different types were registered. The SDK uses an EspList structure to return the events.
 * 
 * \code
 * // create EspList that can hold EspEvent objects. This will need to be
 * // freed later using esp_list_free()
 * EspList * list = esp_list_create(ESP_LIST_EVENT_T, error);
 *
 * ESP_EVENT_CATEGORY_T cat;
 * // this call blocks, until event(s) of interest have triggered
 * rc = esp_selector_select(m_selector, list, error);
 *
 * // the returned list may contain events of different categories. An event is returned
 * // as an opaque pointer, first determine the category of the event, then the type within that 
 * // category.
 * 
 * const void * ev;
 * EspServerEvent * srv_event;
 * EspProjectEvent * prj_event;
 * EspPublisherEvent * pub_event;
 * EspSubscriberEvent * sub_event;
 * 
 * // iterate over all events in the list
 * for (int i = 0; i < esp_list_get_count(list, error); i++)
 * {
 *     ev = esp_list_get_event(list, i, error);
 *     switch ( esp_event_get_category(ev, error)) {
 *         case ESP_EVENT_SERVER:
 *             // This is a Server event, cast to EspServerEvent 
 *             srv_event = (EspServerEvent*) ev;
 *             esp_server_event_get_type(srv_event, &type, error);
 *             switch (type) {
 *                 // process server event types
 *             ......
 *             }
 *             break;
 *         case ESP_EVENT_PROJECT:
 *             // This is a Project event, cast to EspProjectEvent 
 *             prj_event = (EspProjectEvent*) ev;
 *             esp_project_event_get_type(prj_event, &type, error);
 *             // process project events 
 *             ......
 *         case ESP_EVENT_PUBLISHER:
 *             // This is a Publisher event, cast to EspPublisherEvent 
 *             pub_event = (EspPublisherEvent*) ev;
 *             esp_publisher_event_get_type(pub_event, &type, error);
 *             // process publisher events 
 *             ......
 *         case ESP_EVENT_SUBSCRIBER:
 *             // This is a subscriber event, cast to EspSubscriberEvent 
 *             sub_event = (EspSubscriberEvent*) ev;
 *             esp_subscriber_event_get_type(sub_event, &type, error);
 *             // process subscriber events 
 *             ......
 *             break;    
            }
        }
    }

 * \endcode
 * 
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_selector_select(
        const EspSelector * selector,               /*!< EspSelector to monitor */
        EspList * list,                             /*!< EspList to return the events in */
        EspError * error);

/*!
 * Frees an \ref EspSelector object that was created using \ref esp_selector_create()
 * and releases resources held by it.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_selector_free(
        EspSelector * selector,
        EspError * error);


/*! @} */

#ifdef __cplusplus
}
#endif

#endif  /* __ESP_EVENT_FRAMEWORK_H */
