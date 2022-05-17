.PHONY:all clean

all:
	@make -C src
	@make -C cmd


clean:
	@make -C src clean
	@make -C cmd clean