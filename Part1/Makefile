src = $(wildcard *.cpp) # \
	  # $(wildcard subdir1/*.cpp) \
	  # $(wildcard subdir2/*.ccp)

obj = $(src:.cpp=.o)

LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system 

INTELMAC_INCLUDEDIR=/usr/local/include			# Intel mac
APPLESILICON_INCLUDEDIR=/opt/homebrew/include	# Apple Silicon
UBUNTU_APPLESILICON_INCLUDEDIR=/usr/include		# Apple Silicon Ubuntu VM
UBUNTU_INTEL_INCLUDEDIR=/usr/include			# Intel Ubuntu VM

INTELMAC_LIBPATH=/usr/local/lib 							# Intel mac
APPLESILICON_LIBPATH=/opt/homebrew/lib						# Apple Silicon
UBUNTU_APPLESILICON_LIBPATH=/usr/lib/aarch64-linux-gnu		# Apple Silicon Ubuntu VM
UBUNTU_INTEL_LIBPATH=/usr/lib/x86_64-linux-gnu				# Intel Ubuntu VM

MACOS_INCLUDE=$(APPLESILICON_INCLUDEDIR)
MACOS_LIB=$(APPLESILICON_LIBPATH)
UBUNTU_INCLUDE=$(UBUNTU_APPLESILICON_INCLUDEDIR)
UBUNTU_LIB=$(UBUNTU_APPLESILICON_LIBPATH)

MACOS_COMPILER=/usr/bin/clang++
UBUNTU_COMPILER=/usr/bin/g++

all: main

uname_s := $(shell uname -s)
main: $(obj)
ifeq ($(uname_s),Darwin)
	$(MACOS_COMPILER) -std=c++17  -g -o $@ $^ $(LDFLAGS) -L$(MACOS_LIB)
else ifeq ($(uname_s),Linux)
	$(UBUNTU_COMPILER) -std=c++17 -g -o $@ $^ $(LDFLAGS) -L$(UBUNTU_LIB)
endif

uname_s := $(shell uname -s)
%.o: %.cpp
ifeq ($(uname_s),Darwin)
	$(MACOS_COMPILER) -std=c++17 -g -c $^ -I$(MACOS_INCLUDE)
else ifeq ($(uname_s),Linux)
	$(UBUNTU_COMPILER) -std=c++17 -g -c $^ -I$(UBUNTU_INCLUDE)
endif

.PHONY: clean
clean:
	rm -f $(obj) main
