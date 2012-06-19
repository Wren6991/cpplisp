#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>

#include "parser.h"
#include "proc.h"


extern std::shared_ptr<environment> global_env;
extern std::shared_ptr<environment> env;




const cell nil(v_symbol, "NIL");

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
        ss << "(";
        const cell *iter = &x;
        while (iter && iter->car)
        {
            ss << toString(*iter->car);
            iter = iter->cdr;
            if (iter && iter->car)
                ss << " ";
        }
        ss << ")";
        return ss.str();
    }
    else if (x.type == v_proc)
    {
        std::stringstream ss;
        ss << "<native function @" << std::hex << (int)x.proc << ">";
        return ss.str();
    }
    else if (x.type == v_function)
    {
        std::stringstream ss;
        ss << "<interpreted function @" << std::hex << (int)x.cdr->car << ">";
        return ss.str();
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

cell proc_divide(const cell &arglist)
{
    double total = 1;
    const cell *iter = &arglist;
    bool first = true;
    while(iter && iter->car)
    {
        total /= proc_eval(*iter->car).n;
        iter = iter->cdr;
        if (first)
        {
            total = 1.0 / total;
            first = false;
        }
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

cell proc_or(const cell &x)
{
    cell result;
    const cell *iter = &x;
    while(iter && iter->car)
    {
        result = proc_eval(*iter->car);
        iter = iter->cdr;
        if (!(result == nil))
            break;
    }
    return result;
}

cell proc_not(const cell &x)
{
    if (!x.car || proc_eval(*x.car) == nil)
        return cell(v_symbol, "TRUE");
    else
        return nil;
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

cell quasi_quote(const cell& x)
{
    if (x.type != v_list)
        return x;
    if (x.car && x.car->type == v_symbol && x.car->str == "UN-QUOTE")
        return x.cdr? proc_unquote(*x.cdr) : nil;
    cell head(v_list);
    cell *tail = &head;
    const cell *iter = &x;
    while (iter && iter->car)
    {
        tail->car = new cell();
        *tail->car = quasi_quote(*iter->car);
        tail->cdr = new cell(v_list);
        tail = tail->cdr;
        iter = iter->cdr;
    }
    return head;
}

cell proc_quasi_quote(const cell &arglist)      //arglist wrapper (actual implementation is recursive, so we need to strip out the arglist semantics)
{
    if (!arglist.car)
        return nil;
    else
        return quasi_quote(*arglist.car);
}

cell proc_unquote(const cell &arglist)
{
    if (arglist.car)
        return proc_eval(*arglist.car);
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

cell proc_cons(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        return nil;
    cell *car = new cell;
    cell *cdr = new cell;
    *car = proc_eval(*arglist.car);
    *cdr = proc_eval(*arglist.cdr->car);
    return cell(car, cdr);
}

cell proc_car(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell cons = proc_eval(*arglist.car);
    if (cons.type != v_list)
        throw(exception("Error: tried to take car of non-list."));
    if (!cons.car)
        return nil;
    return *cons.car;
}

cell proc_cdr(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell cons = proc_eval(*arglist.car);
    if (cons.type != v_list)
        throw(exception("Error: tried to take cdr of non-list."));
    if (!cons.cdr)
        return nil;
    return *cons.cdr;
}

cell proc_setq(const cell &arglist)
{
    if (!arglist.car || !arglist.cdr || !arglist.cdr->car)
        throw(exception("Error: missing arguments to setq"));
    if (arglist.car->type != v_symbol)
        throw(exception("Error: tried to setq non-symbol."));
    cell val = proc_eval(*arglist.cdr->car);
    env->get(arglist.car->str) = val;
    return val;
}

cell proc_let(const cell &arglist)
{
    if (!arglist.car || arglist.car->type != v_list)
        throw(exception("Error: function let expects assignment list as first argument."));
    std::shared_ptr<environment> newenv(new environment(env));
    const cell *iter = arglist.car;
    while (iter && iter->car)
    {
        if (iter->car->type == v_symbol)
        {
            newenv->vars[iter->car->str] = nil;
        }
        else
        {
            if (iter->car->type != v_list || !iter->car->car || iter->car->car->type != v_symbol || !iter->car->cdr || !iter->car->cdr->car)        //in order: not a list || no first item || first item not symbol || no link to second item || second item has no value
                throw(exception("Error: let assignment must be symbol or symbol-value pair."));
            newenv->vars[iter->car->car->str] = proc_eval(*iter->car->cdr->car);
        }
        iter = iter->cdr;
    }
    if (!arglist.cdr || !arglist.cdr->car)
        throw(exception("Error: let is missing body."));
    std::shared_ptr<environment> oldenv = env;
    env = newenv;
    cell result = proc_eval(*arglist.cdr->car);
    env = oldenv;
    return result;
}


cell proc_eval_arglist(const cell &arglist)     // all procs take an uneval'd arg list, in order for functions such as quote to use the same interface (they don't eval their args):
{                                               // proc_eval_arglist is an interface that is called from LISP code, which unzips the argument list and passes it to eval.
                                                // proc_eval contains the actual eval implementation.
    const cell *iter = &arglist;
    cell result;
    while (iter && iter->car)
    {
        result = proc_eval(proc_eval(*iter->car));      //eval the argument, then perform the _requested_ eval function.
        iter = iter->cdr;
    }
    return result;
}

cell proc_eval(const cell &x)
{
    bool listvars = false;
    if (listvars)
        proc_listvars(cell());
    if (x.type == v_string || x.type == v_number || x.type == v_function)
        return x;
    else if (x.type == v_symbol)
    {
        //std::cout << "fetching var " << x.str << ": " << toString(env->get(x.str)) << "\n";
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
