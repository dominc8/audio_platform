CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -Wall -fmessage-length=0 -Wno-unused-variable
FLAGS := $(DEBUG_FLAGS) -I./

CUNIT_SRC := $(wildcard ./Sources/**/*.c)
CUNIT_OBJ := $(CUNIT_SRC:%.c=%.c.o)
CUNIT_LIB := libCUnit.a

$(CUNIT_LIB): $(CUNIT_OBJ)
	ar rcs $@ $^

clean:
	rm -rf $(CUNIT_LIB) $(CUNIT_OBJ)

%.c.o : %.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: clean
