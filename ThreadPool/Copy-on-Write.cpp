#include <iostream>
#include <memory>
#include <string>
class COWString {/*shared_ptrʵ��дʱ���Ƶļ���*/
public:
    COWString(const std::string& str) : data_(std::make_shared<std::string>(str)) {}

    // ��ȡ�ַ���
    const std::string& get() const {
        return *data_;
    }

    // д���ַ�����ʵ��дʱ���ƣ�
    void set(const std::string& newStr) {
        // ������ü������� 1����ʾ������Ҫ��������
        if (!data_.unique()) {
            data_ = std::make_shared<std::string>(*data_);//ͬʱָ����ֱ���޸����ݣ���Ҫ�ȱ��ݣ����޸�
        }//*data_�ǽ�����,����ֱ���޸�����,���Ƕ�ȡstd::string������
        *data_ = newStr; // ������Ψһӵ�еģ�����ֱ���޸�,�������������󣬼�����һ
    }
   

    // ��ӡ���ü����������ã�
    void printRefCount() const {
        std::cout << "Reference count: " << data_.use_count() << std::endl;
    }

private:
    std::shared_ptr<std::string> data_; // ��������ַ�������
};

int main() {
    COWString cowStr1("Hello, World!");
    COWString cowStr2 = cowStr1; // ��������

    std::cout << "Original string in cowStr1: " << cowStr1.get() << std::endl;
    cowStr1.printRefCount(); // ���ü���Ϊ 2

    cowStr2.set("Hello, C++!"); // cowStr2 дʱ���ƣ������¸���
    cowStr1.printRefCount(); // ���ü���Ϊ 1
    cowStr2.printRefCount(); // ���ü���Ϊ 1

    std::cout << "Modified string in cowStr2: " << cowStr2.get() << std::endl;
    std::cout << "Unchanged string in cowStr1: " << cowStr1.get() << std::endl;

    return 0;
}


