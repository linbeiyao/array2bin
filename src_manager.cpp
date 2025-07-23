#include <fstream>
#include <iostream>
#include <vector>


#include "src_manager.hpp"


uint16_t reg_src_count = 0;
uint16_t reg_src_image_count = 0;
uint16_t reg_src_font_count = 0;
std::vector<tImage*> src_image_array;
std::vector<std::string> images_name;
std::vector<tFont*> src_font_array;
std::vector<std::string> fonts_name;


#define IMAGE_LOGO_MQ_ADDR 0x00000000
#define FONT_MQ2_CN_22_ADDR 0x00005000
#define FONT_MQ2_EN_22_ADDR 0x0000E000
#define FONT_MQ2_CN_14_ADDR 0x00015000


void src_manager_init() {  
   // 注册源数据  
   reg_src_data(&logo_MQ, "logo_MQ");
   reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22");  


   std::cout << "源数据注册完成" << std::endl
    << "注册源数据个数：" << reg_src_count << std::endl
    << "注册图片个数：" << reg_src_image_count << std::endl
    << "注册字体个数：" << reg_src_font_count << std::endl;
}  

void src_manager_deinit() {
    src_image_array.clear();
    src_font_array.clear();
    reg_src_count = 0;
    reg_src_image_count = 0;
    reg_src_font_count = 0;
}

is_type_e is_src_type(size_t index) {
    // 根据注册的数量来判断是图片还是字库
    if (index < reg_src_image_count) {
        return is_image;
    } else if (index < (reg_src_image_count + reg_src_font_count)) {
        return is_font;
    } else {
        return is_type_max; // 超出范围
        std::cout << "未知的资源类型，下标：" << index << std::endl;
    }
}


void reg_src_data(tImage* src, const char *name, size_t addr) {
    src_image_array.push_back(src);
    images_name.push_back(name);
    reg_src_count++;
    reg_src_image_count++;
}

void reg_src_data(tFont* src, const char *name, size_t addr) {
    src_font_array.push_back(src);
    fonts_name.push_back(name);
    reg_src_count++;
    reg_src_font_count++;
}


/**
 * #define BMP_BPP_1       (1<<0)
 * #define BMP_BPP_2       (1<<1)
 * #define BMP_BPP_4       (1<<2)
 * #define BMP_BPP_8       (1<<3)
 * #define BMP_BPP_16      (1<<4)
 * #define BMP_BPP_32      (1<<5)
 */
size_t get_src_data_datasize(size_t index){
    is_type_e src_type = is_src_type(index);
    if(src_type == is_image){
        return src_image_array[index]->dataSize;     
    } else if(src_type == is_font){
        size_t font_index = index - reg_src_image_count;
        return src_font_array[font_index]->chars[0].image->dataSize;
    } else if(src_type == is_type_max){
        std::cout << "获取资源的像素bit位出错,类型未知,下标：" << index << std::endl;
        return 0;
    }
}


// 获取图片资源的数据大小（bit数）
size_t get_src_data_size(size_t image_index) {
    if (image_index >= reg_src_image_count) {
        return 0;
    }
    const tImage* img = src_image_array[image_index];
    if (img == nullptr) return 0;
    size_t bit_count = img->width * img->height * img->dataSize; 
    return bit_count;
}

// 获取字体中某个字符图像的数据大小（bit数）
size_t get_src_data_size(size_t font_index, size_t font_image_index) {
    if (font_index >= reg_src_font_count) {
        return 0;
    }
    const tFont* font = src_font_array[font_index];
    if (font == nullptr) return 0;
    if (font_image_index >= (size_t)font->length) {
        return 0;
    }
    const tImage* img = font->chars[font_image_index].image;
    if (img == nullptr) {
        return 0;
    }
    size_t bit_count = img->width * img->height * img->dataSize; 
    return bit_count;  // 字节数
}



void src_manager_write_image(std::ostream& os, std::map<std::string, size_t>& addr_map, size_t& offset) {
    for (size_t i = 0; i < reg_src_image_count; ++i) {
        if (i >= src_image_array.size()) continue;
        const tImage* img = src_image_array[i];
        if (!img) continue;
        size_t datasize = get_src_data_datasize(i);
        size_t bit_len = get_src_data_size(i);
        size_t pixel_count = bit_len / datasize;
        const uint16_t* data_ptr = img->data;
        if (!data_ptr) continue;

        // 记录当前图片的写入地址
        addr_map[images_name[i]] = offset;
        
        for (size_t j = 0; j < pixel_count; ++j) {
            uint16_t value = data_ptr[j];
            uint8_t low = value & 0xFF;
            uint8_t high = (value >> 8) & 0xFF;
            os.put(low);
            os.put(high);
        }

        offset += (bit_len + 7) / 8;

        std::cout << "图片索引: " << i
                  << " | 图片名称: " << images_name[i]
                  << " | 图片大小: " << pixel_count << " 像素"
                  << " | 图片地址: 0x" << std::hex << std::uppercase << offset << std::dec
                  << std::endl;

        
    }
}

void src_manager_write_font(std::ostream& os, std::map<std::string, size_t>& addr_map, size_t& offset){
    for(size_t i = 0; i < reg_src_font_count; ++i){
        if (i >= src_font_array.size()) continue;
        const tFont* font = src_font_array[i];
        if (!font || !font->chars) continue;
        // 记录当前字体的写入地址
        addr_map[fonts_name[i]] = offset;

        for(size_t j = 0; j < (size_t)font->length; j++){
            const tChar* chars = font->chars;
            if (!chars[j].image) continue;
            size_t datasize = get_src_data_datasize(i + reg_src_image_count);
            if (!datasize) continue;
            size_t bit_len = get_src_data_size(i, j);
            size_t pixel_count = bit_len / datasize;
            const uint16_t* data_ptr = chars[j].image->data;
            if (!data_ptr) continue;
            // 记录每个字符图像的写入地址，key格式为"字体名_字符索引"
            if (i < fonts_name.size()) {
                std::string key = fonts_name[i] + "_" + std::to_string(chars[j].code);
                addr_map[key] = offset;
            }
            for(size_t b  = 0; b < pixel_count; b++){
                uint16_t value = data_ptr[b];
                uint8_t low = value & 0xFF;
                uint8_t high = (value >> 8) & 0xFF;
                os.put(low);
                os.put(high);
                
            }
            offset += (bit_len + 7) / 8;
        }
    }
}





/* 大端写法：高字节在前  */
void write_big_endian(std::ofstream& outfile, uint16_t v) {
    uint8_t high = (v >> 8) & 0xFF;
    uint8_t low = v & 0xFF;
    outfile.put(high);
    outfile.put(low);
}

/* 小端写法：低字节在前 */
void write_little_endian(std::ofstream& outfile, uint16_t v) {
    uint8_t low = v & 0xFF;
    uint8_t high = (v >> 8) & 0xFF;
    outfile.put(low);
    outfile.put(high);
}
