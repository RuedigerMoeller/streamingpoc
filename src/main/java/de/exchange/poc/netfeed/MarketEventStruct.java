package de.exchange.poc.netfeed;

import org.nustaq.offheap.structs.FSTStruct;

/**
 * Created by moelrue on 1/27/15.
 */
public class MarketEventStruct extends FSTStruct {

    protected long sendTimeStampNanos;

    protected double bidPrc,askPrc;
    protected int bidQty, askQty;

    // required
    public MarketEventStruct() {
    }

    public MarketEventStruct(long sendTimeStampNanos, double bidPrc, double askPrc, int bidQty, int askQty) {
        this.sendTimeStampNanos = sendTimeStampNanos;
        this.bidPrc = bidPrc;
        this.askPrc = askPrc;
        this.bidQty = bidQty;
        this.askQty = askQty;
    }

    public long getSendTimeStampNanos() {
        return sendTimeStampNanos;
    }

    public void setSendTimeStampNanos(long sendTimeStampNanos) {
        this.sendTimeStampNanos = sendTimeStampNanos;
    }

    public double getBidPrc() {
        return bidPrc;
    }

    public void setBidPrc(double bidPrc) {
        this.bidPrc = bidPrc;
    }

    public double getAskPrc() {
        return askPrc;
    }

    public void setAskPrc(double askPrc) {
        this.askPrc = askPrc;
    }

    public int getBidQty() {
        return bidQty;
    }

    public void setBidQty(int bidQty) {
        this.bidQty = bidQty;
    }

    public int getAskQty() {
        return askQty;
    }

    public void setAskQty(int askQty) {
        this.askQty = askQty;
    }

}
