package de.exchange.poc.netfeed;

import org.HdrHistogram.Histogram;
import org.nustaq.fastcast.api.FastCast;
import org.nustaq.fastcast.api.util.ObjectPublisher;
import org.nustaq.fastcast.api.util.ObjectSubscriber;
import org.nustaq.fastcast.util.RateMeasure;

import java.lang.invoke.MethodHandle;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;
import java.util.function.IntFunction;

/**
 * Created by moelrue on 1/20/15.
 */
public class EventPublisher {

    FastCast fastCast;
    ObjectPublisher pub;
    Histogram hi = new Histogram(TimeUnit.SECONDS.toNanos(10),3);

    public void initFastCast() throws Exception {
        fastCast =  FastCast.getFastCast();
        fastCast.setNodeId("PUB");
        fastCast.loadConfig("fc.kson");

        pub = new ObjectPublisher(
            fastCast.onTransport("default").publish("stream"),
            MarketEvent.class
        );

        fastCast.onTransport("back").subscribe( "back",
            new ObjectSubscriber(MarketEvent.class) {
                @Override
                protected void objectReceived(String s, long l, Object o) {
                    if ( "END".equals(o) ) {
                        hi.outputPercentileDistribution(System.out,1000.0);
                        hi.reset();
                        return;
                    }
                    hi.recordValue(System.nanoTime()-((MeasuredEvent)o).getSendTimeStampNanos());
                }
            });

    }

    public void run( IntFunction messageGenerator, int pauseNanos, int numEvents ) throws Throwable {
        RateMeasure report = new RateMeasure("send rate");
        for ( int i = 0; i < numEvents; i++ ) {
            Object event = messageGenerator.apply(i);
            pub.sendObject( null, event, true );
            report.count();
            long time = System.nanoTime();
            while( System.nanoTime() - time < pauseNanos ) {
                // spin
            }
        }
        pub.sendObject(null,"END", true);
    }

    public MarketEvent createMarketEvent(int i) {
        double bidPrc = Math.random() * 10;
        return new MarketEvent(System.nanoTime(), bidPrc, bidPrc+1, 10, 110);
    }

    public static void main(String arg[]) throws Throwable {
        EventPublisher pub = new EventPublisher();

        pub.initFastCast();
        while (true)
            pub.run( pub::createMarketEvent, 50000, 100_000 );

    }

}
