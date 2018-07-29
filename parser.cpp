#define _CRT_SECURE_NO_WARNINGS
//
// Created by Rexfield on 2018/4/27.
//
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"
#include "lex.h"
#include "ast_tree.h"

#define GETTOKEN(T) (T) = this->lexer->getToken()
#define CHECK_TOKEN_IF(T, Type, ErrorMsg) if((T).type != (Type)) { \
                                                this->reportError(ErrorMsg); \
                                                return nullptr; \
                                             }
#define REQUIRE_TOKEN(T, Type, ErrorMsg) do { \
                                                GETTOKEN(T); \
                                                CHECK_TOKEN_IF(T, Type, ErrorMsg); \
                                            } while(0)
#define PUSHBACK(T) this->lexer->pushBack(T)
#define SEE_NEXT_TOKEN(T) do { GETTOKEN(T); PUSHBACK(T); } while (0)

#define CALL_UNTERMINAL_PARSER(unterminal) ([&](){ \
                                                   std::unique_ptr<unterminal##Node> node = this->parse##unterminal(); \
                                                   return node; \
                                               })()

parse::parse(const char *src)
{
    this->lexer = new lex(src, &this->tokenManage);
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
 * struct : TOKEN_struct TOKEN_ID { (type TOKENID ;)* } ;
 */
std::unique_ptr<StructNode> parse::parseStruct() {
    token T;
    std::unique_ptr<StructNode> structure(new StructNode());
    REQUIRE_TOKEN(T, TOKEN_struct, "Require `struct`.");
    REQUIRE_TOKEN(T, TOKEN_ID, "Anonymouse struct is not supported.");
    structure->name = T.value.token;
    REQUIRE_TOKEN(T, ':', "Require `:`.");
    REQUIRE_TOKEN(T, '{', "Require `{`.");
    SEE_NEXT_TOKEN(T);
    MemberLists members;
    if (T.type != '}') {
        do {
            std::unique_ptr<TypeNode> type = CALL_UNTERMINAL_PARSER(Type);
            if (!type) return nullptr;
            REQUIRE_TOKEN(T, TOKEN_ID, "Require member name after type.");
            Member member;
            member.first = *type;
            member.second = T.value.string;
            members.push_back(member);
            REQUIRE_TOKEN(T, ';', "Require `;`.");
            SEE_NEXT_TOKEN(T);
        } while (T.type != '}');
    }
    typeManage.registType(structure->name, TypeManage::typeDefType::DeclareStruct, members);
    structure->type = (enum Type)typeManage.getTypeID(structure->name);
    REQUIRE_TOKEN(T, '}', "Require `}`.");
    REQUIRE_TOKEN(T, ';', "Require `;`.");
    return structure;
}

/*
 * type : TOKEN_void | TOKEN_int | TOKEN_string | TOKEN_float | TOKEN_bool | struct;
 */
std::unique_ptr<TypeNode> parse::parseType()
{
    std::unique_ptr<TypeNode> type(new TypeNode());
    token T;
    GETTOKEN(T);
    switch (T.type) {
        case TOKEN_void: {
            type->type = TY_void;
        }
            break;
        case TOKEN_int: {
            type->type = TY_int;
        }
            break;
        case TOKEN_string: {
            type->type = TY_string;
        }
            break;
        case TOKEN_float: {
            type->type = TY_float;
        }
            break;
        case TOKEN_bool: {
            type->type = TY_bool;
        }
            break;
        case TOKEN_ID: {
            if (this->typeManage.isType(T.value.token)) {
                type->type = (enum Type)this->typeManage.getTypeID(T.value.token);
                break;
            }
        }
        default: {
            char tmp[T.length + 1];
            strncpy(tmp, T.literal, T.length);
            tmp[T.length + 1] = '\0';
            this->reportError("Not supported type - %s.", tmp);
            return nullptr;
        }
    }
    // Check if had array declare symbol
    SEE_NEXT_TOKEN(T);
    if (T.type == '[') {
        GETTOKEN(T);
        REQUIRE_TOKEN(T, ']', "Require `]`.");
        type->isArray = true;
    }
    return type;
}

/*
 * parament : type TOKEN_ID ;
 */
std::unique_ptr<ParamNode> parse::parseParam()
{
    std::unique_ptr<ParamNode> param(new ParamNode());
    auto type = CALL_UNTERMINAL_PARSER(Type);
    if (!type) return nullptr;
    param->type = *type;
    if(param->type.type == TY_void) {
        this->reportError("The parameter type should not be void.");
        return nullptr;
    }
    token T;
    REQUIRE_TOKEN(T, TOKEN_ID, "You should provide the argument name.");
    param->name = T.value.string;
    return param;
}

/*
 * api : type TOKEN_ID '(' (parament (',' parament)*)?')' ';' ;
 */
std::unique_ptr<ApiNode> parse::parseApi() {
    std::unique_ptr<ApiNode> api(new ApiNode());
    token T;
    std::unique_ptr<TypeNode> type = CALL_UNTERMINAL_PARSER(Type);
    if (!type) return nullptr;
    api->retType = *type;
    REQUIRE_TOKEN(T, TOKEN_ID, "You should provide the api name.");
    api->name = T.value.string;
    REQUIRE_TOKEN(T, '(', "Request ')'.");
    SEE_NEXT_TOKEN(T);
    if (T.type != ')') {
        do {
            std::unique_ptr<ParamNode> param = CALL_UNTERMINAL_PARSER(Param);
            if (!param) return nullptr;
            api->params.push_back(*param);
            GETTOKEN(T);
        } while (T.type == ',');
        PUSHBACK(T);
    }
    REQUIRE_TOKEN(T, ')', "Request ')'.");
    REQUIRE_TOKEN(T, ';', "Request ';'.");
    return api;
}

/*
 * module : ( TOKEN_MODULE TOKEN_ID ':' '{' (struct|api)* '}' )* ;
 */
std::unique_ptr<ModuleNode> parse::parseModule()
{
    std::unique_ptr<ModuleNode> module(new ModuleNode());
    token T;
    GETTOKEN(T);
    if(T.type == TOKEN_module) {
        REQUIRE_TOKEN(T, TOKEN_ID, "Anonymous module is not supported.");
        module->name = T.value.string;
        REQUIRE_TOKEN(T, ':', "Request ':'.");
        REQUIRE_TOKEN(T, '{', "Request '{'.");
        SEE_NEXT_TOKEN(T);
        while (true) {
            if (T.type == TOKEN_struct) {
                std::unique_ptr<StructNode> structure = CALL_UNTERMINAL_PARSER(Struct);
                if (!structure) return nullptr;
                module->structs.push_back(*structure);
                SEE_NEXT_TOKEN(T);
            } else if (typeManage.isType(T.value.token)) {
                std::unique_ptr<ApiNode> api = CALL_UNTERMINAL_PARSER(Api);
                if (!api) return nullptr;
                module->apis.push_back(*api);
                SEE_NEXT_TOKEN(T);
            } else {
                break;
            }
        }
        REQUIRE_TOKEN(T, '}', "Request '}'.");
    }
    return module;
}

/*
 * document : VERSION '=' INTEGER_LITERAL ';' module* ;
 */
std::unique_ptr<RootNode> parse::parseRoot()
{
    std::unique_ptr<RootNode> root(new RootNode());
    // declare version first
    struct token T;
    REQUIRE_TOKEN(T, TOKEN_version, "Must declare script version first.");
    REQUIRE_TOKEN(T, '=', "Request '='.");
    REQUIRE_TOKEN(T, TOKEN_INTEGER_LITERAL, "Request Version.");
    root->version = (unsigned int)T.value.i;
    REQUIRE_TOKEN(T, ';', "Request ';'.");
    SEE_NEXT_TOKEN(T);
    if (T.type == TOKEN_module) {
        do {
            std::unique_ptr<ModuleNode> module = CALL_UNTERMINAL_PARSER(Module);
            if (!module) return nullptr;
            ModuleNode Tmn = *module;
            root->modules.push_back(Tmn);
            SEE_NEXT_TOKEN(T);
        } while (T.type == TOKEN_module);
    }
    return root;
}

bool parse::work()
{
    this->document = CALL_UNTERMINAL_PARSER(Root);
    if (!this->document) return false;
    else return true;
}