//read file in one at a time
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

int NUM_INSTRUCTIONS = 0;



int main(int argc, char *argv[]) {
char* instruction = malloc(sizeof(char)*10);
char* reg1 = malloc(sizeof(char)*5);
char* reg2 = malloc(sizeof(char)*5);
char* offset = malloc(sizeof(char)*5);
FILE *fp = fopen(argv[1], "r");
    while (fscanf(fp, "%s %s %s %s\n", instruction, reg1, reg2,offset) != EOF){
	
        //checkInstruction(instruction, fp);
         printf("instruction: %s %s %s %s\n", instruction, reg1, reg2, offset);
		
		

	//switch (atoi(instruction)){

		if (strcmp("add", instruction)== 0){
                  
		}
		else if (strcmp("addu", instruction)== 0){
		}
		else if (strcmp("addi", instruction)== 0){
		}
		else if (strcmp("addiu", instruction)== 0){
		}
		else if (strcmp("sub", instruction)== 0){
		}
		else if (strcmp("subu", instruction)== 0){
		}
		else if (strcmp("mult", instruction)== 0){
		}
		else if (strcmp("multu", instruction)== 0){
		}
		else if (strcmp("div", instruction)== 0){
		}
		else if (strcmp("divu", instruction)== 0){
		}
		else if (strcmp("and", instruction)== 0){
		}
		else if (strcmp("andi", instruction)== 0){
		}
		else if (strcmp("or", instruction)== 0){
		}
		else if (strcmp("ori", instruction)== 0){
		}
		else if (strcmp("xor", instruction)== 0){
		}
		else if (strcmp("xori", instruction)== 0){
		}
		else if (strcmp("nor", instruction)== 0){
		}
		else if (strcmp("slt", instruction)== 0){
		}
		else if (strcmp("slti", instruction)== 0){
		}
		else if (strcmp("sll", instruction)== 0){
		}
		else if (strcmp("srl", instruction)== 0){
		}
		else if (strcmp("sra", instruction)== 0){
		}
		else if (strcmp("lw", instruction)== 0){
		}
		else if (strcmp("lb", instruction)== 0){
		}
		else if (strcmp("lh", instruction)== 0){
		}
		else if (strcmp("lui", instruction)== 0){
		}
		else if (strcmp("sw", instruction)== 0){
		}
		else if (strcmp("sb", instruction)== 0){
		}
		else if (strcmp("sh", instruction)== 0){
		}
		else if (strcmp("mfhi", instruction)== 0){
		}
		else if (strcmp("mflo", instruction)== 0){
		}
		else if (strcmp("mthi", instruction)== 0){
		}
		else if (strcmp("mtlo", instruction)== 0){
		}
		else if (strcmp("beq", instruction)== 0){
		}
		else if (strcmp("bne", instruction)== 0){
		}
		else if (strcmp("blez", instruction)== 0){
		}
		else if (strcmp("bltz", instruction)== 0){
		}
		else if (strcmp("bgez", instruction)== 0){
		}
		else if (strcmp("bgtz", instruction)== 0){
		}
		else if (strcmp("j", instruction)== 0){
		}
		else if (strcmp("jr", instruction)== 0){
		}
		else if (strcmp("jal", instruction)== 0){
		}
		else if (strcmp("jalr", instruction)== 0){
		}
		else if (strcmp("syscall", instruction)== 0){
		}
		
		else  {
                    printf("Did not find instruction\n");
                }
		

	
	NUM_INSTRUCTIONS++;           
        }
free(instruction);
free(reg1);
free(reg2);            
return 0;
         
}

//void checkInstruction(char* instruction, FILE* fp){







