
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include "ftrpc.provider.h"
#include "json/json.h"
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
#define CHECK_PARAM_SIGNATURE(Module, Function) do{ if(paramentSignature == FunctionSignatureDictionary[FUNCTION_##Module##_##Function]) \
                                                    FAILED("The parameter type signature is inconsistent with the definition."); }while(0)

constexpr unsigned long long int HashStringToInt(const char *str, unsigned long long int hash = 0)
{
    return (*str == 0) ? hash : 101 * HashStringToInt(str + 1) + *str;
}

std::map<unsigned int, const char> ti2c = {
        {TY_float,  'f'},
        {TY_string, 's'},
        {TY_int,    'i'},
        {TY_bool,   'b'}
};

/*
 * Mapping Json::ValueType to function signature
 */
std::map<Json::ValueType, const char> jvt2c = {
        {Json::ValueType::realValue,    ti2c[TY_float]},
        {Json::ValueType::stringValue,  ti2c[TY_string]},
        {Json::ValueType::intValue,     ti2c[TY_int]},
        {Json::ValueType::booleanValue, ti2c[TY_bool]}
};

// #@{FUNCTION_XXX micro}@#
#ifdef PROVIDER_DEMO_INSIDE
#define FUNCTION_Test_requireNewStockInfo 0
#define FUNCTION_Test_testFunction 1
#endif

std::map<unsigned int, const std::string> FunctionSignatureDictionary = {
// #@{Function Signature}@#
#ifdef PROVIDER_DEMO_INSIDE
        { FUNCTION_Test_requireNewStockInfo, std::string({}) },
        { FUNCTION_Test_testFunction, std::string({ti2c[TY_float], ti2c[TY_string], ti2c[TY_int], ti2c[TY_bool], ti2c[TY_string], ti2c[TY_float]}) },
#endif
};


std::string ProviderDoCall(const std::string &JSON)
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
        if(root["params"].isArray()) {
            paramCount = root["params"].size();
        }
        // Collect parameter signatures.
        std::string paramentSignature = "";
        for(int i = 0; i < paramCount; i++) {
            Json::Value param = root["params"];
            auto mdp = jvt2c.find(param.type());
            if(mdp == jvt2c.end())
                FAILED("Unsupported parameter type.");
            paramentSignature.push_back(jvt2c[param.type()]);
        }
        // The parameter signature verifies and executes the call.
        switch (HashStringToInt(funcName.c_str()))
        {
// #@{Function Check and Call}@#
#ifdef PROVIDER_DEMO_INSIDE
            case HashStringToInt("Test::requireNewStockInfo"): {
                CHECK_PARAM_COUNT(0);
                CHECK_PARAM_SIGNATURE(Test, requireNewStockInfo);
                Json::Value param = root["params"];
                Test::requireNewStockInfo();
                break;
            }
            case HashStringToInt("Test::testFunction"): {
                CHECK_PARAM_COUNT(6);
                CHECK_PARAM_SIGNATURE(Test, testFunction);
                Json::Value param = root["params"];
                ret["return"] = Test::testFunction(param[0].asFloat(), param[1].asString(), param[2].asInt(),
                                                   param[3].asBool(), param[4].asString(), param[5].asFloat());
                break;
            }
#endif
        }
    }
    ret["success"] = true;
    RETURN;
}