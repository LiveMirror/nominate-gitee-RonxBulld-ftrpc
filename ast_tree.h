//
// Created by Rexfield on 2018/4/27.
//

#ifndef FTRPC_AST_TREE_H
#define FTRPC_AST_TREE_H

#include <list>
#include <memory>

enum NodeType {
    NT_NONE = -1, NT_ROOT = 0, NT_MODULE, NT_API, NT_PARAM, NT_TYPE, NT_STRUCT
};

#include "TypeDef.h"


class astTreeNode
{
public:
    enum NodeType nodeType = NT_NONE;
    astTreeNode() {}
};

class TypeNode : public astTreeNode
{
public:
    enum Type type;
    bool isArray = false;
    explicit TypeNode(enum Type type = TY_NAN) {
        this->nodeType = NT_TYPE;
        this->type = type;
    }
};

class StructNode : public TypeNode
{
public:
    unsigned int name = (unsigned int)-1;
    StructNode() {
        this->nodeType = NT_STRUCT;
    }
};

class ParamNode : public astTreeNode
{
public:
    TypeNode type;
    unsigned int name = (unsigned int)-1;
    ParamNode() {
        this->nodeType = NT_PARAM;
    }
};

class ApiNode : public astTreeNode
{
public:
    TypeNode retType;
    unsigned int name = (unsigned int)-1;
    std::list<ParamNode> params;
    ApiNode() {
        this->nodeType = NT_API;
        this->params.clear();
    }
};

class ModuleNode : public astTreeNode
{
public:
    std::list<ApiNode> apis;
    std::list<StructNode> structs;
    unsigned int name = (unsigned int)-1;
    ModuleNode () {
        this->nodeType = NT_MODULE;
        this->apis.clear();
        this->structs.clear();
    }
};

class RootNode : public astTreeNode
{
public:
    unsigned int version = 0;
    std::list<ModuleNode> modules;
    RootNode() {
        this->nodeType = NT_ROOT;
        this->modules.clear();
    }
};

#endif //FTRPC_AST_TREE_H
