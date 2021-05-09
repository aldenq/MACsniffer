CC = g++
OPT = -O3
STD = -std=gnu++20
WARNS = -Wall -Wextra -Werror
#LIBS = 
FILE = src/main.cpp
OUT = -o dist/build.out

MACSniffer:
	${CC} ${OPT} ${STD} ${WARNS} ${FILE} ${OUT}