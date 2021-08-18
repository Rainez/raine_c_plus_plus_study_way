#include <cstdint>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstring>
#include <iostream>
#include <sstream>

struct mp4box {
    int32_t size; // 该box的大小
    char type[5]; // 该box的类型
    int64_t extend_size; // 如果size = 1,则extend_size用来表示扩展的大小
    int64_t offset;
    bool is_container;
    mp4box() {
        type[4] = 0;
        is_container = false;
    }
public:
    virtual std::string des() {
        std::string tmp;
        tmp+= "size= ";
        tmp+= std::to_string(size);
        tmp+=" type = ";
        tmp+= type;
        tmp+=  " file offset = ";
        tmp+= std::to_string(offset);
        return tmp;
    }
};

struct container_box : public mp4box {
    container_box() {
        is_container = true;
    }
    std::vector<mp4box> sub_boxes;
};

struct ftype_box : public mp4box {
    ftype_box() {
        type[0] = 'f';
        type[1] = 't';
        type[2] = 'y';
        type[3] = 'e';
        type[4] = 0;
        major_brand[4] = 0;
        compatible_brand[16] = 0;
    }
    char major_brand[5];
    int32_t minor_version;
    char compatible_brand[17];
    virtual std::string des() override{
        auto des = mp4box::des();
        des+= " majror_brand = ";
        des+=major_brand;
        des+=" minor_version = ";
        des+= std::to_string(minor_version);
        des+= " compatible_brand = ";
        des+= compatible_brand;
        return des;
    }
};

struct full_mp4box : public mp4box {
    int32_t version:8;
    int32_t flags:24;
};

struct mvhd_box : public full_mp4box {
    mvhd_box() {
        type[0] = 'm';
        type[1] = 'v';
        type[2] = 'h';
        type[3] = 'd';
        type[4] = 0;
    }
    int32_t rate; //推荐的播放速率，高16位为小数点的整数部分，低16位为小数点的小数部分
    int16_t volume; // 音量
    int8_t reversed[10]; // 保留位
    int8_t matrix[36]; // 视频变换矩阵
    int8_t pre_defined[24]; 
    int32_t next_track_id; // 下一个track使用的id号
};

struct tkhd_box : public full_mp4box {
    enum track_mode {
        track_enabled = 0x01,
        track_in_movie = 0x02,
        track_in_preview = 0x04
    };
    int32_t creation_time;
    int32_t modification_time;
    int32_t track_id; // track id号码；不能重复且不为0
    int32_t reserved;
    int32_t duration; // track的时间长度
    int64_t  reserved2;
    int16_t layer; // 默认为0，值小的在上面
    int16_t alternate_group; // track分组信息，默认为0，表示该track未与其他track有群组关系
    int16_t volume; // [8.8] 格式，如果为音频track，1.0（0x0100）表示最大音量；否则为0
    int16_t reverved3;
    int8_t matrix[36];
    int32_t width;
    int32_t height; // 高，均为 [16.16] 格式值，与sample描述中的实际画面大小比值，用于播放时的展示宽高

    
};

struct mdhd_box : public full_mp4box {
    int32_t creation_time;
    int32_t modification_time;
    int32_t time_scale; // 文件媒体在一秒时间内的刻度值，可以理解为1秒长度的时间单元数
    int32_t duration;  // 该track的时间长度，用duration和time scale值可以计算track时长，duration/time_scale;
    int16_t language; // 媒体语言码。最高位为0，后面15位为3个字符（见ISO 639-2/T标准中定义）
    int16_t predefined; 

};

struct handler_reference_box : public full_mp4box {
    handler_reference_box() {
        type[0] = 'h';
        type[1] = 'd';
        type[2] = 'l';
        type[3] = 'r';
        type[4] = 0;
    }
    int32_t pre_defined;
    char handler_type[5]; // 在media_box中为四个字符 "vide" - video track "soun" -> audio track "hint" - hint track
    int8_t reserved[12];
    std::string name; // track type的名字
};

struct video_media_header_box : public full_mp4box {
    video_media_header_box() {
        type[0] = 'v';
        type[1] = 'm';
        type[2] = 'h';
        type[3] = 'd';
        type[4] = 0;
    }
    int32_t graphics_mode; // 为0则直接拷贝原始图像，否则与opcolor进行合成
    int16_t opcolor[3]; // todo 对于这个header.还需要再调查，你的理解可能是不对的
};

struct sound_media_header_box : public full_mp4box {
    sound_media_header_box() {
        type[0] = 's';
        type[1] = 'm';
        type[2] = 'h';
        type[3] = 'd';
        type[4] = 0;
    }  
    int16_t balance; // 立体声平衡，[8.8] 格式值，一般为0，-1.0表示全部左声道，1.0表示全部右声道
    int16_t reserved;
};

// 这里的格式只是大概的... 详细的等查询文档了之后再行学习
struct data_reference_box {
    int32_t entry_count; // url或者urn表的元素个数
    std::vector<std::string> urls;
};

struct mvhd_v0_box : public mvhd_box {
    mvhd_v0_box() {
        version = 0;
        flags = 0;
    }
    int32_t creation_time; // 创建时间（相对于UTC时间1904-01-01零点的秒数)
    int32_t modification_time; // 修改时间
    int32_t time_scale; // 文件媒体在一秒时间内的刻度值，可以理解为1秒长度的时间单元数
    int32_t duration;  // 该track的时间长度，用duration和time scale值可以计算track时长，duration/time_scale;
};


struct mp4box_container : public mp4box {
    std::vector<mp4box> box_container;
};

namespace {
const char* mp4_single_box[] = {
    "ftype", // 有且仅有一个，作为mp4格式的标志并且包含文件的一些信息
    "mvhd",
};
const char* container_box[] = {
    "moov", // 子box包含了媒体的metadata信息
    "mdat", // 包含了媒体数据；如果媒体数据完全引用其他文件，那么这个也可以没有
    "trak"
};
}

class boxparser {
public:
    std::shared_ptr<mp4box> parse(FILE* input) {
        if (!input || feof(input)) {
            std::cout << "not valid file " << input << std::endl;
            return nullptr;
        }
        return nullptr;
    }
    std::shared_ptr<mp4box> parseSingleEle(FILE* input) {
       if (!input || feof(input)) {
           std::cout << "not valid file " << input << std::endl;
           return nullptr;
       }
       std::cout << "valid file " << input << std::endl;
       char buf[9];
       buf[8] = 0;
       fpos_t offset;
       fgetpos(input,&offset);
       int readedSize = fread(buf,1,8,input);
       if (readedSize != 8) return nullptr;
       std::shared_ptr<mp4box> box = nullptr;
       int size = parseInt32BE(buf);
       if (std::string(buf+4) == "ftyp") {
         char sub_buf[24];  
         auto tmp = std::make_shared<ftype_box>();
         //这里总共读了20个字符
         int readedSize = fread(sub_buf,1,24,input);
         assert(readedSize == 24);
         memcpy(tmp->major_brand,sub_buf,4);
         tmp->major_brand[4] = 0;
         tmp->minor_version = parseInt32BE(sub_buf+4);
         memcpy(tmp->compatible_brand,sub_buf+8,16);
         tmp->compatible_brand[16] = 0;
         fseek(input,size-32,SEEK_CUR);
         box = tmp;
         
       } else {
           box = std::make_shared<mp4box>();
           strcpy(box->type,buf+4);
           fseek(input,size-8,SEEK_CUR);
       }
       // 不应该再有读文件的操作
       box->size = parseInt32BE(buf);
       box->offset = static_cast<int64_t>(offset);
       return box;
    }
private:
    std::vector<std::shared_ptr<mp4box>> boxes; 
    union lehelper {
        int32_t t;
        char data[4];
    } data{0x01020304};

    bool isLE() {
        return data.data[0] == 0x04;
    }

    int parseInt32BE(const char* buf) {
        if (isLE()) {
            char tmp[4];
            auto swapblock = [](char* buf, int a, int b)->void {
                auto tmp = buf[a];
                buf[a] = buf[b];
                buf[b] = tmp;
            };
            memcpy(tmp,buf,4);
            swapblock(tmp,0,3);
            swapblock(tmp,1,2);
            return *reinterpret_cast<const int*>(tmp);
        } else {
            return *reinterpret_cast<const int*>(buf);
        }
    }
    

};

// int main(void) {

//     boxparser parser;
//     auto box = parser.parseSingleEle(fopen("../1.mp4","rb"));

//     std::cout << box->des() << std::endl;
//     return 0;
// }