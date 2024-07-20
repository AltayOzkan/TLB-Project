.PHONY: all clean debug release systemc

# Check if g++ is available, if not, check if clang++ is available
CXX := $(shell command -v g++ 2>/dev/null)
ifeq ($(strip $(CXX)),)
    CXX := $(shell command -v clang++ 2>/dev/null)
endif

dest_dir := $(shell pwd)/systemc

# Default target to build SystemC and the debug version of the project
all: systemc debug

# Rule to download and install SystemC
systemc: configure
	mkdir -p systemc
	cd temp/systemc/objdir && \
	cmake -DCMAKE_INSTALL_PREFIX="$(dest_dir)" -DCMAKE_CXX_STANDARD=14 .. && \
	make -j$$(nproc) 2> ../../../install.log && \
	make install
	rm -rf temp

# Rule to configure the SystemC build
configure: temp
	cd temp/systemc && \
	mkdir -p objdir

# Rule to create a temporary directory and clone the SystemC repository
temp:
	mkdir -p temp
	cd temp && git clone --depth 1 --branch 2.3.4 https://github.com/accellera-official/systemc.git

# Rule to clean up the SystemC installation
clean:
	rm -rf systemc
	rm -rf temp
	rm -f $(TARGET)
	rm -f src/*.o

# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------
# Source Directories
C_SRCDIR = src
CPP_SRCDIR = src

# Source files for C and C++
C_SRCS = $(C_SRCDIR)/main.c
CPP_SRCS = $(CPP_SRCDIR)/simulation.cpp

# Object files of the source files
C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)

# Header files 
HEADERS = $(wildcard $(C_SRCDIR)/*.h) $(wildcard $(CPP_SRCDIR)/*.h)

# Name of the output executable
TARGET = tlb_simulation

# Path to SystemC
SYSTEMC_HOME = systemc

# C++ Compiler Flags
CXXFLAGS = -std=c++14 -Wall -I$(SYSTEMC_HOME)/include

# C Compiler Flags
CFLAGS = -std=c17 -Wall -I$(SYSTEMC_HOME)/include

# Linker flags -> link SystemC and other libraries
LDFLAGS = -L$(SYSTEMC_HOME)/lib -lsystemc -lm

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Check if g++/ clang++ is available -> set as C++ compiler
CXX := $(shell command -v g++ || command -v clang++)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

# Check if gcc/ clang is available -> set as C compiler
CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# rpath linker  
LDFLAGS += -Wl,-rpath=$(SYSTEMC_HOME)/lib

# Default target to build the debug version
all: debug

# Compile .c files into .o object files
$(C_SRCDIR)/%.o: $(C_SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile .cpp files into .o object files
$(CPP_SRCDIR)/%.o: $(CPP_SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the debug version of the project
debug: CXXFLAGS += -g
debug: $(TARGET)

# Build the release version of the project
release: CXXFLAGS += -O2
release: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $@

# Clean up the build directory
clean:
	rm -f $(TARGET)
	rm -f $(C_SRCDIR)/*.o
	rm -f $(CPP_SRCDIR)/*.o
	rm -rf systemc
	rm -rf temp

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
