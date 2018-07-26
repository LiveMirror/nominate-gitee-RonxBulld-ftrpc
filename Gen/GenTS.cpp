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

#define PROVIDER_TPL_FILE "./template/typescript/provider.tpl.ts"
#define CALLER_TPL_FILE "./template/typescript/caller.tpl.ts"


std::string GenerateTypeScript_StructDeclare(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, unsigned int tabLevel = 0) {
    std::string TabFormat(tabLevel, '\t');
    auto &members = typeSystem.StructsMap[id];
    std::string Structure = TabFormat;
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    Structure += "export class " + name + " {\n";
    for (const auto [type,token] : members) {
        Structure += TabFormat + "\t" + tokenSystem[token] + ": " + GetTsType(type) + ";\n";
    }
    Structure += TabFormat + "}\n";
    return Structure;
}

bool GenerateTypeScript_Caller(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix) {
    char ts_file_name[32];
    sprintf(ts_file_name, "%s.caller%s.ts", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    std::string CallerTplFile = ReadTemplate(CALLER_TPL_FILE);
    std::string ModuleDefine, VerifyReturnAndCall;
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
            FullApiName = CurModuleName + "::" + ApiName;
            // Params
            ModuleDefine.append("\texport function ").append(ApiName).append("(");
            int paramIndex = 0;
            for(auto &param : api.params) {
                std::string paramName = tokenSystem[param.name];
                ModuleDefine += paramName + ": " + GetTsType(param.type) + ", ";
                FunctionParams.append(paramName).append(", ");
                paramIndex++;
            }
            VerifyReturnAndCall += "\t\t\tcase \"" + FullApiName + "\":\n";
            std::string VerifyFmt;
            if (api.retType.type != TY_void) {
                if (api.retType.isArray) {
                    VerifyFmt = "\t\t\t\tif (!(ansStruct.return instanceof Array)) return false;\n"
                                "\t\t\t\tfor (let item of ansStruct.return) {\n"
                                "\t\t\t\t\t// #@{VerifyElement}@#\n"
                                "\t\t\t\t}\n";
                } else {
                    VerifyFmt = "\t\t\t\t// #@{VerifyElement}@#\n";
                }
                if (isBaseType(api.retType.type)) {
                    substring_replace(VerifyFmt, "// #@{VerifyElement}@#", "if (!(typeof ansStruct.return === \"" + GetTsTypeBase(api.retType) + "\")) return false;");
                } else {
                    substring_replace(VerifyFmt, "// #@{VerifyElement}@#", "if (!(ansStruct.return instanceof " + GetTsTypeBase(api.retType) + ")) return false;");
                }
            } else {
                VerifyFmt = "\t\t\t\tif (!(typeof ansStruct.return === \"undefined\")) return false;\n";
            }
            VerifyReturnAndCall += VerifyFmt + "\t\t\t\tbreak;\n";
            if (!api.params.empty()) {
                FunctionParams.erase(FunctionParams.end() - 2, FunctionParams.end());
            }
            std::string ReternValue;
            if (api.retType.type != TY_void) {
                ReternValue = "RetValue: "+ GetTsType(api.retType) + ", ";
            }
            ModuleDefine += "_callback: (" + ReternValue + "extraOption?:any) => void): string {\n"
                            "\t\tlet thisSerial = ftrpc_caller.serial++;\n"
                            "\t\tlet reqStruct: rpcPack = {\n"
                            "\t\t\tframework_version: " PROGRAM_VERSION_STR ",\n"
                            "\t\t\ttype: \"rpc\",\n"
                            "\t\t\tversion: " + std::to_string(document->version) + ",\n"
                            "\t\t\tserial: thisSerial,\n"
                            "\t\t\tfuncName: \"" + FullApiName + "\",\n"
                            "\t\t\tparams: [" + FunctionParams + "],\n"
                            "\t\t};\n";
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
    substring_replace(CallerTplFile, "// #@{Verify Return Value And Call}@#", VerifyReturnAndCall);
    substring_replace(CallerTplFile, "// #@{FRAMEWORK_VERSION}@#", "public framework_version : number = " PROGRAM_VERSION_STR ";");
    FILE *pCallerSrcFile = fopen(ts_file_name, "w+");
#define NOTIFICATION(s) " * " s "\n"
    fprintf(pCallerSrcFile, "/*\n"
#include "../Notification.txt"
                            " */"
                            "%s", CallerTplFile.c_str());
#undef NOTIFICATION
    fclose(pCallerSrcFile);
}

bool GenerateTypeScript_Provider(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix) {
    char ts_file_name[32];
    sprintf(ts_file_name, "%s.provider%s.ts", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    std::string ProviderTplFile = ReadTemplate(PROVIDER_TPL_FILE);
    std::string ModuleDefine, VerifyParamAndCall;
    std::string VersionString("\nlet version: number = " + std::to_string(document->version) + ";\n");
    ProviderTplFile.insert(0, VersionString);
    // Module
    for (auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        ModuleDefine += "export module " + CurModuleName + " {\n";
        // Structure
        for (auto & structure : module.structs) {
            std::string code = GenerateTypeScript_StructDeclare(structure.type, tokenSystem, typeSystem, 1);
            ModuleDefine += code;
        }
        // Api
        int apiIndex = 0;
        for (auto & api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName, ParamsCheck;
            FullApiName = CurModuleName + "::" + ApiName;
            // Params
            ModuleDefine += "\texport async function " + ApiName + "(";
            int paramIndex = 0;
            for (auto & param : api.params) {
                std::string paramName = tokenSystem[param.name];
                ModuleDefine += paramName + ": " + GetTsType(param.type) + ", ";

                std::string VerifyFmt, paramRefStr = "param[" + std::to_string(paramIndex) + "]";
                if (param.type.type != TY_void) {
                    if (param.type.isArray) {
                        VerifyFmt = "\t\t\tif (!(" + paramRefStr + " instanceof Array)) { return JSON.stringify(ret); }\n"
                                    "\t\t\tfor (let item of " + paramRefStr + ") {\n"
                                    "\t\t\t\t// #@{VerifyElement}@#\n"
                                    "\t\t\t}\n";
                    } else {
                        VerifyFmt = "\t\t\t// #@{VerifyElement}@#\n";
                    }
                    if (isBaseType(param.type.type)) {
                        substring_replace(VerifyFmt, "// #@{VerifyElement}@#", "if (!(typeof " + paramRefStr + " === \"" + GetTsTypeBase(param.type) + "\")) { return JSON.stringify(ret); }");
                    } else {
                        substring_replace(VerifyFmt, "// #@{VerifyElement}@#", "if (!(" + paramRefStr + " instanceof " + GetTsTypeBase(param.type) + ")) { return JSON.stringify(ret); }");
                    }
                } else {
                    VerifyFmt = "\t\t\tif (!(typeof " + paramRefStr + " === \"undefined\")) { return JSON.stringify(ret); }\n";
                }
                ParamsCheck += VerifyFmt;

                paramIndex++;
            }
            ModuleDefine += "extraOption?:any): Promise<" + GetTsType(api.retType) + "> { throw \"Function " + CurModuleName + "." + ApiName + " implementation is missing.\"; }\n";
            VerifyParamAndCall += "\t\t\tcase \"" + FullApiName + "\":\n"
                                  "\t\t\t\tif (paramCount != " + std::to_string(api.params.size()) + ") { return JSON.stringify(ret); }\n"
                                  + ParamsCheck;
            if (api.retType.type != TY_void) {
                VerifyParamAndCall += "\t\t\t\tret[\"return\"] = ";
            } else {
                VerifyParamAndCall += "\t\t\t\t";
            }
            VerifyParamAndCall += CurModuleName + "." + ApiName + "(";
            for (int i = 0; i < api.params.size(); i++) {
                VerifyParamAndCall += "param[" + std::to_string(i) + "], ";
            }
            VerifyParamAndCall += "extraOption);\n"
                                  "\t\t\t\tbreak;\n";
        }
        ModuleDefine += "}";
    }
    substring_replace(ProviderTplFile, "// #@{Provider Classes}@#", ModuleDefine);
    substring_replace(ProviderTplFile, "// #@{FRAMEWORK_VERSION}@#", "public framework_version : number = " PROGRAM_VERSION_STR ";");
    substring_replace(ProviderTplFile, "// #@{Set Framework Version}@#", "ret[\"framework_version\"] = " PROGRAM_VERSION_STR ";");
    substring_replace(ProviderTplFile, "// #@{Function Check and Call}@#", VerifyParamAndCall);
    FILE *pProviderSrcFile = fopen(ts_file_name, "w+");
#define NOTIFICATION(s) " * " s "\n"
    fprintf(pProviderSrcFile, "/*\n"
                            #include "../Notification.txt"
                            " */"
                            "%s", ProviderTplFile.c_str());
#undef NOTIFICATION
    fclose(pProviderSrcFile);
}

bool GenerateTypeScript(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix){
    GenerateTypeScript_Caller(document, tokenSystem, typeSystem, prefix);
    GenerateTypeScript_Provider(document, tokenSystem, typeSystem, prefix);
}