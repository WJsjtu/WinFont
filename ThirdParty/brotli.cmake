git_clone("ThirdParty/brotli" https://github.com/google/brotli "v1.0.9")
set(BROTLI_INSTALL_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/brotli")
set(BROTLI_SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/brotli")

execute_process(COMMAND ${CMAKE_COMMAND} 
  -G "${CMAKE_GENERATOR}" 
  -A "${CMAKE_GENERATOR_PLATFORM}" 
  -T "${CMAKE_GENERATOR_TOOLSET}" 
  -DCMAKE_CXX_FLAGS_RELEASE="/MD" 
  -DCMAKE_CXX_FLAGS_DEBUG="/MDd" 
  -DCMAKE_C_FLAGS_RELEASE="/MD" 
  -DCMAKE_C_FLAGS_DEBUG="/MDd" 
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
  -DCMAKE_INSTALL_PREFIX=${BROTLI_INSTALL_DIR} 
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
  -B "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/brotli" 
  WORKING_DIRECTORY "${BROTLI_SOURCE_DIR}"
)
execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target install 
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/ThirdParty/build/${CMAKE_BUILD_TYPE}/brotli"
)

set(BROTLI_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/brotli" CACHE PATH "brotli dir" FORCE)
# set(BROTLI_ROOT "${BROTLI_DIR}" CACHE PATH "brotli root" FORCE)
set(BROTLI_LIBRARY "${BROTLI_DIR}/lib/brotlidec-static.lib" CACHE PATH "brotli library" FORCE)
set(BROTLI_LIBRARIES "${BROTLI_DIR}/lib/brotlicommon-static.lib" "${BROTLI_DIR}/lib/brotlidec-static.lib" "${BROTLI_DIR}/lib/brotlienc-static.lib" CACHE PATH "brotli library" FORCE)