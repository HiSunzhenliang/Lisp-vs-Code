//实现一个简单的语法解析器－－波兰表达式/前缀表达式

#include <mpc.h>

/*申明输入缓冲区*/
static char input[2048];

//错误情况枚举
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// lisp value枚举
enum { LVAL_NUM, LVAL_ERR };

// lisp value结构体定义
typedef struct Lval {
    int type;
    long num;
    int err;
} lval;

//创建新数字类型
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

//创建新错误类型
lval lval_err(long x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

//打印结构体
void lval_print(lval v) {
    switch (v.type) {
        case LVAL_NUM:
            printf("%li", v.num);
            break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division By Zero!");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operato!");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
            break;
    }
    printf("\n");
}

//判断计算符号
lval eval_op(lval x, char* op, lval y) {
    //如果x、y有值为ERR,则return它
    if (x.type == LVAL_ERR) {
        return x;
    }
    if (y.type == LVAL_ERR) {
        return y;
    }
    if (strcmp(op, "+") == 0) {
        return lval_num(x.num + y.num);
    }
    if (strcmp(op, "-") == 0) {
        return lval_num(x.num - y.num);
    }
    if (strcmp(op, "*") == 0) {
        return lval_num(x.num * y.num);
    }
    if (strcmp(op, "/") == 0) {
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }
    return lval_err(LERR_BAD_OP);
}

//递归求值函数
lval eval(mpc_ast_t* t) {
    //如果是叶子为数字
    if (strstr(t->tag, "number")) {
        //返回前先转换，转换后判断转换是否出错
        errno = 0;  // erro记录代码错误的int值
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE
                   ? lval_num(x)
                   : lval_err(LERR_BAD_NUM);  // ERANGE为数学结果无法表示
    }
    //第二个孩子为操作符
    char* op = t->children[1]->contents;
    //将第三个孩子值存入x,递归
    lval x = eval(t->children[2]);
    //遍历剩下孩子
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}

int main() {
    //创建解析器
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    //定义解析器,语法规则
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
            // //成功打印AST 抽象语法树
            // mpc_ast_print(r.output);

            lval result = eval(r.output);
            // printf("%li\n", result);
            lval_print(result);

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