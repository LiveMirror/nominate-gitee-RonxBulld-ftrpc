//
// Created by rex on 2018/7/21.
//

#include <iostream>
#include "../cmake-build-debug/test.provider.h"

void Test::request(std::vector<std::string> req, int tz, bool flag, void *extraOption) {
    ((void(*)(std::vector<std::string>, int, bool))extraOption)(req, tz, flag);
}

struct Test::NewStruct Test::ns(std::string a, void *extraOption) {
    ((void(*)(std::string))extraOption)(a);
    struct Test::NewStruct s;
    s.c = 12;
    s.d = true;
    return s;
}

std::vector<struct Test::NewInfo> Test::requireNewStockInfo(void *extraOption) {
    ((void(*)())extraOption)();
    std::vector<struct Test::NewInfo> sv;
    struct Test::NewInfo n;
    n.name = "测试"; n.code = "test"; n.limit = 7799;
    sv.push_back(n);
    return sv;
}

struct Test::Custom Test::sss(struct Test::Custom p, struct Test::Custom q, void *extraOption) {
    ((void(*)(struct Test::Custom, struct Test::Custom))extraOption)(p, q);
    Test::Custom c;
    c.a = p.a + q.a;
    c.b = p.b + q.b;
    c.c = p.c + q.c;
    c.d = p.d && q.d;
    return c;
}