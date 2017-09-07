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

	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/


	
	uint32_t binInstruction , overflow;
	uint32_t instruction;
	long int rd, rt, rs, sa, base;
	int jumpNum = 4;
	instruction = convertInstruction(mem_read_32(CURRENT_STATE.PC), flag);//does convertInstruction give the opcode?
	binInstruction = mem_read_32(CURRENT_STATE.PC);
	printf("0x%08x from 0x%08x\n",mem_read_32(CURRENT_STATE.PC), CURRENT_STATE.PC);
	printf("instruction %08x\n", instruction);

	if(flag==0){ //If flag is set = instruction is 'left'
		
		switch(instruction){//case statement for right
			//ADD
			case 0x20:
				printf("ADD\n");
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
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
					printf("%u\n", NEXT_STATE.REGS[rd]);
				}
				else
					printf("Addition Overflow\n");
				break;
			//ADDU		
			case 0x21:
				printf("ADDU\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
				break;
			//AND - havent checked
			case 0x24:
				printf("AND\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
				printf("%u\n", NEXT_STATE.REGS[rd]);
				break;
			//SUB no check
			case 0x22: 
				printf("SUB\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				overflow = binInstruction >> 9;
				if ((overflow | 0x0) == 0){
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]-CURRENT_STATE.REGS[rt];
					printf("%u\n", NEXT_STATE.REGS[rd]);
				}
				else
					printf("Subtraction Overflow\n");
				break;
			//SUBU havent checked
			case 0x23:
				printf("SUBU\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]-CURRENT_STATE.REGS[rt];
				printf("%u\n", NEXT_STATE.REGS[rd]);
				break;
		//MULT not pretty sure this is wrong don't know where to store
			case 0x18:
				printf("MULT\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				int64_t temp = CURRENT_STATE.REGS[rt] * CURRENT_STATE.REGS[rs];//bitwise complement of each which is 1's complement add one for 2's complement and multiply
				//need to create temp variable somewhere (maybe a 64 bit signed?)
				//low word gets stored in LO, high word gets stored in high
		/*
			If either of the two preceding instructions is MFHI or MFLO, the results of
			these instructions are undefined. Correct operation requires separating
			reads of HI or LO from writes by a minimum of two other instructions.
		*/
				NEXT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
				NEXT_STATE.HI = temp >> 32;//shift to get the high 32 bits
				break;
			//MULTU needs work
			case 0x19:
				printf("MULTU\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				temp = CURRENT_STATE.REGS[rt] * CURRENT_STATE.REGS[rs];//takes the unsigned multiplication
				//need to create temp variable somewhere (maybe a 64 bit unsigned?)
				//low word gets stoer in LO high word gets stored in high
		/*
			If either of the two preceding instructions is MFHI or MFLO, the results of
			these instructions are undefined. Correct operation requires separating
			reads of HI or LO from writes by a minimum of two other instructions.
		*/
				NEXT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
				NEXT_STATE.HI = temp >> 32;//shift to get the high 32 bits
				break;
			//DIV needs work
			case 0x1A:
				printf("DIV\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				if (rt == 0){
					printf("Cannot divide by 0\n");
				}
				else{
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					temp = CURRENT_STATE.REGS[rs]/CURRENT_STATE.REGS[rt];//is that treating them as the 2's complement
					NEXT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
					NEXT_STATE.HI = temp >> 32;//shift to get the high 32 bits
				}
				/*
					If either of the two preceding instructions is MFHI or MFLO, the results of
					these instructions are undefined. Correct operation requires separating
					reads of HI or LO from writes by a minimum of two other instructions.
				*/
				break;
			
			//DIVU needs work
			case 0x1B:
				printf("DIVU\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				if (rt == 0){
					printf("Cannot divide by 0\n");
				}
				else{
					binInstruction = binInstruction >> 5;
					rs = binInstruction & 0x1F;
					temp = CURRENT_STATE.REGS[rs]/CURRENT_STATE.REGS[rt];//is that treating them as the 2's complement
					NEXT_STATE.LO = temp & 0xFFFFFFFF;//isolate the low 32 bits
					NEXT_STATE.HI = temp >> 32;//shift to get the high 32 bits
				}
				/*
					If either of the two preceding instructions is MFHI or MFLO, the results of
					these instructions are undefined. Correct operation requires separating
					reads of HI or LO from writes by a minimum of two other instructions.
				*/
				break;
			//OR
			case 0x25:
				printf("OR\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
				printf("%u\n", NEXT_STATE.REGS[rd]);
				break;
			
			//NOR haven't checked
			case 0x27:
				printf("NOR\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
				break;
			//XOR
			case 0x26:
				printf("XOR\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]^CURRENT_STATE.REGS[rt]);
				break;
			//SLT
			case 0x2A:
				printf("SLT\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				int32_t tempSLT = CURRENT_STATE.REGS[rs]-CURRENT_STATE.REGS[rt];//because they are to be treated as signed
				if(tempSLT < 0){
					NEXT_STATE.REGS[rd] = 1;
				}
				else{
					NEXT_STATE.REGS[rd] = 0;
				}
				break;
			//SLL not checked
			case 0x0:
				printf("SLL\n");
				binInstruction = binInstruction >> 6;
				sa = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rd = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << CURRENT_STATE.REGS[sa];
				break;
			//SRA not checked
			case 0x3:
				printf("SRA\n");
				binInstruction = binInstruction >> 6;
				sa = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				rd = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rt = binInstruction & 0x1F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[sa];
				break;				
			//HOW TO SIGN EXTEND IT
			//MFLO not checked
			case 0x12:
				printf("MFLO\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x01F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
				break;
			//MFHI
			case 0x10:
				printf("MFHI\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x01F;
				NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
				break;
			//MTHI
			case 0x11:
				printf("MTHI\n");
				binInstruction = binInstruction >> 21;
				rd = binInstruction & 0x01F;
				NEXT_STATE.HI = CURRENT_STATE.REGS[rd];
				break;
			//MTLO
			case 0x13:
				printf("MTLO\n");
				binInstruction = binInstruction >> 21;
				rd = binInstruction & 0x01F;
				NEXT_STATE.LO = CURRENT_STATE.REGS[rd];
				break;
				
			//JR
			case 0x8:
				printf("JR\n");
				binInstruction = binInstruction >> 21;
				rs = binInstruction & 0x01F;
				NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
				break;
			//JALR
			case 0x9:
				printf("JALR\n");
				binInstruction = binInstruction >> 11;
				rd = binInstruction & 0x01F;
				binInstruction = binInstruction >> 10;
				rs = binInstruction & 0x01F;
				NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
				//CURRENT_STATE.REGS[rd] = NEXT_STATE.PC + 1;
				/*
					The program unconditionally jumps to the address contained in general
					register rs, with a delay of one instruction. The address of the instruction
					after the delay slot is placed in general registerrd. The default value of rd,
					if omitted in the assembly language instruction, is 31.
				*/
				break;

			//SYSCALL
			case 0xC:
				printf("SYSCALL");
				if (CURRENT_STATE.REGS[2] == 0xA){
					RUN_FLAG = FALSE;
				}
				break;		
		}
	}
	else if ((flag)){
		printf("inside elseif\n");
		switch(binInstruction){


			case 0x8: //ADDI
				/* The 16-bit immediate is sign-extended and added to the contents of general
				 * register rs to form the result. The result is placed into general register rt.
				 * In 64-bit mode, the operand must be valid sign-extended, 32-bit values.
				 * An overflow exception occurs if carries out of bits 30 and 31 differ (2’s
				 * complement overflow). The destination register rt is not modified when
				 * an integer overflow exception occurs.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;
				rt = rs + (binInstruction & 0x000FFFF);			//ADD rt with the contents of 'immediate'
				binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value

		// NOTE: Need to add overflow functionality!

			case 0x9: //ADDIU
				/* The 16-bit immediate is sign-extended and added to the contents of general
				 * register rs to form the result. The result is placed into general register rt.
				 * No integer overflow exception occurs under any circumstances. In 64-bit
				 * mode, the operand must be valid sign-extended, 32-bit values.
				 * The only difference between this instruction and the ADDI instruction is
				 * that ADDIU never causes an overflow exception.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt = signExtend((binInstruction & 0x000FFFF));	//isolate 'immediate' and sign extend it
				rt = rs + rt;									//ADD rt with the contents of 'immediate'
				binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value

			case 0xC: //ANDI
				/*The 16-bit immediate is zero-extended and combined with the contents of
			 	 * general registerrsin a bit-wise logical AND operation. The result is placed
			 	 * into general register rt.
			 	*/

			case 0xD: //ORI
				/* The 16-bit immediate is zero-extended and combined with the contents of
				 * general register rs in a bit-wise logical OR operation. The result is placed
				 * into general register rt.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;
				rt = rs | (binInstruction & 0x000FFFF);			//OR rt with the contents of 'immediate'
				binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value


			case 0xE: //XORI
				/* The 16-bit immediate is zero-extended and combined with the contents of
				 * general register rs in a bit-wise logical exclusive OR operation.
				 * The result is placed into general register rt.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt = rs ^ (binInstruction & 0x000FFFF);			//XOR rt with the contents of 'immediate'
				binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value

			case 0xA: //SLTI
				/* The 16-bit immediate is sign-extended and subtracted from the contents of
				 * general register rs. Considering both quantities as signed integers, if rs is
				 * less than the sign-extended immediate, the result is set to one; otherwise
				 * the result is set to zero.
				 * The result is placed into general register rt.
				 * No integer overflow exception occurs under any circumstances. The
				 * comparison is valid even if the subtraction used during the comparison overflows.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt = rs ^ (binInstruction & 0x000FFFF);			//isolate the contents of 'immediate'
				rt = signExtend(rt);
				if (rs < rt){
					rt = rt & 0x0;
					rt = rt | 0x00000001;
				}
				else {
					rt = rt & 0x0;
				}

				binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value

			case 0x23: //LW
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base to form a virtual address. The contents of the word at the
				 * memory location specified by the effective address are loaded into general
				 * register rt. In 64-bit mode, the loaded word is sign-extended. If either of
				 * the two least-significant bits of the effective address is non-zero, an
				 * address error exception occurs.
				*/



			case 0x20: //LB
				//sign extend the offset (take left most bit and concatenate 16 of those on)
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				temp = CURRENT_STATE.REGS[base] + offset;
				//sign extend temp at 8th bit
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[base];
				

			case 0x21: //LH
				//sign extend on the 
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				temp = CURRENT_STATE.REGS[base] + offset;
				//sign extend temp at bit 16
				NEXT_STATE.REGS[rt] = temp;	

			case 0x2B: //SW
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				
				//sign extend offset
				temp = offset + CURRENT_STATE.REGS[base];
				temp* = CURRENT_STATE.REGS[rt];

			case 0x28: //SB
			
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;

				//sign extend offset
				temp = offset + CURRENT_STATE.REGS[base];
				//least sig byte of register rt is store at effective address
				(temp)* = CURRENT_STATE.REGS[rt] >> 24;

			case 0x29: //SH
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;

				//sign extend offset
				temp = offset + CURRENT_STATE.REGS[base];
				if (temp & 0x10000000 == 1){
					printf("SH address error exception occurred\n");
				}
				else
					temp* = CURRENT_STATE.REGS[rt] >> 16;

			case 0x4: //BEQ

			case 0x5: //BNE

			case 0x6: //BLEZ

			case 0x7: //BGTZ
				

			case 0x2: //J
				target = binInstruction & 0x03FFFFFFL;
				target = target binInstruction << 2;
				//what are the delay slots??
				//address once combined with delay slot should be subtracted from current address
				//update jumpNum to see how far you have to jump at the end
				//supposed to delay one cycle but I don't know how to do that

			case 0x3: //JAL
				break;

			/* Not included:
			 * BLTZ
			 * BGEZ
			 */
		}
	}
		
	NEXT_STATE.PC = CURRENT_STATE.PC + jumpNum;	
			
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
	printf("Program loaded:\n");
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
uint32_t convertInstruction(uint32_t value, int flag){
	uint32_t right = value << 26;
	right = right >> 26;
	uint32_t left = value >> 24;
	
	if ((right & 0xFF) != 0){
		flag = 0;		
		return right;
	}
	else if ((left & 0xFF) != 0){
		flag=1;
		return left;
	}

	return right;//not having a return was causing compile errors but it should never hit this
	
	//Debug
	// printf("%08x\n",value);
	// printf("%08x\n", right);
	// printf("%08x\n", left);
}

long int signExtend(long int immediate){
    long int val = (immediate & 0x0000FFFF);
    long int mask = 0x00008000;
    if (mask & val) {
        val = val | 0xFFFF0000;
    }
    return val;
}
/*uint32_t signExtend(uint32_t immediate){
    uint32_t val = (immediate & 0x0000FFFF);
   uint32_t mask = 0x00008000;
    if (mask & val) {
        val = val | 0xFFFF0000;
    }
    return val;
}*/
