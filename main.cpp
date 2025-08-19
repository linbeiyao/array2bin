#include <fstream>
#include <cstdint>
#include <iostream>
#include "src_manager.hpp"
#include <map>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>


// ┌───────────────────────────────┐
// │         数据头 src_head_t      │
// ├───────────────────────────────┤
// │         资源数据区            │
// │ ┌─────────────┐              │
// │ │ 图像数据    │              │
// │ ├─────────────┤              │
// │ │ 字体数据    │              │
// │ ├─────────────┤              │
// │ │ ...         │              │
// │ └─────────────┘              │
// ├───────────────────────────────┤
// │         地址表区              │
// │ ┌─────────────┐              │
// │ │ src_addr_item_t[] │        │
// │ └─────────────┘              │
// └───────────────────────────────┘
//
// 结构说明：
// 1. 文件起始为 src_head_t 结构体，包含版本、资源数量、总大小等信息。
// 2. 紧接其后为所有资源的原始数据（图像、字体等），按注册顺序或地址排序存放。
// 3. 最后为资源地址表（src_addr_item_t 数组），记录每个资源的起始地址、大小、类型等元数据。


std::map<std::string, size_t> src_addr_map;



int main() {
    std::string bin_file_name, log_file_name;

    name_add_time_str(&bin_file_name, &log_file_name);
    std::cout << "二进制文件名: " << bin_file_name << " 日志文件名:" << log_file_name << std::endl;

    src_manager_init();


    
    // 等待按下回车开始制作文件  
    // 等待按下回车或输入 'q'  
    printf("请按回车开始制作文件，或输入 'q' 退出程序...");  
    std::string input;  
    std::getline(std::cin, input);  

    if (input == "q") {  
        printf("程序已退出。\n");  
        exit(0);  
    } else if (input.empty()) {  
        printf("继续执行...\n");  
        return;  
    } else {  
        printf("无效输入，请重新运行程序。\n");  
        exit(1);  
    }

    // 制作存储芯片的数据头
    creat_flash_data_head(bin_file_name, src_addr_map);


    std::ofstream outfile(bin_file_name, std::ios::binary);




    src_manager_write_head(outfile, src_head);
    src_manager_write_data(outfile, src_addr_map);
    src_manager_write_addr_table(outfile);


    src_manager_verify_data();




    outfile.close();

    std::cout << "Bin file out Done!" << std::endl;

    // 将 map 拷贝到 vector 并按地址排序
    std::vector<std::pair<std::string, size_t>> sorted_addr(src_addr_map.begin(), src_addr_map.end());
    std::sort(sorted_addr.begin(), sorted_addr.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });

    // 输出地址表到文件（十六进制，按地址顺序）
    std::ofstream addrfile(log_file_name, std::ios::out);
    if (addrfile.is_open()) {
        for (const auto& kv : sorted_addr) {
            addrfile << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;
        }
        addrfile.close();
        std::cout << "地址表已经输出 " << log_file_name << std::endl;
    }
    else {
        std::cout << "无法打开 " << log_file_name << "文件进行写入" << std::endl;
    }

    // 同时在控制台输出（十六进制，按地址顺序）
    for (const auto& kv : sorted_addr)
        std::cout << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;

    return 0;
}
