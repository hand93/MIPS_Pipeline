#include <stdio.h>
#include "MIPS_instruction.h"

extern int Instruction_Memory[(0x100000 / 4)] = { 0 };
//extern int* PC = (&Instruction_Memory[0]);
extern int reg[32] = { 0 };
extern int Mem[1000000] = { 0 };
extern int Hi, Lo = 0;
enum regNum { zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra };
//s8 == fp
void swapbit(int* ptr); //윈도우에서 거꾸로 읽는 것을 원래대로 만들어줌.
void Memory_print();

void Instruction_Fetch(FILE* pFile, IF_ID* if_id);
void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex);
void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem);
void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb);


//input output latch를 배열로 두개씩 만들어 놓았다.
//이전 instrucion을 확인하려고 Execution (IF_ID, ID_EX, EX_MEM ...) 다 받음
//if_latch[1]=if_latch[0]


int main(){
	FILE* spData;
	reg[sp] = 0x8000;
	reg[ra] = 0xFFFFFFFF;
	int i;

	IF_ID* if_id;
	ID_EX* id_ex;
	EX_MEM* ex_mem;
	MEM_WB* mem_wb;

	spData = fopen("fibo.bin", "r");

	if(spData==NULL){
		printf("Could not open the file.\n");
	}

	if_id->PC = (&Instruction_Memory[0]);

	Instruction_Fetch(spData, if_id);

//	while (PC != 0xFFFFFFFF){
	Instruction_Decode(if_id, id_ex);
//		PC++;
//	}
	Instruction_Execution(id_ex, ex_mem);
	Memory(ex_mem, mem_wb);
	
	Memory_print();
	fclose(spData);

	return 0;
}

void swapbit(int* ptr) {
	unsigned int tmp1, tmp2;
//	unsigned int tmp3, tmp4;
	tmp1 = ((*ptr)&0xff000000)>>24;
	tmp2 = ((*ptr)&0x000000ff)<<24;
	*ptr &= 0x00ffff00;
	*ptr |= tmp2;
	*ptr |= tmp1;

	tmp1 = ((*ptr)&0x00ff0000)>>8;
	tmp2 = ((*ptr)&0x0000ff00)<<8;

//	*ptr = tmp1 + tmp2 + tmp3 + tmp4;
	*ptr &= 0xff0000ff;
	*ptr |= tmp1;
	*ptr |= tmp2;
}

void Memory_print(){
	int i;
	for (i = 0; i < 1000000; i++){
		if (Mem[i] != 0){
			printf("                          Mem[%d] : %d \n", i, Mem[i]);
		}
	}
}



void Instruction_Fetch(FILE* pFile, IF_ID* if_id){
	int i=0;
	while( !feof(pFile) ){
		fread(Instruction_Memory+i,4,1,pFile);
		printf("Instruction[%02d] = %08x ", i, Instruction_Memory[i]);
		swapbit(Instruction_Memory+i);
		printf("=> %08x \n", Instruction_Memory[i]);
		if_id->instruction = Instruction_Memory[i];
		i++;
	}

	//PC 건들이기
	//WB stage에서 PC가 0이 되면 종료
}

void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex){
	int inst = if_id->instruction;
	int opcode, rs, rt, rd, shamt, funct = 0;
	int immediate=0;
	int address=0;

	id_ex->PC = if_id->PC;

	opcode = ((inst)&0xFC000000)>>26;
	rs = ((inst)&0x03E00000)>>21;
	rt = ((inst)&0x001F0000)>>16;
	rd = ((inst)&0x0000F800)>>11;
	shamt = ((inst)&0x000007C0)>>6;
	funct = ((inst)&0x0000003F);
	immediate = ((inst)&0x0000FFFF);
	address = ((inst)&0x03FFFFFF);

	if (inst == 0x00000000){
		printf("nop\n");
	}else{
		id_ex->opcode = opcode;
		id_ex->rs_data = reg[rs];
		id_ex->rt_data = reg[rt];
		id_ex->SignExt = SignExtImm(immediate);
		id_ex->rt_address = rt;
		id_ex->rd_address = rd;
		id_ex->sh = shamt;
		id_ex->func = funct;
	}
}

void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem){

	ex_mem->PC = id_ex->PC;

	switch (id_ex->opcode){
	case 0x00:
		switch (id_ex->sh){
		case 0x20:
			ex_mem->ALU_result = id_ex->rs_data + id_ex->rt_data;  //Add
			break;
		case 0x21: 
			if (id_ex->rt_data == 0){
				ex_mem->rd_address = id_ex->rs_data; //Move
			}
			else {
				ex_mem->ALU_result = id_ex->rs_data + id_ex->rt_data; //Add Unsigned
			}
			break;
		case 0x24:
			ex_mem->ALU_result = id_ex->rs_data & id_ex->rt_data;  //And
			break;
		case 0x08:
			Jump_Register(rs); //Jump_Register부터 고치기
			break;
		case 0x27:
			Nor(rd, rs, rt);
			break;
		case 0x25:
			Or(rd, rs, rt);
			break;
		case 0x2a:
			Set_Less_Than(rd, rs, rt);
			break;
		case 0x2b:
			Set_Less_Than_Imm_Unsigned(rd, rs, rt);
			break;
		case 0x00:
			Shift_Left_Logical(rd, rt, sh);
			break;
		case 0x02:
			Shith_Right_Logical(rd, rt, sh);
			break;
		case 0x22:
			Substract(rd, rs, rt);
			break;
		case 0x23:
			Substract_Unsigned(rd, rs, rt);
			break;
		case 0x10:
			Move_From_Hi(rd);
			break;
		case 0x12:
			Move_From_Low(rd);
			break;
		case 0x18:
			Multiply(rs, rt);
			break;
		case 0x19:
			Multiply_Unsigned(rs, rt);
			break;
//		case 0x03:
//			Shift_Right_Arith(rd, rt, sh);

		}
		break;
	case 0x08:
		Add_Immediate(rs, rt, immediate);
		break;
	case 0x09:
		if (rs != 0){
			Add_Imm_Unsigned(rs, rt, immediate);
		}
		else Load_Immediate(rt, immediate);
		break;
	case 0x0c:
		And_Immediate(rs, rt, immediate);
		break;
	case 0x04:
		Branch_On_Equal(rs, rt, immediate);
		break;
	case 0x05:
		if (rt != 0){
			Branch_On_Not_Equal(rs, rt, immediate);
		}else Branch_On_Not_Equal_Zero(rs, immediate);

		break;
	case 0x02:
		Jump(address);
		break;
	case 0x03:
		Jump_And_Link(address);
		break;
	case 0x25:
		Load_Halfword_Unsigned(rt, rs, immediate);
		break;
	case 0x30:
		Load_Linked(rs, rt, immediate);
		break;
	case 0x0f:
		Load_Upper_Imm(rs, rt, immediate);
		break;
	case 0x23:
		Load_Word(rs, rt, immediate);
		break;
	case 0x0d:
		Or_Immediate(rs, rt, immediate);
		break;
	case 0x0a:
		Set_Less_Than_Imm(rs, rt, immediate);
		break;
	case 0x0b:
		Set_Less_Than_Imm_Unsigned(rs, rt, immediate);
		break;
	case 0x38:
		Store_Conditional(rs, rt, immediate);
		break;
	case 0x29:
		Store_Halfword(rs, rt, immediate);
		break;
	case 0x2b:
		Store_Word(rs, rt, immediate);
		break;
	}
}






