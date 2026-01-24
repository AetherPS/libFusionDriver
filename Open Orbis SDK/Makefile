# Library metadata.
TARGET := libFusionDriverOOSDK.a

# Additional compile flags.
EXTRAFLAGS  := -D_OOSDK -std=c++17 -Wno-address-of-packed-member

# You likely won't need to touch anything below this point.

# Root vars
TOOLCHAIN 	:= $(OO_PS4_TOOLCHAIN)

# Check if TOOLCHAIN is set
ifeq ($(TOOLCHAIN),)
$(error OO_PS4_TOOLCHAIN environment variable is not set. Please set it in WSL or configure WSLENV in Windows)
endif

SOLUTIONDIR := ..
SOURCEDIR   := $(SOLUTIONDIR)/Source
INCLUDEDIR  := $(SOLUTIONDIR)/Include
EXTERNALDIR := $(SOLUTIONDIR)/External
BUILDDIR    := $(SOLUTIONDIR)/Build
SHELLCODEDIR := $(BUILDDIR)/ShellCode
INTDIR      := x64/Debug

# Source files - automatically find all cpp files in Source directory
CPPFILES    := $(wildcard $(SOURCEDIR)/*.cpp)

# Object files - convert source paths to object file paths
OBJS        := $(patsubst $(SOURCEDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES)) \
               $(INTDIR)/stdafx.o

# Shellcode object files
SHELLCODE_OBJS := $(INTDIR)/ThreadShellCode.o $(INTDIR)/FuncCallShellCode.o

# Define final C/C++ flags
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c $(EXTRAFLAGS) \
               -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include \
               -I$(INCLUDEDIR) -I$(EXTERNALDIR)/FusionShared -I$(EXTERNALDIR)/StubMaker/include -I.
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1

# Create the intermediate directory if it doesn't already exist.
_unused     := $(shell mkdir -p $(INTDIR))

# Check for linux vs macOS and account for clang/ld path
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CC      := clang
	CCX     := clang++
	AR      := llvm-ar
	OBJCOPY := llvm-objcopy
	CDIR    := linux
endif
ifeq ($(UNAME_S),Darwin)
	CC      := /usr/local/opt/llvm/bin/clang
	CCX     := /usr/local/opt/llvm/bin/clang++
	AR      := /usr/local/opt/llvm/bin/llvm-ar
	OBJCOPY := /usr/local/opt/llvm/bin/llvm-objcopy
	CDIR    := macos
endif

# Main target - static library
$(BUILDDIR)/$(TARGET): $(OBJS) $(SHELLCODE_OBJS)
	@mkdir -p $(BUILDDIR)
	$(AR) rcs $@ $(OBJS) $(SHELLCODE_OBJS)

# Embed shellcode binaries as object files
$(INTDIR)/ThreadShellCode.o: $(SHELLCODEDIR)/ThreadShellCode.bin
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $< $@ \
		--redefine-sym _binary____Build_ShellCode_ThreadShellCode_bin_start=_binary_ThreadShellCode_bin_start \
		--redefine-sym _binary____Build_ShellCode_ThreadShellCode_bin_end=_binary_ThreadShellCode_bin_end \
		--redefine-sym _binary____Build_ShellCode_ThreadShellCode_bin_size=_binary_ThreadShellCode_bin_size

$(INTDIR)/FuncCallShellCode.o: $(SHELLCODEDIR)/FuncCallShellCode.bin
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $< $@ \
		--redefine-sym _binary____Build_ShellCode_FuncCallShellCode_bin_start=_binary_FuncCallShellCode_bin_start \
		--redefine-sym _binary____Build_ShellCode_FuncCallShellCode_bin_end=_binary_FuncCallShellCode_bin_end \
		--redefine-sym _binary____Build_ShellCode_FuncCallShellCode_bin_size=_binary_FuncCallShellCode_bin_size

# Compile source files from Source directory
$(INTDIR)/%.o: $(SOURCEDIR)/%.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

# Compile stdafx.cpp from current directory (precompiled header setup)
$(INTDIR)/stdafx.o: stdafx.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

.PHONY: clean all
.DEFAULT_GOAL := all

all: $(BUILDDIR)/$(TARGET)

clean:
	rm -f $(BUILDDIR)/$(TARGET) $(OBJS) $(SHELLCODE_OBJS)