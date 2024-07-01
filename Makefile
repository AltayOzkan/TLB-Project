# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# Source files for the C and C++ parts of the project
C_SRCS = src/main.c src/utils.c
CPP_SRCS = src/tlb_simulator.cpp src/tlb.cpp

# Object files derived from the source files
C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)

# Header files used in the project
HEADERS = src/tlb.h src/utils.h

# Name of the output executable
TARGET = tlb_simulation

# Path to your SystemC installation
SCPATH = /path/to/systemc

# Compiler flags for C++ compiler
CXXFLAGS = -std=c++14 -Wall -I$(SCPATH)/include

# Compiler flags for C compiler
CFLAGS = -std=c17 -Wall -I$(SCPATH)/include

# Linker flags to link against SystemC and other libraries
LDFLAGS = -L$(SCPATH)/lib -lsystemc -lm

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if g++ or clang++ is available, and set it as the C++ compiler
CXX = $(shell command -v g++ || command -v clang++)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

# Determine if gcc or clang is available, and set it as the C compiler
CC = $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# Add rpath linker option for all platforms except MacOS (Darwin)
UNAME_S = $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
    LDFLAGS += -Wl,-rpath=$(SCPATH)/lib
endif

# Default target to build the debug version
all: debug

# Rule to compile .c files into .o object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile .cpp files into .o object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build the debug version of the project
debug: CXXFLAGS += -g
debug: $(TARGET)

# Rule to build the release version of the project
release: CXXFLAGS += -O2
release: $(TARGET)

# Rule to link all object files into the final executable
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $(TARGET)

# Rule to clean up the build directory
clean:
	rm -f $(TARGET)
	rm -rf $(C_OBJS) $(CPP_OBJS)

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
