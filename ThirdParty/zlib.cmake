git_clone("ThirdParty/zlib" https://github.com/madler/zlib.git "v1.2.11")

set(ZLIB_INSTALL_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/zlib")
set(ZLIB_SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/zlib")

execute_process(COMMAND ${CMAKE_COMMAND} 
  -G "${CMAKE_GENERATOR}" 
  -A "${CMAKE_GENERATOR_PLATFORM}" 
  -T "${CMAKE_GENERATOR_TOOLSET}" 
  -DCMAKE_CXX_FLAGS_RELEASE="/MD" 
  -DCMAKE_CXX_FLAGS_DEBUG="/MDd" 
  -DCMAKE_C_FLAGS_RELEASE="/MD" 
  -DCMAKE_C_FLAGS_DEBUG="/MDd" 
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
  -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR} 
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
  -B "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/zlib" 
  WORKING_DIRECTORY "${ZLIB_SOURCE_DIR}"
)
execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target install 
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/zlib"
)

set(ZLIB_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/zlib" CACHE PATH "zlib dir" FORCE)
# set(ZLIB_ROOT "${ZLIB_DIR}" CACHE PATH "zlib root" FORCE)

if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES RELWITHDEBINFO) 
  set(ZLIB_STATIC_LIBRARY "${ZLIB_DIR}/lib/zlibstaticd.lib" CACHE FILEPATH "zlib library" FORCE)
  set(ZLIB_SHARED_DLL "${ZLIB_DIR}/bin/zlibd.dll" CACHE FILEPATH "zlib library" FORCE)
  set(ZLIB_LIBRARY "${ZLIB_DIR}/lib/zlibd.lib" CACHE FILEPATH "zlib library" FORCE)
endif()
if(CMAKE_BUILD_TYPE MATCHES RELEASE OR CMAKE_BUILD_TYPE MATCHES MINSIZEREL) 
  set(ZLIB_STATIC_LIBRARY "${ZLIB_DIR}/lib/zlibstatic.lib" CACHE FILEPATH "zlib library" FORCE)
  set(ZLIB_SHARED_DLL "${ZLIB_DIR}/bin/zlib.dll" CACHE FILEPATH "zlib library" FORCE)
  set(ZLIB_LIBRARY "${ZLIB_DIR}/lib/zlib.lib" CACHE FILEPATH "zlib library" FORCE)
endif()