/*
帧解析器 v1（前导+长度+CRC+超时）
*/
#include <iostream>
#include <cstdint>
#include <chrono>
#include <thread>

class SimpleUartParser{
public:
    static const uint8_t HEAD1 = 0XAA;
    static const uint8_t HEAD2 = 0X55;
    static const uint8_t MAX_LENGTH = 32;

    //冒号表示成员变量初始化，成员变量为timeout
    SimpleUartParser(uint32_t timeout_us) : timeout(timeout_us){
        reset();
    }

    bool feed(uint8_t byte,uint32_t now_ms){
        if(now_ms - last_time > timeout){
            reset();
        }
        last_time = now_ms;

        //状态机CRC校验
        switch(state){
            case 0:
                if(byte == HEAD1)
                    state = 1;
                break;
            case 1:
                if(byte == HEAD2)
                    state = 2;
                else
                    state = 0;
                break;
            case 2:
                length = byte;
                if(length > MAX_LENGTH){
                    reset();
                }
                else{
                    index = 0;
                    checksum = length;
                    state = 3;
                }               
                break;
            case 3:
                buffer[index++] = byte;
                checksum += byte;
                if(index >= length)
                    state = 4;
                break;
            case 4:
                if((checksum & 0xFF) == byte){
                    state = 0;
                    return true;
                }
                else{
                    reset();
                }
                break;
        }

        return false;
    }
    uint8_t* data() {return buffer;}
    uint8_t size() {return length;}

private:
    void reset(){
        state = 0;
        length = 0;
        index = 0;
        checksum = 0;
    }

    uint8_t state;
    uint8_t length;
    uint8_t index;
    uint8_t checksum;
    uint8_t buffer[MAX_LENGTH];
    uint32_t timeout;
    uint32_t last_time = 0;
};

//获取当前时间（毫秒）
uint32_t now_ms(){
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch())
    .count();
}

int main(){
    SimpleUartParser parser(1000);

    //构造一帧数据
    //AA 55(帧头)
    //05(长度) 
    //01 02 03 04 05(数据) 
    //CRC(长度+数据的二进制结果)
    uint8_t frame[]{
        0xAA,
        0x55,
        0x05,
        0x01,0x02,0x03,0x04,0x05,
        0x14
    };

    for(auto byte : frame){
        if(parser.feed(byte,now_ms())){
            std::cout << "frame received data : ";
            for(int i = 0;i < parser.size();i++)
                std::cout << int(parser.data()[i]) << " ";
            std::cout << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
