/*
 * @Author: lafia4real 1650374245@qq.com
 * @Date: 2026-01-27 19:17:22
 * @LastEditors: lafia4real 1650374245@qq.com
 * @LastEditTime: 2026-01-27 22:30:52
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
volatile:用于演示，而volatile并非线程同步工具
这段代码可能会出现问题
*/
volatile uint32_t g_evt_volatile = false;

/*
atomic：推荐使用
*/
std::atomic<uint32_t> g_evt_atomic{false};

/*
模拟ISR（volatile版本），每隔一段时间把flag置1，表示事件发生/数据就绪
*/
void isr_set_flag_volatile() {
    for (uint32_t i = 0; i < N; ++i) {
        ++g_evt_volatile; // 非原子 RMW，和主线程“读+清零”会打架 -> 丢
    }
}

/*
模拟ISR（atomic版本），每隔一段时间把flag置1，表示事件发生/数据就绪
*/
void isr_set_flag_atomic() {
    for (uint32_t i = 0; i < N; ++i) {
        g_evt_atomic.fetch_add(1, std::memory_order_relaxed);
    }
}
