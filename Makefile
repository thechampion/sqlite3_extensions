CC = gcc
CCFLAGS = -fPIC -std=c99 -Wall

define get_extra_flags
	`pkg-config --cflags --libs $(1)`
endef


all: hashlib.so uriparse.so

hashlib.so: hashlib.c
	$(CC) $(CCFLAGS) -shared $(call get_extra_flags,openssl) -o $@ $^

uriparse.so: uriparse.c
	$(CC) $(CCFLAGS) -shared $(call get_extra_flags,liburiparser) -o $@ $^
