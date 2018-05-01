cmake_minimum_required(VERSION 3.5)
project(ftrpc)

set(CMAKE_CXX_STANDARD 14)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND copy ..\\*.tpl .\\*.tpl
            )
    add_custom_target(CopyTask ALL DEPENDS copy_res)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND copy ../*.tpl ./*.tpl
            )
    add_custom_target(CopyTask ALL DEPENDS copy_res)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

add_executable(ftrpc main.cpp lex.cpp lex.h keywords.h parser.cpp parser.h ast_tree.h GenCPP.cpp GenCPP.h jsoncpp.cpp TypeDef.h copy_res)