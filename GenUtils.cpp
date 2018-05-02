//
// Created by Rexfield on 2018/5/2.
//
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>

#include "GenUtils.h"

std::map<enum Type, std::tuple<std::string, std::string, std::string, std::string>> typeMap = {
        {TY_string, {"asString", "TY_string", "std::string", "string"}},
        {TY_void,   {"",         "TY_void",   "void",        "void"}},
        {TY_int,    {"asInt",    "TY_int",    "int",         "int"}},
        {TY_any,    {"",         "TY_any",    "void*",       "any"}},
        {TY_float,  {"asFloat",  "TY_float",  "float",       "float"}},
        {TY_bool,   {"asBool",   "TY_bool",   "bool",        "boolean"}}
};

std::string &GetJsonAsMethod(enum Type T) { return std::get<0>(typeMap[T]); }
std::string &GetEnumName(enum Type T)     { return std::get<1>(typeMap[T]); }
std::string &GetCppType(enum Type T)      { return std::get<2>(typeMap[T]); }
std::string &GetTsType(enum Type T)       { return std::get<3>(typeMap[T]); }

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
