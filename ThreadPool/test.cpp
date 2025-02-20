#include"threadpool.h"
#include<iostream>
#include<thread>
#include<chrono>
#include<memory>//unique_ptr是包含在memory库中
// #define uLong long long
using namespace std;
using uLong=unsigned long long;
/*
某些场景可能会返回任务的返回值
比如求和:
thread1:1+...1000
thred2:1001+...2000;
thread:2001+...3000
*/
class Any1;
class MyTask:public Task{
   public:
    MyTask(int begin,int end):begin_(begin),end_(end)
    {

    }
     Any1 run()//自定义线程函数
     {
      //Q1:如何设计run的返回值接受任意类型,参考java中的Object思想，它是所有类型的基类==>C++17中的Any1类型:Any1 run()
      cout<<"tid"<<this_thread::get_id()<<"begin!"<<endl;
      uLong sum=0;
      for(uLong i=begin_;i<=end_;i++)
         sum+=i;
      this_thread::sleep_for(chrono::seconds(2));
      cout<<"tid"<<this_thread::get_id<<"finish!"<<endl;
      // cerr<<this_thread::get_id<<endl;
        return sum;
     }
     private:
       int begin_;
       int end_;
};
//Any1类型:接受任意数据的类型
// class Any1{
//   public:
//     template<typename T>
//       Any1(T data):base_(new Derive<T>(data))//基类指针指向派生类的对象
//       {}
//       template<typename T>//Any1类型接收任意其他类型的数据,将Any1对象里面的数据提取出来
//       T cast_()
//       {
//         //如何从base_指向的Derive<T>对象中取出data成员
//         Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//基类指针==>派生类指针 RTTI
//         if(pd==nullptr)
//         {
//           // cout<<"change wrongly!"<<endl;
//           throw "type is wrong!";
//         }
//         return pd->data_;
//       }

//   private:
//      class Base{//基类
//       public:
//         virtual ~Base()=default;//继承结构中,基类的派生对象如是在堆上创建,派生类的就无法调用基类的析构函数 
//      };
//      template<typename T>
//      class Derive:public Base{//派生类
//        public:
//           Derive(T data):data_(data)
//           {}
//         private:
//           T data_;

//      };
//     private:
//       unique_ptr<Base>base_;//定义基类的指针,可以指向任意对象
// };

int main()
{
  //ThreadPool对象析构以后,如何将线程池相关的资源全部回收
  ThreadPool pool;
  pool.setMode(PoolMode::MODE_CACHED);//设置模式
  pool.Start(4);
  // this_thread::sleep_for(chrono::seconds(5));
  // Result res=pool.submitTask(make_shared<MyTask>());//如何设计此处的Result机制
  /*此处task被执行完后，对象即被销毁*/
  // int sum=res.get().cast_<long>();
  // res.get().cast_<>();//返回的类型对象
  // pool.submitTask(make_shared<MyTask>());//自定义提交3个任务
  // pool.submitTask(make_shared<MyTask>());
  // pool.submitTask(make_shared<MyTask>());
  // getchar();//暂停，输入
  // return 0;
  Result res1=pool.submitTask(make_shared<MyTask>(1,1000000));
  Result res2=pool.submitTask(make_shared<MyTask>(10000001,2000000));
  Result res3=pool.submitTask(make_shared<MyTask>(20000001,3000000));
  uLong sum1=res1.getValue().cast_<uLong>();
  uLong sum2=res2.getValue().cast_<uLong>();
  uLong sum3=res3.getValue().cast_<uLong>();
  cout<<(sum1+sum2+sum3)<<endl;//Master-Slave线程模型,Master线程分解任务,分配给Slave,并等待Slave完成的结果由Master合并各个任务结果
  getchar();
  return 0;

  

}

















