#include <stdio.h>

typedef struct IF_ID{
	int PC;
	int instruction;
}IF_ID;

typedef struct ID_EX{
	int PC;
	int rs_data;
	int rt_data;
	int immediate;
	int SignExt;
	int rt_address;
	int rd_address;
	int rs_address; //data dependency Ȯ���ϱ� ����
	int j_address;
	int sh;
	int func;
	//CONTROL
	int ALUSrc; //forwarding Ȯ�ο�
	int ALUop; //opcode ���
	int RegDst;
	int branch;
	int RegWrite;
	int MemWrite;
	int MemRead;
	int MemtoReg;
	int jump;
	int jr;
	int jal;
}ID_EX;

typedef struct EX_MEM{
	int PC;
	int b_address;
	int j_address;
	int ALU_result;
	int rs_data; //jr 
	int rt_data;
	int address;
	//CONTROL
	int ALUSrc; //forwarding Ȯ�ο�
	int branch;
	int bcond;
	int MemWrite;
	int MemRead;
	int MemtoReg;
	int RegWrite;
	int jal;
}EX_MEM;

typedef struct MEM_WB{
	int PC;
	int Mem_data;
	int ALU_result;
	int address;
	//CONTROL
	int ALUSrc; //forwarding Ȯ�ο�
	int MemtoReg;
	int RegWrite;
	int jal;
}MEM_WB;

typedef struct BTB{
	int PC;
	int Branch_target_address;
	int twobit_counter;
	//CONTROL
	int jal;
}BTB;



int SignExtImm(int imm);
int BranchAddr(int imm);
int JumpAddr(int imm, int* PC);

