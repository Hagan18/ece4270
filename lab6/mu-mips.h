#include <stdint.h>

#define FALSE 0
#define TRUE  1

/******************************************************************************/
/* MIPS memory layout                                                                                                                                      */
/******************************************************************************/
#define MEM_TEXT_BEGIN  0x00400000
#define MEM_TEXT_END      0x0FFFFFFF
/*Memory address 0x10000000 to 0x1000FFFF access by $gp*/
#define MEM_DATA_BEGIN  0x10010000
#define MEM_DATA_END   0x7FFFFFFF

#define MEM_KTEXT_BEGIN 0x80000000
#define MEM_KTEXT_END  0x8FFFFFFF

#define MEM_KDATA_BEGIN 0x90000000
#define MEM_KDATA_END  0xFFFEFFFF

/*stack and data segments occupy the same memory space. Stack grows backward (from higher address to lower address) */
#define MEM_STACK_BEGIN 0x7FFFFFFF
#define MEM_STACK_END  0x10010000

typedef struct {
	uint32_t begin, end;
	uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
	{ MEM_TEXT_BEGIN, MEM_TEXT_END, NULL },
	{ MEM_DATA_BEGIN, MEM_DATA_END, NULL },
	{ MEM_KDATA_BEGIN, MEM_KDATA_END, NULL },
	{ MEM_KTEXT_BEGIN, MEM_KTEXT_END, NULL }
};

#define NUM_MEM_REGION 4
#define MIPS_REGS 32

typedef struct CPU_State_Struct {

  uint32_t PC;		                   /* program counter */
  uint32_t REGS[MIPS_REGS]; 		   /* register file. */
  uint32_t HI, LO;                     /* special regs for mult/div. */
} CPU_State;

typedef struct CPU_Pipeline_Reg_Struct{
	uint32_t PC;
	uint32_t IR;
	uint32_t A;
	uint32_t B;
	uint32_t imm;
	uint32_t ALUOutput;
	uint32_t ALUOutput2;
	uint32_t LMD;
	uint32_t instruction_type;
	//uint32_t RegWrite;	//This is used when writing to a register (1 = write, 0 = not writing)
	uint32_t RegisterRs;
	uint32_t RegisterRt;
	uint32_t RegisterRd;
	uint32_t rt;
	uint32_t rd;
	
	/****	Control		****/
	uint32_t RegWrite;	//0=don't write | 1=write
	uint32_t ALUSrc;	//0=immediate | 1=reg | 2=lo | 3=hi
	uint32_t RegDst;	//0=rd | 1=rt | 2=mult/div | 2 = mult or div (TA)
	uint32_t ALUOp;		//0=load/store | 1=ALU operation | 2 - branch instruciton
	uint32_t MemRead;	//0=don't read | 1=read
	uint32_t MemWrite;	//0=don't write | 1=write
	uint32_t MemToReg;	//0=from ALU | 1=from memory
	uint32_t WBH; // 0 = W; 1 = H ; 2 = B
	uint32_t Branch; //0 branch not taken|1 branch taken
	
	
} CPU_Pipeline_Reg;
int branch_jump;
/***************************************************************/
/* CPU State info.                                                                                                               */
/***************************************************************/

CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_FLAG;	/* run flag*/
uint32_t INSTRUCTION_COUNT;
uint32_t CYCLE_COUNT;
uint32_t PROGRAM_SIZE; /*in words*/


/***************************************************************/
/* Pipeline Registers.                                                                                                        */
/***************************************************************/
CPU_Pipeline_Reg IF_ID;
CPU_Pipeline_Reg ID_EX;
CPU_Pipeline_Reg EX_MEM;
CPU_Pipeline_Reg MEM_WB;

char prog_file[32];
int flag;
int SYSCALL_FLAG = 0;
int FORWARD_FLAG_A = 0;// 1 means a forward is coming from mem stage 10 means from ex stage
int FORWARD_FLAG_B = 0;// 1 means a forward is coming from mem stage 10 means from ex stage
int stall = 0; //0 means no stall, anything above that there is a stall]
int stallNum = 0;
int numLoad;
int numStore;
/***************************************************************/
/* Function Declerations.                                                                                                */
/***************************************************************/
void help();
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run(int num_cycles);
void runAll();
void mdump(uint32_t start, uint32_t stop) ;
void rdump();
void handle_command();
void reset();
void init_memory();
void load_program();
void handle_pipeline(); /*IMPLEMENT THIS*/
void WB();/*IMPLEMENT THIS*/
void MEM();/*IMPLEMENT THIS*/
void EX();/*IMPLEMENT THIS*/
void ID();/*IMPLEMENT THIS*/
void IF();/*IMPLEMENT THIS*/
void show_pipeline();/*IMPLEMENT THIS*/
void initialize();
void print_program(); /*IMPLEMENT THIS*/
void execute_instruction(uint32_t instruction, int execute_flag);
void print_instruction(uint32_t addr);
void writeToBuffer(uint32_t *buffer, int tag, int index);



