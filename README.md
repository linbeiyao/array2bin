# array2bin

将嵌入式资源（图片、字体等）按统一格式打包为单一二进制文件，内含数据头与地址表，便于在 MCU/Flash 中按表定位读取。当前示例资源来自 `MQ2_V1` 系列（Logo 与中英文字库）。

### 功能概述
- 资源注册：在启动时注册图片与字体资源，自动计算大小与地址。
- 二进制生成：输出包含数据头、地址表与数据区的 `.bin` 文件。
- 可读性日志：控制台打印完整的地址表明细，辅助调试与验证。
- 时间戳命名：输出与日志文件自动追加时间戳，便于版本归档。

### 目录结构
- `MQ2_V1/`, `MQ2_V2_ 节电小能手/`, `MQ2_V3_ECOSAVER/`: 各版本的字库/图片资源实现（`.cpp`）。
- `include_src.hpp`: 资源类型、结构体与外部资源声明。
- `src_manager.hpp/.cpp`: 资源注册、地址表构建、写文件与校验逻辑。
- `main.cpp`: 程序入口；注册资源、生成 bin、打印日志。
- `output/`: 生成的二进制输出目录。
- `log/`: 日志输出目录（名称同样带时间戳）。

### 构建环境
- 平台：Windows 10/11
- 工具链：Visual Studio 2022（MSVC v143）
- 语言标准：C++（项目默认设置即可）

步骤：
1. 用 VS2022 打开 `array2bin.vcxproj`。
2. 选择构建配置（建议 `x64/Debug` 或 `x64/Release`）。
3. 生成解决方案，得到可执行文件。

可执行文件位于 VS 的构建输出目录（例如 `x64/Debug/array2bin.exe`）。

### 运行与交互
运行程序后：
- 程序会生成带时间戳的输出与日志文件名，并在控制台提示：
  - 回车：开始制作文件
  - 输入 `q`：退出程序
- 生成完成后，控制台提示 `Bin file out Done!` 以及资源与地址表信息。

注意：确保 `output/` 与 `log/` 目录存在（默认项目已包含）。

### 输出文件命名规则
文件名在基础名后自动追加时间戳 `_YYYYMMDD_HHMMSS`：
- 二进制：`output/MQ2_V1_YYYYMMDD_HHMMSS.bin`
- 日志：`log/MQ2_V1_YYYYMMDD_HHMMSS.txt`

命名逻辑见 `name_add_time_str`（时间获取与格式化、拼接后缀等）。

### 二进制文件格式
整体布局：
- [0x0000] `src_head_t`（64 字节）
- [0x0040] 预留间隔（32 字节）
- [head.addr_table_addr] 地址表（若干个 `src_addr_item_t`，每项 64 字节）
- [地址表末尾后] 预留间隔（32 字节）
- [reserved[0]] 数据区（资源像素/字形数据，按地址表项顺序写入）

关键结构体（节选）：
```11:45:src_manager.hpp
/// 数据头结构体 32 + 2
struct src_head_t {
    char     version[32];      // 文件版本/名称，例如 "MQ2_V1_20250815"
    uint16_t resource_count;   // 顶层资源数
    uint16_t image_count;      // 图像资源数量
    uint16_t font_count;       // 字体资源数量
    uint16_t reserved1;        // 对齐填充/保留
    uint32_t total_size;       // 数据区总大小（字节）
    uint32_t addr_table_addr;// 地址表起始位置（相对于文件头）
    // uint32_t addr_table_size;  // 地址表总大小（字节）
    uint32_t reserved[4];      // 预留扩展（CRC、压缩方式等）
};
#pragma pack(push, 1)
/// 地址表项结构体 - 扩展版本，包含更多元数据  64 字节
struct src_addr_item_t {
    uint32_t id;            // 全局唯一 ID  
    uint32_t parent_id;     // 父资源 ID (0 表示顶级)
    uint32_t start_addr;    // 数据起始地址
    uint32_t size;          // 数据大小
    uint16_t type;          // is_image/is_font/is_audio/is_font_char
    uint32_t char_code;     // 仅当 font_char 时有效（Unicode 编码）
    
    // 图像元数据（当 type == is_image 或 is_font_char 时有效）
    uint16_t width;         // 图像宽度（像素）
    uint16_t height;        // 图像高度（像素）
    uint8_t dataSize;       // 每像素 bit 数（一般为16）
    uint8_t reserved1;      // 保留字段，用于对齐
    
    // 字体元数据（当 type == is_font 时有效）
    uint16_t char_count;    // 字体包含的字符数量
    uint16_t reserved2;     // 保留字段，用于对齐
    
    char     name[32];      // 资源名（减少长度以适应新增字段）
};
#pragma pack(pop)
```

类型与资源描述：
```4:32:include_src.hpp
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
```

重要说明：
- `src_head_t::reserved[0]` 保存了数据区在文件中的基址（file offset）。
- 地址表项的 `start_addr` 为“相对数据区”的偏移，解析时需用 `file_offset = reserved[0] + start_addr` 得到文件中的真实偏移。
- 资源像素以 16-bit 小端序写入（低字节在前，高字节在后）。
- 地址表按 64 字节对齐的结构顺序连续写入。

### 资源注册与扩展
资源注册位于 `src_manager_init()`：
- 默认示例：
  - `logo_MQ` 作为图片注册
  - `Font_MQ2_CN_22`, `Font_MQ2_EN_22`, `Font_MQ2_CN_14` 作为字体注册
- 若需要新增资源：
  1) 在相应目录添加实现（例如新增 `*.cpp` 定义 `tImage` / `tFont`）。
  2) 在 `include_src.hpp` 中添加 `extern` 声明。
  3) 在 `src_manager_init()` 中调用 `reg_src_data(&yourAsset, "NAME", is_image/is_font[, start_addr]);`
     - 如指定 `start_addr`，工具会自动避免覆盖；如发生重叠，会向后顺延。

校验与限制：
- 运行时会校验资源数量、尺寸一致性与地址重叠。
- 地址最大支持到 24-bit（≤ 0xFFFFFF）。

### 解析器参考（MCU 侧）
- 读取文件头 `src_head_t`，记下 `addr_table_addr` 与 `reserved[0]`（数据区基址）。
- 遍历地址表 `src_addr_item_t`：
  - 计算 `file_offset = reserved[0] + start_addr`
  - 读取 `size` 字节的数据，并按 `type`、`width/height/dataSize` 解释像素/字形。
- 字体资源包含一个父节点（`type == is_font`，记录 `char_count`），以及多个子项（`type == is_font_char`）。

### 常见问题（FAQ）
- 无法生成文件/大小为 0：确认 `output/` 目录存在；确保已注册的资源数据与尺寸有效。
- 解析到的像素颠倒或颜色异常：确认 16-bit 小端序读取，像素格式需与生成侧一致（`dataSize=16`）。
- 地址不对齐：`start_addr` 为数据区相对偏移，解析侧需加上 `reserved[0]`。

### 代码风格
- 遵循 HAL 风格的命名与模块化思路：
  - 接口清晰、数据结构自描述、避免隐式全局副作用。
  - 结构体与枚举具备明确含义，便于在驱动/上层模块直接复用。
- 新增功能仅在现有基础上“增量添加”，不破坏兼容性。

### 许可与归属
- 资源与代码版权归原作者/团队所有。若需分发，请先确认授权。 