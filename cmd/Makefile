CXX := g++
SRC := ../src
OUTPUT := $(SRC)/process/.output
LIBRARY_PATH := $(SRC)/process/.output/libbpf.a
VMLINUX := ../vmlinux/$(ARCH)/vmlinux.h
INCLUDES := -I$(OUTPUT) -I../libbpf/include/uapi -I$(SRC)/include -I$(dir $(VMLINUX)) -I$(SRC)
CFLAGS := -g -Wall -Wno-unused-function -Wno-unused-variable -std=c++20
APP := cmd

.PHONY:ebpf clean

all: $(APP)

ebpf:
	@make -C ../src

cmd.o: cmd.cpp ebpf
	@$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Build application binary
$(APP): %: ./%.o $(LIBRARY_PATH)
	$(call msg,BINARY,$@)
	$(CXX) $(CFLAGS) $^ -lelf -lz -lpthread -o $@

clean:
	@make -C ../src clean
	