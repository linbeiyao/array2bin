#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>


#include "src_manager.hpp"



uint16_t reg_src_count = 0;
uint16_t reg_src_image_count = 0;
uint16_t reg_src_font_count = 0;

src_head_t src_head;
std::vector<src_item_t> src_item_array;

// ������Դ��ַ����������Դ�ĵ�ַ��Ϣ��䵽 addr_table��
static void build_src_addr_table(src_item_t& src_item);

// ������Դע��ӿڣ�start_addr ��ѡ��Ĭ��0x0��
static void reg_src_data(void* src, const char* name, is_type_e type, size_t start_addr = 0x0);

// ��ȡ��Դ�����ֽ�������λ��byte��
static size_t get_src_data_datasize(size_t index);

// ��ȡ��Դ����
static is_type_e is_src_type(size_t index);

// ��ȡͼƬ��Դ��λ��С����λ��bit��
static size_t get_src_data_size(const tImage* image);

// ��ȡ������ĳ�ַ�ͼ���λ��С����λ��bit��
static size_t get_src_data_size(const tFont* font, size_t font_image_index);

void src_manager_init() {  
    // ע��Դ����  
    //reg_src_data(&logo_MQ, "logo_MQ", is_image, IMAGE_LOGO_MQ_ADDR);
    //reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font, FONT_MQ2_CN_22_ADDR);
    //reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font, FONT_MQ2_EN_22_ADDR);  
    //reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font, FONT_MQ2_CN_14_ADDR);

    reg_src_data(&logo_MQ, "logo_MQ", is_image);
    reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font);
    reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font);
    reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font);



    // ���յ�ַ��������
    std::sort(src_item_array.begin(), src_item_array.end(), [](const src_item_t& a, const src_item_t& b) {
        return a.satrt_addr < b.satrt_addr;
    });


    printf("Դ����ע����ɣ�ע�����ݸ�����%d��image count:%d font count:%d\n", reg_src_count, reg_src_image_count, reg_src_font_count);


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


void creat_flash_data_head(std::string name, std::map<std::string, size_t>& addr_map){
    // ʹ�� strncpy �����ַ�����ȷ������Խ��
    strncpy_s(src_head.version, name.c_str(), sizeof(src_head.version) - 1);
    src_head.version[sizeof(src_head.version) - 1] = '\0'; // ȷ���� '\0' ��β
    src_head.resource_count = reg_src_count;
    src_head.font_count = reg_src_font_count;
    src_head.image_count = reg_src_image_count;

    // �������ݵ��ܴ�С
    src_head.total_size = 0;
    for(const auto& item : src_item_array){
        src_head.total_size += static_cast<uint32_t>(item.data_byte_count); 
    }
    printf("���ݴ�С��%dB ���� %dKB %dB\n", src_head.total_size, src_head.total_size / 1024, src_head.total_size % 1024);

    // ��ַ�����ʼλ��
    src_head.addr_table_offset = src_item_array.back().end_addr + 1;

    if(src_head.addr_table_offset % 4096 != 0) {
        src_head.addr_table_offset = ((src_head.addr_table_offset / 4096) + 1) * 4096;
    }

    printf("��ַ��ı����ַ��0x%08X\n", src_head.addr_table_offset);

    // �����ַ����ܴ�С  
    // is null


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
 *
 */
void reg_src_data(void* src, const char *name, is_type_e type, size_t start_addr) {
    
    // ȡ���һ����ע�����Դ��
    if (!src_item_array.empty()) {
        const src_item_t& last = src_item_array.back();

        // ����ַ�Ƿ��и���
        if (last.end_addr > start_addr) {
            // ����и���
            start_addr = last.end_addr + 1;   // ������������
        }
    }


    // ��ӵ���Դ����
    src_item_array.push_back({src, type, name, start_addr});

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
            src_item_array.back().end_addr = start_addr + bytes - 1;

            

        } else {    // ���Ϊ��
            src_item_array.back().data_byte_count = 0;
            src_item_array.back().end_addr = start_addr;
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
        src_item_array.back().end_addr = start_addr + total_bytes - 1;
    } else {
        // �������ͣ�������ַ������ʼ��ַ
        src_item_array.back().data_byte_count = 0;
        src_item_array.back().end_addr = start_addr;
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

void src_manager_write_head(std::ostream& os, src_head_t head){
    // д��ǰ�Ƚ��ļ�ָ���ƶ����ļ���ͷ����ַ0��
    os.seekp(0, std::ios::beg);
    // ������ͷ�ṹ�尴�ֽ�д���ļ�
    os.write(reinterpret_cast<const char*>(&head), sizeof(src_head_t));
    if (!os) {
        std::cout << "д������ͷʧ�ܣ�" << std::endl;
    } else {
        std::cout << "����ͷ��д�룬��С: " << sizeof(src_head_t) << " �ֽ�" << std::endl;
    }
}

void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map) {
    size_t current_offset = 0;

    for (size_t i = 0; i < src_item_array.size(); ++i) {        // ��Դ��
        src_item_t& item = src_item_array[i];
        if (!item.data) continue;

        // ��������ĵ�ַ��
        build_src_addr_table(item);

        size_t datasize = get_src_data_datasize(i);
        size_t bit_len = 0;
        const uint16_t* data_ptr = nullptr;
        std::string key = item.name;

        size_t target_addr = item.satrt_addr; // ��Դ�����ʼд���ַ

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

        // ================== ��Ƶ��Դ ==================
        else if (item.type == is_audio) {
            // nothing
        }

        else {
            std::cerr << "δ֪��Դ���ͣ�������" << key << std::endl;
        }
    }
}


void src_manager_write_addr_table(std::ostream& os){
    // ����������ע����Դ��������ַ����Ϣ
    for (const auto& item : src_item_array) {
        // �������Դ���Լ��ĵ�ַ���������ÿ���ַ�����������д��
        if (!item.addr_table.empty()) {
            for (const auto& addr_item : item.addr_table) {
                os.write(reinterpret_cast<const char*>(&addr_item), sizeof(addr_item));
            }
        } else {
            // û���ӵ�ַ�����Դ������һ����ַ����
            src_addr_item_t addr_item;
            memset(&addr_item, 0, sizeof(addr_item));
            // ������� name ���ᳬ��
            strncpy_s(addr_item.name, item.name.c_str(), sizeof(addr_item.name) - 1);
            addr_item.name[sizeof(addr_item.name) - 1] = '\0';
            addr_item.id = 0; // �ɸ���ʵ���������ΨһID
            addr_item.parent_id = 0;
            addr_item.start_addr = static_cast<uint32_t>(item.satrt_addr);
            addr_item.size = static_cast<uint32_t>(item.data_byte_count);
            addr_item.type = static_cast<uint16_t>(item.type);
            addr_item.char_code = 0;
            os.write(reinterpret_cast<const char*>(&addr_item), sizeof(addr_item));
        }
    }
    std::cout << "��Դ��ַ����д���������" << std::endl;
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


// void �ṹ��src_manager_write_head(src_head_t head_data, src){}









///* ���д�������ֽ���ǰ  */
//void write_big_endian(std::ofstream& outfile, uint16_t v) {
//    uint8_t high = (v >> 8) & 0xFF;
//    uint8_t low = v & 0xFF;
//    outfile.put(high);
//    outfile.put(low);
//}
//
///* С��д�������ֽ���ǰ */
//void write_little_endian(std::ofstream& outfile, uint16_t v) {
//    uint8_t low = v & 0xFF;
//    uint8_t high = (v >> 8) & 0xFF;
//    outfile.put(low);
//    outfile.put(high);
//}
//

constexpr const char* OUT_FILE_NAME = "output/MQ2_V1.bin";
constexpr const char* LOG_FILE_NAME = "log/MQ2_V1.txt";

void name_add_time_str(std::string* bin_file_name, std::string* log_file_name) {


    std::time_t now = std::time(nullptr);
    std::tm local_tm = { 0 };
#if defined(_MSC_VER)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif
    char datetime_buf[32] = { 0 };
    std::strftime(datetime_buf, sizeof(datetime_buf), "%Y-%m-%d %H:%M:%S", &local_tm);

    // ���ļ���������ϵ�ǰ����ʱ�䣨�� output/new_demo_20240608_153000.bin��
    char out_file_name_with_time[128] = { 0 };
    std::strftime(datetime_buf, sizeof(datetime_buf), "%Y-%m-%d %H:%M:%S", &local_tm);
    char date_for_file[32] = { 0 };
    std::strftime(date_for_file, sizeof(date_for_file), "_%Y%m%d_%H%M%S", &local_tm);

    std::string base_name = OUT_FILE_NAME;
    size_t dot_pos = base_name.find_last_of('.');
    std::string file_name_with_time;
    if (dot_pos != std::string::npos) {
        file_name_with_time = base_name.substr(0, dot_pos) + date_for_file + base_name.substr(dot_pos);
    }
    else {
        file_name_with_time = base_name + date_for_file;
    }

    // ��־�ļ���Ҳ����ʱ���
    std::string log_base_name = LOG_FILE_NAME;
    size_t log_dot_pos = log_base_name.find_last_of('.');
    std::string log_file_name_with_time;
    if (log_dot_pos != std::string::npos) {
        log_file_name_with_time = log_base_name.substr(0, log_dot_pos) + date_for_file + log_base_name.substr(log_dot_pos);
    }
    else {
        log_file_name_with_time = log_base_name + date_for_file;
    }


    // ����ʱ������ļ���д�ش�����ַ���ָ��
    if (bin_file_name) {
        *bin_file_name = file_name_with_time;
    }
    if (log_file_name) {
        *log_file_name = log_file_name_with_time;
    }


}



/// ������Դ��ַ����������Դ�ĵ�ַ��Ϣ��䵽 addr_table��
void build_src_addr_table(src_item_t& src_item){
    uint32_t addr_item_id = 1;  // ��С��1  0 ��ʾ����
    src_addr_item_t item;

    if(src_item.type == is_image){
        item.id = addr_item_id++;
        item.parent_id = 0;
        item.start_addr = src_item.satrt_addr;
        item.size = src_item.data_byte_count;
        item.type = src_item.type;
        item.char_code = 0;
        strncpy_s(item.name, src_item.name.c_str(), sizeof(item.name) - 1);
        item.name[sizeof(item.name) - 1] = '\0';
        src_item.addr_table.push_back(item);
    }
    else if(src_item.type == is_font){
        // �������ͣ�ÿ���ַ�������һ����ַ����
        const tFont* font = static_cast<const tFont*>(src_item.data);
        if(font && font->chars) {
            for(int i = 0; i < font->length; ++i) {
                const tChar& ch = font->chars[i];
                const tImage* img = ch.image;
                if(img) {
                    src_addr_item_t font_item;
                    font_item.id = addr_item_id++;
                    font_item.parent_id = 0; // �ɸ�����Ҫ���ø�ID
                    // ����ÿ���ַ�����ʼ��ַ
                    size_t char_offset = 0;
                    for(int j = 0; j < i; ++j) {
                        const tImage* prev_img = font->chars[j].image;
                        if(prev_img) {
                            size_t bits = prev_img->width * prev_img->height * prev_img->dataSize;
                            size_t bytes = (bits + 7) / 8;
                            char_offset += bytes;
                        }
                    }
                    font_item.start_addr = src_item.satrt_addr + char_offset;
                    size_t bits = img->width * img->height * img->dataSize;
                    size_t bytes = (bits + 7) / 8;
                    font_item.size = bytes;
                    font_item.type = is_font_char;
                    font_item.char_code = static_cast<uint16_t>(ch.code);
                    // ���Ƹ�ʽ��Font_ԭ������_0xʮ�������ַ�����
                    char hex_code[16];
                    snprintf(hex_code, sizeof(hex_code), "0x%lx", ch.code);
                    std::string char_name = "Font_" + src_item.name + "_" + hex_code;
                    strncpy_s(font_item.name, char_name.c_str(), sizeof(font_item.name) - 1);
                    font_item.name[sizeof(font_item.name) - 1] = '\0';
                    src_item.addr_table.push_back(font_item);
                }
            }
        }
    }

    else if(src_item.type == is_audio){


    }
}