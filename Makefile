# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------
# Source Directories
C_SRCDIR = src
CPP_SRCDIR = src

# Source files for C and C++
C_SRCS = $(wildcard $(C_SRCDIR)/*.c)
CPP_SRCS = $(wildcard $(CPP_SRCDIR)/*.cpp)

# Object files of the source files
C_OBJS = $(C_SRCS:$(C_SRCDIR)/%.c=$(C_SRCDIR)/%.o)
CPP_OBJS = $(CPP_SRCS:$(CPP_SRCDIR)/%.cpp=$(CPP_SRCDIR)/%.o)

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

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
