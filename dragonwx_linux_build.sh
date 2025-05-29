#!/bin/bash

if [ -n "$1" ] && [ -n "$2" ]; then
	g++ -I"$1" -I"$1"/include -o "$2" Main.cpp DragonWx.cpp -lfreetype -lX11 -lGL -lpthread -lpng -lstdc++fs -lcurl -std=c++17
else
	echo Syntax: "$0" "<Project directory path> <Executable output filename>"
fi
