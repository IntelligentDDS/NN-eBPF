OUTPUT = .output

BPFTOOL_SRC = $(abspath bpftool/src)
BPFTOOL_OUTPUT = $(abspath $(OUTPUT)/bpftool)

LIBBPF_SRC = $(abspath libbpf/src)
LIBBPF_OUTPUT = $(abspath $(OUTPUT)/libbpf)

TARGET = examples

CLANG = clang-12

.PHONY: clean

all: bpftool libbpf

$(OUTPUT):
	$(shell if [ ! -e $(OUTPUT) ];then mkdir $(OUTPUT); fi)

$(BPFTOOL_OUTPUT): $(OUTPUT)
	$(shell if [ ! -e $(BPFTOOL_OUTPUT) ];then mkdir $(BPFTOOL_OUTPUT); fi)

$(LIBBPF_OUTPUT): $(OUTPUT)
	$(shell if [ ! -e $(LIBBPF_OUTPUT) ];then mkdir $(LIBBPF_OUTPUT); fi)

bpftool: $(BPFTOOL_OUTPUT)
	OUTPUT=$(BPFTOOL_OUTPUT)/ make -C $(BPFTOOL_SRC)

libbpf: $(LIBBPF_OUTPUT)
	BUILD_STATIC_ONLY=y OBJDIR=$(LIBBPF_OUTPUT) DESTDIR=$(LIBBPF_OUTPUT) make -C $(LIBBPF_SRC) install

clean:
	rm -rf $(OUTPUT)
