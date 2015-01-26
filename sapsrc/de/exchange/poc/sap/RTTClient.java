package de.exchange.poc.sap;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.LockSupport;

import javax.security.auth.login.LoginException;

import com.sybase.esp.sdk.Credentials;
import com.sybase.esp.sdk.Project;
import com.sybase.esp.sdk.SDK;
import com.sybase.esp.sdk.Stream;
import com.sybase.esp.sdk.Stream.Operation;
import com.sybase.esp.sdk.Uri;
import com.sybase.esp.sdk.data.*;
import com.sybase.esp.sdk.exception.EntityNotFoundException;
import com.sybase.esp.sdk.exception.RangeOverflowException;
import com.sybase.esp.sdk.exception.ServerErrorException;
import org.HdrHistogram.Histogram;
import org.nustaq.fastcast.util.Sleeper;

/**
 * Created by moelrue on 12/15/14.
 */
public class RTTClient {

    private static final SDK s_sdk = SDK.getInstance();
    private static final int WAIT_TIME_60000_MS = 60000;


//    MarketEvent structure
//    long sendTimeStampNanos;
//
//    double bidPrc,askPrc;
//    int bidQty, askQty;


    /**
     *
     * @throws IOException
     * @throws EntityNotFoundException
     * @throws LoginException
     * @throws RangeOverflowException
     * @throws ServerErrorException
     */
    void doPublish() throws IOException, EntityNotFoundException, LoginException, ServerErrorException
    {
        Uri uri = new Uri.Builder("esp://localhost:19011/dbag/rtt").create();

        // Change credentials to match installation
        Credentials creds = new Credentials.Builder(Credentials.Type.USER_PASSWORD).setUser("sap").setPassword("proofconcept").create();

        Project project = s_sdk.getProject(uri, creds);

        project.connect(WAIT_TIME_60000_MS);

        int blockSize = 0;
        int bufferSize = 1_000;

        PublisherOptions options = new PublisherOptions.Builder()
                .setBufferSize(bufferSize)
                .create();
        Publisher publisher = project.createPublisher(options);

        String streamName = "MarketData";

        // We will be inserting into the following schema
        // CREATE SCHEMA SdkExampleSchema(column1 INTEGER, column2 STRING);

        int loopRecs = 100_000;

        Stream stream = project.getStream(streamName);

        publisher.connect();

        MessageWriter message = publisher.getMessageWriter(stream);
        RelativeRowWriter row = message.getRelativeRowWriter();

        boolean inblock = false;

        //
        // Publish data - one row at a time
        // Use startEnvelope() or startTransaction() for higher throughput
        //
        int count = 400;
        while( count-- > 0 ) {
            long start = System.currentTimeMillis();
            for (int i = 1; i <= loopRecs; i++) {

                Sleeper.spinMicros(90);
//                LockSupport.parkNanos(7000);

                if ((blockSize > 0) && !inblock) {
                    message.startTransaction(0);
                    inblock = true;
                }

                row.startRow();
                row.setOperation(Operation.INSERT);
                row.setLong(System.nanoTime());
                row.setDouble(3.124);
                row.setDouble(3.555);
                row.setInteger(100);
                row.setInteger(200);
                row.endRow();

                if (blockSize > 0) {
                    if (inblock && (i % blockSize) == 0) {
                        message.endBlock();
                        if (bufferSize == 0)
                            publisher.publish(message);//, false);
                        inblock = false;
                    }
                } else {
                    if (bufferSize == 0)
                        publisher.publish(message);//, false);
                }
            }
            long end = System.currentTimeMillis();
            System.out.println("--- done [records=" + loopRecs + ", milliseconds=" + (end - start) + ", throughput=" + loopRecs * 1000.0 / (end - start) + "]");
        }
        long start = System.currentTimeMillis();
        publisher.commit();
        long end = System.currentTimeMillis();
        System.out.println("commit duration: milliseconds=" + (end - start));// + ", throughput=" + loopRecs * 1000.0 / (end - start) + "]");
    }

    /**
     * @param args
     * @throws IOException
     */
    public static void main(String[] args) throws IOException
    {
        RTTClient pe = new RTTClient();
        s_sdk.start();

        new Thread("receiver") {
            public void run() {
                try {
                    pe.doSubcribe();
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (EntityNotFoundException e) {
                    e.printStackTrace();
                } catch (LoginException e) {
                    e.printStackTrace();
                } catch (ServerErrorException e) {
                    e.printStackTrace();
                }
            }
        }.start();

        try {
            pe.doPublish();
        } catch (IOException e) {
            System.err.println("!-- " + e.getLocalizedMessage() + "\n");
            e.printStackTrace();
        } catch (EntityNotFoundException e) {
            System.err.println("!-- " + e.getLocalizedMessage() + "\n");
            e.printStackTrace();
        } catch (LoginException e) {
            System.err.println("!-- " + e.getLocalizedMessage() + "\n");
            e.printStackTrace();
        } catch (ServerErrorException e) {
            System.err.println("!-- " + e.getLocalizedMessage() + "\n");
            e.printStackTrace();
        } catch (RuntimeException e) {
            System.err.println("!-- " + e.getLocalizedMessage() + "\n");
            e.printStackTrace();
        } finally {
            s_sdk.stop();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //  subscriber
    //

    Histogram hi = new Histogram(TimeUnit.SECONDS.toNanos(10),3);

    void doSubcribe() throws IOException, EntityNotFoundException, LoginException, ServerErrorException
    {
        Uri uri = new Uri.Builder("esp://localhost:19011/dbag/rtt").create();

        // Change credentials to match installation
        Credentials creds = new Credentials.Builder(Credentials.Type.USER_PASSWORD).setUser("sap").setPassword("proofconcept").create();

        Project project = s_sdk.getProject(uri, creds);
        project.connect(WAIT_TIME_60000_MS);

        Subscriber subscriber = project.createSubscriber();

        subscriber.subscribeStream("MarketData");
        subscriber.connect();

        SubscriberEvent event = null;
        boolean done = false;
        int histoCounter = 0;

        while (!done) {
            event = subscriber.getNextEvent();
            switch (event.getType()) {
                case SYNC_START:
                    System.out.println("<sync-start/>");
                    break;
                case SYNC_END:
                    System.out.println("<sync-end/>");
                    break;
                case DATA:
                    MessageReader reader = event.getMessageReader();
                    String str = reader.getStream().getName();
                    while(reader.hasNextRow()) {
                        RowReader row = reader.nextRowReader();
                        long time = row.getLong(0);
                        hi.recordValue(System.nanoTime()-time);
                        histoCounter++;
                        if ( histoCounter == 200_000) {
                            hi.outputPercentileDistribution(System.out,1000.0);
                            hi =  new Histogram(TimeUnit.SECONDS.toNanos(10),3);
                            histoCounter = 0;
                        }
                    }
                    break;
                case CLOSED:
//                    System.out.println("<closed/>");
                    done = true;
                    break;
                case ERROR:
                    System.out.println("<error msg=\"" + event.getError() + "\" />");
                    break;
                case DATA_LOST:
                    System.out.println("<data-lost/>");
                    break;
                case DISCONNECTED:
                    System.out.println("<disconnected/>");
                    done = true;
                    break;
            }
        }
    }


}
