#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

void genInterMedia(interType type, string p1, string p2, string p3, string p4)
{
	imTable.exprs[imTable.ind].type = type;
	imTable.exprs[imTable.ind].expr[0] = p1;
	imTable.exprs[imTable.ind].expr[1] = p2;
	imTable.exprs[imTable.ind].expr[2] = p3;
	imTable.exprs[imTable.ind].expr[3] = p4;
	if(type == CONST || type == ARRAY)
		imTable.exprs[imTable.ind].len = 4;
	if(type == VAR)
		imTable.exprs[imTable.ind].len = 3;

	++imTable.ind;
}