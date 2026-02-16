/*
 * @Author: lafia4real 1650374245@qq.com
 * @Date: 2026-01-27 19:17:22
 * @LastEditors: lafia4real 1650374245@qq.com
 * @LastEditTime: 2026-01-27 22:32:32
 * @FilePath: \nightly_3\nightly_3.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
模拟“中断更新标志位”主循环读取（用线程模拟即可）
*/
#include <iostream>
#include <stdio.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "include/nightly_4.hpp"

using namespace std;

/*
主循环(volatile版本)：轮询读取flag，读取到true时处理一次事件，然后清空标志位
*/
uint64_t main_loop_poll_volatile() {
    using namespace std::chrono_literals;

    uint64_t handled = 0;
    auto start = std::chrono::steady_clock::now();

    while (handled < N) {
        // 非原子：这两句之间 ISR 可能又 ++，会被你清零清掉 -> 丢事件
        uint32_t batch = g_evt_volatile;
        g_evt_volatile = 0;
        handled += batch;

        std::this_thread::sleep_for(1ms);

        if (std::chrono::steady_clock::now() - start > 5s) break;
    }
    return handled;
}

/*
主循环(atomic版本)：轮询读取flag，读取到true时处理一次事件，然后清空标志位
*/
uint64_t main_loop_poll_atomic() {
    using namespace std::chrono_literals;

    uint64_t handled = 0;
    auto start = std::chrono::steady_clock::now();

    while (handled < N) {
        // 原子：一次性“取走并清零”
        uint32_t batch = g_evt_atomic.exchange(0, std::memory_order_acq_rel);
        handled += batch;

        std::this_thread::sleep_for(1ms);

        if (std::chrono::steady_clock::now() - start > 5s) break;
    }
    return handled;
}

int main() {
    std::cout << "demo_1: volatile counter\n";
    {
        g_evt_volatile = 0;
        std::thread isr(isr_set_flag_volatile);
        uint64_t handled = main_loop_poll_volatile();
        isr.join();
        std::cout << "[volatile] expected=" << N << ", handled=" << handled << "\n";
    }

    std::cout << "demo_2: atomic counter\n";
    {
        g_evt_atomic.store(0, std::memory_order_relaxed);
        std::thread isr(isr_set_flag_atomic);
        uint64_t handled = main_loop_poll_atomic();
        isr.join();
        std::cout << "[atomic]   expected=" << N << ", handled=" << handled << "\n";
    }

    return 0;
}
