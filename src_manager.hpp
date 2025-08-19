 #ifndef SRC_MANAGER_HPP
#define SRC_MANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <map>
#include "include_src.hpp"

/// ����ͷ�ṹ��
struct src_head_t {
    char     version[32];      // �ļ��汾/���ƣ����� "MQ2_V1_20250815"
    uint16_t resource_count;   // ��Դ���������������ÿ���ַ���
    uint16_t image_count;      // ͼ����Դ����
    uint16_t font_count;       // ������Դ����
    uint16_t reserved1;        // �������/����
    uint32_t total_size;       // �������ܴ�С���ֽڣ�
    uint32_t addr_table_offset;// ��ַ����ʼλ�ã�������ļ�ͷ��
    uint32_t addr_table_size;  // ��ַ���ܴ�С���ֽڣ�
    uint32_t reserved[4];      // Ԥ����չ��CRC��ѹ����ʽ�ȣ�
};

/// ��ַ����ṹ��
struct src_addr_item_t {
    uint32_t id;            // ȫ��Ψһ ID  
    uint32_t parent_id;     // ����Դ ID (0 ��ʾ����)
    uint32_t start_addr;    // ������ʼ��ַ
    uint32_t size;          // ���ݴ�С
    uint16_t type;          // is_image/is_font/is_audio/is_font_char
    uint16_t char_code;     // ���� font_char ʱ��Ч��Unicode ���룩
    char     name[48];      // ��Դ��
};

/// ע�����Դ�ṹ��
struct src_item_t {
    void* data;                // ָ�� tImage* �� tFont*
    is_type_e type;            // ��Դ����
    std::string name;          // ��Դ����
    size_t satrt_addr;         // �洢��ַ
    size_t end_addr;
    size_t data_byte_count;    // ���ݴ�С�����ֽ�Ϊ��λ��
    std::vector<src_addr_item_t> addr_table;        // ������Դ�ĵ�ַ��
};



// ��ע����Դ���������
extern uint16_t reg_src_count;
extern uint16_t reg_src_image_count;
extern uint16_t reg_src_font_count;

extern src_head_t src_head;
extern std::vector<src_item_t> src_item_array;



/// ��Դע�ắ��
void src_manager_init();   // ��ʼ����ע��������Դ
void src_manager_deinit(); // �����Դ��һ�����ڹ��߽�����

/// ����ͷ��������
void creat_flash_data_head(std::string name, std::map<std::string, size_t>& addr_map);

// �����д�뺯��
void src_manager_write_head(std::ostream& os, src_head_t head);
void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map);
void src_manager_write_addr_table(std::ostream& os);

/// ������֤
bool src_manager_verify_data();

void name_add_time_str(std::string* bin_file_name, std::string* log_file_name);







#endif // SRC_MANAGER_HPP
