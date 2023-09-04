OUTPUT = .output

BPFTOOL_SRC = $(abspath bpftool/src)
BPFTOOL_OUTPUT = $(abspath $(OUTPUT)/bpftool)

LIBBPF_SRC = $(abspath libbpf/src)
LIBBPF_OUTPUT = $(abspath $(OUTPUT)/libbpf)

EXAMPLES_OUTPUT = $(abspath $(OUTPUT)/examples)

TARGET = examples

CLANG = clang-12

.PHONY: clean

all: $(TARGET)

$(OUTPUT):
	$(shell if [ ! -e $(OUTPUT) ];then mkdir $(OUTPUT); fi)

$(BPFTOOL_OUTPUT): $(OUTPUT)
	$(shell if [ ! -e $(BPFTOOL_OUTPUT) ];then mkdir $(BPFTOOL_OUTPUT); fi)

$(LIBBPF_OUTPUT): $(OUTPUT)
	$(shell if [ ! -e $(LIBBPF_OUTPUT) ];then mkdir $(LIBBPF_OUTPUT); fi)

$(EXAMPLES_OUTPUT):
	$(shell if [ ! -e $(EXAMPLES_OUTPUT) ];then mkdir $(EXAMPLES_OUTPUT); fi)

bpftool: $(BPFTOOL_OUTPUT)
	OUTPUT=$(BPFTOOL_OUTPUT)/ make -C $(BPFTOOL_SRC)

libbpf: $(LIBBPF_OUTPUT)
	BUILD_STATIC_ONLY=y OBJDIR=$(LIBBPF_OUTPUT) DESTDIR=$(LIBBPF_OUTPUT) make -C $(LIBBPF_SRC) install

examples: bpftool libbpf $(EXAMPLES_OUTPUT)
	OUTPUT=$(EXAMPLES_OUTPUT) \
	BPFTOOL=$(abspath $(BPFTOOL_OUTPUT)/bpftool) \
	LIBBPF_INCLUDE=$(abspath $(LIBBPF_OUTPUT)/usr/include) \
	LIBBPF_OBJ=$(abspath $(LIBBPF_OUTPUT)/libbpf.a) \
	make -C examples

clean:
	rm -rf $(OUTPUT)
