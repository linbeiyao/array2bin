#ifndef SRC_MANAGER_HPP
#define SRC_MANAGER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <map>

/// ��Դ����ö��
enum is_type_e {
    is_image = 0,
    is_font,
    is_audio,
    is_type_max,
    is_unknown = 0xFF // δ֪����
};

/// ͼ��ṹ�壨���ţ�
struct tImage {
    const uint16_t* data;  // ͼ����������
    uint16_t width;        // ��ȣ����أ�
    uint16_t height;       // �߶ȣ����أ�
    uint8_t dataSize;      // ÿ���� bit ����һ��Ϊ16��
};

/// �ַ��ṹ�壨�������壩
struct tChar {
    long int code;         // �ַ����루��Ϊ UNICODE��
    const tImage* image;   // ��Ӧ�ַ���ͼ��ָ��
};

/// ����ṹ�壨�ɶ���ַ�ͼ����ɣ�
struct tFont {
    int length;            // �ַ�����
    const tChar* chars;    // �ַ�����
};

/// ע�����Դ�ṹ��
struct src_item_t {
    void* data;                // ָ�� tImage* �� tFont*
    is_type_e type;            // ��Դ����
    std::string name;          // ��Դ����
    size_t satrt_addr;               // �洢��ַ��һ��Ϊ SPI Flash �ڲ�ƫ�Ƶ�ַ��
    size_t end_addr;
    size_t data_byte_count;    // ���ݴ�С�����ֽ�Ϊ��λ��
};

/// �ⲿ��Դ����������Ը�����Ҫ������ӣ�
extern tImage logo_MQ;
extern tFont Font_MQ2_EN_22;
extern tFont Font_MQ2_CN_22;
extern tFont Font_MQ2_CN_14;

/// ��Դע�ắ��
void src_manager_init();   // ��ʼ����ע��������Դ
void src_manager_deinit(); // �����Դ��һ�����ڹ��߽�����

/// ������Դע��ӿڣ�addr ��ѡ��
void reg_src_data(void* src, const char* name, is_type_e type, size_t addr);

/// ��ȡ��Դ�����ֽ�������λΪ byte��
size_t get_src_data_datasize(size_t index);

/// ��ȡͼƬ/����ĳͼ����Դ��λ��С����λΪ bit��
size_t get_src_data_size(const tImage* image);
size_t get_src_data_size(const tFont* font, size_t font_image_index);

/// ��ȡ��Դ����
is_type_e is_src_type(size_t index);

/// ����Դ����д�뵽�������ļ��У���С�˷�ʽ��
void src_manager_write_data(std::ostream& os, std::map<std::string, size_t>& addr_map);

/// ������֤
/// ����ֵ��true ��ʾУ��ͨ����false ��ʾ�д���
bool src_manager_verify_data();


// ��ע����Դ���������
extern uint16_t reg_src_count;
extern uint16_t reg_src_image_count;
extern uint16_t reg_src_font_count;

extern std::vector<src_item_t> src_item_array;

#endif // SRC_MANAGER_HPP
