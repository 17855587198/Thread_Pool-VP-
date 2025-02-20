#include <iostream>
#include <memory>
#include <string>
class COWString {/*shared_ptr实现写时复制的技术*/
public:
    COWString(const std::string& str) : data_(std::make_shared<std::string>(str)) {}

    // 读取字符串
    const std::string& get() const {
        return *data_;
    }

    // 写入字符串（实现写时复制）
    void set(const std::string& newStr) {
        // 如果引用计数大于 1，表示共享，需要创建副本
        if (!data_.unique()) {
            data_ = std::make_shared<std::string>(*data_);//同时指向不能直接修改数据，需要先备份，再修改
        }//*data_是解引用,不会直接修改数据,而是读取std::string的内容
        *data_ = newStr; // 现在是唯一拥有的，可以直接修改,经过创建副本后，计数减一
    }
   

    // 打印引用计数（调试用）
    void printRefCount() const {
        std::cout << "Reference count: " << data_.use_count() << std::endl;
    }

private:
    std::shared_ptr<std::string> data_; // 管理共享的字符串数据
};

int main() {
    COWString cowStr1("Hello, World!");
    COWString cowStr2 = cowStr1; // 共享数据

    std::cout << "Original string in cowStr1: " << cowStr1.get() << std::endl;
    cowStr1.printRefCount(); // 引用计数为 2

    cowStr2.set("Hello, C++!"); // cowStr2 写时复制，创建新副本
    cowStr1.printRefCount(); // 引用计数为 1
    cowStr2.printRefCount(); // 引用计数为 1

    std::cout << "Modified string in cowStr2: " << cowStr2.get() << std::endl;
    std::cout << "Unchanged string in cowStr1: " << cowStr1.get() << std::endl;

    return 0;
}


