git_clone("ThirdParty/bzip2" https://sourceware.org/git/bzip2.git "bzip2-1.0.8")

set(BZIP2_INSTALL_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/bzip2")
set(BZIP2_SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/bzip2")
if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES RELWITHDEBINFO) 
file(COPY "${PROJECT_SOURCE_DIR}/ThirdParty/bzip2.makefile.debug.msc" DESTINATION "${BZIP2_SOURCE_DIR}")
ExecVSCmd("${BZIP2_SOURCE_DIR}" nmake -f bzip2.makefile.debug.msc all)
endif()
if(CMAKE_BUILD_TYPE MATCHES RELEASE OR CMAKE_BUILD_TYPE MATCHES MINSIZEREL) 
ExecVSCmd("${BZIP2_SOURCE_DIR}" nmake -f makefile.msc all)
endif()

file(COPY "${BZIP2_SOURCE_DIR}/bzip2.exe" DESTINATION "${BZIP2_INSTALL_DIR}/bin")
file(COPY "${BZIP2_SOURCE_DIR}/bzip2recover.exe" DESTINATION "${BZIP2_INSTALL_DIR}/bin")
file(COPY "${BZIP2_SOURCE_DIR}/libbz2.lib" DESTINATION "${BZIP2_INSTALL_DIR}/lib")
file(COPY "${BZIP2_SOURCE_DIR}/bzlib.h" DESTINATION "${BZIP2_INSTALL_DIR}/include")

ExecVSCmd("${BZIP2_SOURCE_DIR}" nmake -f makefile.msc clean)

set(BZIP2_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/install/${CMAKE_BUILD_TYPE}/bzip2" CACHE PATH "bzip2 dir" FORCE)
# set(BZIP2_ROOT "${BZIP2_DIR}" CACHE PATH "bzip2 root" FORCE)
set(BZIP2_LIBRARY "${BZIP2_DIR}/lib/libbz2.lib" CACHE PATH "bzip2 library" FORCE)