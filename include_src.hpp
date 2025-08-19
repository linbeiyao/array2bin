
#include <stdint.h> 

/// ��Դ����ö��
enum is_type_e {
    is_image = 0,
    is_font,
    is_audio,
    is_font_char,
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


/// �ⲿ��Դ����
extern tImage logo_MQ;
extern tFont Font_MQ2_EN_22;
extern tFont Font_MQ2_CN_22;
extern tFont Font_MQ2_CN_14;

extern tFont Font_MQ_V2_CN_14;
extern tFont Font_MQ_V2_CN_16;
extern tFont Font_MQ_V2_CN_22;
extern tFont Font_MQ_V2_EN_16;
extern tFont Font_MQ_V2_EN_22;

extern tFont Font_MQ_V3_EN_14;
extern tFont Font_MQ_V3_EN_16;
extern tFont Font_MQ_V3_EN_22;