/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

#define DIGIT_5_MASK     0x1f
#define DIGIT_6_MASK     0x3f
#define IMM_MASK    0x0000ffff
#define ADDR_MASK   0x03ffffff


instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
    uint32_t inst_code;
    inst_code=strtol(buffer,NULL,2);
    instr.opcode=(inst_code >> 26) & DIGIT_6_MASK;
    instr.value=1;

    mem_write_32(MEM_TEXT_START + index, inst_code);

    //R type
    if(instr.opcode == 0x00) {
        instr.r_t.r_i.rs = (inst_code >> 21) & DIGIT_5_MASK;
        instr.r_t.r_i.rt = (inst_code >> 16) & DIGIT_5_MASK;

        instr.r_t.r_i.r_i.r.rd = (inst_code >> 11) & DIGIT_5_MASK;
        instr.r_t.r_i.r_i.r.shamt = (inst_code >> 6) & DIGIT_5_MASK;
        instr.func_code = inst_code & DIGIT_6_MASK;
    }
    //J type
    else if(instr.opcode == 0x02 || instr.opcode == 0x03) {
        instr.r_t.target = inst_code & ADDR_MASK;
    }
    //I type
    else {
        instr.r_t.r_i.rs = (inst_code >> 21) & DIGIT_5_MASK;
        instr.r_t.r_i.rt = (inst_code >> 16) & DIGIT_5_MASK;
        instr.r_t.r_i.r_i.imm = inst_code & IMM_MASK;
    }

    //printf("%x\n",code);
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
    uint32_t data_code;
    data_code=strtol(buffer,NULL,2);
    mem_write_32(MEM_DATA_START + index, data_code);
    return;
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //I format
	    case 0x9:		//ADDIU
	    case 0xc:		//ANDI
	    case 0xf:		//LUI
	    case 0xd:		//ORI
	    case 0xb:		//SLTIU
	    case 0x23:		//LW
	    case 0x2b:		//SW
	    case 0x4:		//BEQ
	    case 0x5:		//BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //R format
	    case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //J format
	    case 0x2:		//J
	    case 0x3:		//JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
