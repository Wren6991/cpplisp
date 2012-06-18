#include <iostream>
#include <map>
#include <sstream>

#include "tokenizer.h"
#include "parser.h"


std::shared_ptr<environment> global_env;
std::shared_ptr<environment> env;


const cell nil(v_symbol, "NIL");



cell proc_eval(const cell &x);

std::string toString(const cell& x)
{
    if (x.type == v_number)
    {
        std::stringstream ss;
        ss << x.n;
        return ss.str();
    }
    else if (x.type == v_string || x.type == v_symbol)
        return x.str;
    else if (x.type == v_list)
    {
        std::stringstream ss;
        ss << "(" << (x.car? toString(*x.car) : "") << " " << (x.cdr? toString(*x.cdr) : "") << ")";
        return ss.str();
    }
    else if (x.type == v_proc)
    {
        std::stringstream ss;
        ss << "<native function @" << (int)x.proc << ">";
        return ss.str();
    }
    else if (x.type == v_function)
    {
        return "<interpreted function>";
    }
    return "NIL";
}

cell proc_print(const cell &x)
{
    cell output = proc_eval(*x.car);
    std::cout << toString(output) << "\n";
    return output;
}

cell proc_define(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        throw exception("Error: function define expects two arguments.");
    if (arglist.car->type != v_symbol)
        throw(exception("Error: tried to define non-symbol."));
    cell result = proc_eval(*arglist.cdr->car);
    global_env->vars[arglist.car->str] = result;
    return result;
}

cell proc_add(const cell &x)
{
    double total = 0;
    const cell *iter = &x;
    while(iter && iter->car)
    {
        total += proc_eval(*iter->car).n;
        iter = iter->cdr;
    }
    return cell(total);
}

cell proc_subtract(const cell &x)
{
    double total = 0;
    const cell *iter = &x;
    bool first = true;
    while(iter && iter->car)
    {
        total -= proc_eval(*iter->car).n;
        iter = iter->cdr;
        if (first)
        {
            total = -total;
            first = false;
        }
    }
    return cell(total);
}

cell proc_multiply(const cell &x)
{
    double total = 1;
    const cell *iter = &x;
    while(iter && iter->car)
    {
        total *= proc_eval(*iter->car).n;
        iter = iter->cdr;
    }
    return cell(total);
}

cell proc_and(const cell &x)
{
    cell result;
    const cell *iter = &x;
    while(iter && iter->car)
    {
        result = proc_eval(*iter->car);
        iter = iter->cdr;
        if (result == nil)
            break;
    }
    return result;
}

cell proc_if(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell cond = proc_eval(*arglist.car);
    if (!arglist.cdr)
        return nil;
    if (!(cond == nil))
    {
        return arglist.cdr->car? proc_eval(*arglist.cdr->car) : nil;
    }
    else
    {
        if (!arglist.cdr->cdr)
            return nil;
        return arglist.cdr->cdr->car? proc_eval(*arglist.cdr->cdr->car) : nil;
    }
}

cell proc_equal(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell first = proc_eval(*arglist.car);
    const cell *iter = arglist.cdr;

    while (iter && iter->car)
    {
        cell result = proc_eval(*iter->car);
        if (!(result == first))
            return nil;
        iter = iter->cdr;
    }

    return cell(v_symbol, "TRUE");
}

cell proc_less(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        return nil;
    if (proc_eval(*arglist.car).n < proc_eval(*arglist.cdr->car).n)
        return cell(v_symbol, "TRUE");
    else
        return nil;
}

cell proc_greater(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        return nil;
    if (proc_eval(*arglist.car).n > proc_eval(*arglist.cdr->car).n)
        return cell(v_symbol, "TRUE");
    else
        return nil;
}

cell proc_less_equal(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        return nil;
    if (proc_eval(*arglist.car).n <= proc_eval(*arglist.cdr->car).n)
        return cell(v_symbol, "TRUE");
    else
        return nil;
}

cell proc_greater_equal(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        return nil;
    if (proc_eval(*arglist.car).n >= proc_eval(*arglist.cdr->car).n)
        return cell(v_symbol, "TRUE");
    else
        return nil;
}

cell proc_quote(const cell &arglist)
{
    if (arglist.car)
        return *arglist.car;
    else
        return nil;
}

cell proc_begin(const cell &arglist)
{
    const cell *iter = &arglist;
    cell result;
    while (iter && iter->car)
    {
        result = proc_eval(*iter->car);
        iter = iter->cdr;
    }
    return result;
}

cell proc_lambda(const cell &arglist)
{
    if(!arglist.car || arglist.car->type != v_list)
        throw(exception("Error: missing argument list for lambda"));
    if (!arglist.cdr || !arglist.cdr->car)
        throw(exception("Error: missing function body for lambda"));

    const cell *iter = arglist.car;
    while (iter && iter->car)
    {
        if (iter->car->type != v_symbol)
            throw(exception("Error: argument names must be symbols (lambda)."));
        iter = iter->cdr;
    }

    cell func_cell(v_function);
    func_cell.car = arglist.car;
    func_cell.cdr = arglist.cdr;
    func_cell.env = env;
    return func_cell;
}

cell proc_listvars(const cell &_)
{
    std::cout << "Listing variables.\n";
    std::map<std::string, cell>::iterator iter;
    for (iter = global_env->vars.begin(); iter != global_env->vars.end(); iter++)
    {
        std::cout << iter->first << ": ";
        for (int i = iter->first.size(); i < 16; i++)
            std::cout << " ";
        std::cout << toString(iter->second) << "\n";
    }
    return cell();
}


cell proc_eval(const cell &x)
{
    if (x.type == v_string || x.type == v_number || x.type == v_function)
        return x;
    else if (x.type == v_symbol)
    {
        //std::cout << "fetching var " << x.str << "\n";
        return env->get(x.str);
    }
    else if (x.type == v_list)
    {
        if (!x.car)
            return nil;
        cell head = proc_eval(*x.car);
        if (head.type == v_proc)
            return head.proc(x.cdr? *x.cdr : nil);
        if (head.type == v_function)
        {
            std::shared_ptr<environment> oldenv = env;
            std::shared_ptr<environment> newenv(new environment(head.env));
            const cell *name_iter = head.car;
            const cell *arg_iter = x.cdr;
            while (arg_iter && arg_iter->car && name_iter && name_iter->car)
            {
                newenv->vars[name_iter->car->str] = proc_eval(*arg_iter->car);
                arg_iter = arg_iter->cdr;
                name_iter = name_iter->cdr;
            }
            env = newenv;
            cell result = proc_eval(*head.cdr->car);
            env = oldenv;
            return result;
        }

        throw(exception("Error: attempt to call non-proc"));

    }
    else
    {
        throw(exception("Unrecognised cell type! (eval)"));
    }
}

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
        //std::cout << "Entered program: " << progstring << "\n";
        std::vector<token> tokens = tokenize(progstring);

        /*for (int i = 0; i < tokens.size(); i++)
            std::cout << tokens[i].type << ": " << tokens[i].value << "\n";*/
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
