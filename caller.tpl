
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include "ftrpc.caller.h"
#include "json/json.h"
#include "TypeDef.h"

unsigned int GlobalSerialIndex = 0;

#define RETURN do{ Json::StreamWriterBuilder swb; \
               std::unique_ptr<Json::StreamWriter> writer(swb.newStreamWriter()); \
               std::ostringstream os; \
               writer->write(ret, &os); \
               return os.str(); }while(0)

#define BUILD_JSON_HEAD(jv, FuncName) do{ jv["type"] = "rpc"; \
                                      jv["serial"] = serial; \
                                      jv["funcName"] = FuncName; } while(0)

std::map<unsigned int, void *> serialCallbackMap;

// @#{Non-blocking RPC with callback}@#
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
    serialCallbackMap[serial] = (void*)_callback;
    RETURN;
}
#endif

bool ReturnRecived(std::string JSON)
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
        void *cbfptr = serialCallbackMap[serial];
        serialCallbackMap.erase(serial);
        if (!root["success"].asBool())
            return false;
        switch (root["return"].type())
        {
            case Json::ValueType::booleanValue:
                (*(void(*)(bool))(cbfptr))(root["return"].asBool());
                break;
            case Json::ValueType::intValue:
                (*(void(*)(int))(cbfptr))(root["return"].asInt());
                break;
            case Json::ValueType::nullValue:
                (*(void(*)(void))(cbfptr))();
                break;
            case Json::ValueType::stringValue:
                (*(void(*)(std::string))(cbfptr))(root["return"].asString());
                break;
            case Json::ValueType::realValue:
                (*(void(*)(float))(cbfptr))(root["return"].asFloat());
                break;
        }
        return true;
    } else {
        return false;
    }
}