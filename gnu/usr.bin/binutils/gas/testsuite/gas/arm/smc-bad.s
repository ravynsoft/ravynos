.arm
	smc #0xfefe
	smc #0x12
	smc 123

.thumb
	smc #0xdfd
	smc #0x43
	smc 4343343

.arm
.syntax unified
.cpu cortex-a8
	smc #0x6951
