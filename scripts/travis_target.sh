#! /bin/bash

# Travis cpp jobs construct a matrix based on environment variables
# (and the value of 'compiler'). In order to test multiple builds
# (release/debug/cpplint/cppcheck), this uses a TRAVIS_TARGET env
# var to do the right thing.

function travis_make()
{
    mkdir $1 && cd $1

    # Set compiler to GCC 4.8 here, as Travis overrides the global variables.
    export CC=gcc-4.8 CXX=g++-4.8
    # Generate build files
    [ $1 == "debug" ] && export CMAKE_VARS="-DCMAKE_BUILD_TYPE=Debug -DCOVERALLS=ON"
    [ $1 == "shared_release" ] && export CMAKE_VARS="-DLEATHERMAN_SHARED=ON"
    cmake $CMAKE_VARS -DCMAKE_INSTALL_PREFIX=$USERDIR ..
    if [ $? -ne 0 ]; then
        echo "cmake failed."
        exit 1
    fi

    # Build project
    if [ $1 == "cpplint" ]; then
        export MAKE_TARGET="cpplint"
    elif [ $1 == "cppcheck" ]; then
        export MAKE_TARGET="cppcheck"
    else
        export MAKE_TARGET="all"
    fi
    make $MAKE_TARGET -j2
    if [ $? -ne 0 ]; then
        echo "build failed."
        exit 1
    fi

    # Run tests and install if not doing cpplint/cppcheck
    if [ $1 != "cppcheck" -a $1 != "cpplint" ]; then
        LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so ctest -V 2>&1 | c++filt
        if [ ${PIPESTATUS[0]} -ne 0 ]; then
            echo "tests reported an error."
            exit 1
        fi

        if [ $1 == "debug" ]; then
            # Ignore coveralls results, keep service success uncoupled
            coveralls --gcov gcov-4.8 --gcov-options '\-lp' -r .. >/dev/null || true
        fi

        mkdir dest
        make install DESTDIR=`pwd`/dest
    fi

    # If this is a release build, prepare an artifact for github
    if [ $1 == "release" ]; then
        cd dest/$USERDIR
        tar czvf $TRAVIS_BUILD_DIR/leatherman.tar.gz `find . -type f -print`
    fi
}

case $TRAVIS_TARGET in
  "CPPLINT" )  travis_make cpplint ;;
  "CPPCHECK" ) travis_make cppcheck ;;
  "RELEASE" )  travis_make release ;;
  "DEBUG" )    travis_make debug ;;
  "SHARED_RELEASE" ) travis_make shared_release ;;
  *)           echo "Nothing to do!"
esac
