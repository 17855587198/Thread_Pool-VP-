#include<iostream>
#include<functional>
#include<memory>
#include<thread>
// using namespace std;
int add(int x,int y)//绑定普通成员函数
{
    return x+y;
}
class Print{
    public:
       void printMsg(const std::string &mg)
        {
            std::cout<<"mgs is"<<mg<<std::endl;
        }
};
struct adder{//仿函数
    int operator()(int x,int y)const{
        return x+y;
    }
};

int main()
{
    std::cout<<"main-thread"<<std::this_thread::get_id<<std::endl;
    std::cout<<"sub-thread"<<add(1,2)<<std::endl;
    auto f=std::bind(add,10,std::placeholders::_1);//占位符留下1
    std::cout<<f(5)<<std::endl;
    Print *p;
    auto f1=std::bind(&Print::printMsg,&p,std::placeholders::_1);
    adder ar;
    auto f2=std::bind(ar,100,std::placeholders::_1);
    std::cout<<f2(3)<<std::endl;

    return 0;
}