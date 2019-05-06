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
	$(CC) $(CFLAGS) -o nodo_v nodo_validacion.c -lpthread
	$(CC) $(CFLAGS) -o cliente_v cliente_pruebas_validacion.c

jfiles:
	$(JC) $(JFLAGS) Graficas/*.java

clean:
	$(RM) nodo cliente *.class
