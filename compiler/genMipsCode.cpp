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
	//����.data��.text
	struct mipsAsm data;
	data.type = 'N';
	data.instr = ".data";
	mipsTable.push_back(data);
	struct mipsAsm text;
	text.type = 'N';
	text.instr = ".text";
	mipsTable.push_back(text);
	//Ϊÿһ���м�ʽ���ɻ��
	for (int i = 0; i < imTable.ind; ++i) {
		struct mipsAsm mipsCode;
		switch (imTable.exprs[i].type) {
			case VAR:

				break;
			case CONST:	
				//�Ѿ����˳����滻������Ҫ������mips��ʱ���õ�������
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