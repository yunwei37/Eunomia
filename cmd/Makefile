CXX := g++
SRC := ../src
OUTPUT_INCULDE := -I$(SRC)/process/.output -I$(SRC)/syscall/.output
LIBRARY_PATH := $(SRC)/process/.output/libbpf.a
VMLINUX := ../vmlinux/$(ARCH)/vmlinux.h
INCLUDES := $(OUTPUT_INCULDE) -I../libbpf/include/uapi -I$(SRC)/include -I$(dir $(VMLINUX)) -I$(SRC) -isystem library
CFLAGS := -g -Wall -Wno-unused-function -Wno-unused-variable -std=c++20

OBJECT := cmd.o
APP := cmd

.PHONY:ebpf clean

all: $(APP)

ebpf:
	@make -C ../src

$(OBJECT): cmd.cpp ebpf
	@$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Build application binary
$(APP): %: $(OBJECT) $(LIBRARY_PATH)
	$(call msg,BINARY,$@)
	$(CXX) $(CFLAGS) $^ -lelf -lz -lpthread -o $@

clean:
	@make -C ../src clean
	rm -f $(APP) $(OBJECT)
	
