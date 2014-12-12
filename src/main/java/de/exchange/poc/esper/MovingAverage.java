package de.exchange.poc.esper;

import com.espertech.esper.client.EPAdministrator;
import com.espertech.esper.client.EPStatement;
import com.espertech.esper.client.UpdateListener;

/**
 * Created by moelrue on 12/12/14.
 */
public class MovingAverage {
    private EPStatement statement;

    public MovingAverage(EPAdministrator admin)
    {
        String stmt = "insert into PriceAvg " +
                "select Avg(price) as movAvg from MarketDataEvent.win:time_batch(1 sec) group by symbol";

        statement = admin.createEPL(stmt);
    }

    public void addListener(UpdateListener listener)
    {
        statement.addListener(listener);
    }
}
