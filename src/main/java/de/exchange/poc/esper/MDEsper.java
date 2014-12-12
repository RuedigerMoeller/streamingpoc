package de.exchange.poc.esper;

import com.espertech.esper.client.*;
import de.exchange.poc.MarketDataEvent;
import org.HdrHistogram.Histogram;

import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;

/**
 * Created by moelrue on 12/12/14.
 */
public class MDEsper {

    EPServiceProvider epService;
    public void init() {
        // Configure engine with event names to make the statements more readable.
        // This could also be done in a configuration file.
        Configuration configuration = new Configuration();
        configuration.addEventType("MarketDataEvent", MarketDataEvent.class.getName());

        // Get engine instance
        epService = EPServiceProviderManager.getProvider("Marketfeed", configuration);

        // Set up statements
        MovingAverage priceAvg = new MovingAverage(epService.getEPAdministrator());
        priceAvg.addListener(new UpdateListener() {
            @Override
            public void update(EventBean[] eventBeans, EventBean[] eventBeans2) {
                if ( eventBeans.length >  0 )
                    System.out.println("moving Avg "+eventBeans[0].get("movAvg"));
            }
        });

        TicksPerSecondStatement tickPerSecStmt = new TicksPerSecondStatement(epService.getEPAdministrator());
        tickPerSecStmt.addListener(new RateReportingListener());

        final Histogram histo = new Histogram(TimeUnit.SECONDS.toNanos(10),5);

        TimeStampsStatement tss = new TimeStampsStatement(epService.getEPAdministrator());
        tss.addListener(new UpdateListener() {
            int count = 0;
            @Override
            public void update(EventBean[] newEvents, EventBean[] oldEvents) {
                if ( newEvents != null ) {
                    for (int i = 0; i < newEvents.length; i++) {
                        long tim = System.nanoTime();
                        EventBean newEvent = newEvents[i];
                        histo.recordValue(tim-((Long) newEvent.get("nanoTimeStamp")).longValue());
                        count++;
                        if ( count > 100_000_000 ) {
                            count = 0;
                            histo.outputPercentileDistribution(System.out, 1000.0);
                            histo.reset();
                        }
                    }
                }
            }
        });

    }

    String symbols[] = { "BMW", "ALV", "FDAX", "DAX" };

    public void runTest() {
        ThreadLocalRandom random = ThreadLocalRandom.current();

        long nanosPause = 2*1000; // 500k
        double price = 100.0;
        long startTime = System.nanoTime();
        while( true ) {
            while( System.nanoTime() - startTime < nanosPause ) {

            }
            startTime = System.nanoTime();
            price+=random.nextDouble()*2;
            String symbol = symbols[ 0 ];
            MarketDataEvent ev = new MarketDataEvent(price,100,symbol);
            epService.getEPRuntime().sendEvent(ev);
        }
    }


    public static void main(String arg[]) {
        MDEsper esper = new MDEsper();
        esper.init();
        esper.runTest();
    }
}
