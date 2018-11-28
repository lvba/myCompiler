#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

void init()
{
	symTable.top = 0;
	imTable.ind = 0;
}