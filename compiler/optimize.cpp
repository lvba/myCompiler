#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static int blockNum = 0;

void divideBlocks() //���ֻ�����
{
	for (int i = 0; i < imTable.ind; ++i) {
		if (imTable.exprs[i].type == VAR || imTable.exprs[i].type == CONST ||
			imTable.exprs[i].type == ARRAY) {
			continue;
		}
	}

}