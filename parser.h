#ifndef _PARSER_H_INCLUDED_
#define _PARSER_H_INCLUDED_

#include <vector>
#include <map>
#include <iostream>
#include <memory>

#include "tokenizer.h"

std::string toUpper(std::string);

typedef enum
{
    v_symbol = 0,
    v_number,
    v_string,
    v_function,
    v_proc,
    v_list
} cell_type;

struct environment;

struct cell
{
    typedef cell (*proc_t) (const cell&);
    cell_type type;
    cell *car; cell *cdr;
    std::string str;
    double n;
    proc_t proc;
    std::shared_ptr<environment> env;

    bool operator==(const cell&) const;

    ~cell() {}
    cell();
    cell(cell_type);
    cell(cell_type, std::string);
    cell(double);
    cell(proc_t);
    cell(cell*, cell*); //cons
};


struct environment
{
    std::map <std::string, cell> vars;
    std::shared_ptr <environment> parent;

    cell& get(std::string name);
    environment(std::shared_ptr<environment> parent_ = std::shared_ptr<environment>()) {parent = parent_;}
};

class parser
{
    private:
    std::vector<token> tokens;
    int index;
    int ntokens;
    token t;
    token last;

    bool accept(token_type);
    bool expect(token_type);
    token_type peek();

    public:
    parser(std::vector<token>);
    cell read();
};

struct exception
{
    std::string err;
    exception(std::string err_) {err = err_;}
};

#endif // _PARSER_H_INCLUDED_
