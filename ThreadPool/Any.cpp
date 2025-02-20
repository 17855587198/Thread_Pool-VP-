#include<iostream>
#include<functional>
#include<memory>
using namespace std;
class Any{//主要思想是父类的指针可以指向任意的子类的对象
  public:
      Any(){}
      ~Any(){}//unique_ptr将左值引用与拷贝均delete了,可以有右值拷贝构造
      Any(const Any&)=delete;
      Any& operator=(const Any&other)=delete;
      Any(Any&&)=default;//右值的拷贝构造可用
      Any& operator=(const Any&other)=default;//右值的赋值
      //Any(T data):base_(new Derive<T>(data))//基类指针指向派生类的对象,Derive是在堆上动态分配内存
       template<typename T>
       Any(T data):base_(make_unique<Derive<T>>(data))//make_unique是自动管理指针的内存
      {}
  private:
     class Base{//基类
      public:
        virtual ~Base()=default;//继承结构中,基类的派生对象如是在堆上创建,保证在Any对象时能正常调用派生类Derive<T>的析构函数
        /*子类继承了父类的构造函数与析构函数,但是销毁派生类对象时不使用virtual声明时,无法确定是否需要调用派生类的析构函数*/
     };
     template<typename T>
     class Derive:public Base{//派生类
       public:
          Derive(T data):data_(data)
          {}
        private:/*Derive中包含不同类型的数据对象,Base是一个虚基类,base_可以指向任何继承自Base的派生类对象,使得Any可以存储管理不同类型的Derive<T>对象*/
          T data_;

     };
    private:
      unique_ptr<Base>base_;//定义基类的指针,可以指向任意对象
};
// class A{
//     public:
//       A()=default;
//       virtual ~A(){
//          cout<<"call the deconstruct of A";
//       }
// };
// class B:public A{
//      public:
//      ~B()
//      {
//          cout<<"call the deconstruct of B";
//      }
    
     
// };
// void test()
// {
//     A *a=new B();//父类指针指向子类的对象
//     delete a;
// }
/*
unique_ptr是智能指针,管理动态分配的内存 
*/
int main()
{
    Any a(10);
    Any b(22.11);
    unique_ptr<Any> c=make_unique<Any>(10);//由于Any的内部使用了unique_ptr<Base>动态管理了内存,所以不需要使用new来创建指针
    cout<<sizeof(*c)<<endl;
    cout<<sizeof(a)<<endl;
    cout<<sizeof(b)<<endl;
    /*
    此处输出的结果还是都是8,因为sizeof返回分配的静态大小而不是动态大小;
    c 是一个 std::unique_ptr<Any>，
    通过 make_unique 创建了一个 unique_ptr<Any>，
    它管理一个 Any 对象。当你使用 sizeof(*c) 时，
    它实际上是求解 Any 类型对象的大小，
    但是 Any 类的定义中有一个 unique_ptr<Base> 类型的成员 base_，
    它是一个智能指针，用来管理动态内存。
    unique_ptr 本身的大小通常是一个指针的大小（通常是 8 字节，取决于平台和编译器）。
    */
    // test();
    return 0;
}

