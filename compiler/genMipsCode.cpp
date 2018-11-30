#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static int memOffset = 0;//��ǰ�洢����ָ���ƫ��
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

void initial()//��ʼ�����ɴ���
{
	//����.data
	genOneCode(".data", "", "", "");
	//������б�����memory
	genOneCode("memory:", ".space", "5000", "");
	//����ջstack
	genOneCode("stack:", ".space", "5000", "");
	//�洢��ʱ����temp
	genOneCode("temp:", ".space", "1000", "");
	//����.text
	genOneCode(".text", "", "", "");
	//Ϊ����.space��ַ���Ĵ���
	genOneCode("la", "$s0", "memory", "");
	genOneCode("la", "$sp", "stack", "");
	genOneCode("la", "$s1", "temp", "");
	//��ת��main����
	genOneCode("j", "voidmain", "", "");
}

void genMips()
{
	initial();
	//Ϊÿһ���м�ʽ���ɻ��
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
				//�Ѿ����˳����滻������Ҫ������mips��ʱ���õ�������
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