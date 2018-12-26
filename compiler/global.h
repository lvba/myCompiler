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
extern int lastLineCnt;
extern int lastRow;
extern int nowLevel;
extern int addLevel;
extern char nowCh;
extern int errorCnt;
extern int staticTempNum;
extern vector<pair<string, string> > regPool; //�Ĵ�������������
extern vector<string> tempRegTab; //�洢_TEMP�����ļĴ�������״̬

extern enum symbol {//ö�����������
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<�ַ�>*/, CONSTR/*<�ַ���>*/,
	SEMICOLON, ASSIGN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
	IDENT,
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY,
	DOSY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY
};
extern enum interType {//ö���м�ʽ������
	VAR, CONST, VARASS, FUNC, PARAM, PUSH, ARRAY,
	CALL, RET, COMPARE, GOTO, BNZ, BZ, ARRASS, LABEL
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
	int addr; //�����ͺ���������������mips����ջ�����memory��ƫ�ƣ������������һ��Ԫ�ص�ƫ��
	string reg; //��������ļĴ���(û����Ϊ��)
};
struct symtab {
	struct sym syms[5000];
	int top; //ջ��ָ��
	vector<int> funcInd; //��ǰ�����ĺ����ڷ��ű��е��±�
};
extern struct symtab symTable;
struct intermedia {
	string expr[4]; //��Ԫʽ
	interType type; //��Ԫʽ������
	int len;
};
struct interMediaTab {
	struct intermedia exprs[10000];
	int ind; //����ָ��
};
extern struct interMediaTab imTable;
struct polishNote {//����ÿһ�����ʽ�ĺ�׺���ʽ������洢
	vector<string> stack;
	bool canBeCalc;//�Ƿ����ֱ���ɺ�׺���ʽ�����ֵ���������������ã�
};
struct mipsAsm {//�洢mips���Ľṹ
	string instr;
	string r1;
	string r2;
	string r3;
};
extern vector<struct mipsAsm*> mipsTable;
struct basicBlock {//����������ݽṹ
	int blockNum;
	vector<int> codes;//�������еĴ�����imTable�е��±�
	vector<struct basicBlock*> prevBlocks;
	vector<struct basicBlock*> nextBlocks;
	vector<pair<int, int> > gen;//pair�ֱ�Ϊ��������blockGraph���±��Լ��м�ʽ��codes���±�
	vector<pair<int, int> > kill;
	vector<pair<int, int> > in;
	vector<pair<int, int> > out;
	string inStr;
	string outStr;
};
typedef struct basicBlock* block;
extern vector<block> blockGraph;
struct node {
	string name;
	int nodeNum;
	struct node* lchild;
	struct node* rchild;
	vector<struct node*> parents;
	int isLeaf;
	int hasInQue;
};
typedef struct node* dagNode;
extern vector<dagNode> dagGraph;

//ȫ�ֹ���������
extern const int maxLineLen;
extern const int maxIdenLen;
extern const int keywordNum;
extern const int maxNumBit;
extern const int maxNum;
extern const int maxSymNum;
extern const int maxIntermediaNum;
extern const int stackSize;

extern const string symStr[37];
extern const string keyword[13];
extern const symbol keySymbol[13];

//ȫ�ֹ���������
void nextCh();
void getWord();
int error(int errCode, string errInfo);
int getInt();
bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr);
int searchSymTable(string name, int nowLevel);
int searchWithLevel(string name, int object, int nowLevel);
int searchAllLevel(string name, int nowLevel);
void genInterMedia(interType type, string p1, string p2, string p3, string p4);
string genLabel();
string genTemp();
void resetTemp();
void printImTable();
void program();
void genMips();
void printMips();
void printEachIm(int i);
void divideBlocks();
void dagOpt();
void genOneCode(string instr, string r1, string r2, string r3);