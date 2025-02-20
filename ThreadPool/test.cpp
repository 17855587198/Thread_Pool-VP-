#include"threadpool.h"
#include<iostream>
#include<thread>
#include<chrono>
#include<memory>//unique_ptr�ǰ�����memory����
// #define uLong long long
using namespace std;
using uLong=unsigned long long;
/*
ĳЩ�������ܻ᷵������ķ���ֵ
�������:
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
     Any1 run()//�Զ����̺߳���
     {
      //Q1:������run�ķ���ֵ������������,�ο�java�е�Object˼�룬�����������͵Ļ���==>C++17�е�Any1����:Any1 run()
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
//Any1����:�����������ݵ�����
// class Any1{
//   public:
//     template<typename T>
//       Any1(T data):base_(new Derive<T>(data))//����ָ��ָ��������Ķ���
//       {}
//       template<typename T>//Any1���ͽ��������������͵�����,��Any1���������������ȡ����
//       T cast_()
//       {
//         //��δ�base_ָ���Derive<T>������ȡ��data��Ա
//         Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//����ָ��==>������ָ�� RTTI
//         if(pd==nullptr)
//         {
//           // cout<<"change wrongly!"<<endl;
//           throw "type is wrong!";
//         }
//         return pd->data_;
//       }

//   private:
//      class Base{//����
//       public:
//         virtual ~Base()=default;//�̳нṹ��,������������������ڶ��ϴ���,������ľ��޷����û������������ 
//      };
//      template<typename T>
//      class Derive:public Base{//������
//        public:
//           Derive(T data):data_(data)
//           {}
//         private:
//           T data_;

//      };
//     private:
//       unique_ptr<Base>base_;//��������ָ��,����ָ���������
// };

int main()
{
  //ThreadPool���������Ժ�,��ν��̳߳���ص���Դȫ������
  ThreadPool pool;
  pool.setMode(PoolMode::MODE_CACHED);//����ģʽ
  pool.Start(4);
  // this_thread::sleep_for(chrono::seconds(5));
  // Result res=pool.submitTask(make_shared<MyTask>());//�����ƴ˴���Result����
  /*�˴�task��ִ����󣬶��󼴱�����*/
  // int sum=res.get().cast_<long>();
  // res.get().cast_<>();//���ص����Ͷ���
  // pool.submitTask(make_shared<MyTask>());//�Զ����ύ3������
  // pool.submitTask(make_shared<MyTask>());
  // pool.submitTask(make_shared<MyTask>());
  // getchar();//��ͣ������
  // return 0;
  Result res1=pool.submitTask(make_shared<MyTask>(1,1000000));
  Result res2=pool.submitTask(make_shared<MyTask>(10000001,2000000));
  Result res3=pool.submitTask(make_shared<MyTask>(20000001,3000000));
  uLong sum1=res1.getValue().cast_<uLong>();
  uLong sum2=res2.getValue().cast_<uLong>();
  uLong sum3=res3.getValue().cast_<uLong>();
  cout<<(sum1+sum2+sum3)<<endl;//Master-Slave�߳�ģ��,Master�̷ֽ߳�����,�����Slave,���ȴ�Slave��ɵĽ����Master�ϲ�����������
  getchar();
  return 0;

  

}

















