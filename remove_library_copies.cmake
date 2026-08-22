file(GLOB_RECURSE LIBRARY_COPY_FILES
  "${CMAKE_BINARY_DIR}/zerox_game*.dll"
  "${CMAKE_BINARY_DIR}/libzerox_game*.so"
)

if(LIBRARY_COPY_FILES)
  file(REMOVE ${LIBRARY_COPY_FILES})
endif()
