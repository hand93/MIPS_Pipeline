#include <stdio.h>
#include "MIPS_instruction.h"

extern int Instruction_Memory[(0x100000 / 4)] = { 0 };
//extern int* PC = (&Instruction_Memory[0]);
extern int reg[32] = { 0 };
extern int Mem[1000000] = { 0 };
extern int Hi, Lo = 0;
enum regNum { zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra };
//s8 == fp

int pcSrc1 = 0;
int pcSrc2 = 0;
int regDst = 0;
int jump = 0;
int branch = 0;
int memRead = 0;
int memtoReg = 0;
int ALUop = 0;
int memWrite = 0;
//int ALUSrc = 0; 
int regWrite = 0;

void swapbit(int* ptr); //윈도우에서 거꾸로 읽는 것을 원래대로 만들어줌.
void Memory_print();

void Instruction_Fetch(FILE* pFile, IF_ID* if_id);
void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex);
void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem);
void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb);
void Write_Back(MEM_WB* mem_wb);


//input output latch를 배열로 두개씩 만들어 놓았다.
//이전 instrucion을 확인하려고 Execution (IF_ID, ID_EX, EX_MEM ...) 다 받음
//if_latch[1]=if_latch[0]


#define Add 0x0
#define Add_Immediate 0x8
#define Add_Imm_Unsigned 0x9
#define Add_Unsigned 0x0
#define And 0x0
#define And_Immediate 0xc
#define Branch_On_Equal 0x4
#define Branch_On_Not_Equal 0x5
#define Jump 0x2
#define Jump_And_Link 0x3
#define Jump_Register 0x0
#define Load_Byte_Unsigned 0x24
#define Load_Halfword_Unsigned 0x25
#define Load_Linked 0x30
#define Load_Upper_Imm 0xf
#define Load_Word 0x23
#define Nor 0x0
#define Or 0x0
#define Or_Immediate 0xd
#define Set_Less_Than 0x0
#define Set_Less_Than_Imm 0xa
#define Set_Less_Than_Imm_Unsigned 0xb
#define Set_Less_Than_Unsig 0x0
#define Shift_Left_Logical 0x0
#define Shith_Right_Logical 0x0
#define Store_Byte 0x28
#define Store_Conditional 0x38
#define Store_Halfword 0x29
#define Store_Word 0x2b
#define Subtract 0x0
#define Suntract_Unsigned 0x0
#define Divide 0x0
#define Divied_Unsigned 0x0
#define Load_FP_Single 0x31
#define Load_FP 0x35
#define Double1 0x35
#define Move_From_Hi 0x0
#define Move_From_Lo 0x0
#define Move_From_Control 0x10
#define Multiply 0x0
#define Multiply_Unsigned 0x0
#define Shift_Right_Arith 0x0
#define Store_FP_Single 0x39
#define Store_FP 0x3d
#define Double2 0x3d


int main(){
	FILE* spData;
	reg[sp] = 0x8000;
	reg[ra] = 0xFFFFFFFF;
	int i;

	IF_ID* if_id;
	ID_EX* id_ex;
	EX_MEM* ex_mem;
	MEM_WB* mem_wb;

	spData = fopen("temp.bin", "r");

	if(spData==NULL){
		printf("Could not open the file.\n");
	}

	if_id->PC = (&Instruction_Memory[0]);

	Instruction_Fetch(spData, if_id);
	Instruction_Decode(if_id, id_ex);
	Instruction_Execution(id_ex, ex_mem);
	Memory(ex_mem, mem_wb);
	
	Memory_print();
	fclose(spData);

	return 0;
}

void swapbit(int* ptr) {
	unsigned int tmp1, tmp2;

	tmp1 = ((*ptr)&0xff000000)>>24;
	tmp2 = ((*ptr)&0x000000ff)<<24;
	*ptr &= 0x00ffff00;
	*ptr |= tmp2;
	*ptr |= tmp1;

	tmp1 = ((*ptr)&0x00ff0000)>>8;
	tmp2 = ((*ptr)&0x0000ff00)<<8;
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


	//Control
	if(opcode == 0){
		switch (shamt)
		{
		case 0x20:
		case 0x21:
		case 0x24:
		case 0x27:
		case 0x25:
		case 0x00:
		case 0x02:
		case 0x22:
		case 0x23:
		case 0x10:
		case 0x12:
			regDst = 1; //rd
			break;
		}
	}
	else if ((opcode == Load_Word) || (opcode == Load_Byte_Unsigned) || (opcode == Load_FP) || (opcode == Load_FP_Single) || (opcode == Load_Linked) || (opcode == Load_Upper_Imm)){
		memRead = 1;
		memtoReg = 1;
	}
	else if ((opcode == Store_Word) || (opcode == Store_Byte) || (opcode == Store_Conditional) || (opcode == Store_FP) || (opcode == Store_FP_Single) || (opcode == Store_Halfword)){
		memWrite = 1;
	}


	//if (inst == 0x00000000){
	//	printf("nop\n");
	//}else{
		id_ex->opcode = opcode;
		id_ex->rs_data = reg[rs];
		id_ex->rt_data = reg[rt];
		id_ex->r31_data = reg[31];
		id_ex->SignExt = SignExtImm(immediate);
		id_ex->immediate = immediate;
		if (regDst == 1){
			id_ex->rt_rd_address = rd;
		}
		else id_ex->rt_rd_address = rt;
		id_ex->j_address = Instruction_Memory + (JumpAddr(address) / 4);
		id_ex->sh = shamt;
		id_ex->func = funct;
	//}

}

void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem){

	ex_mem->PC = id_ex->PC;
	ex_mem->rt_data = id_ex->rt_data;
	ex_mem->address = id_ex->rt_rd_address;

	int ALU_input;

	switch (id_ex->opcode){
	case 0x00:
		switch (id_ex->sh){
		case 0x20:
			ex_mem->ALU_result = id_ex->rs_data + id_ex->rt_data;  //Add
			break;
		case 0x21: 
			if (id_ex->rt_data == 0){
				ex_mem->ALU_result = id_ex->rs_data; //Move
			}
			else {
				ex_mem->ALU_result = id_ex->rs_data + id_ex->rt_data; //Add Unsigned
			}
			break;
		case 0x24:
			ex_mem->ALU_result = id_ex->rs_data & id_ex->rt_data;  //And
			break;
		case 0x08:
//			Jump_Register(); /////////////////////////////////////////////////////Jump_Register 수정
			break;
		case 0x27:
			ex_mem->ALU_result = ~(id_ex->rs_data | id_ex->rt_data); //Nor

			break;
		case 0x25:
			ex_mem->ALU_result = id_ex->rs_data | id_ex->rt_data; //Or
			break;
		case 0x2a:
			ex_mem->rt_data = (id_ex->rs_data < id_ex->rt_data) ? 1 : 0; // Set_Less_Than
			break;
		case 0x2b:
			ex_mem->rt_data = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0; // Set_Less_Than_Imm_Unsigned
			break;
		case 0x00:
			ex_mem->ALU_result = id_ex->rt_data << id_ex->sh; //Shift_Left_Logical
			break;
		case 0x02:
			ex_mem->ALU_result = id_ex->rt_data >> id_ex->sh; //Shift_Right_Logical
			break;
		case 0x22:
			ex_mem->ALU_result = id_ex->rs_data - id_ex->rt_data; //Subtract
			break;
		case 0x23:
			ex_mem->ALU_result = id_ex->rs_data - id_ex->rt_data; //Subtract_Unsigned
			break;
		case 0x10:
			ex_mem->ALU_result = Hi; //Move_From_Hi
			break;
		case 0x12:
			ex_mem->ALU_result = Lo; //Move_From_Low
			break;
		case 0x18:
			Hi = ((id_ex->rs_data * id_ex->rt_data) & 0xFFFFFFFF00000000) >> 32; //Multiply
			Lo = ((id_ex->rs_data * id_ex->rt_data) & 0x00000000FFFFFFFF);
			break;
		case 0x19:
			Hi = ((id_ex->rs_data * id_ex->rt_data) & 0xFFFFFFFF00000000) >> 32; //Multiply_Unsigned
			Lo = ((id_ex->rs_data * id_ex->rt_data) & 0x00000000FFFFFFFF);
			break;
//		case 0x03:
//			ex_mem->ALU_result = id_ex->rt_data >>> id_ex->sh; //Shift_Right_Arith
//			ex_mem->address = id_ex->rd_address;
		}
		break;
	case Add_Immediate:
		ex_mem->rt_data = id_ex->rs_data & id_ex->SignExt;
		break;
	case 0x09:
		if (id_ex->rs_data != 0){
			ex_mem->rt_data = id_ex->rs_data & id_ex->SignExt; //Add_imm_Unsigned
		}
		else{
			ex_mem->rt_data = id_ex->immediate; //Load_Immediate
		}
		break;
	case And_Immediate:
		ex_mem->rt_data = id_ex->rs_data & id_ex->immediate;

		break;
	case Branch_On_Equal:
		if (id_ex->rs_data == id_ex->rt_data){
			ex_mem->PC = ex_mem->PC + 1 + (id_ex->SignExt / 4);
			pcSrc2 = 1;
		}
		break;
	case 0x5:
		if (id_ex->rt_data != 0){
			if (id_ex->rs_data != id_ex->rt_data){ //Branch_On_Not_Equal
				ex_mem->PC = ex_mem->PC + 1 + (id_ex->SignExt / 4);
				pcSrc2 = 1;
			}
		}
		else{
			if (id_ex->rs_data != 0){ //Branch_On_Not_Equl_Zero
				ex_mem->PC = ex_mem->PC + 1 + (id_ex->SignExt / 4);
				pcSrc2 = 1;
			}
		}
		break;
	case Jump:
//		ex_mem->PC = id_ex->j_address;
		jump = 1; ///////////////////////////////////////////////////////////////////////// 수정하기
		break;
	case Jump_And_Link:
//		ex_mem->PC = id_ex->j_address;
		id_ex->r31_data = ex_mem->PC + 2; 
		jump = 1;
		break;
	case Load_Halfword_Unsigned:
		ex_mem->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];    Memory 단계에서..
		break;
	case Load_Linked:
		ex_mem->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];    Memory 단계에서..
		break;
	case Load_Upper_Imm:
		ex_mem->ALU_result = id_ex->SignExt >> 16;
		break;
	case Load_Word:
		ex_mem->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];	Memory 단계에서..
		break;
	case Or_Immediate:
		ex_mem->ALU_result = id_ex->rs_data | id_ex->immediate;
		break;
	case Set_Less_Than_Imm:
		ex_mem->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
		break;
	case Set_Less_Than_Imm_Unsigned:
		ex_mem->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
		break;
	case Store_Conditional:
		//Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
		//reg[rt] = (atomic) ? 1 : 0;
		break;
	case Store_Halfword:
		//Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
		break;
	case Store_Word:
		ex_mem->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		Mem[(reg[rs] + SignExtImm(imm))/4] = reg[rt]; Memory 단계에서..
		break;
	}
}

void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb){
	
	mem_wb->address = ex_mem->address;

	if (memWrite == 1){
		Mem[ex_mem->ALU_result] = ex_mem->rt_data;
	}
	else if (memRead == 1){
		mem_wb->data = Mem[ex_mem->ALU_result];
	}
	else if (memtoReg == 0){
		mem_wb->data = ex_mem->ALU_result;
	}
	
}

void Write_Back(MEM_WB* mem_wb){
	reg[mem_wb->address] = mem_wb->data;
}







