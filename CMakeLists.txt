cmake_minimum_required(VERSION 3.15)
project(Lab3)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Build settings
set(CMAKE_CXX_STANDARD 20)

# Building ---
add_subdirectory(src)

add_custom_target(build_finished ALL
    COMMENT "Build almost finished...")

add_custom_command(TARGET build_finished POST_BUILD
    COMMAND cp ${PROJECT_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}/
)
message("Compile commands exported...")
