.PHONY: all clean debug release systemc

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
SYSTEMC_HOME = $(shell pwd)/systemc

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
CXX := $(shell command -v g++ 2>/dev/null || command -v clang++ 2>/dev/null)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

# Check if gcc/ clang is available -> set as C compiler
CC := $(shell command -v gcc 2>/dev/null || command -v clang 2>/dev/null)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# rpath linker  
LDFLAGS += -Wl,-rpath=$(SYSTEMC_HOME)/lib

# Default target to build SystemC and the debug version of the project
all: systemc debug

# Download and install SystemC
systemc: configure
	mkdir -p systemc
	cd temp/systemc/objdir && \
	cmake -DCMAKE_INSTALL_PREFIX="$(SYSTEMC_HOME)" -DCMAKE_CXX_STANDARD=14 .. && \
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

# Rule to compile .c files into .o object files
$(C_SRCDIR)/%.o: $(C_SRCDIR)/%.c $(HEADERS) | systemc
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile .cpp files into .o object files
$(CPP_SRCDIR)/%.o: $(CPP_SRCDIR)/%.cpp $(HEADERS) | systemc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build the debug version
debug: CXXFLAGS += -g
debug: $(TARGET)

# Rule to build the release version 
release: CXXFLAGS += -O2
release: $(TARGET)

# Rule to link all object files into the final executable
$(TARGET): $(C_OBJS) $(CPP_OBJS) | systemc
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $(TARGET)

# Rule to clean up the build directory
clean:
	rm -f $(TARGET)
	rm -f $(C_SRCDIR)/*.o
	rm -f $(CPP_SRCDIR)/*.o
	rm -rf systemc
	rm -rf temp

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
