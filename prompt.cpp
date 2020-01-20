#include <iostream>

/*申明输入缓冲区*/
static char input[2048];
// char* inputPointer = input;

int main() {
    /*打印版本和退出信息*/
    std::cout << "Lispy Version 0.0.0.0.1\n";
    std::cout << "Press Ctrl+c to Exit\n\n";

    //死循环
    while (true) {
        //打印提示
        std::cout << "Lispy > ";

        //读取一行输入
        std::cin.getline(input, 2048);

        //回显给用户
        std::cout << "No you're a " << input << std::endl;
    }

    return 0;
}

/*

g++ -std=c++11 prompt.cpp -o prompt

*/