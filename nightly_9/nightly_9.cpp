/*
SPSC RingBuffer（含测试）
数据流：
生产线程 -> RingBuffer -> 消费线程
*/
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>
#include <cstdlib>

constexpr size_t N = 16;

class RingBuffer{
public:
    RingBuffer() : head(0),tail(0) {}

    //写入一个字节
    bool push(char data){
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % N;

        //如果下一位置等于tail，说明满了
        //空条件： head == tail
        //满条件： (head + 1) % N == tail
        if(next_head == tail.load(std::memory_order_acquire))
            return false;

        buffer[current_head] = data;

        head.store(next_head,std::memory_order_release);
        return true;
    }

    //读取一个字节
    bool pop(char &data){
        size_t current_tail = tail.load(std::memory_order_relaxed);

        //如果head = tail，说明空
        if(current_tail == head.load(std::memory_order_acquire))
            return false;

        data = buffer[current_tail];
        size_t next_tail = (current_tail+ 1) % N;
        tail.store(next_tail,std::memory_order_release);

        return true;
    }

private:
    char buffer[N];
    std::atomic<size_t> head;           //只生产者修改
    std::atomic<size_t> tail;           //只消费者修改
};

int main(){
    RingBuffer rb;

    std::thread producer([&](){
        while(true){
            char data = '0' + rand() % 10;

            if(rb.push(data)){
                std::cout << "write:" << data << std::endl;
            }
            else{
                std::cout << "buffer full!!" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1000000));
        }
    });

    std::thread consumer([&](){
        while(true){
            char data = '0' + rand() % 10;

            if(rb.pop(data)){
                std::cout << "read:" << data << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(15000000));
        }
    });   
    
    producer.join();
    consumer.join();
}
