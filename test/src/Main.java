import java.io.IOException;
import java.util.List;

public class Main {
    public static void main(String[] args) {
        try {
            // Load records
            List<Record> records = Storage.readRecordsFromFile();
            Storage.saveRecordsToBinary(records);
            List<Record> loadedRecords = Storage.loadRecordsFromBinary();

            // Insert into B+ Tree
            BPlusTree tree = new BPlusTree();
            for (Record rec : loadedRecords) {
                tree.insert(rec.FG_PCT_home, rec);
            }

            System.out.println("✅ B+ Tree Indexing Complete!");

            // 🔍 Perform Search for FG_PCT_home in Range [0.6, 0.9]
            float min = 0.6f, max = 0.9f;

            // B+ Tree Search
            long startTime = System.nanoTime();
            List<Record> bPlusResults = tree.rangeSearch(min, max);
            long endTime = System.nanoTime();
            long bPlusTime = (endTime - startTime) / 1000000; // Convert to milliseconds

            // Brute Force Search
            startTime = System.nanoTime();
            List<Record> bruteForceResults = Storage.bruteForceSearch(min, max);
            endTime = System.nanoTime();
            long bruteForceTime = (endTime - startTime) / 1000000; // Convert to milliseconds

            // Print results
            System.out.println("\n🔍 Search Results (FG_PCT_home ∈ [" + min + ", " + max + "])");
            System.out.println("✅ B+ Tree: " + bPlusResults.size() + " records found in " + bPlusTime + " ms");
            System.out.println("✅ Brute Force: " + bruteForceResults.size() + " records found in " + bruteForceTime + " ms");

            // Calculate Average FG3_PCT_home
            float avgFG3_BPlus = calculateAverageFG3(bPlusResults);
            float avgFG3_Brute = calculateAverageFG3(bruteForceResults);

            System.out.println("\n📊 Average FG3_PCT_home for results:");
            System.out.println("✅ B+ Tree: " + avgFG3_BPlus);
            System.out.println("✅ Brute Force: " + avgFG3_Brute);

        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    // Helper method to calculate average FG3_PCT_home
    public static float calculateAverageFG3(List<Record> records) {
        if (records.isEmpty()) return 0;
        float sum = 0;
        for (Record r : records) {
            sum += r.FG3_PCT_home;
        }
        return sum / records.size();
    }
}
