//read file in one at a time
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

int NUM_INSTRUCTIONS = 0;

unsigned int r_type_format(unsigned int instruction, unsigned int rd, unsigned int rs, unsigned int rt);
unsigned int i_type_format(unsigned int instruction, unsigned int rs, unsigned int rt, unsigned int offset);
unsigned int registerLookup(char* reg);
unsigned int getRegister(char* reg);
unsigned int hexInstruction=0;


int main(int argc, char *argv[]) {
    unsigned int rd=0,rt=0,rs=0, immediate=0;
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
            // r_type_format(0x20,reg1,reg2,offset); 
            rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
            hexInstruction = r_type_format(0x20,rd,rs,rt);
            printf("ADD: %x\n",hexInstruction);
            
		}
		else if (strcmp("addu", instruction)== 0){//
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    r_type_format(0x21,rd,rs,rt);
		}
		else if (strcmp("addi", instruction)== 0){//
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    i_type_format(0x08,rd,rt,immediate);
		}
		else if (strcmp("addiu", instruction)== 0){//
		  //  i_type_format(0x09,reg1,reg2,offset);
		    rs = getRegister(reg1);
            rt = getRegister(reg2);
            immediate = strtol(offset,NULL,16);//getRegister(offset);
            hexInstruction = i_type_format(0x09,rs,rt,immediate);
            printf("ADDIU: %x\nrs: %x, rt: %x, immediate: %x\n",hexInstruction,rs,rt,immediate);
		}
		else if (strcmp("sub", instruction)== 0){
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x22,rd,rs,rt);
		}
		else if (strcmp("subu", instruction)== 0){
		    //0x23
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x23,rd,rs,rt);
		}
		else if (strcmp("mult", instruction)== 0){
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
		    hexInstruction = r_type_format(0x18,rd,rs,0x0);
		}
		else if (strcmp("multu", instruction)== 0){
		    //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
		    hexInstruction = r_type_format(0x19,rd,rs,0x0);
		}
		else if (strcmp("div", instruction)== 0){
		  //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
		    hexInstruction = r_type_format(0x1A,rd,rs,0x0);
		}
		else if (strcmp("divu", instruction)== 0){
		    //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
		    hexInstruction = r_type_format(0x1B,rd,rs,0x0);
		}
		else if (strcmp("and", instruction)== 0){//
		  //  0x24
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x24,rd,rs,rt);
		}
		else if (strcmp("andi", instruction)== 0){
		  //  0xC
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    hexInstruction = i_type_format(0xC,rs,rt,immediate);
		}
		else if (strcmp("or", instruction)== 0){
		    //0x25
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x25,rd,rs,rt);
		}
		else if (strcmp("ori", instruction)== 0){
		  //  0xD
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    hexInstruction = i_type_format(0xD,rs,rt,immediate);
		}
		else if (strcmp("xor", instruction)== 0){
		  //  0x26
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x26,rd,rs,rt);
		}
		else if (strcmp("xori", instruction)== 0){
		  //  0xE
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    hexInstruction = i_type_format(0xE,rs,rt,immediate);
		}
		else if (strcmp("nor", instruction)== 0){
		  //  0x27
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x27,rd,rs,rt);
		}
		else if (strcmp("slt", instruction)== 0){
		  //  0x2A
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x2A,rd,rs,rt);
		}
		else if (strcmp("slti", instruction)== 0){
		  //  0xA
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    hexInstruction = i_type_format(0xA,rs,rt,immediate);
		}
		else if (strcmp("sll", instruction)== 0){
		  //  0x0
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            rt = getRegister(offset);
		    hexInstruction = r_type_format(0x0,rd,rs,rt);
		}
		else if (strcmp("srl", instruction)== 0){
		  //  0x02
		    //special case
		}
		else if (strcmp("sra", instruction)== 0){
		  //  0x3
		  //special case
		}
		else if (strcmp("lw", instruction)== 0){
		  //  0x23
		    rd = getRegister(reg1);
            rs = getRegister(reg2);
            immediate = getRegister(offset);
		    hexInstruction = i_type_format(0x23,rs,rt,immediate);
		}
		else if (strcmp("lb", instruction)== 0){
		  //  0x20
		}
		else if (strcmp("lh", instruction)== 0){
		  //  0x21
		}
		else if (strcmp("lui", instruction)== 0){
		  //  0xF
		}
		else if (strcmp("sw", instruction)== 0){
		  //  0x2B
		}
		else if (strcmp("sb", instruction)== 0){
		  //  0x28
		}
		else if (strcmp("sh", instruction)== 0){
		  //  0x29
		}
		else if (strcmp("mfhi", instruction)== 0){
		  //  0x10
		}
		else if (strcmp("mflo", instruction)== 0){
		  //  0x12
		}
		else if (strcmp("mthi", instruction)== 0){
		  //  0x11
		}
		else if (strcmp("mtlo", instruction)== 0){
		  //  0x13
		}
		else if (strcmp("beq", instruction)== 0){
		  //  0x04
		}
		else if (strcmp("bne", instruction)== 0){
		  //  0x5
		}
		else if (strcmp("blez", instruction)== 0){
		  //  0x6
		}
		else if (strcmp("bltz", instruction)== 0){
		  //  0x01
		}
		else if (strcmp("bgez", instruction)== 0){
		  //  0x01
		}
		else if (strcmp("bgtz", instruction)== 0){
		  //  0x07
		}
		else if (strcmp("j", instruction)== 0){
		  //  0x02
		}
		else if (strcmp("jr", instruction)== 0){
		  //  0x08
		}
		else if (strcmp("jal", instruction)== 0){
		  //  0x03
		}
		else if (strcmp("jalr", instruction)== 0){
		  //  0x05
		}
		else if (strcmp("syscall", instruction)== 0){
		  //  0x0C
		}
		
		else{
                printf("Did not find instruction\n");
        }
		

	
	NUM_INSTRUCTIONS++;           
        }
free(instruction);
free(reg1);
free(reg2);            
return 0;
         
}

unsigned int getRegister(char* reg){
    return registerLookup(reg);
}

unsigned int r_type_format(unsigned int instruction, unsigned int rd, unsigned int rs, unsigned int rt){
    return (rs << 21) | (rt << 16) | (rd << 11) | instruction;
    
}

unsigned int i_type_format(unsigned int instruction, unsigned int rs, unsigned int rt, unsigned int offset){
    // unsigned int r1 = registerLookup(reg1);
    // unsigned int r2 = registerLookup(reg2);
    // unsigned long immediate;
    // immediate = strtol(offset,NULL,16);
    // printf("instruction: %ld\nreg1: %x\nreg2: %x\noffset: %ld\n",instruction,r1,r2,immediate);
    return ((instruction << 26) | (rs << 21) | (rt << 16) | offset);
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







