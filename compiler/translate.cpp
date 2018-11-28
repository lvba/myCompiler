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
	if (imTable.ind >= maxIntermediaNum)//四元式表溢出
		error(35, "");
	imTable.exprs[imTable.ind].type = type;
	imTable.exprs[imTable.ind].expr[0] = p1;
	imTable.exprs[imTable.ind].expr[1] = p2;
	imTable.exprs[imTable.ind].expr[2] = p3;
	imTable.exprs[imTable.ind].expr[3] = p4;
	int len = 0;
	if (p1 != "") ++len;
	if (p2 != "") ++len;
	if (p3 != "") ++len;
	if (p4 != "") ++len;
	imTable.exprs[imTable.ind].len = len;

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

bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr)
{
	if (symTable.top >= maxSymNum) { //符号表溢出
		error(6, "");
		return false;
	}
	else {
		//查找是否有变量重定义
		if (symTable.top != 0)
			if (symTable.syms[symTable.top - 1].spaceLv == spLv) {
				int cnt = symTable.top - 1;
				while (symTable.syms[cnt].spaceLv == spLv) {
					if (symTable.syms[cnt].name == name) {
						error(7, "");
						return false;
					}
					--cnt;
				}
			}
		//查找函数内的变量是否与函数同名
		if (spLv != 0 && obj != 4) {
			if (name == symTable.syms[symTable.funcInd[symTable.funcInd.size() - 1]].name) {
				error(7, "");
				return false;
			}
		}
		symTable.syms[symTable.top].name = name;
		symTable.syms[symTable.top].object = obj;
		symTable.syms[symTable.top].type = type;
		symTable.syms[symTable.top].size = size;
		symTable.syms[symTable.top].spaceLv = spLv;
		symTable.syms[symTable.top].addr = addr;
		if (obj == 4)
			symTable.funcInd.push_back(symTable.top);
		++symTable.top;
		return true;
	}
}

int searchSymTable(string name, int object) //返回在符号表中的下标
{
	int ind = symTable.top - 1;
	if (ind < 0)
		return -1;
	//查当前作用域
	while (symTable.syms[ind].spaceLv == nowLevel) {
		if (symTable.syms[ind].name == name && symTable.syms[ind].object == object)
			return ind;
		--ind;
	}
	//当前作用域查不到则查全局作用域
	ind = 0;
	while (symTable.syms[ind].spaceLv == 0) {
		if (symTable.syms[ind].name == name && symTable.syms[ind].object == object)
			return ind;
		++ind;
	}
	//查函数下标表
	for (int i = 0; i < symTable.funcInd.size(); ++i) {
		if (symTable.syms[symTable.funcInd[i]].name == name &&
			symTable.syms[symTable.funcInd[i]].object == object)
			return symTable.funcInd[i];
	}
	return -1;
}