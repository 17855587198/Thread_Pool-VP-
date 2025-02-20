任意的其他类型==>怎么定义(template)
一个类型指向其他任意的类型==>基类

Any==>Base*==>{
    Derive:public Base
      template data;
}
//虚函数不能是模板类的返回值