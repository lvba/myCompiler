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
}

//������������
int main()
{
	getline(cin, filePath);
	infile.open(filePath);
	nextCh();//Ԥ�ȶ���һ���ַ��������ʷ��������
	getWord();//Ԥ��һ������
	//�﷨�������ʼ��
	program();
	cout << "�﷨���������" << endl;
	if (errorCnt == 0) {
		//��ӡ�м�ʽ��
		//printImTable();
		//����mips���
		genMips();
		//��ӡmips����
		printMips();
	}
	cout << "������ɣ������ִ���" << errorCnt << "��" << endl;
	return 0;
}