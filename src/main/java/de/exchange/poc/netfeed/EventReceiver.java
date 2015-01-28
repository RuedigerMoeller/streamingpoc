package de.exchange.poc.netfeed;

import com.espertech.esper.client.*;
import de.exchange.poc.MarketDataEvent;
import de.exchange.poc.esper.TicksPerSecondStatement;
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
                    "select count(*) as cnt from MarketEvent.win:time_batch(1 sec)";

            statement = admin.createEPL(stmt);
        }

        public void addListener(UpdateListener listener)
        {
            statement.addListener(listener);
        }
    }

    public static class EmptyStatement
    {
        private EPStatement statement;

        public EmptyStatement(EPAdministrator admin)
        {
            String stmt = "select * from MarketEvent";

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
        configuration.addEventType("MarketEvent", MarketEvent.class.getName());

        // Get engine instance
        epService = EPServiceProviderManager.getProvider("MarketEvent", configuration);

//        RateStatement tickPerSecStmt = new RateStatement(epService.getEPAdministrator());
//        tickPerSecStmt.addListener(new UpdateListener() {
//            @Override
//            public void update(EventBean[] eventBeans, EventBean[] eventBeans1) {
//                if ( eventBeans.length == 1 ) {
//                    System.out.println( "Esper Rate Report "+eventBeans[0].get("cnt") );
//                }
//            }
//        });

        EmptyStatement emptyStatement = new EmptyStatement(epService.getEPAdministrator());
        emptyStatement.addListener(new UpdateListener() {
            @Override
            public void update(EventBean[] eventBeans, EventBean[] eventBeans1) {
                if ( eventBeans.length == 1 ) {
                    MarketEvent ev = (MarketEvent) eventBeans[0].getUnderlying();
                    long nanos = ev.getSendTimeStampNanos();
                    backBuf.setSendTimeStampNanos(nanos);
                    byte[] bytes = backBuf.getBase().asByteArray();
                    while( ! backPub.offer(null, bytes,0,backBuf.getByteSize(),true) ) {
                        // spin
                    }
                }
            }
        });

    }

    MarketEventStruct backBuf;
    public void initFastCast() throws Exception {
        FSTStructFactory.getInstance().registerClz(MarketEventStruct.class);
        backBuf = (MarketEventStruct) new MarketEventStruct().toOffHeap();

        fastCast =  FastCast.getFastCast();
        fastCast.setNodeId("SUB");
        fastCast.loadConfig("fc.kson");

        backPub = fastCast.onTransport("back").publish("back");

        final MarketEventStruct msg = (MarketEventStruct) FSTStructFactory.getInstance().createEmptyStructPointer(MarketEventStruct.class);

        RateMeasure measure = new RateMeasure("receive rate");
        fastCast.onTransport("default").subscribe("stream",
            new FCSubscriber() {
                @Override
                public void messageReceived(String sender, long sequence, Bytez b, long off, final int len) {
                    measure.count();
//                    final byte copy[] = pool.getBA();
//                    if ( len < copy.length ) // prevent segfault :) !
                    Class msgClass = FSTStructFactory.getInstance().getStructPointer(b, off).getPointedClass();
                    if ( msgClass == MarketEventStruct.class )
                    {
                        msg.baseOn(b,(int)off); // fixme could avoid pooling as event is read here ..
                        final MarketEvent ev = new MarketEvent(
                            msg.getSendTimeStampNanos(),
                            msg.getBidPrc(),
                            msg.getAskPrc(),
                            msg.getBidQty(),
                            msg.getAskQty()
                        );
                        epService.getEPRuntime().sendEvent(ev);
                        // do bounce back in different thread, else blocking on send will pressure back to
                        // sender resulting in whacky behaviour+throughput
//                        bounceBackExec.execute(new Runnable() {
//                            @Override
//                            public void run() {
//                                epService.getEPRuntime().sendEvent(ev);
//                            }
//                        });
                    } else { // assume "END"
                        while( ! backPub.offer(null,b,off,len,true) ) {
                            // spin
                        }
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

        rec.initEsper();
        rec.initFastCast();
        while( true )
            Thread.sleep(10_000_000l);

    }

}
