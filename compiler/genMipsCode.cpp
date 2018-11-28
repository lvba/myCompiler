#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

void genMips()
{
	//生成.data和.text
	struct mipsAsm data;
	data.type = 'N';
	data.instr = ".data";
	mipsTable.push_back(data);
	struct mipsAsm text;
	text.type = 'N';
	text.instr = ".text";
	mipsTable.push_back(text);
	//为每一条中间式生成汇编
	for (int i = 0; i < imTable.ind; ++i) {
		struct mipsAsm mipsCode;
		switch (imTable.exprs[i].type) {
			case VAR:

				break;
			case CONST:	
				//已经做了常量替换，不需要在生成mips的时候用到常量名
				break;
			case VARASS:

				break;
			case FUNC:

				break;
			case PARAM:

				break;
			case PUSH:

				break;
			case ARRAY:

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