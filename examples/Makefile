# $(BPFTOOL) $(LIBBPF_INCLUDE) $(LIBBPF_OBJ) from outside

OUTPUT ?= .
CLANG = clang-12
INCLUDES = -I$(OUTPUT) -I$(LIBBPF_INCLUDE) -I/usr/include/x86_64-linux-gnu/
TARGET = $(OUTPUT)/minimal

all: $(TARGET)

$(OUTPUT)/minimal: $(OUTPUT)/minimal.o 
	$(CLANG) -g -Wall -lelf -lz $< $(LIBBPF_OBJ) -o $@

$(OUTPUT)/minimal.o: minimal.c $(OUTPUT)/minimal.skel.h
	$(CLANG) -g -Wall $(INCLUDES) -c $< -o $@

$(OUTPUT)/minimal.skel.h: $(OUTPUT)/minimal.bpf.o
	$(BPFTOOL) gen skeleton $< > $@

$(OUTPUT)/minimal.bpf.o: minimal.bpf.c
	$(CLANG) -g -O2 -target bpf $(INCLUDES) -c $< -o $@

$(OUTPUT)/vmlinux.h: 
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

