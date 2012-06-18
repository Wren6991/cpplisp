#include <iostream>
#include <map>
#include <sstream>

#include "tokenizer.h"
#include "parser.h"
#include "proc.h"


std::shared_ptr<environment> global_env;
std::shared_ptr<environment> env;

void setupGlobals()
{
    global_env = std::shared_ptr<environment>(new environment());
    global_env->vars["PRINT"] = proc_print;
    global_env->vars["EVAL"] = proc_eval;
    global_env->vars["+"] = proc_add;
    global_env->vars["-"] = proc_subtract;
    global_env->vars["*"] = proc_multiply;
    global_env->vars["="] = proc_equal;
    global_env->vars["<"] = proc_less;
    global_env->vars[">"] = proc_greater;
    global_env->vars["<="] = proc_less_equal;
    global_env->vars[">="] = proc_greater_equal;
    global_env->vars["AND"] = proc_and;
    global_env->vars["IF"] = proc_if;
    global_env->vars["BEGIN"] = proc_begin;
    global_env->vars["DEFINE"] = proc_define;
    global_env->vars["QUOTE"] = proc_quote;
    global_env->vars["LAMBDA"] = proc_lambda;
    global_env->vars["LISTVARS"] = proc_listvars;
    global_env->vars["NIL"] = nil;
    global_env->vars["TRUE"] = cell(v_symbol, "TRUE");
    env = global_env;
}


int main()
{
    setupGlobals();
    while (true)
    {
        char progstring[500];
        std::cout << "> ";
        std::cin.getline(progstring, 500, '\n');

        std::vector<token> tokens = tokenize(progstring);
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
    }
    return 0;
}
