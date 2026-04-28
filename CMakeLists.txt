cmake_minimum_required(VERSION 4.0)
project(GeneralFramework)

set(CMAKE_CXX_STANDARD 17)

#IF(CMAKE_BUILD_TYPE MATCHES Debug)
#    message(WARNING "Compiling in debug mode")
##    add_compile_definitions(DEBUG)
#    set(CMAKE_CXX_FLAGS_DEBUG "-g")
#else()
#    message(WARNING "Compiling in optimized mode. Level=3")
#    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(include)
include_directories(submodules/fkYAML/include)
include_directories(submodules/refl-cpp/include)
include_directories(submodules/magic_enum/include)
include_directories(submodules/narcissus/include)
include_directories(submodules/vxworks_linux/include)
#include_directories(submodules/narcissus/submodule/field-reflection/include)
#include_directories(submodules/narcissus/submodule/magic_enum/include)

add_subdirectory(submodules/vxworks_linux/)

add_library(GeneralFramework STATIC examples/example_record_serialization.cpp
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
        include/jackbergus/framework/ndp/FileBlockReader.h
        include/jackbergus/framework/monitor/AnyStructMonitoring.h
        include/jackbergus/concurrency/RWLock.h
        include/jackbergus/framework/monitor/VariableMonitor.h
        include/jackbergus/framework/monitor/AbstractVariableMonitor.h
        src/jackbergus/concurrency/ThreadPool.cpp
        include/jackbergus/concurrency/ThreadPool.h
        src/jackbergus/concurrency/HoareMonitor.cpp
        include/jackbergus/concurrency/HoareMonitor.h
        src/jackbergus/data/FltForTemplateParam.cpp
        src/jackbergus/monitor/deserializer/FieldGetting.cpp
        src/jackbergus/monitor/deserializer/RecordFileDeserializer.cpp
        src/jackbergus/monitor/serializer/binary_to_csv_serializer.cpp
)


add_executable(main examples/example_record_serialization.cpp)
target_link_libraries(main GeneralFramework)

add_executable(simple_double_example examples/simple_double_example.cpp)
target_link_libraries(simple_double_example GeneralFramework)

add_executable(example_for_record_updates examples/example_for_record_updates.cpp
        include/jackbergus/data/UpdatableElement.h)
target_link_libraries(example_for_record_updates GeneralFramework)

add_executable(example_record_serialization examples/example_record_serialization.cpp)
target_link_libraries(example_record_serialization GeneralFramework)

add_executable(signal_example examples/signal_example.cpp)

add_executable(test_triple_buffer examples/test_triple_buffer.cpp)

add_executable(test_thread_pool_with_scheduling examples/test_threadpoolsched.cpp)
target_link_libraries(test_thread_pool_with_scheduling  vxworks cpp_vxworks)
