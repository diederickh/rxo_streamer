#!/bin/sh

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

if [ ! -d ../extern/tinylib ] ; then 
    cd ../extern
    git clone git@github.com:roxlu/tinylib.git
fi 

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/
else
    cd ./../../install/linux-gcc-x86_64/bin/
fi

#./test_ogg
#./test_webcam
#./test_webm
./test_glfw
