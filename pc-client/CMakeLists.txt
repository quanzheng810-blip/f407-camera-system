cmake_minimum_required(VERSION 3.21)

project(RvmSerialClient VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets SerialPort)

qt_standard_project_setup()

qt_add_executable(rvm-serial-client WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/framestreamparser.cpp
    src/framestreamparser.h
    src/serialportcontroller.cpp
    src/serialportcontroller.h
)

target_link_libraries(rvm-serial-client PRIVATE
    Qt6::Widgets
    Qt6::SerialPort
)

if(MSVC)
    target_compile_options(rvm-serial-client PRIVATE /utf-8)
endif()

set_target_properties(rvm-serial-client PROPERTIES
    OUTPUT_NAME "RvmSerialClient"
    WIN32_EXECUTABLE TRUE
)

install(TARGETS rvm-serial-client
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

qt_generate_deploy_script(
    TARGET rvm-serial-client
    OUTPUT_SCRIPT deploy_script
    CONTENT "
qt6_deploy_runtime_dependencies(
    EXECUTABLE \"$<TARGET_FILE:rvm-serial-client>\"
    GENERATE_QT_CONF
)
"
)
install(SCRIPT ${deploy_script})

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(frame-stream-parser-test
        tests/framestreamparser_test.cpp
        src/framestreamparser.cpp
        src/framestreamparser.h
    )
    target_include_directories(frame-stream-parser-test PRIVATE src)
    target_link_libraries(frame-stream-parser-test PRIVATE
        Qt6::Gui
        Qt6::Test
    )
    if(MSVC)
        target_compile_options(frame-stream-parser-test PRIVATE /utf-8)
    endif()
    add_test(NAME frame-stream-parser-test COMMAND frame-stream-parser-test)
endif()
