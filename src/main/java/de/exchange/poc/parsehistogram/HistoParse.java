package de.exchange.poc.parsehistogram;

import java.io.*;
import java.util.ArrayList;

/**
 * Created by moelrue on 2/25/15.
 */
public class HistoParse {

    public ArrayList readFile(File f) throws IOException {
        DataInputStream din = new DataInputStream(new FileInputStream(f));
        String line = null;
        ArrayList lines = new ArrayList();
        while( (line = din.readLine()) != null ) {
            lines.add(line);
        }
        din.close();
        return lines;
    }

    public ArrayList<POCHistogram> parseLines(ArrayList lines) {
        ArrayList<POCHistogram> res = new ArrayList<POCHistogram>();
        POCHistogram currHisto = null;

        for (int i = 0; i < lines.size(); i++) {
            try {
                String line = (String) lines.get(i);
                line = line.trim();
                if (line.startsWith("Value")) {
                    if (currHisto != null) {
                        currHisto.dump();
                    }
                    currHisto = new POCHistogram();
                } else if (line.startsWith("#")) {
                } else if (line.length() == 0) {
                } else {
                    String[] split = line.split("\\s+");
                    double time = Double.parseDouble(split[0].trim());
                    int count = Integer.parseInt(split[2].trim());
                    currHisto.add(time, count);
                }
            } catch (Exception e) {
                String[] split = ((String)lines.get(i)).split("\\s+");
                System.out.println(""+lines.get(i));
                e.printStackTrace();
            }
        }
        currHisto.dump();
        return res;
    }

    private ArrayList<POCHistogram> parse(File file) throws IOException {
        return parseLines(readFile(file));
    }

    public static void main( String a[] ) throws IOException {
        HistoParse hp = new HistoParse();

        File f[] = new File( "/home/moelrue/intermediates/esper_fc/network").listFiles();
        for (int i = 0; i < f.length; i++) {
            File file = f[i];
            if ( file.getName().endsWith(".txt") ) {
                ArrayList<POCHistogram> parse = hp.parse(file);
                for (int j = 0; j < parse.size(); j++) {
                    POCHistogram pocHistogram = parse.get(j);
                    pocHistogram.fillPercentile();
                    final String newFileName = new File(file.getParent() + File.separator + file.getName().substring(file.getName().length() - 4)) + "_" + i + ".csv";
                    PrintStream csv = new PrintStream( new FileOutputStream(newFileName) );
                    pocHistogram.entries.forEach( entry -> {
                        csv.println(entry.getTime()+";"+entry.getCount()+";"+entry.getPerc());
                    });
                    csv.close();
                }
            }
        }
        ArrayList<POCHistogram> parse = hp.parse(new File("/home/moelrue/IdeaProjects/streamingpoc/rawhistos/c-hdr-sample.txt"));
    }

}
