//read file in one at a time
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

int NUM_INSTRUCTIONS = 0;

void r_type_format(long int instruction, char* reg1, char* reg2, char* reg3);
void i_type_format(long int instruction, char* reg1, char* reg2, char* offset);
unsigned int registerLookup(char* reg);


int main(int argc, char *argv[]) {
char* instruction = malloc(sizeof(char)*10);
char* reg1 = malloc(sizeof(char)*5);
char* reg2 = malloc(sizeof(char)*5);
char* offset = malloc(sizeof(char)*5);
FILE *fp = fopen(argv[1], "r");
    while (fscanf(fp, "%s %s %s %s\n", instruction, reg1, reg2,offset) != EOF){
        
        //This gets rid of the commas
        strcpy(reg1,strtok(reg1,","));
        strcpy(reg2,strtok(reg2,","));
	
        //checkInstruction(instruction, fp);
        printf("instruction: %s %s %s %s\n", instruction, reg1, reg2, offset);

		if (strcmp("add", instruction)== 0){//
            //format: instruction, reg1, reg2, reg3
            r_type_format(0x20,reg1,reg2,offset); 
		}
		else if (strcmp("addu", instruction)== 0){//
		    r_type_format(0x21,reg1,reg2,offset);
		}
		else if (strcmp("addi", instruction)== 0){//
		    i_type_format(0x08,reg1,reg2,offset);
		}
		else if (strcmp("addiu", instruction)== 0){//
		    i_type_format(0x09,reg1,reg2,offset);
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
		else if (strcmp("and", instruction)== 0){//
		}
		else if (strcmp("andi", instruction)== 0){//
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

void r_type_format(long int instruction, char* reg1, char* reg2, char* reg3){
    unsigned int r1 = registerLookup(reg1);
    unsigned int r2 = registerLookup(reg2);
    unsigned int r3 = registerLookup(reg3);
    printf("instruction: %08ld\n reg1: %08x\nreg2: %08x\nreg3: %08x\n",instruction,r1,r2,r3);
}

void i_type_format(long int instruction, char* reg1, char* reg2, char* offset){
    
}

unsigned int registerLookup(char* reg){
    if (strcmp("$zero",reg) == 0){
        return 0x00;
    }
    else if (strcmp("$at",reg) == 0){
        return 0x01;
    }
    else if (strcmp("$v0",reg) == 0){
        return 0x02;
    }
    else if (strcmp("$v1",reg) == 0){
        return 0x03;
    }
    else if (strcmp("$a0",reg) == 0){
        return 0x04;
    }
    else if (strcmp("$a1",reg) == 0){
        return 0x05;
    }
    else if (strcmp("$a2",reg) == 0){
        return 0x06;
    }
    else if (strcmp("$a3",reg) == 0){
        return 0x07;
    }
    else if (strcmp("$t0",reg) == 0){
        return 0x08;
    }
    else if (strcmp("$t1",reg) == 0){
        return 0x09;
    }
    else if (strcmp("$t2",reg) == 0){
        return 0x0A;
    }
    else if (strcmp("$t3",reg) == 0){
        return 0x0B;
    }
    else if (strcmp("$t4",reg) == 0){
        return 0x0C;
    }
    else if (strcmp("$t5",reg) == 0){
        return 0x0D;
    }
    else if (strcmp("$t6",reg) == 0){
        return 0x0E;
    }
    else if (strcmp("$t7",reg) == 0){
        return 0x0F;
    }
    else if (strcmp("$s0",reg) == 0){
        return 0x010;
    }
    else if (strcmp("$s1",reg) == 0){
        return 0x011;
    }
    else if (strcmp("$s2",reg) == 0){
        return 0x012;
    }
    else if (strcmp("$s3",reg) == 0){
        return 0x013;
    }
    else if (strcmp("$s4",reg) == 0){
        return 0x014;
    }
    else if (strcmp("$s5",reg) == 0){
        return 0x015;
    }
    else if (strcmp("$s6",reg) == 0){
        return 0x016;
    }
    else if (strcmp("$s7",reg) == 0){
        return 0x017;
    }
    else if (strcmp("$t8",reg) == 0){
        return 0x018;
    }
    else if (strcmp("$t9",reg) == 0){
        return 0x019;
    }
    else if (strcmp("$k0",reg) == 0){
        return 0x01A;
    }
    else if (strcmp("$k1",reg) == 0){
        return 0x01B;
    }
    else if (strcmp("$gp",reg) == 0){
        return 0x01C;
    }
    else if (strcmp("$sp",reg) == 0){
        return 0x01D;
    }
    else if (strcmp("$fp",reg) == 0){
        return 0x01E;
    }
    else if (strcmp("$ra",reg) == 0){
        return 0x01F;
    }
}


//void checkInstruction(char* instruction, FILE* fp){







