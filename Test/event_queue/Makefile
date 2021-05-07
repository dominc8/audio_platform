# INCLUDE_DIRS += ../../Common/Inc ../Stubs ../cunit ../utils
# SOURCES := event_queue_test.c ../../Common/Src/event_queue.c
INCLUDE_DIRS += ../Stubs ../cunit ../utils
SOURCES := event_queue_test.c event_queue.c

BIN = event_queue_test

#########################################

FLAGS := $(DEBUG_FLAGS)
FLAGS += $(addprefix -I,$(INCLUDE_DIRS))
FLAGS += -lm

$(BIN): $(SOURCES)
	cp event_queue.c ../../Common/Src/event_queue.c
	cp event_queue.h ../../Common/Inc/event_queue.h
	$(CC) $(FLAGS) -o $@ $(SOURCES) ../cunit/libCUnit.a ../utils/libutils.a

clean:
	rm -rf -- $(BIN)

run: $(BIN)
	./$(BIN)

.PHONY: clean run

