package de.exchange.poc;

/**
 * Created by moelrue on 12/12/14.
 */
public class MarketDataEvent {

    long nanoTimeStamp;

    double price;
    int quantity;
    String symbol;

    public MarketDataEvent(double price, int quantity, String symbol) {
        this.nanoTimeStamp = System.nanoTime();
        this.price = price;
        this.quantity = quantity;
        this.symbol = symbol;
    }

    public long getNanoTimeStamp() {
        return nanoTimeStamp;
    }

    public void setNanoTimeStamp(long nanoTimeStamp) {
        this.nanoTimeStamp = nanoTimeStamp;
    }

    public double getPrice() {
        return price;
    }

    public void setPrice(double price) {
        this.price = price;
    }

    public int getQuantity() {
        return quantity;
    }

    public void setQuantity(int quantity) {
        this.quantity = quantity;
    }

    public String getSymbol() {
        return symbol;
    }

    public void setSymbol(String symbol) {
        this.symbol = symbol;
    }

}
