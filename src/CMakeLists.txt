# --------------------------------------------------------------
# Find the tools (Bison & Flex). CMake ships with FindBISON/FindFLEX modules.
find_package(BISON REQUIRED)
find_package(FLEX  REQUIRED)

# -------------------------------------------------------------
# Project sources
set(PROJECT_SOURCES
    main.c
    Expression.c
)

# -------------------------------------------------------------
# Paths / filenames
set(BISON_INPUT   ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)
set(FLEX_INPUT    ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l)

set(BISON_OUTPUT_C ${CMAKE_CURRENT_BINARY_DIR}/Parser.c)
set(BISON_OUTPUT_H ${CMAKE_CURRENT_BINARY_DIR}/Parser.h)

set(FLEX_OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/Lexer.c)


# --------------------------------------------------------------
# Bison command
add_custom_command(
   OUTPUT  ${BISON_OUTPUT_C} ${BISON_OUTPUT_H}
    COMMAND ${BISON_EXECUTABLE} -d -o ${BISON_OUTPUT_C} ${BISON_INPUT}
    DEPENDS ${BISON_INPUT}
    COMMENT "Generating Bison parser"
    VERBATIM
)

# Flex command (depends on the header produced by Bison)
add_custom_command(
    OUTPUT  ${FLEX_OUTPUT}
    COMMAND ${FLEX_EXECUTABLE} -o${FLEX_OUTPUT} ${FLEX_INPUT}
    DEPENDS ${FLEX_INPUT} ${BISON_OUTPUT_H}
    COMMENT "Generating Flex lexer"
    VERBATIM
)

# --------------------------------------------------------------
# Object library that compiles the generated sources
add_library(gpcparser OBJECT
    ${BISON_OUTPUT_C}
    ${FLEX_OUTPUT}
    # Add any hand‑written .c files that belong to the parser here:
    # ast.c codegen.c vm.c
)

target_include_directories(gpcparser PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}   # for Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}   # for your own headers
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}   # for Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}   # for your own headers
    ${PROJECT_SOURCE_DIR}/lib/clogger/include
    ${PROJECT_SOURCE_DIR}/lib/clogger/src)

# After you've built your own project, add it like you would a normal library
target_link_libraries(${PROJECT_NAME}
    PUBLIC clogger gpcparser)

# --------------------------------------------------------------
# Optional: make the generated header visible to IDEs (VSCode, CLion, …)
source_group("Generated Files" FILES
    ${BISON_OUTPUT_C}
    ${BISON_OUTPUT_H}
    ${FLEX_OUTPUT}
)