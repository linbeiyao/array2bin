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
    // 注册源数据  
    reg_src_data(&logo_MQ, "logo_MQ", is_image, IMAGE_LOGO_MQ_ADDR);
    reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font, FONT_MQ2_CN_22_ADDR);
    reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font, FONT_MQ2_EN_22_ADDR);  
    reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font, FONT_MQ2_CN_14_ADDR);

    // 按照地址进行排序
    std::sort(src_item_array.begin(), src_item_array.end(), [](const src_item_t& a, const src_item_t& b) {
        return a.satrt_addr < b.satrt_addr;
    });


    

    std::cout << "源数据注册完成" << std::endl
        << "注册源数据个数：" << reg_src_count << std::endl
        << "注册图片个数：" << reg_src_image_count << std::endl
        << "注册字体个数：" << reg_src_font_count << std::endl;

    // 输出每个资源的地址和大小
    std::cout << "资源地址及大小列表：" << std::endl;
    for (const auto& item : src_item_array) {
        std::cout << "名称: " << item.name
                  << " | 开始地址: 0x" << std::hex << std::uppercase << item.satrt_addr
                  << " | 结束地址: 0x" << std::hex << std::uppercase << item.end_addr
                  << " | 大小: " << std::dec << item.data_byte_count << " 字节"
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
        std::cerr << "资源类型查询失败：索引越界 " << index << std::endl;
        return is_type_max;
    }
    return src_item_array[index].type;
}

/**
 * addr 如果指定地址 也可以默认顺序存储
 * 仅添加功能，不影响兼容性
 */
void reg_src_data(void* src, const char *name, is_type_e type, size_t addr = 0x00000000) {
    // 添加到资源数组
    src_item_array.push_back({src, type, name, addr});

    // 区分图片和字体的注册计数，并计算字节数（向上取整）
    if (type == is_image) {
        reg_src_image_count++;
        // 计算图片字节数，向上取整
        const tImage* img = static_cast<const tImage*>(src);
        if(img) {
            size_t bits = (img->width) * (img->height) * (img->dataSize);
            size_t bytes = (bits + 7) / 8; // 向上取整
            src_item_array.back().data_byte_count = bytes;
            // 计算结束地址
            src_item_array.back().end_addr = addr + bytes - 1;
        } else {
            src_item_array.back().data_byte_count = 0;
            src_item_array.back().end_addr = addr;
        }
    } else if (type == is_font) {
        reg_src_font_count++;
        // 计算字体字节数（所有字符图片的总和，向上取整）
        const tFont* font = static_cast<const tFont*>(src);
        size_t total_bytes = 0;
        if(font && font->chars) {
            for(int i = 0; i < font->length; ++i) {
                const tImage* img = font->chars[i].image;
                if(img) {
                    size_t bits = (img->width) * (img->height) * (img->dataSize);
                    size_t bytes = (bits + 7) / 8; // 向上取整
                    total_bytes += bytes;
                }
            }
        }
        src_item_array.back().data_byte_count = total_bytes;
        // 计算结束地址
        src_item_array.back().end_addr = addr + total_bytes - 1;
    } else {
        // 其他类型，结束地址等于起始地址
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
        std::cout << "获取资源的像素bit位出错,下标越界：" << index << std::endl;
        return 0;
    }
    const src_item_t& item = src_item_array[index];
    if(item.type == is_image){
        const tImage* img = static_cast<const tImage*>(item.data);
        if(img) {
            return img->dataSize;
        } else {
            std::cout << "图片资源为空, 下标：" << index << std::endl;
            return 0;
        }
    } else if(item.type == is_font){
        const tFont* font = static_cast<const tFont*>(item.data);
        if(font && font->length > 0 && font->chars && font->chars[0].image) {
            return font->chars[0].image->dataSize;
        } else {
            std::cout << "字体资源为空或无字符, 下标：" << index << std::endl;
            return 0;
        }
    } else {
        std::cout << "获取资源的像素bit位出错,类型未知,下标：" << index << std::endl;
        return 0;
    }
}


// 获取资源的数据大小（bit数）
size_t get_src_data_size(const tImage *image) {
    if (image == nullptr) {
        std::cout << "图片资源为空" << std::endl;
        return 0;
    }
    size_t bit_count = image->width * image->height * image->dataSize;
    return bit_count;
}

// 获取字体中某个字符图像的数据大小（bit数）
// 直接传入字库指针和字符索引
size_t get_src_data_size(const tFont* font, size_t font_image_index) {
    if (font == nullptr) {
        std::cout << "字体指针为空" << std::endl;
        return 0;
    }
    if (font_image_index >= (size_t)font->length) {
        std::cout << "字符索引超出范围: " << font_image_index << std::endl;
        return 0;
    }
    const tImage* img = font->chars[font_image_index].image;
    if (img == nullptr) {
        std::cout << "字符图像为空, 索引: " << font_image_index << std::endl;
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

        size_t target_addr = item.satrt_addr; // 资源的起始写入地址

        // 如果当前写入偏移 < 目标地址，填充 0xFF
        if (current_offset < target_addr) {
            size_t gap = target_addr - current_offset;
            for (size_t g = 0; g < gap; ++g) {
                os.put((char)0xFF);
            }
            current_offset = target_addr;
        }

        // ================== 图像资源 ==================
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

            std::cout << "写入图像：" << key
                << " | 大小：" << (bit_len + 7) / 8 << " 字节"
                << " | 地址: 0x" << std::hex << std::uppercase << target_addr << std::dec
                << std::endl;
        }

        // ================== 字体资源 ==================
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

                std::cout << "写入字体：" << char_key
                    << " | 大小：" << (bit_len + 7) / 8 << " 字节"
                    << " | 地址: 0x" << std::hex << std::uppercase << addr_map[char_key] << std::dec
                    << std::endl;
            }
        }

        else {
            std::cerr << "未知资源类型，跳过：" << key << std::endl;
        }
    }
}

// 数据验证函数，返回 true 表示校验通过，false 表示有错误
bool src_manager_verify_data() {
    bool valid = true;

    // 检查资源数量
    if (src_item_array.size() != reg_src_count) {
        std::cerr << "资源数量校验失败：数组实际数量(" << src_item_array.size()
                  << ") != 注册数量(" << reg_src_count << ")" << std::endl;
        valid = false;
    }

    // 检查图片和字体数量
    size_t image_count = 0;
    size_t font_count = 0;
    for (const auto& item : src_item_array) {
        if (item.type == is_image) image_count++;
        if (item.type == is_font) font_count++;
    }
    if (image_count != reg_src_image_count) {
        std::cerr << "图片数量校验失败：实际(" << image_count
                  << ") != 注册(" << reg_src_image_count << ")" << std::endl;
        valid = false;
    }
    if (font_count != reg_src_font_count) {
        std::cerr << "字体数量校验失败：实际(" << font_count
                  << ") != 注册(" << reg_src_font_count << ")" << std::endl;
        valid = false;
    }

    // 检查每个资源的指针和数据大小
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        if (item.data == nullptr) {
            std::cerr << "资源[" << i << "] 名称: " << item.name << " 数据指针为空" << std::endl;
            valid = false;
        }
        if (item.data_byte_count == 0) {
            std::cerr << "资源[" << i << "] 名称: " << item.name << " 数据大小为0" << std::endl;
            valid = false;
        }
        // 针对图片和字体做更细致的校验
        if (item.type == is_image) {
            const tImage* img = static_cast<const tImage*>(item.data);
            if (!img || !img->data) {
                std::cerr << "图片资源[" << i << "] 名称: " << item.name << " 图像数据指针为空" << std::endl;
                valid = false;
            }
            if (img && (img->width == 0 || img->height == 0)) {
                std::cerr << "图片资源[" << i << "] 名称: " << item.name << " 宽或高为0" << std::endl;
                valid = false;
            }
        } else if (item.type == is_font) {
            const tFont* font = static_cast<const tFont*>(item.data);
            if (!font || !font->chars) {
                std::cerr << "字体资源[" << i << "] 名称: " << item.name << " 字符数组指针为空" << std::endl;
                valid = false;
            }
            if (font && font->length == 0) {
                std::cerr << "字体资源[" << i << "] 名称: " << item.name << " 字符数量为0" << std::endl;
                valid = false;
            }
            // 检查每个字符的图像指针
            if (font && font->chars) {
                for (int j = 0; j < font->length; ++j) {
                    const tImage* img = font->chars[j].image;
                    if (!img || !img->data) {
                        std::cerr << "字体资源[" << i << "] 名称: " << item.name
                                  << " 字符[" << j << "] 图像数据指针为空" << std::endl;
                        valid = false;
                    }
                }
            }
        }
    }


     // 1. 地址升序检查 & 重叠检查
    for (size_t i = 1; i < src_item_array.size(); ++i) {
        const auto& prev = src_item_array[i - 1];
        const auto& curr = src_item_array[i];

        size_t prev_end = prev.satrt_addr + prev.data_byte_count;
        if (prev_end > curr.satrt_addr) {
            std::cerr << "地址重叠错误：资源 [" << prev.name << "] 与 [" << curr.name << "]"
                      << " 存在交叉，"
                      << "地址范围：0x" << std::hex << prev.satrt_addr << "-0x" << (prev_end - 1)
                      << " 与 0x" << curr.satrt_addr << std::dec << std::endl;
            valid = false;
        }
    }

    // 2. 图像数据大小是否等于宽*高*dataSize/8
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        if (item.type == is_image) {
            const tImage* img = static_cast<const tImage*>(item.data);
            if (!img || !img->data) continue;

            size_t calc_bytes = (img->width * img->height * img->dataSize + 7) / 8;
            if (calc_bytes != item.data_byte_count) {
                std::cerr << "图片资源 [" << item.name << "] 的实际数据大小与计算不一致，"
                          << "应为 " << calc_bytes << " 字节，但注册为 " << item.data_byte_count << " 字节" << std::endl;
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
                    std::cerr << "字体 [" << item.name << "] 字符 [" << j << "] 数据大小为 0" << std::endl;
                    valid = false;
                }
            }
            if (sum_bytes != item.data_byte_count) {
                std::cerr << "字体 [" << item.name << "] 的实际字符图像总和为 " << sum_bytes
                          << " 字节，注册为 " << item.data_byte_count << " 字节" << std::endl;
                valid = false;
            }
        }
    }

    // 3. 超出 Flash 地址范围（24-bit最大支持到 0xFFFFFF）
    for (size_t i = 0; i < src_item_array.size(); ++i) {
        const auto& item = src_item_array[i];
        size_t end_addr = item.satrt_addr + item.data_byte_count;
        if (end_addr > 0xFFFFFF) {
            std::cerr << "资源 [" << item.name << "] 地址溢出 24bit Flash 范围，结束地址为 0x"
                      << std::hex << end_addr << std::dec << std::endl;
            valid = false;
        }
    }

    if (valid) {
        std::cout << "资源数据校验通过" << std::endl;
    } else {
        std::cout << "资源数据校验失败" << std::endl;
    }
    return valid;
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
