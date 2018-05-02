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
    const char *mdef = FilenameToMacro(fn).c_str();
    fprintf(fp, "#ifndef %s\n"
                "#define %s\n"
                "\n", mdef, mdef);
    return fp;
}

void CloseHeadToWrite(FILE *fp, const char *fn)
{
    const char *mdef = FilenameToMacro(fn).c_str();
    fprintf(fp, "\n"
                "\n#endif // %s\n", mdef);
    // End generate
    fclose(fp);
}

bool GenerateCPP_Provider(struct RootNode &document, class lex *lexer)
{
    const char *head_file_name = "ftrpc.provider.h";
    const char *src_file_name = "ftrpc.provider.cpp";

    FILE *pProviderHeaderFile = OpenHeadToWrite(head_file_name);
    fprintf(pProviderHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document.version);
    fprintf(pProviderHeaderFile, "\n#include <string>\n\n"
                                 "std::string ProviderDoCall(const std::string &JSON);\n\n");
    FILE *pProviderSrcFile = fopen(src_file_name, "w+");
    std::string ProviderTplFile = ReadFileAsTxt(PROVIDER_TPL_FILE);
    std::string FunctionMicro, FunctionSignature, FunctionCheckAndCall;
    // Module
    for(auto module : document.modules) {
        std::string CurModuleName = lexer->GetString(module->name);
        fprintf(pProviderHeaderFile, "class %s\n"
                                     "{\n"
                                     "public:\n", CurModuleName.c_str());
        // Api
        int apiIndex = 0;
        for(auto api : module->apis) {
            std::string ApiName = lexer->GetString(api->name);
            std::string FullApiName;
            FullApiName.append(CurModuleName).append("::").append(ApiName);

            fprintf(pProviderHeaderFile, "\tstatic %s %s(", GetCppType(api->retType.type).c_str(), ApiName.c_str());

            FunctionMicro.append("#define FUNCTION_").append(CurModuleName).append("_").append(ApiName).append(" ").append(std::to_string(apiIndex++)).append(1,'\n');
            FunctionSignature.append("\t{ FUNCTION_").append(CurModuleName).append("_").append(ApiName).append(", std::string({");
            FunctionCheckAndCall.append("\t\t\tcase HashStringToInt(\"").append(FullApiName).append("\"): {\n\t\t\t\t");
            FunctionCheckAndCall.append("CHECK_PARAM_COUNT(6);\n""\t\t\t\tCHECK_PARAM_SIGNATURE(").append(CurModuleName).append(", ").append(ApiName).append(");\n\t\t\t\tJson::Value param = root[\"params\"];\n\t\t\t\t");
            if(api->retType.type != TY_void) {
                FunctionCheckAndCall.append("ret[\"return\"] = ");
            }
            FunctionCheckAndCall.append(FullApiName).append("(");
            // Params
            int paramIndex = 0;
            for(auto param : api->params) {
                fprintf(pProviderHeaderFile, "%s %s,", GetCppType(param->type.type).c_str(), lexer->GetString(param->name).c_str());

                FunctionSignature.append("ti2c[").append(GetEnumName(param->type.type)).append("], ");
                FunctionCheckAndCall.append("param[").append(std::to_string(paramIndex++)).append("].").append(GetJsonAsMethod(param->type.type)).append("(), ");
            }
            if(!api->params.empty()) {
                fseek(pProviderHeaderFile, -1, SEEK_CUR);
                FunctionSignature.erase(FunctionSignature.end() - 2, FunctionSignature.end());
                FunctionCheckAndCall.erase(FunctionCheckAndCall.end() - 2, FunctionCheckAndCall.end());
            }
            fprintf(pProviderHeaderFile, ");\n");
            FunctionSignature.append("}) },\n");
            FunctionCheckAndCall.append(");\n\t\t\t\tbreak;\n\t\t\t}\n");
        }
        fprintf(pProviderHeaderFile, "};");
    }
    substring_replace(ProviderTplFile, "// #@{FUNCTION_XXX micro}@#", FunctionMicro);
    substring_replace(ProviderTplFile, "// #@{Function Signature}@#", FunctionSignature);
    substring_replace(ProviderTplFile, "// #@{Function Check and Call}@#", FunctionCheckAndCall);

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

bool GenerateCPP_Caller(struct RootNode &document, class lex *lexer)
{
    const char *head_file_name = "ftrpc.caller.h";
    const char *src_file_name = "ftrpc.caller.cpp";

    FILE *pCallerHeaderFile = OpenHeadToWrite(head_file_name);
    fprintf(pCallerHeaderFile, "#define FTRPC_VERSION_MAJOR %d\n\n", document.version);
    fprintf(pCallerHeaderFile, "\n#include <string>\n\n");

    FILE *pCallerSrcFile = fopen(src_file_name, "w+");
    std::string CallerTplFile = ReadFileAsTxt(CALLER_TPL_FILE);

    std::string FunctionWithCallBack;

    // Module
    for(auto module : document.modules) {
        std::string CurModuleName = lexer->GetString(module->name);
        fprintf(pCallerHeaderFile, "class %s\n"
                                   "{\n"
                                   "public:\n", CurModuleName.c_str());
        // Api
        int apiIndex = 0;
        for(auto api : module->apis) {
            std::string ApiName = lexer->GetString(api->name);
            fprintf(pCallerHeaderFile, "\tstatic std::string %s(", ApiName.c_str());
            std::string FullApiName, FunctionParams;
            FullApiName.append(CurModuleName).append("::").append(ApiName);

            FunctionWithCallBack.append("std::string ").append(FullApiName).append("(");
            // Params
            int paramIndex = 0;
            for(auto param : api->params) {
                std::string paramName = lexer->GetString(param->name);
                fprintf(pCallerHeaderFile, "%s %s, ", GetCppType(param->type.type).c_str(), paramName.c_str());
                FunctionWithCallBack.append(GetCppType(param->type.type)).append(" ").append(paramName).append(", ");
                FunctionParams.append("\tparams[").append(std::to_string(paramIndex)).append("] = ").append(paramName).append(";\n");
                paramIndex++;
            }
            FunctionWithCallBack.append("void(*_callback)(");
            if (api->retType.type == TY_void) {
                FunctionWithCallBack.append(GetCppType(api->retType.type));
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
            fprintf(pCallerHeaderFile, "void(*_callback)(%s));\n", GetCppType(api->retType.type).c_str());
        }
        fprintf(pCallerHeaderFile, "};");
    }
    substring_replace(CallerTplFile, "// @#{Non-blocking RPC with callback}@#", FunctionWithCallBack);

    fprintf(pCallerSrcFile, "/*\n"
                            " * Auto generate by ftrpc\n"
                            " * Created by Rexfield on 2018/5/1\n"
                            " * Warning: Please do not modify any code unless you know what you are doing.\n"
                            " */"
                            "%s", CallerTplFile.c_str());
    fclose(pCallerSrcFile);
    CloseHeadToWrite(pCallerHeaderFile, head_file_name);
}

bool GenerateCPP(struct RootNode &document, class lex *lexer)
{
    GenerateCPP_Provider(document, lexer);
    GenerateCPP_Caller(document, lexer);
    return true;
}