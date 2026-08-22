cmake_minimum_required(VERSION 4.3.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

set(PROJECT_NAME zerox_game)
set(INCLUDE_DIR include)
set(SRC_DIR src)
set(MEMORY_READER_DIR memory_reader)

Project(${PROJECT_NAME})

list(APPEND SOURCES
    ${SRC_DIR}/zerox_game.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

find_package(glm 1.0 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    "${MEMORY_READER_DIR}/${INCLUDE_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE_NAME:${PROJECT_NAME}> "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_dummy.pdb"
        VERBATIM
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DTARGET_NAME=${PROJECT_NAME}
        -DTARGET_PATH=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/remove_library_copies.cmake
    COMMENT "Removing existing library copies..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
