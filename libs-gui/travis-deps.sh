#! /usr/bin/env sh

set -ex

DEP_SRC=$HOME/dependency_source/
DEP_ROOT=$HOME/staging

install_gnustep_make() {
    cd $DEP_SRC
    git clone https://github.com/gnustep/tools-make.git
    cd tools-make
    if [ -n "$RUNTIME_VERSION" ]
    then
        WITH_RUNTIME_ABI="--with-runtime-abi=${RUNTIME_VERSION}"
    else
        WITH_RUNTIME_ABI=""
    fi
    ./configure --prefix=$DEP_ROOT --with-library-combo=$LIBRARY_COMBO $WITH_RUNTIME_ABI
    make install
    echo Objective-C build flags: `$HOME/staging/bin/gnustep-config --objc-flags`
}

install_ng_runtime() {
    cd $DEP_SRC
    git clone https://github.com/gnustep/libobjc2.git
    cd libobjc2
    git submodule init
    git submodule sync
    git submodule update
    cd ..
    mkdir libobjc2/build
    cd libobjc2/build
    export CC="clang"
    export CXX="clang++"
    export CXXFLAGS="-std=c++11"
    cmake -DTESTS=off -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGNUSTEP_INSTALL_TYPE=NONE -DCMAKE_INSTALL_PREFIX:PATH=$DEP_ROOT ../
    make install
}

install_libdispatch() {
    cd $DEP_SRC
    # will reference upstream after https://github.com/apple/swift-corelibs-libdispatch/pull/534 is merged
    git clone -b system-blocksruntime https://github.com/ngrewe/swift-corelibs-libdispatch.git
    mkdir swift-corelibs-libdispatch/build
    cd swift-corelibs-libdispatch/build
    export CC="clang"
    export CXX="clang++"
    export LIBRARY_PATH=$DEP_ROOT/lib;
    export LD_LIBRARY_PATH=$DEP_ROOT/lib:$LD_LIBRARY_PATH;
    export CPATH=$DEP_ROOT/include;
    cmake -DBUILD_TESTING=off -DCMAKE_BUILD_TYPE=RelWithDebInfo  -DCMAKE_INSTALL_PREFIX:PATH=$HOME/staging -DINSTALL_PRIVATE_HEADERS=1 -DBlocksRuntime_INCLUDE_DIR=$DEP_ROOT/include -DBlocksRuntime_LIBRARIES=$DEP_ROOT/lib/libobjc.so ../
    make install
}

install_gnustep_base() {
    export GNUSTEP_MAKEFILES=$HOME/staging/share/GNUstep/Makefiles
    . $HOME/staging/share/GNUstep/Makefiles/GNUstep.sh

    cd $DEP_SRC
    git clone https://github.com/gnustep/libs-base.git
    cd libs-base
    ./configure
    make
    make install
}

mkdir -p $DEP_SRC
if [ "$LIBRARY_COMBO" = 'ng-gnu-gnu' ]
then
    install_ng_runtime
    install_libdispatch
fi

install_gnustep_make
install_gnustep_base
