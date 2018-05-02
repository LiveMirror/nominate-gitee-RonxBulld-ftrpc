cmake_minimum_required(VERSION 3.5)
project(ftrpc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -std=c++17)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND xcopy $(CMAKE_SOURCE_DIR)\\template .\\template /e/h/y/i
            COMMAND copy $(CMAKE_SOURCE_DIR)\\License.txt .\\License.txt
            COMMAND copy $(CMAKE_SOURCE_DIR)\\TypeDef.h .\\TypeDef.h
            COMMAND copy $(CMAKE_SOURCE_DIR)\\keywords.h .\\keywords.h
            )
    add_custom_target(CopyTask ALL DEPENDS copy_res)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND cp -a $(CMAKE_SOURCE_DIR)/template ./template
            COMMAND cp $(CMAKE_SOURCE_DIR)/License.txt ./License.txt
            COMMAND cp $(CMAKE_SOURCE_DIR)/TypeDef.h ./TypeDef.h
            COMMAND cp $(CMAKE_SOURCE_DIR)/keywords.h ./keywords.h
            COMMAND sed -i '/\/\* SED REMOVE/,/!SED REMOVE\*\//d' ./template/typescript/caller.tpl.ts
            )
    add_custom_target(CopyTask ALL DEPENDS copy_res)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

add_executable(ftrpc main.cpp lex.cpp lex.h keywords.h parser.cpp parser.h ast_tree.h GenCPP.cpp GenCPP.h jsoncpp.cpp TypeDef.h copy_res GenUtils.cpp GenUtils.h GenTS.cpp GenTS.h)