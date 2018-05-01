#include <iostream>
#include "parser.h"
#include "GenCPP.h"

const char *src ="version=1;\n"
                  "module Test:\n"
                  "{\n"
                  "\tvoid request([out]string req);\n"
                  "\tstring requireNewStockInfo();"
                  "}\0";

int main() {
    parse parser(src);
    parser.work();
    GenerateCPP(parser.document, parser.lexer);
    return 0;
}