#include<iostream>

int main()
{
    int a=10;
    int m=0;
    int *p=&a;
    int &b=a;
    *p=20;
    if("a=2")
    {
        std::cout<<"111"<<std::endl;

    }
    std::cout<<a<<b<<std::endl;
    b=15;
    std::cout<<a<<b<<std::endl;
    int array[5]={};
    int *p1=array;//ָ���sizeof��ȡ����ϵͳλ��
    int (&q)[5]=array;//����ָ������
    std::cout<<array<<" "<<&array<<" "<<sizeof(p1)<<" "<<sizeof(q)<<std::endl;
    int &&d=25;//��ֵ����
    const int &e=20;//�൱��:int temp=20;temp--->e
    return 0;
}


