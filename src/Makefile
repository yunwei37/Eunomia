.PHONY:all ipc process syscall files tcp

all: ipc process syscall files tcp

ipc:
	@make -C ipc

process:
	@make -C process

syscall:
	@make -C syscall

files:
	@make -C files

tcp:
	@make -C tcp

clean:
	@make -C files clean
	@make -C process clean
	@make -C syscall clean
	@make -C ipc clean
	@make -C tcp clean