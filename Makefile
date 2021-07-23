all: main

main: main.cpp tui.cpp tui.hpp crc_32.cpp crc_32.hpp types.hpp
	g++ main.cpp tui.cpp -g -o main -Wall -Wno-unused-variable

run: main
	./main