# SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
# Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）

.PHONY:all ipc process syscall files tcp container oomkill tcpconnlat llcstat bitesize syscount

all: ipc process syscall files tcp container oomkill tcpconnlat llcstat

ipc:
	@make -C ipc

llcstat:
	@make -C llcstat

oomkill:
	@make -C oomkill

tcpconnlat:
	@make -C tcpconnlat

process:
	@make -C process

syscall:
	@make -C syscall

syscount:
	@make -C syscount

files:
	@make -C files

tcp:
	@make -C tcpconnect

container:
	@make -C container

bitesize:
	@make -C bitesize

clean:
	@make -C files clean
	@make -C process clean
	@make -C syscall clean
	@make -C ipc clean
	@make -C tcpconnect clean
	@make -C container clean
	@make -C oomkill clean
	@make -C tcpconnlat clean
	@make -C llcstat clean
	@make -C syscount clean
	@make -C bitesize clean
