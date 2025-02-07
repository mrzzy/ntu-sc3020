import java.io.Serializable;

public class Record implements Serializable {
    float FG_PCT_home; // Field goal percentage (Home team) - Used as B+ tree key
    float FG3_PCT_home; // 3-point field goal percentage (Home team)

    public Record(float FG_PCT_home, float FG3_PCT_home) {
        this.FG_PCT_home = FG_PCT_home;
        this.FG3_PCT_home = FG3_PCT_home;
    }

    @Override
    public String toString() {
        return "FG_PCT_home: " + FG_PCT_home + ", FG3_PCT_home: " + FG3_PCT_home;
    }
}
