//
// Created on 2025/2/20.
//

#ifndef BPTREE_TEAMSRECORD_H
#define BPTREE_TEAMSRECORD_H


struct RecordKey {
    int id;
    float fg3_pct_home;

    // 重载小于号操作符以实现自定义排序规则
    bool operator<(const RecordKey &other) const {
        if (fg3_pct_home == other.fg3_pct_home) {
            // 如果 fg3_pct_home 相等，按 id 排序
            return id < other.id;
        }
        // 否则，按 fg3_pct_home 排序
        return fg3_pct_home < other.fg3_pct_home;
    }

    bool operator>(const RecordKey &other) const {
        return other < *this;
    }

    bool operator==(const RecordKey &other) const {
        return id == other.id && fg3_pct_home == other.fg3_pct_home;
    }

    bool operator>=(const RecordKey &other) const {
        if (fg3_pct_home == other.fg3_pct_home) {
            if (id == -1 || other.id == -1) {
                return true;
            }
            return id >= other.id;
        }
        return fg3_pct_home > other.fg3_pct_home;
    }

    bool operator<=(const RecordKey &other) const {
        if (fg3_pct_home == other.fg3_pct_home) {
            if (id == -1 || other.id == -1) {
                return true;
            }
            return id <= other.id;
        }
        return fg3_pct_home < other.fg3_pct_home;
    }

};


//NBA 队伍的数据
struct TeamsRecord {
    RecordKey key;
    char game_date_set[16];
    uint32_t team_id_home;
    uint32_t pts_home;
    float fg_pct_home;
    float ft_pct_home;
    uint32_t ast_home;
    uint32_t reb_home;
    uint32_t home_team_wins;
};

#endif //BPTREE_TEAMSRECORD_H
