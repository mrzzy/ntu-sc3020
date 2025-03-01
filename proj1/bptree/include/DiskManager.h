//
// Created on 2025/2/20.
//

#ifndef BPTREE_DISKMANAGER_H
#define BPTREE_DISKMANAGER_H
#include "def.h"
#include "DataBaseContext.h"
class DiskManager {
public:
    DiskManager(const std::string &ctx_path, const std::string &file_path, bool is_trunc);

    ~DiskManager();

    // 分配新页面，返回页面 ID
    PageID allocatePage();

    void readCtx(DataBaseContext &ctx);
    void writeCtx(const DataBaseContext &ctx);

    int getTotalPages();

    // 读取指定页到节点对象
    void readPage(PageID page_id, Node &node);

    // 将节点写入指定页
    void writePage(PageID page_id, const Node &node);

    // 释放页面
    void freePage(PageID page_id);

    bool isHasInitCtx() const;

    void setHasInitCtx(bool hasInitCtx);

private:
    std::string file_path_;  // 存储磁盘文件路径
    std::string ctx_path_;
    std::fstream ctx_stream_;
    std::fstream file_stream_;  // 文件流用于读写磁盘
    PageID page_count_ = 0;  // 当前页面计数，控制分配的页面ID
    bool has_init_ctx_;
};


#endif //BPTREE_DISKMANAGER_H
