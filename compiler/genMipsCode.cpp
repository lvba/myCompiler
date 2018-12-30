#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include<algorithm>
#include "global.h"
using namespace std;

static int memOffset = 0;//当前存储分配指针的偏移
static int nowLevel = 0;
static vector<struct intermedia> condStack;//用于嵌套条件判断的栈
static vector<int> paramStack;//用于函数变量的栈
static int strNum = 0;
static string printStr = "";
static ofstream ofile;

void printMips()
{
	ofile.open("../mipsCode.txt");
	for (int i = 0; i < mipsTable.size(); ++i) {
		if (mipsTable[i]->r1 == ".space" || mipsTable[i]->r1 == ".asciiz") {
			cout << mipsTable[i]->instr << " " 
				 << mipsTable[i]->r1 << " " 
				 << mipsTable[i]->r2 << endl;
			ofile << mipsTable[i]->instr << " "
				<< mipsTable[i]->r1 << " "
				<< mipsTable[i]->r2 << endl;
		} else {
			cout << mipsTable[i]->instr;
			ofile << mipsTable[i]->instr;
			if (mipsTable[i]->r1 != "") {
				cout << " "<< mipsTable[i]->r1;
				ofile << " " << mipsTable[i]->r1;
				if (mipsTable[i]->r2 != "") {
					cout << ", " << mipsTable[i]->r2;
					ofile << ", " << mipsTable[i]->r2;
					if (mipsTable[i]->r3 != "") {
						cout << ", " << mipsTable[i]->r3;
						ofile << ", " << mipsTable[i]->r3;
					}					
				}
			}
			cout << endl;
			ofile << endl;
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
			if(tempRegTab[tempNum] != "")
				genOneCode("move", reg, tempRegTab[tempNum], "");
			else { //分配临时寄存器
				vector<string> willBeUse;
				willBeUse.push_back(reg);
				int allocReg = getFromPool(imTable.exprs[i].expr[exprInd], nowLevel, willBeUse, tempNum, 1, 1);
				if (tempRegTab[tempNum] == "")
					cout << "致命错误：变量未分配到临时寄存器" << endl;
				genOneCode("move", reg, tempRegTab[tempNum], "");
			}
			isTEMP = 1;
		}
	}
	if (isTEMP == 0) { //普通变量，函数变量，常量
		int find = searchAllLevel(imTable.exprs[i].expr[exprInd], nowLevel);
		if (find != -1) { //普通变量，函数变量
			if (symTable.syms[find].reg != "")
				genOneCode("move", reg, symTable.syms[find].reg, "");
			else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
				vector<string> willBeUse;
				willBeUse.push_back(reg);
				int allocReg = getFromPool(imTable.exprs[i].expr[exprInd], nowLevel, willBeUse, find, 0, 1);
				if (symTable.syms[find].reg == "")
					cout << "致命错误：变量未分配到临时寄存器" << endl;
				genOneCode("move", reg, symTable.syms[find].reg, "");
			}
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
	genOneCode("temp:", ".space", "5000", "");
	//生成.text
	genOneCode(".text", "", "", "");
	//为三个.space地址赋寄存器
	genOneCode("la", "$a1", "memory", "");
	genOneCode("la", "$a2", "stack", "");
	genOneCode("la", "$a3", "temp", "");
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
		vector<pair<string, int> > assignVars; //赋值语句的所有操作数寄存器或者内存地址
		vector<string> compVars;
		//printEachIm(i);
		//进入基本块时清空临时寄存器池
		for (int bl = 0; bl < blockGraph.size(); ++bl) {
			if (i == blockGraph[bl]->codes[0]) { //进入基本块
				clearPool();
			}
		}
		switch (imTable.exprs[i].type) {
			case VAR:
				find = searchWithLevel(imTable.exprs[i].expr[2], 1, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr = memOffset;
					//cout << imTable.exprs[i].expr[2] << "的地址为" << memOffset << endl;
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
					if (tempRegTab[find] != "")
						assignVars.push_back(make_pair(tempRegTab[find], -1));
					else {
						vector<string> willBeUse;
						int allocReg = getFromPool(imTable.exprs[i].expr[0], nowLevel, willBeUse, find, 1, 0);
						if (tempRegTab[find] == "")
							cout << "致命错误：变量未分配到临时寄存器" << endl;
						assignVars.push_back(make_pair(tempRegTab[find], -1));
					}
				} else {
					if (symTable.syms[find].reg != "")
						assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
					else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
						vector<string> willBeUse;
						int allocReg = getFromPool(imTable.exprs[i].expr[0], nowLevel, willBeUse, find, 0, 0);
						if (symTable.syms[find].reg == "")
							cout << "致命错误：变量未分配到临时寄存器" << endl;
						assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
					}
				}
				//查赋值语句的第一个操作数，将其值存入$t0
				if (imTable.exprs[i].expr[1] == "_RET")
					assignVars.push_back(make_pair("$v0", -1));
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
							//查temRegTab决定是取内存还是用寄存器算
							if (tempRegTab[tempNum] != "")
								assignVars.push_back(make_pair(tempRegTab[tempNum], -1));
							else {
								vector<string> willBeUse;
								willBeUse.push_back(assignVars[0].first);
								int allocReg = getFromPool(imTable.exprs[i].expr[1], nowLevel, willBeUse, tempNum, 1, 1);
								if (tempRegTab[tempNum] == "")
									cout << "致命错误：变量未分配到临时寄存器" << endl;
								assignVars.push_back(make_pair(tempRegTab[tempNum], -1));
							}
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //普通变量，函数变量
							if (symTable.syms[find].reg != "")
								assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
							else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
								vector<string> willBeUse;
								willBeUse.push_back(assignVars[0].first);
								int allocReg = getFromPool(imTable.exprs[i].expr[1], nowLevel, willBeUse, find, 0, 1);
								if (symTable.syms[find].reg == "")
									cout << "致命错误：变量未分配到临时寄存器" << endl;
								assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
							}
							//genOneCode("lw", "$t0", to_string(addr) + "($a1)", "");
						} else { //常量
							genOneCode("li", "$a0", imTable.exprs[i].expr[1], "");
							assignVars.push_back(make_pair("$a0", -2));
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
							if (tempRegTab[tempNum] != "")
								assignVars.push_back(make_pair(tempRegTab[tempNum], -1));
							else {
								vector<string> willBeUse;
								willBeUse.push_back(assignVars[0].first);
								willBeUse.push_back(assignVars[1].first);
								int allocReg = getFromPool(imTable.exprs[i].expr[3], nowLevel, willBeUse, tempNum, 1, 1);
								if (tempRegTab[tempNum] == "")
									cout << "致命错误：变量未分配到临时寄存器" << endl;
								assignVars.push_back(make_pair(tempRegTab[tempNum], -1));
							}
							//genOneCode("lw", "$t1", to_string(tempNum * 4) + "($a3)", "");
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[3], nowLevel);
						if (find != -1) { //普通变量，函数变量
							if (symTable.syms[find].reg != "")
								assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
							else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
								vector<string> willBeUse;
								willBeUse.push_back(assignVars[0].first);
								willBeUse.push_back(assignVars[1].first);
								int allocReg = getFromPool(imTable.exprs[i].expr[3], nowLevel, willBeUse, find, 0, 1);
								if (symTable.syms[find].reg == "")
									cout << "致命错误：变量未分配到临时寄存器" << endl;
								assignVars.push_back(make_pair(symTable.syms[find].reg, -1));
							}
							//genOneCode("lw", "$t1", to_string(addr) + "($a1)", "");
						} else { //常量
							genOneCode("li", "$v1", imTable.exprs[i].expr[3], "");
							assignVars.push_back(make_pair("$v1", -2));
						}
					}				
				}
				//做计算
				if (assignVars.size() == 3) {
					if (imTable.exprs[i].expr[2] == "+")
						genOneCode("addu", assignVars[0].first, assignVars[1].first, assignVars[2].first);
					if (imTable.exprs[i].expr[2] == "-")
						genOneCode("subu", assignVars[0].first, assignVars[1].first, assignVars[2].first);
					if (imTable.exprs[i].expr[2] == "*") {
						genOneCode("mult", assignVars[1].first, assignVars[2].first, "");
						genOneCode("mflo", assignVars[0].first, "", "");
					}
					if (imTable.exprs[i].expr[2] == "/") {
						genOneCode("div", assignVars[1].first, assignVars[2].first, "");
						genOneCode("mflo", assignVars[0].first, "", "");
					}
				} else {
					genOneCode("move", assignVars[0].first, assignVars[1].first, "");
				}
				break;
			case FUNC:
				if(nowLevel != 0) //为上一个函数生成返回跳转语句
					genOneCode("jr", "$ra", "", "");
				label = imTable.exprs[i].expr[0] + imTable.exprs[i].expr[1] + ":";
				genOneCode(label, "", "", "");
				//从内存中取分配了全局寄存器的参数
				for (int x = 0; x < symTable.funcInd.size(); ++x) {
					if (symTable.syms[symTable.funcInd[x]].name == imTable.exprs[i].expr[1]) {
						for (int pl = symTable.funcInd[x] + 1; pl < symTable.top; ++pl) {
							if (symTable.syms[pl].object != 2)
								break;
							if (symTable.syms[pl].reg != "" && symTable.syms[pl].reg[1] == 's') 
								genOneCode("lw", symTable.syms[pl].reg, to_string(symTable.syms[pl].addr) + "($a2)", "");
						}
						break;
					}
				}
				memOffset = 0; //每个函数偏移量都重新计算
				++nowLevel;
				break;
			case PARAM:
				find = searchWithLevel(imTable.exprs[i].expr[2], 2, nowLevel);
				if (find == -1)
					error(44, "");
				else {
					symTable.syms[find].addr = memOffset;
					//cout << imTable.exprs[i].expr[2] << "的地址为" << memOffset << endl;
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
					//cout << imTable.exprs[i].expr[2] << "的地址为" << memOffset << endl;
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
							for (int strInd = 0; strInd < (asciiz->r2).size(); ++strInd) {
								if (asciiz->r2[strInd] == '\\') {
									asciiz->r2.insert(strInd, "\\");
									++strInd;
								}
							}
							mipsTable.insert(mipsTable.begin() + 4, asciiz);
							//输出字符串
							genOneCode("li", "$v0", "4", "");
							genOneCode("la", "$a0", printStr, "");
							genOneCode("syscall", "", "", "");
						} else { //printf '('＜表达式＞')'
							//查push中的参数，存入$a0
							int isChar = findAndSet("$a0", 1, ind);
							if (imTable.exprs[i].expr[3] == "0")
								isChar = 0;
							else
								isChar = 1;
							if (isChar == 1) { //按char输出
								genOneCode("li", "$v0", "11", "");
								genOneCode("syscall", "", "", "");
							} else { //按整型输出
								genOneCode("li", "$v0", "1", "");
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
						for (int strInd = 0; strInd < (asciiz->r2).size(); ++strInd) {
							if (asciiz->r2[strInd] == '\\') {
								asciiz->r2.insert(strInd, "\\");
								++strInd;
							}
						}
						mipsTable.insert(mipsTable.begin() + 4, asciiz);
						genOneCode("li", "$v0", "4", "");
						genOneCode("la", "$a0", printStr, "");
						genOneCode("syscall", "", "", "");
						//再输出表达式
						int isChar = findAndSet("$a0", 1, indExpr);
						if (imTable.exprs[i].expr[3] == "0")
							isChar = 0;
						else
							isChar = 1;
						if (isChar == 1) { //按char输出
							genOneCode("li", "$v0", "11", "");
							genOneCode("syscall", "", "", "");
						} else { //按整型输出
							genOneCode("li", "$v0", "1", "");
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
								if(symTable.syms[find].reg != "")
									genOneCode("move", symTable.syms[find].reg, "$v0", "");
								else {
									if(symTable.syms[find].spaceLv == 0)
										genOneCode("sw", "$v0", to_string(addr) + "($a1)", "");
									else
										genOneCode("sw", "$v0", to_string(addr) + "($a2)", "");
								}						
							} else { //读字符
								genOneCode("li", "$v0", "12", "");
								genOneCode("syscall", "", "", "");
								if (symTable.syms[find].reg != "")
									genOneCode("move", symTable.syms[find].reg, "$v0", "");
								else {
									if (symTable.syms[find].spaceLv == 0)
										genOneCode("sw", "$v0", to_string(addr) + "($a1)", "");
									else
										genOneCode("sw", "$v0", to_string(addr) + "($a2)", "");
								}
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
						//算a2(即函数栈)的偏移
						string parentFuncName = "";
						int stackOffset = -1;
						int funcImInd;
						for (funcImInd = i; funcImInd >= 0; --funcImInd) {
							if (imTable.exprs[funcImInd].type == FUNC) {
								parentFuncName = imTable.exprs[funcImInd].expr[1];
								break;
							}
						}
						for (int temp = 0; temp < symTable.funcInd.size(); ++temp) {
							if (symTable.syms[symTable.funcInd[temp]].name == parentFuncName) {
								int temp2;
								for (temp2 = symTable.funcInd[temp] + 1; temp2 < symTable.top; ++temp2) {
									if (symTable.syms[temp2].object == 4)
										break;
								}
								stackOffset = symTable.syms[temp2 - 1].addr + 4 + 4;//要存ra
								break;
							}
						}
						//算临时变量栈的偏移
						int tempOffset = -1;
						for (int funcInd = funcImInd + 1; funcInd < imTable.ind; ++funcInd) {
							if (imTable.exprs[funcInd].type == FUNC)
								break;
							for (int temp = 0; temp < 4; ++temp) {
								if (imTable.exprs[funcInd].expr[temp].size() > 5 &&
									imTable.exprs[funcInd].expr[temp].substr(0, 5) == "_TEMP") {
									string str = imTable.exprs[funcInd].expr[temp];
									str.erase(str.find("_TEMP"), 5);
									istringstream is(str);
									int tempNum;
									is >> tempNum;
									if (tempNum >= tempOffset)
										tempOffset = tempNum;
								}
							}
						}
						tempOffset = tempOffset * 4 + 4;
						//将所有s0-s7以及t0-t9寄存器，以及$ra回写入内存
							//回写t0-t9
						//vector<pair<string, pair<string, int> > > copyRegPool(regPool);
						//vector<string> copyTempRegTab(tempRegTab);
						writeBack(); 
							//回写s0-s7(即回写除数组外的所有局部变量)
						int callFuncInd = symTable.funcInd[nowLevel - 1];
						++callFuncInd;//指向该函数的第一个局部变量
						while (callFuncInd < symTable.top && symTable.syms[callFuncInd].spaceLv == nowLevel) {
							if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
								if (symTable.syms[callFuncInd].reg != "") {
									if(symTable.syms[callFuncInd].reg[1] == 's')
										genOneCode("sw", symTable.syms[callFuncInd].reg, to_string(symTable.syms[callFuncInd].addr) + "($a2)", "");
								}
							}
							++callFuncInd;
						}
							//回写$ra寄存器
						genOneCode("sw", "$ra", to_string(stackOffset - 4) + "($a2)", "");		
						//push参数
						int pushInd = paramStack.size() - paramNum;
						for (int paraInd = funcInd + 1; symTable.syms[paraInd].object == 2; ++paraInd) {
							findAndSet("$a0", 1, paramStack[pushInd]);
							int addr = symTable.syms[paraInd].addr;
							genOneCode("sw", "$a0", to_string(addr + stackOffset) + "($a2)", "");
							++pushInd;
						}
						for (int temp = 0; temp < paramNum; ++temp)
							paramStack.pop_back();
						//栈指针移动
						genOneCode("addi", "$a2", "$a2", to_string(stackOffset));
						genOneCode("addi", "$a3", "$a3", to_string(tempOffset));
						//再jal调用函数
						genOneCode("jal", tempArr[symTable.syms[funcInd].type] + funcName, "", "");
						//最后恢复现场，反向恢复所有数据
						genOneCode("addi", "$a3", "$a3", to_string(-tempOffset));
						genOneCode("addi", "$a2", "$a2", to_string(-stackOffset));
							//恢复$ra寄存器
						genOneCode("lw", "$ra", to_string(stackOffset - 4) + "($a2)", "");
							//恢复s0-s7
						callFuncInd = symTable.funcInd[nowLevel - 1];
						++callFuncInd;//指向该函数的第一个局部变量
						vector<string> saved;
						while (callFuncInd < symTable.top && symTable.syms[callFuncInd].spaceLv == nowLevel) {
							if (symTable.syms[callFuncInd].object == 1 || symTable.syms[callFuncInd].object == 2) {
								if (symTable.syms[callFuncInd].reg != "") {
									if (symTable.syms[callFuncInd].reg[1] == 's' && (std::find(saved.begin(), saved.end(), symTable.syms[callFuncInd].reg) == saved.end())) {
										genOneCode("lw", symTable.syms[callFuncInd].reg, to_string(symTable.syms[callFuncInd].addr) + "($a2)", "");
										saved.push_back(symTable.syms[callFuncInd].reg);
									}			
								}
							}
							++callFuncInd;
						}
							//恢复t0-t9
						//regPool = copyRegPool;
						//tempRegTab = copyTempRegTab;
						//recover();
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
							if(tempRegTab[tempNum] != "")
								genOneCode("move", "$v0", tempRegTab[tempNum], "");
							else
								genOneCode("lw", "$v0", to_string(tempNum * 4) + "($a3)", "");
							isTEMP = 1;
						}
					}
					if (isTEMP == 0) { //普通变量，函数变量，常量
						int find = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
						if (find != -1) { //普通变量，函数变量
							int addr = symTable.syms[find].addr;
							if(symTable.syms[find].reg != "")
								genOneCode("move", "$v0", symTable.syms[find].reg, "");
							else {
								if(symTable.syms[find].spaceLv == 0)
									genOneCode("lw", "$v0", to_string(addr) + "($a1)", "");
								else
									genOneCode("lw", "$v0", to_string(addr) + "($a2)", "");
							}						
						} else { //常量
							genOneCode("li", "$v0", imTable.exprs[i].expr[1], "");
						}
					}
				}
				if (nowLevel != symTable.funcInd.size())
					genOneCode("jr", "$ra", "", "");
				else
					genOneCode("j", "endofprog", "", "");
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
						if (tempRegTab[tempNum] != "")
							compVars.push_back(tempRegTab[tempNum]);
						else {
							vector<string> willBeUse;
							int allocReg = getFromPool(imTable.exprs[i].expr[0], nowLevel, willBeUse, tempNum, 1, 1);
							if (tempRegTab[tempNum] == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							compVars.push_back(tempRegTab[tempNum]);
						}
						//genOneCode("lw", "$t0", to_string(tempNum * 4) + "($a3)", "");
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) { //普通变量，函数变量，常量
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find != -1) { //普通变量，函数变量
						if (symTable.syms[find].reg != "")
							compVars.push_back(symTable.syms[find].reg);
						else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
							vector<string> willBeUse;
							int allocReg = getFromPool(imTable.exprs[i].expr[0], nowLevel, willBeUse, find, 0, 1);
							if (symTable.syms[find].reg == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							compVars.push_back(symTable.syms[find].reg);
						}
					} else { //常量
						genOneCode("li", "$a0", imTable.exprs[i].expr[0], "");
						compVars.push_back("$a0");
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
						if (tempRegTab[tempNum] != "")
							compVars.push_back(tempRegTab[tempNum]);
						else {
							vector<string> willBeUse;
							willBeUse.push_back(compVars[0]);
							int allocReg = getFromPool(imTable.exprs[i].expr[2], nowLevel, willBeUse, tempNum, 1, 1);
							if (tempRegTab[tempNum] == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							compVars.push_back(tempRegTab[tempNum]);
						}
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) { //普通变量，函数变量，常量
					int find = searchAllLevel(imTable.exprs[i].expr[2], nowLevel);
					if (find != -1) { //普通变量，函数变量
						if (symTable.syms[find].reg != "")
							compVars.push_back(symTable.syms[find].reg);
						else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
							vector<string> willBeUse;
							willBeUse.push_back(compVars[0]);
							int allocReg = getFromPool(imTable.exprs[i].expr[2], nowLevel, willBeUse, find, 0, 1);
							if (symTable.syms[find].reg == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							compVars.push_back(symTable.syms[find].reg);
						}
					} else { //常量
						genOneCode("li", "$v1", imTable.exprs[i].expr[2], "");
						compVars.push_back("$v1");
					}
				}
				//根据操作符进行比较
				jump = condStack[condStack.size() - 1];
				condStack.pop_back();
				if (imTable.exprs[i].expr[1] == "==") {
					if (jump.type == BZ)
						genOneCode("bne", compVars[0], compVars[1], jump.expr[1]);
					else
						genOneCode("beq", compVars[0], compVars[1], jump.expr[1]);
				}
				if (imTable.exprs[i].expr[1] == "!=") {
					if (jump.type == BZ)
						genOneCode("beq", compVars[0], compVars[1], jump.expr[1]);
					else
						genOneCode("bne", compVars[0], compVars[1], jump.expr[1]);
				}
				if (imTable.exprs[i].expr[1] == "<") {
					genOneCode("subu", "$v1", compVars[0], compVars[1]);
					if (jump.type == BZ)
						genOneCode("bgez", "$v1", jump.expr[1], "");
					else
						genOneCode("bltz", "$v1", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == "<=") {
					genOneCode("subu", "$v1", compVars[0], compVars[1]);
					if (jump.type == BZ)
						genOneCode("bgtz", "$v1", jump.expr[1], "");
					else
						genOneCode("blez", "$v1", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == ">") {
					genOneCode("subu", "$v1", compVars[0], compVars[1]);
					if (jump.type == BZ)
						genOneCode("blez", "$v1", jump.expr[1], "");
					else
						genOneCode("bgtz", "$v1", jump.expr[1], "");
				}
				if (imTable.exprs[i].expr[1] == ">=") {
					genOneCode("subu", "$v1", compVars[0], compVars[1]);
					if (jump.type == BZ)
						genOneCode("bltz", "$v1", jump.expr[1], "");
					else
						genOneCode("bgez", "$v1", jump.expr[1], "");
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
					//查变量b，将其值赋给$a0
					findAndSet("$a0", 2, i);
					//查变量c，将其值赋给$v1
					findAndSet("$v1", 3, i);
					int find = searchAllLevel(imTable.exprs[i].expr[0], nowLevel);
					if (find == -1)
						error(30, "");
					else
						if (symTable.syms[find].object != 3)
							error(46, "");
					int addr = symTable.syms[find].addr;
					genOneCode("sll", "$a0", "$a0", "2"); //a0 = a0 * 4
					genOneCode("addiu", "$a0", "$a0", to_string(addr));
					if(symTable.syms[find].spaceLv == 0)
						genOneCode("addu", "$a0", "$a0", "$a1");
					else
						genOneCode("addu", "$a0", "$a0", "$a2");
					genOneCode("sw", "$v1", "0($a0)", "");
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
					//查变量c，赋值给$a0
					findAndSet("$a0", 3, i);
					int find2 = searchAllLevel(imTable.exprs[i].expr[1], nowLevel);
					if (find2 == -1)
						error(30, "");
					else
						if (symTable.syms[find2].object != 3)
							error(46, "");
					int addr = symTable.syms[find2].addr;
					genOneCode("sll", "$a0", "$a0", "2");
					genOneCode("addiu", "$a0", "$a0", to_string(addr));
					if(symTable.syms[find2].spaceLv == 0)
						genOneCode("addu", "$a0", "$a0", "$a1");
					else
						genOneCode("addu", "$a0", "$a0", "$a2");
					//将b[c]的值存入变量a对应的寄存器
					if (type == 1) {
						if (symTable.syms[find].reg != "")
							genOneCode("lw", symTable.syms[find].reg, "0($a0)", "");
						else { //跨越基本块不活跃的局部变量以及全局变量，则用寄存器池
							vector<string> willBeUse;
							int allocReg = getFromPool(symTable.syms[find].name, nowLevel, willBeUse, find, 0, 0);
							if (symTable.syms[find].reg == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							genOneCode("lw", symTable.syms[find].reg, "0($a0)", "");
						}
					} else { //_TEMP变量
						if (tempRegTab[find] != "")
							genOneCode("lw", tempRegTab[find], "0($a0)", "");
						else {
							vector<string> willBeUse;
							int allocReg = getFromPool(imTable.exprs[i].expr[0], nowLevel, willBeUse, find, 1, 0);
							if (tempRegTab[find] == "")
								cout << "致命错误：变量未分配到临时寄存器" << endl;
							genOneCode("lw", tempRegTab[find], "0($a0)", "");
						}
					}
				}
				break;
			case LABEL:
				genOneCode(imTable.exprs[i].expr[0] + ":", "", "", "");
				break;
		}
		for (int bl = 0; bl < blockGraph.size(); ++bl) {
			if (i == blockGraph[bl]->codes.back()) //基本块结尾
				writeBack();
		}
	}
	//程序末尾
	genOneCode("endofprog:", "", "", "");
}