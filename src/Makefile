# $(BPFTOOL) $(LIBBPF_INCLUDE) $(LIBBPF_OBJ) from outside

OUTPUT ?= $(abspath .output)
CLANG = clang-12
LIBBPF_INCLUDE ?= $(abspath ../.output/libbpf/usr/include)
LIBBPF_SRC ?= $(abspath ../libbpf/include)
LIBBPF_OBJ ?= $(abspath ../.output/libbpf/libbpf.a)
BPFTOOL ?= $(abspath ../.output/bpftool/bpftool)

INCLUDES = -I$(OUTPUT) -I$(LIBBPF_INCLUDE) -I$(LIBBPF_SRC) -I/usr/include/x86_64-linux-gnu/
TARGET = $(OUTPUT)/tc

all: $(TARGET)

$(OUTPUT):
	$(shell if [ ! -e $(OUTPUT) ];then mkdir $(OUTPUT); fi)

$(OUTPUT)/tc: $(OUTPUT)/tc.o
	$(CLANG) -O3 -g -Wall -lelf -lz $^ $(LIBBPF_OBJ) -o $@

$(OUTPUT)/tc.o: tc.c $(OUTPUT)/tc.skel.h
	$(CLANG) -O3 -g -Wall $(INCLUDES) -c $< -o $@

$(OUTPUT)/tc.skel.h: $(OUTPUT)/tc.bpf.o 
	$(BPFTOOL) gen skeleton $< > $@

$(OUTPUT)/tc.bpf.o: tc.bpf.c $(OUTPUT)/vmlinux.h
	$(CLANG) -g -O2 -target bpf $(INCLUDES) -c $< -o $@

$(OUTPUT)/vmlinux.h: 
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

.PHONY: clean
clean:
	rm -fr $(OUTPUT)/*