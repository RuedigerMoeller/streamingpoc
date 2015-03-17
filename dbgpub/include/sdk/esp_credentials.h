/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_CREDENTIALS_H
#define __ESP_CREDENTIALS_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspCredentialsGroup Authentication
 * @{
 * Methods and macros supporting authentication in the SDK
 */

typedef enum
{
    /*!
     * Authenticates using a user/password tuple. This is applicable for standalone projects configured
     * with PAM authentication, or clusters configured with user, ldap or kerberos authentication.
     * <p>
     * <i>Required parameters:</i>
     * <ul>
     * <li>user name - esp_credentials_set_user()</li>
     * <li>password - esp_credentials_set_password()</li>
     * </ul>
     */
    ESP_CREDENTIALS_USER_PASSWORD = 1,
    /*!
     * Authentication using digest and signature verification. The keys are retrieved from a keystore.
     * <p>
     * <i>Required parameters:</i>
     * <ul>
     * <li>alias - esp_credentials_set_user()</li>
     * <li>keystore - esp_credentials_set_keyfile()</li>
     * </ul>
     */
    ESP_CREDENTIALS_SERVER_RSA,
    ESP_CREDENTIALS_PROJECT_RSA,    /*!< Legacy authentication mechanism. */
    /*!
     * Authentication using Kerberos ticket. A Kerberos TGT should be obtained and stored in the
     * ticket cache of the user before the authentication attempt. The ticket will be retrieved
     * from the cache.
     * <p>
     * <i>Optional parameters:</i>
     * <ul>
     * <li>user name - esp_credentials_set_user()</li>
     * <li>service name - esp_credentials_set_service()</li>
     * </ul>
     */
	ESP_CREDENTIALS_KERBEROS
}
ESP_CREDENTIALS_T;


/*!
 * Creates an unitialized EspCredentials object. This will need to be freed using
 * esp_credentials_free()
 *
 * @return EspCredentials, NULL on error
 */
ESPAPICALL struct EspCredentials * esp_credentials_create(
        ESP_CREDENTIALS_T type,             /*!< Type of credentials to create */
        struct EspError * error);

/*!
 * Sets the user or alias of the authentication scheme if one is required
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_set_user(
        EspCredentials * credentials,
        const char * user,                  /*!< User name or alias */
        EspError * error);                  /*!< details in case of error */

/*!
 * Sets the password field for authentications that require it.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_set_password(
        EspCredentials * credentials,       /*!< An ESP_CREDENTIALS_USER_PASSWORD type EspCredentials object */
        const char * password,              /*!< password, shared secret, to set */
        EspError * error);                  /*!< details in case of error */

/*!
 * Set the name of the file that contains a PEM encoded unencrypted private key.
 * Currently only RSA keys are supported. The file is not read until authentication
 * takes place, at which time an error may be generated.
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_set_keyfile(
        EspCredentials * credentials,       /*!< An ESP_CREDENTIALS_SERVER_RSA type EspCredentials object */
        const char * keyfile,               /*!< name of the file containing the private key to use */
        EspError * error);                  /*!< details in case of error */


/*!
 * Sets the service principal name of kerberos authentication scheme
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_set_service(
        EspCredentials * credentials,
        const char * service,               /*!< Kerberos service principal name */
        EspError * error);                  /*!< details in case of error */


/*!
 * Whether to encrypt fields in the authentication message for non SSL
 * connections. For SSL connections this setting is ignored
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_encrypt_fields(
        EspCredentials * credentials,       /*!< EspCredentials object to set the value for */
        const int32_t encrypt,              /*!< nonzero field values should be encrypted */
        EspError * error);                  /*!< details in case of error */


/*!
 * Frees an EspCredentials object previously created with esp_credentials_create()
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_credentials_free(
        EspCredentials * credentials,       /*!< EspCredentials to free */
        EspError * error);


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_CREDENTIALS_H */
