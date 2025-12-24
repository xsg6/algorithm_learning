#include<iostream>
#include<functional>
//自由函数
int add(int a,int b)
{
    return a+b;
}
//函数对象
class Multiply
{
public:
    int operator()(int a,int b)
    {
        return a*b;
    }
};
//lambda函数
auto divide = [](int a,int b)->int
{
    return a/b;
};
int main()
{
    //使用std::function包装自由函数
    std::function<int(int,int)> func_add = add;
    std::cout<<"Addition: "<<func_add(3,5)<<std::endl;  
    //使用std::function包装函数对象
    std::function<int(int,int)> func_multiply = Multiply();
    std::cout<<"Multiplication: "<<func_multiply(4,6)<<std::endl;  
    //使用std::function包装lambda函数
    std::function<int(int,int)> func_divide = divide;
    std::cout<<"Division: "<<func_divide(8,2)<<std::endl;  
    return 0;
}