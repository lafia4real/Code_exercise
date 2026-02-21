/*
帧解析器 v1（前导+长度+CRC+超时）
修改为流式输入

原版程序：主线程“手动喂固定数组”
main 里是这样：
自己构造了一个完整 frame 数组
for(auto byte : frame) 逐个字节喂进去
sleep_for(50ms) 只是模拟慢一点到来
数据源、节奏、线程都由 main 控制，完全同步、确定
所以这更像“演示/单元测试”，不是实际 UART 那种“数据随时来”
*/
#include <iostream>
#include <cstdint>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

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

/*
现实中 UART 数据到来通常是：
异步来的（中断 / DMA 回调里来的）
CPU 可能正在忙别的事，不能在“数据到来那一刻”就立刻解析
所以会先把数据放到一个缓存里（ring buffer / queue），之后再解析

StreamProcessor 做的就是“模拟这个结构”：
pushData()：模拟“数据到来”（像 UART 中断/DMA 回调），把 byte 放进队列
processStream() 线程：模拟“后台解析任务”，不断从队列取 byte，喂给 parser.feed()
*/
class StreamProcessor{
public:
    StreamProcessor(SimpleUartParser& parser) : uartParser(parser),stop(false) {}

    void start(){
        processingThread = std::thread(&StreamProcessor::processStream,this);
    }

    void stopProcessing(){
        stop = true;
        cv.notify_all();
        if(processingThread.joinable()){
            processingThread.join();
        }
    }

    void pushData(uint8_t byte){
        {
            std::lock_guard<std::mutex> lock(mtx);
            dataQueue.push(byte);
        }
        cv.notify_one();
    }

private:
    void processStream(){
        while(!stop){
            uint8_t byte;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock,[this] {return !dataQueue.empty() || stop;});

                if(stop) break;

                byte = dataQueue.front();
                dataQueue.pop();
            }

            uint32_t now = now_ms();
            if(uartParser.feed(byte,now)){
                std::cout << "frame received data :";
                for(int i = 0;i < uartParser.size();i++)
                    std::cout << int(uartParser.data()[i]) << " ";
                std::cout << std::endl;
            }
        }
    }

    //获取当前时间（毫秒）
    uint32_t now_ms(){
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch())
        .count();
    }

    SimpleUartParser& uartParser;
    std::queue<uint8_t> dataQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread processingThread;
    bool stop;
};

int main(){
    SimpleUartParser parser(1000);
    StreamProcessor processor(parser);

    processor.start();

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
        // if(parser.feed(byte,now_ms())){
        //     std::cout << "frame received data : ";
        //     for(int i = 0;i < parser.size();i++)
        //         std::cout << int(parser.data()[i]) << " ";
        //     std::cout << std::endl;
        // }

        processor.pushData(byte);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));           //等待帧处理完成
    processor.stopProcessing();

    return 0;
}
