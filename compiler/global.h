#pragma once
#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
using namespace std;

//ȫ�ֹ����������
extern ifstream infile;
extern ofstream ofile;
extern int lineCnt;
extern int row;
extern int nowLevel;
extern int addLevel;

extern enum symbol {//ö�����������
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<�ַ�>*/, CONSTR/*<�ַ���>*/,
	SEMICOLON, ASSIGN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
	IDENT,
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY,
	DOSY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY
};
struct nowword {
	int num;
	string str;
	symbol sym;
};
extern struct nowword nowWord;
struct sym {
	string name; //��ʶ����
	int object; //0-������1-������2-����������3-���������4-����
	int type; //0-int��1-char��2-void����typeΪ����ʱ��
	int size; // �����ά�����ߺ����Ĳ�������
	int spaceLv; // �ñ�ʶ������������
	int addr; //���ڱ������������������ջ�д洢��Ԫ��λ�ƣ����������볣�����е�¼��λ�ã����ڳ�����ֱ������ֵ
};
struct symtab {
	struct sym syms[5000];
	int top; //ջ��ָ��
	int funcNum; //��ǰ�����ĺ�������
};
extern struct symtab symTable;

//ȫ�ֹ���������
extern const int maxLineLen;
extern const int maxIdenLen;
extern const int keywordNum;
extern const int maxNumBit;
extern const int maxNum;
extern const int maxSymNum;
extern const int stackSize;

extern const string symStr[37];
extern const string keyword[13];
extern const symbol keySymbol[13];

//ȫ�ֹ���������
void nextCh();
void getWord();
void error(int errCode);
int getInt();
bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr);