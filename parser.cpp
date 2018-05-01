#define _CRT_SECURE_NO_WARNINGS
//
// Created by Rexfield on 2018/4/27.
//
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"
#include "lex.h"

#define GETTOKEN(T) (T) = this->lexer->getToken()
#define CHECK_TOKEN_IF(T, Type, ErrorMsg) if((T).type != (Type)) { \
                                                this->reportError(ErrorMsg); \
                                                return false; \
                                             }
#define REQUIRE_TOKEN(T, Type, ErrorMsg) GETTOKEN(T); CHECK_TOKEN_IF(T, Type, ErrorMsg)
#define PUSHBACK(T) this->lexer->pushBack(T)
#define CALL_UNTERMINAL_PARSER(unterminal, param) this->lexer->pushPtr(); \
                                                       if (!this->parse##unterminal(param)) { \
                                                           this->lexer->popPtr(); \
                                                           return false; \
                                                       }
#define TRY_CALL_UNTERMINAL_PARSER(unterminal, param) ([&](){ \
                                                            this->noPrint = true; \
                                                            CALL_UNTERMINAL_PARSER(unterminal, param) else { \
                                                                return true;\
                                                            } \
                                                            this->noPrint = false; \
                                                            })()

parse::parse(const char *src)
{
    this->lexer = new lex(src);
}

void parse::reportError(const char *fmt, ...)
{
    if(this->noPrint)
        return;
    struct lexer_info info = this->lexer->getLexerInfo();
    fprintf(stderr, "[%u:%u]Syntax Error:", (unsigned int)info.lineno, (unsigned int)info.rowno);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/*
 * in_out_property : ( '[' (TOKEN_IN('|' TOKEN_OUT)?)|TOKEN_OUT ']' )?
 */
bool parse::parseInOutProperty(struct ParamNode *param)
{
    struct token T;
    GETTOKEN(T);
    if(T.type == '[') {
        GETTOKEN(T);
        if(T.type == TOKEN_in) {
            param->inFeature = true;
            GETTOKEN(T);
            if(T.type == '|') {
                REQUIRE_TOKEN(T, TOKEN_out, "Only input 'out' after '|'.");
                param->outFeature = true;
            } else {
                PUSHBACK(T);
            }
        }
        else if(T.type == TOKEN_out) {
            param->outFeature = true;
        }
        REQUIRE_TOKEN(T, ']', "Request ']'.");
    } else {
        PUSHBACK(T);
    }
    return true;
}

/*
 * type : TOKEN_void | TOKEN_int | TOKEN_string ;
 */
bool parse::parseType(struct TypeNode *type)
{
    struct token T;
    GETTOKEN(T);
    switch (T.type)
    {
        case TOKEN_void:
            type->type = TY_void;
            break;
        case TOKEN_int:
            type->type = TY_int;
            break;
        case TOKEN_string:
            type->type = TY_string;
            break;
        default:
            char tmp[T.length + 1];
            strncpy(tmp, T.literal, T.length);
            tmp[T.length + 1] = '\0';
            this->reportError("Not supported type - %s.", tmp);
            return false;
    }
    return true;
}

/*
 * parament : in_out_property type TOKEN_ID ;
 */
bool parse::parseParament(struct ApiNode *api)
{
    struct ParamNode param;
    CALL_UNTERMINAL_PARSER(InOutProperty, &param);
    CALL_UNTERMINAL_PARSER(Type, &param.type);
    if(param.type.type == TY_void) {
        this->reportError("The parameter type should not be void.");
        return false;
    }
    struct token T;
    REQUIRE_TOKEN(T, TOKEN_ID, "You should provide the argument name.");
    param.name = T.value.string;
    api->params.push_back(new ParamNode(param));
    return true;
}

/*
 * api_list : (type TOKEN_ID '(' (parament (',' parament)*)?')' ';')* ;
 */
bool parse::parseApiList(struct ModuleNode *module)
{
    struct ApiNode api, apiBlank;
    while(TRY_CALL_UNTERMINAL_PARSER(Type, &api.retType)) {
        struct token T;
        REQUIRE_TOKEN(T, TOKEN_ID, "You should provide the api name.");
        api.name = T.value.string;
        REQUIRE_TOKEN(T, '(', "Request ')'.");
        GETTOKEN(T);
        if (T.type != ')') {
            PUSHBACK(T);
            do {
                CALL_UNTERMINAL_PARSER(Parament, &api);
                GETTOKEN(T);
            } while (T.type == ',');
            PUSHBACK(T);
            REQUIRE_TOKEN(T, ')', "Request ')'.");
        }
        REQUIRE_TOKEN(T, ';', "Request ';'.");
        module->apis.push_back(new ApiNode(api));
        api = apiBlank;
    }
    return true;
}

/*
 * module_list : ( TOKEN_MODULE TOKEN_ID ':' '{' api_list '}' )* ;
 */
bool parse::parseModuleList(struct RootNode *root)
{
    struct ModuleNode module, mnBlank;
    struct token T;
    GETTOKEN(T);
    while(T.type == TOKEN_module) {
        REQUIRE_TOKEN(T, TOKEN_ID, "Anonymous module is not supported.");
        module.name = T.value.string;
        REQUIRE_TOKEN(T, ':', "Request ':'.");
        REQUIRE_TOKEN(T, '{', "Request '{'.");
        CALL_UNTERMINAL_PARSER(ApiList, &module);
        REQUIRE_TOKEN(T, '}', "Request '}'.");
        GETTOKEN(T);
        root->modules.push_back(new ModuleNode(module));
        module = mnBlank;
    }
    PUSHBACK(T);
    return true;
}

/*
 * document : VERSION '=' INTEGER_LITERAL ';' module_list ;
 */
bool parse::parseDocument(struct RootNode *root)
{
    // declare version first
    struct token T;
    REQUIRE_TOKEN(T, TOKEN_version, "Must declare script version first.");
    REQUIRE_TOKEN(T, '=', "Request '='.");
    REQUIRE_TOKEN(T, TOKEN_INTEGER_LITERAL, "Request Version.");
    root->version = (unsigned int)T.value.i;
    REQUIRE_TOKEN(T, ';', "Request ';'.");
    CALL_UNTERMINAL_PARSER(ModuleList, root);
    return true;
}

bool parse::work()
{
    CALL_UNTERMINAL_PARSER(Document, &this->document);
    return true;
}