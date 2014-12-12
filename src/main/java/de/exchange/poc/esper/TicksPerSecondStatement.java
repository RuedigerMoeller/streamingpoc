package de.exchange.poc.esper;

import com.espertech.esper.client.EPAdministrator;
import com.espertech.esper.client.EPStatement;
import com.espertech.esper.client.UpdateListener;

/**
 * Created by moelrue on 12/12/14.
 */
public class TicksPerSecondStatement
{
    private EPStatement statement;

    public TicksPerSecondStatement(EPAdministrator admin)
    {
        String stmt = "insert into TicksPerSecond " +
                "select symbol, count(*) as cnt from MarketDataEvent.win:time_batch(1 sec) group by symbol";

        statement = admin.createEPL(stmt);
    }

    public void addListener(UpdateListener listener)
    {
        statement.addListener(listener);
    }
}
