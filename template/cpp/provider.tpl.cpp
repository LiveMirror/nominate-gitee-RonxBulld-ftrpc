
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include "json/json.h"
#define __OVER_FTRPC_INNER_CODE__
// #@{FTRPC Provider Head File}@#
#ifdef PROVIDER_DEMO_INSIDE
#include "ftrpc.provider.v2.h"
#endif
#include "TypeDef.h"

#define RETURN do{ Json::StreamWriterBuilder swb; \
               std::unique_ptr<Json::StreamWriter> writer(swb.newStreamWriter()); \
               std::ostringstream os; \
               writer->write(ret, &os); \
               return os.str(); }while(0)
#define FAILED(RESULT) do{ ret["success"] = false; \
                       ret["result"] = RESULT; \
                       RETURN; }while(0)
#define CHECK_PARAM_COUNT(PARAM_COUNT) do{ if(paramCount != (PARAM_COUNT)) \
                                           FAILED("The number of parameters is incorrect."); }while(0)

constexpr unsigned long long int HashStringToInt(const char *str, unsigned long long int hash = 0)
{
    return (*str == 0) ? hash : 101 * HashStringToInt(str + 1) + *str;
}

// #@{FUNCTION_XXX micro}@#
#ifdef PROVIDER_DEMO_INSIDE
#define FUNCTION_Test_requireNewStockInfo 0
#define FUNCTION_Test_testFunction 1
#endif

#ifdef PROVIDER_DEMO_INSIDE
struct Custom
{
    int a;
    std::string b;
    bool c;
    operator Json::Value() {
        Json::Value _value;
        _value["a"] = a;
        _value["b"] = b;
        _value["c"] = c;
        return _value;
    }
};
#endif

class JsonValueExtra : public Json::Value
{
public:
// #@{Custom struct convert method}@#
#ifdef PROVIDER_DEMO_INSIDE
    bool isCustomStruct() {
        if (!this->operator[]("a").isInt()) { return false; }
        if (!this->operator[]("b").isString()) { return false;}
        if (!this->operator[]("c").isBool()) { return false; }
        return true;
    }
    struct Custom asCustomStruct() {
        if (!this->isCustomStruct()) {
            throw std::runtime_error("Cannot parse custom struct");
        }
        struct Custom _custom;
        _custom.a = this->operator[]("a").asInt();
        _custom.b = this->operator[]("b").asString();
        _custom.c = this->operator[]("c").asBool();
        return _custom;
    }
#endif
    JsonValueExtra(const Json::Value &jvalue) : Json::Value(jvalue) { }
    bool isJsonArray(bool (JsonValueExtra::*method)() const) {
        if (!this->isArray()) { return false; }
        for (int index = 0; index < this->size(); index++) {
            if (!(static_cast<JsonValueExtra>(this->operator[](index)).*method)()) {
                return false;
            }
        }
        return true;
    }
    template <class T> const std::vector<T> asJsonArray(T (JsonValueExtra::*method)() const) {
        std::vector<T> cppArray;
        for (int index = 0; index < this->size(); index++) {
            cppArray.push_back((this->operator[](index).*method)());
        }
        return cppArray;
    }
    template <class T> const std::vector<T> asJsonArray(T (Json::Value::*method)() const){
        std::vector<T> cppArray;
        for (int index = 0; index < this->size(); index++) {
            cppArray.push_back((this->operator[](index).*method)());
        }
        return cppArray;
    }
};



std::string ProviderDoCall(const std::string &JSON, void *extraOption)
{
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> reader(crb.newCharReader());
    Json::Value root;
    Json::Value ret;
    JSONCPP_STRING errs;
    const char *str = JSON.c_str();
    bool success = reader->parse(str, str + JSON.length(), &root, &errs);
    ret["success"] = false;
    if (success && errs.size() == 0)
    {
        // Verify and get basic information.
        if (root["type"].asString() != "rpc")
            FAILED("Note RPC");
        if (root["version"].asInt() != FTRPC_VERSION_MAJOR)
            FAILED("Version does not match.");
        ret["type"] = "rpcAnswer";
        unsigned int serial = root["serial"].asUInt();
        ret["serial"] = serial;
        std::string funcName = root["funcName"].asString();
        int paramCount = -1;
        Json::Value param = root["params"];
        if(param.isArray()) {
            paramCount = param.size();
        }
        // The parameter signature verifies and executes the call.
        switch (HashStringToInt(funcName.c_str()))
        {
// #@{Function Check and Call}@#
        }
    }
    ret["success"] = true;
    RETURN;
}