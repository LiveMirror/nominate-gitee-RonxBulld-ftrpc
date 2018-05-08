//
// Created by Rexfield on 2018/5/2.
//
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <vector>

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
        {TY_float,  {"isDouble", "asFloat",  "float",       "float"}},
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
std::string &GetJsonCheckMethod(enum Type T)   { return typeMap[T][JsonCheckMethod]; }
std::string &GetJsonConvertMethod(enum Type T) { return typeMap[T][JsonParseMethod]; }
std::string &GetCppType(enum Type T)           { return typeMap[T][CppTypeName_Gen]; }
std::string &GetTsType(enum Type T)            { return typeMap[T][TypescriptTypeName_Gen]; }

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

const char *ForceConvert_CPP(enum Type T) {
    if (isBaseType(T)) {
        return "";
    } else {
        return "(Json::Value)";
    }
}

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
