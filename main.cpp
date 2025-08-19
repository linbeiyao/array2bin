#include <fstream>
#include <cstdint>
#include <iostream>
#include "src_manager.hpp"
#include <map>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>


// ������������������������������������������������������������������
// ��         ����ͷ src_head_t      ��
// ������������������������������������������������������������������
// ��         ��Դ������            ��
// �� ������������������������������              ��
// �� �� ͼ������    ��              ��
// �� ������������������������������              ��
// �� �� ��������    ��              ��
// �� ������������������������������              ��
// �� �� ...         ��              ��
// �� ������������������������������              ��
// ������������������������������������������������������������������
// ��         ��ַ����              ��
// �� ������������������������������              ��
// �� �� src_addr_item_t[] ��        ��
// �� ������������������������������              ��
// ������������������������������������������������������������������
//
// �ṹ˵����
// 1. �ļ���ʼΪ src_head_t �ṹ�壬�����汾����Դ�������ܴ�С����Ϣ��
// 2. �������Ϊ������Դ��ԭʼ���ݣ�ͼ������ȣ�����ע��˳����ַ�����š�
// 3. ���Ϊ��Դ��ַ��src_addr_item_t ���飩����¼ÿ����Դ����ʼ��ַ����С�����͵�Ԫ���ݡ�


std::map<std::string, size_t> src_addr_map;



int main() {
    std::string bin_file_name, log_file_name;

    name_add_time_str(&bin_file_name, &log_file_name);
    std::cout << "�������ļ���: " << bin_file_name << " ��־�ļ���:" << log_file_name << std::endl;

    src_manager_init();


    
    // �ȴ����»س���ʼ�����ļ�  
    // �ȴ����»س������� 'q'  
    printf("�밴�س���ʼ�����ļ��������� 'q' �˳�����...");  
    std::string input;  
    std::getline(std::cin, input);  

    if (input == "q") {  
        printf("�������˳���\n");  
        exit(0);  
    } else if (input.empty()) {  
        printf("����ִ��...\n");  
        return;  
    } else {  
        printf("��Ч���룬���������г���\n");  
        exit(1);  
    }

    // �����洢оƬ������ͷ
    creat_flash_data_head(bin_file_name, src_addr_map);


    std::ofstream outfile(bin_file_name, std::ios::binary);




    src_manager_write_head(outfile, src_head);
    src_manager_write_data(outfile, src_addr_map);
    src_manager_write_addr_table(outfile);


    src_manager_verify_data();




    outfile.close();

    std::cout << "Bin file out Done!" << std::endl;

    // �� map ������ vector ������ַ����
    std::vector<std::pair<std::string, size_t>> sorted_addr(src_addr_map.begin(), src_addr_map.end());
    std::sort(sorted_addr.begin(), sorted_addr.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });

    // �����ַ���ļ���ʮ�����ƣ�����ַ˳��
    std::ofstream addrfile(log_file_name, std::ios::out);
    if (addrfile.is_open()) {
        for (const auto& kv : sorted_addr) {
            addrfile << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;
        }
        addrfile.close();
        std::cout << "��ַ���Ѿ���� " << log_file_name << std::endl;
    }
    else {
        std::cout << "�޷��� " << log_file_name << "�ļ�����д��" << std::endl;
    }

    // ͬʱ�ڿ���̨�����ʮ�����ƣ�����ַ˳��
    for (const auto& kv : sorted_addr)
        std::cout << kv.first << ": 0x" << std::hex << std::uppercase << kv.second << std::endl;

    return 0;
}
