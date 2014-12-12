package de.exchange.poc.esper;

import com.espertech.esper.client.EPAdministrator;
import com.espertech.esper.client.EPStatement;
import com.espertech.esper.client.UpdateListener;
import org.HdrHistogram.Histogram;

import java.util.concurrent.TimeUnit;

/**
 * Created by moelrue on 12/12/14.
 */
public class TimeStampsStatement {
    private EPStatement statement;

    public TimeStampsStatement(EPAdministrator admin)
    {
        String st = "insert into Latency select nanoTimeStamp from MarketDataEvent";
        statement = admin.createEPL(st);
    }

    public void addListener(UpdateListener listener)
    {
        statement.addListener(listener);
    }
}
