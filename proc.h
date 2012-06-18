#ifndef PROC_H_INCLUDED
#define PROC_H_INCLUDED

#include "parser.h"

std::string toString(const cell& x);

cell proc_print(const cell &x);
cell proc_define(const cell &arglist);
cell proc_add(const cell &x);
cell proc_subtract(const cell &x);
cell proc_multiply(const cell &x);
cell proc_and(const cell &x);
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
cell proc_eval(const cell &x);

#endif // PROC_H_INCLUDED
