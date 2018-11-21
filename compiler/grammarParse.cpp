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
void expression();

void test(symbol sym, int errCode)
{
	getWord();
	if (nowWord.sym != sym)
		error(errCode);
}

bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr)
{
	if (symTable.top >= maxSymNum) { //���ű����
		error(6);
		return false;
	} else {
		//�����Ƿ��б����ض���
		if (symTable.top != 0)
			if (symTable.syms[symTable.top - 1].spaceLv == spLv) {
				int cnt = symTable.top - 1;
				while (symTable.syms[cnt].spaceLv == spLv) {
					if (symTable.syms[cnt].name == name) {
						error(7);
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
			if (nowWord.sym == LBRACK) { //����ȡ�±�����
				getWord();
				expression();
				if (nowWord.sym != RBRACK)
					error(10);
				getWord();
			} else {
				if (nowWord.sym == LPARENT) { //�з���ֵ�ĺ�������
					getWord();
					if (nowWord.sym != RPARENT) {
						expression();
						while (nowWord.sym == COMMA) {
							getWord();
							expression();
						}
					}
					if (nowWord.sym != RPARENT)
						error(12);
					getWord();
				} else
					; //Ϊ����ͨ�ı�ʶ��������ͨ������
			}
			break;
		case PLUS:
			getWord();
			if (nowWord.sym != CONUNSIGN)
				error(16);
			getWord();
			break;
		case MINUS:
			getWord();
			if (nowWord.sym != CONUNSIGN)
				error(16);
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
			if (nowWord.sym != RPARENT)
				error(12);
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
	if (nowWord.sym == MINUS || nowWord.sym == PLUS) //���ʽǰ���з���
		getWord();
	term();
	while (nowWord.sym == MINUS || nowWord.sym == PLUS) {
		getWord();
		term();
	}
	cout << "����һ�����ʽ" << endl;
}

void condition()
{
	expression();
	if (nowWord.sym == LESS || nowWord.sym == LESSEQL || nowWord.sym == GREAT ||
		nowWord.sym == GREATEQL || nowWord.sym == EQUAL || nowWord.sym == NOTEQL) {
		getWord();
		expression();
	} 
	cout << "����һ������" << endl;
}

void statement()
{
	switch (nowWord.sym) {
		case IFSY: //if...else���
			test(LPARENT, 11);
			getWord();
			condition();
			if (nowWord.sym != RPARENT)
				error(12);
			getWord();
			statement();
			if (nowWord.sym == ELSESY) { //��else�־�
				getWord();
				statement();
			}
			cout << "����һ��if���" << endl;
			break;
		case DOSY: //do...while���
			getWord();
			statement();
			if (nowWord.sym != WHILESY)
				error(17);
			test(LPARENT, 11);
			getWord();
			condition();
			if (nowWord.sym != RPARENT)
				error(12);
			getWord();
			cout << "����һ��do-while���" << endl;
			break;
		case FORSY: //for���
			test(LPARENT, 11);
			getWord();
			if (nowWord.sym != IDENT)
				error(1);
			test(ASSIGN, 2);
			getWord();
			expression();
			if (nowWord.sym != SEMICOLON)
				error(5);
			getWord();
			condition();
			if (nowWord.sym != SEMICOLON)
				error(5);
			getWord();
			if (nowWord.sym != IDENT)
				error(1);
			test(ASSIGN, 2);
			getWord();
			if (nowWord.sym != IDENT)
				error(1);
			getWord();
			if (nowWord.sym != PLUS && nowWord.sym != MINUS)
				error(18);
			getWord();
			if (nowWord.sym != CONUNSIGN)
				error(16);
			test(RPARENT, 12);
			getWord();
			statement();
			cout << "����һ��for���" << endl;
			break;
		case LBRACE: //����
			getWord();
			while (nowWord.sym != RBRACE)
				statement(); //����<���>
			getWord();
			cout << "����һ������" << endl;
			break;
		case IDENT: //�з���ֵ����������䣬�޷���ֵ����������䣬��ֵ���
			getWord();
			if (nowWord.sym == ASSIGN || nowWord.sym == LBRACK) { //��ֵ���
				if (nowWord.sym == ASSIGN) {
					getWord();
					expression();
					if (nowWord.sym != SEMICOLON)
						error(5);
					getWord();
					cout << "������ͨ�����ĸ�ֵ���" << endl;
				} else {
					getWord();
					expression();
					if (nowWord.sym != RBRACK)
						error(10);
					test(ASSIGN, 2);
					getWord();
					expression();
					if (nowWord.sym != SEMICOLON)
						error(5);
					getWord();
					cout << "���������±�ĸ�ֵ���" << endl;
				}
			} else { 
				if (nowWord.sym != LPARENT)
					error(19);
				else { //�з���ֵ���޷���ֵ�ĺ���
					getWord();
					if (nowWord.sym != RPARENT) {
						expression();
						while (nowWord.sym == COMMA) {
							getWord();
							expression();
						}			
					}
					if (nowWord.sym != RPARENT)
						error(12);
					test(SEMICOLON, 5);
					getWord();
					cout << "�����з���ֵ�����޷���ֵ�ĺ�������" << endl;
				}
			}
			break;
		case SCANFSY: //�����
			test(LPARENT, 11);
			getWord();
			if (nowWord.sym != IDENT)
				error(1);
			getWord();
			while (nowWord.sym == COMMA) {
				getWord();
				getWord();
			}
			if (nowWord.sym != RPARENT)
				error(12);
			test(SEMICOLON, 5);
			getWord();
			cout << "���Ƕ����" << endl;
			break;
		case PRINTFSY: //д���
			test(LPARENT, 11);
			getWord();
			if (nowWord.sym == CONSTR) {
				getWord();
				if (nowWord.sym == COMMA) {
					getWord();
					expression();
					if (nowWord.sym != RPARENT)
						error(12);
					test(SEMICOLON, 5);
					getWord();
				} else {
					if (nowWord.sym == RPARENT) {
						test(SEMICOLON, 5);
						getWord();
					} else
						error(20);
				}
			} else { //ֻ������ʽ
				expression();
				if (nowWord.sym != RPARENT)
					error(12);
				test(SEMICOLON, 5);
				getWord();
			}
			cout << "����д���" << endl;
			break;
		case SEMICOLON: //�����
			getWord();
			cout << "���ǿ����" << endl;
			break;
		case RETURNSY: //�������
			getWord();
			if (nowWord.sym == SEMICOLON)
				getWord();
			else {
				if (nowWord.sym == LPARENT) {
					getWord();
					expression();
					if (nowWord.sym != RPARENT)
						error(12);
					test(SEMICOLON, 5);
					getWord();
				}
				else
					error(21);
			}
			cout << "���Ƿ������" << endl;
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
	cout << "����һ���������" << endl;
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
				error(0);
		}
	}
	//��ʱ��ǰ��ʶ��ָ��main
	test(LPARENT, 11);
	test(RPARENT, 12);
	test(LBRACE, 13);
	getWord();//Ԥ��
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14);
	cout << "����һ��main����" << endl;
}

void constDef()
{
	string name;
	if (nowWord.str == "int") { //int�ͳ�������
		do {
			getWord();
			if (nowWord.sym != IDENT)
				error(1);//Ӧ�Ǳ�ʶ��
			else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN)
					error(2);//Ӧ��=
				else {
					int constInt = getInt();
					//��¼���ű�
					if(constInt != -99999998)
						insertSymTable(name, 0, 0, -1, nowLevel, constInt);
					cout << "����һ��int�ͳ����Ķ���" << endl;
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);	
		return;
	}
	if (nowWord.str == "char") { //char�ͳ�������
		do {
			getWord();
			if (nowWord.sym != IDENT)
				error(1);//Ӧ�Ǳ�ʶ��
			else {
				name = nowWord.str;
				getWord();
				if (nowWord.sym != ASSIGN)
					error(2);//Ӧ��=
				else {
					getWord();
					if (nowWord.sym != CONCHAR)
						error(3);
					else {
						char tempc = nowWord.str[1];
						//��¼���ű�
						insertSymTable(name, 0, 1, -1, nowLevel, tempc);
						cout << "����һ��char�����Ķ���" << endl;
					}		
				}
			}
			getWord();
		} while (nowWord.sym == COMMA);
		return;
	}
	error(0);//����ӦΪ���ͱ�ʶ��
}

void constDeclare()
{
	if (nowWord.sym != CONSTSY) 
		error(4);
	do {
		getWord();
		constDef();
		if (nowWord.sym != SEMICOLON)
			error(5);
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
		if (nowWord.sym != IDENT && nowWord.sym != MAINSY)
			error(1);
		else {
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
				cout << "����һ����ͨ��������" << endl;
			} else {
				if (nowWord.sym == LBRACK) { //�������
					int dimen = -1;
					getWord();
					if (nowWord.sym != CONUNSIGN)
						error(8);
					else {
						if (nowWord.num == 0)
							error(9);
						else {
							dimen = nowWord.num;
							test(RBRACK, 10);
							//��¼���ű���ַ��ʱ��0
							insertSymTable(varName, 3, type, dimen, nowLevel, 0);
							cout << "����һ�������������" << endl;
							getWord();//Ԥ����һ�������Ա�ѭ���ж�
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
				error(5);
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
			else
				error(0);
		}
		getWord();
		if (nowWord.sym != IDENT)
			error(1);
		else {
			//���ú�������������¼���ű���ַ��ʱ��0
			insertSymTable(nowWord.str, 2, type, -1, nowLevel, 0);
			++paramNum;
		}
		getWord();
	} while (nowWord.sym == COMMA);
	cout << "����һ������������" << endl;
	return paramNum;
}

void returnFunc()
{
	int funcType;
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
		if (nowWord.sym != IDENT)
			error(1);
		else 
			funcName = nowWord.str;
		test(LPARENT, 11);
	}
	//�����βα�(��ʱnowWordָ��������)
	getWord();//Ԥ��һ�����Ž������������
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12);
	test(LBRACE, 13);
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, funcType, paramNum, nowLevel, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14);
	nowLevel = 0; //�ص�ȫ��������
	getWord();//Ԥ��
	cout << "����һ���з���ֵ�ĺ�������" << endl;
}

void voidFunc()
{
	if (nowWord.sym != VOIDSY)
		error(15);
	string funcName;
	getWord();
	if (nowWord.sym == MAINSY) {//��Ϊvoid main����Ӧ�ɴ��Ӻ�����������
		isReachMain = true;
		return;
	}
	if (nowWord.sym != IDENT)
		error(1);
	else
		funcName = nowWord.str;
	test(LPARENT, 11);
	getWord();//Ԥ��һ�����Ž������������
	int paramNum = paramList();
	if (nowWord.sym != RPARENT)
		error(12);
	test(LBRACE, 13);
	//��������Ϣ��¼�����ű�
	insertSymTable(funcName, 4, 2, paramNum, nowLevel, 0);
	//����飬�ı�level
	nowLevel = (++addLevel);
	getWord();//Ԥ��һ�������Խ��븴����䴦���Ӻ���
	compoundState();
	if (nowWord.sym != RBRACE)
		error(14);
	nowLevel = 0; //�ص�ȫ��������
	getWord();//Ԥ��
	cout << "����һ���޷���ֵ�ĺ�������" << endl;
}

int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	nextCh();//Ԥ�ȶ���һ���ַ��������ʷ��������
	getWord();//Ԥ��һ������
	//�﷨�������ʼ��
	program();
	cout << "�﷨����ɹ����" << endl;
	return 0;
}