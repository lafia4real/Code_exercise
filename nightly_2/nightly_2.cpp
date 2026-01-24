/*
use-after-free(释放后使用)
程序编写与修复
*/
#include <iostream>
#include <stdio.h>

using namespace std;

void use_after_free(){
    //new int(42) 的意思是：在堆（自由存储区）上分配一块足够放下 1 个 int 的内存，
    //并用值 42 初始化这个 int，然后返回指向它的指针（类型是 int*）。
    int* ptr = new int(42);
    // delete ptr;          修复ing

    int* anyway = new int(1);
    *ptr = 100;
    std::cout << *ptr << endl;

    cout << "anyway = " << *anyway << endl;

    delete ptr;             //修复ing
    delete anyway;
    ptr = nullptr;          //修复ing
}

int main(){
    use_after_free();
}
