cmake_minimum_required(VERSION 3.0.0)
project(HeapManager VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()
add_executable(HeapManager main.c ../HeapManager.c )
include_directories(
${PROJECT_SOURCE_DIR}/../HeapManager



)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
