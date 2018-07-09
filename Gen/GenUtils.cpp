//
// Created by Rexfield on 2018/5/2.
//
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "GenUtils.h"

/*
 * TypeID | Json Check Method |Json Parse Method | C++ Type Name(Gen) | TypeScript Type Name(Gen)
 */
enum ColumnSerial {
    JsonCheckMethod, JsonParseMethod, CppTypeName_Gen, TypescriptTypeName_Gen
};
std::map<TypeID, std::vector<std::string>> typeMap = {
        {TY_string, {"isString", "asString", "std::string", "string"}},
        {TY_void,   {"isNull",   "",         "void",        "void"}},
        {TY_int,    {"isInt",    "asInt",    "int",         "number"}},
        {TY_any,    {"isObject", "",         "void*",       "any"}},
        {TY_float,  {"isDouble", "asFloat",  "float",       "number"}},
        {TY_bool,   {"isBool",   "asBool",   "bool",        "boolean"}}
};

template<typename T>
bool RegistType(TypeID type, const T &JsonCheckMethod, const T &JsonParseMethod, const T &CppTypeName_Gen, const T &TypescriptTypeName_Gen) {
    std::vector<std::string> insItem{std::to_string(JsonCheckMethod),
                                   std::to_string(JsonParseMethod),
                                   std::to_string(CppTypeName_Gen),
                                   std::to_string(TypescriptTypeName_Gen)
    };
    typeMap.insert(std::make_pair(type, insItem));
}
bool RegistType(TypeID type, const std::string &JsonCheckMethod, const std::string &JsonParseMethod, const std::string &CppTypeName_Gen, const std::string &TypescriptTypeName_Gen) {
    std::vector<std::string> insItem{JsonCheckMethod, JsonParseMethod, CppTypeName_Gen, TypescriptTypeName_Gen};
    typeMap.insert(std::make_pair(type, insItem));
}
const std::string GetJsonCheckMethod(TypeNode T) {
    if (!T.isArray)
        return typeMap[T.type][JsonCheckMethod] + "()";
    else
        return "isJsonArray(&JsonValueExtra::" + typeMap[T.type][JsonCheckMethod] + ")";
}
const std::string GetJsonConvertMethod(TypeNode T) {
    if (!T.isArray)
        return typeMap[T.type][JsonParseMethod] + "()";
    else
        return "asJsonArray(&JsonValueExtra::" + typeMap[T.type][JsonParseMethod] + ")";
}
const std::string GetCppType(TypeNode T) {
    if (!T.isArray)
        return typeMap[T.type][CppTypeName_Gen];
    else
        return "std::vector<" + typeMap[T.type][CppTypeName_Gen] + ">";
}
const std::string GetTsType(TypeNode T) {
    if (!T.isArray)
        return typeMap[T.type][TypescriptTypeName_Gen];
    else
        return "Array<" + typeMap[T.type][TypescriptTypeName_Gen] + ">";
}

bool isBaseType(enum Type T) {
    switch (T)
    {
        case TY_string:
        case TY_bool:
        case TY_void:
        case TY_float:
        case TY_int:
            return true;
        default:
            return false;
    }
}

std::string ReadFileAsTxt(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == nullptr) {
        std::cerr << "No such file or directory - " << path << std::endl;
        throw std::invalid_argument(path);
    }
    fseek(fp, 0, SEEK_END);
    auto len = (size_t)ftell(fp);
    auto mem = (char *)malloc(len * sizeof(char));
    fseek(fp, 0, SEEK_SET);
    len = fread(mem, sizeof(char), (size_t)len, fp);
    mem[len] = '\0';
    std::string s(mem);
    free(mem);
    return s;
}

std::string ReadFileAsTxt(const std::string &path) {
    return ReadFileAsTxt(path.c_str());
}

#if _WIN32_WINNT || _WIN32 || _WIN || MSVC
#include <windows.h>
#define GETEXEPATH(szfp) GetModuleFileName(nullptr, szfp, MAX_PATH)
#else
#include <unistd.h>
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#define GETEXEPATH(szfp) readlink("/proc/self/exe", szfp, MAX_PATH)
#endif
std::string ReadTemplate(const std::string &path) {
    char szFilePath[MAX_PATH + 1]={0};
    GETEXEPATH(szFilePath);
    char *p = strrchr(szFilePath, '\\');
    if (p) {
        p[1] = 0;
        strcat(szFilePath, path.c_str());
    } else {
        strcpy(szFilePath, path.c_str());
    }
    return ReadFileAsTxt(szFilePath);
}
#undef GETEXEPATH

void substring_replace(std::string &str,const std::string &oldstr,const std::string &newstr) {
    std::string::size_type pos=0;
    std::string::size_type a=oldstr.size();
    std::string::size_type b=newstr.size();
    while((pos=str.find(oldstr,pos))!=std::string::npos)
    {
        str.replace(pos,a,newstr);
        pos+=b;
    }
}
