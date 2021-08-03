CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -g3 -Wall -fmessage-length=0 -Wno-unused-variable

export BUILD_FLAGS

SUBDIRS = biquad event_queue fft_16hist fir scheduler


build_release: BUILD_FLAGS = $(RELEASE_FLAGS)
build_release: build

build_debug: BUILD_FLAGS = $(DEBUG_FLAGS)
build_debug: build

build: $(SUBDIRS)

clean: 
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	$(MAKE) -C cunit clean
	$(MAKE) -C utils clean

run: build
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir run; done

$(SUBDIRS): cunit utils
	$(MAKE) -C $@

cunit:
	$(MAKE) -C $@

utils:
	$(MAKE) -C $@

.PHONY: subdirs $(SUBDIRS) cunit utils
.PHONY: build clean run

