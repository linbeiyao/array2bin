
#include <stdint.h> 

/// 资源类型枚举
enum is_type_e {
    is_image = 0,
    is_font,
    is_audio,
    is_font_char,
    is_type_max,
    is_unknown = 0xFF // 未知类型
};

/// 图像结构体（单张）
struct tImage {
    const uint16_t* data;  // 图像像素数据
    uint16_t width;        // 宽度（像素）
    uint16_t height;       // 高度（像素）
    uint8_t dataSize;      // 每像素 bit 数（一般为16）
};

/// 字符结构体（用于字体）
struct tChar {
    long int code;         // 字符编码（可为 UNICODE）
    const tImage* image;   // 对应字符的图像指针
};

/// 字体结构体（由多个字符图像组成）
struct tFont {
    int length;            // 字符数量
    const tChar* chars;    // 字符数组
};


/// 外部资源声明
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