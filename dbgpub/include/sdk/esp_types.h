/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __ESP_TYPES_H
#define __ESP_TYPES_H

#ifdef _MSC_VER 
#  ifdef esp_sdk_lib_EXPORTS
#    define ESPAPICALL __declspec(dllexport)
#  else     /* esp_sdk_lib_EXPORT_ */
#    define ESPAPICALL __declspec(dllimport)
#  endif    /* esp_sdk_lib_EXPORT_ */
#else       /* _MSC_VER */
#  define ESPAPICALL
#endif      /* _MSC_VER */


#ifdef _MSC_VER
    /* WINDOWS */
    typedef signed char int8_t;
    typedef short int   int16_t;
    typedef int         int32_t;
    typedef __int64     int64_t;
#ifndef uint8_t
    typedef unsigned char uint8_t;
#endif
    typedef unsigned short int  uint16_t;
    typedef unsigned int        uint32_t;
    typedef unsigned long long  uint64_t;

#else
    /* UNIX */
#   include <stdint.h> 
#endif

#include <stddef.h>
#include <time.h>



/*
 * \if 0
 * \struct StringArray
 * API methods use this structure to return arrays of strings. The resources
 * used by the structure must be freed using the macro ESP_ARRAY_FREE afterwards.
 */
typedef struct {
    uint32_t size;
    char ** items;
} ESP_STRING_ARRAY;

#define ESP_ARRAY_FREE(A) \
    { int i;              \
      if ((A) && (A)->size)               \
        for (i = 0; i < (A)->size; ++i)   \
         if ((A)->items[i])               \
            free ((A)->items[i]);         \
         free ((A)->items);               \
    }

/*!
 * \addtogroup DataStructures
 * @{
 */

/*!
 * Enumeration of types supported by the EspList interface.
 */
typedef enum
{
    ESP_LIST_MANAGER_T,         /*!< List to return EspManager references */
    ESP_LIST_CONTROLLER_T,      /*!< List to return EspController references */
    ESP_LIST_WORKSPACE_T,       /*!< List to return EspWorkspace (string) references */
    ESP_LIST_APPLICATION_T,     /*!< List to return EspApplication references */
    ESP_LIST_STRING_T,          /*!< List to return strings */
    ESP_LIST_STREAM_T,          /*!< List to return EspStream references */
    ESP_LIST_EVENT_T,           /*!< List to return references to various events */
    ESP_LIST_UNDEFINED_T    
}
ESP_LIST_ITEM_T;

/*!
 * List like structure for atomically retrieving a collection of entities. The need for this arises
 * from the fact that a lot of information returned by the SDK is dynamic. Returning information in pieces
 * may break data consistency.
 * <p>
 * The SDK provides a number of functions to create and query lists. Using a list would consist of the following
 * steps
 * <ol>
 * <li>Create an EspList of the appropriate type</li>
 * <li>Clear the list if necessary</li>
 * <li>Get number of elements contained in the list</li>
 * <li>Iterate through all the elements</li>
 * <li>Once done, free the EspList</li>
 * </ol>
 * Here is some example code that retrieves a list of currently subscribed streams from an EspSubscriber
 * \code
 *
 * // create EspList that can contain EspStreams
 * EspList * list = esp_list_create(ESP_LIST_STREAM_T, error);
 *
 * // retrieve the list of streams currently subscribed to
 * int rc = esp_subscriber_get_subscribed_streams(subscriber, list, error);
 *
 * // figure out how many streams there are
 * int cnt = esp_list_get_count(list, error);
 *
 * // loop 
 * for (int i = 0; i < cnt; i++) }
 *     const EspStream * stream = esp_list_get_stream(list, i, error);
 *     // do something with the streams
 * }
 * // free list resources
 * esp_list_free(list, m_error);
 * \endcode
 */
typedef struct EspList EspList;

typedef struct EspMap EspMap;

typedef struct EspSdk EspSdk;

/*!
 * Structure to query for low level socket details on a connected EspPublisher
 */
typedef struct EspSocketDetails EspSocketDetails;

/*! @} */

/*!
 * \addtogroup EspEventFramework
 * @{
 */
/*!
 * Enumeration of access mode types supported by the SDK.
 */
typedef enum
{ 
    UNDEFINED_ACCESS = 0,   /*!< Undefined, typically indicates and error condition. */
    /*!
     * In direct access mode all calls to an entity are synchronous. This means that all commands and queries for dynamic properties
     * will result in the SDK issuing a call to the back end. It should be kept in mind that this can be costly. In direct access
     * mode no background threads are involved, all work occurs in the calling thread.
     */ 
    DIRECT_ACCESS = 1,
    /*!
     * In callback mode most (but not all) commands and property accesses are asynchronous. Typically there are
     * two threads involved in this mode, one thread to actually perform the command or make the query to the
     * backend process and another thread to dispatch the results or events. By default these two functions
     * are carried out by a default \ref EspUpdated and a default \ref EspDispatcher object that the
     * SDK maintains internally.
     */
    CALLBACK_ACCESS,
	/**
     * Multiplex multiple entities in a single thread. Selection mode access is similar to callback access but instead of an
     * \ref EspDispatcher, entities are registered with an \ref EspSelector. Commands and most property access are asynchronous. 
     * The \ref EspSelector does not have internal threads. It allows user code to block for events on all entities registered with
     * it. When an event occurs the thread is unblocked and the ready event set is returned.
     */
    SELECTION_ACCESS
}
ESP_ACCESS_MODE_T;

/*! @} */

/*!
 * \ingroup EspErrorGroup
 * Structure for detail information in case of errors
 */
typedef struct EspError EspError;

/*!
 * \ingroup EspCredentialsGroup
 * Structure to handle various authentication mechanisms supported by the SDK
 */
typedef struct EspCredentials EspCredentials;

/*!
 * \addtogroup EspUriGroup
 * @{
 */
/*!
 * Structure used to uniquely identify entities in ESP runtime
 */
typedef struct EspUri EspUri;

/*!
 * Enumeration of different URI types
 */
typedef enum
{
    UNDEFINED_URI = 0,
    SERVER_URI  = 1,        /*!< URI that refers to running a server/cluster */
    WORKSPACE_URI,          /*!< URI that refers to a workspace in a cluster */
    PROJECT_URI,            /*!< URI that refers to a project in a cluster */
    STREAM_URI              /*!< URI that refers to a stream in a project */
}
ESP_URI_T;

/*! @} */

/*!
 * \addtogroup EspEventFramework
 * @{
 */

/*!
 * Event category enumeration - denotes the type of the entity that generated the event. Events are
 * returned by the SDK as opaque objects. The category can be determined by using the esp_event_get_category()
 * call.
 */
typedef enum
{
    ESP_EVENT_UNKNOWN = -1,     /*!< Unknown event - usually an error indication */
    ESP_EVENT_SERVER = 0,       /*!< Event relating to \ref EspServer */
    ESP_EVENT_PROJECT,          /*!< Event relating to \ref EspProject */
    ESP_EVENT_PUBLISHER,        /*!< Event retating to \ref EspPublisher */
    ESP_EVENT_SUBSCRIBER        /*!< Event relating to \ref EspSubscriber */
}
ESP_EVENT_CATEGORY_T;

typedef struct EspDispatcher EspDispatcher;

typedef struct EspUpdater EspUpdater;

/*!
 * EspSelector allows users to register multiple entities and then wait until an 
 * event is triggered for any of the registered entities. The \ref EspSelector returned by this call should be freed
 * using esp_selector_free(). A typical usage pattern for selection would be
 * <ol>
 * <li>Create the entity in \ref SELECTION_ACCESS access mode using the appropriate option.</li>
 * <li>Create an \ref EspSelector using \ref esp_selector_create().</li>
 * <li>Register the entity with the selector by specifying the events to monitor. </li>
 * <li>Wait until an event occurs.</li>
 * <li>Retrieve and process the list of triggered events.</li>
 * </ol>
 * Refer to the SDK Guide for more details and examples of how selection mode can be used.
 * @see ESP_ACCESS_MODE_T
 * @see esp_server_select_with()
 * @see esp_project_select_with()
 * @see esp_publisher_select_with()
 * @see esp_subscriber_select_with()
 */
typedef struct EspSelector EspSelector;

/*! @} */


/*!
 * \addtogroup EspServerGroup
 * @{
 */
/*!
 * An EspServer instance represents a running ESP cluster. This can be used to query for server details. EspServer instance
 * is retrieved using the call esp_server_get()
 */
typedef struct EspServer EspServer;
/*!
 * \ingroup EspServerEventGroup
 * Represents an event that has occured on a \ref EspServer.
 */
typedef struct EspServerEvent EspServerEvent;
/*!
 * \ingroup EspServerOptionsGroup
 * This allows to set various \ref EspServer options to use when creating \ref EspServer instance. 
 */
typedef struct EspServerOptions EspServerOptions;
/*!
 * \ingroup EspServerEventGroup
 * Signature for \ref EspServer event handler function.
 */
typedef void (*SERVER_CALLBACK_T)(const EspServerEvent * event, void * user_data);

/*! @} */

/*!
 * \addtogroup EspProjectGroup
 * @{
 */
/*!
 * Main structure to handle project related functionality
 */
typedef struct EspProject EspProject;
/*!
 * Structure to handle an individual project's deployment
 */
typedef struct EspDeployment EspDeployment;
/*!
 * \ingroup EspProjectEventGroup
 * Structure to handle project lifecycle events
 */
typedef struct EspProjectEvent EspProjectEvent;
/*!
 * \ingroup EspProjectOptionsGroup
 * Structure to handle project related options
 */
typedef struct EspProjectOptions EspProjectOptions;
/*!
 * \ingroup EspProjectEventGroup
 * Signature for project event callback handler function
 */
typedef void (*PROJECT_CALLBACK_T)(const EspProjectEvent * event, void * user_data);

/*! @} */

/*!
 * \addtogroup EspStreamGroup
 * @{
 */
/*!
 * Structure to handle various stream related functionality
 */
typedef struct EspStream EspStream;
/*!
 * \ingroup EspSchemaGroup
 * Structure representing the structure of a stream
 */
typedef struct EspSchema EspSchema;
/*!
 * \ingroup DataTypeGroup
 * Structure to handle money data types supported by ESP
 */
typedef struct EspMoney EspMoney;

/*!
 * \ingroup DataTypeGroup
 * Structure to handle the time supported by ESP
 */
typedef struct EspTime EspTime;

/*!
 * \ingroup DataTypeGroup
 * Structure to handle the fixed decimal supported by ESP
 */
typedef struct EspFixedDecimal EspFixedDecimal;

/*!
 * \ingroup DataTypeGroup
 * Structure to handle the high precision big date time supported by ESP
 */
typedef struct EspBigDatetime EspBigDatetime;

/*! @} */

/*!
 * \addtogroup EspSubscriberGroup
 * @{
 */
/*!
 * The main object for subscribing to data from projects
 */
typedef struct EspSubscriber EspSubscriber;
/*!
 * \ingroup EspSubscriberOptionsGroup
 * Structure to set various subscription options
 */
typedef struct EspSubscriberOptions EspSubscriberOptions;
/*!
 * \ingroup EspSubscriberEventGroup
 * Structure for subscribtions lifecycle and states
 */
typedef struct EspSubscriberEvent EspSubscriberEvent;
/*!
 * \ingroup EspSubscriberEventGroup
 * Signature for subscriber event handler function
 */
typedef void (*SUBSCRIBER_CALLBACK_T)(const EspSubscriberEvent * event, void * user_data);
/*!
 * \ingroup EspMessageReaderGroup
 * Structure to handle messages from a particular stream
 */
typedef struct EspMessageReader EspMessageReader;
/*!
 * \ingroup EspMessageReaderGroup
 * Structure to decode a single data row
 */
typedef struct EspRowReader EspRowReader;

/*! @} */

/*!
 * \addtogroup EspPublisherGroup
 * @{
 */
/*!
 * The main entity used to publish data to the engine
 */
typedef struct EspPublisher EspPublisher;
/*!
 * \ingroup EspPublisherOptionsGroup
 * Object to support various publishing options
 */
typedef struct EspPublisherOptions EspPublisherOptions;
/*!
 * \ingroup EspPublisherEventGroup
 * Publisher event denoting lifecycle and states of an \ref EspPublisher
 */ 
typedef struct EspPublisherEvent EspPublisherEvent;
/*!
 * \ingroup EspPublisherEventGroup
 * Signature for publisher event callback handler function 
 */
typedef void (*PUBLISHER_CALLBACK_T)(const EspPublisherEvent * event, void * user_data);
/*!
 * \ingroup EspMessageWriterGroup
 * Object to format data for a particular stream
 */
typedef struct EspMessageWriter EspMessageWriter;
/*!
 * \ingroup EspMessageWriterGroup
 * EspRawMessageWriter provides a mechanism to publish data buffers formatted with data in ESP binary format.
 * Care must be taken to use these functions, as the SDK does not validate the buffer contents. In general
 * the CCL model being published to should be identical to the model that was used to format the data. 
 * If using buffered publishing, synchronous mode is not supported. So buffers must have the appropriate
 * flag.
 */
typedef struct EspRawMessageWriter EspRawMessageWriter;
/*!
 * \ingroup EspMessageWriterGroup
 * Object to format a single data row.
 */
typedef struct EspRelativeRowWriter EspRelativeRowWriter;

/*! @} */

/*!
 * \ingroup EspClockGroup
 * Structure to handle clock related functions
 */
typedef struct EspClock EspClock;


#endif /* __ESP_TYPES_H */

