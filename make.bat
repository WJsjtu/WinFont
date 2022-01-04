cmake -B ./Build -G "Visual Studio 15 2017" -A x64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_INSTALL_PREFIX=./Install
cmake --build ./Build --config DEBUG --target install