//实现一个简单的语法解析器－－波兰表达式/前缀表达式

#include <mpc.h>

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
      lispy    : /^/ <operator><expr>+  /$/;       \
    ",
              Number, Operator, Expr, Lispy);

    /*打印版本和退出信息*/
    puts("Lispy Version 0.0.0.0.2");
    puts("Press Ctrl+c to Exit\n");

    //死循环
    while (1) {
        //打印提示
        fputs("Lispy > ", stdout);

        //读取一行输入
        fgets(input, 2048, stdin);

        //解析输入
        mpc_result_t r;
        //返回值为 1，失败为 0
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            //成功打印AST 抽象语法树
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            //打印失败信息
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    //清除解析器
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}

/*
gcc -std=c99 -Wall parsing.c mpc.c -o parsing  -lm
*/