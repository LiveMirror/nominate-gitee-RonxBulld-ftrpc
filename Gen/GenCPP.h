//
// Created by Rexfield on 2018/4/29.
//

#ifndef FTRPC_GEN_H
#define FTRPC_GEN_H

#include "../ast_tree.h"
#include "../lex.h"
#include "GenUtils.h"

bool GenerateCPP(struct RootNode &document, TokenManage &tokenSystem, TypeManage &typeSystem);

#endif //FTRPC_GEN_H
