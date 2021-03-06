#ifndef PROC_H_INCLUDED
#define PROC_H_INCLUDED

#include "parser.h"

std::string toString(const cell& x);

cell proc_print(const cell &x);
cell proc_write(const cell &x);
cell proc_define(const cell &arglist);
cell proc_add(const cell &x);
cell proc_subtract(const cell &x);
cell proc_multiply(const cell &x);
cell proc_divide(const cell &arglist);
cell proc_and(const cell &x);
cell proc_or(const cell &x);
cell proc_not(const cell &x);
cell proc_if(const cell &arglist);
cell proc_equal(const cell &arglist);
cell proc_less(const cell &arglist);
cell proc_greater(const cell &arglist);
cell proc_less_equal(const cell &arglist);
cell proc_greater_equal(const cell &arglist);
cell proc_quote(const cell &arglist);
cell proc_quasi_quote(const cell &arglist);
cell proc_unquote(const cell &arglist);
cell proc_begin(const cell &arglist);
cell proc_lambda(const cell &arglist);
cell proc_macro(const cell &arglist);
cell proc_macroexpand(const cell &arglist);
cell proc_listvars(const cell &_);
cell proc_nreverse(const cell &arglist);
cell proc_let(const cell &arglist);
cell proc_tagbody(const cell &arglist);
cell proc_go(const cell &arglist);
cell proc_eval(const cell &x);
cell proc_eval_arglist(const cell &arglist);
cell proc_cons(const cell &arglist);
cell proc_car(const cell &arglist);
cell proc_cdr(const cell &arglist);
cell proc_list(const cell &arglist);
cell proc_setq(const cell &arglist);

struct tag_sym
{
    std::string str;
    tag_sym() {}
    tag_sym(std::string str_) {str = str_;}
};

#endif // PROC_H_INCLUDED
