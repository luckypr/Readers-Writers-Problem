#I want to create a perfect build system

CC=gcc
CFLAGS=-Wall -I$(INC)


TARGET= bin/w1 bin/w2 bin/r

SRC= src
INC= inc
BIN= bin




SOURCE=$(wildcard $(SRC)/*.c)
OBJECT=$(patsubst %,$(BIN)/%, $(notdir $(SOURCE:.c=.o)))
MAINS= bin/w1.o bin/w2.o bin/r.o
DEPEND=$(filter-out $(MAINS),$(OBJECT)) 

RED=\033[1;31m
GREEN=\033[1;32m
NC=\033[1;0m

all: $(TARGET)

$(TARGET): $(OBJECT)
	@echo	"$(RED)Linking $(NC) $@... "
	$(CC) -o $@ $@.o $(DEPEND) #$^
	@echo	"$(GREEN)Finished linking!$(NC)"

$(BIN)/%.o: $(SRC)/%.c
	@echo	"$(RED)Compiling $(NC) $@..."
	$(CC) $(CFLAGS) -c $< -o $@
	@echo	"$(GREEN)Finished compiling$(NC)!"

.PHONY: help clean

help:
	@echo "src: $(SOURCE)" 
	@echo "obj: $(OBJECT)"
	@echo "depend: $(DEPEND)"
	@echo "target is: $(TARGET)"

clean:
	rm -f $(OBJECT) $(TARGET)
