CC=gcc
BIN=bin
SRC=src

all: setup light

setup: clean
	mkdir $(BIN)

light: $(SRC)/light.c
	$(CC) -std=c99 -o $(BIN)/light $(SRC)/light.c -lwiringPi

clean: 
	rm -rf bin/
