/*
函数签名构成：
①函数名，
②参数的数量，
③参数的类型（包括是否为引用/指针/const修饰等），
④成员函数的const/volatile限定符（如果是成员函数）

注意：
返回类型不属于函数签名的一部分。
参数的名称也不属于函数签名。
默认参数值不影响函数签名

void func(int a);           // 签名：func(int)
void func(double a);        // 签名：func(double)
void func(int a, int b);    // 签名：func(int, int)

class Test {
public:
    void show(int a) const; // 签名：show(int) const
    void show(int a);       // 签名：show(int)
};
*/
#include <iostream>
#include <stdio.h>

using namespace std;

//函数签名编写
void first(int a1,float b1){
    cout << "printf first("<< a1 <<","<< b1 <<")" << "\n" << endl;
}

void second(int *p,int* pi){
    if(p)   cout << "p = "<< p << endl;
    if(pi)   cout << "pi = "<< pi << endl;    
    cout << "printf second("<< p <<","<< pi <<")" << "\n" << endl;
}

void third(const int *c,int* const d){
    *d = 100;
    //*c = 50;  const在*左边表示不可改内容,const在*右边表示不可改指针
    cout << "printf third(),c = " << c << ",d = " << *d << "\n" << endl;
}

void fourth(int e,int f){
    cout << "printf fourth(),e = " << e << ",f = " << f << "\n" << endl;
}

//函数签名调用
int main(){
    first(1,2.0f);

    int p = 3;
    int pi = 4;
    second(&p,&pi);

    int c = 5;
    int d = 6;
    third(&c,&d);

    fourth(7,8);
}
