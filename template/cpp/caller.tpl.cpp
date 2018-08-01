
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include <mutex>
#include "json/json.h"
#define __OVER_FTRPC_INNER_CODE__
// #@{FTRPC Caller Head File}@#
#ifdef PROVIDER_DEMO_INSIDE
#include "ftrpc.caller.v2.h"
#endif
#include "TypeDef.h"

unsigned int GlobalSerialIndex = 0;

#define RETURN do{ Json::StreamWriterBuilder swb; \
               std::unique_ptr<Json::StreamWriter> writer(swb.newStreamWriter()); \
               std::ostringstream os; \
               writer->write(ret, &os); \
               return os.str(); }while(0)
#define FAILED(RESULT) do{ throw std::runtime_error(RESULT); \
                       return false; }while(0)
#define BUILD_JSON_HEAD(jv, FuncName) do{ jv["type"] = "rpc"; \
                                      jv["serial"] = serial; \
                                      jv["funcName"] = FuncName; } while(0)

std::map<unsigned int, void *> serialCallbackMap;
std::mutex scmapMutex;

constexpr unsigned long long int HashStringToInt(const char *str, unsigned long long int hash = 0)
{
    return (*str == 0) ? hash : 101 * HashStringToInt(str + 1) + *str;
}

class JsonValueExtra : public Json::Value
{
public:
// #@{Custom struct convert method}@#
#ifdef CALLER_DEMO_INSIDE
    bool isCustomStruct() {
        if (!this->operator[]("a").isInt()) { return false; }
        if (!this->operator[]("b").isString()) { return false;}
        if (!this->operator[]("c").isBool()) { return false; }
        return true;
    }
    struct CustomStruct asCustomStruct() {
        if (!this->isCustomStruct()) {
            throw std::runtime_error("Cannot parse custom struct");
        }
        struct CustomStruct _custom;
        _custom.a = this->operator[]("a").asInt();
        _custom.b = this->operator[]("b").asString();
        _custom.c = this->operator[]("c").asBool();
        return _custom;
    }
#endif
    JsonValueExtra(const Json::Value &jvalue) : Json::Value(jvalue) { }
    bool isJsonArray(bool (JsonValueExtra::*method)()) {
        if (!this->isArray()) { return false; }
        for (int index = 0; index < this->size(); index++) {
            if (!(static_cast<JsonValueExtra>(this->operator[](index)).*method)()) {
                return false;
            }
        }
        return true;
    }
    bool isJsonArray(bool (JsonValueExtra::*method)() const) {
        if (!this->isArray()) { return false; }
        for (int index = 0; index < this->size(); index++) {
            if (!(static_cast<JsonValueExtra>(this->operator[](index)).*method)()) {
                return false;
            }
        }
        return true;
    }
    template <class T> std::vector<T> asJsonArray(T (JsonValueExtra::*method)()) {
        std::vector<T> cppArray;
        for (int index = 0; index < this->size(); index++) {
            cppArray.push_back((((JsonValueExtra)this->operator[](index)).*method)());
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

template <class T> Json::Value CppArrayToJson(T && cppArray) {
    Json::Value arrayObj;
    arrayObj.resize(0);
    for (int index = 0; index < cppArray.size(); index++)
        arrayObj[index] = (Json::Value)cppArray[index];
    return arrayObj;
}

// #@{Non-blocking RPC with callback}@#
#ifdef PROVIDER_DEMO_INSIDE
std::string Test::request(std::string req, void(*_callback)(void))
{
    Json::Value ret;
    unsigned int serial = GlobalSerialIndex++;
    BUILD_JSON_HEAD(ret, "Test::request");
    Json::Value params;
    params[0] = req;
    ret["params"] = params;
    ret["version"] = FTRPC_VERSION_MAJOR;
    scmapMutex.lock();
    serialCallbackMap[serial] = (void*)_callback;
    scmapMutex.unlock();
    RETURN;
}
#endif

bool ReturnRecived(std::string JSON, void *extraOption)
{
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> reader(crb.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;
    const char *str = JSON.c_str();
    bool success = reader->parse(str, str + JSON.length(), &root, &errs);
    if (success && errs.size() == 0)
    {
        if (root["type"].asString() != "rpcAnswer") {
            return false;
        }
        if (root["version"].asInt() != FTRPC_VERSION_MAJOR) {
            return false;
        }
        unsigned int serial = root["serial"].asUInt();
        std::string funcName = root["funcName"].asString();
        scmapMutex.lock();
        void *cbfptr = serialCallbackMap[serial];
        serialCallbackMap.erase(serial);
        scmapMutex.unlock();
        if (!root["success"].asBool())
            return false;
        // The parameter signature verifies and executes the call.
        switch (HashStringToInt(funcName.c_str()))
        {
// #@{Callback Check and Call}@#
        }
        return true;
    } else {
        return false;
    }
}