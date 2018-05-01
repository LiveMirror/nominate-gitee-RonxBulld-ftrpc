//
// Created by Rexfield on 2018/4/27.
//

#ifndef FTRPC_AST_TREE_H
#define FTRPC_AST_TREE_H

#include <list>

enum NodeType {
    NT_NONE = -1, NT_ROOT = 0, NT_MODULE, NT_API, NT_PARAM, NT_TYPE
};

#include "TypeDef.h"

class astTreeNode
{
public:
    enum NodeType nodeType;
    astTreeNode() {
        this->nodeType = NT_NONE;
    }
};

class TypeNode : public astTreeNode
{
public:
    enum Type type;
    TypeNode() {
        this->nodeType = NT_TYPE;
        this->type = TY_NAN;
    }
};

class ParamNode : public astTreeNode
{
public:
    struct TypeNode type;
    unsigned int name;
    bool inFeature;
    bool outFeature;
    ParamNode() {
        this->nodeType = NT_PARAM;
        this->name = (unsigned int)-1;
        this->inFeature = false;
        this->outFeature = true;
    }
};

class ApiNode : public astTreeNode
{
public:
    struct TypeNode retType;
    unsigned int name;
    std::list<struct ParamNode *> params;
    ApiNode() {
        this->nodeType = NT_API;
        this->name = (unsigned int)-1;
        this->params.clear();
    }
};

class ModuleNode : public astTreeNode
{
public:
    std::list<struct ApiNode *> apis;
    unsigned int name;
    ModuleNode () {
        this->name = (unsigned int)-1;
        this->nodeType = NT_MODULE;
        this->apis.clear();
    }
};

class RootNode : public astTreeNode
{
public:
    unsigned int version;
    std::list<struct ModuleNode *> modules;
    RootNode() {
        this->nodeType = NT_ROOT;
        this->version = 0;
        this->modules.clear();
    }
};

#endif //FTRPC_AST_TREE_H
