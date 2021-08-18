// this class implementation huffleman algorithm
#include <string>
#include <utility>
#include <memory>
#include <queue>
#include <functional>
#include <stdexcept>
#include <map>
#include <iostream>
#include <array>
#include <cstdint>
/**
 * 这个类输入bit流，输出完整的一个char,如果bit数据不够,就会补充额外的；
 * bg: huffleman编码的数据有的时候可能不是整数byte.不满足这样的边界条件；
 * 因此我们需要将这样的数据输入.
 * 因此这个类正确的使用方式是
 * 
 * while(condition) {
 *    stream.put("00001");
 *    if (stream.avaiable()) {
 *      auto data = stream.poll();
 *      // 这个时候返回的数据应该是0填充的
 *      write(data.first);  
 *    }
 * }
 * // 当条件不满足的时候，bitstream中可能还会含有数据,这个时候可能需要再读一次数据；
 * if (stream.bitCount() !=0) {
 *  auto data = stream.poll();
 *  write(data.first);
 *  // 这个时候的数据可能是有填充的
 * }
 * 
 * 需要注意的是，这个类的保存空间也就只有4个字节大，用户需要及时读出数据，否则在存储空间不够的情况下，后续的数据直接全部丢弃;并不放在空间中
 */
#include "intasarray.hpp"
class bitstream {
public:
    void put(const std::string& raw);
    bool avaiable() const;
    int  bitCount() const;
    std::pair<char,char> poll();
    bitstream():data(0){}
private:
    intasarray data;    
    static constexpr int INT_BIT_SIZE = 32; // 最大的空间容量，但是有一个bit可以
    int rear = 0; // 目前写数据的位置，指向最后一个元素的下一个元素
    int front = 0;  // 指向第一个位置
    
    bool empty() const {
        return rear == front; 
    }

    bool full() const {
        return (rear+1)%INT_BIT_SIZE == front;
    }
    
    int size() const {
        return (rear+INT_BIT_SIZE-front)%INT_BIT_SIZE;
    }
};

static int numberOfLeadingZeros(int input) {
        // HD, Figure 5-6
        unsigned int i = static_cast<unsigned int>(input);
        if (i == 0)
            return 32;
        int n = 1;
        if (i >> 16 == 0) { n += 16; i <<= 16; }
        if (i >> 24 == 0) { n +=  8; i <<=  8; }
        if (i >> 28 == 0) { n +=  4; i <<=  4; }
        if (i >> 30 == 0) { n +=  2; i <<=  2; }
        n -= i >> 31;
        return n;
}

// 这里当然会有更高效的实现方法，但鉴于我目前的渣渣水平，暂时先用上面的方式去实现
void bitstream::put(const std::string& raw) {
    if (!full() && size() > raw.length()) {
        for (auto code : raw) {
                if (code != '0' && code != '1') {
                    return;
                }
        }
    }

    // pass all test.
    for (auto code : raw) {
        if ('1' == code) {
            data.openbit(rear);
        } else {
            data.clearbit(rear);
        }
        rear = (rear+1)%INT_BIT_SIZE;
    }
}

bool bitstream::avaiable()  const{
    return !full();
}

int  bitstream::bitCount() const {
    return size();
}

/**
 * 以免自己忘记了，我在这里再写一次注释，
 * 这个函数会尝试将队列里一个byte的数据出队，如果数据尚不能完整填充一个byte.那么就手动在末尾进行数据填充
 * 返回值
 * first = 返回的数据
 * second = 填充的位数，有效的值为0～8
 */
std::pair<char,char> bitstream::poll() {
    if (empty()) {
        return std::make_pair(0,8);
    } else {
        if (bitCount() > 8) {
            // 不需要padding. 从原始的读出数据
            int result = 0;
            for (int i=0;i<8;i++) {
                int code = (data.isOpen(front)) ? 1 : 0;
                front = (front+1)%INT_BIT_SIZE;
                result<<=1;
                result|=code;
            }
            return std::make_pair(static_cast<char>(result),0);
        } else {
            // 不足一个bit.填充数据
            int currentBitCount = bitCount();
            int result = 0;
            for (int i=0;i<currentBitCount;i++) {
                int code = (data.isOpen(front)) ? 1 : 0;
                front = (front+1)%INT_BIT_SIZE;
                result <<=1;
                result |= code;
            }
            // 末尾填充0；
            int shift = 8 - currentBitCount;
            result <<= shift;
            return std::make_pair(result,shift);
        }
    }
}

class Huffleman {
    public:
     constexpr static int magichref = 0x4f4f6668; // 这个文件是我自己的压缩标准的文件前缀
     constexpr static int magichref_rev = 0x68664f4f;
     struct Node: std::enable_shared_from_this<Huffleman::Node> {
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
    public:
        int weight;
        char value;
        std::shared_ptr<Node> self() {
            return shared_from_this();
        }

        void setLeft(Node& left) {
            this->left = left.self();
        }

        void setLeft(std::shared_ptr<Node> left) {
            this->left = left;
        }

        void setRight(std::shared_ptr<Node> right) {
            this->right = right;
        }

        void setRight(Node& right) {
            this->right = right.self();
        }

        // ~Node() {
        //     // std::cout << "Node " << this << " weight="  << weight << " value = " << value << std::endl;
        // }

    friend bool operator> (const std::shared_ptr<Huffleman::Node> a, const std::shared_ptr<Huffleman::Node> b);
    };
    private:
    // huffleman树的形式和其他的树不一样，不会发生需要叶节点往上移动的情况，因此对于叶子节点使用新的类编码

    // the huffleman table
    // 对于码表的保存；如果直接保存bit。可能会节省更多的空间；
    // 第一版算法为了实现上的方便；直接使用char[]进行表示
    struct table {
        int size;
        std::array<std::string,256> map;
        table():size(0){}
    };
public:
    // 将原始输入的文件编码输出到output
    static void  encode(FILE* input, FILE* output) {
        table encoderMap;

        if (input == nullptr || output == nullptr) return;
        std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>> , std::greater<std::shared_ptr<Node>>> nodes;   
        std::map<char,int> counter;
        char buf[1];
        while(!feof(input)) {
            int count = fread(buf,1,1,input);
            if (count == 0) break;
            ++counter[buf[0]];
        }
        for (const auto& data : counter) {
            auto leafNode = std::make_shared<Node>();
            leafNode->weight = data.second;
            leafNode->value = data.first;
            nodes.push(leafNode);
        }
        std::shared_ptr<Node> aim_node = nullptr;
        while(!nodes.empty()) {
            auto first = nodes.top();
            nodes.pop();
            if (nodes.empty()) {
                aim_node = first;
                break;
            }
            auto second = nodes.top();
            nodes.pop();
            std::shared_ptr<Node> combined_node = std::make_shared<Node>();
            combined_node->setLeft(first);
            combined_node->value = '-';
            combined_node->setRight(second);
            combined_node->weight = first->weight+second->weight;
            nodes.push(combined_node);
        }
        if (aim_node) {
            std::cout << "aim node found " << aim_node->weight << std::endl;
             /**
              * then raise the second stage. make up the table
                  */
            makeUpTable(aim_node,encoderMap);
            fseek(input,0,SEEK_END);
            int len = static_cast<int>(ftell(input));
            calSaveRate(encoderMap,counter,len);

            /**
             * 第三阶段，开始写文件
             * 1. 写入magichref
             * 2. 写入1字节的padding. 原因因为可能是原始的可能总的字节加上去没有1字节,padding的单位是
             * 3. 写入table 
             *  4. 大端顺序，size,表的item数量
             *  5. 表： 一个字节+可变长度的字符串，以0为结尾
             *  6. 剩下的为编码过的内容去除padding,根据码表阅读，并且还原内容；如果在还原的过程中有出错的话；则抛出参数错误的异常 
             */
            fwriteBLInt(output,magichref);
            std::cout << "write magichref successfully" << std::endl;
            int totalsize = 9;
            totalsize += encoderMap.size;
            std::map<char, std::string> codes;
            std::map<std::string,char> codes_reverse;
            for (int i=0;i<encoderMap.map.size();i++) {
                if (!encoderMap.map[i].empty()) {
                    // 需要注意的是这里记录的code是偏移过后的量，因此需要加上128,才能回复到原始的数值
                    char code = static_cast<signed char>(i-128);
                    codes[code] = encoderMap.map[i];
                    codes_reverse[encoderMap.map[i]] = code;
                    totalsize+=1;
                    totalsize+=encoderMap.map[i].length()+1;
                }
            }
            std::cout << "pre cal some totalsize" << std::endl;
            totalsize*=8;
            // 以上for循环计算出了头所占用的bit数量
            char placeholder = '-';
            // 计算完毕的时候，placeholder位置的值需要重写
            fwrite(&placeholder,1,1,output);
            std::cout << "write placeholder" << std::endl;
            // 写入table的大小
            fwriteBLInt(output,encoderMap.size);
            std::cout << "write table size" << std::endl;
            for(const auto& item : codes) {
                fwriteChar(output,item.first);
                fwriteStr(output,item.second);
            }
            std::cout << "write huffleman table content" << std::endl;
            // 重新读取input的内容，且将其编码成huffleman代码的形式
            fseek(input,0,SEEK_SET);
            char buf[1];
            bitstream stream;
            while (!feof(input)) {
                int readed = fread(buf,1,1,input);
                if (readed == 0) break;
                const auto& encodedform = codes[buf[0]];
                if (encodedform.empty()) {
                    // 没有在码表中找到对应的编码之后的输出，编码阶段有错误；抛出异常；
                    fclose(output);
                    fclose(input);
                    throw std::logic_error("empty encode map!!");
                } else {
                    // 找到了对应的码表
                    stream.put(encodedform);
                    if (stream.bitCount() >= 8) {
                        // 目前已经有一个byte的数据了，读出并输出到文件
                        const auto& item = stream.poll();
                        totalsize+=8;
                        assert(item.second == 0); // 理论上来说这个时候不应该有padding
                        // 问题，在某一个位置写一个值，是会覆盖当前索引上的值，还是
                        fwriteChar(output,item.first);
                    }
                }
            }
            std::cout << "write encoded huffleman table content" << std::endl;

            auto rewritePaddingPlaceHolder = [](FILE* output, char paddingCount) {
                auto pos = ftell(output);
                fseek(output,4,SEEK_SET);
                fwriteChar(output,paddingCount);   
                fseek(output,pos,SEEK_SET);
            };
            if (stream.bitCount() != 0) {
                // 有额外的padding.且有额外的内容
                totalsize += stream.bitCount();
                const auto& item = stream.poll();
                rewritePaddingPlaceHolder(output,item.second);
                fwriteChar(output,item.first);
            } else {
                // 重新写padding的占位符号的值，然后回到当前的位置,刚刚好没有padding
                rewritePaddingPlaceHolder(output,0);
            }
            fclose(input);
            fclose(output);
            std::cout << "write file successfully" << std::endl;
            // 文件写入完成
        } else {
            std::cout << " aim node was not triggered" << std::endl;
        }
    }

    struct FileHandle {
        FileHandle(FILE* h):handle(h) {}
        ~FileHandle() {
            if (handle) {
                fclose(handle);
                handle = nullptr;
            }
        }
    private:
        FILE* handle;
    };
    // input为经过huffle编码过后的文件,output为编码结果输出的huffle
    static void decode(FILE* input, FILE* output) {
        // 关于文件头的压缩算法应该是还可以再压缩的，但文件本算法也只是实验性质的，所以无需压缩
        // 使用这两个类只是为了在退出该函数的时候自动关闭文件
        FileHandle no_care_(input);
        FileHandle no_care__(output);
        
        // read header of the file.
        // this block would try read 4 bytes. and return a bool indicate this file is a valid hf file
        // if it is. the file pointer would go forward a step more.
        // else the file would rewind to its origin pos as if it is not readed...
        auto readHeader = [&]()->bool {
            char buf[4];
            auto cur = ftell(input);
            fseek(input,0,SEEK_END);
            auto len = ftell(input);
            if (len - cur > 4) {
                // read header
                fseek(input,cur,SEEK_SET);
                auto readed = fread(buf,4,1,input);
                if (readed < 4) return false;
                int data = *(reinterpret_cast<int*>(buf));
                if (isLe()) {
                    return data == magichref_rev;
                } else {
                    return data == magichref;
                }
            } else {
                fseek(input,cur,SEEK_SET);
                return false;
            }
        };
        if (readHeader()) {
            // read padding. as huffleman encoding suggests. it may not charge the bit stream at proper boundary.
            int padding;
            // indicate this is a valid hf file. then read the table.
            table encoderTable;
             
        }
    }

    static void makeUpTable(std::shared_ptr<Node> node, table& aim) {
        std::string content = "0";
        /**
         * 目前先暂时使用额外的函数去实现
         */ 
        makeUpTable(node.get(),aim,content);
        aim.size = 0;
        for (auto& data : aim.map) {
            if (!data.empty()) {
                ++aim.size;
            }
        }
        std::cout << " aim.size " << aim.size << std::endl;
    }

    static void calSaveRate(const table& aim, const std::map<char,int>& counter, const int fileLen) {
        int total = 0;
        for(auto& data: counter) {
            total += aim.map[data.first+128].size()*data.second;
        }
        std::cout << "total byte count " << total/8 << " " << "diff " << fileLen-total/8 << " fileLen " << fileLen << std::endl;
    }

    static void makeUpTable(Node* node, table& aim, std::string& content) {
        if (node) {
            auto isLeaf = [](const Node* node) { return node->left == nullptr && node->right == nullptr; };
            if (isLeaf(node)) {
                std::cout << "value = " << static_cast<int>(node->value) << " code = " << content << std::endl;
                aim.map[static_cast<int>(node->value)+128] = content;
            } else {
                content+="0";
                if (node->left && node->left.get()) {
                    makeUpTable(node->left.get(),aim,content);
                }
                if (content.size() == 0) {
                    std::cout << "it seems no more can be erased" << std::endl;
                    exit(-1);
                }
                content.erase(content.end()-1);
                content+="1";
                if (node->right && node->right.get()) {
                    makeUpTable(node->right.get(),aim,content);
                }
                if (content.size() == 0) {
                    std::cout << "it seems no more can be erased" << std::endl;
                    exit(-1);
                }
                content.erase(content.end()-1);
            } 
        }
    }

private:
   

    static void fwriteBLInt(FILE* pf, int32_t data) {
        if (isLe()) {
            auto swapBlock = [](char* data, int p1, int p2) {
                const auto tmp = data[p1];
                data[p1] = data[p2];
                data[p2] = tmp;
            };
            char* dataArray = reinterpret_cast<char*>(&data);
            swapBlock(dataArray,0, 3);
            swapBlock(dataArray,1,2);
            fwrite(dataArray,4,1,pf);
            std::cout << "fwriteBLInt " << data << " successfully" << std::endl;
        } else {
            char* data = reinterpret_cast<char*>(&data);
            fwrite(data,4,1,pf);
        } 
    }

    static void fwriteChar(FILE* pf, char data) {
        fwrite(&data,1,1,pf);
    }

    static void fwriteStr(FILE* pf, const std::string& str) {
        fwrite(str.c_str(),str.length()+1,1,pf);
    }
public:
 static bool isLe() {
    static union u_le{
        int data = 0x12345678;
        char seq;
    } u_le_s;
        return 0x78 == u_le_s.seq;
    } 
    friend bool operator> (const std::shared_ptr<Huffleman::Node> a, const std::shared_ptr<Huffleman::Node> b);
};


bool operator> (const std::shared_ptr<Huffleman::Node> a, const std::shared_ptr<Huffleman::Node> b) {
        if (a== nullptr && b == nullptr) throw std::logic_error("should not be nullptr, why???");
        return a->weight > b->weight;
}