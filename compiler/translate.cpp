#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static int labelNum = 0;
static int tempNum = 0;

void genInterMedia(interType type, string p1, string p2, string p3, string p4)
{
	imTable.exprs[imTable.ind].type = type;
	imTable.exprs[imTable.ind].expr[0] = p1;
	imTable.exprs[imTable.ind].expr[1] = p2;
	imTable.exprs[imTable.ind].expr[2] = p3;
	imTable.exprs[imTable.ind].expr[3] = p4;
	if(type == CONST || type == ARRAY)
		imTable.exprs[imTable.ind].len = 4;
	if(type == VAR || type == FUNC || type == PARAM)
		imTable.exprs[imTable.ind].len = 3;
	if (type == BZ || type == PUSH || type == CALL)
		imTable.exprs[imTable.ind].len = 2;

	++imTable.ind;
}

string genLabel()
{
	string label = "LABEL" + to_string(labelNum++);
	return label;
}

string genTemp()//生成临时变量名
{
	string temp = "_TEMP" + to_string(tempNum++);
	return temp;
}

void resetTemp()
{
	tempNum = 0;
}

void printImTable()
{
	for (int i = 0; i < imTable.ind; ++i) {
		cout << imTable.exprs[i].expr[0] << " "
			<< imTable.exprs[i].expr[1] << " "
			<< imTable.exprs[i].expr[2] << " "
			<< imTable.exprs[i].expr[3] << endl;
	}	
}