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


//������������
int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	init();
	nextCh();//Ԥ�ȶ���һ���ַ��������ʷ��������
	getWord();//Ԥ��һ������
	//�﷨�������ʼ��
	program();
	cout << "�﷨���������" << endl;
	if (errorCnt == 0) {
		//��ӡ�м�ʽ��
		//printImTable();
		//�����Ż�
		//dagOpt();
		//optimize();
		//����mips���
		genMips();
		//��ӡmips����
		printMips();
	}
	cout << "������ɣ������ִ���" << errorCnt << "��" << endl;
	system("pause");
	return 0;
}