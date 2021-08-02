CC := gcc
INCLUDE_DIRS += ../../Common/Inc ../Stubs ../cunit ../utils
SOURCES := fir_test.c ../../CM7/Core/Src/fir.c ../../Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_f32.c ../../Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_f32.c

BIN = fir_test
GEN = ../gensh/fir_generate.sh
INPUT_DATA = x.dat coeff.dat y.dat

#########################################

FLAGS := $(BUILD_FLAGS)
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

