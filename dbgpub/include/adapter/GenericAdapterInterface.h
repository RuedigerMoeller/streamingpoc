/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

/*
 * GenericAdapterInterface.h
 *
 */

#ifndef GENERICADAPTERINTERFACE_H_
#define GENERICADAPTERINTERFACE_H_

#if !defined(DLLEXPORT)
#if defined(_WIN32)
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT
#define DLLIMPORT
#endif
#endif

#ifdef _WIN32
#include "win_stdint.hpp"
#else
#include <stdint.h>
#endif

#include <stdlib.h>

typedef void (*fp)(void);
typedef void* StreamRow;

// The levels for logging, match the ones in syslog.h
    enum LogLevel {
        L_EMERG = 0, /* system is unusable */
        L_ALERT,     /* action must be taken immediately */
        L_CRIT,      /* critical conditions */
        L_ERR,       /* error conditions */
        L_WARNING,   /* warning conditions */
        L_NOTICE,    /* normal but significant condition */
        L_INFO,      /* informational */
        L_DEBUG      /* debug-level messages */
    };

    // The connector states:
	// In the simple case they folow in the order they are defined,
	// except for IDLE - it's a little detour from CONTINUOUS.
	// In the more complicated cases there may be transitions from
	// IDLE and CONTINUOUS back to INITIAL.
	enum RunState {
		// if adding states, don't forget nameOfState() and describeState()
		RS_READY = 0, // ready to be started
		RS_INITIAL, // performing start-up and initial loading
		RS_CONTINUOUS, // continuously waiting for more data to come
		RS_IDLE, // on timeout or trying to restore a broken socket and such
		RS_DONE, // no more input will follow, the thread will exit soon
		RS_DEAD // exited, can't do anything useful any more, until requested to restart
	};



extern "C" {
//Import declarations for error handler utility functions.
	DLLIMPORT void* createConnectionErrors();
	DLLIMPORT bool empty(void* connectionErrors);
	DLLIMPORT size_t sizeConnectionErrors(void* connectionErrors);
	DLLIMPORT void clearConnectionErrors(void* connectionErrors);
	DLLIMPORT void addParam(void* connectionErrors, const char *param, const char *error);
	DLLIMPORT void addGeneral(void* connectionErrors, const char* error);
	DLLIMPORT void getAdapterError(void* connectionErrors, char** errorString);
//Import declarations for error handler utility functions.

//Import declarations for adapter parameter handler utility functions.
	DLLIMPORT void addConnectionParam(void* connectionParam, const char* key, const char* val);
	DLLIMPORT int getConnectionParamInt64_t(void* connectionParams, const char* key);
	DLLIMPORT const char* getConnectionParamString(void* connectionParams, const char* key);
	DLLIMPORT const char* substitute(void* connectionParam, const char *where, const char *paramPrefix="", int depth=0);
	DLLIMPORT int getConfiguredSections(void* connectionParams);
//Import declarations for adapter parameter handler utility functions.

//Import declarations for data conversion related utility functions.
	DLLIMPORT void* createConnectionRow(const char *type);
	DLLIMPORT void deleteConnectionRow(void* connectionRow);
	DLLIMPORT void clear(void* connectionRow);
	DLLIMPORT void clearData(void* connectionRow);
	DLLIMPORT void* createNew(void* connectionRow);
	DLLIMPORT void setDateFormat(void* connectionRow, const char *fmt);
	DLLIMPORT void setTimestampFormat(void* connectionRow, const char *fmt);
	DLLIMPORT void setStreamName(void* connectionRow, const char *sname);
	DLLIMPORT const char* getStreamName(void* connectionRow);
	DLLIMPORT void setStreamType(void* connectionRow, void *connectionRowType, bool own);
	DLLIMPORT void *getStreamType(void* connectionRow);
	DLLIMPORT bool getFieldAsStringWithKey(void *connectionRow, const char *fname, const char **val);
	DLLIMPORT bool setFieldAsStringWithKey(void *connectionRow, const char *fname, const char* val);
	DLLIMPORT bool getFieldAsStringWithIndex(void *connectionRow, int fn, const char **val);
	DLLIMPORT bool setFieldAsStringWithIndex(void* connectionRow, int fn, const char *val);
	DLLIMPORT bool appendFieldAsString(void* connectionRow, const char *val);
	DLLIMPORT char getOp(void* connectionRow );
	DLLIMPORT void setOp(void* connectionRow, char op);
	DLLIMPORT const char *getFlagsAsString(void* connectionRow);
	DLLIMPORT bool setFlagsAsString(void* connectionRow, const char * flags);
	DLLIMPORT int size(void* connectionRow);
	DLLIMPORT void setSeparator(void* connectionRow, char sep);
	DLLIMPORT void setWithService(void* connectionRow, bool val);
	DLLIMPORT char *allocBuffer(void* connectionRow, int size);
	DLLIMPORT void setBuffer(void* connectionRow, char *data);
	DLLIMPORT void setRow(void* connectionRow, void* streamRow, int len = 0);
	DLLIMPORT char *allocRow(void* connectionRow, int len);
	DLLIMPORT void adoptRow(void* connectionRow, void* streamRow);
	DLLIMPORT bool validate(void* connectionRow, void* connectionErrorMsgs);
	DLLIMPORT void* getRow(void* connectionRow);
	DLLIMPORT int getRowSize(void* connectionRow);
	DLLIMPORT void* toRow(void* connectionRow, size_t rowNo, void* connectionErrorMsgs);
	DLLIMPORT bool fromRow(void* connectionRow, void* streamRow, size_t rowNo, void* connectionErrorMsgs);
//Import declarations for data conversion related utility functions.

//Import declarations for call back related functions.
	 DLLIMPORT void postEndSync(void* callBackReference);
	 DLLIMPORT void logMessage(void* callBackReference,int level,const char* message);
	 DLLIMPORT void postStartSync(void* callBackReference);
	 DLLIMPORT void notifyConnState(void* callBackReference, int oldSt, int st);
	 DLLIMPORT int getColumnCount(void* connectionRowType);
	 DLLIMPORT const char* getColumnName(void* connectionRowType, int pos);
	 DLLIMPORT void* getTimeContext(void* callBackReference);
	 DLLIMPORT const char* getStreamName(void* connectionRowType);
	 DLLIMPORT void deactivateOutput(void* callBackReference);
	 DLLIMPORT void activateOutput(void* callBackReference, bool sendBase);
	 DLLIMPORT void setAdapterState(void* callBackReference, int state);
	 DLLIMPORT int getAdapterState(void* callBackReference);
	 DLLIMPORT int getColumnDatatype(void* callBackReference, int pos);
//Import declarations for call back related functions.

//Import declarations for custom adapter statistics functions.
	 DLLIMPORT void addAdapterStatistics(void* adapterStatistics, const char* key, const char* value);
//Import declarations for custom adapter statistics functions.

}
typedef struct AdapterStatistics AdapterStatistics;
extern "C" {
	DLLEXPORT void* getNext(void* adapter);
	DLLEXPORT void  putNext(void* adapter,void* stream);
	DLLEXPORT void  start(void* adapter);
	DLLEXPORT void  stop(void* adapter);
    DLLEXPORT void  stopRequested(void* adapter);
	DLLEXPORT bool  reset(void* adapter);
	DLLEXPORT bool  canDiscover(void* adapter);

	DLLEXPORT void cleanup(void* adapter);
	DLLEXPORT void commitTransaction(void* adapter);
	DLLEXPORT void putStartSync(void* adapter);
	DLLEXPORT void putEndSync(void* adapter);
	DLLEXPORT void purgePending(void* adapter);

	DLLEXPORT int64_t getNumberOfGoodRows(void* adapter);
	DLLEXPORT int64_t getNumberOfBadRows(void* adapter);
	DLLEXPORT int64_t getTotalRowsProcessed(void* adapter);
	DLLEXPORT bool isOutBase(void* adapter);

	DLLEXPORT void* createAdapter();
	DLLEXPORT void  setCallBackReference(void* adapter,void* connectionCallBackReference);
	DLLEXPORT void  setConnectionRowType(void* adapter,void* connectionRowType);
	DLLEXPORT void  setConnectionParams(void* adapter,void* connectionParams);
	DLLEXPORT void deleteAdapter(void* adapter);
	DLLEXPORT void getStatistics(void* adapter, AdapterStatistics* adapterStatistics);
	DLLEXPORT int64_t getLatency(void* adapter);
	DLLEXPORT bool hasError(void* adapter);
	DLLEXPORT void getError(void *adapter, char** errorString);
}

#endif /* GENERICADAPTERINTERFACE_H_ */
