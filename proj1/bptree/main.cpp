#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "BPlusTree.h"


void task1(BPlusTree &tree) {
    std::cout << "Task1:" << std::endl;
    std::cout << "Size of Record:" << sizeof(RecordKey) << std::endl;
    std::cout << "Number of records:" << tree.getRecordNums(RecordKey{-1, 0}, RecordKey{-1, 2}) << std::endl;
    std::cout << "Size of Block:" << PAGE_SIZE << " Block Nums:" << tree.getPageNums() << std::endl;
    std::cout << "Task1 done." << std::endl;
    std::cout << std::endl;
}

void task2(BPlusTree &tree) {
    std::cout << "Task2:" << std::endl;
    std::cout << "the parameter n of the B+ tree is " << DEGREE << std::endl;
    std::cout << "the number of nodes of the B+ tree is" << tree.getPageNums() << std::endl;
    std::cout << "the number of levels of the B+ tree is " << tree.getDepth() << std::endl;
    std::cout << "the content of the root node:" << std::endl;
    //tree.print();
    std::cout << "Task2 done." << std::endl;
    std::cout << std::endl;
}

void task3(BPlusTree &tree) {
    std::cout << "Task3:" << std::endl;

    RecordKey start, end;
    // 统计访问内部节点次数以及统计访问区块次数
    int accessIndexTimes = 0, accessBlockTimes = 0;
    auto vec = tree.rangeSearch(RecordKey{-1, 0.6}, RecordKey{-1, 0.9}, accessIndexTimes,
                                accessBlockTimes);
    std::cout << "search those movies with the attribute \"FG_PCT_home\" from 0.6 to 0.9:" << std::endl;
    double tmp = 0;
    for (auto rec : vec) {
        tmp += rec.key.fg3_pct_home;
    }
    std::cout << "Average FG3%: " << tmp / vec.size() << std::endl;
    std::cout << " accessIndexTimes:" << accessIndexTimes << " accessBlockTimes:" << accessBlockTimes << std::endl;
    std::cout << "Task3 done." << std::endl;
    std::cout << std::endl;
}
int main() {
    BPlusTree tree(DEGREE, "db.ctx", "a.db", true);
//    tree.print();


    std::fstream fileStream;  // 文件流用于读写磁盘
    std::string filePath = "/Users/zuozhiyi/Desktop/proj_1/ntu-sc3020/proj1/games.txt";  // 存储磁盘文件路径
    fileStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
    //打开失败异常处理
    if (!fileStream.is_open()) {
        std::cout << "open file fail!" << std::strerror(errno) << std::endl;
    }
    //跳过第一行的读取，然后按照行方式读取数据
    std::string line;
    std::getline(fileStream, line);
    std::vector<TeamsRecord> records; // 存储解析后的数据
    int id = 0;
    //读取剩余的数据
    while (std::getline(fileStream, line)) {
        std::istringstream ss(line);
        std::string game_date_est;
        TeamsRecord record;
        record.key.id = ++id;
        // 解析各个字段
        std::getline(ss, game_date_est, '\t'); // 读取日期
        strcpy(record.game_date_set, game_date_est.c_str());
        ss >> record.team_id_home;
        ss.ignore(1, '\t');
        ss >> record.pts_home;
        ss.ignore(1, '\t');
        ss >> record.fg_pct_home;
        ss.ignore(1, '\t');
        ss >> record.ft_pct_home;
        ss.ignore(1, '\t');
        ss >> record.key.fg3_pct_home;
        ss.ignore(1, '\t');
        ss >> record.ast_home;
        ss.ignore(1, '\t');
        ss >> record.reb_home;
        ss.ignore(1, '\t');
        ss >> record.home_team_wins;

        records.push_back(record);
    }

    fileStream.close(); // 关闭文件
    // // 排序，默认按 fg3_pct_home 降序，如果相同，则按 id 升序
    // std::sort(records.begin(), records.end(), [](const TeamsRecord &a, const TeamsRecord &b) {
    //     return a.getKey() > b.getKey();
    // });
    //
    // 输出排序后的数据
//    for (const auto &r : records) {
//      std::cout << "Date: " << r.game_date_set
//                << ", Team ID: " << r.team_id_home
//                << ", PTS: " << r.pts_home
//                << ", FG%: " << r.fg_pct_home
//                << ", FT%: " << r.ft_pct_home
//                << ", FG3%: " << r.key.fg3_pct_home
//                << ", AST: " << r.key.id
//                << ", REB: " << r.reb_home
//                << ", Wins: " << r.home_team_wins << std::endl;
//    }

    for (auto rec : records) {
        tree.insert(rec.key, rec);
    }

    task1(tree);
    task2(tree);
    task3(tree);
    return 0;
}
