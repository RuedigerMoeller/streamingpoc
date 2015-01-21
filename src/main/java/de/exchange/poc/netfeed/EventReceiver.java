package de.exchange.poc.netfeed;

import org.HdrHistogram.Histogram;
import org.nustaq.fastcast.api.FastCast;
import org.nustaq.fastcast.api.util.ObjectPublisher;
import org.nustaq.fastcast.api.util.ObjectSubscriber;
import org.nustaq.fastcast.util.RateMeasure;

import java.util.concurrent.TimeUnit;

/**
 * Created by moelrue on 1/20/15.
 */
public class EventReceiver {

    FastCast fastCast;
    Histogram hi = new Histogram(TimeUnit.SECONDS.toNanos(10),3);
    private ObjectPublisher backPub; // pong for rtt measurement

    public void initFastCast() throws Exception {
        fastCast =  FastCast.getFastCast();
        fastCast.setNodeId("SUB");
        fastCast.loadConfig("fc.kson");

        backPub = new ObjectPublisher(
                fastCast.onTransport("back").publish("back"),
                MarketEvent.class
        );

        RateMeasure measure = new RateMeasure("receive rate");
        fastCast.onTransport("default").subscribe( "stream",
            new ObjectSubscriber(MarketEvent.class) {
                @Override
                protected void objectReceived(String s, long l, Object o) {
                    if ( "END".equals(o) ) {
                        backPub.sendObject(null,o,true);
//                        hi.outputPercentileDistribution(System.out,1000.0);
                        hi.reset();
                        return;
                    }
//                    hi.recordValue(System.nanoTime() - ((MeasuredEvent) o).getSendTimeStampNanos());
                    EventReceiver.this.objectReceived(s,l,o);
                    backPub.sendObject(null,o,true);
                    measure.count();
                }
        });
    }

    protected void objectReceived(String sender, long sequence, Object message) {
//        MarketEvent ev = (MarketEvent) message;
    }

    public static void main(String arg[]) throws Throwable {
        EventReceiver rec = new EventReceiver();

        rec.initFastCast();
        while( true )
            Thread.sleep(10_000_000l);

    }

}
