package de.exchange.poc.esper;

import com.espertech.esper.client.EventBean;
import com.espertech.esper.client.UpdateListener;

/**
 * Created by moelrue on 12/12/14.
 */
public class RateReportingListener implements UpdateListener
{
    public void update(EventBean[] newEvents, EventBean[] oldEvents)
    {
        if (newEvents.length > 0)
        {
            logRate(newEvents[0]);
        }
        if (newEvents.length > 1)
        {
            logRate(newEvents[1]);
        }
    }

    private void logRate(EventBean theEvent)
    {
        System.out.println("Current rate for symbol " + theEvent.get("symbol").toString() +
                " is " + theEvent.get("cnt"));
    }

}
