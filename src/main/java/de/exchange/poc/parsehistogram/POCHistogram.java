package de.exchange.poc.parsehistogram;

import java.util.ArrayList;

/**
 * Created by moelrue on 2/25/15.
 */
public class POCHistogram {

    static class HIEntry {
        public HIEntry(double time, int count) {
            this.time = time;
            this.count = count;
        }

        double time;
        int count;
    }

    ArrayList<HIEntry> entries = new ArrayList<>();
    int maxCount = 0;

    public void add(double time, int count) {
        entries.add( new HIEntry(time, count) );
        maxCount = Math.max(maxCount,count);
    }

    public void dump() {
        System.out.println("Histogram");
        int accQty = 0;
        for (int i = 0; i < entries.size(); i++) {
            HIEntry entry = entries.get(i);
            accQty += entry.count;
            double percentile = (double)entry.count/maxCount;
            System.out.println(entry.time+";"+entry.count+";"+String.format("%.2f",percentile));
        }
    }
}
