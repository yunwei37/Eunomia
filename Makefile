.PHONY:all clean

all:
	@make -C bpftools
	@make -C cmd


clean:
	@make -C bpftools clean
	@make -C cmd clean