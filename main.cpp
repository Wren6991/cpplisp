#include <iostream>

#include "tokenizer.h"
#include "parser.h"
#include "proc.h"


std::shared_ptr<environment> global_env;
environment *global_env_ptr;
std::shared_ptr<environment> env;

void setupGlobals()
{
    global_env_ptr = new environment();
    global_env = std::shared_ptr<environment>(global_env_ptr);
    global_env->vars["PRINT"] = proc_print;
    global_env->vars["WRITE"] = proc_write;
    global_env->vars["EVAL"] = proc_eval_arglist;   //arglist interface to actual eval function.
    global_env->vars["+"] = proc_add;
    global_env->vars["-"] = proc_subtract;
    global_env->vars["*"] = proc_multiply;
    global_env->vars["/"] = proc_divide;
    global_env->vars["="] = proc_equal;
    global_env->vars["<"] = proc_less;
    global_env->vars[">"] = proc_greater;
    global_env->vars["<="] = proc_less_equal;
    global_env->vars[">="] = proc_greater_equal;
    global_env->vars["AND"] = proc_and;
    global_env->vars["OR"] = proc_or;
    global_env->vars["NOT"] = proc_not;
    global_env->vars["IF"] = proc_if;
    global_env->vars["BEGIN"] = proc_begin;
    global_env->vars["DEFINE"] = proc_define;
    global_env->vars["QUOTE"] = proc_quote;
    global_env->vars["QUASI-QUOTE"] = proc_quasi_quote;
    global_env->vars["LAMBDA"] = proc_lambda;
    global_env->vars["MACRO"] = proc_macro;
    global_env->vars["MACROEXPAND-1"] = proc_macroexpand;
    global_env->vars["LISTVARS"] = proc_listvars;
    global_env->vars["TAGBODY"] = proc_tagbody;
    global_env->vars["GO"] = proc_go;
    global_env->vars["CONS"] = proc_cons;
    global_env->vars["CAR"] = proc_car;
    global_env->vars["CDR"] = proc_cdr;
    global_env->vars["LIST"] = proc_list;
    global_env->vars["SETQ"] = proc_setq;
    global_env->vars["NREVERSE"] = proc_nreverse;
    global_env->vars["LET"] = proc_let;
    global_env->vars["NIL"] = cell(v_symbol, "NIL");
    global_env->vars["TRUE"] = cell(v_symbol, "TRUE");
    env = global_env;

    std::string runOnStart =
    "(define defmacro (macro (name vars &rest body) `(define ,name (macro ,vars ,@body))))"
    "(defmacro defun (name vars &rest body) `(define ,name (lambda ,vars ,@body)))"
    "(defmacro while (expr &rest body) `(tagbody top (if ,expr (begin ,@body (go top))) end))"
    "(defmacro when (cond &rest body) `(if ,cond (begin ,@body)))"
    "(defmacro unless (cond &rest body) `(if (not ,cond) (begin ,@body)))"
    "(defmacro mapcar (func list) `(let ((acc '()) (lis ,list) (fun ,func))"
    " (tagbody top"
    "  (when (cdr lis)"
    "        (push acc (fun (car lis)))"
    "        (setq lis (cdr lis))"
    "        (go top))"
    "  (nreverse acc))))"
    "(defmacro push (list arg) `(setq ,list (cons ,arg ,list)))";
    parser p(tokenize(runOnStart));
    try
    {
        while (true)
            proc_eval(p.read());
    }
    catch (exception e) {}
}

int countBrackets(std::vector<token> tokens)
{
    int count = 0;
    std::vector<token>::iterator iter = tokens.begin();
    for(; iter != tokens.end(); iter++)
    {
        if (iter->type == t_lparen)
            count++;
        else if (iter->type == t_rparen)
            count--;
    }
    return count;
}


int main()
{
    setupGlobals();
    while (true)
    {
        char progstring[5000];
        std::cout << "> ";
        std::cin.getline(progstring, 5000, '\n');
        std::vector<token> tokens = tokenize(progstring);
        while (countBrackets(tokens) > 0)
        {
            std::cout << ">> ";          //we are expecting more input!
            std::cin.getline(progstring, 5000, '\n');
            std::vector<token> extratokens = tokenize(progstring);
            tokens.reserve(tokens.size() + extratokens.size());
            tokens.insert(tokens.end(), extratokens.begin(), extratokens.end());
        }
        parser p(tokens);
        try
        {
            cell expr = p.read();
            cell result = proc_eval(expr);
            std::cout << "==> " << toString(result) << "\n\n";
        }
        catch (exception e)
        {
            std::cout << e.err << "\n";
        }
        catch (tag t)
        {
            std::cout << "Error: tried to go to unmatched tag \"" << t.str << "\"\n";
        }
    }
    return 0;
}
