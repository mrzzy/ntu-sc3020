import java.io.*;
import java.util.*;

public class Storage {
    private static final String INPUT_FILE = "games.txt"; // 
    private static final String OUTPUT_FILE = "data.bin"; // Binary storage file

    // Method to read records from games.txt
    public static List<Record> readRecordsFromFile() throws IOException {
        List<Record> records = new ArrayList<>();
        BufferedReader br = new BufferedReader(new FileReader(INPUT_FILE));
        String line;

        br.readLine(); // Skip header

        while ((line = br.readLine()) != null) {
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }

            String[] values = line.split("\\s+"); // Use whitespace as the delimiter

            if (values.length > 13) { // Ensure there are enough elements
                try {
                    float FG_PCT_home = Float.parseFloat(values[10]); // Adjust index based on dataset
                    float FG3_PCT_home = Float.parseFloat(values[13]); // Adjust index if needed
                    records.add(new Record(FG_PCT_home, FG3_PCT_home));
                } catch (NumberFormatException e) {
                    System.out.println("Skipping line due to number format issue: " + line);
                }
            }
        }
        br.close();
        return records;
    }

    // Method to save records in binary format
    public static void saveRecordsToBinary(List<Record> records) throws IOException {
        ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(OUTPUT_FILE));
        for (Record record : records) {
            out.writeObject(record);
        }
        out.close();
    }

    // Method to read records from binary file
    public static List<Record> loadRecordsFromBinary() throws IOException, ClassNotFoundException {
        List<Record> records = new ArrayList<>();
        ObjectInputStream in = new ObjectInputStream(new FileInputStream(OUTPUT_FILE));

        try {
            while (true) {
                records.add((Record) in.readObject());
            }
        } catch (EOFException e) {
            in.close();
        }
        return records;
    }

    // ✅ Brute-force search: Scan all records and filter those in range
    public static List<Record> bruteForceSearch(float min, float max) throws IOException, ClassNotFoundException {
        List<Record> records = loadRecordsFromBinary();
        List<Record> results = new ArrayList<>();

        for (Record record : records) {
            if (record.FG_PCT_home >= min && record.FG_PCT_home <= max) {
                results.add(record);
            }
        }
        return results;
    }
}
