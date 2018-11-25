#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static string filePath;
static bool varToFuncFlag = false; //����������������Ӻ�������˺���������ͷ��ʱ
static bool isReachMain = false;
static int misReadType;
static string misReadIdent;
//��������
void compoundState();
void voidFunc();
void varDeclare();
void returnFunc();
void constDeclare();
string expression();

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
	if (symTable.top >= maxSymNum) { //���ű����
		error(6, "");
		return false;
	} else {
		//�����Ƿ��б����ض���
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

string factor()
{
	string retName = "";//�м�ʽ�ı�ʾ��
	string idName, integer, ch;
	switch (nowWord.sym) {
		case IDENT:
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == LBRACK) { //����ȡ�±�����
				getWord();
				string exprName = expression();
				if (nowWord.sym != RBRACK) {
					error(10, "factor");
					return "";
				} else {
					string retName = genTemp();
					genInterMedia(ARRASS, retName, idName, "[]", exprName);
					getWord();
					return retName;
				}		
			} else {
				if (nowWord.sym == LPARENT) { //�з���ֵ�ĺ�������
					getWord();
					if (nowWord.sym != RPARENT) {
						string param = expression();
						genInterMedia(PUSH, "push", param, "", "");
						while (nowWord.sym == COMMA) {
							getWord();
							string param = expression();
							genInterMedia(PUSH, "push", param, "", "");
						}
					}
					if (nowWord.sym != RPARENT) {
						error(12, "factor");
						return "";
					} else {
						genInterMedia(CALL, "call", idName, "", "");
						string retName = genTemp();
						genInterMedia(VARASS, retName, "RET", "", "");
						getWord();
						return retName;
					}	
				} else
					return idName; //Ϊ����ͨ�ı�ʶ��������ͨ������
			}
			break;
		case PLUS:
			integer = "";
			integer += '+';
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return "";
			}
			integer += to_string(nowWord.num);
			getWord();
			return integer;
			break;
		case MINUS:
			integer = "";
			integer += '-';
			getWord();
			if (nowWord.sym != CONUNSIGN) {
				error(16, "factor");
				return "";
			}	
			integer += to_string(nowWord.num);
			getWord();
			return integer;
			break;
		case CONUNSIGN:
			integer = to_string(nowWord.num);
			getWord();
			return integer;
			break;
		case CONCHAR:
			ch = nowWord.str;
			getWord();
			return ch;
			break;
		case LPARENT:
			getWord();
			retName = expression();
			if (nowWord.sym != RPARENT) {
				error(12, "factor");
				return "";
			} else {
				getWord();
				return retName;
			}	
			break;
	}
}

string term()
{
	string ret = factor();
	string retName = ret;
	while (nowWord.sym == TIMES || nowWord.sym == DIV) {
		string op = nowWord.sym == TIMES ? "*" : "/";
		getWord();
		retName = genTemp();
		genInterMedia(VARASS, retName, ret, op, factor());
		ret = retName;
	}
	return retName;
}

string expression()
{
	symbol op = VOIDSY;
	if (nowWord.sym == MINUS || nowWord.sym == PLUS) { //���ʽǰ���з���
		op = nowWord.sym;
		getWord();
	}
	string ret = term();
	string retName = ret;
	if (op == MINUS) {
		retName = genTemp();
		genInterMedia(VARASS, retName, "-1", "*", ret);
	}
	while (nowWord.sym == MINUS || nowWord.sym == PLUS) {
		string op = nowWord.sym == MINUS ? "-" : "+";
		getWord();
		retName = genTemp();
		genInterMedia(VARASS, retName, ret, op, term());
		ret = retName;
	}
	resetTemp();
	//cout << "����һ�����ʽ" << endl;
	return retName;
}

void condition()
{
	string ret1 = expression();
	if (nowWord.sym == LESS || nowWord.sym == LESSEQL || nowWord.sym == GREAT ||
		nowWord.sym == GREATEQL || nowWord.sym == EQUAL || nowWord.sym == NOTEQL) {
		symbol op = nowWord.sym;
		string opArr[6] = { "<", "<=", ">", ">=", "!=", "==" };
		getWord();
		string ret2 = expression();
		genInterMedia(COMPARE, ret1, opArr[op - 4], ret2, "");
		//cout << "����һ������" << endl;
		return;
	} 
	genInterMedia(COMPARE, ret1, "!=", "0", "");
	//cout << "����һ������" << endl;
	return;
}

void statement()
{
	string ifLabel1, ifLabel2;
	string doLabel;
	string idName, idName2, idName3, exprRet, forLabel1, forLabel2, step;
	symbol op;
	string arrDimen;
	switch (nowWord.sym) {
		case IFSY: //if...else���
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
			statement();
			if (nowWord.sym == ELSESY) { //��else�־�
				ifLabel2 = genLabel();
				genInterMedia(GOTO, "GOTO", ifLabel2, "", "");
				genInterMedia(LABEL, ifLabel1, ":", "", "");
				getWord();
				statement();
				genInterMedia(LABEL, ifLabel2, ":", "", "");
			} else {
				genInterMedia(LABEL, ifLabel1, ":", "", "");
			}
			//cout << "����һ��if���" << endl;
			break;
		case DOSY: //do...while���
			doLabel = genLabel();
			genInterMedia(LABEL, doLabel, ":", "", "");
			getWord();
			statement();
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
			//cout << "����һ��do-while���" << endl;
			break;
		case FORSY: //for���
			forLabel1 = genLabel();
			forLabel2 = genLabel();
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}
			idName = nowWord.str;
			if(test(ASSIGN, 2))
				getWord();
			exprRet = expression();
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
			if(test(ASSIGN, 2))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			} 
			idName3 = nowWord.str;
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
			statement();
			genInterMedia(VARASS, idName2, idName3, op == PLUS ? "+" : "-", step);
			genInterMedia(GOTO, "GOTO", forLabel1, "", "");
			genInterMedia(LABEL, forLabel2, ":", "", "");
			//cout << "����һ��for���" << endl;
			break;
		case LBRACE: //����
			getWord();
			while (nowWord.sym != RBRACE)
				statement(); //����<���>
			getWord();
			//cout << "����һ������" << endl;
			break;
		case IDENT: //�з���ֵ����������䣬�޷���ֵ����������䣬��ֵ���
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == ASSIGN || nowWord.sym == LBRACK) { //��ֵ���
				if (nowWord.sym == ASSIGN) {
					getWord();
					exprRet = expression();
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else {
						genInterMedia(VARASS, idName, exprRet, "", "");
						getWord();
					}	
					//cout << "������ͨ�����ĸ�ֵ���" << endl;
				} else {
					getWord();
					arrDimen = expression();
					if (nowWord.sym != RBRACK) {
						error(10, "for");
						return;
					}	
					if(test(ASSIGN, 2))
						getWord();
					exprRet = expression();
					if (nowWord.sym != SEMICOLON) {
						error(5, "for");
						return;
					} else {
						genInterMedia(ARRASS, idName, "[]", arrDimen, exprRet);
						getWord();
					}						
					//cout << "���������±�ĸ�ֵ���" << endl;
				}
			} else { 
				if (nowWord.sym != LPARENT) {
					error(19, "");
					return;
				} else { //�з���ֵ���޷���ֵ�ĺ���
					getWord();
					if (nowWord.sym != RPARENT) {
						exprRet = expression();
						genInterMedia(PUSH, "push", exprRet, "", "");
						while (nowWord.sym == COMMA) {
							getWord();
							exprRet = expression();
							genInterMedia(PUSH, "push", exprRet, "", "");
						}			
					}
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}
					genInterMedia(CALL, "call", idName, "", "");
					//cout << "�����з���ֵ�����޷���ֵ�ĺ�������" << endl;
				}
			}
			break;
		case SCANFSY: //�����
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym != IDENT) {
				error(1, "for");
				return;
			}
			genInterMedia(PUSH, "push", nowWord.str, "", "");
			getWord();
			while (nowWord.sym == COMMA) {
				getWord();
				genInterMedia(PUSH, "push", nowWord.str, "", "");
				getWord();
			}
			if (nowWord.sym != RPARENT) {
				error(12, "while");
				return;
			} else {
				if(test(SEMICOLON, 5))
					getWord();
			}
			genInterMedia(CALL, "call", "scanf", "", "");
			//cout << "���Ƕ����" << endl;
			break;
		case PRINTFSY: //д���
			if(test(LPARENT, 11))
				getWord();
			if (nowWord.sym == CONSTR) {
				genInterMedia(PUSH, "push", "\"" + nowWord.str + "\"", "", "");
				getWord();
				if (nowWord.sym == COMMA) {//printf'('���ַ�����,�����ʽ��')'
					getWord();
					exprRet = expression();
					genInterMedia(PUSH, "push", exprRet, "", "");
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else
						test(SEMICOLON, 5);
					getWord();
				} else {//printf '('���ַ�����')'
					if (nowWord.sym == RPARENT) {
						if(test(SEMICOLON, 5))
							getWord();
					} else {
						error(20, "");
						return;
					}	
				}
				genInterMedia(CALL, "call", "printf", "", "");
			} else { //ֻ������ʽ
				exprRet = expression();
				genInterMedia(PUSH, "push", exprRet, "", "");
				if (nowWord.sym != RPARENT) {
					error(12, "while");
					return;
				} else {
					if(test(SEMICOLON, 5))
						getWord();
				}
				genInterMedia(CALL, "call", "printf", "", "");
			}
			//cout << "����д���" << endl;
			break;
		case SEMICOLON: //�����
			getWord();
			//cout << "���ǿ����" << endl;
			break;
		case RETURNSY: //�������
			getWord();
			if (nowWord.sym == SEMICOLON) {
				genInterMedia(RET, "ret", "", "", "");
				getWord();
			} else {
				if (nowWord.sym == LPARENT) {
					getWord();
					exprRet = expression();
					if (nowWord.sym != RPARENT) {
						error(12, "while");
						return;
					} else {
						if(test(SEMICOLON, 5))
							getWord();
					}	
					genInterMedia(RET, "ret", exprRet, "", "");
				} else {
					error(21, "");
					return;
				}		
			}
			//cout << "���Ƿ������" << endl;
			break;
	}
}

void compoundState()
{
	if(nowWord.sym == CONSTSY)
		constDeclare(); //������˵��
	varDeclare(); //�����������(���ܲ�����)
	//��������У����Ѿ�Ԥ����һ��������
	while (nowWord.sym != RBRACE) 
		statement(); //����<���>
	//cout << "����һ���������" << endl;
}

//����������﷨������
void program()
{
	if (nowWord.sym == CONSTSY) //�����Գ���������ʼ
		constDeclare(); //������˵��
	varDeclare(); //�����������
	//������û�б���������������˵��������֮������з���ֵ�ĺ����������main����ʱ��
	//��������ͷ�����������void������Ӱ�죬��������
	if (varToFuncFlag)  //���������˵����������������з���ֵ�ĺ���
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
	//��ʱ��ǰ��ʶ��ָ��main
	test(LPARENT, 11);
	test(RPARENT, 12);
	test(LBRACE, 13);
	getWord();//Ԥ��
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "program");
	//cout << "����һ��main����" << endl;
}

void constDef()
{
	string name;
	if (nowWord.str == "int") { //int�ͳ�������
		do {
			getWord();
			if (nowWord.sym != IDENT) {
				error(1, "constDef");//Ӧ�Ǳ�ʶ��
				continue;
			} else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN) {
					error(2, "constDef");//Ӧ��=
					continue;
				} else {
					int constInt = getInt();
					//��¼���ű�
					if (constInt != -99999998) {
						insertSymTable(name, 0, 0, -1, nowLevel, constInt);
						genInterMedia(CONST, "const", "int", name, to_string(constInt));
					}	
					//cout << "����һ��int�ͳ����Ķ���" << endl;
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);	
		return;
	}
	if (nowWord.str == "char") { //char�ͳ�������
		do {
			getWord();
			if (nowWord.sym != IDENT) {
				error(1, "constDef");//Ӧ�Ǳ�ʶ��
				continue;
			} else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN) {
					error(2, "constDef");//Ӧ��=
					continue;
				} else {
					getWord();
					if (nowWord.sym != CONCHAR) {
						error(3, "");
						continue;
					} else {
						char tempc = nowWord.str[1];
						//��¼���ű�
						insertSymTable(name, 0, 1, -1, nowLevel, tempc);
						string temps = "";
						temps += tempc;
						genInterMedia(CONST, "const", "char", name, temps);
						//cout << "����һ��char�����Ķ���" << endl;
					}		
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);
		return;
	}
	error(0, "constDef");//����ӦΪ���ͱ�ʶ��
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
			if (nowWord.sym == LPARENT) { //�������ڴ�����﷨�ɷֲ��Ǳ���������Ѿ��Ǻ���������
				//�����൱�ڶ���˺���������ͷ��
				varToFuncFlag = true;
				misReadType = type;
				misReadIdent = varName;
				return false;
			}
			if (nowWord.sym == COMMA || nowWord.sym == SEMICOLON) {
				//��¼���ű�������addr��ʱ��ʱ��ʼ��Ϊ0
				insertSymTable(varName, 1, type, -1, nowLevel, 0);
				genInterMedia(VAR, "var", type == 0 ? "int" : "char", varName, "");
				//cout << "����һ����ͨ��������" << endl;
			} else {
				if (nowWord.sym == LBRACK) { //�������
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
								getWord();//Ԥ����һ�������Ա�ѭ���ж�
							//��¼���ű���ַ��ʱ��0
							insertSymTable(varName, 3, type, dimen, nowLevel, 0);
							genInterMedia(ARRAY, "array", type == 0 ? "int" : "char", varName, to_string(dimen));
							//cout << "����һ�������������" << endl;
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
	if (nowWord.sym == RPARENT) //������Ϊ��
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
			//���ú�������������¼���ű���ַ��ʱ��0
			insertSymTable(nowWord.str, 2, type, -1, nowLevel, 0);
			genInterMedia(PARAM, "para", type == 0 ? "int" : "char", nowWord.str, "");
			++paramNum;
		}
		getWord();
	} while (nowWord.sym == COMMA);
	//cout << "����һ������������" << endl;
	return paramNum;
}

void returnFunc()
{
	int funcType, flag = 0;
	string funcName;
	if (varToFuncFlag) { //�������˵��ʱ���������ͷ��
		funcType = misReadType;
		funcName = misReadIdent;
		varToFuncFlag = false; //�����ٽ�����Ӻ���ʱ�϶������ٷ�������������
	} else { //����������Ӻ���ʱ
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
	//�����ɺ���ͷ�����м�ʽ
	genInterMedia(FUNC, funcType == 0 ? "int" : "char", funcName, "()", "");
	//�����βα�(��ʱnowWordָ��������)
	if(flag == 0)
		getWord();//Ԥ��һ�����Ž������������
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, funcType, paramNum, nowLevel, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//Ԥ��
	nowLevel = 0; //�ص�ȫ��������
	//cout << "����һ���з���ֵ�ĺ�������" << endl;
}

void voidFunc()
{
	if (nowWord.sym != VOIDSY) {
		error(15, "");
		return;
	}	
	string funcName;
	getWord();
	if (nowWord.sym == MAINSY) {//��Ϊvoid main����Ӧ�ɴ��Ӻ�����������
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
		getWord();//Ԥ��һ�����Ž������������
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, 2, paramNum, nowLevel, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//Ԥ��
	nowLevel = 0; //�ص�ȫ��������
	//cout << "����һ���޷���ֵ�ĺ�������" << endl;
}

int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	nextCh();//Ԥ�ȶ���һ���ַ��������ʷ��������
	getWord();//Ԥ��һ������
	//�﷨�������ʼ��
	program();
	cout << "�﷨���������" << endl;
	printImTable();
	return 0;
}