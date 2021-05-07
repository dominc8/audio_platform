CC := gcc
INCLUDE_DIRS += ../../CM4/Core/Inc ../Stubs ../cunit ../utils
SOURCES := scheduler_test.c ../../CM4/Core/Src/scheduler.c

BIN = scheduler_test

#########################################

FLAGS := $(DEBUG_FLAGS)
FLAGS += $(addprefix -I,$(INCLUDE_DIRS))
FLAGS += -lm

$(BIN): $(SOURCES)
	$(CC) $(FLAGS) -o $@ $(SOURCES) ../cunit/libCUnit.a ../utils/libutils.a

clean:
	rm -rf -- $(BIN)

run: $(BIN)
	./$(BIN)

.PHONY: clean run

