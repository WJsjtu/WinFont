cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME WinFont)
project(${PROJECT_NAME} C CXX ASM)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES RELWITHDEBINFO) 
elseif(CMAKE_BUILD_TYPE MATCHES RELEASE OR CMAKE_BUILD_TYPE MATCHES MINSIZEREL)
else()
set(CMAKE_BUILD_TYPE "MINSIZEREL")
endif()

include(tools.cmake)

file (
    GLOB_RECURSE font_src
    LIST_DIRECTORIES false
    "${PROJECT_SOURCE_DIR}/Source/*.cpp"
    "${PROJECT_SOURCE_DIR}/Source/*.h"
    )

set(LIBRARY_NAME "font")

if(BUILD_SHARED_LIBS)
  add_library(${LIBRARY_NAME} SHARED ${font_src})
else()
  add_library(${LIBRARY_NAME} STATIC ${font_src})
  target_compile_definitions(${LIBRARY_NAME} PUBLIC FONT_STATIC)
endif()

add_subdirectory(ThirdParty)

target_link_libraries(${LIBRARY_NAME} ${ZLIB_STATIC_LIBRARY} ${LIBPNG_LIBRARY} ${BZIP2_LIBRARY} ${BROTLI_LIBRARIES} ${FREETYPE_LIBRARY} gdi32.lib Usp10.lib)

target_include_directories(${LIBRARY_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/Source")
target_include_directories(${LIBRARY_NAME} PRIVATE "${FREETYPE_DIR}/include/freetype2")
target_compile_definitions(${LIBRARY_NAME} PRIVATE FONT_DLL)

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

install(TARGETS ${LIBRARY_NAME} ${LIBRARY_NAME}
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}")

set(LIBRARY_PUBLIC_HDRS 
  ${PROJECT_SOURCE_DIR}/Source/Font.h
)

install(FILES ${LIBRARY_PUBLIC_HDRS} DESTINATION "${INSTALL_INC_DIR}")

if(BUILD_SHARED_LIBS)
  install(FILES $<TARGET_PDB_FILE:${LIBRARY_NAME}> DESTINATION bin OPTIONAL)
else()
  get_target_property(linked_libs ${LIBRARY_NAME} LINK_LIBRARIES)
  install(FILES ${linked_libs} DESTINATION lib)
endif()

# ============ Test ==============
file (
  GLOB_RECURSE main_src
  LIST_DIRECTORIES false
  "${PROJECT_SOURCE_DIR}/Test/*.cpp"
  "${PROJECT_SOURCE_DIR}/Test/*.h"
)

add_executable(resource_include_exe "${PROJECT_SOURCE_DIR}/resource.cpp")
function(resource_include out_file input_file var_name)
  add_custom_command(
    OUTPUT "./${var_name}.c"
    COMMAND resource_include_exe "${var_name}" "${input_file}"
    DEPENDS "${input_file}"
    COMMENT "Building binary file for embedding ${input_file}")
  add_custom_target("${var_name}_resource" DEPENDS "${CMAKE_BINARY_DIR}/${var_name}.c")
  set(${out_file}
    "${CMAKE_BINARY_DIR}/${var_name}.c"
    PARENT_SCOPE)
endfunction(resource_include)
resource_include(TEST_FONT "${PROJECT_SOURCE_DIR}/Fonts/pacifico/Pacifico.ttf" "PacificoTTF")

add_executable(${PROJECT_NAME} ${main_src} ${TEST_FONT})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Source")
install_glfw(${PROJECT_NAME})

if(BUILD_SHARED_LIBS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${LIBRARY_NAME}>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARY_NAME})

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
LIBRARY DESTINATION "${INSTALL_LIB_DIR}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_targets_folder()

