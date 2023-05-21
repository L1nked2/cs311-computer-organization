# Project 3. MIPS Pipelined Simulator
Skeleton developed by CMU,
modified for KAIST CS311 purpose by THKIM, BKKIM and SHJEON.

## Instructions
There are three files you may modify: `util.h`, `run.h`, and `run.c`.

### 1. util.h

We have setup the basic CPU\_State that is sufficient to implement the project.
However, you may decide to add more variables, and modify/remove any misleading variables.

/***********************************/
/*   Control Unit                  */
/***********************************/
// PC
unsigned char PC_SRC; /* Choose NPC */ -> make next PC to branch_PC if it is TRUE
//To choose right PC
uint32_t IF_PC; -> next PC
uint32_t JUMP_PC; -> target PC of jump(j, jal, jr)
uint32_t BRANCH_PC; -> target PC of branch(bne, beq)
unsigned char PC_WRITE; /* Check if PC is writable */ -> change PC if it is TRUE
// ID/EX
unsigned char ID_EX_REG_DST; /* determines src of reg1, select RD if TRUE, select RT if FALSE*/
unsigned char ID_EX_ALU_OP; -> distinguish the category of insruction to use ALU appropriately(+, -, &, |, <<, >>, etc,.)
unsigned char ID_EX_ALU_SRC; /* determines src of reg2, select IMM if TRUE, select RT if FALSE*/
unsigned char ID_EX_MEM_BRCH; -> mark as branch
unsigned char ID_EX_MEM_READ; -> mark as inst that reading memory (lw)
unsigned char ID_EX_MEM_WRITE; -> mark as inst that writing memory (sw)
unsigned char ID_EX_REG_WRITE; -> mark as inst that writing register
unsigned char ID_EX_MEM_TO_REG; -> -> mark as inst that writing register with data from memory (lw)

from EX ~ WB, control unit just falls through
// EX/MEM
unsigned char EX_MEM_MEM_READ;
unsigned char EX_MEM_MEM_WRITE;
unsigned char EX_MEM_REG_WRITE;
unsigned char EX_MEM_MEM_TO_REG;
// MEM/WB
unsigned char MEM_WB_REG_WRITE;
unsigned char MEM_WB_MEM_TO_REG;

/***********************************/
/*   Hazard Unit                   */
/***********************************/
// Check if it is okay to proceed PC to NPC
unsigned char IF_ID_WRITE; -> used to handle load-use hazard
// Check if each stage needs flush or jump
unsigned char IF_FLUSH;
unsigned char ID_FLUSH;
unsigned char EX_FLUSH;
-> flush IF ~ EX if branch taken
unsigned char IF_JUMP;
-> stall IF if jump occurred
unsigned char ID_LOAD_USE_HAZARD; -> used to handle load-use hazard

/***********************************/
/*   Pipeline Latch                */
/***********************************/
//IF_ID_latch
uint32_t IF_ID_INST;
uint32_t IF_ID_NPC;

//ID_EX_latch
uint32_t ID_EX_NPC;
unsigned char ID_EX_REG_RT; -> reg num of RT
uint32_t ID_EX_REG1; /* value of RS */
uint32_t ID_EX_REG2; /* value of RT */
short ID_EX_IMM; /* value of IMM */
unsigned char ID_EX_DEST; /* reg num of RT or RD depending on inst */

//EX_MEM_latch
uint32_t EX_MEM_NPC;
uint32_t EX_MEM_ALU_OUT; -> ALU output
uint32_t EX_MEM_W_VALUE; -> value to write to memory
uint32_t EX_MEM_BR_TARGET; -> target address of branch instruction
uint32_t EX_MEM_BR_TAKE; -> marks if branch taken
unsigned char EX_MEM_DEST; /* reg num of RT or RD depending on inst */

//MEM_WB_latch
uint32_t MEM_WB_NPC;
uint32_t MEM_WB_ALU_OUT; -> ALU output
uint32_t MEM_WB_MEM_OUT; -> value read from memory
uint32_t MEM_WB_BR_TAKE; -> marks if branch taken
unsigned char MEM_WB_DEST; /* reg num to write(rd) */

/***********************************/
/*   Forwarding Unit               */
/***********************************/
unsigned char EX_MEM_FORWARD_REG;
unsigned char MEM_WB_FORWARD_REG;
uint32_t EX_MEM_FORWARD_VALUE;
uint32_t MEM_WB_FORWARD_VALUE;

### 2. run.h

You may add any additional functions that will be called by your implementation of `process_instruction()`.
In fact, we encourage you to split your implementation of `process_instruction()` into many other helping functions.
You may decide to have functions for each stages of the pipeline.
Function(s) to handle flushes (adding bubbles into the pipeline), etc.

### 3. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` function to simulate a `cycle` of the pipelined simulator.
Each `cycle()` the pipeline will advance to the next instruction (if there are no stalls/hazards, etc.).
Your internal register, memory, and pipeline register state should be updated according to the instruction
that is being executed at each stage.
