//
// Created by Rexfield on 2018/4/30.
//

#ifndef FTRPC_TYPEDEF_H
#define FTRPC_TYPEDEF_H

enum Type {
    TY_NAN = -1,
#define TYPE(k) TY_##k,
#define KEYWD(k)
#include "keywords.h"
};

#endif //FTRPC_TYPEDEF_H
