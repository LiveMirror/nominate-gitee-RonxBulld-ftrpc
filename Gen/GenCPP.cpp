//
// Created by Rexfield on 2018/4/29.
//
#include <map>
#include <cstring>
#include <cctype>
#include <tuple>
#include <fstream>
#include <exception>
#include <vector>
#include "GenCPP.h"
#include "GenUtils.h"

#define PROVIDER_TPL_FILE "./template/cpp/provider.tpl.cpp"
#define CALLER_TPL_FILE "./template/cpp/caller.tpl.cpp"

/*
 * 将文件名转换为头文件宏定义符号
 * 字母转换为大写 数字被保留 其它符号被转换为_
 * 例如 a12.test.h ==> __A12_TEST_H__
 */
std::string FilenameToMacro(const char *fn)
{
    size_t len = std::strlen(fn) + 1 + 4;
    char mdef[len];
    for (int i = 0; i < strlen(fn); i++) {
        if (std::isalnum(fn[i])) {
            mdef[i+2] = (char)std::toupper(fn[i]);
        } else {
            mdef[i+2] = '_';
        }
    }
    mdef[0] = mdef[1] = '_';
    mdef[len - 2] = mdef[len - 3] = '_';
    mdef[len - 1] = '\0';
    return std::string(mdef);
}

/**
 * 打开一个文件并写入头文件代码
 * @param fn
 * @param mode
 * @return
 */
FILE *OpenHeadToWrite(const char *fn, const char *mode = "w+")
{
    FILE *fp = fopen(fn, mode);
    std::string mdef = FilenameToMacro(fn);
    fprintf(fp, "#ifndef %s\n"
                "#define %s\n"
                "\n", mdef.c_str(), mdef.c_str());
    return fp;
}

/**
 * 写入头文件结束代码并关闭
 * @param fp
 * @param fn
 */
void CloseHeadToWrite(FILE *fp, const char *fn)
{
    std::string mdef = FilenameToMacro(fn);
    fprintf(fp, "\n"
                "\n#endif // %s\n", mdef.c_str());
    // End generate
    fclose(fp);
}

/**
 * 生成从 C++ 到 JSON 的转换前缀，被转换的变量或者函数调用必须用`(``)`包括
 * Generate convert code from c++ to json, a prefix code will return, '('')' must around variable or function after.
 * @param T
 * @return
 */
const std::string ForceConvert_CPP(TypeNode T) {
    if (!T.isArray) {
        if (isBaseType(T.type)) {
            return "";
        } else {
            return "(Json::Value)";
        }
    } else {
        return "CppArrayToJson";
    }
}

/**
 * 生成结构体定义代码
 * 包含成员变量和类型转换运算符定义
 * @param id
 * @param tokenSystem
 * @param typeSystem
 * @param tabLevel
 * @return
 */
std::string GenerateCPP_StructDeclare(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, unsigned int tabLevel = 0) {
    auto &members = typeSystem.StructsMap[id];
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    // 定义头
    std::string Structure = "$tstruct " + name + "\n"
                            "$t{\n";
    // 定义成员变量
    for (const auto[type, token] : members) {
        Structure += "$t\t" + GetCppType(type) + " " + tokenSystem[token] + ";\n";
    }
    // 定义到 Json::Value 的类型转换函数
    Structure += "#ifdef __OVER_FTRPC_INNER_CODE__\n";
    Structure += "$t\texplicit operator Json::Value() {\n"
                 "$t\t\tJson::Value _value;\n";
    for (const auto [type, token] : members) {
        const std::string & TOKEN = tokenSystem[token];
        std::string memberDefine = "$t\t\t_value[\"#@{token}@#\"] = #@{convert}@#(#@{token}@#);\n";
        substring_replace(memberDefine, "#@{token}@#", TOKEN);
        substring_replace(memberDefine, "#@{convert}@#", ForceConvert_CPP(type));
        Structure += memberDefine;
    }
    Structure += "$t\t\treturn _value;\n"
                 "$t\t}\n"
                 "#endif // __OVER_FTRPC_INNER_CODE__\n"
                 "$t};\n";
    Structure = applyTabLevel(Structure, tabLevel);
    return Structure;
}

/**
 * 生成 Json::Value 到结构体的验证和转换代码
 * @param id
 * @param tokenSystem
 * @param typeSystem
 * @param FieldName
 * @return
 */
std::string GenerateCPP_StructCheckConvert(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &FieldName) {
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    std::string FullName = FieldName + "::" + name;
    auto &members = typeSystem.StructsMap[id];
    // Generate check code
    std::string code = "\tbool is" + name + "Struct() {\n";
    for (const auto & [type, token] : members) {
        const std::string TOKEN = "((JsonValueExtra*)&this->operator[](\"" + tokenSystem[token] + "\"))";
        code += "\t\tif (!" + TOKEN + "->" + GetJsonCheckMethod(type) + ") { return false; }\n";
    }
    code += "\t\treturn true;\n"
            "\t}\n";

    // Generate convert code
    code += "\tstruct " + FullName + " as" + name + "Struct() {\n";
    code += "\t\tif (!this->is" + name + "Struct()) {\n"
            "\t\t\tthrow std::runtime_error(\"Cannot parse as " + FullName + "\");\n"
            "\t\t}\n";
    std::string tempValueName = "__Tmp_" + name + "__";
    code += "\t\tstruct " + FullName + " " + tempValueName + ";\n";
    for (const auto & [type, token] : members) {
        const std::string & TOKEN = tokenSystem[token];
        std::string memberAssignment = "\t\t#@{TemplateVariable}@#.#@{MemberName}@# = ((JsonValueExtra*)&this->operator[](\"#@{MemberName}@#\"))->#@{ConvertMethod}@#;\n";
        substring_replace(memberAssignment, "#@{TemplateVariable}@#", tempValueName);
        substring_replace(memberAssignment, "#@{MemberName}@#", TOKEN);
        substring_replace(memberAssignment, "#@{ConvertMethod}@#", GetJsonConvertMethod(type));
        code += memberAssignment;
    }
    code += "\t\treturn " + tempValueName + ";\n"
            "\t}\n";
    return code;
}

bool GenerateCPP_ProviderHead(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &filename) {
    FILE *pProviderHeaderFile = OpenHeadToWrite(filename.c_str());
    fprintf(pProviderHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document->version);
    fprintf(pProviderHeaderFile, "\n#include <string>\n#include <vector>\n\n"
                                 "std::string ProviderDoCall(const std::string &JSON, void *extraOption = nullptr);\n\n");
    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        fprintf(pProviderHeaderFile, "class %s\n"
                                     "{\n"
                                     "public:\n", CurModuleName.c_str());
        // Structure
        for(auto &structure : module.structs) {
            std::string code = GenerateCPP_StructDeclare(structure.type, tokenSystem, typeSystem, 1);
            fputs(code.c_str(), pProviderHeaderFile);
        }
        // Api
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            fprintf(pProviderHeaderFile, "\tstatic %s %s(", GetCppType(api.retType).c_str(), ApiName.c_str());
            // Params
            std::string CheckParaments;
            for(auto &param : api.params) {
                fprintf(pProviderHeaderFile, "%s %s,", GetCppType(param.type).c_str(), tokenSystem[param.name].c_str());
            }
            fprintf(pProviderHeaderFile, "void *extraOption);\n");
        }
        fprintf(pProviderHeaderFile, "};");
    }
    CloseHeadToWrite(pProviderHeaderFile, filename.c_str());
    return true;
}

bool GenerateCPP_ProviderCode(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &filename) {

    FILE *pProviderSrcFile = fopen(filename.c_str(), "w+");
    std::string ProviderTplFile = ReadTemplate(PROVIDER_TPL_FILE);
    std::string FunctionMicro, FunctionCheckAndCall, JsonExtern;
    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        // Structure
        for(auto &structure : module.structs) {
            JsonExtern += GenerateCPP_StructCheckConvert(structure.type, tokenSystem, typeSystem, CurModuleName);
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName = CurModuleName + "::" + ApiName;
            FunctionMicro += "#define FUNCTION_" + CurModuleName + "_" + ApiName + " " + std::to_string(apiIndex++) + "\n";
            FunctionCheckAndCall += "\t\t\tcase HashStringToInt(\"" + FullApiName + "\"): {\n"
                                    "\t\t\t\tCHECK_PARAM_COUNT(" + std::to_string(api.params.size()) + ");\n";
            // Params
            int paramIndex = 0;
            std::string UseParaments;
            for(auto &param : api.params) {
                // Parament check
                const std::string strParam = "param[" + std::to_string(paramIndex) + "]";
                std::string ParamCheckStat = "\t\t\t\tif (!(((JsonValueExtra*)(&{strParam}))->{JsonCheckMethod})) { FAILED(\"While call \\\"{FullApiName}\\\", arguments {paramIndex} type check error.\"); }\n";
                substring_replace(ParamCheckStat, "{strParam}", strParam);
                substring_replace(ParamCheckStat, "{JsonCheckMethod}", GetJsonCheckMethod(param.type));
                substring_replace(ParamCheckStat, "{FullApiName}", FullApiName);
                substring_replace(ParamCheckStat, "{paramIndex}", std::to_string(paramIndex));
                FunctionCheckAndCall += ParamCheckStat;
                // Parament convert
                UseParaments += "((JsonValueExtra*)(&" + strParam + "))->" + GetJsonConvertMethod(param.type) + ", ";
                paramIndex++;
            }
            std::string ReturnValueAssign;
            if(api.retType.type != TY_void) {
                ReturnValueAssign = "ret[\"return\"] = " + ForceConvert_CPP(api.retType);
            }
            FunctionCheckAndCall += "\t\t\t\t" + ReturnValueAssign + "(" + FullApiName + "(" + UseParaments + "extraOption));\n"
                                    "\t\t\t\tbreak;\n"
                                    "\t\t\t}\n";
        }
    }
    substring_replace(ProviderTplFile, "// #@{FUNCTION_XXX micro}@#", FunctionMicro);
    substring_replace(ProviderTplFile, "// #@{Custom struct convert method}@#", JsonExtern + "// #@{Custom struct convert method}@#\n");
    substring_replace(ProviderTplFile, "// #@{Function Check and Call}@#", FunctionCheckAndCall);
    substring_replace(ProviderTplFile, "// #@{FTRPC Provider Head File}@#", std::string("#include \"") + filename.substr(0, filename.length() - 3) + "h\"");
    std::string VersionCheckAndSet = "\t\tret[\"framework_version\"] = " PROGRAM_VERSION_STR ";\n"
                                     "\t\tret[\"version\"] = " + std::to_string(document->version) + ";\n";
    VersionCheckAndSet += "\t\tif (!(root[\"framework_version\"].isInt() && root[\"framework_version\"].asInt() == " PROGRAM_VERSION_STR ")) "
                          "{ FAILED(\"Bad Framework Version, require v" PROGRAM_VERSION_STR " \"); }\n";
    substring_replace(ProviderTplFile, "// #@{Framework Version}@#", VersionCheckAndSet);

#define NOTIFICATION(s) " * " s "\n"
    fprintf(pProviderSrcFile, "/*\n"
#include "../Notification.txt"
                              " */"
                              "%s", ProviderTplFile.c_str());
#undef NOTIFICATION
    fclose(pProviderSrcFile);
}

bool GenerateCPP_Provider(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix) {
    char head_file_name[32], src_file_name[32];
    sprintf(head_file_name, "%s.provider%s.h", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    sprintf(src_file_name, "%s.provider%s.cpp", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    GenerateCPP_ProviderHead(document, tokenSystem, typeSystem, head_file_name);
    GenerateCPP_ProviderCode(document, tokenSystem, typeSystem, src_file_name);
    return true;
}

bool GenerateCPP_CallerHead(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &filename) {
    FILE *pCallerHeaderFile = OpenHeadToWrite(filename.c_str());
    fprintf(pCallerHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document->version);
    fprintf(pCallerHeaderFile, "\n#include <string>\n#include <vector>\n\n"
                               "bool ReturnRecived(std::string JSON, void *extraOption = nullptr);\n\n");
    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        fprintf(pCallerHeaderFile, "class %s\n"
                                   "{\n"
                                   "public:\n", CurModuleName.c_str());
        // Structure
        for(auto &structure : module.structs) {
            std::string code = GenerateCPP_StructDeclare(structure.type, tokenSystem, typeSystem, 1);
            fputs(code.c_str(), pCallerHeaderFile);
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            fprintf(pCallerHeaderFile, "\tstatic std::string %s(", ApiName.c_str());
            // Params
            int paramIndex = 0;
            for(auto &param : api.params) {
                std::string paramName = tokenSystem[param.name];
                fprintf(pCallerHeaderFile, "%s %s, ", GetCppType(param.type).c_str(), paramName.c_str());
            }
            fprintf(pCallerHeaderFile, "void(*_callback)(");
            if (api.retType.type != TY_void) {
                fprintf(pCallerHeaderFile, "%s, ", GetCppType(api.retType).c_str());
            }
            fprintf(pCallerHeaderFile, "void *extraOption));\n");
        }
        fprintf(pCallerHeaderFile, "};");
    }
    CloseHeadToWrite(pCallerHeaderFile, filename.c_str());
}

bool GenerateCPP_CallerCode(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &filename) {
    FILE *pCallerSrcFile = fopen(filename.c_str(), "w+");
    std::string CallerTplFile = ReadTemplate(CALLER_TPL_FILE);
    std::string FunctionWithCallBack, JsonExtern, CallbackCheckAndCall;

    // Module
    for(auto &module : document->modules) {
        std::string CurModuleName = tokenSystem[module.name];
        // Structure
        for(auto &structure : module.structs) {
            JsonExtern += GenerateCPP_StructCheckConvert(structure.type, tokenSystem, typeSystem, CurModuleName);
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName, FunctionParams;
            FullApiName.append(CurModuleName).append("::").append(ApiName);

            FunctionWithCallBack.append("std::string ").append(FullApiName).append("(");
            CallbackCheckAndCall += "\t\t\tcase HashStringToInt(\"" + FullApiName + "\"): {\n";
            // Params
            int paramIndex = 0;
            for(auto &param : api.params) {
                const std::string strParam = "params[" + std::to_string(paramIndex) + "]";
                std::string paramName = tokenSystem[param.name];
                FunctionWithCallBack.append(GetCppType(param.type)).append(" ").append(paramName).append(", ");
                FunctionParams += "\t" + strParam + " = " + ForceConvert_CPP(param.type) + "(" + paramName + ");\n";
                paramIndex++;
            }
            std::string ReturnCheckStat = "\t\t\t\tif (!(((JsonValueExtra*)(&root[\"return\"]))->{JsonCheckMethod})) { FAILED(\"While \\\"{FullApiName}\\\" return, type check error.\"); }\n";
            substring_replace(ReturnCheckStat, "{JsonCheckMethod}", GetJsonCheckMethod(api.retType));
            substring_replace(ReturnCheckStat, "{FullApiName}", FullApiName);
            CallbackCheckAndCall += ReturnCheckStat;
            std::string CallbackParam;
            if (api.retType.type != TY_void) {
                CallbackParam = "((JsonValueExtra*)(&root[\"return\"]))->" + GetJsonConvertMethod(api.retType) + ", ";
            }
            CallbackCheckAndCall += "\t\t\t\t(*(void(*)(" + (api.retType.type != TY_void ? GetCppType(api.retType) + ", " : "") + "void*))(cbfptr))(" + CallbackParam + "extraOption);\n"
                                    "\t\t\t\tbreak;\n"
                                    "\t\t\t}\n";
            FunctionWithCallBack += "void(*_callback)(";
            if (api.retType.type != TY_void) {
                FunctionWithCallBack += GetCppType(api.retType) + ", ";
            }
            FunctionWithCallBack.append("void *extraOption))\n{\n"
                                        "\tJson::Value ret;\n"
                                        "\tunsigned int serial = GlobalSerialIndex++;\n"
                                        "\tBUILD_JSON_HEAD(ret, \"").append(FullApiName).append("\");\n");
            FunctionWithCallBack.append("\tJson::Value params;\n");
            FunctionWithCallBack.append(FunctionParams);
            FunctionWithCallBack.append("\tret[\"params\"] = params;\n"
                                        "\tret[\"version\"] = FTRPC_VERSION_MAJOR;\n"
                                        "\tret[\"framework_version\"] = " PROGRAM_VERSION_STR ";\n"
                                        "\tscmapMutex.lock();\n"
                                        "\tserialCallbackMap[serial] = (void*)_callback;\n"
                                        "\tscmapMutex.unlock();\n"
                                        "\tRETURN;\n"
                                        "}\n\n");
        }
    }
    substring_replace(CallerTplFile, "// #@{Non-blocking RPC with callback}@#", FunctionWithCallBack);
    substring_replace(CallerTplFile, "// #@{Callback Check and Call}@#", CallbackCheckAndCall);
    substring_replace(CallerTplFile, "// #@{Custom struct convert method}@#", JsonExtern + "// #@{Custom struct convert method}@#\n");
    substring_replace(CallerTplFile, "// #@{FTRPC Caller Head File}@#", std::string("#include \"") + filename.substr(0, filename.length() - 3) + "h\"");
    std::string VersionCheckAndSet = "\t\tif (!(root[\"framework_version\"].isInt() && root[\"framework_version\"].asInt() == " PROGRAM_VERSION_STR ")) "
                                     "{ FAILED(\"Bad Framework Version, require v" PROGRAM_VERSION_STR " \"); }\n";
    substring_replace(CallerTplFile, "// #@{Framework Version}@#", VersionCheckAndSet);

#define NOTIFICATION(s) " * " s "\n"
    fprintf(pCallerSrcFile, "/*\n"
                            #include "../Notification.txt"
                            " */"
                            "%s", CallerTplFile.c_str());
#undef NOTIFICATION
    fclose(pCallerSrcFile);
}

bool GenerateCPP_Caller(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix) {
    char head_file_name[32], src_file_name[32];
    sprintf(head_file_name, "%s.caller%s.h", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    sprintf(src_file_name, "%s.caller%s.cpp", prefix, hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");

    GenerateCPP_CallerHead(document, tokenSystem, typeSystem, head_file_name);
    GenerateCPP_CallerCode(document, tokenSystem, typeSystem, src_file_name);
}

bool GenerateCPP(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem, const char *prefix)
{
    GenerateCPP_Provider(document, tokenSystem, typeSystem, prefix);
    GenerateCPP_Caller(document, tokenSystem, typeSystem, prefix);
    return true;
}