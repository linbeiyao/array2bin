#ifndef SRC_MANAGER_HPP
#define SRC_MANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <map>

/// 资源类型枚举
enum is_type_e {
    is_image = 0,
    is_font,
    is_audio,
    is_type_max,
    is_unknown = 0xFF // 未知类型
};

/// 图像结构体（单张）
struct tImage {
    const uint16_t* data;  // 图像像素数据
    uint16_t width;        // 宽度（像素）
    uint16_t height;       // 高度（像素）
    uint8_t dataSize;      // 每像素 bit 数（一般为16）
};

/// 字符结构体（用于字体）
struct tChar {
    long int code;         // 字符编码（可为 UNICODE）
    const tImage* image;   // 对应字符的图像指针
};

/// 字体结构体（由多个字符图像组成）
struct tFont {
    int length;            // 字符数量
    const tChar* chars;    // 字符数组
};

/// 注册的资源结构体
struct src_item_t {
    void* data;                // 指向 tImage* 或 tFont*
    is_type_e type;            // 资源类型
    std::string name;          // 资源名称
    size_t satrt_addr;               // 存储地址（一般为 SPI Flash 内部偏移地址）
    size_t end_addr;
    size_t data_byte_count;    // 数据大小（以字节为单位）
};

/// 外部资源声明（你可以根据需要继续添加）
extern tImage logo_MQ;
extern tFont Font_MQ2_EN_22;
extern tFont Font_MQ2_CN_22;
extern tFont Font_MQ2_CN_14;

/// 资源注册函数
void src_manager_init();   // 初始化并注册所有资源
void src_manager_deinit(); // 清空资源（一般用于工具结束后）

/// 单个资源注册接口（addr 可选）
void reg_src_data(void* src, const char* name, is_type_e type, size_t addr);

/// 获取资源数据字节数（单位为 byte）
size_t get_src_data_datasize(size_t index);

/// 获取图片/字体某图像资源的位大小（单位为 bit）
size_t get_src_data_size(const tImage* image);
size_t get_src_data_size(const tFont* font, size_t font_image_index);

/// 获取资源类型
is_type_e is_src_type(size_t index);

/// 将资源数据写入到二进制文件中（按小端方式）
void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map);

/// 数据验证
/// 返回值：true 表示校验通过，false 表示有错误
bool src_manager_verify_data();


// 已注册资源数组和数量
extern uint16_t reg_src_count;
extern uint16_t reg_src_image_count;
extern uint16_t reg_src_font_count;

extern std::vector<src_item_t> src_item_array;

#endif // SRC_MANAGER_HPP
