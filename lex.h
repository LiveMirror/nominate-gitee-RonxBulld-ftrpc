//
// Created by Rexfield on 2018/4/26.
//

#ifndef FTRPC_LEX_H
#define FTRPC_LEX_H

#include <string>
#include <iostream>
#include <map>
#include <stack>
#include <stdexcept>
#include "symman.h"
typedef unsigned int StringID;

enum TokenEnum
{
    TOKEN_EOF = 0,
    TOKEN_START = 256,
    TOKEN_ID,
    TOKEN_INTEGER_LITERAL,
#define TYPE(k)    TOKEN_##k,
#define KEYWD(k)    TOKEN_##k,
#include "keywords.h"
};

struct token
{
    enum TokenEnum type;
    const char * literal;
    size_t length;
    union MultiVal
    {
        int i;
        StringID string;
        char c;
        TokenID token;
    } value;
};

struct lexer_info
{
    size_t lineno;
    size_t rowno;
};

class LexException : public std::exception
{
private:
    std::string msg;
public:
    explicit LexException(const std::string &msg) {
        this->msg = msg;
        std::cerr << this->msg << std::endl;
    }
    ~LexException() throw() {
    }
};

class lex
{
private:
    size_t iptr = 0;
    size_t start_ptr = 0;
    size_t maxptr = 0;
    void TokenConvert(struct token &Tk);
    void IntegerConvert(struct token &Tk);
    std::map<std::string, enum TokenEnum> keywdMap;
    std::stack<struct token> tokenStack;
    std::stack<size_t> ptrStack;
    const char *src;
    TokenManage *tokenManage;
public:
    explicit lex(const char *src, TokenManage *tokenManage);
    struct token getToken();
//    const std::string &GetString(StringID id);
    void pushBack(struct token T);
    struct lexer_info getLexerInfo();
    void pushPtr();
    void popPtr();
};

#endif //FTRPC_LEX_H
