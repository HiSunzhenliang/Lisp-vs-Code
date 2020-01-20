//实现一个简单的语法解析器－－波兰表达式/前缀表达式

//#include 'mpc.h'

extern "C" {
#include <mpc.h>
}

#include <iostream>

/*申明输入缓冲区*/
static char input[2048];

int main() {
    //创建解析器
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    //定义解析器
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                    \
      number   : /-?[0-9]+/;                       \
      operator : '+'|'-'|'*'|'/';                  \
      expr     : <number>|'('<operator><expr>+')'; \
      lispy    : /^/ <operator><expr>  /$/;        \
    ",
              Number, Operator, Expr, Lispy);

    /*打印版本和退出信息*/
    std::cout << "Lispy Version 0.0.0.0.1\n";
    std::cout << "Press Ctrl+c to Exit\n\n";

    //死循环
    while (true) {
        //打印提示
        std::cout << "Lispy > ";

        //读取一行输入
        std::cin.getline(input, 2048);

        //解析输入
        mpc_result_t r;
        //返回值为 1，失败为 0
        if (mpc_parse("<getline>", input, Lispy, &r)) {
            //成功打印AST 抽象语法树
            mpc_ast_print((mpc_ast_t*)r.output);
            mpc_ast_delete((mpc_ast_t*)r.output);
        } else {
            //打印失败信息
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        //回显给用户
        std::cout << "No you're a " << input << std::endl;
    }

    //清除解析器
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}

/*

g++ -std=c++11 parsing.cpp -o parsing

*/