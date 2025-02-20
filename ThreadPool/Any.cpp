#include<iostream>
#include<functional>
#include<memory>
using namespace std;
class Any{//��Ҫ˼���Ǹ����ָ�����ָ�����������Ķ���
  public:
      Any(){}
      ~Any(){}//unique_ptr����ֵ�����뿽����delete��,��������ֵ��������
      Any(const Any&)=delete;
      Any& operator=(const Any&other)=delete;
      Any(Any&&)=default;//��ֵ�Ŀ����������
      Any& operator=(const Any&other)=default;//��ֵ�ĸ�ֵ
      //Any(T data):base_(new Derive<T>(data))//����ָ��ָ��������Ķ���,Derive���ڶ��϶�̬�����ڴ�
       template<typename T>
       Any(T data):base_(make_unique<Derive<T>>(data))//make_unique���Զ�����ָ����ڴ�
      {}
  private:
     class Base{//����
      public:
        virtual ~Base()=default;//�̳нṹ��,������������������ڶ��ϴ���,��֤��Any����ʱ����������������Derive<T>����������
        /*����̳��˸���Ĺ��캯������������,�����������������ʱ��ʹ��virtual����ʱ,�޷�ȷ���Ƿ���Ҫ�������������������*/
     };
     template<typename T>
     class Derive:public Base{//������
       public:
          Derive(T data):data_(data)
          {}
        private:/*Derive�а�����ͬ���͵����ݶ���,Base��һ�������,base_����ָ���κμ̳���Base�����������,ʹ��Any���Դ洢����ͬ���͵�Derive<T>����*/
          T data_;

     };
    private:
      unique_ptr<Base>base_;//��������ָ��,����ָ���������
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
//     A *a=new B();//����ָ��ָ������Ķ���
//     delete a;
// }
/*
unique_ptr������ָ��,����̬������ڴ� 
*/
int main()
{
    Any a(10);
    Any b(22.11);
    unique_ptr<Any> c=make_unique<Any>(10);//����Any���ڲ�ʹ����unique_ptr<Base>��̬�������ڴ�,���Բ���Ҫʹ��new������ָ��
    cout<<sizeof(*c)<<endl;
    cout<<sizeof(a)<<endl;
    cout<<sizeof(b)<<endl;
    /*
    �˴�����Ľ�����Ƕ���8,��Ϊsizeof���ط���ľ�̬��С�����Ƕ�̬��С;
    c ��һ�� std::unique_ptr<Any>��
    ͨ�� make_unique ������һ�� unique_ptr<Any>��
    ������һ�� Any ���󡣵���ʹ�� sizeof(*c) ʱ��
    ��ʵ��������� Any ���Ͷ���Ĵ�С��
    ���� Any ��Ķ�������һ�� unique_ptr<Base> ���͵ĳ�Ա base_��
    ����һ������ָ�룬��������̬�ڴ档
    unique_ptr ����Ĵ�Сͨ����һ��ָ��Ĵ�С��ͨ���� 8 �ֽڣ�ȡ����ƽ̨�ͱ���������
    */
    // test();
    return 0;
}

