 #ifndef SRC_MANAGER_HPP
#define SRC_MANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <map>
#include "include_src.hpp"

/// 数据头结构体
struct src_head_t {
    char     version[32];      // 文件版本/名称，例如 "MQ2_V1_20250815"
    uint16_t resource_count;   // 资源总数（包含字体的每个字符）
    uint16_t image_count;      // 图像资源数量
    uint16_t font_count;       // 字体资源数量
    uint16_t reserved1;        // 对齐填充/保留
    uint32_t total_size;       // 数据区总大小（字节）
    uint32_t addr_table_offset;// 地址表起始位置（相对于文件头）
    uint32_t addr_table_size;  // 地址表总大小（字节）
    uint32_t reserved[4];      // 预留扩展（CRC、压缩方式等）
};

/// 地址表项结构体
struct src_addr_item_t {
    uint32_t id;            // 全局唯一 ID  
    uint32_t parent_id;     // 父资源 ID (0 表示顶级)
    uint32_t start_addr;    // 数据起始地址
    uint32_t size;          // 数据大小
    uint16_t type;          // is_image/is_font/is_audio/is_font_char
    uint16_t char_code;     // 仅当 font_char 时有效（Unicode 编码）
    char     name[48];      // 资源名
};

/// 注册的资源结构体
struct src_item_t {
    void* data;                // 指向 tImage* 或 tFont*
    is_type_e type;            // 资源类型
    std::string name;          // 资源名称
    size_t satrt_addr;         // 存储地址
    size_t end_addr;
    size_t data_byte_count;    // 数据大小（以字节为单位）
    std::vector<src_addr_item_t> addr_table;        // 该项资源的地址表
};



// 已注册资源数组和数量
extern uint16_t reg_src_count;
extern uint16_t reg_src_image_count;
extern uint16_t reg_src_font_count;

extern src_head_t src_head;
extern std::vector<src_item_t> src_item_array;



/// 资源注册函数
void src_manager_init();   // 初始化并注册所有资源
void src_manager_deinit(); // 清空资源（一般用于工具结束后）

/// 数据头创建函数
void creat_flash_data_head(std::string name, std::map<std::string, size_t>& addr_map);

// 数额据写入函数
void src_manager_write_head(std::ostream& os, src_head_t head);
void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map);
void src_manager_write_addr_table(std::ostream& os);

/// 数据验证
bool src_manager_verify_data();

void name_add_time_str(std::string* bin_file_name, std::string* log_file_name);







#endif // SRC_MANAGER_HPP
