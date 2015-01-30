
import java.io.IOException;

import javax.security.auth.login.LoginException;

import com.sybase.esp.sdk.Credentials;
import com.sybase.esp.sdk.Project;
import com.sybase.esp.sdk.SDK;
import com.sybase.esp.sdk.Stream;
import com.sybase.esp.sdk.Stream.Operation;
import com.sybase.esp.sdk.Uri;
import com.sybase.esp.sdk.Uri.UriType;
import com.sybase.esp.sdk.data.MessageWriter;
import com.sybase.esp.sdk.data.Publisher;
import com.sybase.esp.sdk.data.PublisherOptions;
import com.sybase.esp.sdk.data.RelativeRowWriter;
import com.sybase.esp.sdk.exception.EntityNotFoundException;
import com.sybase.esp.sdk.exception.ServerErrorException;


public class DbagPublisher 
{
	private static final SDK s_sdk = SDK.getInstance();
	
	/**
	 * args : url stream [block-size] [buffer-size]
	 * @throws IOException 
	 * @throws ServerErrorException 
	 * @throws EntityNotFoundException 
	 * @throws LoginException 
	 */
	void publish(String[] args) throws IOException, LoginException, EntityNotFoundException, ServerErrorException
	{
		Project project = null;
		Publisher publisher = null;
		
		Uri uri = new Uri.Builder(args[0]).create();
		if (uri.getType() != UriType.PROJECT)
			throw new IllegalArgumentException("need project uri");
		
		Credentials creds = new Credentials.Builder(Credentials.Type.USER_PASSWORD)
								.setUser("sybase")
								.setPassword("sybase")
								.create();
		
		project = s_sdk.getProject(uri, creds);
		
		project.connect(60000);

		int blockSize = 5;
		if (args.length > 2)
			blockSize = Integer.parseInt(args[2]);
		
		int bufferSize = 0;
		if (args.length > 3)
			bufferSize = Integer.parseInt(args[3]);
		
		if (bufferSize > 0) {
			PublisherOptions popts = new PublisherOptions.Builder().setBufferSize(bufferSize).create();
			publisher = project.createPublisher(popts);
		}
		else
			publisher = project.createPublisher();
		
		Stream stream = project.getStream(args[1]);
		
		publisher.connect();
		
		MessageWriter message = publisher.getMessageWriterCached(stream);
		RelativeRowWriter row = message.getRelativeRowWriter();

		// Data variable we will be publishing
		int int_value = 0;
		long long_value = 0;
        boolean inblock = false;

        int lcount = 20;
        int numRows = 100000;
        
        while (lcount-- != 0) {
        	
        	long start = System.currentTimeMillis();
        	
        	for (int i = 0; i < numRows; i++) {
                if ((blockSize > 0) && !inblock) {
                    message.startTransaction(0);
                    inblock = true;
                }
    			row.startRow();
    			row.setOperation(Operation.INSERT);
				row.setLong(long_value++);
				row.setInteger(int_value++);
    			
    			row.endRow();
    			
    			if (blockSize > 0) {
    				if (inblock && (i % blockSize) == 0) {
    					message.endBlock();
    					if (bufferSize == 0)
    						publisher.publish(message, false);
    					inblock = false;
    				}
    			} else {
    				if (bufferSize == 0)
    					publisher.publish(message, false);
    			}
    		}
        	if (lcount == 0)
        		publisher.commit();
        	
        	long millis = System.currentTimeMillis() - start;
        	System.out.println("Time in milliseconds is " + millis + " " + (numRows / millis) * 1000);
        	
        }

		s_sdk.stop();
	}

	
	/**
	 * @param args
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException 
	{
		if (args.length < 2) {
			System.out.println("Usage:");
			System.out.println("java DbagPuublisher <url> <stream> [block-size] [buffer-size]");
			System.exit(0);
		}

		DbagPublisher dp = new DbagPublisher();

		s_sdk.start();

		try {
			dp.publish(args);
		} catch (LoginException e) {
			e.printStackTrace();
		} catch (EntityNotFoundException e) {
			e.printStackTrace();
		} catch (ServerErrorException e) {
			e.printStackTrace();
		}
		finally {
			s_sdk.stop();
		}
	}
	
	
}
