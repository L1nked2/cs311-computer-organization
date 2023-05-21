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
instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

//unsigned char debug = TRUE;
unsigned char debug = FALSE;

void IF_Stage() {
  instruction *inst;
  // if PC is out of MEM_REGIONS, stop fetching
  if (CURRENT_STATE.PC < MEM_REGIONS[IF_STAGE].start ||
    CURRENT_STATE.PC >= (MEM_REGIONS[IF_STAGE].start + (NUM_INST*BYTES_PER_WORD))) {
    FETCH_BIT = FALSE;
    CURRENT_STATE.PIPE[IF_STAGE] = 0;
  }
  // ordinary case
  else {
    CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.IF_FLUSH ? 0 : CURRENT_STATE.PC;
    inst = get_inst_info(CURRENT_STATE.PIPE[IF_STAGE]);
    // get control unit
    CURRENT_STATE.IF_ID_WRITE = !CURRENT_STATE.ID_LOAD_USE_HAZARD;
    CURRENT_STATE.PC_WRITE = CURRENT_STATE.IF_ID_WRITE;
    CURRENT_STATE.PC_SRC = CURRENT_STATE.EX_MEM_BR_TAKE;
    CURRENT_STATE.BRANCH_PC = CURRENT_STATE.EX_MEM_BR_TARGET;
    // set IF_PC
    // if PC_SRC set, make PC to be BRANCH_PC
    if (CURRENT_STATE.PC_SRC) {
      CURRENT_STATE.IF_PC = CURRENT_STATE.BRANCH_PC;
    }
    // if IF_JUMP set, make PC to be JUMP_PC
    // and stall pipe by setting IF noop
    else if (CURRENT_STATE.IF_JUMP) {
      CURRENT_STATE.IF_PC = CURRENT_STATE.JUMP_PC;
    }
    // else, just make PC to be PC+4
    else {
      CURRENT_STATE.IF_PC = CURRENT_STATE.PC + BYTES_PER_WORD;
    }
    // set PC
    // check PC_WRITE and set PC to NPC if set
    if (CURRENT_STATE.PC_WRITE) {
      CURRENT_STATE.PC = CURRENT_STATE.IF_PC;
    }
  }
  // fill IF_ID_latch
  // base
	CURRENT_STATE.IF_ID_INST;
  // stay same ID_Stage inst if stall
	CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.ID_LOAD_USE_HAZARD ? CURRENT_STATE.PIPE[ID_STAGE] : CURRENT_STATE.PIPE[IF_STAGE]; 
  // disable flush after flushed
  if (CURRENT_STATE.IF_FLUSH) {
    CURRENT_STATE.IF_FLUSH = FALSE;
    CURRENT_STATE.ID_FLUSH = FALSE;
    CURRENT_STATE.EX_FLUSH = FALSE;
  }
  return;
}

// ID/EX latch helper
void setup_ID_EX_control(unsigned char REG_DST, unsigned char ALU_OP, unsigned char ALU_SRC, unsigned char MEM_BRCH, 
                  unsigned char MEM_READ, unsigned char MEM_WRITE, unsigned char REG_WRITE, unsigned char MEM_TO_REG) {
  CURRENT_STATE.ID_EX_REG_DST = REG_DST;
  CURRENT_STATE.ID_EX_ALU_OP = ALU_OP;
  CURRENT_STATE.ID_EX_ALU_SRC = ALU_SRC;
  CURRENT_STATE.ID_EX_MEM_BRCH = MEM_BRCH;
  CURRENT_STATE.ID_EX_MEM_READ = MEM_READ;
  CURRENT_STATE.ID_EX_MEM_WRITE = MEM_WRITE;
  CURRENT_STATE.ID_EX_REG_WRITE = REG_WRITE;
  CURRENT_STATE.ID_EX_MEM_TO_REG = MEM_TO_REG;
  return;
}
void ID_Stage() {
  instruction *inst = NULL;
  uint32_t dest;
  CURRENT_STATE.PIPE[ID_STAGE] = CURRENT_STATE.ID_FLUSH ? 0 : CURRENT_STATE.IF_ID_NPC;

  // disable jump after stall
  if (CURRENT_STATE.IF_JUMP) {
    CURRENT_STATE.IF_JUMP = FALSE;
    CURRENT_STATE.PIPE[ID_STAGE] = 0;
  }

  // setup hazard flags to default
  CURRENT_STATE.ID_LOAD_USE_HAZARD = FALSE;

  // If ID is fetched
  if (CURRENT_STATE.PIPE[ID_STAGE] > 0) {
    inst = get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]);

    // check load-use hazard
    unsigned char lu_rs = FALSE, lu_rt = FALSE;
    if (CURRENT_STATE.ID_EX_MEM_READ) {
      // stall the pipeline, set this ID_Stage to noop, IF_Stage stay the same
      if (CURRENT_STATE.ID_EX_REG_RT == RS(inst)) {
        CURRENT_STATE.ID_LOAD_USE_HAZARD = TRUE;
        lu_rs = TRUE;
        if(debug)printf("Load-use hazard entered, %d == (%d, %d)\n", CURRENT_STATE.ID_EX_REG_RT, RS(inst), RT(inst));///test
      }
      if (CURRENT_STATE.ID_EX_REG_RT == RT(inst)) {
        CURRENT_STATE.ID_LOAD_USE_HAZARD = TRUE;
        lu_rt = TRUE;
        if(debug)printf("Load-use hazard entered, %d == (%d, %d)\n", CURRENT_STATE.ID_EX_REG_RT, RS(inst), RT(inst));///test
      }
    }
    // Fill in ID/EX latch
    CURRENT_STATE.ID_EX_REG_RT = RT(inst);
    CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS(inst)];
    CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT(inst)];
    CURRENT_STATE.ID_EX_IMM = IMM(inst);

    // Set default values for handling jump and branch
    CURRENT_STATE.IF_JUMP = FALSE;
    CURRENT_STATE.JUMP_PC = 0;

    // Handle control unit
    // Change ID_EX control unit
    switch (OPCODE(inst)) {
      case 0x00://R, JR instruction
        if (FUNC(inst) == 0x08) { //JR
          setup_ID_EX_control(TRUE, 3, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
          CURRENT_STATE.IF_JUMP = TRUE;
          CURRENT_STATE.JUMP_PC = CURRENT_STATE.REGS[31];
        }
        else {  // ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU
          setup_ID_EX_control(TRUE, 3, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE);
        }
        break;
      
      case 0x02://J
        setup_ID_EX_control(TRUE, 0, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
        CURRENT_STATE.IF_JUMP = TRUE;
        CURRENT_STATE.JUMP_PC = (CURRENT_STATE.IF_ID_NPC & 0xf0000000) | (TARGET(inst) * BYTES_PER_WORD);
        break;
      
      case 0x03://JAL
        setup_ID_EX_control(TRUE, 0, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
        CURRENT_STATE.IF_JUMP = TRUE;
        CURRENT_STATE.JUMP_PC = (CURRENT_STATE.IF_ID_NPC & 0xf0000000) | (TARGET(inst) * BYTES_PER_WORD);
        CURRENT_STATE.REGS[31] = CURRENT_STATE.IF_ID_NPC + BYTES_PER_WORD;
        if(debug)printf("JAL entered, jump to 0x%x and saved 0x%x to $31\n", CURRENT_STATE.JUMP_PC, CURRENT_STATE.REGS[31]);///test
        break;
      //I Instructions
      case 0x09://ADDIU
        setup_ID_EX_control(FALSE, 4, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE);
        break;
      case 0x0c://ANDI
        setup_ID_EX_control(FALSE, 5, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE);
        break;
      case 0x04://BEQ
        setup_ID_EX_control(FALSE, 1, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE);
        break;
      case 0x05://BNE
        setup_ID_EX_control(FALSE, 2, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE);
        break;
      case 0x0f://LUI
        setup_ID_EX_control(FALSE, 8, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE);
        break;
      case 0x23://LW
        setup_ID_EX_control(FALSE, 0, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE);
        break;
      case 0x0d://ORI
        setup_ID_EX_control(FALSE, 6, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE);
        break;
      case 0x0b://SLTIU
        setup_ID_EX_control(FALSE, 7, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE);
        break;
      case 0x2b://SW
        setup_ID_EX_control(FALSE, 0, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE);
        break;
    }
    dest = CURRENT_STATE.ID_EX_REG_DST ? RD(inst) : RT(inst);

    
  }

  // fill ID/EX latch
  // base
  CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.ID_LOAD_USE_HAZARD ? 0 : CURRENT_STATE.PIPE[ID_STAGE];
	CURRENT_STATE.ID_EX_DEST = dest;
  // control
  CURRENT_STATE.ID_EX_REG_DST;
  CURRENT_STATE.ID_EX_ALU_OP;
  CURRENT_STATE.ID_EX_ALU_SRC;
  CURRENT_STATE.ID_EX_MEM_BRCH;
  CURRENT_STATE.ID_EX_MEM_READ;
  CURRENT_STATE.ID_EX_MEM_WRITE;
  CURRENT_STATE.ID_EX_REG_WRITE;
  CURRENT_STATE.ID_EX_MEM_TO_REG;
  return;
}

void EX_Stage() {
  instruction *inst = NULL;
  CURRENT_STATE.PIPE[EX_STAGE] = CURRENT_STATE.EX_FLUSH ? 0 : CURRENT_STATE.ID_EX_NPC;
  inst = get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]);
  uint32_t alu_out;
  if(debug)printf("EX %d\n", (CURRENT_STATE.PIPE[EX_STAGE]- MEM_TEXT_START) >> 2);///test

  // set branch to default
  CURRENT_STATE.EX_MEM_BR_TAKE = FALSE;

  // setup forward value and reg num for EX forwarding
  CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
  CURRENT_STATE.EX_MEM_FORWARD_REG = CURRENT_STATE.EX_MEM_DEST;

  if (CURRENT_STATE.PIPE[EX_STAGE] > 0) {
    // check forwarding and feed EX_MEM_FORWARD_VALUE or MEM_WB_FORWARD_VALUE to EX stage if forwarding is set
    // EX forward
    unsigned char mute_mem_forward_a = FALSE, mute_mem_forward_b = FALSE;
    if(debug)printf("Checking EX forward, %d, %d, %d, %d\n",CURRENT_STATE.EX_MEM_REG_WRITE, CURRENT_STATE.EX_MEM_FORWARD_REG, RS(inst), RT(inst));///test
    if (CURRENT_STATE.PIPE[MEM_STAGE] > 0 && CURRENT_STATE.EX_MEM_REG_WRITE && CURRENT_STATE.EX_MEM_FORWARD_REG != 0) {
      if (CURRENT_STATE.EX_MEM_FORWARD_REG == RS(inst)) {
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
        mute_mem_forward_a = TRUE;
        if(debug)printf("EX forward a, 0x%x on %d\n", CURRENT_STATE.EX_MEM_FORWARD_VALUE, CURRENT_STATE.EX_MEM_FORWARD_REG);///test
      }
      if (CURRENT_STATE.EX_MEM_FORWARD_REG == RT(inst)) {
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
        mute_mem_forward_b = TRUE;
        if(debug)printf("EX forward b, 0x%x on %d\n", CURRENT_STATE.EX_MEM_FORWARD_VALUE, CURRENT_STATE.EX_MEM_FORWARD_REG);///test
      }
    }
    // MEM forward
    if(debug)printf("Checking MEM forward, %d, %d, %d, %d\n", CURRENT_STATE.MEM_WB_REG_WRITE, CURRENT_STATE.MEM_WB_FORWARD_REG, RS(inst), RT(inst));///test
    if (CURRENT_STATE.PIPE[WB_STAGE] > 0 && CURRENT_STATE.MEM_WB_REG_WRITE && CURRENT_STATE.MEM_WB_FORWARD_REG != 0) {
      if (CURRENT_STATE.MEM_WB_FORWARD_REG == RS(inst) && !mute_mem_forward_a) {
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
        if(debug)printf("MEM forward a, 0x%x on %d\n", CURRENT_STATE.MEM_WB_FORWARD_VALUE, CURRENT_STATE.MEM_WB_FORWARD_REG);///test
      }
      else if (CURRENT_STATE.MEM_WB_FORWARD_REG == RT(inst) && !mute_mem_forward_b) {
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
        if(debug)printf("MEM forward b, 0x%x on %d\n", CURRENT_STATE.MEM_WB_FORWARD_VALUE, CURRENT_STATE.MEM_WB_FORWARD_REG);///test
      }
    }

    // alu_out calculation and control unit setup
    uint32_t src1, src2;
    uint32_t funct, shamt;
    src1 = CURRENT_STATE.ID_EX_REG1;
    src2 = CURRENT_STATE.ID_EX_ALU_SRC ? CURRENT_STATE.ID_EX_IMM : CURRENT_STATE.ID_EX_REG2;
    switch (CURRENT_STATE.ID_EX_ALU_OP)
    {
      case 0://LW, SW
        alu_out = src1 + src2;
        if(debug)printf("EX LW/SW entered, 0x%x = 0x%x + %x\n", alu_out, src1, src2);///test
        break;
      case 1://BEQ
        alu_out = src1 - src2;
        if(debug)printf("EX beq entered, %d\n", alu_out);///test
        if (alu_out == 0) {
          CURRENT_STATE.EX_MEM_BR_TAKE = TRUE;
          CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.PIPE[EX_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2);
          printf("EX beq taken, %x\n", CURRENT_STATE.EX_MEM_BR_TARGET);///test
        }
        break;
      case 2://BNE
        alu_out = src1 - src2;
        if(debug)printf("EX bne entered, %d\n", alu_out);///test
        if (alu_out != 0) {
          CURRENT_STATE.EX_MEM_BR_TAKE = TRUE;
          CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.PIPE[EX_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2);
          if(debug)printf("EX bne taken, 0x%x\n", CURRENT_STATE.EX_MEM_BR_TARGET);///test
        }
        break;
      case 3://R
        funct = FUNC(inst);
        shamt = SHAMT(inst);
        switch (funct) {
        case 0x21://ADDU
          alu_out = src1 + src2;
          break;
        case 0x24://AND
          alu_out = src1 & src2;
          if(debug)printf("EX and entered, %d\n", alu_out);///test
          break;
        case 0x27://NOR
          alu_out = ~(src1 | src2);
          break;
        case 0x25://OR
          alu_out = src1 | src2;
          break;
        case 0x2b://SLTU
          alu_out = src1 < src2 ? TRUE : FALSE;
          break;
        case 0x23://SUBU
          alu_out = src1 - src2;
          break;
        case 0x00://SLL
          alu_out = src2 << shamt;
          if(debug)printf("EX sll entered, %d = %d << %d\n", alu_out, src2, shamt);///test
          break;
        case 0x02://SRL
          alu_out = src2 >> shamt;
          break;
        }
        break;
    case 4://ADDIU
        alu_out = src1 + src2;
        break;
    case 5://ANDI
        alu_out = src1 & src2;
        break;
    case 6://ORI
        alu_out = src1 | (src2 & IMM_MAX);
        if(debug)printf("EX ori entered, 0x%x, 0x%x, 0x%x\n", src1, src2 & IMM_MAX, alu_out);///test
        break;
    case 7://SLTIU
        alu_out = src1 < src2 ? TRUE : FALSE;
        break;
    case 8://LUI
        alu_out = src2 << 16;
        if(debug)printf("EX lui entered, 0x%x\n", alu_out);///test
        break;
    }
  }

  // fill EX_MEM_latch
  // base
	CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.PIPE[EX_STAGE];
	CURRENT_STATE.EX_MEM_ALU_OUT = alu_out;
	CURRENT_STATE.EX_MEM_W_VALUE;
	CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_DEST;
  // control
  CURRENT_STATE.EX_MEM_MEM_READ = CURRENT_STATE.ID_EX_MEM_READ;
  CURRENT_STATE.EX_MEM_MEM_WRITE = CURRENT_STATE.ID_EX_MEM_WRITE;
  CURRENT_STATE.EX_MEM_REG_WRITE = CURRENT_STATE.ID_EX_REG_WRITE;
  CURRENT_STATE.EX_MEM_MEM_TO_REG = CURRENT_STATE.ID_EX_MEM_TO_REG;
  return;
}

void MEM_Stage() {
  // fetch pc from npc
  CURRENT_STATE.PIPE[MEM_STAGE] = CURRENT_STATE.EX_MEM_NPC;
  uint32_t read_data;

  if (CURRENT_STATE.PIPE[MEM_STAGE] > 0) {
  
    // flush if Branch Taken
    if (CURRENT_STATE.EX_MEM_BR_TAKE) {
      if(debug)printf("MEM EX_MEM_BR_TAKE entered, flush set\n");///test
      CURRENT_STATE.IF_FLUSH = TRUE;
      CURRENT_STATE.ID_FLUSH = TRUE;
      CURRENT_STATE.EX_FLUSH = TRUE;
    }

    // write first and read later
    // mem write
    if (CURRENT_STATE.EX_MEM_MEM_WRITE) {
      if(debug)printf("MEM mem_write entered, writing %d on 0x%x\n", CURRENT_STATE.EX_MEM_W_VALUE, CURRENT_STATE.EX_MEM_ALU_OUT);///test
      mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.EX_MEM_W_VALUE);
    }
    // mem read
    if (CURRENT_STATE.EX_MEM_MEM_READ) {
      if(debug)printf("MEM mem_read entered, reading 0x%x\n", CURRENT_STATE.EX_MEM_ALU_OUT);///test
      read_data = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
    }
  } 
  // setup forward value and reg num for MEM forwarding
  // because MEM_Stage performed before EX_stage, need to save before it is done
  CURRENT_STATE.MEM_WB_FORWARD_VALUE = CURRENT_STATE.MEM_WB_MEM_TO_REG ? CURRENT_STATE.MEM_WB_MEM_OUT : CURRENT_STATE.MEM_WB_ALU_OUT;
  CURRENT_STATE.MEM_WB_FORWARD_REG = CURRENT_STATE.MEM_WB_DEST;

  // fill MEM_WB_latch
  // base
  CURRENT_STATE.MEM_WB_NPC = CURRENT_STATE.PIPE[MEM_STAGE];
  CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
  CURRENT_STATE.MEM_WB_MEM_OUT = read_data;
  CURRENT_STATE.MEM_WB_BR_TAKE = CURRENT_STATE.EX_MEM_BR_TAKE;
  CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
  // control
  CURRENT_STATE.MEM_WB_REG_WRITE = CURRENT_STATE.EX_MEM_REG_WRITE;
  CURRENT_STATE.MEM_WB_MEM_TO_REG = CURRENT_STATE.EX_MEM_MEM_TO_REG;
  return;
}

void WB_Stage() {
  // fetch NPC
  CURRENT_STATE.PIPE[WB_STAGE] = CURRENT_STATE.MEM_WB_NPC;
  // write back
  uint32_t write_data;
  if (CURRENT_STATE.PIPE[WB_STAGE] > 0) {
    if (CURRENT_STATE.MEM_WB_REG_WRITE) {
      // select data from mem
      if (CURRENT_STATE.MEM_WB_MEM_TO_REG) {
        write_data = CURRENT_STATE.MEM_WB_MEM_OUT;
      }
      // select alu out
      else {
        write_data = CURRENT_STATE.MEM_WB_ALU_OUT;
      }
      CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = write_data;
      if(debug)printf("WB REG_WRITE set, writing 0x%x on %d\n", write_data, CURRENT_STATE.MEM_WB_DEST);///test
    }

    // only count actual instructions done
    INSTRUCTION_COUNT++;
  }
  return;
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	/** Your implementation here */
  WB_Stage();
	MEM_Stage();
	EX_Stage();
	ID_Stage();
	IF_Stage();
  
  // If there's leftover instructions on pipe, keep running
  if (!FETCH_BIT) {
    RUN_BIT = FALSE;
    for(int i = 0; i < PIPE_STAGE-1; i++) {
      if (CURRENT_STATE.PIPE[i] != 0) {
          RUN_BIT = TRUE;
      }
    }
  }
  return;
}
