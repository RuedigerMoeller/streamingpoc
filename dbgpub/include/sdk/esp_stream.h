/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_STREAM_H
#define __ESP_STREAM_H

#include <sdk/esp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup EspStreamGroup Streams
 * @{
 */

/*!
 * \defgroup DataTypeGroup Data Types
 * @{
 */
#define ESP_DATATYPE_INTEGER    1   /*!< 32-bit integer @hideinitializer */
#define ESP_DATATYPE_LONG       2   /*!< 64-bit integer @hideinitializer */
#define ESP_DATATYPE_FLOAT      3   /*!< 64-bit floating point @hideinitializer */
#define ESP_DATATYPE_DATE       4   /*!< unsigned integer representing seconds since the epoch (time_t) @hideinitializer */
#define ESP_DATATYPE_STRING     5   /*!< variable length string @hideinitializer */
#define ESP_DATATYPE_NULL       6   /*!< generic type for null values. Not used. Use is_null() to check @hideinitializer */
#define ESP_DATATYPE_MONEY      7   /*!< legacy money type whose precision depends on the setting in the project connect to @hideinitializer */
#define ESP_DATATYPE_TIMESTAMP  8   /*!< 64 bit signed # of milliseconds since unix epoch. @hideinitializer */
#define ESP_DATATYPE_BOOLEAN   10   /*!< boolean @hideinitializer */
#define ESP_DATATYPE_MONEY01   11   /*!< Money type with precision 1 (range -922,337,203,685,477,580.8 through +922,337,203,685,477,580.7) @hideinitializer */
#define ESP_DATATYPE_MONEY02   12   /*!< Money type with precision 2 (range -92,233,720,368,547,758.08 through +92,233,720,368,547,758.07) @hideinitializer */
#define ESP_DATATYPE_MONEY03   13   /*!< Money type with precision 3 (range -9,223,372,036,854,775.808 through +9,223,372,036,854,775.807) @hideinitializer */
#define ESP_DATATYPE_MONEY04   14   /*!< Money type with precision 4 (range -922,337,203,685,477.5808  through +922,337,203,685,477.5807 ) @hideinitializer */
#define ESP_DATATYPE_MONEY05   15   /*!< Money type with precision 5 (range -92,233,720,368,547.75808  through +92,233,720,368,547.75807 ) @hideinitializer */
#define ESP_DATATYPE_MONEY06   16   /*!< Money type with precision 6 (range -9,223,372,036,854.775808  through +9,223,372,036,854.775807 ) @hideinitializer */
#define ESP_DATATYPE_MONEY07   17   /*!< Money type with precision 7 (range -922,337,203,685.4775808   through +922,337,203,685.4775807  ) @hideinitializer */
#define ESP_DATATYPE_MONEY08   18   /*!< Money type with precision 8 (range -92,233,720,368.54775808   through +92,233,720,368.54775807  ) @hideinitializer */
#define ESP_DATATYPE_MONEY09   19   /*!< Money type with precision 9 (range -9,223,372,036.854775808   through +9,223,372,036.854775807  ) @hideinitializer */
#define ESP_DATATYPE_MONEY10   20   /*!< Money type with precision 10 (range -922,337,203.6854775808    through +922,337,203.6854775807   ) @hideinitializer */
#define ESP_DATATYPE_MONEY11   21   /*!< Money type with precision 11 (range -92,233,720.36854775808    through +92,233,720.36854775807   ) @hideinitializer */
#define ESP_DATATYPE_MONEY12   22   /*!< Money type with precision 12 (range -9,223,372.036854775808    through +9,223,372.036854775807   ) @hideinitializer */
#define ESP_DATATYPE_MONEY13   23   /*!< Money type with precision 13 (range -9,223.372036854775808     through +9,223.372036854775807    ) @hideinitializer */
#define ESP_DATATYPE_MONEY14   24   /*!< Money type with precision 14 (range -9,223.372036854775808     through +9,223.372036854775807    ) @hideinitializer */
#define ESP_DATATYPE_MONEY15   25   /*!< Money type with precision 15 (range -9,223.372036854775808     through +9,223.372036854775807    ) @hideinitializer */
#define ESP_DATATYPE_INTERVAL  26   /*!< 64-bit integer representing interval value in microseconds @hideinitializer */
#define ESP_DATATYPE_BIGDATETIME 27 /*!< 64 bit signed # of *micro*seconds since unix epoch. @hideinitializer */
#define ESP_DATATYPE_BINARY    28   /*!< raw buffer containing binary data @hideinitializer */
#define ESP_DATATYPE_TIME      29   /*!< 32 bit integer. Number of seconds since midnight. @hideinitializer */
#define ESP_DATATYPE_FIXED_DECIMAL 30   /*!< Fixed point decimal @hideinitializer */

#define ESP_DATATYPE_GET_DATATYPE(fullDataType) (fullDataType) & 0xff
/*! @} */

/*!
 * \defgroup StreamOpGroup Stream Operations
 * @{
 */
typedef enum
{
    ESP_STREAM_OP_NOOP =   0,       /*!< No-op. @hideinitializer */
    ESP_STREAM_OP_INSERT = 1,       /*!< Insert the row. Will fail if key already exists. @hideinitializer */
    ESP_STREAM_OP_UPDATE = 3,       /*!< Update a row. Will fail if key does not exist. @hideinitializer */
    ESP_STREAM_OP_DELETE = 5,       /*!< Delete a row. Will fail if key does not exist. @hideinitializer */
    ESP_STREAM_OP_UPSERT = 7,       /*!< Upsert a row - insert if key does not exist, update a row if it does. @hideinitializer */
    ESP_STREAM_OP_SAFEDELETE = 13   /*!< Delete only if row exists. @hideinitializer */
}
ESP_OPERATION_T;

/*! @} */


/*!
 * \defgroup StreamTypeGroup Stream Types
 * @{
 * Enumaration of different stream types. Please refer to the CCL Programmers Guide for details on various stream types
 * supported in ESP. 
 */
typedef enum
{
    SOURCE,                 /*!< Source streams in ASP, deprecated @hideinitializer */
    DERIVED,                /*!< Derived streams in ASP, deprecated @hideinitializer */
    ESP_STREAM_SQL_QUERY,   /*!< Virtual stream, corresponding to a SQL subscription @hideinitializer */
    ESP_STREAM_DELTA,       /*!< Stateless, no data retention, supports keys @hideinitializer */
    ESP_STREAM_SIMPLE,      /*!< Stateless streams with no data retention, but does not support keys @hideinitializer */
    ESP_STREAM_WINDOW,      /*!< Stateful, retains data based on retention policies @hideinitializer */
    ESP_STREAM_SYSTEM,      /*!< System streams used internally by ESP @hideinitializer */
    ESP_STREAM_ERROR,       /*!< Error stream, holds error events @hideinitializer */
    ESP_STREAM_LEGACY,      /*!< Legacy stream type (source/derived) @hideinitializer */
    ESP_STREAM_KEYED        /*!< Stateless streams with no data retention, but supports keys @hideinitializer */
}
ESP_STREAM_TYPE_T;

/*! @} */

/*!
 * \defgroup StreamVisGroup Stream Visibility
 * @{
 */

/*!
 * Enumeration of various stream visibilities. Please refer to the CCL programmers guide for details.
 */
typedef enum
{
    ESP_STREAM_INPUT,          /*!< Stream can be published to and subscribed from */
    ESP_STREAM_OUTPUT,         /*!< Stream can be subscribed from */
    ESP_STREAM_LOCAL           /*!< Stream cannot be published to or subscribed from */
}
ESP_STREAM_VISIBILITY_T;
/*! @} */


/*!
 * \defgroup EspSchemaGroup Schema
 * @{
 */

/*!
 * Get the number of columns for this schema.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_schema_get_numcolumns(
        const EspSchema * schema,           /*!< EspSchema to query */
        int32_t * numcolumns,               /*!< Pointer to  int32_t to return number of columns in */
        EspError * error);

/*!
 * Returns the column name for the specified position.
 *
 * @return column name, NULL on error.
 */
ESPAPICALL const char * esp_schema_get_column_name(
        const EspSchema * schema,           /*!< EspSchema to query */
        const int32_t column_no,            /*!< Column number to query */
        EspError * error);

/*!
 * Returns the data type of the column at the specified position.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_schema_get_column_type(
        const EspSchema * schema,           /*!< EspSchema to query */
        const int32_t column_no,            /*!< Column number to query */
        int32_t * type,                     /*!< Pointer to int32_t to return column type in */
        EspError * error);

/*!
 * Returns the data full type of the column at the specified position.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_schema_get_column_fulltype(
        const EspSchema * schema,           /*!< EspSchema to query */
        const int32_t column_no,            /*!< Column number to query */
        int32_t * type,                     /*!< Pointer to int32_t to return column type in */
        EspError * error);


/*!
 * Determine the data type for the specified column name.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_schema_get_column_type_by_name(
        const EspSchema * schema,           /*!< EspSchema to query */
        const char * column_name,           /*!< Column name to query */
        int32_t * type,                     /*!< Pointer to int32_t to return column type in */
        EspError * error);

/*!
 * Determine if the column at the specified position is a key column.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_schema_is_keycolumn(
        const EspSchema * schema,           /*!< EspSchema to query */
        const int32_t column_no,            /*!< Column number to query */
        int32_t * key,                      /*!< Pointer to int32_t in which to return the key indication. Non-zero indicates a key column. */
        EspError * error);


/*! @} */

/*!
 * Retrieve the type of the EspStream.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_stream_get_type(
        const EspStream * stream,                   /*!< EspStream to query */
        ESP_STREAM_TYPE_T * type,                   /*!< Pointer in which to return the type */
        EspError * error);

/*!
 * Retrieve the visibility of the EspStream.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_stream_get_visibility(
        const EspStream * stream,                   /*!< EspStream to query */
        ESP_STREAM_VISIBILITY_T * visibility,       /*!< Pointer in which to return the visibility */
        EspError * error);

/*!
 * Retrieve the unique id assigned to the EspStream.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_stream_get_id(
        const EspStream * stream,                   /*!< EspStream to query */
        int32_t * id,                               /*!< Pointer in which to return the id */
        EspError * error);

/*!
 * Retrieve the name for this EspStream.
 *
 * @return stream name, NULL on error.
 */
ESPAPICALL const char * esp_stream_get_name(
        const EspStream * stream,                   /*!< EspStream to query */
        EspError * error);

/*!
 * Retrieve the schema for the EspStream.
 *
 *. @return the schema, NULL on error
 */
ESPAPICALL const EspSchema * esp_stream_get_schema(
        const EspStream * stream,                   /*!< EspStream to query */
        EspError * error);

/*!
 * Determine if the stream is GD enabled
 *
 * @return 0  if stream is GD enabled
 */
ESPAPICALL int32_t esp_stream_is_gd_enabled(
        const EspStream * stream,           
        EspError * error);


/*!
 * Determine if the stream is GD enabled and Checkpointed 
 *
 * @return 0  if stream is GD enabled and checkpointed
 */
ESPAPICALL int32_t esp_stream_is_checkpoint_enabled(
        const EspStream * stream,           
        EspError * error);


/*!
 * \addtogroup DataTypeGroup
 * @{
 */


/*!
 * Converts the passed hexstring to its corresponding binary representation.
 * The size of the binary data buffer must be passed in the parameter buffsize.
 * If the buffer is large enough, converted binary data is copied to the supplied
 * buffer. If not, an error is returned. In both cases, buffsize will contain the size
 * of the binary data. 
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_data_binary_from_hexstring(
        const char * hexstring,     /*!< hexstring to convert */
        uint8_t * binary_data,      /*!< buffer to hold binary data */
        uint32_t * buffsize,        /*!< on input size of buffer, on output size of binary data */
        EspError * error);          /*!< error details */


/*!
 * Converts passed binary data to its corresponding hex string representation.
 * The size of the string buffer must be passed in the parameter buffsize.
 * If the buffer is large enough, the hex string is copied to the supplied
 * buffer. If not, an error is returned. In both cases, buffsize will contain the length 
 * of the hex string.
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_data_hexstring_from_binary(
        const uint8_t * binary_data,    /*!< binary data to convert */
        const uint32_t data_size,       /*!< length of the binary data */
        char * hexstring,               /*!< buffer to hold converted hex string */
        uint32_t * buffsize,            /*!< on input size of hex string buffer, on output size of the hex string */
        EspError * error);              /*!< error details */


/*!
 * Creates an uninitialized EspMoney object. The object must be freed
 * using esp_money_free().
 *
 * @return NULL on error.
 */
ESPAPICALL EspMoney * esp_money_create(
        EspError * error);

/*!
 * Creates an EspMoney object from the supplied string value. 
 * The function expects to find a decimal point in the string to
 * determine the precision. If it does not find one, it will generate an error.
 * The object must be freed using \ref esp_money_free().
 *
 * @return NULL on error.
 */
ESPAPICALL EspMoney * esp_money_create_string(
        const char * string_value,      /*!< string value with a decimal point */
        EspError * error);              /*!< error details */

/*!
 * Creates an EspMoney object from the supplied 64-bit integer value and the specified
 * precision. For example, a value of 72634947 and a precision of 4 will result in an
 * EspMoney object with the effective value 7263.4947.
 * The object must be freed using \ref esp_money_free().
 *
 * @return NULL on error.
 */
ESPAPICALL EspMoney * esp_money_create_long(
        const int64_t long_value,           /*!< Integral value for the money to create */
        const uint32_t precision,           /*!< Precision to use */
        EspError * error);

/*!
 * Creates a new EspMoney object with the specified precision. 
 * The returned EspMoney must be released using \ref esp_money_free().
 * Returns an error if the conversion would result in a loss of precision
 * or range.
 *
 * @return NULL on error.
 */
ESPAPICALL EspMoney * esp_money_as_precision(
        const EspMoney * money,         /*!< EspMoney to convert */
        const uint32_t precision,       /*!< precision to which to convert */
        EspError * error);              /*!< error details */

/*!
 * Free an EspMoney object created with \ref esp_money_create_long() or \ref esp_money_create_string().
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_money_free(
        EspMoney * money,               /*!< EspMoney object to free */
        EspError * error);

/*!
 * Retrieve the effective integral value in this EspMoney object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_money_get_long(
        const EspMoney * money,         /*!< EspMoney object to query */
        int64_t * long_value,           /*!< Pointer in which to return the integral value */
        EspError * error);

/*!
 * Retrieve the precision of this EspMoney object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_money_get_precision(
        const EspMoney * money,         /*!< EspMoney object to query */
        uint32_t * precision,           /*!< Pointer in which to return the precision */
        EspError * error);

/*!
 * Retrieve the effective factor in this EspMoney object.
 *
 * @return 0 on success.
 */
ESPAPICALL int32_t esp_money_get_factor(
        const EspMoney * money,         /*!< EspMoney object to query */
        uint64_t * factor,              /*!< Pointer in which to return the factor */
        EspError * error);

/*!
 * Create an uninitialized EspTime object. EspTime objects have a second
 * precision. The created object will need to be freed using \ref esp_time_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspTime * esp_time_create(
        EspError * error);

/*!
 * Create an EspTime object initialized from the supplied string. It expects
 * the string to be in the standard ESP time format which is 
 * "HH:mm:ss".
 *
 * The created object will need to be freed using \ref esp_time_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspTime * esp_time_create_string(
        const char * string_value,          /*!< string represenation in the format "HH:mm:ss" */
        EspError * error);

/*!
 * Create an EspTime object initialized with the passed value.
 * The value is interpreted as seconds since midnight.
 * The created object will need to be freed using \ref esp_time_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspTime * esp_time_create_seconds(
        const int64_t time_seconds,             /*!< seconds since midnight */
        EspError * error);

/*!
 * Frees an EspTime object previously created using one of the creation functions
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_time_free(
        EspTime * datetime,              /*!< EspTime object to free */
        EspError * error);

/*!
 * Returns the seconds in the passed EspTime object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_time_get_seconds(
        const EspTime * datetime,        /*!< EspTime to query */
        int64_t * time_seconds,                 /*!< pointer to int64_t to hold the value */
        EspError * error);
        
        
/*!
 * Create an uninitialized EspFixedDecimal object. EspFixedDecimal objects have a second
 * precision. The created object will need to be freed using \ref esp_fixeddecimal_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspFixedDecimal * esp_fixeddecimal_create(
        EspError * error);

/*!
 * Create an EspFixedDecimal object initialized from the supplied string. It expects
 * the string to be in the standard Esp FixedDecimal format which example is 
 * "1234567.89".
 *
 * The created object will need to be freed using \ref esp_fixeddecimal_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspFixedDecimal * esp_fixeddecimal_create_string(
        const char * string_value,          /*!< string represenation in the format "HH:mm:ss" */
        const int32_t thePrecision,          /*!< decimal precision */  
        const int32_t theScale,              /*!< decimal scale */       
        EspError * error);
        
/*!
 * Create an EspFixedDecimal object initialized from the pre-stored string. It expects
 * the string to be in the standard Esp FixedDecimal format which example is 
 * "1234567.89".
 *
 * The created object will need to be freed using \ref esp_fixeddecimal_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspFixedDecimal * esp_fixeddecimal_create_prestring(
        const char * string_value,          /*!< string represenation in the format "HH:mm:ss" */
        EspError * error);
        

/*!
 * Create an EspFixedDecimal  object initialized with the passed value.
 * The value is interpreted as bytes.
 * The created object will need to be freed using \ref esp_fixeddecimal_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspFixedDecimal * esp_fixeddecimal_create_decimal(
        const void * decimal_vale,             /*!< decimal value char[18] */
        const int32_t max_decimal_len,              /*!< maximum decimal value buffer length */
        const int32_t thePrecision,             /*!< decimal precision */ 
        const int32_t theScale,                 /*!< decimal scale */      
        EspError * error);

/*!
 * Frees an EspFixedDecimal object previously created using one of the creation functions
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_fixeddecimal_free(
        EspFixedDecimal * theFixedDecimal,          /*!< EspFixedDecimal object to free */
        EspError * error);
        
/*!
 * Using the pre-stored string to generate the decimal value
 *
 * @return NULL on error.
 */
ESPAPICALL int32_t esp_fixeddecimal_set_decimal_by_prestring(
        const EspFixedDecimal * theFixedDecimal,      /*!< EspFixedDecimal to query */
        const int32_t thePrecision,          /*!< decimal precision */  
        const int32_t theScale,              /*!< decimal scale */       
        EspError * error);
        

/*!
 * Returns the string value in the passed EspFixedDecimal object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_fixeddecimal_get_string(
        const EspFixedDecimal * theFixedDecimal,      /*!< EspFixedDecimal to query */
        char * string_val,                           /*!< pointer to decimal string buffer */
        const int32_t max_string_len,                /*!< pointer to maximum decimal string buffer length */
        EspError * error);
        
/*!
 * Returns the decimal bytes value in the passed EspFixedDecimal object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_fixeddecimal_get_decimal(
        const EspFixedDecimal * theFixedDecimal,      /*!< EspFixedDecimal to query */
        void * decimal_val,                           /*!< pointer to char[16] to hold the value */
        const int32_t max_decimal_len,               /*!< pointer to maximum decimal buffer length */
        EspError * error);


/*!
 * Returns the decimal precision value in the passed EspFixedDecimal object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_fixeddecimal_get_precision(
        const EspFixedDecimal * theFixedDecimal,      /*!< EspFixedDecimal to query */
        int32_t * thePrecisionP,                       /*!< pointer to int632_t to hold the precision value */
        EspError * error);

/*!
 * Returns the decimal scale value in the passed EspFixedDecimal object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_fixeddecimal_get_scale(
        const EspFixedDecimal * theFixedDecimal,      /*!< EspFixedDecimal to query */
        int32_t * theScaleP,                       /*!< pointer to int632_t to hold the scale value */
        EspError * error);

/*!
 * Create an uninitialized EspBigDatetime object. BigDatetime objects have a microsecond
 * precision. The created object will need to be freed using \ref esp_bigdatetime_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspBigDatetime * esp_bigdatetime_create(
        EspError * error);

/*!
 * Create an EspBigDatetime object initialized from the supplied string. It expects
 * the string to be in the standard ESP big datetime format which is 
 * "yyyy-MM-ddTHH:mm:ss.UUUUUU" where the letter 'U' stands for microseconds.
 * Note there is no explicit millisecond component in the format. So to specify a time value with
 * 123 milliseconds the string will need to be "2012-01-01T09:09:00.123000"
 *
 * The created object will need to be freed using \ref esp_bigdatetime_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspBigDatetime * esp_bigdatetime_create_string(
        const char * string_value,          /*!< string represenation in the format "yyyy-MM-ddTHH:mm:ss.UUUUUU" */
        EspError * error);

/*!
 * Create an EspBigDatetime object initialized with the passed value.
 * The value is interpreted as microseconds since Epoch.
 * The created object will need to be freed using \ref esp_bigdatetime_free()
 *
 * @return NULL on error.
 */
ESPAPICALL EspBigDatetime * esp_bigdatetime_create_microseconds(
        const int64_t microseconds,             /*!< microseconds since Epoch */
        EspError * error);

/*!
 * Frees an EspBigDatetime object previously created using one of the creation functions
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_bigdatetime_free(
        EspBigDatetime * datetime,              /*!< EspBigDatetime object to free */
        EspError * error);

/*!
 * Returns the microseconds in the passed EspBigDatetime object
 *
 * @return 0 on success
 */
ESPAPICALL int32_t esp_bigdatetime_get_microseconds(
        const EspBigDatetime * datetime,        /*!< EspBigDatetime to query */
        int64_t * microseconds,                 /*!< pointer to int64_t to hold the value */
        EspError * error);

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* __ESP_STREAM_H */
