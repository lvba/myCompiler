#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static int labelNum = 0;

void genInterMedia(interType type, string p1, string p2, string p3, string p4)
{
	if (imTable.ind >= maxIntermediaNum)//��Ԫʽ�����
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

string genTemp()//������ʱ������
{
	string temp = "_TEMP" + to_string(staticTempNum++);
	return temp;
}

void resetTemp()
{
	staticTempNum = 0;
}

void printImTable()
{
	for (int i = 0; i < imTable.ind; ++i) {
		//if (imTable.exprs[i].type == PUSH) {
			cout << imTable.exprs[i].expr[0] << " "
				<< imTable.exprs[i].expr[1] << " "
				<< imTable.exprs[i].expr[2] << " "
				<< imTable.exprs[i].expr[3] << endl;
		//}	
	}	
}

void printEachIm(int i)
{
	cout << imTable.exprs[i].expr[0] << " "
		<< imTable.exprs[i].expr[1] << " "
		<< imTable.exprs[i].expr[2] << " "
		<< imTable.exprs[i].expr[3] << endl;
}

bool insertSymTable(string name, int obj, int type, int size, int spLv, int addr)
{
	if (symTable.top >= maxSymNum) { //���ű����
		error(6, "");
		return false;
	}
	else {
		//�����Ƿ��б����ض���
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
		//���Һ����ڵı����Ƿ��뺯��ͬ��
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
		if (obj == 4) {
			for (int i = 0; i < symTable.funcInd.size(); ++i) {
				if (symTable.syms[symTable.funcInd[i]].name == name &&
					symTable.syms[symTable.funcInd[i]].type == type) {
					error(43, "");
					break;
				}
			}
			symTable.funcInd.push_back(symTable.top);
		}		
		++symTable.top;
		return true;
	}
}

int searchSymTable(string name, int nowLevel) //�����ڷ��ű��е��±�
{
	int ind = symTable.top - 1;
	if (ind < 0)
		return -1;
	//�鵱ǰ������
	while (symTable.syms[ind].spaceLv == nowLevel) {
		if (symTable.syms[ind].name == name)
			return ind;
		--ind;
	}
	//��ǰ������鲻�����ȫ��������
	ind = 0;
	while (symTable.syms[ind].spaceLv == 0) {
		if (symTable.syms[ind].name == name)
			return ind;
		++ind;
	}
	//�麯���±��
	for (int i = 0; i < symTable.funcInd.size(); ++i) {
		if (symTable.syms[symTable.funcInd[i]].name == name)
			return symTable.funcInd[i];
	}
	return -1;
}

int searchWithLevel(string name, int object, int nowLevel)//��ָ��level�ı���
{
	if (nowLevel == 0) {
		for (int i = 0; symTable.syms[i + 1].spaceLv == 0; ++i) {
			if (symTable.syms[i + 1].spaceLv != 0)
				return -1;
			if (symTable.syms[i].name == name && symTable.syms[i].object == object)
				return i;
		}
	} else {
		int index = symTable.funcInd[nowLevel - 1] + 1;
		while (index < symTable.top && symTable.syms[index].spaceLv == nowLevel) {
			if (symTable.syms[index].name == name && symTable.syms[index].object == object)
				return index;
			++index;
		}
	}
	return -1;
}

int searchAllLevel(string name, int nowLevel)
{
	//�鵱ǰ������
	if (nowLevel != 0) {
		int index = symTable.funcInd[nowLevel - 1] + 1;
		while (index < symTable.top && symTable.syms[index].spaceLv == nowLevel) {
			if (symTable.syms[index].name == name)
				return index;
			++index;
		}
	}
	//��ǰ������δ�鵽���ȫ��������
	for (int i = 0; symTable.syms[i + 1].spaceLv == 0; ++i) {
		if (symTable.syms[i + 1].spaceLv != 0)
			return -1;
		if (symTable.syms[i].name == name)
			return i;
	}
	return -1;
}
