#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include "mu-mips.h"
int flag;
uint32_t prevInstruction;
int jumpFlag;


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

	//if right, comare with the known right commands

	//if left, compare with the known left commands

	//call command when one matches.

	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/

        
	
	uint32_t binInstruction , overflow;
	uint32_t instruction, offset;
	long int rd, rt, rs, sa, base, temp, target;
        int64_t  tempMult;
        uint64_t tempMultU;
        uint32_t effectiveAddress;// = (uint32_t* )0x123456; //= malloc(sizeof (uint32_t)); 
        //int fd = open("/dev/mem", O_RDWR);
	//int jumpNum = 4;
	instruction = convertInstruction(mem_read_32(CURRENT_STATE.PC));
	binInstruction = mem_read_32(CURRENT_STATE.PC);
	printf("0x%08x from 0x%08x\n",mem_read_32(CURRENT_STATE.PC), CURRENT_STATE.PC);
	//printf("instruction %08x\n", instruction);
       
                
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
		//MULT 
			case 0x18:
				printf("MULT\n");
				//int64_t tempMult;
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				tempMult = CURRENT_STATE.REGS[rt] * CURRENT_STATE.REGS[rs];
				
				/*
					If either of the two preceding instructions is MFHI or MFLO, the results of
					these instructions are undefined. Correct operation requires separating
					reads of HI or LO from writes by a minimum of two other instructions.
				*/
				if (prevInstruction == 0x12 || prevInstruction == 0x10)
				{
					printf("Result undefined\n");
				}
				else{
					NEXT_STATE.LO = tempMult & 0xFFFFFFFF;//isolate the low 32 bits
					NEXT_STATE.HI = tempMult >> 32;//shift to get the high 32 bits
				}
				break;
			//MULTU needs work
			case 0x19:
				//uint64_t tempMultU;
				printf("MULTU\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				binInstruction = binInstruction >> 5;
				rs = binInstruction & 0x1F;
				tempMultU = CURRENT_STATE.REGS[rt] * CURRENT_STATE.REGS[rs];//takes the unsigned multiplication
				if (prevInstruction == 0x12 || prevInstruction == 0x10){				
					printf("Result undefined\n");
				}
				/*
					If either of the two preceding instructions is MFHI or MFLO, the results of
					these instructions are undefined. Correct operation requires separating
					reads of HI or LO from writes by a minimum of two other instructions.
				*/		
				else{
					NEXT_STATE.LO = tempMultU & 0xFFFFFFFF;//isolate the low 32 bits
					NEXT_STATE.HI = tempMultU >> 32;//shift to get the high 32 bits
				}
				break;
			//DIV 
			case 0x1A:
				//int32_t tempDiv;
				printf("DIV\n");
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				if (prevInstruction == 0x12 || prevInstruction == 0x10){				
					printf("Result undefined\n");
				}
				else{				
					if (CURRENT_STATE.REGS[rt] == 0){
						printf("Cannot divide by 0\n");
					}
					else{
						binInstruction = binInstruction >> 5;
						rs = binInstruction & 0x1F;
						NEXT_STATE.LO = CURRENT_STATE.REGS[rs]/CURRENT_STATE.REGS[rt];
						NEXT_STATE.HI = CURRENT_STATE.REGS[rs]%CURRENT_STATE.REGS[rt];
					
				
					}
						/*
							If either of the two preceding instructions is MFHI or MFLO, the results of
							these instructions are undefined. Correct operation requires separating
							reads of HI or LO from writes by a minimum of two other instructions.
						*/
				}
				break;
			
			//DIVU 
			case 0x1B:
				printf("DIVU\n");
				//uint32_t tempDivU;
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x01F;
				if (prevInstruction == 0x12 || prevInstruction == 0x10){				
					printf("Result undefined\n");
				}
				else{
					if (CURRENT_STATE.REGS[rt] == 0){
						printf("Cannot divide by 0\n");
					}
					else{
						binInstruction = binInstruction >> 5;
						rs = binInstruction & 0x1F;
                                                NEXT_STATE.LO = CURRENT_STATE.REGS[rs]/CURRENT_STATE.REGS[rt];
						NEXT_STATE.HI = CURRENT_STATE.REGS[rs]%CURRENT_STATE.REGS[rt];
					}
					/*
						If either of the two preceding instructions is MFHI or MFLO, the results of
						these instructions are undefined. Correct operation requires separating
						reads of HI or LO from writes by a minimum of two other instructions.
					*/
                                }
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
			
			//NOR 
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
			//SLL 
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
			//SRA 
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
				//ignoring the delaty slot because he said in class to make rd PC =4
				NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
				/*
					The program unconditionally jumps to the address contained in general
					register rs, with a delay of one instruction. The address of the instruction
					after the delay slot is placed in general registerrd. The default value of rd,
					if omitted in the assembly language instruction, is 31.
				*/
				break;
                        //SRL
                        case 0x02:
                            binInstruction = binInstruction >> 6;
                            sa = binInstruction & 0x01F;
                            binInstruction = binInstruction >> 5;
                            rd = binInstruction & 0x01F;
                            binInstruction = binInstruction >> 5;
                            rt = binInstruction & 0x01F;
                            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[sa];
                
			//SYSCALL
			case 0xC:
				printf("SYSCALL\n");
				if (CURRENT_STATE.REGS[2] == 0xA){
					RUN_FLAG = FALSE;
				}
				break;	
                        default:
                            printf("default on right hit\n");
		}
	}
	else if ((flag)){
		printf("inside elseif\n");
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

				
				immediate = signExtend(binInstruction & 0x0000FFFF);
                                rt = (binInstruction >> 16) & 0x0000001F;
                                rs = (binInstruction >> 21) & 0x0000001F;
				// rt = rs + (binInstruction & 0x000FFFF);			//ADD rt with the contents of 'immediate'
				// binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				// binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value
				CURRENT_STATE.REGS[rt] = immediate + CURRENT_STATE.REGS[rs];

		// NOTE: Need to add overflow functionality!

			case 0x9: //ADDIUlong int
				/* The 16-bit immediate is sign-extended and added to the contents of general
				 * register rs to form the result. The result is placed into general register rt.
				 * No integer overflow exception occurs under any circumstances. In 64-bit
				 * mode, the operand must be valid sign-extended, 32-bit values.
				 * The only difference between this instruction and the ADDI instruction is
				 * that ADDIU never causes an overflow exception.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt =  (binInstruction >> 16) & 0x0000001F;
				immediate = signExtend((binInstruction & 0x0000FFFF));	//isolate 'immediate' and sign extend it
				// rt = rs + rt;									//ADD rt with the contents of 'immediate'
				// binInstruction = binInstruction & 0xFE0FFFFF;	//clear out the rt register
				// binInstruction = (binInstruction | (rt << 16)); //shift rt to its correct position and OR it with the original value
				CURRENT_STATE.REGS[rt] = immediate + CURRENT_STATE.REGS[rs];

			case 0xC: //ANDI
				/*The 16-bit immediate is zero-extended and combined with the contents of
			 	 * general registerrsin a bit-wise logical AND operation. The result is placed
			 	 * into general register rt.
			 	*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt = (binInstruction >> 16) & 0x0000001F;
				immediate = binInstruction & 0x0000FFFF;
				CURRENT_STATE.REGS[rt] = immediate & CURRENT_STATE.REGS[rs];

			case 0xD: //ORI
				/* The 16-bit immediate is zero-extended and combined with the contents of
				 * general register rs in a bit-wise logical OR operation. The result is placed
				 * into general register rt.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;
                                rt = (binInstruction >> 16) & 0x0000001F;
				immediate = binInstruction & 0x0000FFFF;
				CURRENT_STATE.REGS[rt] = immediate | CURRENT_STATE.REGS[rs];

			case 0xE: //XORI
				/* The 16-bit immediate is zero-extended and combined with the contents of
				 * general register rs in a bit-wise logical exclusive OR operation.
				 * The result is placed into general register rt.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs
				rt = (binInstruction >> 16) & 0x0000001F;
				immediate = binInstruction & 0x0000FFFF;		//zero extend and isolate immediate
				CURRENT_STATE.REGS[rt] = immediate ^ CURRENT_STATE.REGS[rs];
				

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
				rt = (binInstruction >> 16) & 0x0000001F;
				immediate = (binInstruction & 0x000FFFF);		//isolate the contents of 'immediate'
				immediate = signExtend(immediate);
				CURRENT_STATE.REGS[rt] = immediate - CURRENT_STATE.REGS[rs];
				if (CURRENT_STATE.REGS[rs] < immediate){
					CURRENT_STATE.REGS[rt] = 0x01;
				}
				else {
					CURRENT_STATE.REGS[rt] = rt & 0x0;
				}

			case 0x23: //LW
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base to form a virtual address. The contents of the word at the
				 * memory location specified by the effective address are loaded into general
				 * register rt. In 64-bit mode, the loaded word is sign-extended. If either of
				 * the two least-significant bits of the effective address is non-zero, an
				 * address error exception occurs.
				*/

				rs = (binInstruction >> 21) & 0x0000001F;		//isolate rs (in this case, base)
                                rt = (binInstruction >> 16) & 0x0000001F;               //isolate rt
				immediate = (binInstruction & 0x000FFFF);		//isolate the contents of 'immediate'
				immediate = signExtend(immediate);

				CURRENT_STATE.REGS[rt] = mem_read_32(immediate + CURRENT_STATE.REGS[rs]);


			case 0x20: //LB
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base tbltz o form a virtual address. The contents of the byte at the
				 * memory location specified by the effective address are sign-extended and
				 * loaded into general register rt.
				*/

				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
                                offset = signExtend(offset);                                
				NEXT_STATE.REGS[rt] = mem_read_32(CURRENT_STATE.REGS[base] + offset);
				

			case 0x21: //LH
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base to form a virtual address. The contents of the halfword at the
				 * memory location specified by the effective address are sign-extended and
				 * loaded into general register rt.
				 * If the least-significant bit of the effective address is non-zero, an address
				 * error exception occurs.
				*/
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				temp = CURRENT_STATE.REGS[base] + offset;
                                
				//sign extend temp at bit 16
				NEXT_STATE.REGS[rt] = temp;	

				if (temp == (temp & 0xFFFFFFFE)){
					printf("an overflow has occurred");
				}

			case 0x2B: //SW
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base to form a virtual address. The contents of general register rt
				 * are stored at the memory location specified by the effective address.
				 * If either of the two least-significant bits of the effective address are nonzero,
				 * an address error exception occurs
				*/
                                printf("SW\n");
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				offset = signExtend(offset);
				//sign extend offset
                                
				effectiveAddress = offset + CURRENT_STATE.REGS[base];//NEED TO FIGURE THIS OUT
				mem_write_32(effectiveAddress, CURRENT_STATE.REGS[rt]);

			case 0x28: //SB
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register baseto form a virtual address. The least-significant byte of register
				 * rt is stored at the effective address.
				*/
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
                                offset = signExtend(offset);
				//sign extend offset
				effectiveAddress = offset + CURRENT_STATE.REGS[base];//NEED TO FIGURE THIS OUT
				//least sig byte of register rt is store at effective address
				//*(effectiveAddress) = CURRENT_STATE.REGS[rt] >> 24;
				mem_write_32(effectiveAddress, CURRENT_STATE.REGS[rt]>>24);

			case 0x29: //SH
				/* The 16-bit offset is sign-extended and added to the contents of general
				 * register base to form an unsigned effective address. The least-significant
				 * halfword of register rt is stored at the effective address. If the leastsignificant
				 * bit of the effective address is non-zero, an address error
				 * exception occurs
				*/
				offset =  binInstruction & 0x0FFFF; 
				binInstruction = binInstruction >> 16;
				rt = binInstruction & 0x001F;
				binInstruction = binInstruction >> 5;
				base = binInstruction & 0x001F;
				offset = signExtend(offset);
				effectiveAddress = offset + CURRENT_STATE.REGS[base];//NEED TO FIGURE THIS OUT
				if ((effectiveAddress & 0x10000000) == 0x10000000){
					printf("SH address error exception occurred\n");
				}
				else{
					//*effectiveAddress = CURRENT_STATE.REGS[rt] >> 16;
                                    mem_write_32(effectiveAddress, CURRENT_STATE.REGS[rt]>>16);
                                }

			case 0x4: //BEQ
				/* A branch target address is computed from the sum of the address of the
				 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
				 * sign-extended. The contents of general register rs and the contents of
				 * general register rt are compared. If the two registers are equal, then the
				 * program branches to the target address, with a delay of one instruction.
				*/

				// long int delay = 
				rt = binInstruction & 0x001F0000;
				rt = rs >> 16;
				rs = binInstruction & 0x03E00000;
				rs = rs >> 21;
				if (rs == rt){
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}

			case 0x5: //BNE
				/* A branch target address is computed from the sum of the address of the
				 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
				 * sign-extended. The contents of general register rs and the contents of
				 * general register rt are compared. If the two registers are not equal, then
				 * the program branches to the target address, with a delay of one
				 * instruction.
				*/

				rt = binInstruction & 0x001F0000;
				rt = rs >> 16;
				rs = binInstruction & 0x03E00000;
				rs = rs >> 21;
				if (rs != rt){
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}
                        break;
			case 0x6: //BLEZ
				/* A branch target address is computed from the sum of the address of the
				 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
				 * sign-extended. The contents of general registerrs are compared to zero. If
				 * the contents of general registerrs have the sign bit set, or are equal to zero,
				 * then the program branches to the target address, with a delay of one instruction.
				*/

				rs = binInstruction & 0x03E00000;
				rs = rs >> 21;
				if (rs == 0x0){
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}
				else if (((rs & 0x00008000) >> 16) == 0x1){		//the msb of offset is 1 i.e. offset is negative
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}
                        break;
			case 0x7: //BGTZ
				/* A branch target address is computed from the sum of the address of the
				 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
				 * sign-extended. The contents of general registerrs are compared to zero. If
				 * the contents of general register rs have the sign bit cleared and are not
				 * equal to zero, then the program branches to the target address, with a
				 * delay of one instruction.
				*/

				rs = binInstruction & 0x03E00000;
				rs = rs >> 21;
				if (rs != 0x0){
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}
				else if (((rs & 0x00008000) >> 16) == 0x0){		//the msb of offset is 0
					CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
				}
			break;	

			case 0x2: //J
				/* The 26-bit target address is shifted left two bits and combined with the
				 * high-order bits of the address of the delay slot. The program
				 * unconditionally jumps to this calculated address with a delay of one instruction.
				*/
				// target = binInstruction & 0x03FFFFFF;
				// target = target binInstruction << 2;
				//what are the delay slots??
				//address once combined with delay slot should be subtracted from current address
				//update jumpNum to see how far you have to jump at the end
				//supposed to delay one cycle but I don't know how to do that

				target = binInstruction & 0x03FFFFFF;
				target = target << 2;
				NEXT_STATE.PC = target + (CURRENT_STATE.PC & 0xF0000000) - 4;
				//subtracted 4 because we add four at the end for all the other instructions to move on
                        break;
			case 0x03: //JAL
				target = binInstruction & 0x03FFFFFF;
				target = target << 2;
				NEXT_STATE.PC = target + (CURRENT_STATE.PC & 0xF0000000) - 4;
				NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
				break;
				//subtracted 4 because we add four at the end for all the other instructions to move on
                        break;
			//this opcode is in the middle
			case 0x01://BLTZ and BGEZ
				if (((binInstruction & 0x001F0000) >> 16) == 0x0){		
					/* A branch target address is computed from the sum of the address of the
					 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
					 * sign-extended. If the contents of general register rs have the sign bit set,
					 * then the program branches to the target address, with a delay of one instruction.
					*/

					rs = binInstruction & 0x03E00000; 			//mask the rs register
					rs = rs >> 21;
					if (((rs & 0x00008000) >> 16) == 0x1){		//the msb of offset is 1 i.e. offset is negative
						30CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
					}

				else if (((binInstruction & 0x001F0000) >> 16) == 0x0){	//BGEZ
					/* A branch target address is computed from the sum of the address of the
					 * instruction in the delay slot and the 16-bit offset, shifted left two bits and
					 * sign-extended. If the contents of general register rs have the sign bit
					 * cleared, then the program branches to the target address, with a delay of
					 * one instruction.
					*/

					rs = binInstruction & 0x03E00000; 			//mask the rs register
					rs = rs >> 21;
					if (((rs & 0x00008000) >> 16) == 0x0){		//the msb of offset is 1 i.e. offset is negative
						CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
					}
				}
			}
			break;
			//LUI
                        case 0x0F: 
                            printf("LUI\n");
                            immediate = binInstruction & 0x0FFFF;//isolate immediate
                            binInstruction = binInstruction >> 16;
                            rt = binInstruction & 0x1F;//isolate rt
                            NEXT_STATE.REGS[rt] = (immediate << 16) | 0x0000;//shift 16 bits left 16 bits and concatenated with 16 bits of 0s
                            break;
                        default:
                            printf("default on left hit\n");
		}
	}
	prevInstruction = instruction;
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
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
uint32_t convertInstruction(uint32_t value){
	uint32_t right = value << 26;
	right = right >> 26;
	uint32_t left = value >> 26;//I chagned this to 26 from 24
	
	if ((right & 0x3F) != 0){//I changed these from FF to 3F
		flag = 0;
                printf("right: %08x\n", right);
		return right;
	}
	else if ((left & 0x3F) != 0){
		flag=1;
                 printf("left: %08x, flag: %d\n", left, flag);
		return left;
	}
        else{
            
            printf("Not a valid instruction\n");
        }
        
        return 0xFF;
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
