#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
using namespace std;
//全局共享常量定义（必须定义在变量之前的）
extern const int maxIdenLen = 50; //变量名最长为50
extern const int maxSymNum = 5000; //符号表能容纳的最大符号量为5000
extern const int maxIntermediaNum = 10000; //四元式表最多容纳10000条四元式

//全局共享变量定义
ifstream infile;
ofstream ofile;
int lineCnt = 0;
int row = 0;
int lastLineCnt = 0;
int lastRow = 0;
int nowLevel = 0;
int addLevel = 0;
char nowCh;
int errorCnt = 0;

enum symbol {//枚举所有类别码
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<字符>*/, CONSTR/*<字符串>*/,
	SEMICOLON, ASSIGN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
	IDENT,
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY,
	DOSY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY
};
enum interType {//枚举中间式的种类
	VAR, CONST, VARASS, FUNC, PARAM, PUSH, ARRAY,
	CALL, RET, COMPARE, GOTO, BNZ, BZ, ARRASS
};
struct nowword {
	int num;
	string str;
	symbol sym;
};
struct nowword nowWord;
struct sym {
	string name; //标识符名
	int object; //0-常量，1-变量，2-函数变量，3-数组变量，4-函数
	int type; //0-int，1-char，2-void（当type为函数时）
	int size; // 数组的维数或者函数的参数个数
	int spaceLv; // 该标识符的作用域层次
	int addr; //对于变量则填入变量在运行栈中存储单元的位移，常量则填入常量表中登录的位置，对于常量则直接填入值
};
struct symtab {
	struct sym syms[maxSymNum];
	int top; //栈顶指针
	vector<int> funcInd; //当前声明的函数在符号表中的下标
};
struct symtab symTable;
struct intermedia {
	string expr[4]; //四元式
	interType type; //四元式的类型
	int len;
};
struct interMediaTab {
	struct intermedia exprs[maxIntermediaNum];
	int ind; //计数指针
};
struct interMediaTab imTable;
struct mipsAsm {//存储mips汇编的结构
	string instr = "";
	char type = 'N';//R, I, J
	string r1 = "";
	string r2 = "";
	string r3 = "";
};
vector<struct mipsAsm> mipsTable;

//全局共享常量定义
extern const int maxLineLen = 200;
extern const int keywordNum = 13;
extern const int maxNumBit = 10;
extern const int maxNum = (1 << 30) - 1;
extern const int stackSize = 8000;
extern const string symStr[] = {//枚举对应的字符串表示
	"PLUS", "MINUS", "TIMES", "DIV",
	"LESS", "LESSEQL", "GREAT", "GREATEQL", "NOTEQL", "EQUAL",
	"CONINT", "CONUNSIGN", "CONCHAR"/*<字符>*/, "CONSTR"/*<字符串>*/,
	"SEMICOLON", "ASSIGN", "COMMA", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE",
	"IDENT",
	"CONSTSY", "INTSY", "CHARSY", "VOIDSY", "MAINSY", "IFSY", "ELSESY",
	"DOSY", "WHILESY", "FORSY", "SCANFSY", "PRINTFSY", "RETURNSY"
};
extern const string keyword[13] = {//关键字集
	"char",
	"const",
	"do",
	"else",
	"for",
	"if",
	"int",
	"main",
	"printf",
	"return",
	"scanf",
	"void",
	"while"
};
extern const symbol keySymbol[13] = {//关键字对应的类别码
	CHARSY,
	CONSTSY,
	DOSY,
	ELSESY,
	FORSY,
	IFSY,
	INTSY,
	MAINSY,
	PRINTFSY,
	RETURNSY,
	SCANFSY,
	VOIDSY,
	WHILESY,
};

//全局共享函数定义
