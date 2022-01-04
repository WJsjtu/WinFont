find_package(Git QUIET)

if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git is not available.")
endif()

function(git_clone Library Url Branch)
  set(LibraryDir "${CMAKE_SOURCE_DIR}/${Library}")
  get_filename_component(LibraryParentDir "${CMAKE_SOURCE_DIR}/${Library}/.." ABSOLUTE)
  file(RELATIVE_PATH Library "${LibraryParentDir}" "${CMAKE_SOURCE_DIR}/${Library}")
  if(NOT EXISTS "${LibraryDir}/.git")
    file(MAKE_DIRECTORY "${LibraryParentDir}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} clone ${Url} ${Library} ENCODING UTF8
      WORKING_DIRECTORY "${LibraryParentDir}"
      RESULT_VARIABLE GIT_CLONE_RESULT)
    if(NOT GIT_CLONE_RESULT EQUAL "0")
      message(FATAL_ERROR "git clone failed with ${GIT_CLONE_RESULT}, please fix issue.")
    endif()
    message(STATUS "Git checkout ${Library}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} checkout ${Branch} ENCODING UTF8
      WORKING_DIRECTORY "${LibraryDir}"
      RESULT_VARIABLE GIT_CHECKOUT_RESULT)
    if(NOT GIT_CHECKOUT_RESULT EQUAL "0")
      message(FATAL_ERROR "git checkout failed with ${GIT_CHECKOUT_RESULT}, please fix issue.")
    endif()
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ENCODING UTF8
      WORKING_DIRECTORY "${LibraryDir}"
      RESULT_VARIABLE GIT_SUBMODULE_RESULT)
    if(NOT GIT_SUBMODULE_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule failed with ${GIT_SUBMODULE_RESULT}, please fix issue.")
    endif()
  else()
    execute_process(COMMAND ${GIT_EXECUTABLE} reset --hard WORKING_DIRECTORY "${LibraryDir}" ENCODING UTF8)
    execute_process(COMMAND ${GIT_EXECUTABLE} fetch origin WORKING_DIRECTORY "${LibraryDir}" ENCODING UTF8)
    message(STATUS "Git checkout ${Library}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} checkout ${Branch} ENCODING UTF8
      WORKING_DIRECTORY "${LibraryDir}"
      RESULT_VARIABLE GIT_CHECKOUT_RESULT)
    if(NOT GIT_CHECKOUT_RESULT EQUAL "0")
      message(FATAL_ERROR "git checkout failed with ${GIT_CHECKOUT_RESULT}, please fix issue.")
    endif()
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ENCODING UTF8
      WORKING_DIRECTORY "${LibraryDir}"
      RESULT_VARIABLE GIT_SUBMODULE_RESULT)
    if(NOT GIT_SUBMODULE_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule failed with ${GIT_SUBMODULE_RESULT}, please fix issue.")
    endif()
  endif()
  message(INFO "git clone ${Library} ${Url} ${Branch} done.")
endfunction(git_clone)

function(set_targets_folder)
  set_target_folder_recursive(${PROJECT_SOURCE_DIR})
endfunction()
macro(set_target_folder_recursive dir)
  get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
  foreach(subdir ${subdirectories})
    set_target_folder_recursive(${subdir})
  endforeach()
  get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
  if(current_targets)
    list(APPEND ${targets} ${current_targets})
    foreach(current_target IN LISTS current_targets)
      file(RELATIVE_PATH current_target_relative ${PROJECT_SOURCE_DIR} ${dir})
      get_target_property(current_files ${current_target} SOURCES)
      if(NOT("${current_files}" STREQUAL "current_files-NOTFOUND"))
        foreach(current_file IN LISTS current_files)
          if (IS_ABSOLUTE ${current_file})
            set(current_file ${current_file})
          else()
            set(current_file ${dir}/${current_file})
          endif()
          set(path "${dir}")
          cmake_path(IS_PREFIX path "${current_file}" NORMALIZE is_prefix)
          if(is_prefix)
            file(RELATIVE_PATH current_file_relative ${dir} ${current_file})
            get_filename_component(current_file_source_path "${current_file_relative}" PATH)
            if (WIN32)
              string(REPLACE "/" "\\" current_file_source_path_replaced "${current_file_source_path}")
            else()
              set(current_file_source_path_replaced "${current_file_source_path}")
            endif()
            source_group("${current_file_source_path_replaced}" FILES ${current_file})
          endif()
        endforeach()
      endif()
      if (NOT("${current_target_relative}" STREQUAL ""))
        set_target_properties(${current_target} PROPERTIES FOLDER ${current_target_relative})
      endif()
    endforeach()
  endif()
endmacro()

function(ExecVSCmd pwd)
  if (NOT VSCmdPath)
    get_filename_component(LOCAL_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    while (NOT "${LOCAL_COMPILER_DIR}" MATCHES "/VC$")
      get_filename_component(LOCAL_COMPILER_DIR ${LOCAL_COMPILER_DIR} DIRECTORY)
    endwhile()
    file(GLOB_RECURSE VCVARSALL_BAT LIST_DIRECTORIES false "${LOCAL_COMPILER_DIR}/**/vcvarsall.bat")
    set(VSCmdPath "${VCVARSALL_BAT}" CACHE PATH "Visual Studio Command Line Path" FORCE)
  endif()
  if (VSCmdPath AND (NOT "${VSCmdPath}" STREQUAL ""))
    if ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
      execute_process(COMMAND cmd /C "${VSCmdPath}" "amd64" & ${ARGN} WORKING_DIRECTORY "${pwd}")
    else()
      execute_process(COMMAND cmd /C "${VSCmdPath}" "x86" & ${ARGN} WORKING_DIRECTORY "${pwd}")
    endif()
  endif()
endfunction()

function(install_glfw Target)
  git_clone("ThirdParty/glfw" "https://github.com/glfw/glfw.git" "7614d088e94ad7e245ea00f77f982c8709adb060")
  git_clone("ThirdParty/glew-cmake" "https://github.com/Perlmint/glew-cmake.git" "glew-cmake-2.2.0")
  set(GLFW_LIBRARY_TYPE "STATIC")
  add_subdirectory("${PROJECT_SOURCE_DIR}/ThirdParty/glfw")
  add_subdirectory("${PROJECT_SOURCE_DIR}/ThirdParty/glew-cmake")
  target_include_directories(${Target} PRIVATE "${PROJECT_SOURCE_DIR}/ThirdParty/glfw/include")
  target_include_directories(${Target} PRIVATE "${PROJECT_SOURCE_DIR}/ThirdParty/glew-cmake/include")
  target_link_libraries(${Target} PRIVATE ${LIBRARY_NAME} PRIVATE glfw PRIVATE libglew_static)
  if (WIN32)
    target_compile_definitions(glfw PRIVATE GLFW_EXPOSE_NATIVE_WIN32)
    target_compile_definitions(${Target} PRIVATE GLFW_EXPOSE_NATIVE_WIN32)
    target_link_libraries(${Target} PRIVATE opengl32.lib)
  elseif (APPLE)
    target_compile_definitions(glfw PRIVATE GLFW_EXPOSE_NATIVE_COCOA)
    target_compile_definitions(${Target} PRIVATE GLFW_EXPOSE_NATIVE_COCOA)
    find_library(OPENGL OpenGL)
    target_link_libraries(${Target} PRIVATE ${OPENGL})
    find_library(APPKIT AppKit)
    target_link_libraries(${Target} PRIVATE ${APPKIT})
    find_library(CORE_FOUNDATION CoreFoundation)
    target_link_libraries(${Target} PRIVATE ${CORE_FOUNDATION})
    find_library(CORE_GRAPHICS CoreGraphics)
    target_link_libraries(${Target} PRIVATE ${CORE_GRAPHICS})
    find_library(QUARTZ_CORE QuartzCore)
    target_link_libraries(${Target} PRIVATE ${QUARTZ_CORE})
  endif()
endfunction()