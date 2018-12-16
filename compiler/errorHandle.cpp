#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include<stdexcept>
#include "global.h"
using namespace std;

static const string errMsg[] = {
	"此处应为类型标识符", "应是标识符",//0
	"应是=", "常量定义赋值符的右边必须是整数或者字符",//2
	"应为const", "应为;", //4
	"符号表溢出", "变量在同一作用域中重定义", //6
	"数组的维数必须为无符号整数", "数组的维数不能为0", //8
	"应为]", "应为(", //10
	"应为)", "应为{", //12
	"应为}", "应为void", //14
	"应为无符号整数", "应为while", //16
	"应为加法运算符", "语句中只有赋值语句，有/无返回值的函数调用以标识符作为开始", //18
	"printf语句句式不符合规范", "return语句句式不符合规范", //20
	"不允许前导零", "数字过大", //22
	"应为'", "应为加法运算符或乘法运算符或字母或数字", //24
	"已读到文件末尾", "字符串中非法字符", //26
	"非法符号作为开始的字符", "变量未定义",  //28
	"应为数组", "应为有返回值的函数", //30
	"应为普通变量", "函数调用参数类型不匹配", //32
	"函数调用参数个数不匹配", "中间式表溢出", //34
	"条件中的表达式必须为整型", "应为整型", //36
	"赋值操作符两边类型不匹配", "有返回值的函数不应返回空值", //38
	"无返回值的函数不应返回表达式", "有返回值的函数应返回一个值", //40
	"函数名与当前作用域内变量名重复", "不支持同名同返回值的函数重载", //42
	"生成mips代码发生严重错误：未定义的变量", "应为函数", //44
	"生成mips代码发生严重错误：应为数组", "生成mips代码发生严重错误：应为变量", //46



};

void printErrorMsg(int errCode)
{
	cout << "（错误）位于行"<<lastRow<<"列"<<lastLineCnt<<"："<<errMsg[errCode] << endl;
}

void skip(int errCode, string errInfo) //适当修复错误然后跳一定范围
{
	switch (errCode) {
		case 22:
			while (nowCh >= '0' && nowCh <= '9')
				nextCh();
			break;
		case 2:
			if (errInfo == "!=") {
				nowWord.sym = NOTEQL;
				nowWord.str = "!=";
				while (!(nowCh == '+' || nowCh == '-' || (nowCh >= '0' && nowCh <= '9') ||
					     nowCh == '_' || (nowCh >= 'A' && nowCh <= 'Z') || nowCh == ';' ||
					     nowCh == ')' || (nowCh >= 'a' && nowCh <= 'z')))
					nextCh();
			}
			if (errInfo == "constDef")
				while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
					getWord();
			break;
		case 24:
			nowWord.sym = CONCHAR;
			nowWord.str = "'0'";
			while (!(nowCh == ';' || nowCh == ',' || nowCh == '+' ||
					 nowCh == '-' || nowCh == '*' || nowCh == '/'))
				nextCh();
			break;
		case 25:
			nowWord.sym = CONCHAR;
			nowWord.str = "'0'";
			while (!(nowCh == ';' || nowCh == ',' || nowCh == '+' ||
				     nowCh == '-' || nowCh == '*' || nowCh == '/'))
				nextCh();
			break;
		case 26:
			throw exception("异常地读到了文件末尾！"); //中断
			exit(0); //程序非正常退出
			break;
		case 27:
			while (!(nowCh == '"' || nowCh == ';'))
				nextCh();
			break;
		case 28:
			nextCh();
			break;
		case 6:
			throw out_of_range("符号表溢出！"); //中断
			exit(0); //程序非正常退出
			break;
		case 35:
			throw out_of_range("中间式表溢出！"); //中断
			exit(0); //程序非正常退出
			break;
		case 16:
			if(errInfo == "factor")
				while (!(nowWord.sym == PLUS || nowWord.sym == MINUS || nowWord.sym == TIMES ||
					nowWord.sym == DIV || nowWord.sym == RPARENT || nowWord.sym == COMMA ||
					nowWord.sym == SEMICOLON))
					getWord();
			if (errInfo == "for") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			break;
		case 17:
			if (errInfo == "while") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			break;
		case 1:
			if (errInfo == "for") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			if(errInfo == "scanf")
				while (!(nowWord.sym == RPARENT || nowWord.sym == SEMICOLON || nowWord.sym == COMMA))
					getWord();
			if(errInfo == "constDef")
				while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
					getWord();
			if(errInfo == "varDef")
				while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
					getWord();
			if(errInfo == "paramList")
				while (!(nowWord.sym == SEMICOLON || nowWord.sym == COMMA || nowWord.sym == RPARENT))
					getWord();
			break;
		case 19:
			while (!(nowWord.sym == SEMICOLON))
				getWord();
			getWord();
			break;
		case 20:
			while (!(nowWord.sym == SEMICOLON))
				getWord();
			getWord();
			break;
		case 21:
			while (!(nowWord.sym == SEMICOLON))
				getWord();
			getWord();
			break;
		case 3:
			while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
				getWord();
			break;
		case 0:
			if (errInfo == "program") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			if (errInfo == "constDef") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}	
			if(errInfo == "paramList")
				while (!(nowWord.sym == SEMICOLON || nowWord.sym == COMMA || nowWord.sym == RPARENT))
					getWord();
			break;
		case 4:
			while (!(nowWord.sym == SEMICOLON))
				getWord();
			getWord();
			break;
		case 8:
			while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
				getWord();
			break;
		case 9:
			while (!(nowWord.sym == COMMA || nowWord.sym == SEMICOLON))
				getWord();
			break;
		case 15:
			while (!(nowWord.sym == SEMICOLON))
				getWord();
			break;
		case 10:
			if(errInfo == "factor")
				while (!(nowWord.sym == PLUS || nowWord.sym == MINUS || nowWord.sym == TIMES ||
					     nowWord.sym == DIV  || nowWord.sym == RPARENT || nowWord.sym == COMMA ||
					     nowWord.sym == SEMICOLON))
					getWord();
			if (errInfo == "for") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			break;
		case 12:
			if (errInfo == "factor")
				while (!(nowWord.sym == PLUS || nowWord.sym == MINUS || nowWord.sym == TIMES ||
					nowWord.sym == DIV || nowWord.sym == RPARENT || nowWord.sym == COMMA ||
					nowWord.sym == SEMICOLON))
					getWord();
			if (errInfo == "if") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			if (errInfo == "while") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			if(errInfo == "returnFunc")
				while (!(nowWord.sym == LBRACE))
					getWord();
			break;
		case 5:
			if (errInfo == "for") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			break;
		case 18:
			if (errInfo == "for") {
				while (!(nowWord.sym == SEMICOLON))
					getWord();
				getWord();
			}
			break;
		case 14:
			if (errInfo == "program")
				exit(0);
			break;
	}
}

int error(int errCode, string errInfo)
{
	++errorCnt;
	printErrorMsg(errCode);
	skip(errCode, errInfo);
	return 0;
}