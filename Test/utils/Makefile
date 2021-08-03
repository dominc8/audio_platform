CC := gcc
FLAGS := $(BUILD_FLAGS) -I./

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
