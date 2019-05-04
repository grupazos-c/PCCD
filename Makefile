# the compiler: gcc for C program, define as g++ for C++
CC = gcc
# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

all:
	$(CC) $(CFLAGS) -o nodo_d nodo_demostracion.c -lpthread
	$(CC) $(CFLAGS) -o cliente cliente.c

clean:
	$(RM) $(TARGET)
