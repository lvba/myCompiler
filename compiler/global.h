#pragma once
#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
using namespace std;

//全局共享变量声明
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
extern vector<pair<string, string> > regPool; //寄存器名，变量名
extern vector<string> tempRegTab; //存储_TEMP变量的寄存器分配状态

extern enum symbol {//枚举所有类别码
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<字符>*/, CONSTR/*<字符串>*/,
	SEMICOLON, ASSIGN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
	IDENT,
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY,
	DOSY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY
};
extern enum interType {//枚举中间式的种类
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
	string name; //标识符名
	int object; //0-常量，1-变量，2-函数变量，3-数组变量，4-函数
	int type; //0-int，1-char，2-void（当type为函数时）
	int size; // 数组的维数或者函数的参数个数
	int spaceLv; // 该标识符的作用域层次
	int addr; //变量和函数变量填入其在mips运行栈中相对memory的偏移，数组填入其第一个元素的偏移
	string reg; //变量分配的寄存器(没有则为空)
};
struct symtab {
	struct sym syms[5000];
	int top; //栈顶指针
	vector<int> funcInd; //当前声明的函数在符号表中的下标
};
extern struct symtab symTable;
struct intermedia {
	string expr[4]; //四元式
	interType type; //四元式的类型
	int len;
};
struct interMediaTab {
	struct intermedia exprs[10000];
	int ind; //计数指针
};
extern struct interMediaTab imTable;
struct polishNote {//负责每一个表达式的后缀表达式计算与存储
	vector<string> stack;
	bool canBeCalc;//是否可以直接由后缀表达式计算出值（即不含函数调用）
};
struct mipsAsm {//存储mips汇编的结构
	string instr;
	string r1;
	string r2;
	string r3;
};
extern vector<struct mipsAsm*> mipsTable;
struct basicBlock {//基本块的数据结构
	int blockNum;
	vector<int> codes;//基本块中的代码在imTable中的下标
	vector<struct basicBlock*> prevBlocks;
	vector<struct basicBlock*> nextBlocks;
	vector<pair<int, int> > gen;//pair分别为基本块在blockGraph的下标以及中间式在codes的下标
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

//全局共享常量声明
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

//全局共享函数声明
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