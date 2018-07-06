#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "parser.h"
#include "Gen/GenCPP.h"
#include "Gen/GenTS.h"
#include "Gen/GenUtils.h"
#include "json_export.h"

#define WRITE_JSON_FILE(file, data) do{ FILE *fp = fopen(file, "wb+"); \
                                       fwrite(data, data##_len, 1, fp); \
                                       fclose(fp); } while(0)

bool cppEnable = false, pyEnable = false, jsEnable = false, tsEnable = false, builtJson = false;
bool hadVersionInfo = true;

void help() {
    fprintf(stdout, "\nUsage: ftrpc [--help|-h] [--no-version|-n] [--output|-o [c++,python,js,ts]] <IDL File>\n\n"
                    "--help|-h            Show this message.\n"
                    "--output|-o          Who will choose to generate.\n"
                    "--no-version-n|-n    No version infomation in output.\n"
                    "--builtin-json|-j    Use built-in json module with C++.\n"
                    "\nCreated by Rexfield.\n"
                    "Allow with GPLv3.\n"
                    "BUG report: https://gitee.com/RonxBulld/ftrpc/issues\n"
                    "\n");
}

int main(int argc, char **argv) {
    if (argc == 1) {
        help();
        return -1;
    }
    int opt;
    int option_index = 0;
    const char *optstring = "ho:";
    struct option long_options[] = {
            {"output",       required_argument, nullptr, 'o'},
            {"help",         no_argument,       nullptr, 'h'},
            {"no-version",   no_argument,       nullptr, 'n'},
            {"builtin-json", no_argument,       nullptr, 'j'},
            {nullptr,        0,                 nullptr, 0}
    };
    while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1 || argc == 1) {
        switch (opt) {
            case 'o': {
                const char *split = ",";
                char *p = strtok(optarg, split);
                while(p != nullptr) {
                    if (strncmp(p, "c++", strlen("c++") + 1) == 0) {
                        cppEnable = true;
                    } else if (strncmp(p, "python", strlen("python") + 1) == 0) {
                        pyEnable = true;
                    } else if (strncmp(p, "ts", strlen("js") + 1) == 0) {
                        tsEnable = true;
                    } else if (strncmp(p, "js", strlen("js") + 1) == 0) {
                        jsEnable = true;
                    } else {
                        fprintf(stderr, "Unknow generate type - %s\n", p);
                    }
                    p = strtok(nullptr, split);
                }
                break;
            }
            case 'n': {
                hadVersionInfo = false;
                break;
            }
            case 'j': {
                builtJson = true;
                break;
            }
            case 'h':
            default:
                help();
                return -1;
        }
    }
    fprintf(stdout, "Will generate: ");
    if (cppEnable) fprintf(stdout, "c++ ");
    if (pyEnable) fprintf(stdout, "python ");
    if (jsEnable) fprintf(stdout, "javascript ");
    if (tsEnable) fprintf(stdout, "typescript ");
    fprintf(stdout, "\n");
    std::string Su = ReadFileAsTxt(argv[argc - 1]);
    parse parser(Su.c_str());
    if (!parser.work()) {
        return -1;
    }
    for (auto &module : parser.document->modules) {
        std::string module_name = parser.tokenManage[module.name];
        for (auto &structure : module.structs) {
            TokenID token = parser.typeManage.ty2tk[structure.type];
            std::string name = parser.tokenManage[token];
            RegistType(structure.type, "is"+name+"Struct", "as"+name+"Struct", "struct "+module_name+"::"+name, name);
        }
    }
//    for (auto structure : parser.typeManage.StructsMap) {
//        TokenID token = parser.typeManage.ty2tk[structure.first];
//        std::string name = parser.tokenManage[token];
//        RegistType((enum Type)structure.first, "is"+name+"Struct", "as"+name+"Struct", "struct "+name, name);
//    }
    char filename[260];
    char *nptr = argv[argc - 1];
    char *lptr = strrchr(nptr, '\\');
    if (lptr == nullptr) { lptr = nptr; } else { lptr++; }
    char *rptr = strrchr(nptr, '.');
    if (rptr == nullptr) { rptr = nptr + strlen(nptr); }
    strncpy(filename, lptr, rptr - lptr);
    filename[rptr-lptr] = '\0';
    if (cppEnable) {
        GenerateCPP(parser.document, parser.tokenManage, parser.typeManage, filename);
        if (builtJson) {
            WRITE_JSON_FILE("json.cpp", jsoncpp_cpp);
#ifndef WIN32
            mkdir("json", 755);
#else
            mkdir("json");
#endif
            WRITE_JSON_FILE("json/json.h", json_json_h);
            WRITE_JSON_FILE("json/json-forwards.h", json_json_forwards_h);
        }
    }
    if (tsEnable) {
        GenerateTypeScript(parser.document, parser.tokenManage, parser.typeManage, filename);
    }
    return 0;
}
