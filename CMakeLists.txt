set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -fno-rtti")

add_llvm_loadable_module( LLVMAAM
    AAM.cpp
    ConcreteAAM.cpp
    AbstractAAM.cpp
    Utils.cpp

    PLUGIN_TOOL
    opt
    )