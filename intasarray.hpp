#include <cstdint>
class intasarray {
public:
    intasarray(int data = 0): _data(data){}
    // 将index指向的数据的bit设置为1,index从0-31,0为最高位，31为最低位,个人感觉符合平时的思维；
    void openbit(int index) {
        if (index >= 0 && index <= 31) {
            int mask = 1 << (INT_BIT_SIZE-index-1);
            _data |= mask;
        }
    }

    // 将index指向的数据的bit设置为0
    void clearbit(int index) {
        if (index >= 0 && index <= 31) {
            int mask = 1 << (INT_BIT_SIZE-index-1);
            _data &= ~mask;
        }
    }

    // 返回某一位是否打开
    bool isOpen(int index) {
        if (index >= 0 && index <= 31) {
            int mask = 1 << (INT_BIT_SIZE-index-1);
            return (_data & mask) != 0;
        }
        return false;
    }
private:
    int32_t _data;
    constexpr static int32_t INT_BIT_SIZE = 32;
};