#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include "global.h"
using namespace std;

static string line;
static int lineLen;

void printWordInfo() {
	/*if (nowWord.sym == CONINT || nowWord.sym == CONUNSIGN)
		cout << symStr[nowWord.sym] << " " << nowWord.num << endl;
	else
		cout << symStr[nowWord.sym] << " " << nowWord.str << endl;*/
}

//读下一个字符存入nowCh中
void nextCh()
{
	if (lineCnt == 0) {
		getline(infile, line);
		lastRow = row;
		++row;
		lineLen = line.size();
	}
	lastLineCnt = lineCnt;
	nowCh = line[lineCnt++];
	if (lineCnt == lineLen) {
		lastLineCnt = lineCnt;
		lineCnt = 0;
	}	
}

int parseUnsigned()//处理无符号整数，且不允许前导零
{
	int value = 0, k = 0;
	bool isZero = nowCh == '0' ? true : false;
	while (nowCh >= '0' && nowCh <= '9') {
		value = value * 10 + (nowCh - '0');
		++k;
		if (k == 2 && isZero) { //不允许前导零！
			error(22, "");
			value = 0;
			return value;
		}
		nextCh();
	}
	if (k > maxNumBit || value > maxNum) {
		error(23, ""); //错误：数字过大
		value = 0;
		k = 0;
	}
	return value;
}

int bsearch(int low, int high, string target)
{
	int mid = 0;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (keyword[mid] == target)
			return mid;
		if (keyword[mid] < target)
			low = mid + 1;
		else {
			if (keyword[mid] > target)
				high = mid - 1;
			else
				return mid;
		}
	}
	return -1;
}

//读单词，并在struct结构中存储当前单词的信息
void getWord()
{
	while (nowCh == ' ')
		nextCh();
	if ((nowCh >= 'A' && nowCh <= 'Z') || (nowCh >= 'a' && nowCh <= 'z') || nowCh == '_') {
		//标识符
		nowWord.str = "";
		int temp = 0;
		while (((nowCh >= 'A' && nowCh <= 'Z') || (nowCh >= 'a' && nowCh <= 'z')
			|| nowCh == '_' || (nowCh >= '0' && nowCh <= '9')) && temp < maxIdenLen) {
			nowWord.str += nowCh;
			nextCh();
			++temp;
		}
		//二分搜索查找关键字
		int low = 0, high = keywordNum - 1;
		int isFound = bsearch(low, high, nowWord.str);
		if (isFound != -1)
			nowWord.sym = keySymbol[isFound];
		else
			nowWord.sym = IDENT;
		printWordInfo();
		return;
	}
	if (nowCh >= '0' && nowCh <= '9') {//以数字开头的单词
		nowWord.sym = CONUNSIGN;
		nowWord.num = parseUnsigned();
		printWordInfo();
		return;
	}
	if (nowCh == '+') {
		nowWord.sym = PLUS;
		nowWord.str = "+";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '-') {
		nowWord.sym = MINUS;
		nowWord.str = "-";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '*') {
		nowWord.sym = TIMES;
		nowWord.str = "*";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '/') {
		nowWord.sym = DIV;
		nowWord.str = "/";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '<') { //<=或<
		nextCh();
		if (nowCh == '=') {
			nowWord.sym = LESSEQL;
			nowWord.str = "<=";
			nextCh();
			printWordInfo();
			return;
		}
		else {
			nowWord.sym = LESS;
			nowWord.str = "<";
			printWordInfo();
			return;
		}
	}
	if (nowCh == '>') { //>=或>
		nextCh();
		if (nowCh == '=') {
			nowWord.sym = GREATEQL;
			nowWord.str = ">=";
			nextCh();
			printWordInfo();
			return;
		}
		else {
			nowWord.sym = GREAT;
			nowWord.str = ">";
			printWordInfo();
			return;
		}
	}
	if (nowCh == '!') {//!=
		nextCh();
		if (nowCh == '=') {
			nowWord.sym = NOTEQL;
			nowWord.str = "!=";
			nextCh();
			printWordInfo();
			return;
		}
		else {
			error(2, "!=");//错误：应为=
			printWordInfo();
			return;
		}			
	}
	if (nowCh == '=') {
		nextCh();
		if (nowCh == '=') {
			nowWord.sym = EQUAL;
			nowWord.str = "==";
			nextCh();
			printWordInfo();
			return;
		}
		else {
			nowWord.sym = ASSIGN;
			nowWord.str = "=";
			printWordInfo();
			return;
		}
	}
	if (nowCh == '\'') {//字符
		nextCh();
		char temp;
		if (nowCh == '+' || nowCh == '-' || nowCh == '*' || nowCh == '/' || nowCh == '_' ||
			(nowCh >= 'A' && nowCh <= 'Z') || (nowCh >= 'a' && nowCh <= 'z') ||
			(nowCh >= '0' && nowCh <= '9')) {
			temp = nowCh;
			nextCh();
			if (nowCh == '\'') {
				nowWord.str = "'";
				nowWord.sym = CONCHAR;
				nowWord.str += temp;
				nowWord.str += '\'';
				nextCh();
				printWordInfo();
				return;
			}
			else {
				error(24, "");//错误：应为'
				return;
			}
		}
		else {
			error(25, "");//错误：应为加法，乘法运算符以及字母数字
			return;
		}
	}
	if (nowCh == '"') {//字符串
		nextCh();
		string s = "";
		while (nowCh != '"') {
			//判断是否合法字符
			if (nowCh == 32 || nowCh == 33 || (nowCh >= 35 && nowCh <= 126)) {
				s += nowCh;
				nextCh();
				if (infile.eof() && lineCnt == lineLen - 1) {
					error(26, "");//错误：已读到文件末尾
				}
			}
			else {
				error(27, "");//错误：字符串中非法字符
				s = "wrong string";
				break;
			}
		}
		nowWord.sym = CONSTR;
		nowWord.str = s;
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == ';') {
		nowWord.sym = SEMICOLON;
		nowWord.str = ";";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == ',') {
		nowWord.sym = COMMA;
		nowWord.str = ",";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '(') {
		nowWord.sym = LPARENT;
		nowWord.str = "(";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == ')') {
		nowWord.sym = RPARENT;
		nowWord.str = ")";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '[') {
		nowWord.sym = LBRACK;
		nowWord.str = "[";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == ']') {
		nowWord.sym = RBRACK;
		nowWord.str = "]";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '{') {
		nowWord.sym = LBRACE;
		nowWord.str = "{";
		nextCh();
		printWordInfo();
		return;
	}
	if (nowCh == '}') {
		nowWord.sym = RBRACE;
		nowWord.str = "}";
		nextCh();
		printWordInfo();
		return;
	}
	//不是上述合法开始符的情况，报错
	error(28, "");//错误：非法的符号开始字符
}

int getInt()
{
	int ret = -99999998;
	getWord();
	if (nowWord.sym == MINUS) {
		getWord();
		if (nowWord.sym != CONUNSIGN) {
			error(3, "");
			return ret;
		} else {
			ret = -(nowWord.num);
			return ret;
		}
	}
	if (nowWord.sym == PLUS) {
		getWord();
		if (nowWord.sym != CONUNSIGN) {
			error(3, "");
			return -ret;
		} else {
			ret = nowWord.num;
			return ret;
		}
	}
	if (nowWord.sym == CONUNSIGN)
		return nowWord.num;
	error(3, "");
	return ret;
}


/*int main()
{
	infile.open("E:\\16061056_test.txt");//此为待处理代码的存放文件
	ofile.open("E:\\word_parse_result.txt");
	nextCh();
	while (!(infile.eof() && lineCnt >= lineLen - 1)) {
		getWord();
	}
	if (lineLen == 1)
		getWord();
	else {
		getWord();
		getWord();
	}
	cout << "词法分析处理完毕，请参见当前文件夹下的result.txt文件" << endl;
	return 0;
}*/