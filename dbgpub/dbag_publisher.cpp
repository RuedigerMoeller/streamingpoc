//---------------------------------------------------------------------------
// dbag_publisher.cpp
//---------------------------------------------------------------------------
// Comment: This one is based on the DemoPublisher provided as example
// Copyright (c) 2012 SAP Inc. 
// All rights reserved.
// Use at your own risk. - 
//--------------------------------------------------------------------------
#include "stdio.h"
#include "stdlib.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>                                   
#include <string>
#include <vector>
#include <exception>
#include <string.h>
#include <utmp.h>
#include <pthread.h>

#include "sdk/esp_sdk.h"
#include "sdk/esp_error.h"
#include "sdk/esp_uri.h"
#include "sdk/esp_credentials.h"
#include "sdk/esp_project.h"
#include "sdk/esp_stream.h"
#include "sdk/esp_publisher.h"
#include "sdk/esp_types.h"
#include "sdk/esp_subscriber.h"

#define BUFFSIZE 1024

#include "dbag_publisher.hpp"

dbagPublisher opub;

Options g_options;

// ---------------------------------------------------------------------------
//  error & exit:
//     ESP simple error catch
// ---------------------------------------------------------------------------
void print_error_and_exit(EspError * espError, int line) {
    printf("An error was encountered on line %d\n", line);
    printf("%s", esp_error_get_message(espError));
    printf("\nExiting\n");
    exit(1);
}
//----------------------------------------------------------------------------
// init
// (re-) initializes values 
// negative numeric values indicate these are empty/invalid
//-----------------------------------------------------------------------------
void dbagPublisher::init()
{
    csrec.nanoTime = 0;                  //Key
}


// ---------------------------------------------------------------------------
//  loadData method:
//     Check the shut down flag
// Parse the record from the input file
//                                  
// Invoke the Publisher APIs to send the message 
// ---------------------------------------------------------------------------
void dbagPublisher::loadData(const char * url, const char * stream_name, Options& opts)
{
    std::vector<std::string> v;
    std::string tmp;
    char ThisField[300];
    struct tm * TStamp;
    time_t ThisTime;
    char * tok;
    char * ThisTag ;
    char * ThisValue;
    int count = 1;  //Create a variable to count the line number
    uint32_t currSeqNumber = 1; //Create a variable to set quote sequence number
    int rc = 0;
    char buff[BUFFSIZE];
    ThisField[0] = '\0';
    EspBigDatetime *dt;

    //--------- simply connect to ESP ------------------------------------------
    // this will be encapsulated in a separate method - for now - we stay with this mess
    //---------------------------------------------------------------------------------

    EspError * espError = esp_error_create();

    rc = esp_sdk_start(espError);
    if(rc != 0) { print_error_and_exit(espError, __LINE__); }

    EspUri * uri = esp_uri_create_string(url, espError);

    if(esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    EspCredentials * creds = esp_credentials_create(ESP_CREDENTIALS_USER_PASSWORD, espError);

    esp_credentials_set_user(creds, "sap", espError);
    esp_credentials_set_password(creds, "proofconcept", espError);

    if(esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    EspProject * project = esp_project_get(uri, creds, NULL, espError);
    if(esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    rc = esp_project_connect(project, espError);
    if(rc != 0) { print_error_and_exit(espError, __LINE__); }

    const EspStream * stream = esp_project_get_stream(project, stream_name, espError);
    if (esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    EspPublisher * publisher = NULL;
    if (opts.buffer_size) {
        EspPublisherOptions * popts = esp_publisher_options_create(espError);
        esp_publisher_options_set_buffersize(popts, opts.buffer_size, espError);
        publisher = esp_project_create_publisher(project, popts, espError);
        esp_publisher_options_free(popts, espError);
   
    }
    else
        publisher = esp_project_create_publisher(project, NULL, espError);


    if (esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    rc = esp_publisher_connect(publisher, espError);
    if(rc != 0) { print_error_and_exit(espError, __LINE__); }

    EspMessageWriter * message = esp_publisher_get_writer(publisher, stream, espError);
    if(esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }

    EspRelativeRowWriter * row = esp_message_writer_get_relative_rowwriter(message, espError);
    if(esp_error_get_code(espError) != 0) { print_error_and_exit(espError, __LINE__); }


    int numRows = 100000;
    int lcount = 20;

    bool inblock = false;

    timeval start;
    while ( lcount-- )
    {
        gettimeofday(&start, NULL );

        timespec ts;
        for ( int n = 0; n < numRows; n++ ) {

            // Batch if specified
            if (opts.block_size && !inblock) {
                rc = esp_message_writer_start_transaction(message, 0, espError);
                if (rc != 0) { print_error_and_exit(espError, __LINE__); }
                inblock = true;
            }

            csrec.bidPrc = 100.0;
            csrec.askPrc = 101.0;


            // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
            clock_gettime(CLOCK_REALTIME, &ts);
            csrec.nanoTime = ts.tv_nsec;

            //std::cout << "Prepare RowWriter "  <<  std::endl;
            rc = esp_relative_rowwriter_start_row(row, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

            rc = esp_relative_rowwriter_set_operation(row, ESP_STREAM_OP_INSERT, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

        //std::cout << "Prepare Data "  <<  std::endl;
            //----------------------------------------------------------------------------
            // prepare DATA
            //----------------------------------------------------------------------------
            //Key
            // c 1  tsid       i
            rc = esp_relative_rowwriter_set_long(row, csrec.nanoTime, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

        //std::cout << "write tsid "  <<  std::endl;
            // c2  domain_id   i
            rc = esp_relative_rowwriter_set_float(row, csrec.bidPrc, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

            rc = esp_relative_rowwriter_set_float(row, csrec.askPrc, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

            rc = esp_relative_rowwriter_set_integer(row, csrec.bidQty, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

            rc = esp_relative_rowwriter_set_integer(row, csrec.askQty, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }

            //----------------------------------------------------------------------------
            // end row
            //------------------------------------------------------------------------------
            rc = esp_relative_rowwriter_end_row(row, espError);
            if(rc != 0) { print_error_and_exit(espError, __LINE__); }
        // std::cout << "end row "  <<  std::endl;

            // Publish
//            if (opts.block_size) {
//                if ((csrec.rowNum++ % opts.block_size) == 0) {
//                    rc = esp_message_writer_end_block(message, espError);
//                    if (rc != 0) { print_error_and_exit(espError, __LINE__); }
//                    if (!opts.buffer_size) {
//                        rc = esp_publisher_publish(publisher, message, espError);
//                        if (rc != 0) { print_error_and_exit(espError, __LINE__); }
//                    }
//                    inblock = false;
//                }
//            }
//            else
            {
                if (!opts.buffer_size) {
                    rc = esp_publisher_publish(publisher, message, espError);
                    if(rc != 0) { print_error_and_exit(espError, __LINE__); }
                }
            }
        }

        // Commit only at the last loop
        if (!lcount) {
            rc = esp_publisher_commit(publisher, espError);
            if (rc != 0) { print_error_and_exit(espError, __LINE__); }
        }

        timeval endTime;
        gettimeofday(&endTime, NULL);
        long seconds  = endTime.tv_sec  - start.tv_sec;
        long useconds = endTime.tv_usec - start.tv_usec;

        long millitime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        long perSec = (numRows/millitime) * 1000;

        std::cout << "Time in milliseconds is " << millitime << " " << perSec << "\n";

    }


//   std::cout << "Publish commit "  <<  std::endl;

    rc = esp_sdk_stop(espError);
    if(rc != 0) { print_error_and_exit(espError, __LINE__); }

    printf("Messages published successfully\n");

}
// ---------------------------------------------------------------------------
//  help method:
//     Displays the command line parameters.
// ---------------------------------------------------------------------------
static void help(){
    std::cout << "*** Clickstream Publisher for ESP ***" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "dbag_pub <url> <stream> [block-size] [buffer-size]" << std::endl;
    /*
    std::cout << "dbag_pub [-?|-h| --help]" << std::endl;
    std::cout << "prints this message\n\n "<< std::endl;
    std::cout << "dbag_pub [-m datafile.csv]" << std::endl;
    std::cout << "Version 0.1"  << std::endl;
    std::cout << "(C)2012 SAP Inc All rights reserved\n"  << std::endl;
    */
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EspError        * g_error = NULL;
EspUri          * g_uri = NULL;
EspCredentials  * g_creds = NULL;


void *listen(void *threadid) {
    printf("running listener");
	int rc = 0;
	uint32_t buffsize = BUFFSIZE;
	char buff[BUFFSIZE];
	int32_t col1;

	uint32_t eventType;

	g_error = esp_error_create();

	rc = esp_sdk_start(g_error);
	if (rc != 0) { print_error_and_exit(g_error, __LINE__); }

	g_uri = esp_uri_create_string("esp://192.168.0.1:19011/dbag/rtt", g_error);
	if (NULL == g_uri) { print_error_and_exit(g_error, __LINE__); }

	g_creds = esp_credentials_create(ESP_CREDENTIALS_USER_PASSWORD, g_error);
	if (NULL == g_creds) { print_error_and_exit(g_error, __LINE__); }

    // Change these to match the installation
    esp_credentials_set_user(g_creds, "sap", g_error);
    esp_credentials_set_password(g_creds, "proofconcept", g_error);

	EspProject * project = esp_project_get(g_uri, g_creds, NULL, g_error);
	if (NULL == project) { print_error_and_exit(g_error, __LINE__); }

    // Once we have retrieved the project we do not need the EspCredentials and EspUri
    // objects since we are not going to reuse them. Need to free them
    esp_credentials_free(g_creds, g_error);
    g_creds = NULL;
    esp_uri_free(g_uri, g_error);
    g_uri = NULL;

	rc = esp_project_connect(project, g_error);
	if (rc != 0) { print_error_and_exit(g_error, __LINE__); }

	EspSubscriber * subscriber = esp_project_create_subscriber(project, NULL, g_error);
	if (NULL == subscriber) { print_error_and_exit(g_error, __LINE__); }

	const EspStream * stream = esp_project_get_stream(project, "SdkExampleInputWindow", g_error);
	if (NULL == stream) { print_error_and_exit(g_error, __LINE__); }

	rc = esp_subscriber_connect(subscriber, g_error);
	if (rc != 0) { print_error_and_exit(g_error, __LINE__); }

	rc = esp_subscriber_subscribe(subscriber, stream, g_error);
	if (rc != 0) { print_error_and_exit(g_error, __LINE__); }

	EspSubscriberEvent * event;
	EspMessageReader * reader;
	EspRowReader * row;
	int done = 0;

    timespec timestamp;
	long ts = 0;

    int count = 0;

    printf("listening");

	while (!done) {
		event = esp_subscriber_get_next_event(subscriber, g_error);
		if (esp_error_get_code(g_error) != 0) { print_error_and_exit(g_error, __LINE__); }

		rc = esp_subscriber_event_get_type(event, &eventType, g_error);
		if (rc != 0) {
            esp_subscriber_event_free(event, g_error);
            print_error_and_exit(g_error, __LINE__);
        }

		switch(eventType) {
			case ESP_SUBSCRIBER_EVENT_SYNC_START:
				printf("<sync-start/>\n");
				break;
			case ESP_SUBSCRIBER_EVENT_SYNC_END:
				printf("<sync-end/>\n");
				break;
			case ESP_SUBSCRIBER_EVENT_DATA:
				reader = esp_subscriber_event_get_reader(event, g_error);
				if (NULL == reader) { print_error_and_exit(g_error, __LINE__); }

                while ( row = esp_message_reader_next_row(reader, g_error) ) {
					rc = esp_row_reader_get_long(row, 0, &ts, g_error);
					if (rc != 0) { print_error_and_exit(g_error, __LINE__); }
                    // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
                    clock_gettime(CLOCK_REALTIME, &timestamp);
                    long duration = timestamp.tv_nsec - ts;

                    if ( (count++ % 100000) == 0)
                        printf("dur %ld "+duration);
				}
                if (esp_error_get_code(g_error) != 0) { print_error_and_exit(g_error, __LINE__); }
				break;

			case ESP_SUBSCRIBER_EVENT_CLOSED:
				printf("<closed/>\n");
				done = 1;
				break;
			case ESP_SUBSCRIBER_EVENT_DISCONNECTED:
				printf("<disconnected/>\n");
				done = 1;
				break;
            case ESP_SUBSCRIBER_EVENT_STREAM_EXIT:
				printf("<stream exit/>\n");
				done = 1;
                break;
			default:
				printf("<unhandled %08x/>\n", eventType);
				break;
		}
        // In DIRECT access mode, user is responsible for freeing the
        // event object
        esp_subscriber_event_free(event, g_error);
	}

	rc = esp_sdk_stop(g_error);
	if (rc != 0) { print_error_and_exit(g_error, __LINE__); }

    esp_error_free(g_error);
    pthread_exit(NULL);

}


// ---------------------------------------------------------------------------
//  main method:
//     CS Publisher for ESP  program entry point
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    printf("start program");

//    g_options.block_size = 0;
//    g_options.buffer_size = 1024;
//
//    pthread_t thr;
//    int rc = pthread_create(&thr, NULL, listen, NULL );
//    if (rc) {
//       printf("Error:unable to create thread," );
//       exit(-1);
//    }
//
//    printf("start publisher ..");
//    opub.loadData("esp://192.168.0.1:19011/dbag/rtt", 0 /*block size*/, g_options);

}

dbagPublisher::dbagPublisher() {
}
dbagPublisher::~dbagPublisher() {
}
  

