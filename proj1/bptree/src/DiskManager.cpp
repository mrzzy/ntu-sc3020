//
// Created on 2025/2/20.
//

#include "DiskManager.h"

DiskManager::DiskManager(const std::string &ctx_path, const std::string &file_path, bool is_trunc) : file_path_(file_path) {
    if (!is_trunc) {
        //不清数据的方法
        ctx_stream_.open(ctx_path, std::ios::in | std::ios::out | std::ios::binary);
        if (!ctx_stream_) {
            has_init_ctx_ = false;
            std::cout << "database not exist,try to init" << std::endl;
            ctx_stream_.open(ctx_path, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
            if (!ctx_stream_) {
                std::cerr << "Error: Failed to open disk file!" << std::endl;
                exit(1);
            }
            // 打开磁盘文件（数据库文件）
            file_stream_.open(file_path_, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
            if (!file_stream_) {
                std::cerr << "Error: Failed to open disk file!" << std::endl;
                exit(1);
            }
        } else {
            has_init_ctx_ = true;
            std::cout << "database context open success" << std::endl;
            // 打开磁盘文件（数据库文件）
            file_stream_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
            if (!file_stream_) {
                std::cerr << "Error: Failed to open disk file!" << std::endl;
                exit(1);
            }
        }
    } else {
        //清数据的方法
        has_init_ctx_ = false;
        std::cout << "trunc mode: database not exist,try to init" << std::endl;
        ctx_stream_.open(ctx_path, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
        if (!ctx_stream_) {
            std::cerr << "Error: Failed to open disk file!" << std::endl;
            exit(1);
        }
        // 打开磁盘文件（数据库文件）
        file_stream_.open(file_path_, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
        if (!file_stream_) {
            std::cerr << "Error: Failed to open disk file!" << std::endl;
            exit(1);
        }
    }

}

PageID DiskManager::allocatePage() {
    file_stream_.seekp(0, std::ios::end);  // 定位到文件末尾
    PageID new_page_id = page_count_;
    page_count_++;  // 页面计数递增

    // 确保文件足够大
    file_stream_.seekp(new_page_id * PAGE_SIZE, std::ios::beg);
    std::vector<char> empty_page(PAGE_SIZE, 0);  // 填充空白页
    file_stream_.write(empty_page.data(), PAGE_SIZE);

    return new_page_id;
}

void DiskManager::readPage(PageID page_id, Node &node) {
    file_stream_.seekg(page_id * PAGE_SIZE, std::ios::beg);  // 定位到对应页面
    char buffer[PAGE_SIZE];
    file_stream_.read(buffer, PAGE_SIZE);

    // 将磁盘数据反序列化到节点
    std::memcpy(&node, buffer, sizeof(Node));
}

void DiskManager::writePage(PageID page_id, const Node &node) {
    file_stream_.seekp(page_id * PAGE_SIZE, std::ios::beg);  // 定位到对应页面
    char buffer[PAGE_SIZE];

    // 将节点数据序列化到磁盘
    std::memcpy(buffer, &node, sizeof(Node));
    file_stream_.write(buffer, PAGE_SIZE);
}

void DiskManager::freePage(PageID page_id) {
    // 简单地把页面数据覆盖为零，表示页面已被释放
    file_stream_.seekp(page_id * PAGE_SIZE, std::ios::beg);
    std::vector<char> empty_page(PAGE_SIZE, 0);
    file_stream_.write(empty_page.data(), PAGE_SIZE);
}


DiskManager::~DiskManager() {
    if (file_stream_) {
        file_stream_.close();
    }
}

void DiskManager::readCtx(DataBaseContext &ctx) {
    ctx_stream_.seekg(0, std::ios::beg);  // 定位到对应页面
    char buffer[PAGE_SIZE];
    ctx_stream_.read(buffer, PAGE_SIZE);

    // 将磁盘数据反序列化到节点
    std::memcpy(&ctx, buffer, sizeof(DataBaseContext));
}

void DiskManager::writeCtx(const DataBaseContext &ctx) {
    ctx_stream_.seekg(0, std::ios::beg);  // 定位到对应页面
    char buffer[PAGE_SIZE];

    // 将节点数据序列化到磁盘
    std::memcpy(buffer, &ctx, sizeof(DataBaseContext));
    ctx_stream_.write(buffer, PAGE_SIZE);

    has_init_ctx_ = true;

}

bool DiskManager::isHasInitCtx() const {
    return has_init_ctx_;
}

void DiskManager::setHasInitCtx(bool hasInitCtx) {
    has_init_ctx_ = hasInitCtx;
}

int DiskManager::getTotalPages() {
    //读取整个文件file_stream大小
    file_stream_.seekg(0, std::ios::end);
    int total_size = file_stream_.tellg();
    return total_size / PAGE_SIZE;
}
