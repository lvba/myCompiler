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
static vector<struct intermedia> condStack;//����Ƕ�������жϵ�ջ
static vector<int> paramStack;//���ں���������ջ
static int strNum = 0;
static string printStr = "";
static int sp = 0;

void printMips()
{
	for (int i = 0; i < mipsTable.size(); ++i) {
		if (mipsTable[i]->r1 == ".space" || mipsTable[i]->r1 == ".asciiz") {
			cout << mipsTable[i]->instr << " " 
				 << mipsTable[i]->r1 << " " 
				 << mipsTable[i]->r2 << endl;
		} else {
			cout << mipsTable[i]->instr;
			if (mipsTable[i]->r1 != "") {
				cout << " "<< mipsTable[i]->r1;
				if (mipsTable[i]->r2 != "") {
					cout << ", " << mipsTable[i]->r2;
					if(mipsTable[i]->r3 != "")
						cout << ", " << mipsTable[i]->r3;
				}
			}
			cout << endl;
		}
	}
}

string genStrLabel()
{
	string ret = "STRING" + to_string(strNum++);
	return ret;
}

void genOneCode(string instr, string r1, string r2, string r3)
{
	struct mipsAsm* mipsCode = new struct mipsAsm;
	mipsCode->instr = instr;
	mipsCode->r1 = r1;
	mipsCode->r2 = r2;
	mipsCode->r3 = r3;
	mipsTable.push_back(mipsCode);
}

int findAndSet(string reg, int exprInd, int i)
{
	int isTEMP = 0;
	if (imTable.exprs[i].expr[exprInd].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
		string str = imTable.exprs[i].expr[exprInd];
		if (str.substr(0, 5) == "_TEMP") {
			string str = imTable.exprs[i].expr[exprInd];
			str.erase(str.find("_TEMP"), 5);
			istringstream is(str);
			int tempNum;
			is >> tempNum;
			genOneCode("lw", reg, to_string(tempNum * 4) + "($s1)", "");
			isTEMP = 1;
		}
	}
	if (isTEMP == 0) { //��ͨ��������������������
		int find = searchAllLevel(imTable.exprs[i].expr[exprInd], nowLevel);
		if (find != -1) { //��ͨ��������������
			int addr = symTable.syms[find].addr;
			genOneCode("lw", reg, to_string(addr) + "($s0)", "");
			if (symTable.syms[find].type == 1)
				return 1;
		} else { //����
			genOneCode("li", reg, imTable.exprs[i].expr[exprInd], "");
			if (imTable.exprs[i].expr[exprInd][0] == '\'')
				return 1;
		}
	}
	return -1;
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
	genOneCode("la", "$s2", "stack", "");
	genOneCode("la", "$s1", "temp", "");
	//��ת��main����
	genOneCode("j", "voidmain", "", "");
}

void genMips()
{
	initial();
	//Ϊÿһ���м�ʽ���ɻ��
	for (int i = 0; i < imTable.ind; ++i) {
		int find, type = 1;
		string label;
		int isTEMP;
		struct intermedia jump;
		//printEachIm(i);
		switch (imTable.exprs[i].type) {
			case VAR:
				find = searchWithLevel(imTable.exprs[i].expr[2], 1, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr = memOffset;
					memOffset += 4;
				}
				break;
			case CONST:	
				//�Ѿ����˳����滻������Ҫ������mips��ʱ���õ�������
				break;
			case VARASS:
				//�鱻��ֵ�ı���
				find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
				if (find == -1) { //_TEMP����
					string str = imTable.exprs[i].expr[0];
					str.erase(str.find("_TEMP"), 5);
					istringstream is(str);
					int tempNum;
					is >> tempNum;
					find = tempNum;
					type = -1;
				}
				//�鸳ֵ���ĵ�һ��������������ֵ����$t0
				if (imTable.exprs[i].expr[1] == "_RET")
					genOneCode("move", "$t0", "$v0", "");
				else {
					int isTEMP = 0;
					if (imTable.exprs[i].expr[1].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
						string str = imTable.exprs[i].expr[1];
						if (str.substr(0, 5) == "_TEMP") {
							string str = imTable.exprs[i].expr[1];
							str.erase(str.find("_TEMP"), 5);
							istringstream is(str);
							int tempNum;
							is >> tempNum;
							genOneCode("lw", "$t0", to_string(tempNum * 4) + "($s1)", "");
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //��ͨ��������������������
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //��ͨ��������������
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
						} else { //����
							genOneCode("li", "$t0", imTable.exprs[i].expr[1], "");
						}
					}
				}
				if (imTable.exprs[i].expr[2] != "") {
					//�鸳ֵ���ĵڶ���������������$t1
					int isTEMP = 0;
					if (imTable.exprs[i].expr[3].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
						string str = imTable.exprs[i].expr[3];
						if (str.substr(0, 5) == "_TEMP") {
							string str = imTable.exprs[i].expr[3];
							str.erase(str.find("_TEMP"), 5);
							istringstream is(str);
							int tempNum;
							is >> tempNum;
							genOneCode("lw", "$t1", to_string(tempNum * 4) + "($s1)", "");
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //��ͨ��������������������
						int find = searchAllLevel(imTable.exprs[i].expr[3], nowLevel);
						if (find != -1) { //��ͨ��������������
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$t1", to_string(addr) + "($s0)", "");
						} else { //����
							genOneCode("li", "$t1", imTable.exprs[i].expr[3], "");
						}
					}
					//t0��t1������(+-*/)����������t0
					if (imTable.exprs[i].expr[2] == "+")
						genOneCode("add", "$t0", "$t0", "$t1");
					if (imTable.exprs[i].expr[2] == "-")
						genOneCode("sub", "$t0", "$t0", "$t1");
					if (imTable.exprs[i].expr[2] == "*") {
						genOneCode("mult", "$t0", "$t1", "");
						genOneCode("mflo", "$t0", "", "");
					}
					if (imTable.exprs[i].expr[2] == "/") {
						genOneCode("div", "$t0", "$t1", "");
						genOneCode("mflo", "$t0", "", "");
					}
				}
				if (type == 1) { //��ͨ������������
					int addr = symTable.syms[find].addr;
					genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
				} else { //_TEMP����
					int addr = find * 4;
					genOneCode("sw", "$t0", to_string(addr) + "($s1)", "");
				}
				break;
			case FUNC:
				if(nowLevel != 0) //Ϊ��һ���������ɷ�����ת���
					genOneCode("jr", "$ra", "", "");
				label = imTable.exprs[i].expr[0] + imTable.exprs[i].expr[1] + ":";
				genOneCode(label, "", "", "");
				++nowLevel;
				break;
			case PARAM:
				find = searchWithLevel(imTable.exprs[i].expr[2], 2, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr = memOffset;
					memOffset += 4;
				}
				break;
			case PUSH:
				paramStack.push_back(i);
				break;
			case ARRAY:
				find = searchWithLevel(imTable.exprs[i].expr[2], 3, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr = memOffset;
					istringstream is(imTable.exprs[i].expr[3]);
					int dimen;
					is >> dimen;
					memOffset += (4 * dimen);
				}
				break;
			case CALL:
				if (imTable.exprs[i].expr[1] == "printf") { //����printf
					if (imTable.exprs[i].expr[2] == "1") {
						int ind = paramStack[paramStack.size() - 1];
						struct intermedia param = imTable.exprs[ind];
						paramStack.pop_back();
						if (param.expr[1][0] == '"') { //printf '('���ַ�����')'
							printStr = genStrLabel();
							struct mipsAsm *asciiz = new struct mipsAsm;
							asciiz->instr = printStr + ":";
							asciiz->r1 = ".asciiz";
							asciiz->r2 = param.expr[1];
							asciiz->r3 = "";
							mipsTable.insert(mipsTable.begin() + 4, asciiz);
							//����ַ���
							genOneCode("li", "$v0", "4", "");
							genOneCode("la", "$a0", printStr, "");
							genOneCode("syscall", "", "", "");
						} else { //printf '('�����ʽ��')'
							//��push�еĲ���������$t0
							int isChar = findAndSet("$t0", 1, ind);
							if (isChar == 1) { //��char���
								genOneCode("li", "$v0", "11", "");
								genOneCode("move", "$a0", "$t0", "");
								genOneCode("syscall", "", "", "");
							} else { //���������
								genOneCode("li", "$v0", "1", "");
								genOneCode("move", "$a0", "$t0", "");
								genOneCode("syscall", "", "", "");
							}
						}
					} else { //printf'('���ַ�����,�����ʽ��')'
						int indExpr = paramStack[paramStack.size() - 1];
						int indStr = paramStack[paramStack.size() - 2];
						paramStack.pop_back();
						paramStack.pop_back();
						struct intermedia param = imTable.exprs[indStr];
						//������ַ���
						printStr = genStrLabel();
						struct mipsAsm *asciiz = new struct mipsAsm;
						asciiz->instr = printStr + ":";
						asciiz->r1 = ".asciiz";
						asciiz->r2 = param.expr[1];
						asciiz->r3 = "";
						mipsTable.insert(mipsTable.begin() + 4, asciiz);
						genOneCode("li", "$v0", "4", "");
						genOneCode("la", "$a0", printStr, "");
						genOneCode("syscall", "", "", "");
						//��������ʽ
						int isChar = findAndSet("$t0", 1, indExpr);
						if (isChar == 1) { //��char���
							genOneCode("li", "$v0", "11", "");
							genOneCode("move", "$a0", "$t0", "");
							genOneCode("syscall", "", "", "");
						} else { //���������
							genOneCode("li", "$v0", "1", "");
							genOneCode("move", "$a0", "$t0", "");
							genOneCode("syscall", "", "", "");
						}
					}
				} else {
					if (imTable.exprs[i].expr[1] == "scanf") { //����scanf
						istringstream is(imTable.exprs[i].expr[2]);
						int paramNum;
						is >> paramNum;
						for (int paraInd = paramStack.size() - paramNum; paraInd < paramStack.size(); ++paraInd) {
							int find = searchAllLevel(imTable.exprs[paramStack[paraInd]].expr[1], nowLevel);
							if (find == -1)
								error(32, "");
							else
								if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
									error(47, "");
							int addr = symTable.syms[find].addr;
							if (symTable.syms[find].type == 0) { //������
								genOneCode("li", "$v0", "5", "");
								genOneCode("syscall", "", "", "");
								genOneCode("sw", "$v0", to_string(addr) + "($s0)", "");
							} else { //���ַ�
								genOneCode("li", "$v0", "12", "");
								genOneCode("syscall", "", "", "");
								genOneCode("sw", "$v0", to_string(addr) + "($s0)", "");
							}
						}
						//paramStack��ջ
						for (int temp = 0; temp < paramNum; ++temp)
							paramStack.pop_back();
					} else { //�����Զ��庯��������ǰ�����ֳ������ú�ָ��ֳ�
						string funcName = imTable.exprs[i].expr[1];
						string tempArr[3] = { "int", "char", "void" };
						int funcInd = -1; //�����ڷ��ű��е��±�
						for (int x = 0; x < symTable.funcInd.size(); ++x) {
							if (symTable.syms[symTable.funcInd[x]].name == funcName) {
								funcInd = symTable.funcInd[x];
								break;
							}
						}
						int paramNum = symTable.syms[funcInd].size;
						//�ȱ����ֳ�������������������ͨ���������������_TEMP������$ra�Ĵ�����$v0�Ĵ���
						int callFuncInd = symTable.funcInd[nowLevel - 1];
						++callFuncInd;//ָ��ú����ĵ�һ���ֲ�����
						istringstream is(imTable.exprs[i].expr[2]);
						int maxTempNum;
						is >> maxTempNum;
							//���溯����������ͨ�������������
						if (nowLevel != symTable.funcInd.size()) {
							while (callFuncInd < symTable.top && symTable.syms[callFuncInd].spaceLv == nowLevel) {
								if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
									int addr = symTable.syms[callFuncInd].addr;
									genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
									genOneCode("sw", "$t0", to_string(sp) + "($s2)", "");
									sp += 4;
								}
								if (symTable.syms[callFuncInd].object == 3) { //��������
									int addr = symTable.syms[callFuncInd].addr;
									int dimen = symTable.syms[callFuncInd].size;
									for (int ind = 0; ind < dimen; ++ind) {
										int ad = addr + ind * 4;
										genOneCode("lw", "$t0", to_string(ad) + "($s0)", "");
										genOneCode("sw", "$t0", to_string(sp) + "($s2)", "");
										sp += 4;
									}
								}
								++callFuncInd;
							}
						}						
							//����_TEMP����
						for (int tempInd = 0; tempInd < maxTempNum; ++tempInd) {
							genOneCode("lw", "$t0", to_string(4 * tempInd) + "($s1)", "");
							genOneCode("sw", "$t0", to_string(sp) + "($s2)", "");
							sp += 4;
						}
							//����ra��v0�Ĵ���
						genOneCode("sw", "$ra", to_string(sp) + "($s2)", "");
						sp += 4;
						genOneCode("sw", "$v0", to_string(sp) + "($s2)", "");
						sp += 4;
						//��PUSH����
						int pushInd = paramStack.size() - paramNum;
						for (int paraInd = funcInd + 1; symTable.syms[paraInd].object == 2; ++paraInd) {
							findAndSet("$t0", 1, paramStack[pushInd]);
							int addr = symTable.syms[paraInd].addr;
							genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
							++pushInd;
						}
						for (int temp = 0; temp < paramNum; ++temp)
							paramStack.pop_back();
						//��jal���ú���
						genOneCode("jal", tempArr[symTable.syms[funcInd].type] + funcName, "", "");
						//���ָ��ֳ�������ָ���������
						sp -= 4;
						genOneCode("lw", "$v0", to_string(sp) + "($s2)", "");
						sp -= 4;
						genOneCode("lw", "$ra", to_string(sp) + "($s2)", "");
						sp -= 4;
							//�ָ�_TEMP����
						for (int tempInd = maxTempNum - 1; tempInd >= 0; --tempInd) {
							genOneCode("lw", "$t0", to_string(sp) + "($s2)", "");
							sp -= 4;
							genOneCode("sw", "$t0", to_string(tempInd * 4) + "($s1)", "");
						}
							//�ָ�������������ͨ�������������
						if (nowLevel != symTable.funcInd.size()) {
							--callFuncInd;//��ʱָ���������һ������
							while (symTable.syms[callFuncInd].spaceLv == nowLevel) {
								if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
									int addr = symTable.syms[callFuncInd].addr;
									genOneCode("lw", "$t0", to_string(sp) + "($s2)", "");
									sp -= 4;
									genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
								}
								if (symTable.syms[callFuncInd].object == 3) { //��������
									int addr = symTable.syms[callFuncInd].addr;
									int dimen = symTable.syms[callFuncInd].size;
									for (int ind = dimen - 1; ind >= 0; --ind) {
										int ad = addr + 4 * ind;
										genOneCode("lw", "$t0", to_string(sp) + "($s2)", "");
										sp -= 4;
										genOneCode("sw", "$t0", to_string(ad) + "($s0)", "");
									}
								}
								--callFuncInd;
							}
						}						
						sp += 4;
					}
				}
				break;
			case RET:
				if (imTable.exprs[i].expr[1] != "") { //�з���ֵ�ķ������
					//�鷵��ֵ�����ͺ�ֵ������$v0
					int isTEMP = 0;
					if (imTable.exprs[i].expr[1].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
						string str = imTable.exprs[i].expr[1];
						if (str.substr(0, 5) == "_TEMP") {
							string str = imTable.exprs[i].expr[1];
							str.erase(str.find("_TEMP"), 5);
							istringstream is(str);
							int tempNum;
							is >> tempNum;
							genOneCode("lw", "$v0", to_string(tempNum * 4) + "($s1)", "");
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //��ͨ��������������������
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //��ͨ��������������
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$v0", to_string(addr) + "($s0)", "");
						} else { //����
							genOneCode("li", "$v0", imTable.exprs[i].expr[1], "");
						}
					}
				}
				break;
			case COMPARE:
				//���һ�����������������$t0��
				isTEMP = 0;
				if (imTable.exprs[i].expr[0].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
					string str = imTable.exprs[i].expr[0];
					if (str.substr(0, 5) == "_TEMP") {
						string str = imTable.exprs[i].expr[0];
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						genOneCode("lw", "$t0", to_string(tempNum * 4) + "($s1)", "");
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) { //��ͨ��������������������
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find != -1) { //��ͨ��������������
						int addr = symTable.syms[find].addr;
						genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
					} else { //����
						genOneCode("li", "$t0", imTable.exprs[i].expr[0], "");
					}
				}
				//��ڶ������������������$t1��
				isTEMP = 0;
				if (imTable.exprs[i].expr[2].size() > 5) {//_TEMP���߳��ȴ���5����ͨ����
					string str = imTable.exprs[i].expr[2];
					if (str.substr(0, 5) == "_TEMP") {
						string str = imTable.exprs[i].expr[2];
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						genOneCode("lw", "$t1", to_string(tempNum * 4) + "($s1)", "");
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) { //��ͨ��������������������
					int find = searchAllLevel(imTable.exprs[i].expr[2], nowLevel);
					if (find != -1) { //��ͨ��������������
						int addr = symTable.syms[find].addr;
						genOneCode("lw", "$t1", to_string(addr) + "($s0)", "");
					} else { //����
						genOneCode("li", "$t1", imTable.exprs[i].expr[2], "");
					}
				}
				//���ݲ��������бȽ�
				jump = condStack[condStack.size() - 1];
				condStack.pop_back();
				if (imTable.exprs[i].expr[1] == "==") {
					if (jump.type == BZ)
						genOneCode("bne", "$t0", "$t1", jump.expr[1]);
					else
						genOneCode("beq", "$t0", "$t1", jump.expr[1]);
				}
				if (imTable.exprs[i].expr[1] == "!=") {
					if (jump.type == BZ)
						genOneCode("beq", "$t0", "$t1", jump.expr[1]);
					else
						genOneCode("bne", "$t0", "$t1", jump.expr[1]);
				}
				if (imTable.exprs[i].expr[1] == "<") {
					genOneCode("sub", "$t0", "$t0", "$t1");
					if (jump.type == BZ)
						genOneCode("bgez", "$t0", jump.expr[1], "");
					else
						genOneCode("bltz", "$t0", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == "<=") {
					genOneCode("sub", "$t0", "$t0", "$t1");
					if (jump.type == BZ)
						genOneCode("bgtz", "$t0", jump.expr[1], "");
					else
						genOneCode("blez", "$t0", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == ">") {
					genOneCode("sub", "$t0", "$t0", "$t1");
					if (jump.type == BZ)
						genOneCode("blez", "$t0", jump.expr[1], "");
					else
						genOneCode("bgtz", "$t0", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == ">=") {
					genOneCode("sub", "$t0", "$t0", "$t1");
					if (jump.type == BZ)
						genOneCode("bltz", "$t0", jump.expr[1], "");
					else
						genOneCode("bgez", "$t0", jump.expr[1], "");
				}
				break;
			case GOTO:
				genOneCode("j", imTable.exprs[i].expr[1], "", "");
				break;
			case BNZ:
				condStack.push_back(imTable.exprs[i]);
				break;
			case BZ:
				condStack.push_back(imTable.exprs[i]);
				break;
			case ARRASS:
				if (imTable.exprs[i].expr[1] == "[]") { //a[]b = c
					//�����b������ֵ����$t0
					findAndSet("$t0", 2, i);
					//�����c������ֵ����$t1
					findAndSet("$t1", 3, i);
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find == -1)
						error(30, "");
					else
						if (symTable.syms[find].object != 3)
							error(46, "");
					int addr = symTable.syms[find].addr;
					genOneCode("add", "$t0", "$t0", "$t0");
					genOneCode("add", "$t0", "$t0", "$t0");
					genOneCode("li", "$t2", to_string(addr), "");
					genOneCode("add", "$t2", "$t2", "$t0");
					genOneCode("sw", "$t1", "0($t2)", "");
				} else { //a = b[]c
					//�鱻��ֵ�ı���
					int type = 1;
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find == -1) { //_TEMP����
						string str = imTable.exprs[i].expr[0];
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						find = tempNum;
						type = -1;
					}
					//�����c����ֵ��$t0
					findAndSet("$t0", 3, i);
					find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
					if (find == -1)
						error(30, "");
					else
						if (symTable.syms[find].object != 3)
							error(46, "");
					int addr = symTable.syms[find].addr;
					genOneCode("add", "$t0", "$t0", "$t0");
					genOneCode("add", "$t0", "$t0", "$t0");
					genOneCode("li", "$t2", to_string(addr), "");
					genOneCode("add", "$t2", "$t2", "$t0");
					//��b[c]��ֵ����$t1
					genOneCode("lw", "$t1", "0($t2)", "");
					//��b[c]����a
					if (type == 1) {
						int addr = symTable.syms[find].addr;
						genOneCode("sw", "$t1", to_string(addr) + "($s0)", "");
					} else {
						int addr = find * 4;
						genOneCode("sw", "$t1", to_string(addr) + "($s1)", "");
					}
				}
				break;
			case LABEL:
				genOneCode(imTable.exprs[i].expr[0] + ":", "", "", "");
				break;
		}
	}
}