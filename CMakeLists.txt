cmake_minimum_required(VERSION 3.0.0)
project(StructuralEditor VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)

include(CTest)
enable_testing()

add_executable(StructuralEditor Main.c Implementations.c Font.c Lexer.c Parser.c Writer.c Saver.c Block.c Math.c Color.c Cursor.c Input.c Shapes.c Camera.c SearchBar.c Theme.c)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_compile_options(StructuralEditor PRIVATE /W4 /WX)
endif()

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw-3.3.8 ${CMAKE_CURRENT_BINARY_DIR}/glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

set(FT_DISABLE_HARFBUZZ TRUE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/freetype ${CMAKE_CURRENT_BINARY_DIR}/freetype)
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW_SOURCE_DIR}/include
    ${GLFW_SOURCE_DIR}/dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sokol
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sokol_gp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/fontstash
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
