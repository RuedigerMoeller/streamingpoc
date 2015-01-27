package de.exchange.poc.netfeed;

import org.HdrHistogram.Histogram;
import org.nustaq.fastcast.api.FCPublisher;
import org.nustaq.fastcast.api.FCSubscriber;
import org.nustaq.fastcast.api.FastCast;
import org.nustaq.fastcast.api.util.ObjectPublisher;
import org.nustaq.fastcast.api.util.ObjectSubscriber;
import org.nustaq.fastcast.util.RateMeasure;
import org.nustaq.offheap.bytez.Bytez;
import org.nustaq.offheap.structs.FSTStruct;
import org.nustaq.offheap.structs.structtypes.StructString;
import org.nustaq.offheap.structs.unsafeimpl.FSTStructFactory;

import java.lang.invoke.MethodHandle;
import java.util.ArrayList;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;
import java.util.function.IntFunction;

/**
 * Created by moelrue on 1/20/15.
 */
public class EventPublisher {

    FastCast fastCast;
    FCPublisher pub;
    Histogram hi = new Histogram(TimeUnit.SECONDS.toNanos(10),3);
    Executor dumper = Executors.newCachedThreadPool();

    public void initFastCast() throws Exception {
        fastCast =  FastCast.getFastCast();
        fastCast.setNodeId("PUB");
        fastCast.loadConfig("fc.kson");

        pub = fastCast.onTransport("default").publish("stream");
        final FSTStruct msg = FSTStructFactory.getInstance().createEmptyStructPointer(FSTStruct.class);


        fastCast.onTransport("back").subscribe( "back",
        new FCSubscriber() {
            @Override
            public void messageReceived(String sender, long sequence, Bytez b, long off, int len) {
                // as structs decode literally in zero time, we can decode inside receiver thread
                msg.baseOn(b, (int) off);
                Class type = msg.getPointedClass();
                if ( type == StructString.class ) {
                    // sequence end, print histogram
                    final Histogram oldHi = hi;
                    hi = new Histogram(TimeUnit.SECONDS.toNanos(2),3);
                    // no lambdas to stay 1.7 compatible
                    // move printing out of the receiving thread
                    dumper.execute(new Runnable() {
                        @Override
                        public void run() {
                            oldHi.outputPercentileDistribution(System.out,1000.0);
                        }
                    });
                } else { // a regular message, record latency
                    MarketEventStruct mdata = msg.cast();
                    long value = System.nanoTime() - mdata.getSendTimeStampNanos();
                    if ( value < 1_000_000_000 ) { // avoid AIOB during init + JITTING
                        hi.recordValue(value);
                    }
                    // a real app would need to copy the message (recycle byte arrays/objects !) and
                    // run msg processing in an executor to get out of the receiving thread.
                    // mdata gets invalid on finish of this method
                }
            }

            @Override
            public boolean dropped() {
                return false;
            }

            @Override
            public void senderTerminated(String senderNodeId) {

            }

            @Override
            public void senderBootstrapped(String receivesFrom, long seqNo) {

            }
        });
    }

    public void run( int pauseNanos, int numEvents ) throws Throwable {
        RateMeasure report = new RateMeasure("send rate");
        MarketEventStruct struct = (MarketEventStruct) new MarketEventStruct().toOffHeap();
        Bytez end = new StructString("END").toOffHeap().getBase();
        for ( int i = 0; i < numEvents; i++ ) {
            double bidPrc = Math.random() * 10;
            struct.setBidPrc(bidPrc);
            struct.setAskPrc(bidPrc + 1);
            struct.setAskQty(10);
            struct.setBidQty(11);
            struct.setSendTimeStampNanos(System.nanoTime());
            while( ! pub.offer( null, struct.getBase(), true ) ) {
                // spin
            }
            report.count();
            long time = System.nanoTime();
            while( System.nanoTime() - time < pauseNanos ) {
                // spin
            }
        }
        while( ! pub.offer(null, end, true) ) {
            // spin
        }
    }

    public MarketEvent createMarketEvent(int i) {
        double bidPrc = Math.random() * 10;
        return new MarketEvent(System.nanoTime(), bidPrc, bidPrc+1, 10, 110);
    }

    public static void main(String arg[]) throws Throwable {
        EventPublisher pub = new EventPublisher();

        pub.initFastCast();
        while (true)
            pub.run( 27_000, 1_000_000 ); // 93_000 = 10k, 27_000 = 30k, 10_500 = 70k, 4_900 = 140k

    }

}
