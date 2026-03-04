cmake_minimum_required(VERSION 4.0)
project(GeneralFramework)

set(CMAKE_CXX_STANDARD 20)

#IF(CMAKE_BUILD_TYPE MATCHES Debug)
#    message(WARNING "Compiling in debug mode")
##    add_compile_definitions(DEBUG)
#    set(CMAKE_CXX_FLAGS_DEBUG "-g")
#else()
#    message(WARNING "Compiling in optimized mode. Level=3")
#    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(include)

include_directories(submodules/narcissus/include)
include_directories(submodules/narcissus/submodule/field-reflection/include)
include_directories(submodules/narcissus/submodule/magic_enum/include)


add_library(GeneralFramework STATIC main.cpp
        src/jackbergus/framework/Controller.cpp
        include/jackbergus/framework/Controller.h
        include/jackbergus/framework/structures/ControllerRequest.h
        src/jackbergus/framework/Model.cpp
        include/jackbergus/framework/Model.h
        include/jackbergus/framework/structures/ModelStatus.h
        src/jackbergus/framework/View.cpp
        include/jackbergus/framework/View.h
        src/jackbergus/framework/ndp/FileSerializer.cpp
        include/jackbergus/framework/ndp/FileSerializer.h
        include/jackbergus/framework/monitor/ContinuousVariableMonitoring.h
        include/jackbergus/framework/types/NativeTypes.h
        include/jackbergus/framework/monitor/ContinuousMonitoring.h
        submodules/narcissus/src/narcissus/lightweight_any.cpp
        src/jackbergus/framework/monitor/NativeTypeMonitoring.cpp
        include/jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h
        include/jackbergus/framework/ndp/FileBlockWrapper.h
        include/jackbergus/framework/ndp/FileBlockReader.h)


add_executable(main main.cpp)
target_link_libraries(main GeneralFramework)

add_executable(simple_double_example examples/simple_double_example.cpp)
target_link_libraries(simple_double_example GeneralFramework)