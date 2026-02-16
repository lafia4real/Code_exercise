#include <iostream>
#include <stdio.h>
#include <atomic>
#include <chrono>
#include <thread>

/*
volatile:用于演示，而volatile并非线程同步工具
这段代码可能会出现问题
*/
extern volatile uint32_t g_evt_volatile;

/*
atomic：推荐使用
*/
extern std::atomic<uint32_t> g_evt_atomic;

//事件总数
inline constexpr uint32_t N = 5000000;

void isr_set_flag_volatile();
void isr_set_flag_atomic();
uint64_t main_loop_poll_volatile();
uint64_t main_loop_poll_atomic();

