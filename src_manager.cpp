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
   // ע��Դ����  
   reg_src_data(&logo_MQ, "logo_MQ");
   reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22");  


   std::cout << "Դ����ע�����" << std::endl
    << "ע��Դ���ݸ�����" << reg_src_count << std::endl
    << "ע��ͼƬ������" << reg_src_image_count << std::endl
    << "ע�����������" << reg_src_font_count << std::endl;
}  

void src_manager_deinit() {
    src_image_array.clear();
    src_font_array.clear();
    reg_src_count = 0;
    reg_src_image_count = 0;
    reg_src_font_count = 0;
}

is_type_e is_src_type(size_t index) {
    // ����ע����������ж���ͼƬ�����ֿ�
    if (index < reg_src_image_count) {
        return is_image;
    } else if (index < (reg_src_image_count + reg_src_font_count)) {
        return is_font;
    } else {
        return is_type_max; // ������Χ
        std::cout << "δ֪����Դ���ͣ��±꣺" << index << std::endl;
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
        std::cout << "��ȡ��Դ������bitλ����,����δ֪,�±꣺" << index << std::endl;
        return 0;
    }
}


// ��ȡͼƬ��Դ�����ݴ�С��bit����
size_t get_src_data_size(size_t image_index) {
    if (image_index >= reg_src_image_count) {
        return 0;
    }
    const tImage* img = src_image_array[image_index];
    if (img == nullptr) return 0;
    size_t bit_count = img->width * img->height * img->dataSize; 
    return bit_count;
}

// ��ȡ������ĳ���ַ�ͼ������ݴ�С��bit����
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
    return bit_count;  // �ֽ���
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

        // ��¼��ǰͼƬ��д���ַ
        addr_map[images_name[i]] = offset;
        
        for (size_t j = 0; j < pixel_count; ++j) {
            uint16_t value = data_ptr[j];
            uint8_t low = value & 0xFF;
            uint8_t high = (value >> 8) & 0xFF;
            os.put(low);
            os.put(high);
        }

        offset += (bit_len + 7) / 8;

        std::cout << "ͼƬ����: " << i
                  << " | ͼƬ����: " << images_name[i]
                  << " | ͼƬ��С: " << pixel_count << " ����"
                  << " | ͼƬ��ַ: 0x" << std::hex << std::uppercase << offset << std::dec
                  << std::endl;

        
    }
}

void src_manager_write_font(std::ostream& os, std::map<std::string, size_t>& addr_map, size_t& offset){
    for(size_t i = 0; i < reg_src_font_count; ++i){
        if (i >= src_font_array.size()) continue;
        const tFont* font = src_font_array[i];
        if (!font || !font->chars) continue;
        // ��¼��ǰ�����д���ַ
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
            // ��¼ÿ���ַ�ͼ���д���ַ��key��ʽΪ"������_�ַ�����"
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





/* ���д�������ֽ���ǰ  */
void write_big_endian(std::ofstream& outfile, uint16_t v) {
    uint8_t high = (v >> 8) & 0xFF;
    uint8_t low = v & 0xFF;
    outfile.put(high);
    outfile.put(low);
}

/* С��д�������ֽ���ǰ */
void write_little_endian(std::ofstream& outfile, uint16_t v) {
    uint8_t low = v & 0xFF;
    uint8_t high = (v >> 8) & 0xFF;
    outfile.put(low);
    outfile.put(high);
}
