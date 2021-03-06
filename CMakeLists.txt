cmake_minimum_required(VERSION 3.5)
project(ftrpc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -std=c++17)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND @xcopy $(CMAKE_SOURCE_DIR)\\template .\\template /e/h/y/i
            COMMAND @copy $(CMAKE_SOURCE_DIR)\\License.txt .\\License.txt
            COMMAND @copy $(CMAKE_SOURCE_DIR)\\TypeDef.h .\\TypeDef.h
            COMMAND @copy $(CMAKE_SOURCE_DIR)\\keywords.h .\\keywords.h
            COMMAND @copy $(CMAKE_SOURCE_DIR)\\sample\\test.idl .\\test.idl
            )
    ADD_CUSTOM_COMMAND(OUTPUT auto_deploy
            COMMAND @copy .\\ftrpc.exe ..\\bin\\ftrpc.exe
            COMMAND @xcopy $(CMAKE_SOURCE_DIR)\\template ..\\bin\\template /e/h/y/i
            )
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    ADD_CUSTOM_COMMAND(OUTPUT copy_res
            COMMAND cp -a $(CMAKE_SOURCE_DIR)/template ./template
            COMMAND cp $(CMAKE_SOURCE_DIR)/License.txt ./License.txt
            COMMAND cp $(CMAKE_SOURCE_DIR)/TypeDef.h ./TypeDef.h
            COMMAND cp $(CMAKE_SOURCE_DIR)/keywords.h ./keywords.h
            COMMAND cp $(CMAKE_SOURCE_DIR)/sample/test.idl ./test.idl
            )
    ADD_CUSTOM_COMMAND(OUTPUT auto_deploy
            COMMAND copy ./ftrpc ../bin/ftrpc
            COMMAND cp -a $(CMAKE_SOURCE_DIR)/template ../bin/template
            )
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
add_executable(ftrpc main.cpp json_export.h
        lex.cpp lex.h keywords.h parser.cpp parser.h ast_tree.h TypeDef.h symman.cpp symman.h
        jsoncpp.cpp
        Gen/GenCPP.cpp Gen/GenCPP.h Gen/GenUtils.cpp Gen/GenUtils.h Gen/GenTS.cpp Gen/GenTS.h
        copy_res)
add_custom_target(AutoDeploy ALL DEPENDS ftrpc auto_deploy)
