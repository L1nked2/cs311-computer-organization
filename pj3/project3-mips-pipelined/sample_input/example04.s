	.data
	.text
main:
	addu	$2, $4, $5 // 000
	addu	$2, $6, $7 // 004
	subu	$9, $3, $2 // 008
lab1:
	and	$11, $11, $0 // 00c
	addiu	$10, $10, 0x1 // 010
	or	$6, $6, $0 // 014
	jal	lab3 // 018
	addu	$0, $0, $0 // 01c
lab3:
	sll	$7, $6, 2 // 020
	srl	$5, $4, 2 // 024
	sltiu	$9, $10, 100 // 028
	beq	$9, $0, lab4 // 02c
	jr	$31 // 030
lab4:
	sltu	$4, $2, $3 // 034
	bne	$4, $0, lab5 // 038
	j	lab1 // 03c
lab5:
	ori	$16, $16, 0xf0f0 // 040
