//
// Created by Rexfield on 2018/4/27.
//

#ifndef FTRPC_PARSER_H
#define FTRPC_PARSER_H

#include <memory>
#include "lex.h"
#include "symman.h"
#include "ast_tree.h"

class parse
{
private:
    bool noPrint = false;
    void reportError(const char *fmt, ...);
    std::unique_ptr<RootNode> parseRoot();
    std::unique_ptr<ModuleNode> parseModule();
    std::unique_ptr<ApiNode> parseApi();
    std::unique_ptr<ParamNode> parseParam();
    std::unique_ptr<TypeNode> parseType();
    std::unique_ptr<StructNode> parseStruct();
public:
    lex *lexer = nullptr;
    TokenManage tokenManage;
    TypeManage typeManage {
#define KEYWD(K)
#define TYPE(T) {TY_##T, TOKEN_##T},
#include "keywords.h"
    };
    explicit parse(const char *src);
    std::unique_ptr<RootNode> document;
    bool work();
};

#endif //FTRPC_PARSER_H
