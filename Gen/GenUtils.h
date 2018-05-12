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

template<typename T> bool RegistType(TypeID type, const T &JsonCheckMethod, const T &JsonParseMethod, const T &CppTypeName_Gen, const T &TypescriptTypeName_Gen);
bool RegistType(TypeID type, const std::string &JsonCheckMethod, const std::string &JsonParseMethod, const std::string &CppTypeName_Gen, const std::string &TypescriptTypeName_Gen);
std::string &GetJsonCheckMethod(enum Type T);
std::string &GetJsonConvertMethod(enum Type T);
std::string &GetCppType(enum Type T);
std::string &GetTsType(enum Type T);
std::string ReadFileAsTxt(const char *path);
std::string ReadFileAsTxt(std::string &path);
std::string ReadTemplate(const std::string &path);
void substring_replace(std::string &str,const std::string &oldstr,const std::string &newstr);
bool isBaseType(enum Type T);

#endif //FTRPC_GENUTILS_H
