cmake_minimum_required(VERSION 3.5)

project(otoolrecursive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(otoolrecursive main.cpp)

include(GNUInstallDirs)
install(TARGETS otoolrecursive
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
