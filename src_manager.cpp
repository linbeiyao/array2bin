#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>


#include "src_manager.hpp"


uint16_t reg_src_count = 0;
uint16_t reg_src_image_count = 0;
uint16_t reg_src_font_count = 0;




std::vector<src_item_t> src_item_array;





#define IMAGE_LOGO_MQ_ADDR 0x00000000
#define FONT_MQ2_CN_22_ADDR 0x00005000
#define FONT_MQ2_EN_22_ADDR 0x0000E000
#define FONT_MQ2_CN_14_ADDR 0x00015000


void src_manager_init() {  
    // ע��Դ����  
    reg_src_data(&logo_MQ, "logo_MQ", is_image, IMAGE_LOGO_MQ_ADDR);
    reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font, FONT_MQ2_CN_22_ADDR);
    reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font, FONT_MQ2_EN_22_ADDR);  
    reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font, FONT_MQ2_CN_14_ADDR);

    // ���յ�ַ��������
    std::sort(src_item_array.begin(), src_item_array.end(), [](const src_item_t& a, const src_item_t& b) {
        return a.satrt_addr < b.satrt_addr;
    });


    

    std::cout << "Դ����ע�����" << std::endl
        << "ע��Դ���ݸ�����" << reg_src_count << std::endl
        << "ע��ͼƬ������" << reg_src_image_count << std::endl
        << "ע�����������" << reg_src_font_count << std::endl;

    // ���ÿ����Դ�ĵ�ַ�ʹ�С
    std::cout << "��Դ��ַ����С�б�" << std::endl;
    for (const auto& item : src_item_array) {
        std::cout << "����: " << item.name
                  << " | ��ʼ��ַ: 0x" << std::hex << std::uppercase << item.satrt_addr
                  << " | ������ַ: 0x" << std::hex << std::uppercase << item.end_addr
                  << " | ��С: " << std::dec << item.data_byte_count << " �ֽ�"
                  << std::endl;
    }
}  

void src_manager_deinit(){
    src_item_array.clear();
    reg_src_count = 0;
    reg_src_image_count = 0;
    reg_src_font_count = 0;
}

is_type_e is_src_type(size_t index) {
    if (index >= src_item_array.size()) {
        std::cerr << "��Դ���Ͳ�ѯʧ�ܣ�����Խ�� " << index << std::endl;
        return is_type_max;
    }
    return src_item_array[index].type;
}

/**
 * addr ���ָ����ַ Ҳ����Ĭ��˳��洢
 * ����ӹ��ܣ���Ӱ�������
 */
void reg_src_data(void* src, const char *name, is_type_e type, size_t addr = 0x00000000) {
    // ��ӵ���Դ����
    src_item_array.push_back({src, type, name, addr});

    // ����ͼƬ�������ע��������������ֽ���������ȡ����
    if (type == is_image) {
        reg_src_image_count++;
        // ����ͼƬ�ֽ���������ȡ��
        const tImage* img = static_cast<const tImage*>(src);
        if(img) {
            size_t bits = (img->width) * (img->height) * (img->dataSize);
            size_t bytes = (bits + 7) / 8; // ����ȡ��
            src_item_array.back().data_byte_count = bytes;
            // ���������ַ
            src_item_array.back().end_addr = addr + bytes - 1;
        } else {
            src_item_array.back().data_byte_count = 0;
            src_item_array.back().end_addr = addr;
        }
    } else if (type == is_font) {
        reg_src_font_count++;
        // ���������ֽ����������ַ�ͼƬ���ܺͣ�����ȡ����
        const tFont* font = static_cast<const tFont*>(src);
        size_t total_bytes = 0;
        if(font && font->chars) {
            for(int i = 0; i < font->length; ++i) {
                const tImage* img = font->chars[i].image;
                if(img) {
                    size_t bits = (img->width) * (img->height) * (img->dataSize);
                    size_t bytes = (bits + 7) / 8; // ����ȡ��
                    total_bytes += bytes;
                }
            }
        }
        src_item_array.back().data_byte_count = total_bytes;
        // ���������ַ
        src_item_array.back().end_addr = addr + total_bytes - 1;
    } else {
        // �������ͣ�������ַ������ʼ��ַ
        src_item_array.back().data_byte_count = 0;
        src_item_array.back().end_addr = addr;
    }
    reg_src_count++;
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
    if (index >= src_item_array.size()) {
        std::cout << "��ȡ��Դ������bitλ����,�±�Խ�磺" << index << std::endl;
        return 0;
    }
    const src_item_t& item = src_item_array[index];
    if(item.type == is_image){
        const tImage* img = static_cast<const tImage*>(item.data);
        if(img) {
            return img->dataSize;
        } else {
            std::cout << "ͼƬ��ԴΪ��, �±꣺" << index << std::endl;
            return 0;
        }
    } else if(item.type == is_font){
        const tFont* font = static_cast<const tFont*>(item.data);
        if(font && font->length > 0 && font->chars && font->chars[0].image) {
            return font->chars[0].image->dataSize;
        } else {
            std::cout << "������ԴΪ�ջ����ַ�, �±꣺" << index << std::endl;
            return 0;
        }
    } else {
        std::cout << "��ȡ��Դ������bitλ����,����δ֪,�±꣺" << index << std::endl;
        return 0;
    }
}


// ��ȡ��Դ�����ݴ�С��bit����
size_t get_src_data_size(const tImage *image) {
    if (image == nullptr) {
        std::cout << "ͼƬ��ԴΪ��" << std::endl;
        return 0;
    }
    size_t bit_count = image->width * image->height * image->dataSize;
    return bit_count;
}

// ��ȡ������ĳ���ַ�ͼ������ݴ�С��bit����
// ֱ�Ӵ����ֿ�ָ����ַ�����
size_t get_src_data_size(const tFont* font, size_t font_image_index) {
    if (font == nullptr) {
        std::cout << "����ָ��Ϊ��" << std::endl;
        return 0;
    }
    if (font_image_index >= (size_t)font->length) {
        std::cout << "�ַ�����������Χ: " << font_image_index << std::endl;
        return 0;
    }
    const tImage* img = font->chars[font_image_index].image;
    if (img == nullptr) {
        std::cout << "�ַ�ͼ��Ϊ��, ����: " << font_image_index << std::endl;
        return 0;
    }
    size_t bit_count = img->width * img->height * img->dataSize;
    return bit_count;
}

void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map) {
    size_t current_offset = 0;

    for (size_t i = 0; i < src_item_array.size(); ++i) {
        src_item_t& item = src_item_array[i];
        if (!item.data) continue;

        size_t datasize = get_src_data_datasize(i);
        size_t bit_len = 0;
        const uint16_t* data_ptr = nullptr;
        std::string key = item.name;

        size_t target_addr = item.satrt_addr; // ��Դ����ʼд���ַ

        // �����ǰд��ƫ�� < Ŀ���ַ����� 0xFF
        if (current_offset < target_addr) {
            size_t gap = target_addr - current_offset;
            for (size_t g = 0; g < gap; ++g) {
                os.put((char)0xFF);
            }
            current_offset = target_addr;
        }

        // ================== ͼ����Դ ==================
        if (item.type == is_image) {
            const tImage* img = static_cast<const tImage*>(item.data);
            if (!img || !img->data) continue;
            bit_len = get_src_data_size(img);
            data_ptr = img->data;

            size_t pixel_count = bit_len / datasize;
            for (size_t j = 0; j < pixel_count; ++j) {
                uint16_t value = data_ptr[j];
                os.put(static_cast<char>(value & 0xFF));
                os.put(static_cast<char>((value >> 8) & 0xFF));
                current_offset += 2;
            }

            addr_map[key] = target_addr;

            std::cout << "д��ͼ��" << key
                << " | ��С��" << (bit_len + 7) / 8 << " �ֽ�"
                << " | ��ַ: 0x" << std::hex << std::uppercase << target_addr << std::dec
                << std::endl;
        }

        // ================== ������Դ ==================
        else if (item.type == is_font) {
            const tFont* font = static_cast<const tFont*>(item.data);
            if (!font || !font->chars) continue;

            for (int j = 0; j < font->length; ++j) {
                const tImage* img = font->chars[j].image;
                if (!img || !img->data) continue;

                bit_len = get_src_data_size(font, j);
                datasize = img->dataSize;
                size_t pixel_count = bit_len / datasize;
                data_ptr = img->data;

                std::string char_key = key + "_" + std::to_string(font->chars[j].code);
                addr_map[char_key] = current_offset;

                for (size_t k = 0; k < pixel_count; ++k) {
                    uint16_t value = data_ptr[k];
                    os.put(static_cast<char>(value & 0xFF));
                    os.put(static_cast<char>((value >> 8) & 0xFF));
                    current_offset += 2;
                }

                std::cout << "д�����壺" << char_key
                    << " | ��С��" << (bit_len + 7) / 8 << " �ֽ�"
                    << " | ��ַ: 0x" << std::hex << std::uppercase << addr_map[char_key] << std::dec
                    << std::endl;
            }
        }

        else {
            std::cerr << "δ֪��Դ���ͣ�������" << key << std::endl;
        }
    }
}

// ������֤���������� true ��ʾУ��ͨ����false ��ʾ�д���
bool src_manager_verify_data() {
    bool valid = true;

    // �����Դ����
    if (src_item_array.size() != reg_src_count) {
        std::cerr << "��Դ����У��ʧ�ܣ�����ʵ������(" << src_item_array.size()
                  << ") != ע������(" << reg_src_count << ")" << std::endl;
        valid = false;
    }

    // ���ͼƬ����������
    size_t image_count = 0;
    size_t font_count = 0;
    for (const auto& item : src_item_array) {
        if (item.type == is_image) image_count++;
        if (item.type == is_font) font_count++;
    }
    if (image_count != reg_src_image_count) {
        std::cerr << "ͼƬ����У��ʧ�ܣ�ʵ��(" << image_count
                  << ") != ע��(" << reg_src_image_count << ")" << std::endl;
        valid = false;
    }
    if (font_count != reg_src_font_count) {
        std::cerr << "��������У��ʧ�ܣ�ʵ��(" << font_count
                  << ") != ע��(" << reg_src_font_count << ")" << std::endl;
        valid = false;
    }

    // ���ÿ����Դ��ָ������ݴ�С
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        if (item.data == nullptr) {
            std::cerr << "��Դ[" << i << "] ����: " << item.name << " ����ָ��Ϊ��" << std::endl;
            valid = false;
        }
        if (item.data_byte_count == 0) {
            std::cerr << "��Դ[" << i << "] ����: " << item.name << " ���ݴ�СΪ0" << std::endl;
            valid = false;
        }
        // ���ͼƬ����������ϸ�µ�У��
        if (item.type == is_image) {
            const tImage* img = static_cast<const tImage*>(item.data);
            if (!img || !img->data) {
                std::cerr << "ͼƬ��Դ[" << i << "] ����: " << item.name << " ͼ������ָ��Ϊ��" << std::endl;
                valid = false;
            }
            if (img && (img->width == 0 || img->height == 0)) {
                std::cerr << "ͼƬ��Դ[" << i << "] ����: " << item.name << " ����Ϊ0" << std::endl;
                valid = false;
            }
        } else if (item.type == is_font) {
            const tFont* font = static_cast<const tFont*>(item.data);
            if (!font || !font->chars) {
                std::cerr << "������Դ[" << i << "] ����: " << item.name << " �ַ�����ָ��Ϊ��" << std::endl;
                valid = false;
            }
            if (font && font->length == 0) {
                std::cerr << "������Դ[" << i << "] ����: " << item.name << " �ַ�����Ϊ0" << std::endl;
                valid = false;
            }
            // ���ÿ���ַ���ͼ��ָ��
            if (font && font->chars) {
                for (int j = 0; j < font->length; ++j) {
                    const tImage* img = font->chars[j].image;
                    if (!img || !img->data) {
                        std::cerr << "������Դ[" << i << "] ����: " << item.name
                                  << " �ַ�[" << j << "] ͼ������ָ��Ϊ��" << std::endl;
                        valid = false;
                    }
                }
            }
        }
    }


     // 1. ��ַ������ & �ص����
    for (size_t i = 1; i < src_item_array.size(); ++i) {
        const auto& prev = src_item_array[i - 1];
        const auto& curr = src_item_array[i];

        size_t prev_end = prev.satrt_addr + prev.data_byte_count;
        if (prev_end > curr.satrt_addr) {
            std::cerr << "��ַ�ص�������Դ [" << prev.name << "] �� [" << curr.name << "]"
                      << " ���ڽ��棬"
                      << "��ַ��Χ��0x" << std::hex << prev.satrt_addr << "-0x" << (prev_end - 1)
                      << " �� 0x" << curr.satrt_addr << std::dec << std::endl;
            valid = false;
        }
    }

    // 2. ͼ�����ݴ�С�Ƿ���ڿ�*��*dataSize/8
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        if (item.type == is_image) {
            const tImage* img = static_cast<const tImage*>(item.data);
            if (!img || !img->data) continue;

            size_t calc_bytes = (img->width * img->height * img->dataSize + 7) / 8;
            if (calc_bytes != item.data_byte_count) {
                std::cerr << "ͼƬ��Դ [" << item.name << "] ��ʵ�����ݴ�С����㲻һ�£�"
                          << "ӦΪ " << calc_bytes << " �ֽڣ���ע��Ϊ " << item.data_byte_count << " �ֽ�" << std::endl;
                valid = false;
            }
        } else if (item.type == is_font) {
            const tFont* font = static_cast<const tFont*>(item.data);
            if (!font || !font->chars) continue;

            size_t sum_bytes = 0;
            for (int j = 0; j < font->length; ++j) {
                const tImage* img = font->chars[j].image;
                if (!img || !img->data) continue;

                size_t calc_bytes = (img->width * img->height * img->dataSize + 7) / 8;
                sum_bytes += calc_bytes;

                if (calc_bytes == 0) {
                    std::cerr << "���� [" << item.name << "] �ַ� [" << j << "] ���ݴ�СΪ 0" << std::endl;
                    valid = false;
                }
            }
            if (sum_bytes != item.data_byte_count) {
                std::cerr << "���� [" << item.name << "] ��ʵ���ַ�ͼ���ܺ�Ϊ " << sum_bytes
                          << " �ֽڣ�ע��Ϊ " << item.data_byte_count << " �ֽ�" << std::endl;
                valid = false;
            }
        }
    }

    // 3. ���� Flash ��ַ��Χ��24-bit���֧�ֵ� 0xFFFFFF��
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        size_t end_addr = item.satrt_addr + item.data_byte_count;
        if (end_addr > 0xFFFFFF) {
            std::cerr << "��Դ [" << item.name << "] ��ַ��� 24bit Flash ��Χ��������ַΪ 0x"
                      << std::hex << end_addr << std::dec << std::endl;
            valid = false;
        }
    }

    if (valid) {
        std::cout << "��Դ����У��ͨ��" << std::endl;
    } else {
        std::cout << "��Դ����У��ʧ��" << std::endl;
    }
    return valid;
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
