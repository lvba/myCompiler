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
static vector<struct intermedia> condStack;//用于嵌套条件判断的栈
static vector<int> paramStack;//用于函数变量的栈
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
	if (imTable.exprs[i].expr[exprInd].size() > 5) {//_TEMP或者长度大于5的普通变量
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
	if (isTEMP == 0) { //普通变量，函数变量，常量
		int find = searchAllLevel(imTable.exprs[i].expr[exprInd], nowLevel);
		if (find != -1) { //普通变量，函数变量
			int addr = symTable.syms[find].addr;
			genOneCode("lw", reg, to_string(addr) + "($s0)", "");
			if (symTable.syms[find].type == 1)
				return 1;
		} else { //常量
			genOneCode("li", reg, imTable.exprs[i].expr[exprInd], "");
			if (imTable.exprs[i].expr[exprInd][0] == '\'')
				return 1;
		}
	}
	return -1;
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
	genOneCode("la", "$s2", "stack", "");
	genOneCode("la", "$s1", "temp", "");
	//跳转到main函数
	genOneCode("j", "voidmain", "", "");
}

void genMips()
{
	initial();
	//为每一条中间式生成汇编
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
				//已经做了常量替换，不需要在生成mips的时候用到常量名
				break;
			case VARASS:
				//查被赋值的变量
				find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
				if (find == -1) { //_TEMP变量
					string str = imTable.exprs[i].expr[0];
					str.erase(str.find("_TEMP"), 5);
					istringstream is(str);
					int tempNum;
					is >> tempNum;
					find = tempNum;
					type = -1;
				}
				//查赋值语句的第一个操作数，将其值存入$t0
				if (imTable.exprs[i].expr[1] == "_RET")
					genOneCode("move", "$t0", "$v0", "");
				else {
					int isTEMP = 0;
					if (imTable.exprs[i].expr[1].size() > 5) {//_TEMP或者长度大于5的普通变量
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
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //普通变量，函数变量
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
						} else { //常量
							genOneCode("li", "$t0", imTable.exprs[i].expr[1], "");
						}
					}
				}
				if (imTable.exprs[i].expr[2] != "") {
					//查赋值语句的第二个操作符，存入$t1
					int isTEMP = 0;
					if (imTable.exprs[i].expr[3].size() > 5) {//_TEMP或者长度大于5的普通变量
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
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[3], nowLevel);
						if (find != -1) { //普通变量，函数变量
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$t1", to_string(addr) + "($s0)", "");
						} else { //常量
							genOneCode("li", "$t1", imTable.exprs[i].expr[3], "");
						}
					}
					//t0和t1做计算(+-*/)，将结果存回t0
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
				if (type == 1) { //普通变量或函数参数
					int addr = symTable.syms[find].addr;
					genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
				} else { //_TEMP变量
					int addr = find * 4;
					genOneCode("sw", "$t0", to_string(addr) + "($s1)", "");
				}
				break;
			case FUNC:
				if(nowLevel != 0) //为上一个函数生成返回跳转语句
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
				if (imTable.exprs[i].expr[1] == "printf") { //调用printf
					if (imTable.exprs[i].expr[2] == "1") {
						int ind = paramStack[paramStack.size() - 1];
						struct intermedia param = imTable.exprs[ind];
						paramStack.pop_back();
						if (param.expr[1][0] == '"') { //printf '('＜字符串＞')'
							printStr = genStrLabel();
							struct mipsAsm *asciiz = new struct mipsAsm;
							asciiz->instr = printStr + ":";
							asciiz->r1 = ".asciiz";
							asciiz->r2 = param.expr[1];
							asciiz->r3 = "";
							mipsTable.insert(mipsTable.begin() + 4, asciiz);
							//输出字符串
							genOneCode("li", "$v0", "4", "");
							genOneCode("la", "$a0", printStr, "");
							genOneCode("syscall", "", "", "");
						} else { //printf '('＜表达式＞')'
							//查push中的参数，存入$t0
							int isChar = findAndSet("$t0", 1, ind);
							if (isChar == 1) { //按char输出
								genOneCode("li", "$v0", "11", "");
								genOneCode("move", "$a0", "$t0", "");
								genOneCode("syscall", "", "", "");
							} else { //按整型输出
								genOneCode("li", "$v0", "1", "");
								genOneCode("move", "$a0", "$t0", "");
								genOneCode("syscall", "", "", "");
							}
						}
					} else { //printf'('＜字符串＞,＜表达式＞')'
						int indExpr = paramStack[paramStack.size() - 1];
						int indStr = paramStack[paramStack.size() - 2];
						paramStack.pop_back();
						paramStack.pop_back();
						struct intermedia param = imTable.exprs[indStr];
						//先输出字符串
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
						//再输出表达式
						int isChar = findAndSet("$t0", 1, indExpr);
						if (isChar == 1) { //按char输出
							genOneCode("li", "$v0", "11", "");
							genOneCode("move", "$a0", "$t0", "");
							genOneCode("syscall", "", "", "");
						} else { //按整型输出
							genOneCode("li", "$v0", "1", "");
							genOneCode("move", "$a0", "$t0", "");
							genOneCode("syscall", "", "", "");
						}
					}
				} else {
					if (imTable.exprs[i].expr[1] == "scanf") { //调用scanf
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
							if (symTable.syms[find].type == 0) { //读整型
								genOneCode("li", "$v0", "5", "");
								genOneCode("syscall", "", "", "");
								genOneCode("sw", "$v0", to_string(addr) + "($s0)", "");
							} else { //读字符
								genOneCode("li", "$v0", "12", "");
								genOneCode("syscall", "", "", "");
								genOneCode("sw", "$v0", to_string(addr) + "($s0)", "");
							}
						}
						//paramStack退栈
						for (int temp = 0; temp < paramNum; ++temp)
							paramStack.pop_back();
					} else { //调用自定义函数，调用前保存现场，调用后恢复现场
						string funcName = imTable.exprs[i].expr[1];
						string tempArr[3] = { "int", "char", "void" };
						int funcInd = -1; //函数在符号表中的下标
						for (int x = 0; x < symTable.funcInd.size(); ++x) {
							if (symTable.syms[symTable.funcInd[x]].name == funcName) {
								funcInd = symTable.funcInd[x];
								break;
							}
						}
						int paramNum = symTable.syms[funcInd].size;
						//先保存现场。包括函数参数，普通变量，数组变量，_TEMP变量，$ra寄存器，$v0寄存器
						int callFuncInd = symTable.funcInd[nowLevel - 1];
						++callFuncInd;//指向该函数的第一个局部变量
						istringstream is(imTable.exprs[i].expr[2]);
						int maxTempNum;
						is >> maxTempNum;
							//保存函数参数，普通变量，数组变量
						if (nowLevel != symTable.funcInd.size()) {
							while (callFuncInd < symTable.top && symTable.syms[callFuncInd].spaceLv == nowLevel) {
								if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
									int addr = symTable.syms[callFuncInd].addr;
									genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
									genOneCode("sw", "$t0", to_string(sp) + "($s2)", "");
									sp += 4;
								}
								if (symTable.syms[callFuncInd].object == 3) { //保护数组
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
							//保存_TEMP变量
						for (int tempInd = 0; tempInd < maxTempNum; ++tempInd) {
							genOneCode("lw", "$t0", to_string(4 * tempInd) + "($s1)", "");
							genOneCode("sw", "$t0", to_string(sp) + "($s2)", "");
							sp += 4;
						}
							//保存ra和v0寄存器
						genOneCode("sw", "$ra", to_string(sp) + "($s2)", "");
						sp += 4;
						genOneCode("sw", "$v0", to_string(sp) + "($s2)", "");
						sp += 4;
						//再PUSH参数
						int pushInd = paramStack.size() - paramNum;
						for (int paraInd = funcInd + 1; symTable.syms[paraInd].object == 2; ++paraInd) {
							findAndSet("$t0", 1, paramStack[pushInd]);
							int addr = symTable.syms[paraInd].addr;
							genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
							++pushInd;
						}
						for (int temp = 0; temp < paramNum; ++temp)
							paramStack.pop_back();
						//再jal调用函数
						genOneCode("jal", tempArr[symTable.syms[funcInd].type] + funcName, "", "");
						//最后恢复现场，反向恢复所有数据
						sp -= 4;
						genOneCode("lw", "$v0", to_string(sp) + "($s2)", "");
						sp -= 4;
						genOneCode("lw", "$ra", to_string(sp) + "($s2)", "");
						sp -= 4;
							//恢复_TEMP变量
						for (int tempInd = maxTempNum - 1; tempInd >= 0; --tempInd) {
							genOneCode("lw", "$t0", to_string(sp) + "($s2)", "");
							sp -= 4;
							genOneCode("sw", "$t0", to_string(tempInd * 4) + "($s1)", "");
						}
							//恢复函数参数，普通变量，数组变量
						if (nowLevel != symTable.funcInd.size()) {
							--callFuncInd;//此时指向函数内最后一个变量
							while (symTable.syms[callFuncInd].spaceLv == nowLevel) {
								if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
									int addr = symTable.syms[callFuncInd].addr;
									genOneCode("lw", "$t0", to_string(sp) + "($s2)", "");
									sp -= 4;
									genOneCode("sw", "$t0", to_string(addr) + "($s0)", "");
								}
								if (symTable.syms[callFuncInd].object == 3) { //保护数组
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
				if (imTable.exprs[i].expr[1] != "") { //有返回值的返回语句
					//查返回值的类型和值，存入$v0
					int isTEMP = 0;
					if (imTable.exprs[i].expr[1].size() > 5) {//_TEMP或者长度大于5的普通变量
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
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //普通变量，函数变量
							int addr = symTable.syms[find].addr;
							genOneCode("lw", "$v0", to_string(addr) + "($s0)", "");
						} else { //常量
							genOneCode("li", "$v0", imTable.exprs[i].expr[1], "");
						}
					}
				}
				break;
			case COMPARE:
				//查第一个操作数，将其存入$t0中
				isTEMP = 0;
				if (imTable.exprs[i].expr[0].size() > 5) {//_TEMP或者长度大于5的普通变量
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
				if (isTEMP == 0) { //普通变量，函数变量，常量
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find != -1) { //普通变量，函数变量
						int addr = symTable.syms[find].addr;
						genOneCode("lw", "$t0", to_string(addr) + "($s0)", "");
					} else { //常量
						genOneCode("li", "$t0", imTable.exprs[i].expr[0], "");
					}
				}
				//查第二个操作数，将其存在$t1中
				isTEMP = 0;
				if (imTable.exprs[i].expr[2].size() > 5) {//_TEMP或者长度大于5的普通变量
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
				if (isTEMP == 0) { //普通变量，函数变量，常量
					int find = searchAllLevel(imTable.exprs[i].expr[2], nowLevel);
					if (find != -1) { //普通变量，函数变量
						int addr = symTable.syms[find].addr;
						genOneCode("lw", "$t1", to_string(addr) + "($s0)", "");
					} else { //常量
						genOneCode("li", "$t1", imTable.exprs[i].expr[2], "");
					}
				}
				//根据操作符进行比较
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
					//查变量b，将其值赋给$t0
					findAndSet("$t0", 2, i);
					//查变量c，将其值赋给$t1
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
					//查被赋值的变量
					int type = 1;
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find == -1) { //_TEMP变量
						string str = imTable.exprs[i].expr[0];
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						find = tempNum;
						type = -1;
					}
					//查变量c，赋值给$t0
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
					//将b[c]的值存入$t1
					genOneCode("lw", "$t1", "0($t2)", "");
					//将b[c]赋给a
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