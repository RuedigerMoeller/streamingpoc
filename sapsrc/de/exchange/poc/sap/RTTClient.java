package de.exchange.poc.sap;

import java.io.IOException;

import javax.security.auth.login.LoginException;

import com.sybase.esp.sdk.Credentials;
import com.sybase.esp.sdk.Project;
import com.sybase.esp.sdk.SDK;
import com.sybase.esp.sdk.Stream;
import com.sybase.esp.sdk.Stream.Operation;
import com.sybase.esp.sdk.Uri;
import com.sybase.esp.sdk.data.MessageWriter;
import com.sybase.esp.sdk.data.Publisher;
import com.sybase.esp.sdk.data.PublisherOptions;
import com.sybase.esp.sdk.data.RelativeRowWriter;
import com.sybase.esp.sdk.exception.EntityNotFoundException;
import com.sybase.esp.sdk.exception.RangeOverflowException;
import com.sybase.esp.sdk.exception.ServerErrorException;

/**
 * Created by moelrue on 12/15/14.
 */
public class RTTClient {

    private static final SDK s_sdk = SDK.getInstance();
    private static final int WAIT_TIME_60000_MS = 60000;

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

        PublisherOptions options = new PublisherOptions.Builder()
//                .setBlockingMode(PublisherOptions.BlockingMode.AUTOBLOCKING)
//                .setAccessMode(SDK.AccessMode.DIRECT)
//                .setBufferSize(10_000_000)
//                .setAsyncPublishing(true)
                .create();
        Publisher publisher = project.createPublisher();

        String streamName = "NEWSTREAM";

        // We will be inserting into the following schema
        // CREATE SCHEMA SdkExampleSchema(column1 INTEGER, column2 STRING);

        int totalRecs = 100_000;

        Stream stream = project.getStream(streamName);

        publisher.connect();

        MessageWriter message = publisher.getMessageWriter(stream);
        RelativeRowWriter row = message.getRelativeRowWriter();


        //
        // Publish data - one row at a time
        // Use startEnvelope() or startTransaction() for higher throughput
        //
        while( true ) {
            long start = System.currentTimeMillis();
            for (int rowNum = 1; rowNum <= totalRecs; rowNum++) {
                row.startRow();
                row.setOperation(Operation.INSERT);

                row.setLong(System.nanoTime());
                row.setInteger(rowNum);

                row.endRow();
                publisher.publish(message);
            }
            publisher.commit();
            long end = System.currentTimeMillis();
            System.out.println("--- done [records=" + totalRecs + ", milliseconds=" + (end - start) + ", throughput=" + totalRecs * 1000.0 / (end - start) + "]");
        }
    }

    /**
     * @param args
     * @throws IOException
     */
    public static void main(String[] args) throws IOException
    {
        RTTClient pe = new RTTClient();

        s_sdk.start();

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

}
