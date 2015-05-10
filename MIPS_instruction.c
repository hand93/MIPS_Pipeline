#include "MIPS_instruction.h"

int Instruction_Memory[(0x100000 / 4)];
int* PC;
int reg[32];
int Mem[1000000];
int Hi, Lo;


int SignExtImm(int imm){
	int s_bit;
	short s_imm; //16bit
	int s_ext_imm; //32bit

	s_bit = ((imm & 0x8000) >> 15);

	if (s_bit){
//		s_ext_imm = 0xffff0000|imm;
		s_imm = imm;
		s_ext_imm = s_imm;
//		printf("if %X\n", s_ext_imm);
	}
	else{
		s_ext_imm = imm;
//		printf("else %X\n", s_ext_imm);
	}

	return s_ext_imm;
}


int BranchAddr(int imm){
	return SignExtImm(imm) <<2;
}

int JumpAddr(int address){
	int i = (PC-&Instruction_Memory[0]) + 4;
	return (i & (0xF0000000)) | (address<<2);
}


int Add(int rs, int rt){
	return reg[rs] + reg[rt];
}

void Add_Immediate(int rs, int rt, int imm){
	reg[rt] = reg[rs] + SignExtImm(imm);
}


void Add_Imm_Unsigned(int rs, int rt, int imm){
	reg[rt] = reg[rs] + SignExtImm(imm);
	printf("addiu	r[%d], r[%d], %d\n", rt, rs, SignExtImm(imm));
}

void Add_Unsighed(int rd, int rs, int rt){
	reg[rd] = reg[rs] + reg[rt];
	printf("addu	r[%d], r[%d], r[%d]\n", rd, rs, rt);
}

void And(int rd, int rs, int rt){
	reg[rd] = reg[rs] & reg[rt];
}

void And_Immediate(int rs, int rt, int imm){
	reg[rt] = reg[rs] & imm;
}

void Branch_On_Equal(int rs, int rt, int imm){
	if (reg[rs] == reg[rt]){
		PC = PC + 1 + (BranchAddr(imm) /4);
		PC--;
	}
	printf("beq	r[%d], r[%d], <main+%X>\n", rs, rt, BranchAddr(imm));
}

void Branch_On_Not_Equal(int rs, int rt, int imm){
	if (reg[rs] != reg[rt]){
		PC = PC + 1 + (BranchAddr(imm)/4);
		PC--;
	}
	
}

void Branch_On_Not_Equal_Zero(int rs, int imm){
	if (reg[rs] != 0){
		PC = PC + 1 + (BranchAddr(imm) /4);
		PC--;
	}
	printf("bnez	r[%d], %X, <main + %X>\n", rs, BranchAddr(imm), BranchAddr(imm));
}


void Jump(int address){
	PC  = Instruction_Memory + (JumpAddr(address)/4);
	printf("j	%X <main+%X>\n", JumpAddr(address), JumpAddr(address));
	PC--;
}

void Jump_And_Link(int address){
	reg[31] = PC + 2;
	PC = Instruction_Memory + (JumpAddr(address) / 4);
	PC--;
	printf("jal	%X <main>\n", JumpAddr(address));
}

void Jump_Register(int rs){
	PC = reg[rs];
	PC--;
	printf("jr	r[%d]\n", rs);
}


void Load_Halfword_Unsigned(int rs, int rt, int imm){
	reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];
}

void Load_Linked(int rs, int rt, int imm){
	reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];
}

void Load_Upper_Imm(int rs, int rt, int imm){
	reg[rt] = imm >> 16;
}

void Load_Word(int rs, int rt, int imm){
	reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];
	printf("lw	r[%d], %d(%X)\n", rt, imm, rs);
}


void Nor(int rd, int rs, int rt){
	reg[rd] = ~(reg[rs] | reg[rt]);
}

void Or(int rd, int rs, int rt){
	reg[rd] = reg[rs] | reg[rt];
}

void Or_Immediate(int rs, int rt, int imm){
	reg[rt] = reg[rs] | imm;
}

void Set_Less_Than(int rd, int rs, int rt){
	reg[rd] = (reg[rs] < reg[rt]) ? 1 : 0;
}

void Set_Less_Than_Imm(int rs, int rt, int imm){
	reg[rt] = (reg[rs] < SignExtImm(imm)) ? 1 : 0;
	printf("slti	r[%d], r[%d], %d\n", rs, rt, imm);
}

void Set_Less_Than_Imm_Unsigned(int rs, int rt, int imm){
	reg[rt] = (reg[rs] < SignExtImm(imm)) ? 1 : 0;
}
void Set_Less_Than_Unsig(int rd, int rs, int rt){
	reg[rd] = (reg[rs] < reg[rt]) ? 1 : 0;
}

void Shift_Left_Logical(int rd, int rt, int sh){
	reg[rd] = reg[rt] << sh;
}

void Shith_Right_Logical(int rd, int rt, int sh){
	reg[rd] = reg[rt] >> sh;
}

void Store_Conditional(int rs, int rt, int imm){
//	Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
	//reg[rt] = (atomic) ? 1 : 0;
}

void Store_Halfword(int rs, int rt, int imm){
//	Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
}

void Store_Word(int rs, int rt, int imm){
	Mem[(reg[rs] + SignExtImm(imm))] = reg[rt];
	printf("sw	r[%d], %d(r[%d])\n", rt, imm, rs);
}

void Substract(int rd, int rs, int rt){
	reg[rd] = reg[rs] - reg[rt];
}

void Substract_Unsigned(int rd, int rs, int rt){
	reg[rd] = reg[rs] - reg[rt];
}

void Divide(int rd, int rs, int rt){
	Lo = reg[rs] / reg[rt];
	Hi = reg[rs] % reg[rt];
}

void Divide_Unsigned(int rd, int rs, int rt){
	Lo = reg[rs] / reg[rt];
	Hi = reg[rs] % reg[rt];
}

void Move_From_Hi(int rd){
	reg[rd] = Hi;
}

void Move_From_Low(int rd){
	reg[rd] = Lo;
}

void Multiply(int rs, int rt){
	Hi = ((reg[rs] * reg[rt]) & 0xFFFFFFFF00000000) >> 32;
	Lo = ((reg[rs] * reg[rt]) & 0x00000000FFFFFFFF);
}

void Multiply_Unsigned(int rs, int rt){
	Hi = ((reg[rs] * reg[rt]) & 0xFFFFFFFF00000000) >> 32;
	Lo = ((reg[rs] * reg[rt]) & 0x00000000FFFFFFFF);
}

/*void Shift_Right_Arith(int rd, int rt, int shamt){
	reg[rd] = reg[rt] >>> shamt;
}*/






//prsedo instruction set
void Move(int rd, int rs){
	reg[rd] = reg[rs];
	printf("move	reg[%d], reg[%d]\n", rd, rs);
}

void Load_Immediate(int rt, int immediate){
	reg[rt] = immediate;
	printf("li	r[%d] = %X\n", rt, immediate);
}
