CC = g++-10
OPT = -O3
STD = -std=gnu++20
WARNS = -Wall -Wextra -Werror
LIBS = dist/objects/KnownManufacturers.o -lpthread
FILE = src/main.cpp
OUT = -o dist/build.out

dist/objects/KnownManufacturers.o: src/data/KnownManufacturers.txt src/data/KnownManufacturers.cpp
	${CC} ${OPT} ${STD} -c src/data/KnownManufacturers.cpp -o dist/objects/KnownManufacturers.o

MACSniffer: dist/objects/KnownManufacturers.o
	${CC} ${OPT} ${STD} ${WARNS} ${FILE} ${OUT} ${LIBS}

all : MACSniffer
	