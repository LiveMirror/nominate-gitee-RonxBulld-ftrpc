//
// Created by Rexfield on 2018/5/2.
//

#include <map>
#include <cstring>
#include <cctype>
#include <tuple>
#include <fstream>
#include <exception>
#include "GenTS.h"
#include "GenUtils.h"

// #define PROVIDER_TPL_FILE "./template/typescript/provider.tpl.ts"
#define CALLER_TPL_FILE "./template/typescript/caller.tpl.ts"

bool GenerateTypeScript_Caller(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem) {
    char ts_file_name[32];
    sprintf(ts_file_name, "ftrpc.caller%s.ts", hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    std::string CallerTplFile = ReadFileAsTxt(CALLER_TPL_FILE);
    std::string FunctionWithCallBack;
    std::string VersionString("\nlet version: number = ");
    VersionString.append(std::to_string(document->version)).append(";\n");
    CallerTplFile.insert(0, VersionString);
    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        FunctionWithCallBack.append("export class ").append(CurModuleName).append(" {\n");
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName, FunctionParams;
            FullApiName.append(CurModuleName).append("::").append(ApiName);
            // Params
            FunctionWithCallBack.append("\tpublic static ").append(ApiName).append("(");
            int paramIndex = 0;
            for(auto &param : api.params) {
                std::string paramName = tokenSystem[param.name];
                FunctionWithCallBack.append(paramName).append(": ").append(GetTsType(param.type.type)).append(", ");
                FunctionParams.append(paramName).append(", ");
                paramIndex++;
            }
            if (!api.params.empty()) {
                FunctionParams.erase(FunctionParams.end() - 2, FunctionParams.end());
            }
            FunctionWithCallBack.append("_callback: (");
            if (api.retType.type != TY_void) {
                FunctionWithCallBack.append("RetValue: ").append(GetTsType(api.retType.type));
            }
            FunctionWithCallBack.append(") => void): string {\n");
            FunctionWithCallBack.append("\t\tlet thisSerial = ftrpc_caller.serial++;\n");
            FunctionWithCallBack.append("\t\tlet reqStruct: rpcPack = {\n"
                                        "\t\t\ttype: \"rpc\",\n");
            FunctionWithCallBack.append("\t\t\tversion: ").append(std::to_string(document->version)).append(",\n");
            FunctionWithCallBack.append("\t\t\tserial: thisSerial,\n"
                                        "\t\t\tfuncName: \"").append(FullApiName).append("\",\n");
            FunctionWithCallBack.append("\t\t\tparams: [").append(FunctionParams).append("],\n");
            FunctionWithCallBack.append("\t\t};\n");
            if (api.retType.type ==  TY_void) {
                FunctionWithCallBack.append("\t\tftrpc_caller.callbackMap_noret[thisSerial] = _callback;\n");
            } else {
                FunctionWithCallBack.append("\t\tftrpc_caller.callbackMap[thisSerial] = _callback;\n");
            }
            FunctionWithCallBack.append("\t\treturn JSON.stringify(reqStruct);\n"
                                        "\t}\n");
        }
        FunctionWithCallBack.append("}\n\n");
    }
    substring_replace(CallerTplFile, "// #@{Non-blocking RPC with callback}@#", FunctionWithCallBack);
    static const char *HeadText = "/*\n"
                                  " * Auto generate by ftrpc\n"
                                  " * Created by Rexfield on 2018/5/2\n"
                                  " * Warning: Please do not modify any code unless you know what you are doing.\n"
                                  " */";
    FILE *pCallerSrcFile = fopen(ts_file_name, "w+");
    fputs(HeadText, pCallerSrcFile);
    fputs(CallerTplFile.c_str(), pCallerSrcFile);
    fclose(pCallerSrcFile);
}

bool GenerateTypeScript(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem){
    GenerateTypeScript_Caller(document, tokenSystem, typeSystem);
}