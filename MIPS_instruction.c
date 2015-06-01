
#include "MIPS_instruction.h"

int SignExtImm(int imm){
	int s_bit;
	short s_imm; //16bit
	int s_ext_imm; //32bit

	s_bit = ((imm & 0x8000) >> 15);

	if (s_bit){
		s_imm = imm;
		s_ext_imm = s_imm;
	}
	else{
		s_ext_imm = imm;
	}

	return s_ext_imm;
}


int BranchAddr(int imm){
	return (SignExtImm(imm) <<2);
}

int JumpAddr(int address, int PC){
	int i = PC + 4;
	return ((i & (0xF0000000)) | (address<<2));
}


