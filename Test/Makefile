CC := gcc
DEBUG_FLAGS := -O0 -g3 -Wall -fmessage-length=0 -Wno-unused-variable
RELEASE_FLAGS := -O3 -Wall -fmessage-length=0 -Wno-unused-variable

SUBDIRS = fft_16hist

build: $(SUBDIRS)

clean: 
	$(MAKE) -C $(SUBDIRS) clean
	$(MAKE) -C cunit clean

run: build
	$(MAKE) -C $(SUBDIRS) run

$(SUBDIRS): cunit
	$(MAKE) -C $@

cunit:
	$(MAKE) -C $@

.PHONY: subdirs $(SUBDIRS) cunit
.PHONY: build clean run

