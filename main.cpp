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

    // �� map ������ vector ������ַ����
    std::vector<std::pair<std::string, size_t>> sorted_addr(src_addr_map.begin(), src_addr_map.end());
    std::sort(sorted_addr.begin(), sorted_addr.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // �����ַ���ļ���ʮ�����ƣ�����ַ˳��
    std::ofstream addrfile("addr_map.txt", std::ios::out);
    if (addrfile.is_open()) {
        for (const auto& kv : sorted_addr) {
            addrfile << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;
        }
        addrfile.close();
        std::cout << "��ַ���Ѿ���� addr_map.txt" << std::endl;
    } else {
        std::cout << "�޷��� addr_map.txt �ļ�����д��" << std::endl;
    }

    // ͬʱ�ڿ���̨�����ʮ�����ƣ�����ַ˳��
    for (const auto& kv : sorted_addr)
        std::cout << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;

    return 0;
}
