# the compiler: gcc for C program, define as g++ for C++
CC = gcc
# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

JC = javac
JFLAGS = -cp "Graficas/lib/*"

CLASSES= Proceso.java Valores.java Graficas.java LogParser.java

ALL= cfiles jfiles

all: $(ALL)

cfiles:
	$(CC) $(CFLAGS) -o nodo_d nodo_demostracion.c -lpthread
	$(CC) $(CFLAGS) -o cliente_p cliente_pagos.c
	$(CC) $(CFLAGS) -o cliente_p_p cliente_pagos_periodo.c

jfiles:
	$(JC) $(JFLAGS) Graficas/*.java

clean:
	$(RM) nodo cliente *.class
