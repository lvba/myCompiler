#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
using namespace std;
//ȫ�ֹ��������壨���붨���ڱ���֮ǰ�ģ�
extern const int maxIdenLen = 50; //�������Ϊ50
extern const int maxSymNum = 5000; //���ű������ɵ���������Ϊ5000
extern const int maxIntermediaNum = 10000; //��Ԫʽ���������10000����Ԫʽ

//ȫ�ֹ����������
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
int staticTempNum = 0;

enum symbol {//ö�����������
	PLUS, MINUS, TIMES, DIV,
	LESS, LESSEQL, GREAT, GREATEQL, NOTEQL, EQUAL,
	CONINT, CONUNSIGN, CONCHAR/*<�ַ�>*/, CONSTR/*<�ַ���>*/,
	SEMICOLON, ASSIGN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
	IDENT,
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY,
	DOSY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY
};
enum interType {//ö���м�ʽ������
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
	string name; //��ʶ����
	int object; //0-������1-������2-����������3-���������4-����
	int type; //0-int��1-char��2-void����typeΪ����ʱ��
	int size; // �����ά�����ߺ����Ĳ�������
	int spaceLv; // �ñ�ʶ������������
	int addr; //���ڱ������������������ջ�д洢��Ԫ��λ�ƣ����������볣�����е�¼��λ�ã����ڳ�����ֱ������ֵ
};
struct symtab {
	struct sym syms[maxSymNum];
	int top; //ջ��ָ��
	vector<int> funcInd; //��ǰ�����ĺ����ڷ��ű��е��±�
};
struct symtab symTable;
struct intermedia {
	string expr[4]; //��Ԫʽ
	interType type; //��Ԫʽ������
	int len;
};
struct interMediaTab {
	struct intermedia exprs[maxIntermediaNum];
	int ind; //����ָ��
};
struct interMediaTab imTable;
struct mipsAsm {//�洢mips���Ľṹ
	string instr = "";
	string r1 = "";
	string r2 = "";
	string r3 = "";
};
vector<struct mipsAsm*> mipsTable;
struct basicBlock {//����������ݽṹ
	int blockNum;
	vector<int> codes;//�������еĴ�����imTable�е��±�
	vector<struct basicBlock*> prevBlocks;
	vector<struct basicBlock*> nextBlocks;
};
typedef struct basicBlock* block;
vector<block> blockGraph;

//ȫ�ֹ���������
extern const int maxLineLen = 200;
extern const int keywordNum = 13;
extern const int maxNumBit = 10;
extern const int maxNum = (1 << 30) - 1;
extern const int stackSize = 8000;
extern const string symStr[] = {//ö�ٶ�Ӧ���ַ�����ʾ
	"PLUS", "MINUS", "TIMES", "DIV",
	"LESS", "LESSEQL", "GREAT", "GREATEQL", "NOTEQL", "EQUAL",
	"CONINT", "CONUNSIGN", "CONCHAR"/*<�ַ�>*/, "CONSTR"/*<�ַ���>*/,
	"SEMICOLON", "ASSIGN", "COMMA", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE",
	"IDENT",
	"CONSTSY", "INTSY", "CHARSY", "VOIDSY", "MAINSY", "IFSY", "ELSESY",
	"DOSY", "WHILESY", "FORSY", "SCANFSY", "PRINTFSY", "RETURNSY"
};
extern const string keyword[13] = {//�ؼ��ּ�
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
extern const symbol keySymbol[13] = {//�ؼ��ֶ�Ӧ�������
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
//ȫ�ֹ���������
