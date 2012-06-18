#include <stdlib.h>

#include "parser.h"

std::string toUpper(std::string str)
{
    unsigned int nchars = str.size();
    char c;
    for(unsigned int i = 0; i < nchars; i++)
    {
        c = str[i];
        if (c >= 'a' && c <= 'z')
            str[i] = c - 32;
        else
            str[i] = c;
    }
    return str;
}

cell::cell()
{
    type = v_symbol;
    str = "NIL";
}
cell::cell(cell_type type_)
{
    type = type_;
    switch(type)
    {
        case v_number:
            n = 0;
            break;
        case v_list:
            car = 0;
            cdr = 0;
            break;
        case v_proc:
            proc = 0;
            break;
        default:
            break;
    }
}

cell::cell(cell_type type_, std::string value_)
{
    type = type_;
    str = value_;
}

cell::cell(double n_)
{
    type = v_number;
    n = n_;
}

cell::cell(proc_t proc_)
{
    type = v_proc;
    proc = proc_;
}

cell::cell(cell* car_, cell* cdr_)
{
    type = v_list;
    car = car_;
    cdr = cdr_;
}

bool cell::operator==(const cell &c) const
{
    if (type != c.type)
        return false;
    switch(type)
    {
        case v_symbol:
        case v_string:
            return str == c.str;
        case v_number:
            return n == c.n;
        case v_list:
            return car == c.car && cdr == c.cdr;            //pointer comparison only - shallow comparison.
        default:
            return false;
    }
}


cell& environment::get(std::string name)
{
    std::map<std::string, cell>::iterator iter = vars.find(name);
    if (iter != vars.end())
        return iter->second;
    if (parent)
        return parent->get(name);
    return vars[name];
}


parser::parser(std::vector<token> tokens_)
{
    tokens = tokens_;
    ntokens = tokens.size();
    if (ntokens > 0)
        t = tokens[0];
    index = 0;
}

bool parser::accept(token_type type)
{
    if (index < ntokens && t.type == type)
    {
        last = t;
        if (++index < ntokens)
            t = tokens[index];
        return true;
    }
    return false;
}

bool parser::expect(token_type type)
{
    if (!accept(type))
    {
        throw(exception("Error: expected something around here somewhere"));
        return false;
    }
    else
    {
        return true;
    }
}

cell parser::read()
{
    if (accept(t_quote))
    {
        cell *car = new cell();
        *car = read();
        return cell(new cell(v_symbol, "QUOTE"), new cell(car, (cell*)0));
    }
    else if (accept(t_string))
        return cell(v_string, last.value);
    else if (accept(t_symbol))
        return cell(v_symbol, toUpper(last.value));
    else if (accept(t_number))
        return cell(atof(last.value.c_str()));
    else if (accept(t_lparen))
    {
        cell head(v_list);
        cell *tail = &head;
        while (index < ntokens && t.type != t_rparen)
        {
            cell newcar = read();
            tail->car = new cell();
            *tail->car = newcar;
            tail->cdr = new cell(v_list);
            tail = tail->cdr;
        }
        expect(t_rparen);
        return head;
    }
    throw (exception("Nothing to read."));
}
