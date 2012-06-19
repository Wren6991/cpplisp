#ifndef PROC_H_INCLUDED
#define PROC_H_INCLUDED

#include "parser.h"

std::string toString(const cell& x);

cell proc_print(const cell &x);
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
cell proc_begin(const cell &arglist);
cell proc_lambda(const cell &arglist);
cell proc_listvars(const cell &_);
cell proc_let(const cell &arglist);
cell proc_eval(const cell &x);
cell proc_eval_arglist(const cell &arglist);
cell proc_cons(const cell &arglist);
cell proc_car(const cell &arglist);
cell proc_cdr(const cell &arglist);
cell proc_setq(const cell &arglist);

#endif // PROC_H_INCLUDED
