package de.exchange.poc.netfeed;

import com.espertech.esper.client.*;
import de.exchange.poc.MarketDataEvent;
import org.HdrHistogram.Histogram;
import org.nustaq.fastcast.api.FCPublisher;
import org.nustaq.fastcast.api.FCSubscriber;
import org.nustaq.fastcast.api.FastCast;
import org.nustaq.fastcast.api.util.ObjectPublisher;
import org.nustaq.fastcast.api.util.ObjectSubscriber;
import org.nustaq.fastcast.util.RateMeasure;
import org.nustaq.offheap.bytez.Bytez;
import org.nustaq.offheap.structs.FSTStruct;
import org.nustaq.offheap.structs.unsafeimpl.FSTStructFactory;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReferenceArray;

/**
 * Created by moelrue on 1/20/15.
 */
public class EventReceiver {

    FastCast fastCast;
    Histogram hi = new Histogram(TimeUnit.SECONDS.toNanos(10),3);
    private FCPublisher backPub; // pong for rtt measurement

    StupidPool pool = new StupidPool(2048);
    Executor bounceBackExec = Executors.newSingleThreadExecutor();


    private EPServiceProvider epService;
    public static class RateStatement
    {
        private EPStatement statement;

        public RateStatement(EPAdministrator admin)
        {
            String stmt = "insert into TicksPerSecond " +
                    "select count(*) as cnt from MarketDataEvent.win:time_batch(1 sec) group by symbol";

            statement = admin.createEPL(stmt);
        }

        public void addListener(UpdateListener listener)
        {
            statement.addListener(listener);
        }
    }

    public void initEsper() {
        // Configure engine with event names to make the statements more readable.
        // This could also be done in a configuration file.
        Configuration configuration = new Configuration();
        configuration.addEventType("MarketEventStruct", MarketEventStruct.class.getName());

        // Get engine instance
        epService = EPServiceProviderManager.getProvider("Marketfeed", configuration);


    }

    public void initFastCast() throws Exception {
        fastCast =  FastCast.getFastCast();
        fastCast.setNodeId("SUB");
        fastCast.loadConfig("fc.kson");

        backPub = fastCast.onTransport("back").publish("back");

        RateMeasure measure = new RateMeasure("receive rate");
        fastCast.onTransport("default").subscribe("stream",
            new FCSubscriber() {
                @Override
                public void messageReceived(String sender, long sequence, Bytez b, long off, final int len) {
                    measure.count();
                    final byte copy[] = pool.getBA();
                    if ( len < copy.length ) // prevent segfault :) !
                    {
//                        b.getArr(off,copy,0,len);
                        final MarketEventStruct msg = (MarketEventStruct) FSTStructFactory.getInstance().createStructWrapper(b, off).createCopy();
                        // do bounce back in different thread, else blocking on send will pressure back to
                        // sender resulting in whacky behaviour+throughput
                        bounceBackExec.execute(new Runnable() {
                            @Override
                            public void run() {
                                epService.getEPRuntime().sendEvent(msg);
                                while( ! backPub.offer(null,copy,0,len,true) ) {
                                    // spin
                                }
                                
//                                pool.returnBA(copy); // give back to pool
                            }
                        });
                    } else {
                        throw new RuntimeException("was soll das ?");
                    }
                }

                @Override
                public boolean dropped() {
                    // fatal, exit
                    System.out.println("process dropped ");
                    System.exit(-1);
                    return false;
                }

                @Override
                public void senderTerminated(String senderNodeId) {

                }

                @Override
                public void senderBootstrapped(String receivesFrom, long seqNo) {

                }
            }
        );
    }

    // just a simple test pool impl. Pretty inefficient, but don't want to introduce more dependencies on this
    // example project
    public static class StupidPool {
        volatile AtomicReferenceArray<byte[]> pool;

        public StupidPool(int size) {
            pool = new AtomicReferenceArray<byte[]>(size);
        }

        public void returnBA( byte ba[] ) {
            for ( int i=0; i < pool.length(); i++ ) {
                if ( pool.get(i) == null ) {
                    if (pool.compareAndSet(i,null,ba)) {
                        return;
                    }
                }
            }
        }

        static int success = 0;
        static int fail = 0;
        public byte[] getBA() {
            for ( int i=0; i < pool.length(); i++ ) {
                byte ba[] = pool.get(i);
                if ( ba != null ) {
                    if (pool.compareAndSet(i,ba,null)) {
                        success++;
                        return ba;
                    }
                }
            }
            fail++;
            if ( fail % 100000 == 0) {
                System.out.println("************* SUCC: "+success+" fail "+fail);
            }
            return new byte[1500]; // this is just a test impl with fixed size byte arrays
        }

    }

    public static void main(String arg[]) throws Throwable {
        EventReceiver rec = new EventReceiver();

        rec.initFastCast();
        while( true )
            Thread.sleep(10_000_000l);

    }

}
