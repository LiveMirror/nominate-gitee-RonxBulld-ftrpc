//
// Created by Rexfield on 2018/4/29.
//
#include <map>
#include <cstring>
#include <cctype>
#include <tuple>
#include <fstream>
#include <exception>
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

FILE *OpenHeadToWrite(const char *fn, const char *mode = "w+")
{
    FILE *fp = fopen(fn, mode);
    std::string mdef = FilenameToMacro(fn);
    fprintf(fp, "#ifndef %s\n"
                "#define %s\n"
                "\n", mdef.c_str(), mdef.c_str());
    return fp;
}

void CloseHeadToWrite(FILE *fp, const char *fn)
{
    std::string mdef = FilenameToMacro(fn);
    fprintf(fp, "\n"
                "\n#endif // %s\n", mdef.c_str());
    // End generate
    fclose(fp);
}

std::string GenerateCPP_StructDeclare(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, unsigned int tabLevel = 0) {
    std::string TabFormat(tabLevel, '\t');
    auto &members = typeSystem.StructsMap[id];
    std::string Structure = TabFormat;
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    Structure += "struct " + name + "\n" + TabFormat + "{\n";
    for (const auto[type, token] : members) {
        Structure += TabFormat + "\t" + GetCppType((enum Type) type) + " " + tokenSystem[token] + ";\n";
    }
    Structure += TabFormat + "\texplicit operator Json::Value() {\n"
               + TabFormat + "\t\tJson::Value _value;\n";
    for (const auto [type, token] : members) {
        Structure += TabFormat + "\t\t_value[\"" + tokenSystem[token] + "\"] = " + tokenSystem[token] + ";\n";
    }
    Structure += TabFormat + "\t\treturn _value;\n"
               + TabFormat + "\t}\n";
    Structure += TabFormat + "};\n";
    return Structure;
}

std::string GenerateCPP_StructCheckConvert(TypeID id, TokenManage &tokenSystem, TypeManage &typeSystem, const std::string &FieldName) {
    std::string name = tokenSystem[typeSystem.ty2tk[id]];
    std::string FullName = FieldName + "::" + name;
    auto &members = typeSystem.StructsMap[id];
    std::string code = "\tbool is" + name + "Struct() {\n";
    for (const auto & [type, token] : members) {
        code += "\t\tif (!this->operator[](\"" + tokenSystem[token] + "\")." + GetJsonCheckMethod((enum Type)type) + "()) { return false; }\n";
    }
    code += "\t\treturn true;\n"
            "\t}\n";

    code += "\tstruct " + FullName + " as" + name + "Struct() {\n";
    code += "\t\tif (!this->is" + name + "Struct()) {\n"
            "\t\t\tthrow std::runtime_error(\"Cannot parse as " + FullName + "\");\n"
            "\t\t}\n";
    std::string tempValueName = "__Tmp_" + name + "__";
    code += "\t\tstruct " + FullName + " " + tempValueName + ";\n";
    for (const auto & [type, token] : members) {
        code += "\t\t" + tempValueName + "." + tokenSystem[token] + " = this->operator[](\"" + tokenSystem[token] + "\")." + GetJsonConvertMethod((enum Type)type) + "();\n";
    }
    code += "\t\treturn " + tempValueName + ";\n"
            "\t}\n";
    return code;
}

bool GenerateCPP_Provider(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem)
{
    char head_file_name[32], src_file_name[32];
    sprintf(head_file_name, "ftrpc.provider%s.h", hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    sprintf(src_file_name, "ftrpc.provider%s.cpp", hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");

    FILE *pProviderHeaderFile = OpenHeadToWrite(head_file_name);
    fprintf(pProviderHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document->version);
    fprintf(pProviderHeaderFile, "\n#include <string>\n\n"
                                 "std::string ProviderDoCall(const std::string &JSON);\n\n");
    FILE *pProviderSrcFile = fopen(src_file_name, "w+");
    std::string ProviderTplFile = ReadFileAsTxt(PROVIDER_TPL_FILE);
    std::string FunctionMicro, FunctionCheckAndCall, JsonExtern;
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
            JsonExtern += GenerateCPP_StructCheckConvert(structure.type, tokenSystem, typeSystem, CurModuleName);
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            std::string FullApiName;
            FullApiName.append(CurModuleName).append("::").append(ApiName);

            fprintf(pProviderHeaderFile, "\tstatic %s %s(", GetCppType(api.retType.type).c_str(), ApiName.c_str());

            FunctionMicro.append("#define FUNCTION_").append(CurModuleName).append("_").append(ApiName).append(" ").append(std::to_string(apiIndex++)).append(1,'\n');
            FunctionCheckAndCall.append("\t\t\tcase HashStringToInt(\"").append(FullApiName).append("\"): {\n\t\t\t\t");
            FunctionCheckAndCall.append("CHECK_PARAM_COUNT(").append(std::to_string(api.params.size())).append(");\n");
            FunctionCheckAndCall.append("// #@{Arguments Type Check}@#\n\t\t\t\t");
            if(api.retType.type != TY_void) {
                FunctionCheckAndCall.append("ret[\"return\"] = ");
                FunctionCheckAndCall.append(ForceConvert_CPP(api.retType.type));
            }
            FunctionCheckAndCall.append(FullApiName).append("(");
            // Params
            int paramIndex = 0;
            std::string CheckParaments;
            for(auto &param : api.params) {
                fprintf(pProviderHeaderFile, "%s %s,", GetCppType(param.type.type).c_str(), tokenSystem[param.name].c_str());
                CheckParaments.append("((JsonValueExtra*)(&param[").append(std::to_string(paramIndex)).append("]))->").append(GetJsonCheckMethod(param.type.type)).append("() && ");
                FunctionCheckAndCall.append("((JsonValueExtra*)(&param[").append(std::to_string(paramIndex)).append("]))->").append(
                        GetJsonConvertMethod(param.type.type)).append("(), ");
                paramIndex++;
            }
            if(!api.params.empty()) {
                fseek(pProviderHeaderFile, -1, SEEK_CUR);
                FunctionCheckAndCall.erase(FunctionCheckAndCall.end() - 2, FunctionCheckAndCall.end());
                CheckParaments.insert(0, "\t\t\t\tif (!(");
                CheckParaments.erase(CheckParaments.end() - 4, CheckParaments.end());
                CheckParaments.append(")) {\n\t\t\t\t\tFAILED(\"Arguments type check error.\");\n\t\t\t\t}\n");
            }
            substring_replace(FunctionCheckAndCall, "// #@{Arguments Type Check}@#\n", CheckParaments);
            fprintf(pProviderHeaderFile, ");\n");
            FunctionCheckAndCall.append(");\n\t\t\t\tbreak;\n\t\t\t}\n");
        }
        fprintf(pProviderHeaderFile, "};");
    }
    substring_replace(ProviderTplFile, "// #@{FUNCTION_XXX micro}@#", FunctionMicro);
    substring_replace(ProviderTplFile, "// #@{Custom struct convert method}@#", JsonExtern + "// #@{Custom struct convert method}@#\n");
    substring_replace(ProviderTplFile, "// #@{Function Check and Call}@#", FunctionCheckAndCall);
    substring_replace(ProviderTplFile, "// #@{FTRPC Provider Head File}@#", std::string("#include \"") + head_file_name + "\"");

    fprintf(pProviderSrcFile, "/*\n"
                              " * Auto generate by ftrpc\n"
                              " * Created by Rexfield on 2018/5/1\n"
                              " * Warning: Please do not modify any code unless you know what you are doing.\n"
                              " */"
                              "%s", ProviderTplFile.c_str());
    fclose(pProviderSrcFile);
    CloseHeadToWrite(pProviderHeaderFile, head_file_name);
    return true;
}

bool GenerateCPP_Caller(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem)
{
    char head_file_name[32], src_file_name[32];
    sprintf(head_file_name, "ftrpc.caller%s.h", hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");
    sprintf(src_file_name, "ftrpc.caller%s.cpp", hadVersionInfo ? ".v" PROGRAM_VERSION_STR : "");

    FILE *pCallerHeaderFile = OpenHeadToWrite(head_file_name);
    fprintf(pCallerHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document->version);
    fprintf(pCallerHeaderFile, "\n#include <string>\n\n");

    FILE *pCallerSrcFile = fopen(src_file_name, "w+");
    std::string CallerTplFile = ReadFileAsTxt(CALLER_TPL_FILE);

    std::string FunctionWithCallBack, JsonExtern;

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
            JsonExtern += GenerateCPP_StructCheckConvert(structure.type, tokenSystem, typeSystem, CurModuleName);
        }
        // Api
        int apiIndex = 0;
        for(auto &api : module.apis) {
            std::string ApiName = tokenSystem[api.name];
            fprintf(pCallerHeaderFile, "\tstatic std::string %s(", ApiName.c_str());
            std::string FullApiName, FunctionParams;
            FullApiName.append(CurModuleName).append("::").append(ApiName);

            FunctionWithCallBack.append("std::string ").append(FullApiName).append("(");
            // Params
            int paramIndex = 0;
            for(auto &param : api.params) {
                std::string paramName = tokenSystem[param.name];
                fprintf(pCallerHeaderFile, "%s %s, ", GetCppType(param.type.type).c_str(), paramName.c_str());
                FunctionWithCallBack.append(GetCppType(param.type.type)).append(" ").append(paramName).append(", ");
                FunctionParams.append("\tparams[").append(std::to_string(paramIndex)).append("] = ").append(
                        ForceConvert_CPP(param.type.type)).append(paramName).append(";\n");
                paramIndex++;
            }
            FunctionWithCallBack.append("void(*_callback)(");
            if (api.retType.type != TY_void) {
                FunctionWithCallBack.append(GetCppType(api.retType.type));
            }
            FunctionWithCallBack.append("))\n{\n");
            FunctionWithCallBack.append("\tJson::Value ret;\n"
                                        "\tunsigned int serial = GlobalSerialIndex++;\n"
                                        "\tBUILD_JSON_HEAD(ret, \"").append(FullApiName).append("\");\n");
            FunctionWithCallBack.append("\tJson::Value params;\n");
            FunctionWithCallBack.append(FunctionParams);
            FunctionWithCallBack.append("\tret[\"params\"] = params;\n"
                                        "\tret[\"version\"] = FTRPC_VERSION_MAJOR;\n"
                                        "\tscmapMutex.lock();\n"
                                        "\tserialCallbackMap[serial] = (void*)_callback;\n"
                                        "\tscmapMutex.unlock();\n"
                                        "\tRETURN;\n"
                                        "}\n\n");
            fprintf(pCallerHeaderFile, "void(*_callback)(%s));\n", GetCppType(api.retType.type).c_str());
        }
        fprintf(pCallerHeaderFile, "};");
    }
    substring_replace(CallerTplFile, "// @#{Non-blocking RPC with callback}@#", FunctionWithCallBack);
    substring_replace(CallerTplFile, "// #@{Custom struct convert method}@#", JsonExtern + "// #@{Custom struct convert method}@#\n");
    substring_replace(CallerTplFile, "// #@{FTRPC Caller Head File}@#", std::string("#include \"") + head_file_name + "\"");

    fprintf(pCallerSrcFile, "/*\n"
                            " * Auto generate by ftrpc\n"
                            " * Created by Rexfield on 2018/5/1\n"
                            " * Warning: Please do not modify any code unless you know what you are doing.\n"
                            " */"
                            "%s", CallerTplFile.c_str());
    fclose(pCallerSrcFile);
    CloseHeadToWrite(pCallerHeaderFile, head_file_name);
}

bool GenerateCPP(std::unique_ptr<RootNode> &document, TokenManage &tokenSystem, TypeManage &typeSystem)
{
    GenerateCPP_Provider(document, tokenSystem, typeSystem);
    GenerateCPP_Caller(document, tokenSystem, typeSystem);
    return true;
}