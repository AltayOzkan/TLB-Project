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
all: $(SYSTEMC_HOME) debug

# Download and install SystemC
$(SYSTEMC_HOME):
	wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
	tar -xzf systemc-2.3.3.tar.gz
	cd systemc-2.3.3 && mkdir -p objdir && cd objdir && \
	../configure --prefix=$(SYSTEMC_HOME) && \
	$(MAKE) -j4 && $(MAKE) install
	rm -rf systemc-2.3.3 systemc-2.3.3.tar.gz

# compile .c files into .o object files
$(C_SRCDIR)/%.o: $(C_SRCDIR)/%.c $(HEADERS) | $(SYSTEMC_HOME)
	$(CC) $(CFLAGS) -c $< -o $@

# compile .cpp files into .o object files
$(CPP_SRCDIR)/%.o: $(CPP_SRCDIR)/%.cpp $(HEADERS) | $(SYSTEMC_HOME)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# build the debug version
debug: CXXFLAGS += -g
debug: $(TARGET)

# build the release version 
release: CXXFLAGS += -O2
release: $(TARGET)

# link all object files into the final executable
$(TARGET): $(C_OBJS) $(CPP_OBJS) | $(SYSTEMC_HOME)
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $(TARGET)

# clean up the build directory
clean:
	rm -f $(TARGET)
	rm -f $(C_SRCDIR)/*.o
	rm -f $(CPP_SRCDIR)/*.o

# Mark targets as phony to prevent conflicts with files of the same name
.PHONY: all debug release clean
