#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
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
	"非法符号作为开始的字符"


};

void errorMsg(int errCode)
{
	cout << "（错误）位于行"<<row<<"列"<<lineCnt<<"："<<errMsg[errCode] << endl;
}

void errorFix(int errCode)
{

}

void skip(int errCode)
{

}

void error(int errCode)
{
	errorMsg(errCode);
}