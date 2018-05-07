//
// Created by Rexfield on 2018/5/2.
//
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>

#include "GenUtils.h"

/*
 * TypeID | Json Check Method |Json Parse Method | C++ Enum Name | C++ Type Name(Gen) | TypeScript Type Name(Gen)
 */
enum ColumnSerial {
    JsonCheckMethod, JsonParseMethod, CppTypeName_Gen, TypescriptTypeName_Gen
};
std::map<enum Type, std::tuple<std::string, std::string, std::string, std::string>> typeMap = {
        {TY_string, {"isString", "asString", "std::string", "string"}},
        {TY_void,   {"isNull",   "",         "void",        "void"}},
        {TY_int,    {"isInt",    "asInt",    "int",         "number"}},
        {TY_any,    {"isObject", "",         "void*",       "any"}},
        {TY_float,  {"isDouble", "asFloat",  "float",       "float"}},
        {TY_bool,   {"isBool",   "asBool",   "bool",        "boolean"}}
};

bool RegistType(enum Type type, std::string &JsonCheckMethod, std::string &JsonParseMethod, std::string &CppTypeName_Gen, std::string &TypescriptTypeName_Gen) {
    std::tuple<std::string, std::string, std::string, std::string> insItem{JsonCheckMethod, JsonParseMethod, CppTypeName_Gen, TypescriptTypeName_Gen};
    typeMap.insert(std::make_pair(type, insItem));
}
std::string &GetJsonCheckMethod(enum Type T)   { return std::get<JsonCheckMethod>(typeMap[T]); }
std::string &GetJsonConvertMethod(enum Type T) { return std::get<JsonParseMethod>(typeMap[T]); }
std::string &GetCppType(enum Type T)           { return std::get<CppTypeName_Gen>(typeMap[T]); }
std::string &GetTsType(enum Type T)            { return std::get<TypescriptTypeName_Gen>(typeMap[T]); }

std::string ReadFileAsTxt(const std::string path) {
    std::ifstream ifs(path);
    if(!ifs.is_open()) {
        throw std::invalid_argument(path);
    }
    std::string s;
    while(!ifs.eof()) {
        std::string tmp;
        std::getline(ifs, tmp);
        s.push_back('\n');
        s.append(tmp);
    }
    ifs.close();
    return s;
}

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
