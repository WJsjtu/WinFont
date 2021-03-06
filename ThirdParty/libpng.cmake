git_clone("ThirdParty/libpng" https://github.com/glennrp/libpng.git "v1.6.37")
set(LIBPNG_INSTALL_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/libpng")
set(LIBPNG_SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/libpng")

execute_process(COMMAND ${CMAKE_COMMAND} 
  -G "${CMAKE_GENERATOR}" 
  -A "${CMAKE_GENERATOR_PLATFORM}" 
  -T "${CMAKE_GENERATOR_TOOLSET}" 
  -DCMAKE_CXX_FLAGS_RELEASE="/MD" 
  -DCMAKE_CXX_FLAGS_DEBUG="/MDd" 
  -DCMAKE_C_FLAGS_RELEASE="/MD" 
  -DCMAKE_C_FLAGS_DEBUG="/MDd" 
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
  -DPNG_SHARED=OFF 
  -DPNG_EXECUTABLES=OFF 
  -DZLIB_INCLUDE_DIR=${ZLIB_DIR}/include 
  -DZLIB_LIBRARY=${ZLIB_LIBRARY} 
  -DCMAKE_INSTALL_PREFIX=${LIBPNG_INSTALL_DIR} 
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
  -B "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/libpng" 
  WORKING_DIRECTORY "${LIBPNG_SOURCE_DIR}"
)
execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target install 
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/libpng"
)

set(LIBPNG_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/libpng" CACHE PATH "libpng dir" FORCE)
# set(LIBPNG_ROOT "${LIBPNG_DIR}" CACHE PATH "libpng root" FORCE)

if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES RELWITHDEBINFO) 
  set(LIBPNG_LIBRARY "${LIBPNG_DIR}/lib/libpng16_staticd.lib" CACHE FILEPATH "libpng library" FORCE)
endif()
if(CMAKE_BUILD_TYPE MATCHES RELEASE OR CMAKE_BUILD_TYPE MATCHES MINSIZEREL) 
  set(LIBPNG_LIBRARY "${LIBPNG_DIR}/lib/libpng16_static.lib" CACHE FILEPATH "libpng library" FORCE)
endif()