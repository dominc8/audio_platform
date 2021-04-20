CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -Wall -fmessage-length=0 -Wno-unused-variable

INCLUDE_DIRS := ../CM7/Core/Inc ./Stubs
SOURCES := fft_16hist_test.c ../CM7/Core/Src/fft_hist.c

BIN = fft_16hist_test
GEN = ./gensh/fft_16hist_generate.sh

#########################################

FLAGS := $(DEBUG_FLAGS)
FLAGS += $(addprefix -I,$(INCLUDE_DIRS))
FLAGS += -lm

$(BIN): generator
	$(CC) $(FLAGS) -o $@ $(SOURCES)

generator:
	$(GEN)

