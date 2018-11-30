#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static int memOffset = 0;//当前存储分配指针的偏移
static int nowLevel = 0;

void genOneCode(string instr, string r1, string r2, string r3)
{
	struct mipsAsm* mipsCode = new struct mipsAsm;
	mipsCode->instr = instr;
	mipsCode->r1 = r1;
	mipsCode->r2 = r2;
	mipsCode->r3 = r3;
	mipsTable.push_back(mipsCode);
}

void initial()//初始化生成代码
{
	//生成.data
	genOneCode(".data", "", "", "");
	//存放所有变量的memory
	genOneCode("memory:", ".space", "5000", "");
	//运行栈stack
	genOneCode("stack:", ".space", "5000", "");
	//存储临时变量temp
	genOneCode("temp:", ".space", "1000", "");
	//生成.text
	genOneCode(".text", "", "", "");
	//为三个.space地址赋寄存器
	genOneCode("la", "$s0", "memory", "");
	genOneCode("la", "$sp", "stack", "");
	genOneCode("la", "$s1", "temp", "");
	//跳转到main函数
	genOneCode("j", "voidmain", "", "");
}

void genMips()
{
	initial();
	//为每一条中间式生成汇编
	for (int i = 0; i < imTable.ind; ++i) {
		int find;
		string label;
		switch (imTable.exprs[i].type) {
			case VAR:
				find = searchWithLevel(imTable.exprs[i].expr[2], 1, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr == memOffset;
					memOffset += 4;
				}
				break;
			case CONST:	
				//已经做了常量替换，不需要在生成mips的时候用到常量名
				break;
			case VARASS:

				break;
			case FUNC:
				label = imTable.exprs[i].expr[0] + imTable.exprs[i].expr[1] + ":";
				genOneCode(label, "", "", "");
				break;
			case PARAM:

				break;
			case PUSH:

				break;
			case ARRAY:
				find = searchWithLevel(imTable.exprs[i].expr[2], 3, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr == memOffset;
					istringstream is(imTable.exprs[i].expr[3]);
					int dimen;
					is >> dimen;
					memOffset += (4 * dimen);
				}
				break;
			case CALL:

				break;
			case RET:

				break;
			case COMPARE:

				break;
			case GOTO:

				break;
			case BNZ:

				break;
			case BZ:

				break;
			case ARRASS:

				break;
			case LABEL:

				break;
		}
	}
}