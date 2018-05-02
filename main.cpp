#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "parser.h"
#include "GenCPP.h"

const char *src ="version=1;\n"
                  "module Test:\n"
                  "{\n"
                  "\tvoid request([out]string req);\n"
                  "\tstring requireNewStockInfo();"
                  "}\0";

bool cppEnable = true, pyEnable = false, jsEnable = false;
int main(int argc, char **argv) {
    int opt;
    int option_index = 0;
    const char *optstring = "ho:";
    struct option long_options[] = {
            {"output", required_argument, NULL, 'o'},
            {"help",  no_argument,       NULL, 'h'},
            {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o': {
                cppEnable = pyEnable = jsEnable = false;
                const char *split = ",";
                char *p = strtok(optarg, split);
                while(p != nullptr) {
                    if (strncmp(p, "c++", strlen("c++") + 1) == 0) {
                        cppEnable = true;
                    } else if (strncmp(p, "python", strlen("python") + 1) == 0) {
                        pyEnable = true;
                    } else if (strncmp(p, "js", strlen("js") + 1) == 0) {
                        jsEnable = true;
                    } else {
                        fprintf(stderr, "Unknow generate type - %s\n", p);
                    }
                    p = strtok(nullptr, split);
                }
                break;
            }
            case 'h':
            default:
                fprintf(stdout, "\nUsage: ftrpc [-help|-h] [-output|-o [c++,python,js]]\n\n"
                                "--help|-h            Show this message.\n"
                                "--output|-o          Who will choose to generate.\n"
                                "                     Default is c++.\n"
                                "\nCreated by Rexfield.\n"
                                "Allow with GPLv3.\n"
                                "BUG report: https://gitee.com/RonxBulld/ftrpc/issues\n"
                                "\n");
                return -1;
        }
    }
    fprintf(stdout, "Will generate: ");
    if (cppEnable) fprintf(stdout, "c++ ");
    if (pyEnable) fprintf(stdout, "python ");
    if (jsEnable) fprintf(stdout, "javascript ");
    fprintf(stdout, "\n");
    parse parser(src);
    parser.work();
    if (cppEnable)
        GenerateCPP(parser.document, parser.lexer);

    return 0;
}