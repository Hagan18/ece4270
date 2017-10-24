#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"
int ENABLE_FORWARDING;

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	//CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
                case 'f':
		case 'F':
                        printf("Enter 1 for forwarding\nEnter 0 for no forwarding.\n");
			if(scanf("%d", &ENABLE_FORWARDING) != 1){
				break;
			}
			ENABLE_FORWARDING ==0 ? printf("Forwarding OFF\n") : printf("Forwarding ON\n");
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()//TA told us to update current state not next state, but updating current state, we never moved through the program
{

	if (EX_MEM.RegDst == 1 && EX_MEM.ALUOp == 0 && EX_MEM.ALUSrc == 1 && EX_MEM.MemRead == 1 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 1) {
			CURRENT_STATE.REGS[MEM_WB.RegisterRt] = MEM_WB.LMD;
	}
	//R-format
	if (EX_MEM.ALUOp == 1 && EX_MEM.ALUSrc == 1 && EX_MEM.MemRead == 0 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 0){
		if(EX_MEM.RegDst == 1){//rt
		//don't think this will ever hit
			CURRENT_STATE.REGS[MEM_WB.RegisterRt] = MEM_WB.ALUOp;
		}
		else if (EX_MEM.RegDst == 0){//rd
			CURRENT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOp;
		}
		else if (EX_MEM.RegDst == 2){//mult divide
			CURRENT_STATE.LO = MEM_WB.ALUOutput;
			CURRENT_STATE.HI = MEM_WB.ALUOutput2;
		}
	}
	//I-type
	if (EX_MEM.ALUOp == 1 && EX_MEM.ALUSrc == 0 && EX_MEM.MemRead == 0 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 0){
		if(EX_MEM.RegDst == 1){
			CURRENT_STATE.REGS[MEM_WB.RegisterRt] = MEM_WB.ALUOp;
		}
		else if (EX_MEM.RegDst == 0){
			CURRENT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOp;
		}
	}
	//mtlo
	if (EX_MEM.ALUOp == 1 && EX_MEM.ALUSrc == 2 && EX_MEM.MemRead == 0 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 0){
		CURRENT_STATE.LO = MEM_WB.ALUOutput;
	}
	
	//mthi
	if (EX_MEM.ALUOp == 1 && EX_MEM.ALUSrc == 2 && EX_MEM.MemRead == 0 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 0){
		CURRENT_STATE.HI = MEM_WB.ALUOutput2;
	}
	
	//syscall
	if (MEM_WB.instruction_type == 1){
		if(MEM_WB.A == 0xa){
		RUN_FLAG = FALSE;
		//(CURRENT_STATE.PC);
		}
		else
			printf("SYSCALL error\n");		
	}
	
	INSTRUCTION_COUNT++;
	

	
	
/*	if (MEM_WB.instruction_type == 0 || EX_MEM.instruction_type == 7 || EX_MEM.instruction_type == 8){//load
		
		NEXT_STATE.REGS[MEM_WB.C] = MEM_WB.LMD;
	}
	
	
	//store doesn't do anything in WB
	else if (MEM_WB.instruction_type == 2){ //register
		
		NEXT_STATE.REGS[MEM_WB.C] = MEM_WB.ALUOutput;
		
	}
	else if (MEM_WB.instruction_type == 3){//MULT DIV
		
		NEXT_STATE.HI = MEM_WB.ALUOutput2;
		CURRENT_STATE.LO = MEM_WB.ALUOutput;
	}
	else if (MEM_WB.instruction_type == 4){//MTLO/MTHI
		
		NEXT_STATE.LO = MEM_WB.ALUOutput;
	}
	else if (MEM_WB.instruction_type == 5){
		
		NEXT_STATE.HI = MEM_WB.ALUOutput2;
	}
	else if (MEM_WB.instruction_type == 6){
		
		if(MEM_WB.A == 0xa){
			RUN_FLAG = FALSE;
			//(CURRENT_STATE.PC);
		}
		else
			printf("SYSCALL error\n");		
	}
*/
//	INSTRUCTION_COUNT++;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	//load
	if (EX_MEM.RegDst == 1 && EX_MEM.ALUOp == 0 && EX_MEM.ALUSrc == 1 && EX_MEM.MemRead == 1 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 1 && EX_MEM.MemToReg == 1)  {//load
		
		MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
		if (EX_MEM.WBH == 1){//LH
			MEM_WB.LMD = ((MEM_WB.LMD & 0x0000FFFF) & 0x8000) > 0 ? (MEM_WB.LMD | 0xFFFF0000) : (MEM_WB.LMD & 0x0000FFFF);
		}
		else if (EX_MEM.WBH == 2){ //LB
			MEM_WB.LMD = ((MEM_WB.LMD & 0x000000FF) & 0x80) > 0 ? (MEM_WB.LMD | 0xFFFFFF00) : (MEM_WB.LMD & 0x000000FF);
		}
	}
	
	if (EX_MEM.ALUOp == 0 && EX_MEM.ALUSrc == 1 && EX_MEM.MemRead == 0 && EX_MEM.MemWrite == 0 && EX_MEM.RegWrite == 0 ){//store
		mem_write_32(EX_MEM.ALUOutput, EX_MEM.A);
	}
	
	
	
	/*IMPLEMENT THIS*/
/*	//MEM_WB.IR = EX_MEM.IR;
	if (EX_MEM.instruction_type == 0 || EX_MEM.instruction_type == 7 || EX_MEM.instruction_type == 8){
		//load instruction
		MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
		if (EX_MEM.instruction_type == 7){//LH
			MEM_WB.LMD = ((MEM_WB.LMD & 0x0000FFFF) & 0x8000) > 0 ? (MEM_WB.LMD | 0xFFFF0000) : (MEM_WB.LMD & 0x0000FFFF);
		}
		else if (EX_MEM.instruction_type == 8){ //LB
			MEM_WB.LMD = ((MEM_WB.LMD & 0x000000FF) & 0x80) > 0 ? (MEM_WB.LMD | 0xFFFFFF00) : (MEM_WB.LMD & 0x000000FF);
		}
		
		
	}
	else if (EX_MEM.instruction_type == 1){
		//store
		mem_write_32(EX_MEM.ALUOutput, EX_MEM.A);
	}
	*/

	//forward on the values to the next register
	//MEM_WB.instruction_type = EX_MEM.instruction_type;
	MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	MEM_WB.ALUOutput2 = EX_MEM.ALUOutput2;
	MEM_WB.A = EX_MEM.A;
	MEM_WB.B = EX_MEM.B;
	//MEM_WB.C = EX_MEM.C;
	MEM_WB.PC = EX_MEM.PC;	
	MEM_WB.IR = EX_MEM.IR;
	MEM_WB.RegDst = EX_MEM.RegDst;
	MEM_WB.ALUOp = EX_MEM.ALUOp;
	MEM_WB.MemRead = EX_MEM.MemRead;
	MEM_WB.MemWrite = EX_MEM.MemWrite;
	MEM_WB.MemToReg = EX_MEM.MemToReg;
	MEM_WB.WBH = EX_MEM.WBH;
	MEM_WB.RegisterRs = EX_MEM.RegisterRs;
	MEM_WB.RegisterRt = EX_MEM.RegisterRt;
	MEM_WB.RegisterRd = EX_MEM.RegisterRd;
	MEM_WB.rt = EX_MEM.rt;
	MEM_WB.rd = EX_MEM.rd;

    if(ENABLE_FORWARDING){
    	if((MEM_WB.RegWrite && (MEM_WB.RegisterRd != 0)) && (!(EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0) && (EX_MEM.RegisterRd == ID_EX.RegisterRs)) && (MEM_WB.RegisterRd == ID_EX.RegisterRs))){
            //forwardA=01
            
        }
        
        if((MEM_WB.RegWrite && (MEM_WB.RegisterRd != 0)) && (!(EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0) && (EX_MEM.RegisterRd == ID_EX.RegisterRt)) && (MEM_WB.RegisterRd == ID_EX.RegisterRt))){
            //forwardB=01
        }
    }
}

/************************************************************/
/* execution (EX) pipeline stage:                           */ 
/************************************************************/
void EX()
{
	EX_MEM.IR = ID_EX.IR;
	execute_instruction(ID_EX.IR, 1);
        
	//passing on all of the values on to the next stage
	EX_MEM.A = ID_EX.A;
	EX_MEM.B = ID_EX.B;
	// EX_MEM.C = ID_EX.C;
//	EX_MEM.instruction_type = ID_EX.instruction_type;
	EX_MEM.PC = ID_EX.PC;
	EX_MEM.RegWrite = ID_EX.RegWrite;//need to check to see if it is a load
	EX_MEM.RegDst = ID_EX.RegDst;
	EX_MEM.ALUOp = ID_EX.ALUOp;
	EX_MEM.MemRead = ID_EX.MemRead;
	EX_MEM.MemWrite = ID_EX.MemWrite;
	EX_MEM.MemToReg = ID_EX.MemToReg;
	EX_MEM.WBH = ID_EX.WBH;
	EX_MEM.RegisterRs = ID_EX.RegisterRs;
	EX_MEM.RegisterRt = ID_EX.RegisterRt;
	EX_MEM.RegisterRd = ID_EX.RegisterRd;
	EX_MEM.rt = ID_EX.rt;
	EX_MEM.rd = ID_EX.rd;
	
    if(ENABLE_FORWARDING){
        if((EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRs)){
            //forwardA = 10
        }
        if((EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRt)){
            //forwardB = 10
        }
    }
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                  */ 
/************************************************************/
void ID()
{
	/*IMPLEMENT THIS*/
	execute_instruction(IF_ID.IR, 0);
	if (ID_EX.instruction_type == 1){//syscall
		SYSCALL_FLAG = 1;
	}
	ID_EX.IR = IF_ID.IR;
	ID_EX.PC = IF_ID.PC;
    // if (ID_EX.instruction_type == 1){
    //     ID_EX.RegWrite = 0;
    // }
    // else{
    //     ID_EX.RegWrite = 1;
    // }
    
    
    if (!ENABLE_FORWARDING){
        if(((EX_MEM.RegWrite) && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRs)){
            //stall
            
        }
        if(((EX_MEM.RegWrite) && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRt)){
            //stall
        }
        if(((MEM_WB.RegWrite) && (MEM_WB.RegisterRd != 0)) && (MEM_WB.RegisterRd == ID_EX.RegisterRs)){
            //stall
        }
        if(((MEM_WB.RegWrite) && (MEM_WB.RegisterRd != 0)) && (MEM_WB.RegisterRd == ID_EX.RegisterRt)){
            //stall
        }
    }
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{

	/*IMPLEMENT THIS*/
	IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
	
	if (SYSCALL_FLAG == 1){//are not incrementing the program counter after there is a syscall
	}
	else{
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
		IF_ID.PC = NEXT_STATE.PC;
	}
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE; 
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
	
}

/***************************************X/MEM*********************/
/* Print the current pipeline                                    */ 
/*****************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
	printf("---------------------------------IF--------------------------------------\n");
	printf("IF_ID.PC: %x\n",IF_ID.PC);
	printf("IF_ID.IR: %x\n",IF_ID.IR);
	printf("IF_ID.A: %x\n",IF_ID.A);
	printf("IF_ID.B: %x\n",IF_ID.B);
	//printf("IF_ID.C: %lx\n",IF_ID.C);
	printf("IF_ID.imm: %x\n",IF_ID.imm);
	printf("IF_ID.ALUOutput: %x\n",IF_ID.ALUOutput);
	printf("IF_ID.ALUOutput2: %x\n",IF_ID.ALUOutput2);
	printf("---------------------------------ID--------------------------------------\n");
	printf("ID_EX.PC: %x\n",ID_EX.PC);
	printf("ID_EX.IR: %x\n",ID_EX.IR);
	printf("ID_EX.A: %x\n",ID_EX.A);
	printf("ID_EX.B: %x\n",ID_EX.B);
	//printf("ID_EX.C: %lx\n",ID_EX.C);
	printf("ID_EX.imm: %x\n",ID_EX.imm);
	printf("ID_EX.ALUOutput: %x\n",ID_EX.ALUOutput);
	printf("ID_EX.ALUOutput2: %x\n",ID_EX.ALUOutput2);
	printf("---------------------------------EX--------------------------------------\n");
	printf("EX_MEM.PC: %x\n",EX_MEM.PC);
	printf("EX_MEM.IR: %x\n",EX_MEM.IR);
	printf("EX_MEM.A: %x\n",EX_MEM.A);
	printf("EX_MEM.B: %x\n",EX_MEM.B);
	//printf("EX_MEM.C: %lx\n",EX_MEM.C);
	printf("EX_MEM.imm: %x\n",EX_MEM.imm);
	printf("EX_MEM.ALUOutput: %x\n",EX_MEM.ALUOutput);
	printf("EX_MEM.ALUOutput2: %x\n",EX_MEM.ALUOutput2);
	printf("---------------------------------MEM--------------------------------------\n");
	printf("MEM_WB.PC: %x\n",MEM_WB.PC);
	printf("MEM_WB.IR: %x\n",MEM_WB.IR);
	printf("MEM_WB.A: %x\n",MEM_WB.A);
	printf("MEM_WB.B: %x\n",MEM_WB.B);
	//printf("MEM_WB.C: %lx\n",MEM_WB.C);
	printf("MEM_WB.imm: %x\n",MEM_WB.imm);
	printf("MEM_WB.ALUOutput: %x\n",MEM_WB.ALUOutput);
	printf("MEM_WB.ALUOutput2: %x\n",MEM_WB.ALUOutput2);
}

/***************************************************************/
/* main                                                        */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}

long int signExtend(long int immediate){
    long int val = (immediate & 0x0000FFFF);
    long int mask = 0x00008000;
    if (mask & val) {
        val = val | 0xFFFF0000;
    }
    return val;
}

uint32_t convertInstruction(uint32_t value){
	uint32_t right = value << 26;
	right = right >> 26;
	uint32_t left = value >> 26;
	
        if ((left & 0x3F) != 0){
            flag=1;
            return left;
	}
	else if (((right & 0x3F) != 0) && ((left & 0x3F) == 0)){//I changed these from FF to 3F
		flag = 0;
		return right;
	}
        else if (right == 0){
            return right;
        }
        
        return 0xFF;
}

void execute_instruction(uint32_t instruction, int execute_flag){
	uint32_t binInstruction, data;
	uint32_t offset;
	uint64_t product;// p1, p2;
	long int rd, rt, rs, sa, base;

	binInstruction = instruction;
	instruction = convertInstruction(instruction);
	
                
	if(flag==0){ //If flag is set = instruction is 'left'
		
		switch(instruction){//case statement for right
			//ADD
			case 0x20:
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
					//ID_EX.instruction_type = 2;
					
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput =ID_EX.A + ID_EX.B;
					
				}
				else {printf("Execute_Flag error\n");}
				break;
			//ADDU		
			case 0x21:
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if(execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
				}
				else {printf("Execute_Flag error\n");}
				break;
			//AND 
			case 0x24:
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd =CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
					//ID_EX.instruction_type = 2;
				}
				else if(execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A & ID_EX.B;
				}
				else{printf("Execute_flag error\n");}
				break;
			//SUB 
			case 0x22: 
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if(execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A - ID_EX.B;
				}
				else{printf("Execute_flag error\n");}
				break;
				
			//SUBU 
			case 0x23:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					//NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]-CURRENT_STATE.REGS[rt];
					EX_MEM.ALUOutput = ID_EX.A - ID_EX.B;
				}
				else{
					printf("Execute_flag error\n");	
				}
				
				break;
		//MULT 
			case 0x18:
				if(execute_flag == 0){
					//int64_t tempMult;
					binInstruction = binInstruction >> 16;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegisterRd = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 2;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					uint32_t p1=0,p2=0;
					if ((ID_EX.A & 0x80000000) == 0x80000000){
						p1 = 0xFFFFFFFF00000000 | ID_EX.A;
					}
					else{
						p1 = 0x00000000FFFFFFFF & ID_EX.A;
					}
					if ((ID_EX.B & 0x80000000) == 0x80000000){
						p2 = 0xFFFFFFFF00000000 | ID_EX.B;
					}
					else{
						p2 = 0x00000000FFFFFFFF & ID_EX.B;
					}
					product = p1 * p2;
					// NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					// NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					EX_MEM.ALUOutput = (product & 0X00000000FFFFFFFF);			//Low
					EX_MEM.ALUOutput2 = (product & 0XFFFFFFFF00000000)>>32;		//High
					//print_instruction(CURRENT_STATE.PC);
				}
			
				else{
					printf("Execute_flag error\n");	
				}
				
				break;
			//MULTU needs work
			case 0x19:
				//uint64_t tempMultU;
				if(execute_flag == 0){
					binInstruction = binInstruction >> 16;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegisterRd = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 2;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				
				}
				else if (execute_flag == 1){
					product = (uint64_t)ID_EX.A * (uint64_t)ID_EX.B;
					EX_MEM.ALUOutput = (product & 0X00000000FFFFFFFF);
					EX_MEM.ALUOutput2 = (product & 0XFFFFFFFF00000000)>>32;
					//print_instruction(CURRENT_STATE.PC);
				}
				/*
					If either of the two preceding instructions is MFHI or MFLO, the results of
					these instructions are undefined. Correct operation requires separating
					reads of HI or LO from writes by a minimum of two other instructions.
				*/		
				else{
						printf("Execute_flag error\n");	
				}
				
				break;
			//DIV 
			case 0x1A:
				//int32_t tempDiv;
				
				if(execute_flag == 0){
					binInstruction = binInstruction >> 16;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 2;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if(execute_flag == 1){
					
						if(ID_EX.B != 0){
							EX_MEM.ALUOutput = ID_EX.A/ID_EX.B;
							EX_MEM.ALUOutput2 = ID_EX.A % ID_EX.B;
						}
						
				}
				else printf("Execute flag error\n");
				break;
			
			//DIVU 
			case 0x1B:
				if (execute_flag == 0){
					//uint32_t tempDivU;
					binInstruction = binInstruction >> 16;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 2;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
					
				}
				else if (execute_flag == 1){
					if(ID_EX.B != 0){
						EX_MEM.ALUOutput = ID_EX.A / ID_EX.B;
						EX_MEM.ALUOutput2 = ID_EX.A % ID_EX.B;
					}
				//print_instruction(CURRENT_STATE.PC);
				}
			
				break;
			//OR
			case 0x25:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd =CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A | ID_EX.B;
				}
				break;
			
			//NOR 
			case 0x27:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd =CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){	
					EX_MEM.ALUOutput = ~(ID_EX.A | ID_EX.B);
				}
				break;
			//XOR
			case 0x26:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd =CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A ^ ID_EX.B;//(CURRENT_STATE.REGS[rs]^CURRENT_STATE.REGS[rt]);
				}
				break;
			//SLT
			case 0x2A:
				
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					ID_EX.A = CURRENT_STATE.REGS[rs];
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd =CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					if(ID_EX.A < ID_EX.B){
						EX_MEM.ALUOutput = 0x1;
					}
					else{
						EX_MEM.ALUOutput  = 0x0;
					}
				//print_instruction(CURRENT_STATE.PC);
				}
				break;
			//SLL 
			case 0x0:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 6;
					sa = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rd = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x1F;
					ID_EX.A = sa;
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.B << ID_EX.A;
					//print_instruction(CURRENT_STATE.PC);
				}
				break;
			//SRA
			case 0x3:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 6;
					sa = binInstruction & 0x001F;
					binInstruction = binInstruction >> 5;
					rd = binInstruction & 0x01F;
					binInstruction = binInstruction >> 5;
					rt = binInstruction & 0x1F;
					ID_EX.A = sa;
					ID_EX.B = CURRENT_STATE.REGS[rt];
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.B >> ID_EX.A;
					//print_instruction(CURRENT_STATE.PC);
				}
				break;				
			
			//MFLO not checked
			case 0x12:
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x01F;
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = 0;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 2;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = CURRENT_STATE.LO;
				}
				break;
			//MFHI
			case 0x10:
				if(execute_flag == 0){
					binInstruction = binInstruction >> 11;
					rd = binInstruction & 0x01F;
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = 0;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 3;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput= CURRENT_STATE.HI;
				}
				else printf("Execute_flag error\n");	
				break;
//**************************************************************************************
			//MTHI
			case 0x11:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 21;
					rd = binInstruction & 0x01F;
					ID_EX.A = CURRENT_STATE.REGS[rd];
					ID_EX.instruction_type = 5;
					ID_EX.rd = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = 0;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 0;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput2 = ID_EX.A;//needs to go in high register
				}
				break;
			//MTLO
			case 0x13:
				if (execute_flag == 0){
					binInstruction = binInstruction >> 21;
					rd = binInstruction & 0x01F;
					ID_EX.A = CURRENT_STATE.REGS[rd];
					ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = 0;
					ID_EX.RegisterRs = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.A;
				}
				break;
//******************************************************************************************
	        //SRL
	        case 0x02:
	        	if (execute_flag == 0){
		            binInstruction = binInstruction >> 6;
		            sa = binInstruction & 0x01F;
		            binInstruction = binInstruction >> 5;
		            rd = binInstruction & 0x01F;
		            binInstruction = binInstruction >> 5;
		            rt = binInstruction & 0x01F;
		            ID_EX.A = sa;
		            ID_EX.B = CURRENT_STATE.REGS[rt];
		            ID_EX.rd = CURRENT_STATE.REGS[rd];
		            ID_EX.RegisterRd = rd;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
	        	}
	        	else if (execute_flag == 1){
	            	EX_MEM.ALUOutput = ID_EX.B >> ID_EX.A;
	        	}
	        	break;
			//SYSCALL
			case 0xC:
			//********************************************************************
				if (execute_flag == 0){
					ID_EX.A = CURRENT_STATE.REGS[2];
					SYSCALL_FLAG = 1;
				}
				else {
					ID_EX.instruction_type = 1;
				}
				/*else if (execute_flag == 1){
					if(ID_EX.A == 0xa){
						RUN_FLAG = FALSE;
						//(CURRENT_STATE.PC);
					}*/
				//}
				break;	
			//************************************************************************	
                default:
                    printf("default on right hit\n");
		}
	}
	else if ((flag)){
		long int immediate;
		switch(instruction){
			case 0x8: //ADDI
		        /* The 16-bit immediate is sign-extended and added to the contents of general
		            * register rs to form the result. The result is placed into general register rt.
		            * In 64-bit mode, the operand must be valid sign-extended, 32-bit values.
		            * An overflow exception occurs if carries out of bits 30 and 31 differ (2’s
		            * complement overflow). The destination register rt is not modified when
		            * an integer overflow exception occurs.
		        */
		        if(execute_flag == 0){
			        immediate = signExtend(binInstruction & 0x0000FFFF);
			        rt = (binInstruction >> 16) & 0x0000001F;
			        rs = (binInstruction >> 21) & 0x0000001F;
			        ID_EX.A = CURRENT_STATE.REGS[rs];
			        ID_EX.imm = CURRENT_STATE.REGS[immediate];
			        ID_EX.rt = CURRENT_STATE.REGS[rt];
			        ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
		        }
		        else if(execute_flag == 1){
			        EX_MEM.ALUOutput = ID_EX.imm + ID_EX.A;
			        //print_instruction(CURRENT_STATE.PC);
		        }
				// NOTE: Need to add overflow functionality!
			case 0x9: //ADDIU
                /* The 16-bit immediate is sign-extended and added to the contents of general
                    * register rs to form the result. The result is placed into general register rt.
                    * No integer overflow exception occurs under any circumstances. In 64-bit
                    * mode, the operand must be valid sign-extended, 32-bit values.
                    * The only difference between this instruction and the ADDI instruction is
                    * that ADDIU never causes an overflow exception.
                */
                	if(execute_flag == 0){
                    	rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
                    	rt =  (binInstruction >> 16) & 0x0000001F;
                    	immediate = signExtend((binInstruction & 0x0000FFFF));	//isolate 'immediate' and sign extend it
                	ID_EX.imm = immediate;
                	ID_EX.A = CURRENT_STATE.REGS[rs];
			        ID_EX.rt = CURRENT_STATE.REGS[rt];
			        ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
			
                	ID_EX.instruction_type = 2;
                }
                else if(execute_flag == 1){
			
            		EX_MEM.ALUOutput = ID_EX.imm + ID_EX.A;
                }
                break;
                
		case 0xC: //ANDI
            	if(execute_flag == 1){
	                rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
	                rt = (binInstruction >> 16) & 0x0000001F;
	                immediate = binInstruction & 0x0000FFFF;
	                ID_EX.A = CURRENT_STATE.REGS[rs];
	                ID_EX.imm = immediate;
			        ID_EX.rt = CURRENT_STATE.REGS[rt];
			        ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
            	}
            	else if (execute_flag == 1){
	                EX_MEM.ALUOutput = ID_EX.imm & ID_EX.A;
            	}
            	else printf("execute_flag error\n");
                break;
			case 0xD: //ORI
                if (execute_flag == 0){            
	                rs = (binInstruction >> 21) & 0x0000001F;
	                rt = (binInstruction >> 16) & 0x0000001F;
	                immediate = binInstruction & 0x0000FFFF;
	                ID_EX.A = CURRENT_STATE.REGS[rs];
	                ID_EX.imm = immediate;
			        ID_EX.rt = CURRENT_STATE.REGS[rt];
			        ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
                }
                else if (execute_flag == 1){
                	EX_MEM.ALUOutput = ID_EX.A | (ID_EX.imm);
                }
                else printf("execute flag error\n");
                break;
			case 0xE: //XORI
                if (execute_flag == 0){           
	                rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
	                rt = (binInstruction >> 16) & 0x0000001F;
	                immediate = binInstruction & 0x0000FFFF;
	                ID_EX.A = CURRENT_STATE.REGS[rs];
	                ID_EX.imm = immediate;
			        ID_EX.rt = CURRENT_STATE.REGS[rt];
			        ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
					ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
                }
                else if(execute_flag == 1){
                	EX_MEM.ALUOutput = ID_EX.A ^ ID_EX.imm;
                }
                else printf("execute flag error\n");
                break;

			case 0xA: //SLTI
				if (execute_flag == 0){
	                rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
	                rt = (binInstruction >> 16) & 0x0000001F;
	                immediate = (binInstruction & 0x000FFFF);		//isolate the contents of 'immediate'
	                ID_EX.A = CURRENT_STATE.REGS[rs];
	                ID_EX.rt = CURRENT_STATE.REGS[rt];
	                ID_EX.imm = immediate;
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
	                ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 0;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 1;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 0;
				}
				else if (execute_flag == 1){
					if ( (  (int32_t)ID_EX.A - (int32_t)( (ID_EX.imm & 0x8000) > 0 ? (ID_EX.imm | 0xFFFF0000) : (ID_EX.imm & 0x0000FFFF))) < 0){
						EX_MEM.ALUOutput = 0x1;
					}else{
						EX_MEM.ALUOutput = 0x0;
					}
				}
				else printf("Execute flag error\n");
                 break;

			case 0x23: //LW
				if (execute_flag == 0){
					rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs (in this case, base)
	                rt = (binInstruction >> 16) & 0x0000001F;               //isolate rt
	                immediate = (binInstruction & 0x000FFFF);		//isolate the contents of 'immediate'
                	ID_EX.imm = immediate;
	                ID_EX.rt =  CURRENT_STATE.REGS[rt]; 
	                ID_EX.B = CURRENT_STATE.REGS[rs];
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = rs;
	                ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 1;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 1;
					ID_EX.WBH = 0;
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput =   ID_EX.B  + ( (ID_EX.imm & 0x8000) > 0 ? (ID_EX.imm | 0xFFFF0000) : (ID_EX.imm & 0x0000FFFF));
					
				}
				else printf("execute flag error\n");
               
                break;

			case 0x20: //LB
				if(execute_flag == 0){
	                offset =  binInstruction & 0x0FFFF; 
	                binInstruction = binInstruction >> 16;
	                rt = binInstruction & 0x001F;
	                binInstruction = binInstruction >> 5;
	                base = binInstruction & 0x001F;
	                ID_EX.imm = offset;
	                // ID_EX.C = rt;
	                ID_EX.B = CURRENT_STATE.REGS[base];
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
	                 ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 1;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 1;
					ID_EX.WBH = 2;
	                
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput = ID_EX.B + ( ( ID_EX.imm  & 0x8000) > 0 ? ( ID_EX.imm  | 0xFFFF0000) : ( ID_EX.imm  & 0x0000FFFF)) ;
					//EX_MEM.ALUOutput = ((data & 0x000000FF) & 0x80) > 0 ? (data | 0xFFFFFF00) : (data & 0x000000FF);
				}
				else printf("execute flag error\n");
               
                break;

			case 0x21: //LH
				if (execute_flag == 0){
		            offset =  binInstruction & 0x0FFFF; 
		            binInstruction = binInstruction >> 16;
		            rt = binInstruction & 0x001F;
		            binInstruction = binInstruction >> 5;
		            base = binInstruction & 0x001F;
		            ID_EX.imm = offset;
	                // ID_EX.C = rt;
	                ID_EX.B = CURRENT_STATE.REGS[base];
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
	                ID_EX.RegWrite = 1;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 1;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 1;
					ID_EX.MemWrite = 0;
					ID_EX.MemToReg = 1;
					ID_EX.WBH = 1;
	                
				}
				else if (execute_flag == 1){
					EX_MEM.ALUOutput =  ID_EX.B + ( (ID_EX.imm & 0x8000) > 0 ? (ID_EX.imm | 0xFFFF0000) : (ID_EX.imm & 0x0000FFFF)) ;	
					
					
				}
	            else printf("execute flag error\n");
	           
	            break;

			case 0x2B: //SW
				if (execute_flag == 0){
	                offset =  binInstruction & 0x0FFFF; 
	                binInstruction = binInstruction >> 16;
	                rt = binInstruction & 0x001F;
	                binInstruction = binInstruction >> 5;
	                base = binInstruction & 0x001F;
	                ID_EX.imm = offset;
	                ID_EX.A = CURRENT_STATE.REGS[rt]; 
	                ID_EX.B = CURRENT_STATE.REGS[base];
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
	                ID_EX.RegWrite = 0;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 1;
					ID_EX.MemToReg = 0;
				}
             else if (execute_flag == 1){
             	
             	EX_MEM.ALUOutput = ID_EX.B + ( ( ID_EX.imm  & 0x8000) > 0 ? ( ID_EX.imm  | 0xFFFF0000) : ( ID_EX.imm  & 0x0000FFFF));
				//mem_write_32(addr, ID_EX.A);
             }
             else printf("execute_flag error\n");
        
                break;

			case 0x28: //SB
                           
        	if (execute_flag == 0){
        		offset =  binInstruction & 0x0FFFF; 
	            binInstruction = binInstruction >> 16;
	            rt = binInstruction & 0x001F;
	            binInstruction = binInstruction >> 5;
	            base = binInstruction & 0x001F;
	            ID_EX.imm = offset;
                ID_EX.A = CURRENT_STATE.REGS[rt]; 
                ID_EX.B = CURRENT_STATE.REGS[base];
            	ID_EX.RegisterRd = 0;
				ID_EX.RegisterRt = rt;
				ID_EX.RegisterRs = 0;
                ID_EX.RegWrite = 0;
				ID_EX.ALUSrc = 1;
				ID_EX.RegDst = 0;
				ID_EX.ALUOp = 0;
				ID_EX.MemRead = 0;
				ID_EX.MemWrite = 1;
				ID_EX.MemToReg = 0;
	            
        	}
        	else if (execute_flag == 1){
        		EX_MEM.ALUOutput = ID_EX.B + ( (ID_EX.imm & 0x8000) > 0 ? (ID_EX.imm | 0xFFFF0000) : (ID_EX.imm & 0x0000FFFF));
        		data = mem_read_32(EX_MEM.ALUOutput);
				ID_EX.A = (data & 0xFFFFFF00) | (ID_EX.A & 0x000000FF);//this is what needs to be written to the mem address 
				//mem_write_32(addr, data);
				//mem_write_32(addr, data);
        	}
        	else printf("execute flag error\n");

            break;

			case 0x29: //SH
                if (execute_flag == 0){            
	                offset =  binInstruction & 0x0FFFF; 
	                binInstruction = binInstruction >> 16;
	                rt = binInstruction & 0x001F;
	                binInstruction = binInstruction >> 5;
	                base = binInstruction & 0x001F;
	                ID_EX.imm = offset;
	                ID_EX.A = CURRENT_STATE.REGS[rt]; 
	                ID_EX.B = CURRENT_STATE.REGS[base];
	                ID_EX.RegisterRd = 0;
					ID_EX.RegisterRt = rt;
					ID_EX.RegisterRs = 0;
					ID_EX.RegWrite = 0;
					ID_EX.ALUSrc = 1;
					ID_EX.RegDst = 0;
					ID_EX.ALUOp = 0;
					ID_EX.MemRead = 0;
					ID_EX.MemWrite = 1;
					ID_EX.MemToReg = 0;
                }
                else if (execute_flag == 1){
                	
                	EX_MEM.ALUOutput = ID_EX.B + ( (ID_EX.imm & 0x8000) > 0 ? (ID_EX.imm | 0xFFFF0000) : (ID_EX.imm & 0x0000FFFF));
					data = mem_read_32(EX_MEM.ALUOutput);
					ID_EX.A = (data & 0xFFFF0000) | (ID_EX.A & 0x0000FFFF);
					//mem_write_32(addr, data);
					//print_instruction(CURRENT_STATE.PC);
                	
                }
                else printf("execute flag error\n");
               
                break;
                

			//LUI
                case 0x0F: 
                    if (execute_flag == 0){
	                    immediate = binInstruction & 0x0FFFF;//isolate immediate
	                    binInstruction = binInstruction >> 16;
	                    rt = binInstruction & 0x1F;//isolate rt
	                    ID_EX.imm = immediate;
	                    //ID_EX.C = rt;
	                    ID_EX.instruction_type = 2;
	                    ID_EX.rt = CURRENT_STATE.REGS[rt];
						ID_EX.RegisterRd = 0;
						ID_EX.RegisterRt = rt;
						ID_EX.RegisterRs = 0;
						ID_EX.RegWrite = 1;
						ID_EX.ALUSrc = 0;
						ID_EX.RegDst = 1;
						ID_EX.ALUOp = 1;
						ID_EX.MemRead = 0;
						ID_EX.MemWrite = 0;
						ID_EX.MemToReg = 0;
                    }
                    else if (execute_flag == 1){
                    	EX_MEM.ALUOutput = ID_EX.imm << 16;
						//print_instruction(CURRENT_STATE.PC);
                    }
                    else printf("execute flag error\n");
                  break; 
                default:
                    printf("default on left hit\n");
                 
		}
	}
}

void print_instruction(uint32_t addr){
	uint32_t instruction, opcode, function, rs, rt, rd, sa, immediate, target;
	
	instruction = mem_read_32(addr);
	
	opcode = (instruction & 0xFC000000) >> 26;
	function = instruction & 0x0000003F;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	sa = (instruction & 0x000007C0) >> 6;
	immediate = instruction & 0x0000FFFF;
	target = instruction & 0x03FFFFFF;
	
	if(opcode == 0x00){
		/*R format instructions here*/
		
		switch(function){
			case 0x00:
				printf("SLL $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x02:
				printf("SRL $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x03:
				printf("SRA $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x08:
				printf("JR $r%u\n", rs);
				break;
			case 0x09:
				if(rd == 31){
					printf("JALR $r%u\n", rs);
				}
				else{
					printf("JALR $r%u, $r%u\n", rd, rs);
				}
				break;
			case 0x0C:
				printf("SYSCALL\n");
				break;
			case 0x10:
				printf("MFHI $r%u\n", rd);
				break;
			case 0x11:
				printf("MTHI $r%u\n", rs);
				break;
			case 0x12:
				printf("MFLO $r%u\n", rd);
				break;
			case 0x13:
				printf("MTLO $r%u\n", rs);
				break;
			case 0x18:
				printf("MULT $r%u, $r%u\n", rs, rt);
				break;
			case 0x19:
				printf("MULTU $r%u, $r%u\n", rs, rt);
				break;
			case 0x1A:
				printf("DIV $r%u, $r%u\n", rs, rt);
				break;
			case 0x1B:
				printf("DIVU $r%u, $r%u\n", rs, rt);
				break;
			case 0x20:
				printf("ADD $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x21:
				printf("ADDU $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x22:
				printf("SUB $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x23:
				printf("SUBU $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x24:
				printf("AND $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x25:
				printf("OR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x26:
				printf("XOR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x27:
				printf("NOR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x2A:
				printf("SLT $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			default:
				printf("Instruction is not implemented!\n");
				break;
		}
	}
	else{
		switch(opcode){
			case 0x01:
				if(rt == 0){
					printf("BLTZ $r%u, 0x%x\n", rs, immediate<<2);
				}
				else if(rt == 1){
					printf("BGEZ $r%u, 0x%x\n", rs, immediate<<2);
				}
				break;
			case 0x02:
				printf("J 0x%x\n", (addr & 0xF0000000) | (target<<2));
				break;
			case 0x03:
				printf("JAL 0x%x\n", (addr & 0xF0000000) | (target<<2));
				break;
			case 0x04:
				printf("BEQ $r%u, $r%u, 0x%x\n", rs, rt, immediate<<2);
				break;
			case 0x05:
				printf("BNE $r%u, $r%u, 0x%x\n", rs, rt, immediate<<2);
				break;
			case 0x06:
				printf("BLEZ $r%u, 0x%x\n", rs, immediate<<2);
				break;
			case 0x07:
				printf("BGTZ $r%u, 0x%x\n", rs, immediate<<2);
				break;
			case 0x08:
				printf("ADDI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x09:
				printf("ADDIU $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0A:
				printf("SLTI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0C:
				printf("ANDI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0D:
				printf("ORI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0E:
				printf("XORI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0F:
				printf("LUI $r%u, 0x%x\n", rt, immediate);
				break;
			case 0x20:
				printf("LB $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x21:
				printf("LH $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x23:
				printf("LW $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x28:
				printf("SB $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x29:
				printf("SH $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x2B:
				printf("SW $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			default:
				printf("Instruction is not implemented!\n");
				break;
		}
	}
}



