
set(PROJECT_SOURCES
    main.c
)

# Add the Clogger library
include_directories("${PROJECT_SOURCE_DIR}/lib/clogger/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/clogger/src")

add_executable(${PROJECT_NAME}
    main.c
)

# After you've built your own project, add it like you would a normal library
target_link_libraries(${PROJECT_NAME} clogger)
