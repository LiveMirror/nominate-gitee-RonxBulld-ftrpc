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

extern bool hadVersionInfo;

bool RegistType(enum Type type, std::string &JsonCheckMethod, std::string &JsonParseMethod, std::string &CppTypeName_Gen, std::string &TypescriptTypeName_Gen);
std::string &GetJsonCheckMethod(enum Type T);
std::string &GetJsonConvertMethod(enum Type T);
std::string &GetCppType(enum Type T);
std::string &GetTsType(enum Type T);
std::string ReadFileAsTxt(std::string path);
void substring_replace(std::string &str,const std::string &oldstr,const std::string &newstr);

#endif //FTRPC_GENUTILS_H
