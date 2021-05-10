CC = g++
OPT = -O3
STD = -std=gnu++20
WARNS = -Wall -Wextra -Werror
LIBS = src/objects/KnownManufacturers.o
FILE = src/main.cpp
OUT = -o dist/build.out

src/objects/KnownManufacturers.o: src/KnownManufacturers.txt src/KnownManufacturers.cpp
	${CC} ${OPT} ${STD} -c src/KnownManufacturers.cpp -o src/objects/KnownManufacturers.o

MACSniffer: src/objects/KnownManufacturers.o
	${CC} ${OPT} ${STD} ${WARNS} ${FILE} ${OUT}