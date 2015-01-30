/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_DISPATCHER_H
#define __ESP_DISPATCHER_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \addtogroup EspEventFramework
 * @{
 */

/*!
 * Creates a custom dispatcher.
 * @param name specifies a name to assign the dispatcher for logging purposes.
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
 * Starts an internal thread in the dispatcher.
 */
ESPAPICALL int32_t esp_dispatcher_start(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Dispatches any queued events for all entities registered with this dispatcher.
 * Returns when all pending events have been dispatched.
 */
ESPAPICALL int32_t esp_dispatcher_dispatch(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Stops the running internal thread in the dispatcher.
 */
ESPAPICALL int32_t esp_dispatcher_stop(
        EspDispatcher * dispatcher,
        EspError * error);

/*!
 * Deletes the dispatcher retrieved using esp_dispatcher_create().
 */
ESPAPICALL int32_t esp_dispatcher_free(
        EspDispatcher * dispatcher,
        EspError * error);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif  /* __ESP_DISPATCHER_H */
