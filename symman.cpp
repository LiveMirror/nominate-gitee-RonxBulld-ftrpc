//
// Created by Rexfield on 2018/5/3.
//
#include <exception>
#include "symman.h"

TokenID TokenManage::operator[](const std::string &Token) {
    if (this->s2i.find(Token) != this->s2i.end()) {
        return this->s2i[Token];
    }
    TokenID i = this->indexReg++;
    this->s2i.insert(std::make_pair(Token, i));
    this->i2s.insert(std::make_pair(i, Token));
    return i;
}

const std::string &TokenManage::operator[](const TokenID &ID) {
    if (this->i2s.find(ID) != this->i2s.end()) {
        return this->i2s[ID];
    }
}

TypeID TypeManage::getTypeID(TokenID Name) {
    if (this->tk2ty.find(Name) != this->tk2ty.end()) {
        throw std::runtime_error("Referenced an undefined type.");
    }
    return this->tk2ty[Name];
}

void TypeManage::registType(TokenID Name, enum typeDefType DeclareType, MemberLists &MemberList) {
    if (this->tk2ty.find(Name) != this->tk2ty.end()) {
        throw std::runtime_error("Redefine the type.");
    }
    TypeID i = this->indexReg++;
    this->tk2ty.insert(std::make_pair(Name, i));
    this->ty2tk.insert(std::make_pair(i, Name));
    switch (DeclareType)
    {
        case typeDefType::DeclareStruct: {
            this->StructsMap[i] = MemberList;
            break;
        }
        case typeDefType::DeclareBase: {
            // If *MemberList is nullptr, it means that the underlying type is being defined.
            // No extra work needs to be done.
            break;
        }
    }
}