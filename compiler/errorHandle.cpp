#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static const string errMsg[] = {
	"�˴�ӦΪ���ͱ�ʶ��", "Ӧ�Ǳ�ʶ��",//0
	"Ӧ��=", "�������帳ֵ�����ұ߱��������������ַ�",//2
	"ӦΪconst", "ӦΪ;", //4
	"���ű����", "������ͬһ���������ض���", //6
	"�����ά������Ϊ�޷�������", "�����ά������Ϊ0", //8
	"ӦΪ]", "ӦΪ(", //10
	"ӦΪ)", "ӦΪ{", //12
	"ӦΪ}", "ӦΪvoid", //14
	"ӦΪ�޷�������", "ӦΪwhile", //16
	"ӦΪ�ӷ������", "�����ֻ�и�ֵ��䣬��/�޷���ֵ�ĺ��������Ա�ʶ����Ϊ��ʼ", //18
	"printf����ʽ�����Ϲ淶", "return����ʽ�����Ϲ淶", //20
	"������ǰ����", "���ֹ���", //22
	"ӦΪ'", "ӦΪ�ӷ��������˷����������ĸ������", //24
	"�Ѷ����ļ�ĩβ", "�ַ����зǷ��ַ�", //26
	"�Ƿ�������Ϊ��ʼ���ַ�"


};

void errorMsg(int errCode)
{
	cout << "������λ����"<<row<<"��"<<lineCnt<<"��"<<errMsg[errCode] << endl;
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