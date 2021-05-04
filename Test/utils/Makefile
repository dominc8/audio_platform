CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -Wall -fmessage-length=0 -Wno-unused-variable
FLAGS := $(DEBUG_FLAGS) -I./

SRC := $(wildcard ./*.c)
OBJ := $(SRC:%.c=%.c.o)
LIB := libutils.a

$(LIB): $(OBJ)
	ar rcs $@ $^

clean:
	rm -rf $(LIB) $(OBJ)

%.c.o : %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: clean
