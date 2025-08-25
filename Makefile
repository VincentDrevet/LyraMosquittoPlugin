include config.mk

.PHONY: clean

CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
LIBS = -lcjson -lcurl

TARGET = lyra_plugin.so
SOURCES = src/lyra_plugin.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -I include/
	mv $@ build/

clean:
	rm -vf build/*