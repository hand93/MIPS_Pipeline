#include <stdio.h>
#include "MIPS_instruction.h"
//#include "input_data.h"

int reg[32] = { 0 };
int Mem[0x1000000] = { 0 };
int Hi = 0;
int Lo = 0;
enum regNum { zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra };
BTB BTB_table[0x10000];
CASHE cashe[4][4096];
//int flush[2] = { 0 };

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

void swapbit(int* ptr);
void Register_print();
int find_PC_in_BTB(int* PC);
int Memory_Read(int PC);
void Memory_Write(int PC, int data);

void Instruction_Fetch(IF_ID* if_id_i, ID_EX* id_ex, EX_MEM* ex_mem, IF_ID* if_id_o);
void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex);
void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem_i, MEM_WB* mem_wb_o, MEM_WB* mem_wb_i, EX_MEM* ex_mem_o);
void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb_i, MEM_WB* mem_wb_o);
void Write_Back(MEM_WB* mem_wb);

int main(){
	FILE* spData;
	reg[sp] = 0x100000;
	reg[ra] = 0xFFFFFFFF;
	int i = 0;
	int count = 0;

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

	spData = fopen("input4.bin", "rb");

	if (spData == NULL){
		//printf("Could not open the file.\n");
	}

	while (!feof(spData)){
		fread(Mem + i, 4, 1, spData);
		//printf("Mem[0x%02x] = 0x%08x ", i * 4, Mem[i]);
		swapbit(Mem + i);
		//printf("=> 0x%08x \n", Mem[i]);
		i++;
	} //Instruction을 가져와서 Memory에 저장하기
	//printf("i : %d\n", i);

	if_id[0]->PC = 0x0;

	while (if_id[0]->PC != -1){
		Write_Back(mem_wb[1]);
		Memory(ex_mem[1], mem_wb[0], mem_wb[1]);
		Instruction_Execution(id_ex[1], ex_mem[0], mem_wb[1], mem_wb[0], ex_mem[1]);
		Instruction_Decode(if_id[1], id_ex[0]);
		Instruction_Fetch(if_id[0], id_ex[0], ex_mem[0], if_id[1]);

		mem_wb[1]->PC = mem_wb[0]->PC;
		mem_wb[1]->Mem_data = mem_wb[0]->Mem_data;
		mem_wb[1]->ALU_result = mem_wb[0]->ALU_result;
		mem_wb[1]->address = mem_wb[0]->address;
		mem_wb[1]->MemtoReg = mem_wb[0]->MemtoReg;
		mem_wb[1]->RegWrite = mem_wb[0]->RegWrite;
		mem_wb[1]->jal = mem_wb[0]->jal;

		ex_mem[1]->PC = ex_mem[0]->PC;
		ex_mem[1]->b_address = ex_mem[0]->b_address;
		ex_mem[1]->j_address = ex_mem[0]->j_address;
		ex_mem[1]->ALU_result = ex_mem[0]->ALU_result;
		ex_mem[1]->rs_data = ex_mem[0]->rs_data;
		ex_mem[1]->rt_data = ex_mem[0]->rt_data;
		ex_mem[1]->address = ex_mem[0]->address;
		ex_mem[1]->branch = ex_mem[0]->branch;
		ex_mem[1]->bcond = ex_mem[0]->bcond;
		ex_mem[1]->MemWrite = ex_mem[0]->MemWrite;
		ex_mem[1]->MemRead = ex_mem[0]->MemRead;
		ex_mem[1]->MemtoReg = ex_mem[0]->MemtoReg;
		ex_mem[1]->RegWrite = ex_mem[0]->RegWrite;
		ex_mem[1]->jal = ex_mem[0]->jal;
		ex_mem[1]->rt_address = ex_mem[0]->rt_address;

		id_ex[1]->PC = id_ex[0]->PC;
		id_ex[1]->rs_data = id_ex[0]->rs_data;
		id_ex[1]->rt_data = id_ex[0]->rt_data;
		id_ex[1]->immediate = id_ex[0]->immediate;
		id_ex[1]->SignExt = id_ex[0]->SignExt;
		id_ex[1]->rt_address = id_ex[0]->rt_address;
		id_ex[1]->rd_address = id_ex[0]->rd_address;
		id_ex[1]->rs_address = id_ex[0]->rs_address;
		id_ex[1]->j_address = id_ex[0]->j_address;
		id_ex[1]->sh = id_ex[0]->sh;
		id_ex[1]->func = id_ex[0]->func;
		id_ex[1]->ALUop = id_ex[0]->ALUop;
		id_ex[1]->RegDst = id_ex[0]->RegDst;
		id_ex[1]->branch = id_ex[0]->branch;
		id_ex[1]->RegWrite = id_ex[0]->RegWrite;
		id_ex[1]->MemWrite = id_ex[0]->MemWrite;
		id_ex[1]->MemRead = id_ex[0]->MemRead;
		id_ex[1]->MemtoReg = id_ex[0]->MemtoReg;
		id_ex[1]->jump = id_ex[0]->jump;
		id_ex[1]->jr = id_ex[0]->jr;
		id_ex[1]->jal = id_ex[0]->jal;

		if_id[1]->PC = if_id[0]->PC;
		if_id[1]->instruction = if_id[0]->instruction;
		//		if (count % 1000000 == 0) 
		//printf("         r[v0]=%d, cycle : %d\n", reg[2], count);
		count++;
	}
	Write_Back(mem_wb[1]);
	//printf("         r[v0]=%d, cycle : %d\n", reg[2], ++count);
	Memory(ex_mem[1], mem_wb[0], mem_wb[1]);
	mem_wb[1] = mem_wb[0];
	Write_Back(mem_wb[1]);
	printf("         r[v0]=%d, cycle : %d\n", reg[2], ++count);

	//printf("The end\n");

	Register_print();

	fclose(spData);

	return 0;
}

void swapbit(int* ptr) {
	unsigned int tmp1, tmp2;

	tmp1 = ((*ptr) & 0xff000000) >> 24;
	tmp2 = ((*ptr) & 0x000000ff) << 24;
	*ptr &= 0x00ffff00;
	*ptr |= tmp2;
	*ptr |= tmp1;

	tmp1 = ((*ptr) & 0x00ff0000) >> 8;
	tmp2 = ((*ptr) & 0x0000ff00) << 8;
	*ptr &= 0xff0000ff;
	*ptr |= tmp1;
	*ptr |= tmp2;
}

void Register_print(){
	int i;

	//printf("\n");

	for (i = 0; i < 32; i++){
		if (reg[i] != 0){
			//printf("reg[%d] : %d \n", i, reg[i]);
		}
	}

	for (i = 0; i < 10000; i++){
		if (BTB_table[i].PC != 0){
			//printf("BTB_table[%d].PC = %x\n", i, BTB_table[i].PC);
		}
	}
}

int find_PC_in_BTB(int PC){
	int i = 0;

	for (i = 0; i < 1024; i++){
		if (BTB_table[i].PC == PC){
			if ((BTB_table[i].twobit_counter == 3) || (BTB_table[i].twobit_counter == 2)){
				return BTB_table[i].Branch_target_address;
			}//BTB에 일치하는 PC값이 있고, counter가 strongly taken이거나 weakly taken이면 branch.
			else return NULL; //BTB에 일치하는 PC값이 있지만 strongly !taken이거나 weakly !taken이면 branch 안함
		}
	}
	return NULL; //BTB에 일치하는 PC값이 없음.
}

int Memory_Read(int PC){
	int tag = (PC & 0xFFFC0000) >> 18;
	int index = (PC & 0x0003FFC0) >> 6;
	int offset = (PC & 0x0000003F);
	int tmp = (PC & 0xFFFFFFC0);
	int i,j = 0;
	int k = 0;
	int hit = 0;
	int victim = 0;

	for (i = 0; i < 4; i++){
		if (cashe[i][index].valid == 0){
			for (k = 0; k < 16; k++){
				cashe[i][index].data[k] = Mem[(tmp / 4) + k];
			}//cashe에 채우는 과정
			cashe[i][index].tag = tag;
			cashe[i][index].valid = 1;
			cashe[i][index].sca = 1;
			hit = 1;
			break;
		}//cold miss
		else{
			if (cashe[i][index].tag == tag){
				hit = 1;
				cashe[i][index].sca = 1;
				break;
			}//hit
			else{
				cashe[i][index].sca = 0;
			}//miss
		}
	}

	//replacement
	if (!hit){
		for (i = 0; i < 4; i++){
			victim = cashe[i][index].sca;
			if (victim>cashe[i][index].sca){
				victim = cashe[i][index].sca;
				j = i;
			}
		}

		if (cashe[j][index].dirty == 1){
			for (k = 0; k < 16; k++){
				Mem[(tmp / 4) + k] = cashe[j][index].data[k];
			}
			cashe[j][index].dirty = 0;
		}//메모리에 업로드
		
		for (k = 0; k < 16; k++){
			cashe[j][index].data[k]= Mem[(tmp / 4) + k];
		}//cashe에 채우는 과정
		cashe[j][index].tag = tag;
		cashe[j][index].valid = 1;
		cashe[j][index].sca = 1;
		return cashe[j][index].data[(offset / 4)];
	}

	return cashe[i][index].data[(offset / 4)];
}

void Memory_Write(int PC, int data){
	int tag = (PC & 0xFFFC0000) >> 18;
	int index = (PC & 0x0003FFC0) >> 6;
	int offset = (PC & 0x0000003F);
	int tmp = (PC & 0xFFFFFFC0);
	int i,j,k = 0;
	int hit, victim=0;

	for (i = 0; i < 4; i++){
		if (cashe[i][index].valid == 1){
			if (cashe[i][index].tag == tag){
				cashe[i][index].data[offset / 4] = data;
				cashe[i][index].dirty = 1;
				cashe[i][index].sca=1;
				hit = 1;
				break;
			}
			cashe[i][index].sca=0;
		}
		else{
			cashe[i][index].tag = tag;
			for (k = 0; k < 16; k++){
				cashe[i][index].data[k] = Mem[(tmp / 4) + k];
			}//cashe에 채우는 과정
			cashe[i][index].valid = 1;
			cashe[i][index].sca = 1;
			cashe[i][index].dirty = 1;
			cashe[i][index].data[offset / 4] = data;
			Mem[PC / 4] = data;
			hit = 1;
			break;
		}
	}

	////////////replacement
	if (!hit){
		for (i = 0; i < 4; i++){
			victim = cashe[i][index].sca;
			if (victim>cashe[i][index].sca){
				victim = cashe[i][index].sca;
				j = i;
			}
		}

		if (cashe[j][index].dirty == 1){
			for (k = 0; k < 16; k++){
				Mem[(tmp / 4) + k] = cashe[j][index].data[k];
			}//메모리에 업로드
			cashe[j][index].dirty = 0;
		}

		for (k = 0; k < 16; k++){
			cashe[j][index].data[k] = Mem[(tmp / 4) + k];
		}//cashe에 채우는 과정

		cashe[j][index].data[offset / 4] = data;
		cashe[j][index].valid = 1;
		cashe[j][index].sca = 1;
		cashe[j][offset].dirty = 1;
	}
}



void Instruction_Fetch(IF_ID* if_id_i, ID_EX* id_ex, EX_MEM* ex_mem, IF_ID* if_id_o){

	if_id_i->instruction = Memory_Read(if_id_i->PC);

	if (if_id_i->PC == -1){
		//printf("The end");
	}
	else if (id_ex->jump == 1){
		if_id_i->PC = id_ex->j_address;
	}
	else if (id_ex->jal == 1){
		if_id_i->PC = id_ex->j_address;
	}
	else if (id_ex->jr == 1){
		if_id_i->PC = id_ex->rs_data;
	}
	else if (id_ex->branch == 1){
		int target_address = find_PC_in_BTB((id_ex->PC) - 4);
		int i = 0;

		while ((BTB_table[i].PC != 0)){
			if (BTB_table[i].PC == ((id_ex->PC) - 4)){
				break;
			}
			i++;
		}
		BTB_table[i].PC = (id_ex->PC) - 4;

		if (target_address == NULL){
			if_id_i->PC += 4;
		}//PC가 BTB_table에 저장이 안되어있거나, 저장이 되어있지만 weakliy !taken, strongly !taken 일때
		else{
			if_id_i->PC = if_id_i->PC + 4 + target_address;
			if_id_i->instruction = Memory_Read(((if_id_i->PC) - 4));
		}
	}
	else if (ex_mem->branch == 1){
		int i;

		if (ex_mem->bcond == 0){
			int i = 0;

			while (((ex_mem->PC) - 4) != BTB_table[i].PC){
				i++;
			}
			BTB_table[i].Branch_target_address = ex_mem->b_address;

			if ((ex_mem->PC + 4 + BTB_table[i].Branch_target_address) == if_id_i->PC){
				if_id_i->PC = (ex_mem->PC) + 4;
				if_id_i->instruction = Memory_Read(((if_id_i->PC) - 4));
				memset(if_id_o, 0, sizeof(struct IF_ID)); //flush
				memset(id_ex, 0, sizeof(struct ID_EX));
				//printf("                       flush\n");
			} //branch 안해야하는데 했을 때
			else{
				if_id_i += 4;
			} //branch 안해야하는데 안했을 때

			for (i = 0; i < 1024; i++){
				if (BTB_table[i].PC == ((ex_mem->PC) - 4)){
					if (BTB_table[i].twobit_counter != 0){
						BTB_table[i].twobit_counter--;
					}
				}
			}
		}//branch인데 condition을 만족하지 않을 때(!taken)
		else {
			int i = 0;

			while (((ex_mem->PC) - 4) != BTB_table[i].PC){
				i++;
			}
			BTB_table[i].Branch_target_address = ex_mem->b_address;

			if ((ex_mem->PC + 4 + BTB_table[i].Branch_target_address) != id_ex->PC){
				if_id_i->PC = ex_mem->PC + 4 + BTB_table[i].Branch_target_address;
				if_id_i->instruction = Memory_Read(((if_id_i->PC) - 4));

				memset(if_id_o, 0, sizeof(struct IF_ID)); //flush
				memset(id_ex, 0, sizeof(struct ID_EX));
				//printf("                       flush\n");
			}//prediction을 branch 해야하는데 '안함'으로 predict 했을때 PC를 다시 target address로
			else {
				if_id_i->PC += 4; //prediction을 branch '함'으로 predict 했을때 정상적으로 PC+4
			}

			if (BTB_table[i].twobit_counter != 3){
				BTB_table[i].twobit_counter++;
			}
		}//branch인데 condition을 만족할 때(taken)
	}
	else if_id_i->PC += 4;
}

void Instruction_Decode(IF_ID* if_id, ID_EX* id_ex){
	int inst = if_id->instruction;
	int opcode, rs, rt, rd, shamt, funct = 0;
	int immediate = 0;
	int address = 0;

	id_ex->PC = if_id->PC;

	opcode = ((inst)& 0xFC000000) >> 26;
	rs = ((inst)& 0x03E00000) >> 21;
	rt = ((inst)& 0x001F0000) >> 16;
	rd = ((inst)& 0x0000F800) >> 11;
	shamt = ((inst)& 0x000007C0) >> 6;
	funct = ((inst)& 0x0000003F);
	immediate = ((inst)& 0x0000FFFF);
	address = ((inst)& 0x03FFFFFF);


	/////////////Control
	if (opcode == 0){
		id_ex->ALUSrc = 0;
		id_ex->RegWrite = 1;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemRead = 0;
		id_ex->MemtoReg = 0;
		id_ex->RegDst = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;

		if ((funct == 0x20) || (funct == 0x24) || (funct == 0x27) || (funct == 0x25) || (funct == 0x00)
			|| (funct == 0x02) || (funct == 0x22) || (funct == 0x23) || (funct == 0x10) || (funct == 0x12) || (opcode == 0x2b)){
			if (inst != 0){
				id_ex->RegDst = 1; //rd
			}
			else{ //nop
				id_ex->ALUSrc = 0;
				id_ex->jr = 0;
				id_ex->jump = 0;
				id_ex->RegWrite = 0;
				id_ex->branch = 0;
				id_ex->MemWrite = 0;
				id_ex->MemRead = 0;
				id_ex->RegDst = 0;
				id_ex->MemtoReg = 0;
				id_ex->jal = 0;
			}
		}
		else if(opcode == 0x2a){ //slt
			id_ex->ALUSrc = 0;
			id_ex->jr = 0;
			id_ex->jump = 0;
			id_ex->RegWrite = 1;
			id_ex->branch = 0;
			id_ex->MemWrite = 0;
			id_ex->MemRead = 0;
			id_ex->RegDst = 1;
			id_ex->MemtoReg = 0;
			id_ex->jal = 0;
		}
		else if (funct == 0x21){
			if (rt== 0){ //Move
				id_ex->ALUSrc = 0;
				id_ex->jr = 0;
				id_ex->jump = 0;
				id_ex->RegWrite = 1;
				id_ex->branch = 0;
				id_ex->MemWrite = 0;
				id_ex->MemRead = 0;
				id_ex->RegDst = 1;
				id_ex->MemtoReg = 0;
				id_ex->jal = 0;
			}
			else { //Add Unsigned
				id_ex->ALUSrc = 0;
				id_ex->jr = 0;
				id_ex->jump = 0;
				id_ex->RegWrite = 1;
				id_ex->branch = 0;
				id_ex->MemWrite = 0;
				id_ex->MemRead = 0;
				id_ex->RegDst = 1;
				id_ex->MemtoReg = 0;
				id_ex->jal = 0;
			}
		}
		else if (funct == 0x08){ //Jump_Register
			id_ex->ALUSrc = 0;
			id_ex->jr = 1;
			id_ex->jump = 0;
			id_ex->RegWrite = 0;
			id_ex->branch = 0;
			id_ex->MemWrite = 0;
			id_ex->MemRead = 0;
			id_ex->RegDst = 0;
			id_ex->MemtoReg = 0;
			id_ex->jal = 0;
		}
		else if ((rs == 0)&(rt == 0)&(rd == 0)&(shamt == 0)&(funct == 0)){ //nop
			id_ex->ALUSrc = 0;
			id_ex->RegWrite = 0;
			id_ex->branch = 0;
			id_ex->RegWrite = 0;
			id_ex->RegDst = 0;
			id_ex->MemRead = 0;
			id_ex->MemtoReg = 0;
			id_ex->jump = 0;
			id_ex->jr = 0;
			id_ex->jal = 0;
			//printf("nop\n");
		}
	}
	else if ((opcode == Add_Immediate) || ((opcode == Add_Imm_Unsigned)&&(rs != 0))){
		id_ex->ALUSrc = 1;
		id_ex->MemRead = 0;
		id_ex->RegWrite = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == And_Immediate)){
		id_ex->ALUSrc = 1;
		id_ex->MemRead = 0;
		id_ex->RegWrite = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if (opcode == Or_Immediate){
		id_ex->ALUSrc = 1;
		id_ex->MemRead = 0;
		id_ex->RegWrite = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Set_Less_Than_Imm) || (opcode == Set_Less_Than_Imm_Unsigned)){
		id_ex->ALUSrc = 1;
		id_ex->MemRead = 0;
		id_ex->RegWrite = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Load_Word) || (opcode == Load_Byte_Unsigned) || (opcode == Load_FP) || (opcode == Load_FP_Single) || (opcode == Load_Linked)){
		id_ex->ALUSrc = 1;
		id_ex->MemRead = 1;
		id_ex->RegWrite = 1;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemtoReg = 1;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if (opcode == Load_Upper_Imm){
		id_ex->ALUSrc = 1;
		id_ex->RegWrite = 1;
		id_ex->MemtoReg = 0;
		id_ex->MemRead = 0;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Add_Imm_Unsigned) && (rs == 0)){ //Load_immediate
		id_ex->ALUSrc = 1;
		id_ex->RegWrite = 1;
		id_ex->MemtoReg = 0;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemWrite = 0;
		id_ex->MemRead = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if ((opcode == Store_Word) || (opcode == Store_Byte) || (opcode == Store_Conditional) || (opcode == Store_FP) || (opcode == Store_FP_Single) || (opcode == Store_Halfword)){
		id_ex->ALUSrc = 1;
		id_ex->MemWrite = 1;
		id_ex->MemRead = 0;
		id_ex->RegWrite = 0;
		id_ex->RegDst = 0;
		id_ex->branch = 0;
		id_ex->MemtoReg = 0;
		id_ex->jump = 0;
		id_ex->jr = 0;
		id_ex->jal = 0;
	}
	else if (((opcode == Branch_On_Equal)) || (opcode == Branch_On_Not_Equal) || (opcode == Branch_On_Not_Equal_Zero)){
		id_ex->ALUSrc = 1;
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
		id_ex->ALUSrc = 1;
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
		id_ex->ALUSrc = 1;
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
	id_ex->ALUop = opcode;
	id_ex->rs_data = reg[rs];
	id_ex->rt_data = reg[rt];
	id_ex->immediate = immediate;
	id_ex->SignExt = SignExtImm(immediate);
	id_ex->rt_address = rt;
	id_ex->rd_address = rd;
	id_ex->rs_address = rs;
	id_ex->j_address = JumpAddr(address, if_id->PC);
	id_ex->sh = shamt;
	id_ex->func = funct;
}


void Instruction_Execution(ID_EX* id_ex, EX_MEM* ex_mem_i, MEM_WB* mem_wb_o, MEM_WB* mem_wb_i, EX_MEM* ex_mem_o){

	////////////////Data Dependency _forwarding
	if (id_ex->rs_address == mem_wb_o->address){  //rs
		if (mem_wb_o->RegWrite == 1){
			if (mem_wb_o->MemtoReg == 1){
				id_ex->rs_data = mem_wb_o->Mem_data;
			}
			else if(mem_wb_o->jal==0){
				id_ex->rs_data = mem_wb_o->ALU_result;
			}
			else if (mem_wb_o->jal == 1){
				id_ex->rs_data = (mem_wb_o->PC) + 4;
			}
		}
	}

	if (id_ex->rs_address == ex_mem_o->address){
		if (ex_mem_o->RegWrite == 1){
			if (ex_mem_o->MemtoReg == 1){
				id_ex->rs_data = mem_wb_i->Mem_data;
			}
			else if (ex_mem_o->jal == 0){
				id_ex->rs_data = ex_mem_i->ALU_result;
			}
			else if (ex_mem_o->jal == 1){
				id_ex->rs_data = (ex_mem_i->PC) + 4;
			}
		}
	}

	if (mem_wb_o->ALUSrc == 0){//rt
		if (id_ex->rt_address == mem_wb_o->address){
			if (mem_wb_o->RegWrite == 1){
				if (mem_wb_o->MemtoReg == 1){
					id_ex->rt_data = mem_wb_o->Mem_data;
				}
				else if (mem_wb_o->jal == 0){
					id_ex->rt_data = mem_wb_o->ALU_result;
				}
				else if (mem_wb_o->jal == 1) {
					id_ex->rt_data = (mem_wb_o->PC) + 4;
				}
			}
		}
	}

	if (ex_mem_o->ALUSrc == 0){
		if (id_ex->rt_address == ex_mem_o->address){
			if (ex_mem_o->RegWrite == 1){
				if (ex_mem_o->MemtoReg == 1){
					id_ex->rt_data = mem_wb_i->Mem_data;
				}
				else if (ex_mem_o->jal == 0){
					id_ex->rt_data = mem_wb_i->ALU_result;
				}
				else if (ex_mem_o->jal == 1){
					id_ex->rt_data = (mem_wb_i->PC) + 4;
				}
			}
		}
	}

	/////////////데이터 전달하는 부분
	ex_mem_i->PC = id_ex->PC;
	ex_mem_i->rt_data = id_ex->rt_data;
	ex_mem_i->rs_data = id_ex->rs_data;
	ex_mem_i->rt_address = id_ex->rt_address;
	ex_mem_i->b_address = BranchAddr(id_ex->immediate);

	if (id_ex->RegDst == 1){
		ex_mem_i->address = id_ex->rd_address;
	}
	else ex_mem_i->address = id_ex->rt_address;

	switch (id_ex->ALUop){
	case 0x00:
		switch (id_ex->func){
		case 0x20:
			ex_mem_i->ALU_result = id_ex->rs_data + id_ex->rt_data;  //Add
			break;
		case 0x21:
			if (id_ex->rt_address == 0){
				ex_mem_i->ALU_result = id_ex->rs_data; //Move
				//printf("%x move\n", ex_mem_i->PC);
			}
			else {
				ex_mem_i->ALU_result = id_ex->rs_data + id_ex->rt_data; //Add Unsigned
				//printf("%x addu\n", ex_mem_i->PC);
			}
			break;
		case 0x24:
			ex_mem_i->ALU_result = id_ex->rs_data & id_ex->rt_data;  //And
			break;
		case 0x08:
			//printf("%x jr\n", ex_mem_i->PC);
			break;
		case 0x27:
			ex_mem_i->ALU_result = ~(id_ex->rs_data | id_ex->rt_data); //Nor
			break;
		case 0x25:
			ex_mem_i->ALU_result = id_ex->rs_data | id_ex->rt_data; //Or
			break;
		case 0x2a:
			if (id_ex->rs_data < id_ex->rt_data){ // Set_Less_Than
				ex_mem_i->ALU_result = 1;
			}
			else{
				ex_mem_i->ALU_result = 0;
			}
			//ex_mem_i->rt_data = (id_ex->rs_data < id_ex->rt_data) ? 1 : 0; 
			//printf("%x slt", ex_mem_i->PC);
			break;
		case 0x2b:
			if (id_ex->rs_data < id_ex->SignExt){ // Set_Less_Than_Imm_Unsigned
				ex_mem_i->rt_data = 1;
			}
			else{
				ex_mem_i->rt_data = 0;
			}
			//ex_mem_i->rt_data = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
			break;
		case 0x00:
			if (id_ex->immediate != 0){
				ex_mem_i->ALU_result = id_ex->rt_data << id_ex->sh; //Shift_Left_Logical
				//printf("%x sll\n", ex_mem_i->PC);
			}
			else{
				//printf("%x nop\n", ex_mem_i->PC);
			}
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
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		break;
	case 0x09:
		if (id_ex->rs_address != 0){
			ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt; //Add_imm_Unsigned
			//printf("%x addiu\n", ex_mem_i->PC);
		}
		else{
			ex_mem_i->ALU_result = id_ex->immediate; //Load_Immediate
			//printf("%x li\n", ex_mem_i->PC);
		}
		break;
	case And_Immediate:
		ex_mem_i->ALU_result = id_ex->rs_data & id_ex->immediate;
		break;
	case Branch_On_Equal:
		if (id_ex->rt_address != 0){//Branch_On_Equal
			if (id_ex->rs_data == id_ex->rt_data){
				ex_mem_i->bcond = 1;
			}
			else ex_mem_i->bcond = 0;
		}
		else{//Branch_On_Equal_Zero
			if (id_ex->rs_data == 0){
				ex_mem_i->bcond = 1;
			}
			else{
				ex_mem_i->bcond = 0;
			}
			//printf("%x beqz\n", ex_mem_i->PC);
		}
		break;
	case 0x5:
		if (id_ex->rt_address != 0){
			if (id_ex->rs_data != id_ex->rt_data){ //Branch_On_Not_Equal
				ex_mem_i->bcond = 1;
			}
			else ex_mem_i->bcond = 0;
			//printf("%x bne\n", ex_mem_i->PC);
		}
		else if (id_ex->rs_address != 0){ //Branch_On_Not_Equal_Zero
			if (id_ex->rs_data != 0){
				ex_mem_i->bcond = 1;
			}
			else ex_mem_i->bcond = 0;
			//printf("%x bnez\n", ex_mem_i->PC);
		}
		break;
	case Jump:
		//printf("%x j\n", ex_mem_i->PC);
		break;
	case Jump_And_Link:
		//printf("%x jal\n", ex_mem_i->PC);
		break;
	case Load_Halfword_Unsigned:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		break;
	case Load_Linked:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		break;
	case Load_Upper_Imm:
		ex_mem_i->ALU_result = id_ex->immediate << 16;
		//printf("%x lui\n", ex_mem_i->PC);
		break;
	case Load_Word:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		//printf("%x lw\n", ex_mem_i->PC);
		break;
	case Or_Immediate:
		ex_mem_i->ALU_result = id_ex->rs_data | id_ex->immediate;
		break;
	case Set_Less_Than_Imm:
		if (id_ex->rs_data < id_ex->SignExt){
			ex_mem_i->ALU_result = 1;
		}
		else ex_mem_i->ALU_result = 0;
		//	ex_mem_i->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
		//printf("%x slti\n", ex_mem_i->PC);
		break;
	case Set_Less_Than_Imm_Unsigned:
		ex_mem_i->ALU_result = (id_ex->rs_data < id_ex->SignExt) ? 1 : 0;
		break;
	case Store_Conditional:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		break;
	case Store_Halfword:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		break;
	case Store_Word:
		ex_mem_i->ALU_result = id_ex->rs_data + id_ex->SignExt;
		//printf("%x sw\n", ex_mem_i->PC);
		break;
	}

	////////////////control
	ex_mem_i->ALUSrc = id_ex->ALUSrc;
	ex_mem_i->branch = id_ex->branch;
	ex_mem_i->MemWrite = id_ex->MemWrite;
	ex_mem_i->MemRead = id_ex->MemRead;
	ex_mem_i->RegWrite = id_ex->RegWrite;
	ex_mem_i->MemtoReg = id_ex->MemtoReg;
	ex_mem_i->jal = id_ex->jal;
}

void Memory(EX_MEM* ex_mem, MEM_WB* mem_wb_i, MEM_WB* mem_wb_o){
	
	/////////////데이터 전달
	mem_wb_i->PC = ex_mem->PC;
	mem_wb_i->address = ex_mem->address;
	mem_wb_i->ALU_result = ex_mem->ALU_result;

	/////////////control value 전달
	mem_wb_i->ALUSrc = ex_mem->ALUSrc;
	mem_wb_i->MemtoReg = ex_mem->MemtoReg;
	mem_wb_i->RegWrite = ex_mem->RegWrite;
	mem_wb_i->jal = ex_mem->jal;



	if (ex_mem->MemWrite == 1){
		Memory_Write((ex_mem->ALU_result), ex_mem->rt_data);
		mem_wb_i->Mem_data = ex_mem->rt_data;
	}
	else if (ex_mem->MemRead == 1){
		mem_wb_i->Mem_data = Memory_Read((ex_mem->ALU_result));
	}
}

void Write_Back(MEM_WB* mem_wb){
	if (mem_wb->MemtoReg == 1){
		if (mem_wb->RegWrite == 1){
			reg[mem_wb->address] = mem_wb->Mem_data;
		}
	}
	else{
		if (mem_wb->RegWrite == 1){
			if (mem_wb->jal == 1){
				reg[31] = (mem_wb->PC) + 4;
			}
			else{
				reg[mem_wb->address] = mem_wb->ALU_result;
			}
		}
	}
}
