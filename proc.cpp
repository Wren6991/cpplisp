#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>

#include "parser.h"
#include "proc.h"


std::shared_ptr<environment> global_env;
std::shared_ptr<environment> env;




const cell nil(v_symbol, "NIL");

std::string toString(const cell& x)
{
    switch(x.type)
    {
        case v_string:
        {
            std::stringstream ss;
            ss << "\"" << x.str << "\"";
            return ss.str();
        }
        case v_symbol:
            return x.str;
        case v_number:
        {
            std::stringstream ss;
            ss << x.n;
            return ss.str();
        }
        case v_list:
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
        case v_proc:
        {
            std::stringstream ss;
            ss << "<native function @" << std::hex << (int)x.proc << ">";
            return ss.str();
        }
        case v_function:
        {
            std::stringstream ss;
            ss << "<interpreted function @" << std::hex << (int)x.cdr->car << ">";
            return ss.str();
        }
        case v_macro:
        {
            std::stringstream ss;
            ss << "<macro @" << std::hex << (int)x.cdr->car << ">";
            return ss.str();
        }
        default:
            return "NIL";
    }
}

cell proc_print(const cell &x)
{
    cell output = proc_eval(*x.car);
    std::cout << toString(output) << "\n";
    return output;
}

cell proc_write(const cell &x)
{
    cell output = proc_eval(*x.car);
    std::cout << toString(output);
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
        if (first && iter->cdr)
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

cell quasi_quote(const cell& x, bool &signalSplice)
{
    if (x.type != v_list)
        return x;
    if (x.car && x.car->type == v_symbol)
    {
        if (x.car->str == "SPLICE-UN-QUOTE")
            signalSplice = true;
        if (signalSplice || x.car->str == "UN-QUOTE")
            return x.cdr? proc_unquote(*x.cdr) : nil;
    }
    cell head(v_list);
    cell *tail = &head;
    cell *pretail = 0;      //cell before tail - so we can splice in ,@forms
    bool splicetail;
    const cell *iter = &x;
    while (iter && iter->car)
    {
        splicetail = false;
        tail->car = new cell();
        *tail->car = quasi_quote(*iter->car, splicetail);
        if (splicetail)
        {
            if (tail->car->type != v_list)
                throw(exception("Error: attempt to splice non-list (,@)"));
            if (pretail)                                //if we have a pointer to the cons before the tail:
            {
                pretail->cdr = tail->car;               //append the spliced list to the pretail
                delete tail;
                tail = pretail->cdr;
            }
            else
            {                                           //if we don't have a pretail, we only have a head, and it's currently empty.
                head = *head.car;                       //replace the head cons with its contents.
            }
            while (tail && tail->car)      //cdr to the end of the list so more stuff can be appended.
            {
                pretail = tail;
                tail = tail->cdr;
            }
        }
        else                                            //if the tail has not just been spliced in, we need to add a new cons to the end.
        {
            tail->cdr = new cell(v_list);
            pretail = tail;
            tail = tail->cdr;
        }
        iter = iter->cdr;
    }
    return head;
}

cell proc_quasi_quote(const cell &arglist)      //arglist wrapper (actual implementation is recursive, so we need to strip out the arglist semantics)
{
    if (!arglist.car)
        return nil;
    bool _ = false;
    return quasi_quote(*arglist.car, _);
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

cell proc_macro(const cell &arglist)
{
    if(!arglist.car || arglist.car->type != v_list)
        throw(exception("Error: missing argument list for macro"));
    if (!arglist.cdr || !arglist.cdr->car)
        throw(exception("Error: missing macro body for macro"));

    const cell *iter = arglist.car;
    while (iter && iter->car)
    {
        if (iter->car->type != v_symbol)
            throw(exception("Error: argument names must be symbols (macro)."));
        iter = iter->cdr;
    }

    cell macro_cell(v_macro);
    macro_cell.car = arglist.car;
    macro_cell.cdr = arglist.cdr;
    return macro_cell;
}

cell expand_macro(const cell& macro, const cell& arglist)
{
    env = std::shared_ptr<environment>(new environment(env));       //push a new closure for the arguments.
    const cell *arg_iter = &arglist;
    const cell *name_iter = macro.car;
    while (arg_iter && arg_iter->car && name_iter && name_iter->car)
    {
        if (name_iter->car->str == "&REST")
        {
            if (!(name_iter->cdr && name_iter->cdr->car && name_iter->cdr->car->type == v_symbol))
                throw(exception("Error: no symbol provided for macro &rest argument name"));
            env->vars[name_iter->cdr->car->str] = *arg_iter;
            break;
        }
        env->vars[name_iter->car->str] = *arg_iter->car;
        arg_iter = arg_iter->cdr;
        name_iter = name_iter->cdr;
    }
    cell expandedval = proc_eval(*macro.cdr->car);
    env = env->parent;      //pop the argument closure.
    return expandedval;
}

cell proc_macroexpand(const cell &arglist)
{
    cell macro;
    if (!arglist.car || (macro = proc_eval(*arglist.car)).type != v_macro)
        throw(exception("Error: expected macro as first argument to macroexpand."));
    return expand_macro(macro, arglist.cdr? *arglist.cdr : cell(v_list));
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
    if (cons.type != v_list || !cons.car)
        return nil;
    return *cons.car;
}

cell proc_cdr(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell cons = proc_eval(*arglist.car);
    if (cons.type != v_list || !cons.cdr)
        return nil;
    return *cons.cdr;
}

cell proc_list(const cell &arglist)
{
    cell head(v_list);
    cell *tail = &head;
    const cell *iter = &arglist;
    while (iter && iter->car)
    {
        tail->car = new cell();
        *tail->car = proc_eval(*iter->car);
        tail->cdr = new cell(v_list);
        tail = tail->cdr;
        iter = iter->cdr;
    }
    return head;
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

cell proc_tagbody(const cell &arglist)
{
    std::map <std::string, int> tagindices;
    std::vector <const cell*> expressions;
    const cell *iter = &arglist;
    while (iter && iter->car)
    {
        if (iter->car->type == v_symbol)
            tagindices[iter->car->str] = expressions.size();
        else
            expressions.push_back(iter->car);
        iter = iter->cdr;
    }
    cell result;
    int exprindex = 0;
    while (exprindex < expressions.size())
    {
        try
        {
            result = proc_eval(*expressions[exprindex]);
            exprindex++;
        }
        catch (tag_sym t)
        {
            if (tagindices.find(t.str) == tagindices.end())
                throw(t);                                   //doesn't belong to this tag body - pass it on to the next, or to the REPL if it isn't caught.
            else
                exprindex = tagindices[t.str];
        }
    }
    return result;
}

cell proc_go(const cell &arglist)
{
    if (!arglist.car || arglist.car->type != v_symbol)
        throw(exception("Error: expected symbol as argument to go."));
    throw(tag_sym(arglist.car->str));
}

cell proc_nreverse(const cell &arglist)
{
    cell head;
    if (!arglist.car || (head = proc_eval(*arglist.car)).type != v_list)
        throw(exception("Error: expected list as argument to nreverse."));
    cell *last = new cell(v_list);
    cell *tail = new cell(v_list);
    *tail = head;
    while (tail && tail->car)
    {
        cell *next = tail->cdr;
        tail->cdr = last;
        last = tail;
        tail = next;
    }
    env->get(arglist.car->str) = *last;
    return *last;
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
    std::shared_ptr<environment> oldenv = env;
    env = newenv;
    cell result;
    iter = arglist.cdr;
    while (iter && iter->car)
    {
        result = proc_eval(*iter->car);
        iter = iter->cdr;
    }
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
                if (name_iter->car->str == "&REST")
                {
                    if (!(name_iter->cdr && name_iter->cdr->car && name_iter->cdr->car->type == v_symbol))
                        throw(exception("Error: expected name for &rest parameter"));
                    cell head(v_list);
                    cell *tail = &head;
                    while (arg_iter && arg_iter->car)
                    {
                        tail->car = new cell();
                        *tail->car = proc_eval(*arg_iter->car);
                        tail->cdr = new cell(v_list);
                        tail = tail->cdr;
                        arg_iter = arg_iter->cdr;
                    }
                    newenv->vars[name_iter->cdr->car->str] = head;
                    name_iter = name_iter->cdr->cdr;
                    break;                              //skip the outer loop so we don't dereference the null car pointer.
                }
                newenv->vars[name_iter->car->str] = proc_eval(*arg_iter->car);
                arg_iter = arg_iter->cdr;
                name_iter = name_iter->cdr;
            }
            if (arg_iter && arg_iter->car)
                throw(exception("Error: too many arguments to function"));
            if (name_iter && name_iter->car)
                throw(exception("Error: too few arguments to function"));
            env = newenv;
            cell result;
            cell *body_iter = head.cdr;
            while (body_iter && body_iter->car)
            {
                result = proc_eval(*body_iter->car);
                body_iter = body_iter->cdr;
            }
            env = oldenv;
            return result;
        }
        if (head.type == v_macro)
            return proc_eval(expand_macro(head, x.cdr? *x.cdr : cell(v_list)));

        throw(exception("Error: attempt to call non-proc"));

    }
    else
    {
        throw(exception("Unrecognised cell type! (eval)"));
    }
}
