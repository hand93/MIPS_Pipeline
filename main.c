#include <stdio.h>
#include "MIPS_instruction.h"

extern int Instruction_Memory[(0x100000 / 4)] = { 0 };
//extern int* PC = (&Instruction_Memory[0]);
int reg[32] = { 0 };
int Mem[1000000] = { 0 };
int Hi = 0;
int Lo = 0;
enum regNum { zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra };
BTB BTB_table[1024];
//s8 == fp

#define Add 0x0
#define Add_Immediate 0x8
#define Add_Imm_Unsigned 0x9
#define Add_Unsigned 0x0
#define And 0x0
#define And_Immediate 0xc
#define Branch_On_Equal 0x4
#define Branch_On_Not_Equal 0x5
#define Branch_On_Not_Equal_Zero 0x5
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

void swapbit(int* ptr); //윈도우에서 거꾸로 읽는 것을 원래대로 만들어줌.
void Memory_print();
int find_PC_in_BTB(int* PC);


void Instruction_Fetch(IF_ID* if_id, ID_EX* id_ex, EX_MEM* ex_mem);
void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex);
void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem_o, MEM_WB* mem_wb, EX_MEM* ex_mem_i);
void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb);
void Write_Back(MEM_WB* mem_wb);

int main(){
	FILE* spData;
	reg[sp] = 0x8000;
	reg[ra] = 0xFFFFFFFF;
	int i = 0;

	IF_ID* if_id[2];
	ID_EX* id_ex[2];
	EX_MEM* ex_mem[2];
	MEM_WB* mem_wb[2]; 

	if_id[0] = (int *)malloc(sizeof(IF_ID));
	if_id[1] = (int *)malloc(sizeof(IF_ID));
	id_ex[0] = (int *)malloc(sizeof(ID_EX));
	id_ex[1] = (int *)malloc(sizeof(ID_EX));
	ex_mem[0] = (int *)malloc(sizeof(EX_MEM));
	ex_mem[1] = (int *)malloc(sizeof(EX_MEM));
	mem_wb[0] = (int *)malloc(sizeof(MEM_WB));
	mem_wb[1] = (int *)malloc(sizeof(MEM_WB)); //구조체 메모리 할당

	memset(if_id[0], 0, sizeof(struct IF_ID));
	memset(if_id[1], 0, sizeof(struct IF_ID));
	memset(id_ex[0], 0, sizeof(struct ID_EX));
	memset(id_ex[1], 0, sizeof(struct ID_EX));
	memset(ex_mem[0], 0, sizeof(struct EX_MEM));
	memset(ex_mem[1], 0, sizeof(struct EX_MEM));
	memset(mem_wb[0], 0, sizeof(struct MEM_WB));
	memset(mem_wb[1], 0, sizeof(struct MEM_WB)); //구조체 초기화

	spData = fopen("temp.bin", "r");

	if (spData == NULL){
		printf("Could not open the file.\n");
	}

	while (!feof(spData)){
		fread(Instruction_Memory + i, 4, 1, spData);
		printf("Instruction[%02d] = %08x ", i, Instruction_Memory[i]);
		swapbit(Instruction_Memory + i);
		printf("=> %08x \n", Instruction_Memory[i]);
		i++;
	} //Instruction을 가져와서 Instruction Memory에 저장하기

	if_id[0]->PC = Instruction_Memory;

	while (if_id[0]->PC != -1){
		Write_Back(mem_wb[1]);

		Memory(ex_mem[1], mem_wb[0]);
		mem_wb[1] = mem_wb[0];

		Instruction_Execution(id_ex[1], ex_mem[0], mem_wb[1], ex_mem[1]);
		ex_mem[1] = ex_mem[0];

		Instruction_Decode(if_id[1], id_ex[0]);
		id_ex[1] = id_ex[0];

		Instruction_Fetch(if_id[0], id_ex[1], ex_mem[0], if_id[1]);
		if_id[1] = if_id[0];		
	}
	Write_Back(mem_wb[1]);
	printf("The end\n");

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
			printf("Mem[%d] : %d \n", i, Mem[i]);
		}
	}
}

int find_PC_in_BTB(int PC){
	int i=0;

	for (i = 0; i < 1024; i++){
		if (BTB_table[i].PC == PC){
			if (BTB_table[i].twobit_counter == (3 | 2)){
				return BTB_table[i].Branch_target_address;
			}//BTB에 일치하는 PC값이 있고, counter가 strongly taken이거나 weakly taken이면 branch.
			else return NULL; //BTB에 일치하는 PC값이 있지만 strongly !taken이거나 weakly !taken이면 branch 안함
		}
	}
	return NULL; //BTB에 일치하는 PC값이 없음.
}



void Instruction_Fetch(IF_ID* if_id_i, ID_EX* id_ex, EX_MEM* ex_mem, IF_ID* if_id_o){
	int branch_target_address = find_PC_in_BTB(if_id_i->PC);
	if_id_i->instruction = *(if_id_i->PC);
	id_ex->PC = if_id_i->PC;

	if (if_id_i->PC == -1){
		printf("The end");
	}
	else if(id_ex->jump == 1){
		if_id_i->PC = Instruction_Memory + id_ex->j_address;
	}
	else if (id_ex->jal == 1){
		if_id_i->PC += 2;
	}
	else if (id_ex->jr == 1){
		if_id_i->PC = id_ex->rs_data;
	}
	else if (id_ex->branch == 1){
		int target_address = find_PC_in_BTB(if_id_i->PC);

		if (target_address == NULL){
			
			(if_id_i->PC)++;
		}
		else if_id_i->PC = Instruction_Memory + 1 + target_address;
	}
	else if (ex_mem->branch == 1){ 
		int i;

		if (ex_mem->bcond == 0){
			if_id_i->PC = (ex_mem->PC)++;
			memset(if_id_o, 0, sizeof(struct IF_ID)); //flush

			for (i = 0; i < 1024; i++){
				if (BTB_table[i].PC == id_ex->PC){
					if (BTB_table[i].twobit_counter != 0){
						BTB_table[i].twobit_counter--;
					}
				}
			}
		}//branch인데 condition을 만족하지 않을 때(!taken)
		else {
			for (i = 0; i < 1024; i++){
				if (BTB_table[i].PC == id_ex->PC){
					if (BTB_table[i].twobit_counter != 3){
						BTB_table[i].twobit_counter++;
					}
				}
			}
			(if_id_i->PC)++;
		}//branch인데 condition을 만족할 때(taken)		//prediction을 bracnh 해야하는데 '안함'으로 predict 했을때..... 수정하기
	}
	else (if_id_i->PC)++;

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


	/////////////Control
	if(opcode == 0){
		id_ex->RegWrite = 1;
		id_ex->branch = 0;
		id_ex->RegWrite = 0;
		id_ex->MemRead = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;

		if ((funct == 0x20) || (funct == 0x21) || (funct == 0x24) || (funct == 0x27) || (funct == 0x25) || (funct == 0x00)
			|| (funct == 0x02) || (funct == 0x22) || (funct == 0x23) || (funct == 0x10) || (funct == 0x12)){
			id_ex->RegDst = 1; //rd
		}
		else if (funct == 0x08){ //Jump_Register
			id_ex->jr = 1;
			id_ex->jump = 0;
			id_ex->RegWrite = 0;
			id_ex->branch = 0;
			id_ex->MemWrite = 0;
			id_ex->MemRead = 0;
			id_ex->RegDst = 0;
			id_ex->MemtoReg = 0;
			id_ex->jal = 0;
			printf("jr\n");
		}
	}
	else if ((opcode == Load_Word) || (opcode == Load_Byte_Unsigned) || (opcode == Load_FP) || (opcode == Load_FP_Single) || (opcode == Load_Linked)){
//		id_ex->ALUSrc = 1;
		id_ex->MemRead = 1;
		id_ex->RegWrite = 1; 
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite= 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if (opcode == Load_Upper_Imm){
//		id_ex->ALUSrc = 1;
		id_ex->RegWrite = 1;
		id_ex->MemtoReg = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Store_Word) || (opcode == Store_Byte) || (opcode == Store_Conditional) || (opcode == Store_FP) || (opcode == Store_FP_Single) || (opcode == Store_Halfword)){
//		id_ex->ALUSrc = 1;
		id_ex->MemWrite = 1;
		id_ex->MemRead = 0;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Branch_On_Equal) || (opcode == Branch_On_Not_Equal) || (opcode == Branch_On_Not_Equal_Zero)){
		id_ex->branch = 1;
		id_ex->MemWrite = 0;
		id_ex->MemRead = 0;
		id_ex->RegDst = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->RegWrite = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Jump)){
		id_ex->jump = 1;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemRead = 0;
		id_ex->RegDst = 0;
		id_ex->MemtoReg = 0;
		id_ex->jr = 0;
		id_ex->RegWrite = 0;
		id_ex->jal = 0;
	}
	else if (opcode == Jump_And_Link){
		id_ex->RegWrite = 1;
		id_ex->jal = 1;
		id_ex->jump = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemRead = 0;
		id_ex->RegDst = 0;
		id_ex->MemtoReg = 0;
		id_ex->jr = 0;
	}

	//id_ex->opcode = opcode;
	id_ex->ALUop = opcode;
	id_ex->rs_data = reg[rs];
	id_ex->rt_data = reg[rt];
	id_ex->r31_data = reg[31];
	id_ex->immediate = immediate;
	id_ex->SignExt = SignExtImm(immediate);
	id_ex->rt_address = rt;
	id_ex->rd_address = rd;
	id_ex->rs_address = rs;
	id_ex->j_address = JumpAddr(address, if_id->PC);
	id_ex->sh = shamt;
	id_ex->func = funct;
}





void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem_i, MEM_WB* mem_wb, EX_MEM* ex_mem_o){

	/////////////데이터 전달하는 부분
	ex_mem_i->PC = id_ex->PC;
	ex_mem_i->rt_data = id_ex->rt_data;
	ex_mem_i->rs_data = id_ex->rs_data;
	ex_mem_i->b_address = BranchAddr(id_ex->immediate);

	if (id_ex->RegDst == 1){
		ex_mem_i->address = id_ex->rd_address;
	}
	else ex_mem_i->address = id_ex->rt_address;

	////////////////Data Dependency _forwarding
	if (id_ex->rs_address == mem_wb->address){  //rs
		if (mem_wb->MemtoReg == 1){
			id_ex->rs_data = mem_wb->Mem_data;
		}
		else{
			id_ex->rs_data = mem_wb->ALU_result;
		}
	}
	else if (id_ex->rs_address == ex_mem_o->address){
		id_ex->rs_data = ex_mem_o->ALU_result;
	}

	if (id_ex->rt_address == mem_wb->address){  //rt
		if (mem_wb->MemtoReg == 1){
			id_ex->rt_data = mem_wb->Mem_data;
		}
		else{
			id_ex->rt_data = mem_wb->ALU_result;
		}
	}
	else if (id_ex->rt_address == ex_mem_o){
		id_ex->rt_data = ex_mem_o->ALU_result;
	}

	switch (id_ex->ALUop){
	case 0x00:
		switch (id_ex->func){
		case 0x20:
			ex_mem_i->ALU_result = id_ex->rs_data + id_ex->rt_data;  //Add
			break;
		case 0x21: 
			if (id_ex->rt_address == 0){
				ex_mem_i->ALU_result = id_ex->rs_data; //Move
				printf("move\n");
			}
			else {
				ex_mem_i->ALU_result = id_ex->rs_data + id_ex->rt_data; //Add Unsigned
				printf("addu\n");
			}
			break;
		case 0x24:
			ex_mem_i->ALU_result = id_ex->rs_data & id_ex->rt_data;  //And
			break;
		case 0x08:
//			PC = reg[rs];	Fetch 단계에서..  //Jump_Regster
			printf("jr\n");
			break;
		case 0x27:
			ex_mem_i->ALU_result = ~(id_ex->rs_data | id_ex->rt_data); //Nor
			break;
		case 0x25:
			ex_mem_i->ALU_result = id_ex->rs_data | id_ex->rt_data; //Or
			break;
		case 0x2a:
			ex_mem_i->rt_data = (id_ex->rs_data < id_ex->rt_data) ? 1 : 0; // Set_Less_Than
			break;
		case 0x2b:
			ex_mem_i->rt_data = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0; // Set_Less_Than_Imm_Unsigned
//			ALUSrc = 1;
			break;
		case 0x00:
			ex_mem_i->ALU_result = id_ex->rt_data << id_ex->sh; //Shift_Left_Logical
			break;
		case 0x02:
			ex_mem_i->ALU_result = id_ex->rt_data >> id_ex->sh; //Shift_Right_Logical
			break;
		case 0x22:
			ex_mem_i->ALU_result = id_ex->rs_data - id_ex->rt_data; //Subtract
			break;
		case 0x23:
			ex_mem_i->ALU_result = id_ex->rs_data - id_ex->rt_data; //Subtract_Unsigned
			break;
		case 0x10:
			ex_mem_i->ALU_result = Hi; //Move_From_Hi
			break;
		case 0x12:
			ex_mem_i->ALU_result = Lo; //Move_From_Low
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
//			ex_mem_i->ALU_result = id_ex->rt_data >>> id_ex->sh; //Shift_Right_Arith
//			ex_mem_i->address = id_ex->rd_address;
		}
		break;
	case Add_Immediate:
		ex_mem_i->rt_data = id_ex->rs_data & id_ex->SignExt;
		break;
	case 0x09:
		if (id_ex->rs_address != 0){
			ex_mem_i->rt_data = id_ex->rs_data & id_ex->SignExt; //Add_imm_Unsigned
//			ALUSrc = 1;
			printf("addiu\n");

		}
		else{
			ex_mem_i->rt_data = id_ex->immediate; //Load_Immediate
//			ALUSrc = 1;
		}
		break;
	case And_Immediate:
		ex_mem_i->rt_data = id_ex->rs_data & id_ex->immediate;
//		ALUSrc = 1;

		break;
	case Branch_On_Equal:
		if (id_ex->rs_data == id_ex->rt_data){
//			ex_mem_i->PC = ex_mem_i->PC + 1 + (id_ex->SignExt / 4);		Fetch 단계에서..
			ex_mem_i->bcond = 1;
		}
		break;
	case 0x5:
		if (id_ex->rt_address != 0){
			if (id_ex->rs_data != id_ex->rt_data){ //Branch_On_Not_Equal
				if (id_ex->rs_data != id_ex->rt_data){
					ex_mem_i->bcond = 1;
				}
//				ex_mem_i->PC = ex_mem_i->PC + 1 + (id_ex->SignExt / 4);		Fetch 단계에서..
			}
		}
		else if (id_ex->rs_address != 0){ //Branch_On_Not_Equal_Zero
//			ex_mem_i->PC = ex_mem_i->PC + 1 + (id_ex->SignExt / 4);		Fetch 단계에서..
			if (id_ex->rs_data != 0){
				ex_mem_i->bcond = 1;
			}
			printf("bnez\n");
		}
		break;
	case Jump:
		printf("j\n");
		break;
	case Jump_And_Link:
		//ex_mem_i->PC + 2 =id_ex->r31_data; Fetch 단계에서..
		break;
	case Load_Halfword_Unsigned:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		ALUSrc = 1;
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];    Memory 단계에서..
		break;
	case Load_Linked:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		ALUSrc = 1;
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];    Memory 단계에서..
		break;
	case Load_Upper_Imm:
		ex_mem_i->ALU_result = id_ex->SignExt >> 16;
//		ALUSrc = 1;
		break;
	case Load_Word:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		ALUSrc = 1;
		printf("lw\n");
//		reg[rt] = Mem[(reg[rs] + SignExtImm(imm))];	Memory 단계에서..
		break;
	case Or_Immediate:
		ex_mem_i->ALU_result = id_ex->rs_data | id_ex->immediate;
//		ALUSrc = 1;
		break;
	case Set_Less_Than_Imm:
		ex_mem_i->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
		printf("slti\n");
//		ALUSrc = 1;
		break;
	case Set_Less_Than_Imm_Unsigned:
		ex_mem_i->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
//		ALUSrc = 1;
		break;
	case Store_Conditional:
		//Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
		//reg[rt] = (atomic) ? 1 : 0;
		//ALUSrc = 1;
		break;
	case Store_Halfword:
		//Mem[(reg[rs] + SignExtImm(imm)) / 4] = reg[rt];
		//ALUSrc = 1;
		break;
	case Store_Word:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
//		ALUSrc = 1;
		printf("sw\n");
//		Mem[(reg[rs] + SignExtImm(imm))/4] = reg[rt]; Memory 단계에서..
		break;
	}

	////////////////control
	ex_mem_i->PCSrc = (id_ex->branch) & (ex_mem_i->bcond);
	ex_mem_i->branch = id_ex->branch;
	ex_mem_i->MemWrite = id_ex->MemWrite;
	ex_mem_i->MemRead = id_ex->MemRead;
	ex_mem_i->RegWrite = id_ex->RegWrite;
	ex_mem_i->MemtoReg = id_ex->MemtoReg;
}

void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb){
	
	mem_wb->PC = ex_mem->PC;
	mem_wb->address = ex_mem->address;

	//control
	mem_wb->MemtoReg = ex_mem->MemtoReg;
	mem_wb->RegWrite = ex_mem->RegWrite;

	if (ex_mem->MemWrite == 1){
		Mem[ex_mem->ALU_result] = ex_mem->rt_data;
	}
	
	if (ex_mem->MemRead == 1){
		mem_wb->Mem_data = Mem[ex_mem->ALU_result];
	}
}

void Write_Back(MEM_WB* mem_wb){	
	if (mem_wb->MemtoReg == 1){
		reg[mem_wb->address] = mem_wb->Mem_data;
	}
	else{
		if (mem_wb->RegWrite == 1){
			reg[mem_wb->address] = mem_wb->ALU_result;
		}
	}

}
