cmake_minimum_required(VERSION 3.20)
project(rvm_firmware_contracts LANGUAGES C)

add_library(rvm_interface_contracts OBJECT
    interface_compile_test.c
)

target_compile_features(rvm_interface_contracts PRIVATE c_std_11)
target_include_directories(rvm_interface_contracts PRIVATE
    ../../Components/buffer_pool/include
    ../../Services/camera/include
    ../../Services/network/include
    ../../Services/system/include
)

if(MSVC)
    target_compile_options(rvm_interface_contracts PRIVATE /W4 /WX)
else()
    target_compile_options(rvm_interface_contracts PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()
