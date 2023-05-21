	.data
data1:	.word	100
data2:	.word	200
data3:	.word	0x12345678
	.text
main:
and	$17, $17, $0  // 000
and	$18, $18, $0 // 004
la	$8, data1 // 008, lui
la	$9, data2 // 00c, 010, lui+ori
and	$10, $10, $0 // 014
lab1:
and	$11, $11, $0 // 018
lab2:
addiu	$17, $17, 0x1 // 01c
addiu	$11, $11, 0x1 // 020
or	$9, $9, $0 // 024
bne	$11, $8, lab2 // 028
lab3:
addiu	$18, $18, 0x2 // 02c
addiu	$11, $11, 1 // 030
sll	$18, $17, 1 // 034
srl	$17, $18, 1 // 038
and	$19, $17, $18 // 03c
bne	$11, $9, lab3 // 040
lab4:
addu	$5, $5, $31 // 044
nor	$16, $17, $18 // 048
beq	$10, $8, lab5 // 04c
j	lab1 // 050
lab5:
ori	$16, $16, 0xf0f0 // 054
