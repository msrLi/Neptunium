cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../../tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=debug  .
mingw32-make -j4 2> build_log.txt 
IF "%1" == "" ( pause ) 
