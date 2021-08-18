#include <iostream>
#include <string>   
#include <cstdio>
#include <cstring>
#include <complex>
#include <array>
#include "huffleman.hpp"
#include <utility>
#include <typeinfo>
#include <functional>
#include <type_traits>
#include <thread>
#include "tweaked_array.hpp"
typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12
} NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIORITY_LOW = 1,
    NALU_PRIORITY_HIGH = 2,
    NALU_PRIORITY_HIGHEST = 3
} NaluPriority;

struct NALU_t{
    int startcodeprefix_len; // 4 是parameter sets 和图片的第一个slice
    unsigned len; // 包括前面的startcodeprefix,虽然前面的只是一个分割符号
    unsigned max_size; // Nal Unit 负载的大小
    int forbidden_bit; // 不知道干啥的
    int nal_reference_idc; // 优先级
    int nal_unit_type; // nal 单元的类型
    char* buf;
    
    ~NALU_t() {
        delete[] buf;
    }

    NALU_t() = default;

    NALU_t(NALU_t&& t): startcodeprefix_len(t.startcodeprefix_len), len(t.len),max_size(t.max_size),forbidden_bit(t.forbidden_bit),nal_reference_idc(t.nal_reference_idc),nal_unit_type(t.nal_unit_type),buf(t.buf) {
        t.buf = nullptr;
    }
};

struct h264parser {
    FILE* h264bitstream = nullptr;
    bool info2=false;
    bool info3=false;
    // 000001 return true else return false 
    bool FindStartCode2(unsigned char* buf) {
        if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1) return false;
        return true;
    }

    bool FindStartCode3(unsigned char* buf) {
        if (buf[0] != 0 || buf[1] != 0 || buf[2] != 0 || buf[3] != 1) return false;
        return true;
    }

    int GetAnnexbNALU(NALU_t& nalu) {
        int pos = 0;
        int StartCodeFound,rewind;
        unsigned char* buf;
        if ((buf = (unsigned char*)calloc(nalu.max_size, sizeof(char))) == nullptr) {
            std::cout << "GetAnnexbNALU: Could not allocate buf memory" << std::endl;
        }
        nalu.startcodeprefix_len = 3;
        if (3 != fread(buf,1,3,h264bitstream)) {
            free(buf);
            return 0;
        }
        info2 = FindStartCode2(buf);
        if (!info2) {
            if (1 != fread(buf+3,1,1,h264bitstream)) {
                free(buf);
                return 0;
            }
            info3 = FindStartCode3(buf);
            if (!info3) {
                free(buf);
                return -1;
            } else {
                pos = 4;
                nalu.startcodeprefix_len = 4;
            }
        } else {
            nalu.startcodeprefix_len = 3;
            pos = 3;
        }
        StartCodeFound = 0;
        info2 = false;
        info3 = false;
        while (!StartCodeFound) {
            if (feof(h264bitstream)) {
                nalu.len = (pos-1) - nalu.startcodeprefix_len;
                memcpy(nalu.buf, &buf[nalu.startcodeprefix_len], nalu.len);
                nalu.forbidden_bit = nalu.buf[0] & 0x80;
                nalu.nal_reference_idc = nalu.buf[0] & 0x60;
                nalu.nal_unit_type = nalu.buf[0] & 0x1f;
                free(buf);
                return pos-1; 
            }
            buf[pos++] = fgetc(h264bitstream);
            info3 = FindStartCode3(&buf[pos-4]);
            if (!info3) {
                info2 = FindStartCode2(&buf[pos-3]);
            }
            StartCodeFound = (info3 || info2);
        }
        rewind = (info3 == 1) ? -4 : -3;
        if (0 != fseek(h264bitstream, rewind, SEEK_CUR)) {
            free(buf);
            std::cout << "GetAnnexbNALU: Cannot fseek in the bit stream file" << std::endl;
        }
        nalu.len = (pos+rewind) - nalu.startcodeprefix_len;
        memcpy(nalu.buf,&buf[nalu.startcodeprefix_len], nalu.len);
        nalu.forbidden_bit = nalu.buf[0] & 0x80;
        nalu.nal_reference_idc = nalu.buf[0] & 0x60;
        nalu.nal_unit_type = nalu.buf[0] & 0x1f;
        free(buf);
        return (pos+rewind);
    }

    int simplest_h264_parser(char* url) {
        NALU_t n;
        int buffersize = 100000;
        FILE* myout = stdout;
        h264bitstream = fopen(url,"rb+");
        if (!h264bitstream) {
            std::cout << "open file error" << std::endl;
            return 0;
        }
        n.max_size = buffersize;
        n.buf = static_cast<char*>(calloc(buffersize, sizeof(char)));
        if (!n.buf) {
            std::cout << "AllocNALU: n.buf"<< std::endl;
            return 0;
        }
        int data_offset = 0;
        int nal_num = 0;
        std::cout << "-----+-------- NALU Table ------+---------+\n" << std::endl;
        std::cout << " NUM |    POS  |    IDC |  TYPE |   LEN   |\n" << std::endl;
        std::cout << "-----+---------+--------+-------+---------+\n" << std::endl;
        while (!feof(h264bitstream)) {
            int data_length;
            data_length = GetAnnexbNALU(n);
            char type_str[20] = {0};
            switch(n.nal_unit_type) {
                case NALU_TYPE_SLICE: sprintf(type_str, "SLICE"); break;
                case NALU_TYPE_DPA: sprintf(type_str, "DPA"); break;
                case NALU_TYPE_DPB: sprintf(type_str, "DPB"); break;
                case NALU_TYPE_DPC: sprintf(type_str, "DPC"); break;
                case NALU_TYPE_IDR: sprintf(type_str, "IDR"); break;
                case NALU_TYPE_SEI: sprintf(type_str, "SEI"); break;
                case NALU_TYPE_SPS: sprintf(type_str, "SPS"); break;
                case NALU_TYPE_PPS: sprintf(type_str, "PPS"); break;
                case NALU_TYPE_AUD: sprintf(type_str, "AUD"); break;
                case NALU_TYPE_EOSEQ:sprintf(type_str,"EOSEQ");break;
                case NALU_TYPE_EOSTREAM:sprintf(type_str,"EOSTREAM");break;
    			case NALU_TYPE_FILL:sprintf(type_str,"FILL");break;
            }
            char idc_str[20] = {0};
            switch(n.nal_reference_idc>>5) {
                case NALU_PRIORITY_DISPOSABLE: sprintf(idc_str, "DISPOS");break;
                case NALU_PRIORITY_LOW: sprintf(idc_str, "LOW");break;
                case NALU_PRIORITY_HIGH: sprintf(idc_str, "HIGH");break;
                case NALU_PRIORITY_HIGHEST: sprintf(idc_str, "HIGHEST");break;
            }
            fprintf(myout,"%5d| %8d| %7s| %6s| %8d|\n",nal_num,data_offset,idc_str,type_str,n.len);
            data_offset+=data_length;
            nal_num++;
        }
        return 0;
    }
};


template <typename T, size_t size>
class fixed_vector {
public:
typedef T* iterator;
typedef const T* const_iterator;
typedef fixed_vector<T,size> self;
fixed_vector() = default;
fixed_vector(const fixed_vector<T,size>& data) {
    for (int i=0;i<size;i++) {
        v_[i] = data.v_[i];
    }
}

self& operator=(const self& data) {
    if (this == &data) return *this;
    for (int i=0;i<size;i++) {
        v_[i] = data.v_[i];
    }
    return *this;
}


iterator begin() { return v_; }
iterator end() { return v_+size;}
const_iterator begin() const {return v_; }
const_iterator end() const  {return v_+size;}
private:
    T v_[size];
};

using std::cout;
using std::endl;
class Bclass
{
private:
	virtual void fun() { cout << "Bclass fun is here!" << endl ; }
	void fun2() { cout << "Bclass fun2 is here!" << endl; }
	friend int main();
};
 
class Dclass : public Bclass
{
public:
	virtual void fun() { cout << "Dclass fun is here!" << endl; }
	void fun2() { cout << "Dclass fun2 is here!" << endl; }
};

template <typename T>
class MyList {
public:
    bool Insert(const T& data, size_t index);
    T Access(size_t index) const;
    size_t Size() const;
private:
    T* buf;
    size_t bufsize_;    
};

template <class T>
class MySet1 : private MyList<T>
{
public:
    bool Add(const T&);
    T Get(size_t index) const;
    using MyList<T>::Size;
};

class LibMat {
public:
    LibMat() {
        std::cout << "LibMat::LibMat() default constructor!\n";
    }
    virtual ~LibMat() {
        std::cout << "LibMat::~LibMat() destructor!\n";
    }

    virtual void print() const {
        std::cout << "LibMat::print() -- I am a LibMat object!\n";
    }
};


void print(const LibMat& mat) {
    std::cout << "in global print(): about to print mat. print()\n";
    mat.print();
}


#include <map>
#include <vector>
#include <algorithm>
using namespace std;
class Solution {
public:
    int halfQuestions(vector<int>& questions) {

        map<int,int> counter;
        for (auto index : questions) {
            ++counter[index];
        }
        int type_count = 0;
        int q_count = questions.size()/2;
        vector<pair<int,int>> data(counter.begin(), counter.end());
        sort(data.begin(),data.end(),[](const pair<int,int> item1, const pair<int, int> item2)->bool {
            return item1.second > item2.second;
        });
        for(auto start = data.begin(); start != data.end();++start) {
                q_count -= start->second;
                ++type_count;
                if (q_count <= 0) {
                    break;
                }
        }
        return type_count;
    }
};

class Demo {
public:
    ~Demo() {
        std::cout << "demo has been destroyed" << std::endl;
    }
private: 
    int data;
};

Demo returnDemo() {
    Demo d;
    std::cout << "local d " << &d << std::endl;
    return d;
}

class range {
public:
    range(int end):_begin(0),_end(end),_step(1){}
    struct iterator {
        iterator(int x, int s):n(x),step(s){}
        
        iterator& operator++() {
            n+=step;
            return *this;
        }

        bool operator!=(const iterator&x) {
            return (step>0) ? n< x.n : n>x.n;
        }

        int& operator*() {
            return n;
        }

        private:
            int n,step;
    };

    iterator begin() {
        return iterator(_begin,_step);
    }
    iterator end() {
        return iterator(_end,_step);
    }

private:
    int _begin;
    int _end;
    int _step;

};

#include <cmath>
template <size_t size>
std::array<complex<float>,size> raine_dft(std::array<double,size> f) {
    using namespace std;
    std::array<complex<float>,size> F;
    auto N = size;
    for (auto n : range(N)) {
        F[n].real(0);
        F[n].imag(0);
        for (auto t : range(N)) {
            complex<float> temp;
            // 欧拉公式；计算实部
            temp.real(cos(-2*M_PI/N*t*n)*f[t]);
            // 欧拉公式；计算虚部
            temp.imag(sin(-2*M_PI/N*t*n)*f[t]);
            F[n]+=temp;
        }
    }
    return F;
}

// 离散傅立叶变换的逆变换
template <::size_t size>
std::array<std::complex<float>,size> raine_idft(std::array<std::complex<float>,size> F) {
    auto N = size;
    std::array<complex<float>,size> f;
    for (auto t : range(N)) {
        f[t].real(0);
        f[t].imag(0);
        for (auto n : range(N)) {
            complex<float> temp;
            double real = std::cos(2*M_PI*n/N*t);
            double imag = std::sin(2*M_PI*n/N*t);
            temp.real(real);
            temp.imag(imag);
            temp = F[n]*temp;
            f[t]+=temp;
        }
        f[t].real(f[t].real()/N);
        f[t].imag(f[t].imag()/N);
    }
    return f;
}

// 测试我的傅立叶变换
void testdft() {
    std::array<double,6> data{14,12,10,8,6,10};
    std::cout << "˚开始计算傅立叶变换" << std::endl;
    auto rate = raine_dft(data);
    for(auto item: rate) {
        std::cout << item << std::endl;
    }
    std::cout << " 计算反傅立叶变换" << std::endl;
    auto irate = raine_idft(rate);
    for(auto item : irate) {
        std::cout << item << std::endl;
    }
}
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <complex>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>

class array_keeper {
public:
     array_keeper(char*buf) {
         this->buf = buf;
     }    
     ~array_keeper() {
         delete [] buf;
     }
     char* get() {
         return buf;
     }
private:
    char* buf;    
};

void displayYuv420(const int w, const int h, const std::string& path) {
    // 使用opencv展示yuv的图片；不管这个文件有多大，我只读取其中的一帧
    FILE* pFileIn = fopen(path.c_str(),"rb");
    const size_t frameSize = w*h*3/2;
    fseek(pFileIn,0,SEEK_END);
    int size = static_cast<int>(ftell(pFileIn));
    // 只有数据中有足够的内容才展示
    if (size > frameSize) {
        array_keeper buf(new char[frameSize]);
        fread(buf.get(),frameSize,1,pFileIn);
        cv::Mat yuvImg;
        yuvImg.create(h*3/2,w,CV_8UC1);
        memcpy(yuvImg.data,buf.get(),frameSize);
        cv::Mat rgbImg;
        cv::cvtColor(yuvImg,rgbImg,cv::COLOR_RGB2YUV_I420);
        cv::imshow("rgb",rgbImg);
        cv::waitKey(0);
    }
}

static std::string hex(char code) {
    int mask = 0xf;
    static char hexCodeMap[] = {
        '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
    };

    if (code == 0) {
        return "0x0";
    }
    unsigned char code_u = code;
    std::string form;
    while (code_u) {
        form += hexCodeMap[code_u & mask];
        code_u >>= 4;
    }
    form+="x0";
    std::reverse(form.begin(),form.end());
    return form;
}

class Noisy {
    public:
        Noisy() {
            std::cout << "I am happy noisy " << this << std::endl;
        }
        Noisy(const Noisy& noi) {
            std::cout << "I am coping another noisy" << this << std::endl;
        }
    ~Noisy() {
        std::cout << "Noisy is destroying" << std::endl;
    }
};

struct Trivial1 {};
struct Trivial2 {
    int x;
};

struct Trivial3 : Trivial2 {
    Trivial3() = default;
    int y;
};

struct Trivial4 {
public:
    int a;
private:
    int b;
};

struct Trivial5 {
    Trivial1 a;
    Trivial2 b;
    Trivial3 c;
    Trivial4 d;
};

struct Trivial6 {
    Trivial2 a[23];
};

struct Trivial7 {
    Trivial6 c;
    void f();
};

struct Trivial8 {
    Trivial8() = default;
    Trivial8(int x): x(x) {};
    int x;
};

#include "MutexGuard.hpp"
#include "CountDownLatch.hpp"
#include "Thread.hpp"
#include "CurrentThread.h"
raine::MutexLock mylock;
raine::CountDownLatch latch(2);
int counter = 0;

void incCounter(int repeat) {
    raine::MutexLockGuard guard(mylock);
    for (int i=0;i<repeat;i++) {
        ++counter;
    }
    latch.countDown();
}

class GlobalScope {
public:
    GlobalScope() {}

    ~GlobalScope() {
        raine::MutexLockGuard guard(lock);
        std::cout << "release successfully" << std::endl;
    }
    void doInit() {
        raine::MutexLockGuard guard(lock);
        someFuncJustExit();
    }
    void someFuncJustExit() {
        exit(1);
    }
private:
    raine::MutexLock lock;
};

struct PureObj {
    std::string name;
    ~PureObj() {}
};

#include <deque>
#include "Timestamp.h"
int main(void) {
    PureObj obj;
    deque<int> value;
    value.push_back(1);
    value.push_back(2);
    deque<int> value2(std::move(value));
    for (auto& data : value2) {
        std::cout << " data " << data << std::endl;
    }
    value.push_back(1);
    for (auto& data : value) {
        std::cout << " data " << data << std::endl;
    }
    obj.name = "hello world";
    PureObj another = std::move(obj);
    std::cout << "origin " << obj.name <<  " " << std::endl;
    std::cout << "moved ..." << another.name << " " << std::endl;
    std::thread thread1{incCounter, 100000};
    std::thread thread2{incCounter, 200000};
    latch.await();
    thread1.join();
    thread2.join();
    raine::Thread myThread([](){
        std::cout << raine::CurrentThread::tidString() << std::endl;
        std::cout <<raine::CurrentThread::stackTrace(true) << std::endl;
        std::cout << "hello world" << std::endl;
    });
    std::cout << counter << std::endl;
    std::string stack = raine::CurrentThread::stackTrace(false);
    std::cout << stack << std::endl;
    myThread.start();
    myThread.join();
    return 0;
}