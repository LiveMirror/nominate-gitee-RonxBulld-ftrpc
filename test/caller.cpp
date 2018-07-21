//
// Created by rex on 2018/7/21.
//
// g++ -g -std=c++17 ../test/caller.cpp ../test/provider.cpp json.cpp test.caller.cpp test.provider.cpp

#include <iostream>
#include "../cmake-build-debug/test.caller.h"
std::string ProviderDoCall(const std::string &JSON, void *extraOption = nullptr);

void Test_ns_Check(std::string a) {
    std::cout << "Check Test::ns ...";
    if (a == "test ns") { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

void Test_ns_Return(Test::NewStruct s) {
    std::cout << "Check Test::ns return ...";
    if (s.c == 12 && s.d == true) { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

void Test_request_Check(std::vector<std::string> req, int tz, bool flag) {
    std::cout << "Check Test::request ...";
    if (req.size() == 2 && req[0] == "123" && req[1] == "789" && tz == 9 && flag == true) { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

void Test_request_Return(void) {
    std::cout << "Check Test::request return ... PASSED" << std::endl;
}

void Test_requireNewStockInfo_Check(void) {
    std::cout << "Check Test::requireNewStockInfo ... PASSED" << std::endl;
}

void Test_requireNewStockInfo_Return(std::vector<struct Test::NewInfo> infos) {
    std::cout << "Check Test::requireNewStockInfo return ...";
    if (infos.size() == 1 && infos[0].name == "测试" && infos[0].code == "test" && infos[0].limit == 7799) { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

void Test_sss_Check(struct Test::Custom p, struct Test::Custom q) {
    std::cout << "Check Test::sss ...";
    if (p.a == 12 && p.b == 12.5 && p.c == "123" && p.d == true &&
        q.a == 23 && q.b == 10.5 && q.c == "789" && q.d == false) { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

void Test_sss_Return(struct Test::Custom c) {
    std::cout << "Check Test::sss return ...";
    if (c.a == 35 && c.b == 23 && c.c == "123789" && c.d == false) { std::cout << "PASSED"; } else { std::cout << "FAIL"; }
    std::cout << std::endl;
}

int main()
{
    ReturnRecived(ProviderDoCall(Test::ns("test ns", Test_ns_Return), (void*)Test_ns_Check));
    std::vector<std::string> vs;
    vs.push_back("123"); vs.push_back("789");
    ReturnRecived(ProviderDoCall(Test::request(vs, 9, true, Test_request_Return), (void*)Test_request_Check));
    ReturnRecived(ProviderDoCall(Test::requireNewStockInfo(Test_requireNewStockInfo_Return), (void*)Test_requireNewStockInfo_Check));
    Test::Custom p, q;
    p.a = 12; p.b = 12.5; p.c = "123"; p.d = true;
    q.a = 23; q.b = 10.5; q.c = "789"; q.d = false;
    ReturnRecived(ProviderDoCall(Test::sss(p, q, Test_sss_Return), (void*)Test_sss_Check));
    return 0;
}