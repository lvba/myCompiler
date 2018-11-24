#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static string filePath;
static bool varToFuncFlag = false; //当处理变量声明的子函数误读了函数的声明头部时
static bool isReachMain = false;
static int misReadType;
static string misReadIdent;
//函数声明
void compoundState();
void voidFunc();
void varDeclare();
void returnFunc();
void constDeclare();
void expression();

bool test(symbol sym, int errCode)
{
	getWord();
	if (nowWord.sym != sym) {
		error(errCode, "");
		return false;
	}
	return true;
}

bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr)
{
	if (symTable.top >= maxSymNum) { //符号表溢出
		error(6, "");
		return false;
	} else {
		//查找是否有变量重定义
		if (symTable.top != 0)
			if (symTable.syms[symTable.top - 1].spaceLv == spLv) {
				int cnt = symTable.top - 1;
				while (symTable.syms[cnt].spaceLv == spLv) {
					if (symTable.syms[cnt].name == name) {
						error(7, "");
						return false;
					}
					--cnt;
				}
			}
		symTable.syms[symTable.top].name = name;
		symTable.syms[symTable.top].object = obj;
		symTable.syms[symTable.top].type = type;
		symTable.syms[symTable.top].size = size;
		symTable.syms[symTable.top].spaceLv = spLv;
		symTable.syms[symTable.top].addr = addr;
		if (obj == 4)
			++symTable.funcNum;
		++symTable.top;
		return true;
	}
}

void factor()
{
	switch (nowWord.sym) {
		case IDENT:
			getWord();
			if (nowWord.sym == LBRACK) { //数组取下标的情况
				getWord();
				expression();
				if (nowWord.sym != RBRACK) {
					error(10, "factor");
					return;
				} else
					getWord();
			} else {
				if (nowWord.sym == LPARENT) { //有返回值的函数调用
					getWord();
					if (nowWord.sym != RPARENT) {
						expression();
						while (nowWord.sym == COMMA) {
							getWord();
							expression();
						}
					}
					if (nowWord.sym != RPARENT) {
						error(12, "factor");
						return;
					} else
						getWord();
				} else
					; //为最普通的标识符（即普通变量）
			}
			break;
		case PLUS:
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return;
			}	
			getWord();
			break;
		case MINUS:
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return;
			}	
			getWord();
			break;
		case CONUNSIGN:
			getWord();
			break;
		case CONCHAR:
			getWord();
			break;
		case LPARENT:
			getWord();
			expression();
			if (nowWord.sym != RPARENT) {
				error(12, "factor");
				return;
			} else
				getWord();
			break;
	}
}

void term()
{
	factor();
	while (nowWord.sym == TIMES || nowWord.sym == DIV) {
		getWord();
		factor();
	}
}

void expression()
{
	if (nowWord.sym == MINUS || nowWord.sym == PLUS) //表达式前面有符号
		getWord();
	term();
	while (nowWord.sym == MINUS || nowWord.sym == PLUS) {
		getWord();
		term();
	}
	cout << "这是一个表达式" << endl;
}

void condition()
{
	expression();
	if (nowWord.sym == LESS || nowWord.sym == LESSEQL || nowWord.sym == GREAT ||
		nowWord.sym == GREATEQL || nowWord.sym == EQUAL || nowWord.sym == NOTEQL) {
		getWord();
		expression();
	} 
	cout << "这是一个条件" << endl;
}

void statement()
{
	switch (nowWord.sym) {
		case IFSY: //if...else语句
			if(test(LPARENT, 11))
				getWord();
			condition();
			if (nowWord.sym != RPARENT) {
				error(12, "if");
				return;
			} else
				getWord();
			statement();
			if (nowWord.sym == ELSESY) { //有else分句
				getWord();
				statement();
			}
			cout << "这是一个if语句" << endl;
			break;
		case DOSY: //do...while语句
			getWord();
			statement();
			if (nowWord.sym != WHILESY) {
				error(17, "while");
				return;
			}		
			if(test(LPARENT, 11))
				getWord();
			condition();
			if (nowWord.sym != RPARENT) {
				error(12, "while");
				return;
			} else 
				getWord();
			cout << "这是一个do-while语句" << endl;
			break;
		case FORSY: //for语句
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}	
			if(test(ASSIGN, 2))
				getWord();
			expression();
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
			if(test(ASSIGN, 2))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			} getWord();
			if (nowWord.sym != PLUS && nowWord.sym != MINUS) {
				error(18, "for");
				return;
			} else
				getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "for");
				return;
			}
			if(test(RPARENT, 12))
				getWord();
			statement();
			cout << "这是一个for语句" << endl;
			break;
		case LBRACE: //语句块
			getWord();
			while (nowWord.sym != RBRACE)
				statement(); //处理<语句>
			getWord();
			cout << "这是一个语句块" << endl;
			break;
		case IDENT: //有返回值函数调用语句，无返回值函数调用语句，赋值语句
			getWord();
			if (nowWord.sym == ASSIGN || nowWord.sym == LBRACK) { //赋值语句
				if (nowWord.sym == ASSIGN) {
					getWord();
					expression();
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else
						getWord();
					cout << "这是普通变量的赋值语句" << endl;
				} else {
					getWord();
					expression();
					if (nowWord.sym != RBRACK) {
						error(10, "for");
						return;
					}	
					if(test(ASSIGN, 2))
						getWord();
					expression();
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else
						getWord();
					cout << "这是数组下标的赋值语句" << endl;
				}
			} else { 
				if (nowWord.sym != LPARENT) {
					error(19, "");
					return;
				} else { //有返回值和无返回值的函数
					getWord();
					if (nowWord.sym != RPARENT) {
						expression();
						while (nowWord.sym == COMMA) {
							getWord();
							expression();
						}			
					}
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}
					cout << "这是有返回值或者无返回值的函数调用" << endl;
				}
			}
			break;
		case SCANFSY: //读语句
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}
			getWord();
			while (nowWord.sym == COMMA) {
				getWord();
				getWord();
			}
			if (nowWord.sym != RPARENT) {
				error(12, "while");
				return;
			} else {
				if(test(SEMICOLON, 5))
					getWord();
			}	
			cout << "这是读语句" << endl;
			break;
		case PRINTFSY: //写语句
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym == CONSTR) {
				getWord();
				if (nowWord.sym == COMMA) {
					getWord();
					expression();
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					}
						
					else
						test(SEMICOLON, 5);
					getWord();
				} else {
					if (nowWord.sym == RPARENT) {
						if(test(SEMICOLON, 5))
							getWord();
					} else {
						error(20, "");
						return;
					}	
				}
			} else { //只输出表达式
				expression();
				if (nowWord.sym != RPARENT) {
					error(12, "while");
					return;
				} else {
					if(test(SEMICOLON, 5))
						getWord();
				}
			}
			cout << "这是写语句" << endl;
			break;
		case SEMICOLON: //空语句
			getWord();
			cout << "这是空语句" << endl;
			break;
		case RETURNSY: //返回语句
			getWord();
			if (nowWord.sym == SEMICOLON)
				getWord();
			else {
				if (nowWord.sym == LPARENT) {
					getWord();
					expression();
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}	
				}
				else {
					error(21, "");
					return;
				}		
			}
			cout << "这是返回语句" << endl;
			break;
	}
}

void compoundState()
{
	if(nowWord.sym == CONSTSY)
		constDeclare(); //处理常量说明
	varDeclare(); //处理变量声明(可能不存在)
	//处理＜语句列＞，已经预读了一个符号了
	while (nowWord.sym != RBRACE) 
		statement(); //处理<语句>
	cout << "这是一个复合语句" << endl;
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
	getWord();//预读
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "program");
	cout << "这是一个main函数" << endl;
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
					cout << "这是一个int型常量的定义" << endl;
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
						temps += tempc;
						genInterMedia(CONST, "const", "char", name, temps);
						cout << "这是一个char常量的定义" << endl;
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
				cout << "这是一个普通变量定义" << endl;
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
							cout << "这是一个数组变量定义" << endl;
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
			++paramNum;
		}
		getWord();
	} while (nowWord.sym == COMMA);
	cout << "这是一个函数参数表" << endl;
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
	//处理形参表(此时nowWord指向左括号)
	if(flag == 0)
		getWord();//预读一个符号进入参数表处理函数
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//将函数信息登录进符号表
	insertSymTable(funcName, 4, funcType, paramNum, nowLevel, 0);
	//进入块，改变level
	nowLevel = (++addLevel);
	getWord();//预读一个符号以进入复合语句处理子函数
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//预读
	nowLevel = 0; //回到全局作用域
	cout << "这是一个有返回值的函数定义" << endl;
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
	if(test(LPARENT, 11))
		getWord();//预读一个符号进入参数表处理函数
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//将函数信息登录进符号表
	insertSymTable(funcName, 4, 2, paramNum, nowLevel, 0);
	//进入块，改变level
	nowLevel = (++addLevel);
	getWord();//预读一个符号以进入复合语句处理子函数
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//预读
	nowLevel = 0; //回到全局作用域
	cout << "这是一个无返回值的函数定义" << endl;
}

int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	nextCh();//预先读入一个字符以启动词法处理程序
	getWord();//预读一个单词
	//语法处理程序开始！
	program();
	cout << "语法处理成功完成" << endl;
	return 0;
}