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

// 构建资源地址表（将所有资源的地址信息填充到 addr_table）
static void build_src_addr_table(src_item_t& src_item);

// 单个资源注册接口（start_addr 可选，默认0x0）
static void reg_src_data(void* src, const char* name, is_type_e type, size_t start_addr = 0x0);

// 获取资源数据字节数（单位：byte）
static size_t get_src_data_datasize(size_t index);

// 获取资源类型
static is_type_e is_src_type(size_t index);

// 获取图片资源的位大小（单位：bit）
static size_t get_src_data_size(const tImage* image);

// 获取字体中某字符图像的位大小（单位：bit）
static size_t get_src_data_size(const tFont* font, size_t font_image_index);

void src_manager_init() {  
    // 注册源数据  
    //reg_src_data(&logo_MQ, "logo_MQ", is_image, IMAGE_LOGO_MQ_ADDR);
    //reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font, FONT_MQ2_CN_22_ADDR);
    //reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font, FONT_MQ2_EN_22_ADDR);  
    //reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font, FONT_MQ2_CN_14_ADDR);

    reg_src_data(&logo_MQ, "logo_MQ", is_image);
    reg_src_data(&Font_MQ2_CN_22, "MQ2_CN_22", is_font);
    reg_src_data(&Font_MQ2_EN_22, "MQ2_EN_22", is_font);
    reg_src_data(&Font_MQ2_CN_14, "MQ2_CN_14", is_font);



    // 按照地址进行排序
    std::sort(src_item_array.begin(), src_item_array.end(), [](const src_item_t& a, const src_item_t& b) {
        return a.satrt_addr < b.satrt_addr;
    });


    printf("源数据注册完成，注册数据个数：%d，image count:%d font count:%d\n", reg_src_count, reg_src_image_count, reg_src_font_count);


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


void creat_flash_data_head(std::string name, std::map<std::string, size_t>& addr_map){
    // 使用 strncpy 拷贝字符串，确保不会越界
    strncpy_s(src_head.version, name.c_str(), sizeof(src_head.version) - 1);
    src_head.version[sizeof(src_head.version) - 1] = '\0'; // 确保以 '\0' 结尾
    src_head.resource_count = reg_src_count;
    src_head.font_count = reg_src_font_count;
    src_head.image_count = reg_src_image_count;

    // 计算数据的总大小
    src_head.total_size = 0;
    for(const auto& item : src_item_array){
        src_head.total_size += static_cast<uint32_t>(item.data_byte_count); 
    }
    printf("数据大小：%dB 换算 %dKB %dB\n", src_head.total_size, src_head.total_size / 1024, src_head.total_size % 1024);

    // 地址表的起始位置
    src_head.addr_table_offset = src_item_array.back().end_addr + 1;

    if(src_head.addr_table_offset % 4096 != 0) {
        src_head.addr_table_offset = ((src_head.addr_table_offset / 4096) + 1) * 4096;
    }

    printf("地址表的保存地址：0x%08X\n", src_head.addr_table_offset);

    // 计算地址表的总大小  
    // is null


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
 *
 */
void reg_src_data(void* src, const char *name, is_type_e type, size_t start_addr) {
    
    // 取最后一个已注册的资源项
    if (!src_item_array.empty()) {
        const src_item_t& last = src_item_array.back();

        // 检测地址是否有覆盖
        if (last.end_addr > start_addr) {
            // 如果有覆盖
            start_addr = last.end_addr + 1;   // 依次往后排序
        }
    }


    // 添加到资源数组
    src_item_array.push_back({src, type, name, start_addr});

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
            src_item_array.back().end_addr = start_addr + bytes - 1;

            

        } else {    // 如果为空
            src_item_array.back().data_byte_count = 0;
            src_item_array.back().end_addr = start_addr;
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
        src_item_array.back().end_addr = start_addr + total_bytes - 1;
    } else {
        // 其他类型，结束地址等于起始地址
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

void src_manager_write_head(std::ostream& os, src_head_t head){
    // 写入前先将文件指针移动到文件开头（地址0）
    os.seekp(0, std::ios::beg);
    // 将数据头结构体按字节写入文件
    os.write(reinterpret_cast<const char*>(&head), sizeof(src_head_t));
    if (!os) {
        std::cout << "写入数据头失败！" << std::endl;
    } else {
        std::cout << "数据头已写入，大小: " << sizeof(src_head_t) << " 字节" << std::endl;
    }
}

void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map) {
    size_t current_offset = 0;

    for (size_t i = 0; i < src_item_array.size(); ++i) {        // 资源项
        src_item_t& item = src_item_array[i];
        if (!item.data) continue;

        // 构建该项的地址表
        build_src_addr_table(item);

        size_t datasize = get_src_data_datasize(i);
        size_t bit_len = 0;
        const uint16_t* data_ptr = nullptr;
        std::string key = item.name;

        size_t target_addr = item.satrt_addr; // 资源项的起始写入地址

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

        // ================== 音频资源 ==================
        else if (item.type == is_audio) {
            // nothing
        }

        else {
            std::cerr << "未知资源类型，跳过：" << key << std::endl;
        }
    }
}


void src_manager_write_addr_table(std::ostream& os){
    // 遍历所有已注册资源，输出其地址表信息
    for (const auto& item : src_item_array) {
        // 如果该资源有自己的地址表（如字体的每个字符），则逐项写入
        if (!item.addr_table.empty()) {
            for (const auto& addr_item : item.addr_table) {
                os.write(reinterpret_cast<const char*>(&addr_item), sizeof(addr_item));
            }
        } else {
            // 没有子地址表的资源，构造一个地址表项
            src_addr_item_t addr_item;
            memset(&addr_item, 0, sizeof(addr_item));
            // 这里假设 name 不会超长
            strncpy_s(addr_item.name, item.name.c_str(), sizeof(addr_item.name) - 1);
            addr_item.name[sizeof(addr_item.name) - 1] = '\0';
            addr_item.id = 0; // 可根据实际需求分配唯一ID
            addr_item.parent_id = 0;
            addr_item.start_addr = static_cast<uint32_t>(item.satrt_addr);
            addr_item.size = static_cast<uint32_t>(item.data_byte_count);
            addr_item.type = static_cast<uint16_t>(item.type);
            addr_item.char_code = 0;
            os.write(reinterpret_cast<const char*>(&addr_item), sizeof(addr_item));
        }
    }
    std::cout << "资源地址表已写入输出流。" << std::endl;
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


// void 结构体src_manager_write_head(src_head_t head_data, src){}









///* 大端写法：高字节在前  */
//void write_big_endian(std::ofstream& outfile, uint16_t v) {
//    uint8_t high = (v >> 8) & 0xFF;
//    uint8_t low = v & 0xFF;
//    outfile.put(high);
//    outfile.put(low);
//}
//
///* 小端写法：低字节在前 */
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

    // 在文件名后面加上当前日期时间（如 output/new_demo_20240608_153000.bin）
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

    // 日志文件名也加上时间戳
    std::string log_base_name = LOG_FILE_NAME;
    size_t log_dot_pos = log_base_name.find_last_of('.');
    std::string log_file_name_with_time;
    if (log_dot_pos != std::string::npos) {
        log_file_name_with_time = log_base_name.substr(0, log_dot_pos) + date_for_file + log_base_name.substr(log_dot_pos);
    }
    else {
        log_file_name_with_time = log_base_name + date_for_file;
    }


    // 将带时间戳的文件名写回传入的字符串指针
    if (bin_file_name) {
        *bin_file_name = file_name_with_time;
    }
    if (log_file_name) {
        *log_file_name = log_file_name_with_time;
    }


}



/// 构建资源地址表（将所有资源的地址信息填充到 addr_table）
void build_src_addr_table(src_item_t& src_item){
    uint32_t addr_item_id = 1;  // 最小是1  0 表示顶级
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
        // 字体类型，每个字符都生成一个地址表项
        const tFont* font = static_cast<const tFont*>(src_item.data);
        if(font && font->chars) {
            for(int i = 0; i < font->length; ++i) {
                const tChar& ch = font->chars[i];
                const tImage* img = ch.image;
                if(img) {
                    src_addr_item_t font_item;
                    font_item.id = addr_item_id++;
                    font_item.parent_id = 0; // 可根据需要设置父ID
                    // 计算每个字符的起始地址
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
                    // 名称格式：Font_原字体名_0x十六进制字符编码
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