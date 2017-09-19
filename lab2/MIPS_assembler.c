//read file in one at a time
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

int NUM_INSTRUCTIONS = 0;

unsigned int r_type_format(unsigned int instruction, unsigned int rd, unsigned int rs, unsigned int rt);
unsigned int i_type_format(unsigned int instruction, unsigned int rs, unsigned int rt, char* offset);
unsigned int j_type_format(unsigned int instruction, unsigned int target);
void printToFile(unsigned int hexInstruction);
char* getNext (FILE* fp);
unsigned int registerLookup(char* reg);
unsigned int getRegister(char* reg);
unsigned int hexInstruction=0;

char* nextString;

int main(int argc, char *argv[]) {
	
	if(argc < 2){
		printf("Not enought arguments\n");
		return 0;
	}
    unsigned int rd=0,rt=0,rs=0, immediate=0, base = 0;
    char* instruction = malloc(sizeof(char)*10);
    char* reg1 = malloc(sizeof(char)*5);
    char* reg2 = malloc(sizeof(char)*5);
    char* offset = malloc(sizeof(char)*5);
    nextString = malloc(sizeof(char*)*5);
    FILE *fp = fopen(argv[1], "r");
    FILE* pfile = fopen("output.in", "w");
    fclose(pfile);
    
    // while (fscanf(fp, "%s %s %s %s\n", instruction, reg1, reg2, offset) != EOF){
    while (fscanf(fp, "%s", instruction) != EOF){

		if (strcmp("add", instruction)== 0){//
            //format: instruction, reg1, reg2, reg3
            // r_type_format(0x20,reg1,reg2,offset); 
            rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
            hexInstruction = r_type_format(0x20,rd,rs,rt);
            printf("ADD: %x\n",hexInstruction);
            
		}
		else if (strcmp("addu", instruction)== 0){//
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x21,rd,rs,rt);
		}
		else if (strcmp("addi", instruction)== 0){//
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = getRegister(offset);
            strcpy(offset, getNext(fp));
		    hexInstruction = i_type_format(0x08,rd,rt,offset);
		}
		else if (strcmp("addiu", instruction)== 0){//
		  //  i_type_format(0x09,reg1,reg2,offset);
		    rs = getRegister(strcpy(reg1, getNext(fp)));
            rt = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = strtol(offset,NULL,16);
            strcpy(offset, getNext(fp));
            hexInstruction = i_type_format(0x09,rs,rt,offset);
            printf("ADDIU: %x\nrs: %x, rt: %x, immediate: %x\n",hexInstruction,rs,rt,immediate);
		}
		else if (strcmp("sub", instruction)== 0){
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x22,rd,rs,rt);
		}
		else if (strcmp("subu", instruction)== 0){
		    //0x23
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x23,rd,rs,rt);
		}
		else if (strcmp("mult", instruction)== 0){
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
		    hexInstruction = r_type_format(0x18,rd,rs,0x0);
		}
		else if (strcmp("multu", instruction)== 0){
		    //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
		    hexInstruction = r_type_format(0x19,rd,rs,0x0);
		}
		else if (strcmp("div", instruction)== 0){
		  //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
		    hexInstruction = r_type_format(0x1A,rd,rs,0x0);
		}
		else if (strcmp("divu", instruction)== 0){
		    //since there are only 2 registers used, pass rt as 0x0
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
		    hexInstruction = r_type_format(0x1B,rd,rs,0x0);
		}
		else if (strcmp("and", instruction)== 0){//
		  //  0x24
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x24,rd,rs,rt);
		}
		else if (strcmp("andi", instruction)== 0){
		  //  0xC
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = getRegister(offset);
            strcpy(offset, getNext(fp));
		    hexInstruction = i_type_format(0xC,rs,rt,offset);
		}
		else if (strcmp("or", instruction)== 0){
		    //0x25
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x25,rd,rs,rt);
		}
		else if (strcmp("ori", instruction)== 0){
		  //  0xD
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = getRegister(offset);
            strcpy(offset, getNext(fp));
		    hexInstruction = i_type_format(0xD,rs,rt,offset);
		}
		else if (strcmp("xor", instruction)== 0){
		  //  0x26
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x26,rd,rs,rt);
		}
		else if (strcmp("xori", instruction)== 0){
		  //  0xE
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = getRegister(offset);
            strcpy(offset, getNext(fp));
		    hexInstruction = i_type_format(0xE,rs,rt,offset);
		}
		else if (strcmp("nor", instruction)== 0){
		  //  0x27
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x27,rd,rs,rt);
		}
		else if (strcmp("slt", instruction)== 0){
		  //  0x2A
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
		    hexInstruction = r_type_format(0x2A,rd,rs,rt);
		}
		else if (strcmp("slti", instruction)== 0){
		  //  0xA
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            // immediate = getRegister(offset);
            strcpy(offset, getNext(fp));
		    hexInstruction = i_type_format(0xA,rs,rt,offset);
		}
		else if (strcmp("sll", instruction)== 0){
		  //  0x0
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            rt = getRegister(strcpy(offset, getNext(fp)));
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
		    rd = getRegister(strcpy(reg1, getNext(fp)));
            rs = getRegister(strcpy(reg2, getNext(fp)));
            strcpy(offset, getNext(fp));
            // immediate = getRegister(offset);
		    hexInstruction = i_type_format(0x23,rs,rt,offset);
		}
		else if (strcmp("lb", instruction)== 0){
		  //  0x20
		  base = getRegister(strcpy(reg1, getNext(fp)));//actually the base
		  rt = getRegister(strcpy(reg2, getNext(fp)));
		  //immediate	= getRegister(offset);
		  strcpy(offset, getNext(fp));
		  hexInstruction = i_type_format(0x20,base,rt,offset);
		}
		else if (strcmp("lh", instruction)== 0){
		  //  0x21
		  base = getRegister(strcpy(reg1, getNext(fp)));
		  rt = getRegister(strcpy(reg2, getNext(fp)));
		  //immediate = getRegister(offset);
		  strcpy(offset, getNext(fp));
		  hexInstruction = i_type_format(0x21,base,rt,offset);
		}
		else if (strcmp("lui", instruction)== 0){
		  //  0xF
		  // This instruction is (inst, 00000,rt,immediate)
		  rt = getRegister(strcpy(reg1, getNext(fp)));
		  //immediate = getRegister(reg2);
		  strcpy(offset, getNext(fp));
		  hexInstruction = i_type_format(0xF,0x00000,rt,offset);
		}
		else if (strcmp("sw", instruction)== 0){
		  //  0x2B
		   base = getRegister(strcpy(reg1, getNext(fp)));
		   rt = getRegister(strcpy(reg2, getNext(fp)));
		   //immediate = getRegister(offset);
		   strcpy(offset, getNext(fp));
		   hexInstruction = i_type_format(0x2B,base,rt,offset);
		}
		else if (strcmp("sb", instruction)== 0){
		  //  0x28
		  base = getRegister(strcpy(reg1, getNext(fp)));
		  rt = getRegister(strcpy(reg2, getNext(fp)));
		  //immediate = getRegister(offset);
		  strcpy(offset, getNext(fp));
		  hexInstruction = i_type_format(0x28,base,rt,offset);
		}
		else if (strcmp("sh", instruction)== 0){
		  //  0x29
		  base = getRegister(strcpy(reg1, getNext(fp)));
		  rt = getRegister(strcpy(reg2, getNext(fp)));
		  //immediate = getRegister(offset);
		  strcpy(offset, getNext(fp));
		  hexInstruction = i_type_format(0x29,base,rt,offset);
		}
		else if (strcmp("mfhi", instruction)== 0){
		  //  0x10
		  strcpy(reg1, getNext(fp));
		  rd = getRegister(reg1);
		  hexInstruction = r_type_format(0x10,0x0,0x0,rd);
		}
		else if (strcmp("mflo", instruction)== 0){
		  //  0x12
		  strcpy(reg1, getNext(fp));
		  rd = getRegister(reg1);
		  hexInstruction = r_type_format(0x12,0x0,0x0,rd);
		}
		else if (strcmp("mthi", instruction)== 0){
		  //  0x11
		  strcpy(reg1, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = r_type_format(0x11,rs,0x0,0x0);
		}
		else if (strcmp("mtlo", instruction)== 0){
		  //  0x13
		  strcpy(reg1, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = r_type_format(0x13,rs,0x0,0x0);
		}
		else if (strcmp("beq", instruction)== 0){
		  //  0x04
		  strcpy(reg1, getNext(fp));
		  strcpy(reg2, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  rt = getRegister(reg2);
		  hexInstruction = i_type_format(0x04,rs,rt,offset);
		}
		else if (strcmp("bne", instruction)== 0){
		  //  0x5
		  strcpy(reg1, getNext(fp));
		  strcpy(reg2, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  rt = getRegister(reg2);
		  hexInstruction = i_type_format(0x05,rs,rt,offset);
		}
		else if (strcmp("blez", instruction)== 0){
		  //  0x6
		  strcpy(reg1, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = i_type_format(0x6,rs,0x0,offset);
		}
		else if (strcmp("bltz", instruction)== 0){
		  //  0x01
		  strcpy(reg1, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = i_type_format(0x01,rs,0x0,offset);
		}
		else if (strcmp("bgez", instruction)== 0){
		  //  0x01
		  strcpy(reg1, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = i_type_format(0x01,rs,0x1,offset);
		}
		else if (strcmp("bgtz", instruction)== 0){
		  //  0x07
		  strcpy(reg1, getNext(fp));
		  strcpy(offset, getNext(fp));
		  rs = getRegister(reg1);
		  hexInstruction = i_type_format(0x07,rs,0x0,offset);
		}
		else if (strcmp("j", instruction)== 0){
		  //  0x02
		  strcpy(reg1, getNext(fp));
		  hexInstruction = j_type_format(0x02,strtol(reg1,NULL, 16));

		}
		else if (strcmp("jr", instruction)== 0){
		  //  0x08
		  strcpy(reg1, getNext(fp));
		 rs = getRegister(reg1);//the adress jumping to is in rs
		 hexInstruction = r_type_format(0x08,0x0, rs, 0x0);
		}
		else if (strcmp("jal", instruction)== 0){
		  //  0x03
		  //rt = getRegister(reg1);//actually target
		  strcpy(offset, getNext(fp));
		  hexInstruction = j_type_format(0x03,strtol(offset,NULL, 16));
		}
		else if (strcmp("jalr", instruction)== 0){
			//  0x09
			//NEEDS FIXINGS
		}
		else if (strcmp("syscall", instruction)== 0){
		  //  0x0C
		   rd = 0x0;
           rs = 0x0;
           rt = 0x0;
		   hexInstruction = r_type_format(0xC,rd,rs,rt);
		}
		
		else{
                printf("Did not find instruction\n");
        }

	printToFile(hexInstruction);
	NUM_INSTRUCTIONS++;
    }
free(instruction);
free(reg1);
free(reg2);
free(nextString);
fclose (fp);
return 0;
         
}

unsigned int getRegister(char* reg){
    return registerLookup(reg);
}

unsigned int r_type_format(unsigned int instruction, unsigned int rd, unsigned int rs, unsigned int rt){
    return (rs << 21) | (rt << 16) | (rd << 11) | instruction;
}

unsigned int i_type_format(unsigned int instruction, unsigned int rs, unsigned int rt, char* offset){
    // unsigned int r1 = registerLookup(reg1);
    // unsigned int r2 = registerLookup(reg2);
    // unsigned long immediate;
    // immediate = strtol(offset,NULL,16);
    // printf("instruction: %ld\nreg1: %x\nreg2: %x\noffset: %ld\n",instruction,r1,r2,immediate);
    unsigned int immediate = strtol(offset,NULL,16);
    return ((instruction << 26) | (rs << 21) | (rt << 16) | immediate);
}

unsigned int j_type_format(unsigned int instruction, unsigned int target){
		//target = strol(target, NULL, 16);
		return ((instruction << 26) | target);
}

char* getNext (FILE* fp){
	
	fscanf(fp, "%s", nextString);
	strcpy(nextString,strtok(nextString,","));
	return nextString;
}

void printToFile(unsigned int hexInstruction){
	FILE* fp = fopen("output.in", "a+");
	fprintf(fp, "%x\n", hexInstruction);
	fclose(fp);
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







