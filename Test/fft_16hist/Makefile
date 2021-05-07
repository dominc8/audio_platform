CC := gcc
INCLUDE_DIRS += ../../CM7/Core/Inc ../Stubs ../cunit ../utils
SOURCES := fft_16hist_test.c ../../CM7/Core/Src/fft_hist.c

BIN = fft_16hist_test
GEN = ../gensh/fft_16hist_generate.sh
INPUT_DATA = x.dat yl.dat yr.dat

#########################################

FLAGS := $(DEBUG_FLAGS)
FLAGS += $(addprefix -I,$(INCLUDE_DIRS))
LD_FLAGS = -lm

$(BIN): $(INPUT_DATA) $(SOURCES)
	$(CC) $(FLAGS) -o $@ $(SOURCES) ../cunit/libCUnit.a ../utils/libutils.a $(LD_FLAGS)

$(INPUT_DATA):
	$(GEN)

clean:
	rm -rf -- $(BIN) *.dat

run: $(BIN)
	./$(BIN)

.PHONY: clean run

