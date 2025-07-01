cmake_minimum_required(VERSION 3.10)
project(docscanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(docmain
    src/main.cpp
    src/lum_doc.cpp
    src/tool.cpp
)
target_compile_definitions(docmain PRIVATE
    NO_ANDROID
)
target_include_directories(docmain PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(docmain
    ${OpenCV_LIBS}
)

