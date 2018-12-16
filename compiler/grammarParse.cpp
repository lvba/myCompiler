#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static bool varToFuncFlag = false; //当处理变量声明的子函数误读了函数的声明头部时
static bool isReachMain = false;
static int misReadType;
static string misReadIdent;
static bool hasReturn = false;
//函数声明
void compoundState(int funcType);
void voidFunc();
void varDeclare();
void returnFunc();
void constDeclare();
pair<string, int> expression(int depth);

bool test(symbol sym, int errCode)
{
	getWord();
	if (nowWord.sym != sym) {
		error(errCode, "");
		return false;
	}
	return true;
}

pair<string, int> factor(int depth)//返回两个参数，分别为临时变量名和因子（项，表达式）类型
{
	string retName = "";//中间式的表示符
	string idName, integer, ch;
	switch (nowWord.sym) {
		case IDENT:
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == LBRACK) { //数组取下标的情况
				//检查是否已定义
				int find = searchSymTable(idName, nowLevel);
				if (find == -1) {
					error(29, "");
					find = 0;
				} else
					if (symTable.syms[find].object != 3)
						error(30, "");
				getWord();
				pair<string, int> exprP = expression(depth + 1);
				string exprName = exprP.first;
				int exprType = exprP.second;
				if (exprType != 0)
					error(37, "");
				if (nowWord.sym != RBRACK) {
					error(10, "factor");
					return make_pair("", -1);
				} else {
					string retName = genTemp();
					genInterMedia(ARRASS, retName, idName, "[]", exprName);
					getWord();
					return make_pair(retName, symTable.syms[find].type);
				}		
			} else {
				if (nowWord.sym == LPARENT) { //有返回值的函数调用
					//检查变量名是否合法
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (!(symTable.syms[find].object == 4 && (symTable.syms[find].type != 2)))
							error(31, "");
					//检查该作用域中是否有同名的变量
					int symInd = symTable.top - 1;
					if(symInd >= 0)
						while (symTable.syms[symInd].spaceLv == nowLevel) {
							if (symTable.syms[symInd].name == idName && symTable.syms[symInd].object != 4) {
								error(42, "");
								break;
							}			
							--symInd;
						}
					getWord();
					//函数调用传参是否匹配
					int paramStart = find + 1, paramEnd = find + symTable.syms[find].size;
					int paramInd = paramStart;
					if (nowWord.sym != RPARENT) {
						pair<string, int> p = expression(depth + 1);
						string param = p.first;
						if (symTable.syms[paramInd].type != p.second)
							error(33, "");
						++paramInd;
						genInterMedia(PUSH, "push", param, "", "");
						while (nowWord.sym == COMMA) {
							getWord();
							pair<string, int> p = expression(depth + 1);
							string param = p.first;
							if (symTable.syms[paramInd].type != p.second)
								error(33, "");
							++paramInd;
							genInterMedia(PUSH, "push", param, "", "");
						}
						if (paramInd - 1 != paramEnd)
							error(34, "");
					}
					if (nowWord.sym != RPARENT) {
						error(12, "factor");
						return make_pair("", -1);
					} else {
						genInterMedia(CALL, "call", idName, to_string(staticTempNum), "");
						string retName = genTemp();
						genInterMedia(VARASS, retName, "_RET", "", "");
						getWord();
						return make_pair(retName, symTable.syms[find].type);
					}	
				} else { //为最普通的标识符（即普通变量或常量）
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else {
						if (symTable.syms[find].object != 0 &&
							symTable.syms[find].object != 1 &&
							symTable.syms[find].object != 2) {
							error(32, "");
						}
					}						
					if (symTable.syms[find].object == 0) {//常量替换
						if(symTable.syms[find].type == 0)
							return make_pair(to_string(symTable.syms[find].addr), 0);
						else {
							char tempc = symTable.syms[find].addr;
							string temps = "'";
							temps += tempc;
							temps += "'";
							return make_pair(temps, 1);
						}
					} else
						return make_pair(idName, symTable.syms[find].type);
				}		
			}
			break;
		case PLUS:
			integer = "";
			integer += '+';
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return make_pair("", -1);
			}
			integer += to_string(nowWord.num);
			getWord();
			return make_pair(integer, 0);
			break;
		case MINUS:
			integer = "";
			integer += '-';
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return make_pair("", -1);
			}	
			integer += to_string(nowWord.num);
			getWord();
			return make_pair(integer, 0);
			break;
		case CONUNSIGN:
			integer = to_string(nowWord.num);
			getWord();
			return make_pair(integer, 0);
			break;
		case CONCHAR:
			ch = nowWord.str;
			getWord();
			return make_pair(ch, 1);
			break;
		case LPARENT:
			getWord();
			pair<string, int> p = expression(depth + 1);
			retName = p.first;
			if (nowWord.sym != RPARENT) {
				error(12, "factor");
				return make_pair("", -1);
			} else {
				getWord();
				return make_pair(retName, 0);//加了括号可以强制转换为整型
			}	
			break;
	}
}

pair<string, int> term(int depth)
{
	pair<string, int> retP = factor(depth);
	string ret = retP.first;
	int retType = retP.second;
	string retName = ret;
	while (nowWord.sym == TIMES || nowWord.sym == DIV) {
		retType = 0;
		string op = nowWord.sym == TIMES ? "*" : "/";
		getWord();
		retName = genTemp();
		genInterMedia(VARASS, retName, ret, op, factor(depth).first);
		ret = retName;
	}
	return make_pair(retName, retType);
}

pair<string, int> expression(int depth)
{
	symbol op = VOIDSY;
	if (nowWord.sym == MINUS || nowWord.sym == PLUS) { //表达式前面有符号
		op = nowWord.sym;
		getWord();
	}
	pair<string, int> p = term(depth);
	string ret = p.first;
	int retType = p.second;
	string retName = ret;
	if (op == MINUS) {
		retType = 0;
		retName = genTemp();
		genInterMedia(VARASS, retName, "-1", "*", ret);
	}
	ret = retName;
	while (nowWord.sym == MINUS || nowWord.sym == PLUS) {
		retType = 0;
		string op = nowWord.sym == MINUS ? "-" : "+";
		getWord();
		retName = genTemp();
		genInterMedia(VARASS, retName, ret, op, term(depth).first);
		ret = retName;
	}
	if(depth == 0)
		resetTemp();
	//cout << "这是一个表达式" << endl;
	return make_pair(retName, retType);
}

void condition()
{
	bool isSingle = true;
	pair<string, int> ret1P = expression(1);
	string ret1 = ret1P.first;
	int ret1Type = ret1P.second;
	if (nowWord.sym == LESS || nowWord.sym == LESSEQL || nowWord.sym == GREAT ||
		nowWord.sym == GREATEQL || nowWord.sym == EQUAL || nowWord.sym == NOTEQL) {
		isSingle = false;
		symbol op = nowWord.sym;
		string opArr[6] = { "<", "<=", ">", ">=", "!=", "==" };
		getWord();
		pair<string, int> ret2P = expression(0);
		string ret2 = ret2P.first;
		int ret2Type = ret2P.second;
		if (!(ret1Type == 0 && ret2Type == 0))
			error(36, "");
		genInterMedia(COMPARE, ret1, opArr[op - 4], ret2, "");
		//cout << "这是一个条件" << endl;
		return;
	}
	if (isSingle)
		resetTemp();
	if (ret1Type != 0)
		error(36, "");
	genInterMedia(COMPARE, ret1, "!=", "0", "");
	//cout << "这是一个条件" << endl;
	return;
}

void statement(int funcType)
{
	string ifLabel1, ifLabel2;
	string doLabel;
	string idName, idName2, idName3, exprRet, forLabel1, forLabel2, step;
	symbol op;
	string arrDimen;
	pair<string, int> exprP;
	int exprType, find, scanfParam;
	switch (nowWord.sym) {
		case IFSY: //if...else语句
			ifLabel1 = genLabel();
			genInterMedia(BZ, "BZ", ifLabel1, "", "");
			if(test(LPARENT, 11))
				getWord();
			condition();
			if (nowWord.sym != RPARENT) {
				error(12, "if");
				return;
			} else
				getWord();
			statement(funcType);
			if (nowWord.sym == ELSESY) { //有else分句
				ifLabel2 = genLabel();
				genInterMedia(GOTO, "GOTO", ifLabel2, "", "");
				genInterMedia(LABEL, ifLabel1, ":", "", "");
				getWord();
				statement(funcType);
				genInterMedia(LABEL, ifLabel2, ":", "", "");
			} else {
				genInterMedia(LABEL, ifLabel1, ":", "", "");
			}
			//cout << "这是一个if语句" << endl;
			break;
		case DOSY: //do...while语句
			doLabel = genLabel();
			genInterMedia(LABEL, doLabel, ":", "", "");
			getWord();
			statement(funcType);
			if (nowWord.sym != WHILESY) {
				error(17, "while");
				return;
			}		
			if(test(LPARENT, 11))
				getWord();
			genInterMedia(BNZ, "BNZ", doLabel, "", "");
			condition();
			if (nowWord.sym != RPARENT) {
				error(12, "while");
				return;
			} else 
				getWord();
			//cout << "这是一个do-while语句" << endl;
			break;
		case FORSY: //for语句
			forLabel1 = genLabel();
			forLabel2 = genLabel();
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}
			idName = nowWord.str;
			find = searchSymTable(idName, nowLevel);
			if (find == -1) {
				error(29, "");
				find = 0;
			} else
				if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
					error(32, "");
			if(test(ASSIGN, 2))
				getWord();
			exprP = expression(0);
			exprRet = exprP.first;
			exprType = exprP.second;
			if (symTable.syms[find].type != exprType)
				error(38, "");
			genInterMedia(VARASS, idName, exprRet, "", "");
			genInterMedia(LABEL, forLabel1, ":", "", "");
			genInterMedia(BZ, "BZ", forLabel2, "", "");
			if (nowWord.sym != SEMICOLON) {
				error(5, "for");
				return;
			} else
				getWord();
			condition();
			if (nowWord.sym != SEMICOLON) {
				error(5, "for");
				return;
			} else
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			} 
			idName2 = nowWord.str;
			find = searchSymTable(idName2, nowLevel);
			if (find == -1) {
				error(29, "");
				find = 0;
			} else
				if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
					error(32, "");
			if (symTable.syms[find].type != 0)
				error(37, "");
			if(test(ASSIGN, 2))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			} 
			idName3 = nowWord.str;
			find = searchSymTable(idName3, nowLevel);
			if (find == -1) {
				error(29, "");
				find = 0;
			} else
				if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
					error(32, "");
			getWord();
			if (nowWord.sym != PLUS && nowWord.sym != MINUS) {
				error(18, "for");
				return;
			} else {
				op = nowWord.sym;
				getWord();
			}			
			if (nowWord.sym != CONUNSIGN) {
				error(16, "for");
				return;
			}
			step = to_string(nowWord.num);
			if(test(RPARENT, 12))
				getWord();
			statement(funcType);
			genInterMedia(VARASS, idName2, idName3, op == PLUS ? "+" : "-", step);
			genInterMedia(GOTO, "GOTO", forLabel1, "", "");
			genInterMedia(LABEL, forLabel2, ":", "", "");
			//cout << "这是一个for语句" << endl;
			break;
		case LBRACE: //语句块
			getWord();
			while (nowWord.sym != RBRACE)
				statement(funcType); //处理<语句>
			getWord();
			//cout << "这是一个语句块" << endl;
			break;
		case IDENT: //有返回值函数调用语句，无返回值函数调用语句，赋值语句
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == ASSIGN || nowWord.sym == LBRACK) { //赋值语句
				if (nowWord.sym == ASSIGN) {
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
							error(32, "");
					getWord();
					pair<string, int> exprP = expression(0);
					exprRet = exprP.first;
					int exprType = exprP.second;
					if (symTable.syms[find].type != exprType)
						error(38, "");
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else {
						genInterMedia(VARASS, idName, exprRet, "", "");
						getWord();
					}	
					//cout << "这是普通变量的赋值语句" << endl;
				} else {
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (symTable.syms[find].object != 3)
							error(30, "");
					getWord();
					pair<string, int> arrDimenP = expression(1);
					arrDimen = arrDimenP.first;
					int arrType = arrDimenP.second;
					if (arrType != 0)
						error(37, "");
					if (nowWord.sym != RBRACK) {
						error(10, "for");
						return;
					}	
					if(test(ASSIGN, 2))
						getWord();
					pair<string, int> exprP = expression(0);
					exprRet = exprP.first;
					int exprType = exprP.second;
					if (symTable.syms[find].type != exprType)
						error(38, "");
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else {
						genInterMedia(ARRASS, idName, "[]", arrDimen, exprRet);
						getWord();
					}						
					//cout << "这是数组下标的赋值语句" << endl;
				}
			} else { 
				if (nowWord.sym != LPARENT) {
					error(19, "");
					return;
				} else { //有返回值和无返回值的函数
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (symTable.syms[find].object != 4)
							error(45, "");
					//检查该作用域中是否有同名的变量
					int symInd = symTable.top - 1;
					if (symInd >= 0)
						while (symTable.syms[symInd].spaceLv == nowLevel) {
							if (symTable.syms[symInd].name == idName && symTable.syms[symInd].object != 4) {
								error(42, "");
								break;
							}
							--symInd;
						}
					getWord();
					//函数调用传参是否匹配
					int paramStart = find + 1, paramEnd = find + symTable.syms[find].size;
					int paramInd = paramStart;
					if (nowWord.sym != RPARENT) {
						pair<string, int> p = expression(1);
						exprRet = p.first;
						if (symTable.syms[paramInd].type != p.second)
							error(33, "");
						++paramInd;
						genInterMedia(PUSH, "push", exprRet, "", "");
						while (nowWord.sym == COMMA) {
							getWord();
							pair<string, int> p = expression(1);
							exprRet = p.first;
							if (symTable.syms[paramInd].type != p.second)
								error(33, "");
							++paramInd;
							genInterMedia(PUSH, "push", exprRet, "", "");
						}
						if (paramInd - 1 != paramEnd)
							error(34, "");
					}
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}
					genInterMedia(CALL, "call", idName, to_string(staticTempNum), "");
					resetTemp();
					//cout << "这是有返回值或者无返回值的函数调用" << endl;
				}
			}
			break;
		case SCANFSY: //读语句
			scanfParam = 0;
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}
			find = searchSymTable(nowWord.str, nowLevel);
			if (find == -1) {
				error(29, "");
				find = 0;
			} else
				if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
					error(32, "");
			genInterMedia(PUSH, "push", nowWord.str, "", "");
			++scanfParam;
			getWord();
			while (nowWord.sym == COMMA) {
				getWord();
				int find = searchSymTable(nowWord.str, nowLevel);
				if (find == -1) {
					error(29, "");
					find = 0;
				} else
					if (symTable.syms[find].object != 1 && symTable.syms[find].object != 2)
						error(32, "");
				genInterMedia(PUSH, "push", nowWord.str, "", "");
				++scanfParam;
				getWord();
			}
			if (nowWord.sym != RPARENT) {
				error(12, "while");
				return;
			} else {
				if(test(SEMICOLON, 5))
					getWord();
			}
			genInterMedia(CALL, "call", "scanf", to_string(scanfParam), "");
			//cout << "这是读语句" << endl;
			break;
		case PRINTFSY: //写语句
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym == CONSTR) {
				genInterMedia(PUSH, "push", "\"" + nowWord.str + "\"", "", "");
				getWord();
				if (nowWord.sym == COMMA) {//printf'('＜字符串＞,＜表达式＞')'
					getWord();
					pair<string, int> p = expression(0);
					exprRet = p.first;
					genInterMedia(PUSH, "push", exprRet, "", "");
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else
						test(SEMICOLON, 5);
					getWord();
					genInterMedia(CALL, "call", "printf", "2", to_string(p.second));
				} else {//printf '('＜字符串＞')'
					if (nowWord.sym == RPARENT) {
						if(test(SEMICOLON, 5))
							getWord();
					} else {
						error(20, "");
						return;
					}	
					genInterMedia(CALL, "call", "printf", "1", "");
				}				
			} else { //只输出表达式
				pair<string, int> p = expression(0);
				exprRet = p.first;
				genInterMedia(PUSH, "push", exprRet, "", "");
				if (nowWord.sym != RPARENT) {
					error(12, "while");
					return;
				} else {
					if(test(SEMICOLON, 5))
						getWord();
				}
				genInterMedia(CALL, "call", "printf", "1", to_string(p.second));
			}
			//cout << "这是写语句" << endl;
			break;
		case SEMICOLON: //空语句
			getWord();
			//cout << "这是空语句" << endl;
			break;
		case RETURNSY: //返回语句
			getWord();
			if (nowWord.sym == SEMICOLON) {
				if (funcType == 0 || funcType == 1)
					error(39, "");
				genInterMedia(RET, "ret", "", "", "");
				getWord();
			} else {
				if (nowWord.sym == LPARENT) {
					getWord();
					exprRet = expression(0).first;
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}
					hasReturn = true;
					if (funcType == 2)
						error(40, "");
					genInterMedia(RET, "ret", exprRet, "", "");
				} else {
					error(21, "");
					return;
				}		
			}
			//cout << "这是返回语句" << endl;
			break;
	}
}

void compoundState(int funcType)
{
	if(nowWord.sym == CONSTSY)
		constDeclare(); //处理常量说明
	varDeclare(); //处理变量声明(可能不存在)
	//处理＜语句列＞，已经预读了一个符号了
	while (nowWord.sym != RBRACE) 
		statement(funcType); //处理<语句>
	//cout << "这是一个复合语句" << endl;
}

//整个程序的语法处理函数
void program()
{
	if (nowWord.sym == CONSTSY) //程序以常量声明开始
		constDeclare(); //处理常量说明
	varDeclare(); //处理变量声明
	//不管有没有变量声明，当变量说明处理完之后紧跟有返回值的函数定义或者main函数时，
	//会多读函数头部，如果紧跟void函数则不影响，正常进行
	if (varToFuncFlag)  //发生误读，说明变量声明后紧接有返回值的函数
		returnFunc();
	else
		voidFunc();
	while (!isReachMain) {
		if (nowWord.sym == VOIDSY)
			voidFunc();
		else {
			if (nowWord.sym == INTSY || nowWord.sym == CHARSY)
				returnFunc();
			else
				error(0, "program");
		}
	}
	//此时当前标识符指向main

	test(LPARENT, 11);
	test(RPARENT, 12);
	test(LBRACE, 13);
	nowLevel = (++addLevel);
	//将main函数登录入符号表
	insertSymTable("main", 4, 2, 0, 0, 0);
	genInterMedia(FUNC, "void", "main", "()", "");
	getWord();//预读
	compoundState(2);
	if (nowWord.sym != RBRACE)
		error(14, "program");
	//cout << "这是一个main函数" << endl;
}

void constDef()
{
	string name;
	if (nowWord.str == "int") { //int型常量定义
		do {
			getWord();
			if (nowWord.sym != IDENT) {
				error(1, "constDef");//应是标识符
				continue;
			} else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN) {
					error(2, "constDef");//应是=
					continue;
				} else {
					int constInt = getInt();
					//登录符号表
					if (constInt != -99999998) {
						insertSymTable(name, 0, 0, -1, nowLevel, constInt);
						genInterMedia(CONST, "const", "int", name, to_string(constInt));
					}	
					//cout << "这是一个int型常量的定义" << endl;
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);	
		return;
	}
	if (nowWord.str == "char") { //char型常量定义
		do {
			getWord();
			if (nowWord.sym != IDENT) {
				error(1, "constDef");//应是标识符
				continue;
			} else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN) {
					error(2, "constDef");//应是=
					continue;
				} else {
					getWord();
					if (nowWord.sym != CONCHAR) {
						error(3, "");
						continue;
					} else {
						char tempc = nowWord.str[1];
						//登录符号表
						insertSymTable(name, 0, 1, -1, nowLevel, tempc);
						string temps = "";
						temps += '\'';
						temps += tempc;
						temps += '\'';
						genInterMedia(CONST, "const", "char", name, temps);
						//cout << "这是一个char常量的定义" << endl;
					}		
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);
		return;
	}
	error(0, "constDef");//错误：应为类型标识符
}

void constDeclare()
{
	if (nowWord.sym != CONSTSY) {
		error(4, "");
		return;
	}	
	do {
		getWord();
		constDef();
		if (nowWord.sym != SEMICOLON)
			error(5, "for");
		else
			getWord();
	} while (nowWord.sym == CONSTSY);
	return;
}

bool varDef()
{
	int type = -1;
	string varName;
	if (nowWord.sym == INTSY)
		type = 0;
	else {
		if (nowWord.sym == CHARSY)
			type = 1;
		else
			return false;
	}
	do {
		getWord();
		if (nowWord.sym != IDENT && nowWord.sym != MAINSY) {
			error(1, "varDef");
			continue;
		} else {
			varName = nowWord.str;
			getWord();
			if (nowWord.sym == LPARENT) { //现在正在处理的语法成分不是变量定义而已经是函数定义了
				//而且相当于多读了函数的声明头部
				varToFuncFlag = true;
				misReadType = type;
				misReadIdent = varName;
				return false;
			}
			if (nowWord.sym == COMMA || nowWord.sym == SEMICOLON) {
				//登录符号表，变量的addr此时暂时初始化为0
				insertSymTable(varName, 1, type, -1, nowLevel, 0);
				genInterMedia(VAR, "var", type == 0 ? "int" : "char", varName, "");
				//cout << "这是一个普通变量定义" << endl;
			} else {
				if (nowWord.sym == LBRACK) { //数组变量
					int dimen = -1;
					getWord();
					if (nowWord.sym != CONUNSIGN) {
						error(8, "");
						continue;
					} else {
						if (nowWord.num == 0) {
							error(9, "");
							continue;
						} else {
							dimen = nowWord.num;
							if(test(RBRACK, 10))
								getWord();//预读下一个符号以便循环判断
							//登录符号表，地址暂时填0
							insertSymTable(varName, 3, type, dimen, nowLevel, 0);
							genInterMedia(ARRAY, "array", type == 0 ? "int" : "char", varName, to_string(dimen));
							//cout << "这是一个数组变量定义" << endl;
						}
					}
				}
			}
		}
	} while (nowWord.sym == COMMA);
	return true;
}

void varDeclare()
{
	bool isFinish;
	do {
		isFinish = varDef();
		if (isFinish) {
			if (nowWord.sym != SEMICOLON)
				error(5, "for");
			else
				getWord();
		}		
	} while (isFinish);
}

int paramList()
{
	int type;
	int paramNum = 0;
	if (nowWord.sym == RPARENT) //参数表为空
		return paramNum;
	do {
		if (nowWord.sym == COMMA)
			getWord();
		if (nowWord.sym == INTSY)
			type = 0;
		else {
			if (nowWord.sym == CHARSY)
				type = 1;
			else {
				error(0, "paramList");
				continue;
			}	
		}
		getWord();
		if (nowWord.sym != IDENT) {
			error(1, "paramList");
			continue;
		} else {
			//将该函数参数变量登录符号表，地址暂时填0
			insertSymTable(nowWord.str, 2, type, -1, nowLevel, 0);
			genInterMedia(PARAM, "para", type == 0 ? "int" : "char", nowWord.str, "");
			++paramNum;
		}
		getWord();
	} while (nowWord.sym == COMMA);
	//cout << "这是一个函数参数表" << endl;
	return paramNum;
}

void returnFunc()
{
	int funcType, flag = 0;
	string funcName;
	if (varToFuncFlag) { //处理变量说明时多读了声明头部
		funcType = misReadType;
		funcName = misReadIdent;
		varToFuncFlag = false; //后续再进入该子函数时肯定不会再发生误读的情况了
	} else { //正常进入该子函数时
		if (nowWord.sym == INTSY)
			funcType = 0;
		else
			funcType = 1;
		getWord();
		if (nowWord.sym != IDENT) {
			error(1, "");
			funcName = "nameless_ret_func";
		} else 
			funcName = nowWord.str;
		if (!test(LPARENT, 11))
			flag = 1;
	}
	//先生成函数头部的中间式
	genInterMedia(FUNC, funcType == 0 ? "int" : "char", funcName, "()", "");
	//处理形参表(此时nowWord指向左括号)
	if(flag == 0)
		getWord();//预读一个符号进入参数表处理函数
	//将函数信息登录进符号表
	insertSymTable(funcName, 4, funcType, -1, 0, 0);
	//进入块，改变level
	nowLevel = (++addLevel);
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//修改函数参数个数
	symTable.syms[symTable.funcInd[symTable.funcInd.size() - 1]].size = paramNum;
	hasReturn = false;
	getWord();//预读一个符号以进入复合语句处理子函数
	compoundState(funcType);
	if (!hasReturn)
		error(41, "");
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//预读
	nowLevel = 0; //回到全局作用域
	//cout << "这是一个有返回值的函数定义" << endl;
}

void voidFunc()
{
	if (nowWord.sym != VOIDSY) {
		error(15, "");
		return;
	}	
	string funcName;
	getWord();
	if (nowWord.sym == MAINSY) {//此为void main，不应由此子函数处理，跳出
		isReachMain = true;
		return;
	}
	if (nowWord.sym != IDENT) {
		error(1, "");
		funcName = "nameless_void_func";
	} else
		funcName = nowWord.str;
	genInterMedia(FUNC, "void", funcName, "()", "");
	if(test(LPARENT, 11))
		getWord();//预读一个符号进入参数表处理函数
	//将函数信息登录进符号表
	insertSymTable(funcName, 4, 2, -1, 0, 0);
	//进入块，改变level
	nowLevel = (++addLevel);
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//修改函数参数个数
	symTable.syms[symTable.funcInd[symTable.funcInd.size() - 1]].size = paramNum;
	getWord();//预读一个符号以进入复合语句处理子函数
	compoundState(2);
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//预读
	nowLevel = 0; //回到全局作用域
	//cout << "这是一个无返回值的函数定义" << endl;
}
