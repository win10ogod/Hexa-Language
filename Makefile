CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include
SOURCES = src/main.c src/lexer.c src/parser.c src/value.c src/environment.c src/evaluator.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = hexai

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean 