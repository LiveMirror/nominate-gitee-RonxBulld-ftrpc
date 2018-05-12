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

std::string GenerateTypeScript_StructDeclare(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, unsigned int tabLevel = 0) {
    std::string TabFormat(tabLevel, '\t');
    auto &members = typeSystem.StructsMap[id];
    std::string Structure = TabFormat;
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    Structure += "export class " + name + " {\n";
    for (const auto [type,token] : members) {
        Structure += TabFormat + "\t" + tokenSystem[token] + ": " + GetTsType((enum Type) type) + ";\n";
    }
    Structure += TabFormat + "}\n";
    return Structure;
}

bool GenerateTypeScript_Caller(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix) {
    char ts_file_name[32];
    sprintf(ts_file_name, "%s.caller%s.ts", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    std::string CallerTplFile = ReadTemplate(CALLER_TPL_FILE);
    std::string ModuleDefine;
    std::string VersionString("\nlet version: number = ");
    VersionString.append(std::to_string(document->version)).append(";\n");
    CallerTplFile.insert(0, VersionString);
    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        ModuleDefine.append("export module ").append(CurModuleName).append(" {\n");
        // Structure
        for (auto & structure : module.structs) {
            std::string code = GenerateTypeScript_StructDeclare(structure.type, tokenSystem, typeSystem, 1);
            ModuleDefine += code;
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName, FunctionParams;
            FullApiName.append(CurModuleName).append("::").append(ApiName);
            // Params
            ModuleDefine.append("\texport function ").append(ApiName).append("(");
            int paramIndex = 0;
            for(auto &param : api.params) {
                std::string paramName = tokenSystem[param.name];
                ModuleDefine.append(paramName).append(": ").append(GetTsType(param.type.type)).append(", ");
                FunctionParams.append(paramName).append(", ");
                paramIndex++;
            }
            if (!api.params.empty()) {
                FunctionParams.erase(FunctionParams.end() - 2, FunctionParams.end());
            }
            ModuleDefine.append("_callback: (");
            if (api.retType.type != TY_void) {
                ModuleDefine.append("RetValue: ").append(GetTsType(api.retType.type));
            }
            ModuleDefine.append(") => void): string {\n");
            ModuleDefine.append("\t\tlet thisSerial = ftrpc_caller.serial++;\n");
            ModuleDefine.append("\t\tlet reqStruct: rpcPack = {\n"
                                        "\t\t\ttype: \"rpc\",\n");
            ModuleDefine.append("\t\t\tversion: ").append(std::to_string(document->version)).append(",\n");
            ModuleDefine.append("\t\t\tserial: thisSerial,\n"
                                        "\t\t\tfuncName: \"").append(FullApiName).append("\",\n");
            ModuleDefine.append("\t\t\tparams: [").append(FunctionParams).append("],\n");
            ModuleDefine.append("\t\t};\n");
            if (api.retType.type ==  TY_void) {
                ModuleDefine.append("\t\tftrpc_caller.callbackMap_noret[thisSerial] = _callback;\n");
            } else {
                ModuleDefine.append("\t\tftrpc_caller.callbackMap[thisSerial] = _callback;\n");
            }
            ModuleDefine.append("\t\treturn JSON.stringify(reqStruct);\n"
                                        "\t}\n");
        }
        ModuleDefine.append("}\n\n");
    }
    substring_replace(CallerTplFile, "// #@{Non-blocking RPC with callback}@#", ModuleDefine);
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

bool GenerateTypeScript(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix){
    GenerateTypeScript_Caller(document, tokenSystem, typeSystem, prefix);
}