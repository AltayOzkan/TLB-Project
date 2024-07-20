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
C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)

# Header files 
HEADERS = $(wildcard $(C_SRCDIR)/*.h) $(wildcard $(CPP_SRCDIR)/*.h)

# Name of the output executable
TARGET = tlb_simulation

# Path to SystemC
SYSTEMC_HOME = ../systemc

# C++ Compiler Flags
CXXFLAGS = -std=c++14 -Wall -I$(SYSTEMC_HOME)/include

# C Compiler Flags
CFLAGS = -std=c17 -Wall -I$(SYSTEMC_HOME)/include

# Linker flags -> link SystemC and other libraries
LDFLAGS = -L$(SYSTEMC_HOME)/lib -lsystemc -lm

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# check if g++/ clang++ is available -> set as C++ compiler
CXX = $(shell command -v g++ || command -v clang++)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

# check if gcc/ clang is available -> set as C compiler
CC = $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# rpath linker  
LDFLAGS += -Wl,-rpath=$(SYSTEMC_HOME)/lib

# Default target to build the debug version
all: debug

# compile .c files into .o object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# compile .cpp files into .o object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# build the debug version
debug: CXXFLAGS += -g
debug: $(TARGET)

# build the release version 
release: CXXFLAGS += -O2
release: $(TARGET)

# link all object files into the final executable
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $(TARGET)

# clean up the build directory
clean:
	rm -f $(TARGET)
	rm -rf $(C_OBJS) $(CPP_OBJS)

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
