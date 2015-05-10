#include <stdio.h>

typedef struct IF_ID{
	int* PC;
	int instruction;
	//valid
}IF_ID;

typedef struct ID_EX{
	int* PC;
	int opcode;
	int rs_data;
	int rt_data;
	int SignExt;
	int rt_address;
	int rd_address;
	int sh;
	int func;
}ID_EX;

typedef struct EX_MEM{
	int* PC;
	int blanch_address;
	int ALU_result;
	int data2;
	int rd_address;
}EX_MEM;

typedef struct MEM_WB{
	int data;
	int address;
	int dest_address;
}MEM_WB;

int SignExtImm(int imm);
int BranchAddr(int imm);
int JumpAddr(int imm);

int Add(int rs, int rt);
void Add_Immediate(int rs, int rt, int imm);
void Add_Imm_Unsigned(int rs, int rt, int imm);
void Add_Unsighed(int rd, int rs, int rt);

void And(int rd, int rs, int rt);
void And_Immediate(int rs, int rt, int imm);

void Branch_On_Equal(int rs, int rt, int imm);
void Branch_On_Not_Equal(int rs, int rt, int imm);
void Branch_On_Not_Equal_Zero(int rs, int imm); //Ãß°¡


void Jump(int address);
void Jump_And_Link(int address);
void Jump_Register(int rs);

//void Load_Byte_Unsigned(int rs, int rt, int imm);
void Load_Halfword_Unsigned(int rs, int rt, int imm);
void Load_Linked(int rs, int rt, int imm);
void Load_Upper_Imm(int rs, int rt, int imm);
void Load_Word(int rs, int rt, int imm);

void Nor(int rd, int rs, int rt);
void Or(int rd, int rs, int rt);
void Or_Immediate(int rs, int rt, int imm);
void Set_Less_Than(int rd, int rs, int rt);
void Set_Less_Than_Imm(int rs, int rt, int imm);
void Set_Less_Than_Imm_Unsigned(int rs, int rt, int imm);
void Set_Less_Than_Unsig(int rd, int rs, int rt);

void Shift_Left_Logical(int rd, int rt, int sh);
void Shith_Right_Logical(int rd, int rt, int sh);

//void Store_Byte(int rs, int rt, int imm);
void Store_Conditional(int rs, int rt, int imm);
void Store_Halfword(int rs, int rt, int imm);
void Store_Word(int rs, int rt, int imm);

void Substract(int rd, int rs, int rt);
void Substract_Unsigned(int rd, int rs, int rt);

void Divide(int rd, int rs, int rt);
void Divide_Unsigned(int rd, int rs, int rt);

void Move_From_Hi(int rd);
void Move_From_Low(int rd);

void Multiply(int rs, int rt);
void Multiply_Unsigned(int rs, int rt);
//void Shift_Right_Arith(int rd, int rt, int shamt);





//psedo instruction set
void Move(int rd, int rs);
void Load_Immediate(int rt, int immediate);





