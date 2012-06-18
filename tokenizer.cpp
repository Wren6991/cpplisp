#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "tokenizer.h"

token::token()
{
    type = (token_type)0;
    value = std::string();
}
token::token(token_type type_, std::string value_)
{
    type = type_;
    value = value_;
}

typedef enum {
    s_start = 0,
    s_symbol,
    s_number,
    s_string
} state_enum;

std::vector<token> tokenize(std::string progstring)
{
    bool characterAllowed[256];
    for (int i = 0; i < 256; i++)
        characterAllowed[i] = false;

    for (int i = 'A'; i <= 'Z'; i++)
        characterAllowed[i] = true;
    for (int i = 'a'; i <= 'z'; i++)
        characterAllowed[i] = true;
    characterAllowed['+'] = true;
    characterAllowed['-'] = true;
    characterAllowed['*'] = true;
    characterAllowed['/'] = true;
    characterAllowed['<'] = true;
    characterAllowed['>'] = true;
    characterAllowed['='] = true;
    characterAllowed['!'] = true;
    characterAllowed['?'] = true;

    std::vector<token> tokens;
    state_enum state = s_start;
    int index = -1;
    int regionstart;

    while (++index < progstring.size())
    {
        char v = progstring.c_str()[index];
        switch(state)
        {
            case s_start:
                regionstart = index;
                if (v == '(')
                    tokens.push_back(token(t_lparen, "("));
                else if (v == ')')
                    tokens.push_back(token(t_rparen, ")"));
                else if (v >= '0' && v <= '9')
                    state = s_number;
                else if (v == ';')
                    while (++index < progstring.size() && progstring.c_str()[index] != 10 && progstring.c_str()[index] != 13);
                else if (characterAllowed[v])
                    state = s_symbol;
                else if (v == '"')
                    state = s_string;
            break;
            case s_number:
                if (v < '0' || v > '9')
                {
                    tokens.push_back(token(t_number, progstring.substr(regionstart, index - regionstart)));
                    index--;
                    state = s_start;
                }
            break;
            case s_symbol:
                if (!characterAllowed[v] || index == progstring.size() - 1)
                {
                    tokens.push_back(token(t_symbol, progstring.substr(regionstart, index - regionstart)));
                    if (index < progstring.size() - 1)
                        index--;
                    state = s_start;
                }
            break;
            case s_string:
                if (v == '"')
                {
                    tokens.push_back(token(t_string, progstring.substr(regionstart + 1, index - regionstart - 1)));
                    state = s_start;
                }
            break;
        }
    }
    return tokens;
}
