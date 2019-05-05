# the compiler: gcc for C program, define as g++ for C++
CC = gcc
# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

JC = javac
JFLAGS = -cp "./lib/*"
.SUFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) Graficas/$*.java

CLASSES= Proceso.java Valores.java Graficas.java LogParser.java

ALL= cfiles jfiles

all: $(ALL)

cfiles:
	$(CC) $(CFLAGS) -o nodo nodo.c -lpthread
	$(CC) $(CFLAGS) -o cliente cliente.c

jfiles:
	$(CLASSES:.java=.class)

clean:
	$(RM) nodo cliente *.class
