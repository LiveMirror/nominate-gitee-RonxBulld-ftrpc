#define _CRT_SECURE_NO_WARNINGS
//
// Created by Rexfield on 2018/4/26.
//
#include <string.h>
#include <ctype.h>
#include <stdexcept>
#include <set>
#include "lex.h"


#define PREVCHAR (this->src[this->iptr-1])
#define CURCHAR (this->src[this->iptr])
#define NEXTCHAR (this->src[this->iptr+1])
#define MOVENEXT (this->iptr++)
#define ISEOF (CURCHAR=='\0')
#define COLLECT_TOKEN(T)    \
    (T).length = this->iptr - this->start_ptr;\
    (T).literal = this->src + this->start_ptr

std::set<char> Blank = {' ','\t','\r','\n','\a'};
std::set<char> Symbol = {':',';','[',']','{','}','(',')','=',','};

const token EOF_Token { TOKEN_EOF, nullptr, 0, 0 };

lex::lex(const char *src, TokenManage *tokenManage)
{
    this->src = strdup(src);
    this->maxptr = strlen(this->src);
    this->tokenManage = tokenManage;
#define TYPE(k)    this->keywdMap[#k] = TOKEN_##k;
#define KEYWD(k)    this->keywdMap[#k] = TOKEN_##k;
#include "keywords.h"
}

void lex::TokenConvert(struct token &Tk)
{
    auto *tmp = new char[Tk.length+1];
    strncpy(tmp, Tk.literal, Tk.length);
    tmp[Tk.length] = '\0';
    if(this->keywdMap.find(tmp) != this->keywdMap.end())
    {
        // Process keyword
        Tk.type = this->keywdMap[tmp];
        Tk.value.i = (int)Tk.type;
    }
    else
    {
        // Process identified
        Tk.value.string = this->tokenManage->operator[](tmp);
        Tk.type = TOKEN_ID;
    }
	delete tmp;
}

void lex::IntegerConvert(struct token &Tk)
{
	auto *tmp = new char[Tk.length + 1];
    strncpy(tmp, Tk.literal, Tk.length);
    tmp[Tk.length] = '\0';
    Tk.type = TOKEN_INTEGER_LITERAL;
    Tk.value.i = (int)strtol(tmp, nullptr, 10);
	delete tmp;
}

void lex::pushBack(struct token T)
{
    this->tokenStack.push(T);
}

void lex::pushPtr()
{
    this->ptrStack.push(this->iptr);
}

void lex::popPtr()
{
    this->iptr = this->ptrStack.top();
    this->ptrStack.pop();
}

struct token lex::getToken()
{
    if(!this->tokenStack.empty())
    {
        struct token T = this->tokenStack.top();
        this->tokenStack.pop();
        return T;
    }
    if(this->iptr >= this->maxptr) {
        return EOF_Token;
    }
    while(true) {
        // pass blank charactor
        while (Blank.find(CURCHAR)!=Blank.end()) {
            MOVENEXT;
        }
        // pass line comment
        if (CURCHAR == '/' && NEXTCHAR == '/') {
            do {
                MOVENEXT;
            } while (!(CURCHAR == '\n' || ISEOF));
            if (!ISEOF) MOVENEXT;
        }
        // pass block comment
        if (CURCHAR == '/' && NEXTCHAR == '*') {
            MOVENEXT;
            do {
                MOVENEXT;
            } while (!((PREVCHAR == '*' && CURCHAR == '/') || ISEOF));
            if (ISEOF)
                throw LexException("Block comment unclosed.");
            else
                MOVENEXT;
        }
        this->start_ptr = this->iptr;
        if (isalpha(CURCHAR) || CURCHAR == '_') {
            while (isalnum(CURCHAR) || CURCHAR == '_') { MOVENEXT; }
            struct token T;
            COLLECT_TOKEN(T);
            this->TokenConvert(T);
            return T;
        } else if (isdigit(CURCHAR)) {
            while (isdigit(CURCHAR)) { MOVENEXT; }
            struct token T;
            COLLECT_TOKEN(T);
            this->IntegerConvert(T);
            return T;
        } else if (Symbol.find(CURCHAR) != Symbol.end()) {
            MOVENEXT;
            struct token T;
            T.type = (enum TokenEnum)PREVCHAR;
            T.value.c = PREVCHAR;
            COLLECT_TOKEN(T);
            return T;
        } else if (ISEOF) {
            return EOF_Token;
        } else {
            throw LexException("Bad charactor");
        }
    }
}

struct lexer_info lex::getLexerInfo()
{
    struct lexer_info lexinfo;
//    printf("%u\n", (unsigned int)this->start_ptr);
    fflush(stdout);
    lexinfo.lineno = lexinfo.rowno = 1;
    for (size_t pos = 0; pos <= this->start_ptr; pos++) {
        if(this->src[pos] == '\n') {
            lexinfo.lineno++;
            lexinfo.rowno = 1;
        } else {
            lexinfo.rowno++;
        }
    }
    return lexinfo;
}

//const std::string &lex::GetString(StringID id)
//{
//    for (auto iter = this->literalMap.begin(); iter != this->literalMap.end(); iter++) {
//        if (iter->second == id) {
//            return iter->first;
//        }
//    }
//    throw LexException("Cannot find STRING in literal map");
//}