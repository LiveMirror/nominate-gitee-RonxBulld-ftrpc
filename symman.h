//
// Created by Rexfield on 2018/5/3.
//

#ifndef FTRPC_SYMMAN_H
#define FTRPC_SYMMAN_H

#include <map>
#include <string>
#include <list>

typedef unsigned int TokenID;
class TokenManage
{
private:
    TokenID indexReg = 0;
    std::map<std::string, TokenID> s2i;
    std::map<TokenID, std::string> i2s;
public:
    TokenID operator[](const std::string &Token);
    const std::string &operator[](const TokenID &ID);
    TokenManage(std::initializer_list<std::pair<std::string, TokenID>> initToken);
    TokenManage() {}
};

typedef unsigned int TypeID;
typedef std::pair<TypeID, TokenID> Member;
typedef std::list<Member> MemberLists;
class TypeManage
{
private:
    TypeID indexReg = 0;
public:
    std::map<TokenID, TypeID> tk2ty;    // TokenID -> TypeID
    std::map<TypeID, TokenID> ty2tk;    // TypeID -> TokenID
    std::map<TypeID, MemberLists> StructsMap;    // TypeID { TypeID* }
    TypeManage(std::initializer_list<Member> initBaseType);
    TypeManage() {}
    enum typeDefType { DeclareBase, DeclareStruct };
    void registType(TokenID Name, enum typeDefType DeclareType, MemberLists &MemberList);
    TypeID getTypeID(TokenID Name);
    bool isType(TokenID Name);
};

#endif //FTRPC_SYMMAN_H
