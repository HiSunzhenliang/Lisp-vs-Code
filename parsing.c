//实现一个简单的语法解析器－－波兰表达式/前缀表达式

#include <mpc.h>

/*申明输入缓冲区*/
static char input[2048];

//错误情况枚举
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// lisp value枚举
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

// lisp value结构体定义
typedef struct Lval {
    int type;
    long num;
    // Error和Symbol有string类型数据
    char* err;
    char* sym;
    //统计列表个数
    int count;
    //指向率lval*列表
    lval** cell;
} lval;

//创建新数字类型
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

//创建新错误类型
lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

//创建新符号类型
lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

//创建空s表达式类型
lval* laval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

//释放lval*
void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            //不改变
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
            //释放列表所有
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

// lval_add，这个函数将表达式的子表达式计数加一
lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}
//象语法树(abstract syntax tree)转换为一个 S-表达式
lval* lval_read(mpc_ast_t* t) {
    /* If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }
    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }

    /* If root (>) or sexpr then create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->tag, "regex") == 0) {
            continue;
        }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

//打印 S-表达式
void lval_print(lval* v);
void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        if (i != (v->count - 1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

//打印结构体
void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            printf("%li", v->num);
            break;
        case LVAL_ERR:
            if (v->err == LERR_DIV_ZERO) {
                printf("Error: Division By Zero!");
            }
            if (v->err == LERR_BAD_OP) {
                printf("Error: Invalid Operato!");
            }
            if (v->err == LERR_BAD_NUM) {
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
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    //定义解析器的语法规则
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                    \
      number   : /-?[0-9]+/;                       \
      symbol   : '+'|'-'|'*'|'/';                  \
      expr     : '('<expr>*')'                     \
      expr     : <number>|'('<operator><expr>+')'; \
      lispy    : /^/ <operator><expr>+  /$/;       \
    ",
              Number, Symbol, Sexpr, Expr, Lispy);

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
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;
}

/*
gcc -std=c99 -Wall parsing.c mpc.c -o parsing  -lm
*/