//实现一个简单的语法解析器－－波兰表达式/前缀表达式

#include "mpc.h"

/*申明输入缓冲区*/
static char input[2048];

// //错误情况枚举
// enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// lisp value枚举
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

// lisp value结构体定义
typedef struct lval {
    int type;
    long num;
    // Error和Symbol有string类型数据
    char* err;
    char* sym;
    //统计列表个数
    int count;
    //指向率lval*列表
    struct lval** cell;
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
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

//创建Q-表达式
lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
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
        case LVAL_QEXPR:
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

// lval_add，这个函数将表达式的子表达式计数加一
lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

// lval_pop 函数将所操作的 S-表达式的第 i
// 个元素取出，并将在其后面的元素向前移动填补空缺，使得这个
// S-表达式不再包含这个元素。然后将取出的元素返回。
lval* lval_pop(lval* v, int i) {
    //列表中寻找元素i
    lval* x = v->cell[i];
    //平移内存中i之后的元素
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval*) * (v->count - i - 1));
    //减少count
    v->count--;
    //重新分配列表内存容量
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_join(lval* x, lval* y) {
    //每个cell列表，y转移到中
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    //删除空y，返回x
    lval_del(y);
    return x;
}
//它将取出元素之后剩下的列表删除
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
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

//打印结构体，非换行打印
void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            printf("%li", v->num);
            break;
        case LVAL_ERR:
            printf("Error: %s", v->err);
            break;
        case LVAL_SYM:
            printf("%s", v->sym);
            break;
        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
        case LVAL_QEXPR:
            lval_expr_print(v, '{', '}');
            break;
    }
}
//换行打印
void lval_println(lval* v) {
    lval_print(v);
    putchar('\n');
}

lval* lval_eval(lval* v);

// list 接收一个或者多个参数，返回一个包含所有参数的Q-表达式
lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

// Q-表达式操作符 head接受一个Q-表达式，返回一个包含其第一个元素的Q-表达式
lval* builtin_head(lval* a) {
    //检查错误情况
    //参数不唯一
    if (a->count != 1) {
        lval_del(a);
        return lval_err("Function 'head' passed too many arguments!");
    }
    //传入参数不是Q-表达式
    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'head' passed incorrect types!");
    }
    // Q空值传入
    if (a->cell[0]->count == 0) {
        lval_del(a);
        return lval_err("Function 'head' passed {}!");
    }
    //取出首元素
    lval* v = lval_take(a, 0);
    //删除剩余元素
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

// tail 接受一个Q-表达式，返回一个除首元素外的Q-表达式
lval* builtin_tail(lval* a) {
    //检查错误情况
    //参数不唯一
    if (a->count != 1) {
        lval_del(a);
        return lval_err("Function 'head' passed too many arguments!");
    }
    //传入参数不是Q-表达式
    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'head' passed incorrect types!");
    }
    // Q空值传入
    if (a->cell[0]->count == 0) {
        lval_del(a);
        return lval_err("Function 'head' passed {}!");
    }
    //取出首元素
    lval* v = lval_take(a, 0);
    //删除首元素
    lval_del(lval_pop(v, 0));

    return v;
}

// eval 接受一个Q-表达式，将其看做一个S-表达式，并运行
lval* builtin_eval(lval* a) {
    //参数不唯一
    if (a->count != 1) {
        lval_del(a);
        return lval_err("Function 'head' passed too many arguments!");
    }
    //传入参数不是Q-表达式
    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'head' passed incorrect types!");
    }

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

// join 接受一个或者多个Q-表达式，返回一个将其连在一起的Q-表达式
lval* lval_join(lval* x, lval* y);
lval* builtin_join(lval* a) {
    //传入参数不是Q-表达式
    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'head' passed incorrect types!");
    }
    //将y中元素依次弹出并添加进x中，然后将y删除，返回x。
    lval* x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

//求值函数
lval* builtin_op(lval* a, char* op) {
    //确保输入参数类型都为数字
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }
    //弹出第一个元素进行判断
    lval* x = lval_pop(a, 0);
    //操作符为‘-’且只有一个元素,取反
    if (strcmp(op, "-") == 0 && a->count == 0) {
        x->num = -x->num;
    }
    /*如果还有更多的参数，它就不断地从列表中取出，
    将其和之前的计算结果一起进行相应的数学运算。如果
    做除法时遇到被除数为零的情况，就将临时变量
    x 和 y 以及参数列表删除，并返回一个错误。
    如果没有错误，参数列表最终会被删除，并返回一个新的表达式。*/
    while (a->count > 0) {  // pop下一个元素
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) {
            return lval_num(x->num + y->num);
        }
        if (strcmp(op, "-") == 0) {
            return lval_num(x->num - y->num);
        }
        if (strcmp(op, "*") == 0) {
            return lval_num(x->num * y->num);
        }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division By Zero!");
                break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }
    lval_del(a);
    return x;
}

//索引调用symbol方法
lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) {
        return builtin_list(a);
    }
    if (strcmp("head", func) == 0) {
        return builtin_head(a);
    }
    if (strcmp("tail", func) == 0) {
        return builtin_tail(a);
    }
    if (strcmp("join", func) == 0) {
        return builtin_join(a);
    }
    if (strcmp("eval", func) == 0) {
        return builtin_eval(a);
    }
    if (strstr("+-/*", func)) {
        return builtin_op(a, func);
    }
    lval_del(a);
    return lval_err("Unkonwn Function!");
}

// s-表达式求值，lval* 作为输入，通过某种方式将其转化为新的 lval* 并输出
lval* lval_eval_sexpr(lval* v) {
    //计算孩子节点
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    //错误检查
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    //空表达式
    if (v->count == 0) {
        return v;
    }

    //单表达式
    if (v->count == 1) {
        return lval_take(v, 0);
    }

    //确保第一个元素是符号,非符号则报错
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression Does not start with symbol!");
    }

    //调用 buildin with operator
    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}

//计算s-表达式
lval* lval_eval(lval* v) {
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(v);
    }
    //其他lval类型保持不变
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

//将语法树(abstract syntax tree)转换为一个 S-表达式
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
    if (strstr(t->tag, "qexpr")) {
        x = lval_qexpr();
    }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, "}") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, "{") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->tag, "regex") == 0) {
            continue;
        }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

// //判断计算符号
// lval eval_op(lval x, char* op, lval y) {
//     //如果x、y有值为ERR,则return它
//     if (x.type == LVAL_ERR) {
//         return x;
//     }
//     if (y.type == LVAL_ERR) {
//         return y;
//     }
//     if (strcmp(op, "+") == 0) {
//         return lval_num(x.num + y.num);
//     }
//     if (strcmp(op, "-") == 0) {
//         return lval_num(x.num - y.num);
//     }
//     if (strcmp(op, "*") == 0) {
//         return lval_num(x.num * y.num);
//     }
//     if (strcmp(op, "/") == 0) {
//         return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num /
//         y.num);
//     }
//     return lval_err(LERR_BAD_OP);
// }

// //递归求值函数
// lval eval(mpc_ast_t* t) {
//     //如果是叶子为数字
//     if (strstr(t->tag, "number")) {
//         //返回前先转换，转换后判断转换是否出错
//         errno = 0;  // erro记录代码错误的int值
//         long x = strtol(t->contents, NULL, 10);
//         return errno != ERANGE
//                    ? lval_num(x)
//                    : lval_err(LERR_BAD_NUM);  // ERANGE为数学结果无法表示
//     }
//     //第二个孩子为操作符
//     char* op = t->children[1]->contents;
//     //将第三个孩子值存入x,递归
//     lval x = eval(t->children[2]);
//     //遍历剩下孩子
//     int i = 3;
//     while (strstr(t->children[i]->tag, "expr")) {
//         x = eval_op(x, op, eval(t->children[i]));
//         i++;
//     }
//     return x;
// }

int main() {
    puts("++++++++++++++++++++++++++++++++++++");
    //创建解析器
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    // puts("++++++++++++++++++++++++++++++++++++\n");

    //定义解析器的语法规则
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                            \
      number   : /-?[0-9]+/;                               \
      symbol   : \"list\"|\" head \"|\" tail \"            \
               |\" join \"|\" eval \"|'+'|'-'|'*'|'/';     \
      sexpr    : '(' <expr>* ')' ;                         \
      qexpr    : '{' <expr>* '}' ;                         \
      expr     : <number> | <symbol> | <sexpr> | <qexpr>;  \
      lispy    : /^/ <expr>*  /$/;                         \
    ",
              Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    // puts("++++++++++++++++++++++++++++++++++++\n");
    /*打印版本和退出信息*/
    puts("Lispy Version 0.0.0.0.5");
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

            // lval result = eval(r.output);
            // // printf("%li\n", result);
            // lval_print(result);

            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);

            mpc_ast_delete(r.output);
        } else {
            //打印失败信息
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    //清除解析器
    mpc_cleanup(5, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}

/*
gcc -std=c99 -Wall parsing.c mpc.c -o parsing
*/