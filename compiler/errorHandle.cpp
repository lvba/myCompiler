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
	"�Ƿ�������Ϊ��ʼ���ַ�", "����δ����",  //28
	"ӦΪ����", "ӦΪ�з���ֵ�ĺ���", //30
	"ӦΪ��ͨ����", "�������ò������Ͳ�ƥ��", //32
	"�������ò���������ƥ��", "�м�ʽ�����", //34
	"�����еı��ʽ����Ϊ����", "ӦΪ����", //36
	"��ֵ�������������Ͳ�ƥ��", "�з���ֵ�ĺ�����Ӧ���ؿ�ֵ", //38
	"�޷���ֵ�ĺ�����Ӧ���ر��ʽ", "�з���ֵ�ĺ���Ӧ����һ��ֵ", //40
	"�������뵱ǰ�������ڱ������ظ�", "��֧��ͬ��ͬ����ֵ�ĺ�������", //42
	"����mips���뷢�����ش���δ����ı���", "ӦΪ����", //44
	"����mips���뷢�����ش���ӦΪ����", "����mips���뷢�����ش���ӦΪ����", //46



};

void printErrorMsg(int errCode)
{
	cout << "������λ����"<<lastRow<<"��"<<lastLineCnt<<"��"<<errMsg[errCode] << endl;
}

void skip(int errCode, string errInfo) //�ʵ��޸�����Ȼ����һ����Χ
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
			throw exception("�쳣�ض������ļ�ĩβ��"); //�ж�
			exit(0); //����������˳�
			break;
		case 27:
			while (!(nowCh == '"' || nowCh == ';'))
				nextCh();
			break;
		case 28:
			nextCh();
			break;
		case 6:
			throw out_of_range("���ű������"); //�ж�
			exit(0); //����������˳�
			break;
		case 35:
			throw out_of_range("�м�ʽ�������"); //�ж�
			exit(0); //����������˳�
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