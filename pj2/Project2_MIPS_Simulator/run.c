/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction() {
	instruction *instr;
    instr = get_inst_info(CURRENT_STATE.PC);
    if(instr->value == 0) {
        RUN_BIT = FALSE;
        return;
    }

    CURRENT_STATE.PC += BYTES_PER_WORD;

    if(instr->opcode == 0x00) {
        //R type
        uint8_t rs,rt,rd,shamt,funct;
        rs = instr->r_t.r_i.rs;
        rt = instr->r_t.r_i.rt;
        rd = instr->r_t.r_i.r_i.r.rd;
        shamt = instr->r_t.r_i.r_i.r.shamt;
        funct = instr->func_code;

        switch(funct) {
        case 0x21: //addu
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        break;
        case 0x24: //and
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
        break;
        case 0x08: //jr
            CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
        break;
        case 0x27: //nor
            CURRENT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
        break;
        case 0x25: //or
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
        break;
        case 0x2b: //sltu
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt];
        break;
        case 0x00: //sll
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
        break;
        case 0x02: //SRL
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
        break;
        case 0x23: //subu
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
        break;
        default:
            //assert(0);
        break;
        }
    }
    else if(instr->opcode == 0x02 || instr->opcode == 0x03) {
        //J type
        uint32_t addr;
        addr = instr->r_t.target;

        if(instr->opcode == 0x02) { //j
            CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (addr * BYTES_PER_WORD);
        }
        else if(instr->opcode == 0x03){ //jal
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
            CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (addr * BYTES_PER_WORD);
        }
        else {
            //assert(0);
        }
    }
    else {
        //I type
        uint8_t rs, rt;
        int32_t simm, zimm;
        rs = instr->r_t.r_i.rs;
        rt = instr->r_t.r_i.rt;
        simm = instr->r_t.r_i.r_i.imm; //sign extended
        zimm = simm & 0x0000ffff;      //zero extended

        switch(instr->opcode) {
        case 0x09: //addiu
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + simm;
        break;
        case 0x0c: //andi
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & zimm;
        break;
        case 0x04: //beq
            if(CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt])
                CURRENT_STATE.PC += simm * BYTES_PER_WORD;
        break;
        case 0x05: //bne
            if(CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt])
                CURRENT_STATE.PC += simm * BYTES_PER_WORD;
        break;
        case 0x0f: //lui
            CURRENT_STATE.REGS[rt] = simm << 16;
        break;
        case 0x23: //lw
            CURRENT_STATE.REGS[rt] = mem_read_32(CURRENT_STATE.REGS[rs] + simm);
        break;
        case 0x0d: //ori
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | zimm;
        break;
        case 0x0b: //sltiu
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] < simm;
        break;
        case 0x2b: //sw
            mem_write_32(CURRENT_STATE.REGS[rs] + simm, CURRENT_STATE.REGS[rt]);
        break;
        default:
            //assert(0);
        break;
        }
    }

	return;
}
