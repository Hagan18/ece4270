#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

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
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
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
			runAll(); 
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
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/

	/****	Pseudo code		***/
	//read the memory of the starting memory location

	//shift the value of that value to figure out if its left or right & store in var

	//if right, compare with the known right commands

	//if left, compare with the known left commands

	//call command when one matches.

	int i = 0;
	


	// for (i; i< 6;i++){
		//if(CURRENT_STATE.REGS[i] )
	// }
	printf("\nCurrent State: %016u\n", *CURRENT_STATE.REGS);
	
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	printf("in handle instruction\n");
	//uint32_t currentMem = mem_read_32(MEM_BEGIN);

	//printf("%u\n", (unsigned)currentMem);

	uint32_t binInstruction = 0x22A8820, overflow;
	long int rd, rt, rs, sa;
	//case(instruction){//case statement for right
		//ADD
		//case 100000:
			binInstruction = binInstruction >> 11;//define rd rt rs as ints?
			printf("after first shift %u\n", binInstruction);
			rd = binInstruction & 0x001F;
			printf("rd after first and %ld\n", rd);
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			printf("rt after first and %ld\n", rt);
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			printf("rs after first and %ld\n", rs);
			printf("after rs shift %u\n", binInstruction);
			// check for overflow
			overflow = binInstruction >> 9;
			if ((overflow | 0x0) == 0){
				CURRENT_STATE.REGS[rd] = rt + rs;
				printf("%u\n", CURRENT_STATE.REGS[rd]);
			}
			else
				printf("Addition Overflow\n");
		//ADDU- no overflow can occur		
		//case 100001:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rt + rs;
		//AND - havent checked
		//case 100100:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rs & rt;
			printf("%u\n", CURRENT_STATE.REGS[rd]);
		//SUB no check
		//case 1000010: 
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			overflow = binInstruction >> 9;
			if ((overflow | 0x0) == 0){
				CURRENT_STATE.REGS[rd] = rs-rt;
				printf("%u\n", CURRENT_STATE.REGS[rd]);
			}
			else
				printf("Subtraction Overflow\n");
		//SUBU havent checked
		//case 100011:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rs-rt;
			printf("%u\n", CURRENT_STATE.REGS[rd]);
	//MULT not pretty sure this is wrong don't know where to store
		//case 011000:
			binInstruction = binInstruction >> 16;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			int64_t temp = (int64_t)(~(rt+1) * ~(rs+1));//bitwise complement of each which is 1's complement add one for 2's complement and multiply
			//need to create temp variable somewhere (maybe a 64 bit signed?)
			//low word gets stored in LO, high word gets stored in high
	/*
		If either of the two preceding instructions is MFHI or MFLO, the results of
		these instructions are undefined. Correct operation requires separating
		reads of HI or LO from writes by a minimum of two other instructions.
	*/
			CURRENT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
			CURRENT_STATE.HI = temp >> 32;//shift to get the high 32 bits
		//MULTU needs work
		//case 011001:
			binInstruction = binInstruction >> 16;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			temp = (uint64_t)rt * rs;//takes the unsigned multiplication
			//need to create temp variable somewhere (maybe a 64 bit unsigned?)
			//low word gets stoer in LO high word gets stored in high
	/*
		If either of the two preceding instructions is MFHI or MFLO, the results of
		these instructions are undefined. Correct operation requires separating
		reads of HI or LO from writes by a minimum of two other instructions.
	*/
			CURRENT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
			CURRENT_STATE.HI = temp >> 32;//shift to get the high 32 bits
			
		//DIV needs work
		//case 011010:
			binInstruction = binInstruction >> 16;
			rt = binInstruction & 0x01F;
			if (rt == 0){
				printf("Cannot divide by 0\n");
			}
			else{
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				temp = (int32_t)(~(rs+1)/~(rt+1));//is that treating them as the 2's complement
				CURRENT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
				CURRENT_STATE.HI = temp >> 32;//shift to get the high 32 bits
			}
			/*
				If either of the two preceding instructions is MFHI or MFLO, the results of
				these instructions are undefined. Correct operation requires separating
				reads of HI or LO from writes by a minimum of two other instructions.
			*/
		
		//DIVU needs work
		//case 011011:
			binInstruction = binInstruction >> 16;
			rt = binInstruction & 0x01F;
			if (rt == 0){
				printf("Cannot divide by 0\n");
			}
			else{
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				temp = rs/rt;//is that treating them as the 2's complement
				CURRENT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
				CURRENT_STATE.HI = temp >> 32;//shift to get the high 32 bits
			}
			/*
				If either of the two preceding instructions is MFHI or MFLO, the results of
				these instructions are undefined. Correct operation requires separating
				reads of HI or LO from writes by a minimum of two other instructions.
			*/
		
		//OR
		//case 100101:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rs | rt;
			printf("%u\n", CURRENT_STATE.REGS[rd]);
		
		//NOR haven't checked
		//case 100111:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = ~(rs | rt);
			
		//XOR
		//case 100110:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = (rs^rt);
		
		//SLT
		//case 101010:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rs = binInstruction & 0x1F;
			int32_t tempSLT = (int32_t)(rs-rt);//because they are to be treated as signed
			if(tempSLT < 0){
				CURRENT_STATE.REGS[rd] = 1;
			}
			else{
				CURRENT_STATE.REGS[rd] = 0;
			}
		
		//SLL not checked
		//case 00000:
			binInstruction = binInstruction >> 6;
			sa = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rd = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rt << sa;
			
		//SRA not checked
		//case 000011:
			binInstruction = binInstruction >> 6;
			sa = binInstruction & 0x001F;
			binInstruction = binInstruction >> 5;
			rd = binInstruction & 0x01F;
			binInstruction = binInstruction >> 5;
			rt = binInstruction & 0x1F;
			CURRENT_STATE.REGS[rd] = rt >> sa;
			//HOW TO SIGN EXTEND IT
		//MFLO not checked
		//case 010010:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x01F;
			CURRENT_STATE.REGS[rd] = CURRENT_STATE.LO;
		//MFHI
		//case 010000:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x01F;
			CURRENT_STATE.REGS[rd] = CURRENT_STATE.HI;
			
		//MTHI
		//case 010001:
			binInstruction = binInstruction >> 21;
			rs = binInstruction & 0x01F;
			CURRENT_STATE.HI = CURRENT_STATE.REGS[rd];
		
		//MTLO
		//case 010011:
			binInstruction = binInstruction >> 21;
			rs = binInstruction & 0x01F;
			CURRENT_STATE.LO = CURRENT_STATE.REGS[rd];
			
		//JR- not thinking this is right
		//case 001000:
			binInstruction = binInstruction >> 21;
			rs = binInstruction & 0x01F;
			NEXT_STATE.PC = rs;//next state because it says to delay a clock cycle
		
		//JALR
		//case 001001:
			binInstruction = binInstruction >> 11;
			rd = binInstruction & 0x01F;
			binInstruction = binInstruction >> 10;
			rs = binInstruction & 0x01F;
			NEXT_STATE.PC = rs;
			CURRENT_STATE.REGS[rd] = NEXT_STATE.PC + 1;
			/*
				The program unconditionally jumps to the address contained in general
				register rs, with a delay of one instruction. The address of the instruction
				after the delay slot is placed in general registerrd. The default value of rd,
				if omitted in the assembly language instruction, is 31.
			*/
			
		//SYSCALL
		//case 001100:
			reset();//what is supposed to happen on a syscall
	//}
			
			
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
}

/***************************************************************/
/* main                                                                                                                                   */
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

/**************************************************
 * if the flag is set, the instruction is 'left'
 *************************************************/
uint32_t convertInstruction(uint32_t value, int* flag){
	uint32_t right = value << 26;
	right = right >> 26;
	uint32_t left = value >> 24;
	
	if ((right & 0xFF) != 0){
		return right;
	}
	else if ((left & 0xFF) != 0){
		*flag=1;
		return left;
	}
	
	//Debug
	// printf("%08x\n",value);
	// printf("%08x\n", right);
	// printf("%08x\n", left);
}
