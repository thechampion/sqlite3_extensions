CC = gcc
CCFLAGS = -fPIC -std=c99 -Wall `pkg-config --cflags --libs openssl`

all: hashlib.so

hashlib.so: hashlib.c
	$(CC) $(CCFLAGS) -shared -o $@ $^
