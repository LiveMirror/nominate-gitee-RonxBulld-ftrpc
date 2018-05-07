#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include "parser.h"
#include "Gen/GenCPP.h"
#include "Gen/GenTS.h"
#include "Gen/GenUtils.h"

bool cppEnable = false, pyEnable = false, jsEnable = false, tsEnable = false;
bool hadVersionInfo = true;

void help() {
    fprintf(stdout, "\nUsage: ftrpc [--help|-h] [--no-version|-n] [--output|-o [c++,python,js]] <IDL File>\n\n"
                    "--help|-h            Show this message.\n"
                    "--output|-o          Who will choose to generate.\n"
                    "--no-version-n       No version infomation in output.\n"
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
            {"output",     required_argument, nullptr, 'o'},
            {"help",       no_argument,       nullptr, 'h'},
            {"no-version", no_argument,       nullptr, 'n'},
            {nullptr,      0,                 nullptr, 0}
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
    std::string Su = ReadFileAsTxt(std::string(argv[argc - 1]));
    parse parser(Su.c_str());
    parser.work();
    if (cppEnable)
        GenerateCPP(parser.document, parser.tokenManage, parser.typeManage);
    if (tsEnable)
        GenerateTypeScript(parser.document, parser.tokenManage, parser.typeManage);
    return 0;
}