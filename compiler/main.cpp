#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static string filePath;
void init()
{
	symTable.top = 0;
	imTable.ind = 0;
	for (int i = 0; i < 10; ++i) {
		string regName = "$t";
		regName += to_string(i);
		regPool.push_back(make_pair(regName, ""));
	}
	for (int i = 0; i < 250; ++i) 
		tempRegTab.push_back("");
	
}


//编译器主函数
int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	init();
	nextCh();//预先读入一个字符以启动词法处理程序
	getWord();//预读一个单词
	//语法处理程序开始！
	program();
	cout << "语法处理结束！" << endl;
	if (errorCnt == 0) {
		//打印中间式表
		//printImTable();
		//进行优化
		//dagOpt();
		//optimize();
		//生成mips汇编
		genMips();
		//打印mips代码
		printMips();
	}
	cout << "编译完成，共发现错误" << errorCnt << "个" << endl;
	system("pause");
	return 0;
}