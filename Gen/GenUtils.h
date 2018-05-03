//
// Created by Rexfield on 2018/5/2.
//

#ifndef FTRPC_GENUTILS_H
#define FTRPC_GENUTILS_H

#define PROGRAM_VERSION_STR "2"
#include "../ast_tree.h"
#include "../lex.h"
#include "../symman.h"
#include <string>

std::string &GetJsonAsMethod(enum Type T);
std::string &GetEnumName(enum Type T);
std::string &GetCppType(enum Type T);
std::string &GetTsType(enum Type T);
std::string ReadFileAsTxt(std::string path);
void substring_replace(std::string &str,const std::string &oldstr,const std::string &newstr);

#endif //FTRPC_GENUTILS_H
