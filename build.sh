#!/bin/bash

cmake_cmd=""

do_build()
{
	rm -rf build >> /dev/null
	mkdir build
	pushd build
	cmake ..
	make -j4
	popd
}

do_build

