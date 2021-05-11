INCLUDE_DIRS += ../../Common/Inc ../Stubs ../cunit ../utils
SOURCES := event_queue_test.c ../../Common/Src/event_queue.c ../Stubs/shared_data.c

BIN = event_queue_test

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

