/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_ERROR_H
#define __ESP_ERROR_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_ERROR_CODE_NULL_PARAMETER
#define ESP_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE
#define ESP_ERROR_CODE_ENTITY_ILLEGAL_STATE

/*!
 * \addtogroup EspErrorGroup
 * @{
 * Public methods in the SDK return int32_t or pointers. Most of the methods return
 * a 0 or non null value for success, non zero/NULL for an error. If the method accepts
 * an EspError parameter, detailed information about the error is returned in it. There are
 * a few methods that do not conform to this pattern. Those that do not are documented as such.
 * <p>
 * EspError is used to return error information from method calls. It provides
 * methods to retrieve error codes, error messages, sybsystem information and
 * nested error objects if any.
 */

/*!
 * Creates an error object that can be used to return extended
 * error information from the API calls. This must be freed using \ref esp_error_free().
 */
ESPAPICALL EspError * esp_error_create();

/*!
 * Release previously created error object.
 */
ESPAPICALL int32_t esp_error_free(
        EspError * error);

/*!
 * Returns a nested error if there is one, NULL otherwise.
 */
ESPAPICALL EspError * esp_error_get_nested_error(
        const EspError * error);

/*!
 * Get the code for this error.
 */
ESPAPICALL int esp_error_get_code(
        const EspError * error);

/*!
 * Get the message for this error. The message is valid for the lifetime of this error
 * object or until it has been reused.
 */
ESPAPICALL const char * esp_error_get_message(
        const EspError * error);

/*!
 * Get the system name that originated the error. The system is valid for the lifetime of this
 * error object or until it has been reused.
 */
ESPAPICALL const char * esp_error_get_system(
        const EspError * error);

/*!
 * Dumps the error contents to stdout.
 */
ESPAPICALL int32_t esp_error_screen_dump(
        const EspError * error);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_ERROR_H */
