#!/bin/sh

\rm -fr CMakeCache.txt  CMakeFiles  cmake_install.cmake  libmistnet.so Makefile


#make[2]: Entering directory '/projects/volbird/vol2birdR/mistnet'
#[ 50%] Building CXX object CMakeFiles/mistnet.dir/src/mistnet.cpp.o
#/usr/bin/c++ -DUSE_C10D_GLOO -DUSE_DISTRIBUTED -DUSE_RPC -DUSE_TENSORPIPE -Dmistnet_EXPORTS -I/projects/volbird/vol2birdR/mistnet/include -isystem /projects/volbird/vol2birdR/mistnet/libtorch/include -isystem /projects/volbird/vol2birdR/mistnet/libtorch/include/torch/csrc/api/include -D_GLIBCXX_USE_CXX11_ABI=1 -fPIC -D_GLIBCXX_USE_CXX11_ABI=1 -std=gnu++17 -o CMakeFiles/mistnet.dir/src/mistnet.cpp.o -c /projects/volbird/vol2birdR/mistnet/src/mistnet.cpp
#[100%] Linking CXX shared library libmistnet.so
#/usr/bin/cmake -E cmake_link_script CMakeFiles/mistnet.dir/link.txt --verbose=1
#/usr/bin/c++ -fPIC  -D_GLIBCXX_USE_CXX11_ABI=1 -shared -Wl,-soname,libmistnet.so -o libmistnet.so CMakeFiles/mistnet.dir/src/mistnet.cpp.o  -Wl,-rpath,/projects/volbird/vol2birdR/mistnet/libtorch/lib libtorch/lib/libtorch.so libtorch/lib/libc10.so libtorch/lib/libkineto.a -Wl,--no-as-needed,"/projects/volbird/vol2birdR/mistnet/libtorch/lib/libtorch_cpu.so" -Wl,--as-needed libtorch/lib/libc10.so -Wl,--no-as-needed,"/projects/volbird/vol2birdR/mistnet/libtorch/lib/libtorch.so" -Wl,--as-needed 

