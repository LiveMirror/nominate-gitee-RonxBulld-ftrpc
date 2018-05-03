//
// Created by Rexfield on 2018/4/27.
//

#ifndef FTRPC_PARSER_H
#define FTRPC_PARSER_H

#include "lex.h"
#include "symman.h"
#include "ast_tree.h"

class parse
{
private:
    bool noPrint = false;
    void reportError(const char *fmt, ...);
    bool parseDocument(struct RootNode *root);
    bool parseModuleList(struct RootNode *root);
    bool parseApiList(struct ModuleNode *module);
    bool parseInOutProperty(struct ParamNode *param);
    bool parseParament(struct ApiNode *api);
    bool parseType(struct TypeNode *type);
public:
    lex *lexer = nullptr;
    TokenManage tokenManage;
    TypeManage typeManage;
    explicit parse(const char *src);
    struct RootNode document;
    bool work();
};

#endif //FTRPC_PARSER_H
