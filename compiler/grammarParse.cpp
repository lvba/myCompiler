#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static bool varToFuncFlag = false; //����������������Ӻ�������˺���������ͷ��ʱ
static bool isReachMain = false;
static int misReadType;
static string misReadIdent;
static bool hasReturn = false;
//��������
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

pair<string, int> factor(int depth)//���������������ֱ�Ϊ��ʱ�����������ӣ�����ʽ������
{
	string retName = "";//�м�ʽ�ı�ʾ��
	string idName, integer, ch;
	switch (nowWord.sym) {
		case IDENT:
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == LBRACK) { //����ȡ�±�����
				//����Ƿ��Ѷ���
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
				if (nowWord.sym == LPARENT) { //�з���ֵ�ĺ�������
					//���������Ƿ�Ϸ�
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (!(symTable.syms[find].object == 4 && (symTable.syms[find].type != 2)))
							error(31, "");
					//�������������Ƿ���ͬ���ı���
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
					//�������ô����Ƿ�ƥ��
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
				} else { //Ϊ����ͨ�ı�ʶ��������ͨ����������
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
					if (symTable.syms[find].object == 0) {//�����滻
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
				return make_pair(retName, 0);//�������ſ���ǿ��ת��Ϊ����
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
	if (nowWord.sym == MINUS || nowWord.sym == PLUS) { //���ʽǰ���з���
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
	//cout << "����һ�����ʽ" << endl;
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
		//cout << "����һ������" << endl;
		return;
	}
	if (isSingle)
		resetTemp();
	if (ret1Type != 0)
		error(36, "");
	genInterMedia(COMPARE, ret1, "!=", "0", "");
	//cout << "����һ������" << endl;
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
			statement(funcType);
			if (nowWord.sym == ELSESY) { //��else�־�
				ifLabel2 = genLabel();
				genInterMedia(GOTO, "GOTO", ifLabel2, "", "");
				genInterMedia(LABEL, ifLabel1, ":", "", "");
				getWord();
				statement(funcType);
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
			//cout << "����һ��for���" << endl;
			break;
		case LBRACE: //����
			getWord();
			while (nowWord.sym != RBRACE)
				statement(funcType); //����<���>
			getWord();
			//cout << "����һ������" << endl;
			break;
		case IDENT: //�з���ֵ����������䣬�޷���ֵ����������䣬��ֵ���
			idName = nowWord.str;
			getWord();
			if (nowWord.sym == ASSIGN || nowWord.sym == LBRACK) { //��ֵ���
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
					//cout << "������ͨ�����ĸ�ֵ���" << endl;
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
					//cout << "���������±�ĸ�ֵ���" << endl;
				}
			} else { 
				if (nowWord.sym != LPARENT) {
					error(19, "");
					return;
				} else { //�з���ֵ���޷���ֵ�ĺ���
					int find = searchSymTable(idName, nowLevel);
					if (find == -1) {
						error(29, "");
						find = 0;
					} else
						if (symTable.syms[find].object != 4)
							error(45, "");
					//�������������Ƿ���ͬ���ı���
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
					//�������ô����Ƿ�ƥ��
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
					//cout << "�����з���ֵ�����޷���ֵ�ĺ�������" << endl;
				}
			}
			break;
		case SCANFSY: //�����
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
				} else {//printf '('���ַ�����')'
					if (nowWord.sym == RPARENT) {
						if(test(SEMICOLON, 5))
							getWord();
					} else {
						error(20, "");
						return;
					}	
					genInterMedia(CALL, "call", "printf", "1", "");
				}				
			} else { //ֻ������ʽ
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
			//cout << "����д���" << endl;
			break;
		case SEMICOLON: //�����
			getWord();
			//cout << "���ǿ����" << endl;
			break;
		case RETURNSY: //�������
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
			//cout << "���Ƿ������" << endl;
			break;
	}
}

void compoundState(int funcType)
{
	if(nowWord.sym == CONSTSY)
		constDeclare(); //������˵��
	varDeclare(); //�����������(���ܲ�����)
	//��������У����Ѿ�Ԥ����һ��������
	while (nowWord.sym != RBRACE) 
		statement(funcType); //����<���>
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
	nowLevel = (++addLevel);
	//��main������¼����ű�
	insertSymTable("main", 4, 2, 0, 0, 0);
	genInterMedia(FUNC, "void", "main", "()", "");
	getWord();//Ԥ��
	compoundState(2);
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
						temps += '\'';
						temps += tempc;
						temps += '\'';
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
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, funcType, -1, 0, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//�޸ĺ�����������
	symTable.syms[symTable.funcInd[symTable.funcInd.size() - 1]].size = paramNum;
	hasReturn = false;
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState(funcType);
	if (!hasReturn)
		error(41, "");
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
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, 2, -1, 0, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12, "returnFunc");
	else
		test(LBRACE, 13);
	//�޸ĺ�����������
	symTable.syms[symTable.funcInd[symTable.funcInd.size() - 1]].size = paramNum;
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState(2);
	if (nowWord.sym != RBRACE)
		error(14, "");
	else
		getWord();//Ԥ��
	nowLevel = 0; //�ص�ȫ��������
	//cout << "����һ���޷���ֵ�ĺ�������" << endl;
}
