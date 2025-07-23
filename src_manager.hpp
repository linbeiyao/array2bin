#ifndef SRC_MANAGER_HPP
#define SRC_MANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <map>

enum is_type_e {
    is_image = 0,
    is_font,
    is_audio,
    is_type_max
};

struct tImage {
    const uint16_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t dataSize;
};

struct tChar {
    long int code;
    const tImage* image;
};

struct tFont {
    int length;
    const tChar* chars;
};

// 注册资源
void src_manager_init();
void src_manager_deinit();
void reg_src_data(tImage* src, const char* name);
void reg_src_data(tFont* src, const char* name);


// 数据获取接口
size_t get_src_data_datasize(size_t index);
size_t get_src_data_size(size_t index);
size_t get_src_data_size(size_t font_index, size_t font_image_index);
is_type_e is_src_type(size_t index);

// 写入资源数据（小端）
void src_manager_write_image(std::ostream& os, std::map<std::string, size_t>& addr_map, size_t& offset);
void src_manager_write_font(std::ostream& os, std::map<std::string, size_t>& addr_map, size_t& offset);

// 外部资源声明
extern tImage logo_MQ;
extern tFont Font_MQ2_EN_22;

// 已注册资源数组和数量
extern uint16_t reg_src_count;
extern uint16_t reg_src_image_count;
extern uint16_t reg_src_font_count;

extern std::vector<tImage*> src_image_array;
extern std::vector<tFont*> src_font_array;
extern std::vector<std::string> images_name;
extern std::vector<std::string> fonts_name;

#endif // SRC_MANAGER_HPP
