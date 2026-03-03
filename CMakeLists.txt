cmake_minimum_required(VERSION 4.0)
project(GeneralFramework)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)

include_directories(submodules/narcissus/include)
include_directories(submodules/narcissus/submodule/field-reflection/include)
include_directories(submodules/narcissus/submodule/magic_enum/include)

add_executable(GeneralFramework main.cpp
        src/jackbergus/framework/Controller.cpp
        include/jackbergus/framework/Controller.h
        include/jackbergus/framework/structures/ControllerRequest.h
        src/jackbergus/framework/Model.cpp
        include/jackbergus/framework/Model.h
        include/jackbergus/framework/structures/ModelStatus.h
        src/jackbergus/framework/View.cpp
        include/jackbergus/framework/View.h
        src/jackbergus/framework/FileSerializer.cpp
        include/jackbergus/framework/FileSerializer.h
        include/jackbergus/framework/monitor/ContinuousVariableMonitoring.h
        include/jackbergus/framework/types/NativeTypes.h
        include/jackbergus/framework/monitor/ContinuousMonitoring.h
        submodules/narcissus/src/narcissus/lightweight_any.cpp)
