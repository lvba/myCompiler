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
extern int nowLevel;
extern int addLevel;

extern enum symbol {//枚举所有类别码
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<字符>*/, CONSTR/*<字符串>*/,
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
	string name; //标识符名
	int object; //0-常量，1-变量，2-函数变量，3-数组变量，4-函数
	int type; //0-int，1-char，2-void（当type为函数时）
	int size; // 数组的维数或者函数的参数个数
	int spaceLv; // 该标识符的作用域层次
	int addr; //对于变量则填入变量在运行栈中存储单元的位移，常量则填入常量表中登录的位置，对于常量则直接填入值
};
struct symtab {
	struct sym syms[5000];
	int top; //栈顶指针
	int funcNum; //当前声明的函数个数
};
extern struct symtab symTable;

//全局共享常量声明
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

//全局共享函数声明
void nextCh();
void getWord();
void error(int errCode);
int getInt();
bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr);