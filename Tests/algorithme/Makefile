# Makefile for ENIGMA-Embedded-System-Design-Project

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -g
SRCDIR := .
TARGET := app

SRC := main.c enigma.c
OBJ := main.o enigma.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/enigma.h
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
