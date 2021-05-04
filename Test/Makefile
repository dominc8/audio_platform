CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -Wall -fmessage-length=0 -Wno-unused-variable

SUBDIRS = fft_16hist

build: $(SUBDIRS)

clean: 
	$(MAKE) -C $(SUBDIRS) clean
	$(MAKE) -C cunit clean
	$(MAKE) -C utils clean

run: build
	$(MAKE) -C $(SUBDIRS) run

$(SUBDIRS): cunit utils
	$(MAKE) -C $@

cunit:
	$(MAKE) -C $@

utils:
	$(MAKE) -C $@

.PHONY: subdirs $(SUBDIRS) cunit utils
.PHONY: build clean run

