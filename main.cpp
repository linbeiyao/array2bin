#include <fstream>
#include <cstdint>
#include <iostream>
#include "src_manager.hpp"
#include <map>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

constexpr const char* OUT_FILE_NAME = "demo.bin";


extern std::vector<std::string> images_name;
extern std::vector<std::string> fonts_name;

int main() {
    src_manager_init();

    std::ofstream outfile(OUT_FILE_NAME, std::ios::binary);

    std::map<std::string, size_t> src_addr_map;

    size_t offset = 0;
    src_manager_write_image(outfile, src_addr_map, offset);
    src_manager_write_font(outfile, src_addr_map, offset);

    outfile.close();

    std::cout << "Bin file out Done!" << std::endl;

    // 将 map 拷贝到 vector 并按地址排序
    std::vector<std::pair<std::string, size_t>> sorted_addr(src_addr_map.begin(), src_addr_map.end());
    std::sort(sorted_addr.begin(), sorted_addr.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // 输出地址表到文件（十六进制，按地址顺序）
    std::ofstream addrfile("addr_map.txt", std::ios::out);
    if (addrfile.is_open()) {
        for (const auto& kv : sorted_addr) {
            addrfile << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;
        }
        addrfile.close();
        std::cout << "地址表已经输出 addr_map.txt" << std::endl;
    } else {
        std::cout << "无法打开 addr_map.txt 文件进行写入" << std::endl;
    }

    // 同时在控制台输出（十六进制，按地址顺序）
    for (const auto& kv : sorted_addr)
        std::cout << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;

    return 0;
}
