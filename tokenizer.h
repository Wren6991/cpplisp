#ifndef _TOKENIZER_H_INCLUDED_
#define _TOKENIZER_H_INCLUDED_

#include <string>
#include <vector>

typedef enum
{
    t_lparen = 0,
    t_rparen,
    t_symbol,
    t_number,
    t_string,
    t_quote
} token_type;

struct token
{
    token_type type;
    std::string value;
    token();
    token(token_type type_, std::string value_);
};

std::vector<token> tokenize(std::string progstring);


#endif // _TOKENIZER_H_INCLUDED_
