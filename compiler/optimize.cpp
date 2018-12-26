#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<bitset>
#include "global.h"
using namespace std;

static int blockNum = 0;
static int stNodeNum = 0;

void divideBlocks() //划分基本块
{
	//寻找入口语句
	vector<int> entryStatement;
	for (int i = 0; i < imTable.ind; ++i) {
		if (i == 0) {
			entryStatement.push_back(i);
			continue;
		}			
		if(imTable.exprs[i].type == LABEL || imTable.exprs[i].type == FUNC)
			entryStatement.push_back(i);
		if (imTable.exprs[i].type == GOTO || imTable.exprs[i].type == COMPARE ||
			imTable.exprs[i].type == RET) {
			if(i + 1 < imTable.ind)
				entryStatement.push_back(i + 1);
		}
		if (imTable.exprs[i].type == CALL && imTable.exprs[i].expr[1] != "scanf" &&
			imTable.exprs[i].expr[1] != "printf") {
			if (i + 1 < imTable.ind)
				entryStatement.push_back(i + 1);
		}
	}
	sort(entryStatement.begin(), entryStatement.end());
	entryStatement.erase(unique(entryStatement.begin(), entryStatement.end()), entryStatement.end());
	entryStatement.push_back(imTable.ind);
	//划分基本块
	for (int i = 0; i < entryStatement.size() - 1; ++i) {
		block b = new struct basicBlock;
		if (imTable.exprs[entryStatement[i]].type == FUNC && imTable.exprs[entryStatement[i]].expr[1] == "main")
			b->blockNum = 0;
		else
			b->blockNum = i + 1;
		for (int cnt = entryStatement[i]; cnt < entryStatement[i + 1]; ++cnt)
			b->codes.push_back(cnt);
		blockGraph.push_back(b);
	}
	//连接基本块
	for (int i = 0; i < blockGraph.size(); ++i) {
		struct intermedia im = imTable.exprs[blockGraph[i]->codes[blockGraph[i]->codes.size() - 1]];
		string label, funcName;
		int nowInd = blockGraph[i]->codes[blockGraph[i]->codes.size() - 1];
		switch (im.type) {
			case GOTO:
				label = im.expr[1];
				for (int j = 0; j < blockGraph.size(); ++j) {
					if (imTable.exprs[blockGraph[j]->codes[0]].type == LABEL &&
						imTable.exprs[blockGraph[j]->codes[0]].expr[0] == label) {
						blockGraph[i]->nextBlocks.push_back(blockGraph[j]);
						blockGraph[j]->prevBlocks.push_back(blockGraph[i]);
						break;
					}
				}
				break;
			case COMPARE:
				while (imTable.exprs[nowInd].type != BZ && imTable.exprs[nowInd].type != BNZ)
					--nowInd;
				label = imTable.exprs[nowInd].expr[1];
				for (int j = 0; j < blockGraph.size(); ++j) {
					if (imTable.exprs[blockGraph[j]->codes[0]].type == LABEL &&
						imTable.exprs[blockGraph[j]->codes[0]].expr[0] == label) {
						blockGraph[i]->nextBlocks.push_back(blockGraph[j]);
						blockGraph[j]->prevBlocks.push_back(blockGraph[i]);
						break;
					}
				}
				if (i + 1 < blockGraph.size()) {
					blockGraph[i]->nextBlocks.push_back(blockGraph[i + 1]);
					blockGraph[i + 1]->prevBlocks.push_back(blockGraph[i]);
				}
				break;
			case RET:
				while (imTable.exprs[nowInd].type != FUNC)
					--nowInd;
				funcName = imTable.exprs[nowInd].expr[1];
				for (int j = 0; j < blockGraph.size(); ++j) {
					if (imTable.exprs[blockGraph[j]->codes[blockGraph[j]->codes.size() - 1]].type == CALL &&
						imTable.exprs[blockGraph[j]->codes[blockGraph[j]->codes.size() - 1]].expr[1] == funcName) {
						if (j + 1 < blockGraph.size()) {
							blockGraph[i]->nextBlocks.push_back(blockGraph[j + 1]);
							blockGraph[j + 1]->prevBlocks.push_back(blockGraph[i]);
						}
					}
				}
				break;
			case CALL:
				funcName = im.expr[1];
				for (int j = 0; j < blockGraph.size(); ++j) {
					if (imTable.exprs[blockGraph[j]->codes[0]].type == FUNC &&
						imTable.exprs[blockGraph[j]->codes[0]].expr[1] == funcName) {
						blockGraph[i]->nextBlocks.push_back(blockGraph[j]);
						blockGraph[j]->prevBlocks.push_back(blockGraph[i]);
						break;
					}
				}
				break;
			default:
				//cout << "在划分基本块时发生错误" << endl;
				//cout << im.expr[0] << ' ' << im.expr[1] << ' ' << im.expr[2] << ' ' << im.expr[3] << endl;
				break;
		}
	}
}

int findNode(vector<pair<string, int> > nodeTab, string name)
{
	for (int i = 0; i < nodeTab.size(); ++i) {
		if (name == nodeTab[i].first)
			return i;
	}
	return -1;
}

int findOpNode(string name, int lNodeNum, int rNodeNum)
{
	for (int i = 0; i < dagGraph.size(); ++i) {
		if (dagGraph[i]->name == name && dagGraph[i]->lchild != NULL && dagGraph[i]->rchild != NULL) {
			if (dagGraph[i]->lchild->nodeNum == lNodeNum && dagGraph[i]->rchild->nodeNum == rNodeNum)
				return i;
		}
	}
	return -1;
}

int genNewNode(string name, dagNode lchild, dagNode rchild, dagNode parent, int isLeaf)
{
	dagNode newNode = new struct node;
	newNode->name = name;
	newNode->nodeNum = stNodeNum++;
	newNode->lchild = lchild;
	newNode->rchild = rchild;
	newNode->isLeaf = isLeaf;
	newNode->hasInQue = -1;
	dagGraph.push_back(newNode);
	//为左右子节点添加父节点
	if (lchild != NULL && lchild->isLeaf == 0) {
		int flag = 0;
		for (int i = 0; i < lchild->parents.size(); ++i) {
			if (lchild->parents[i]->nodeNum == newNode->nodeNum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0)
			lchild->parents.push_back(newNode);
	}
	if (rchild != NULL && rchild->isLeaf == 0) {
		int flag = 0;
		for (int i = 0; i < rchild->parents.size(); ++i) {
			if (rchild->parents[i]->nodeNum == newNode->nodeNum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0)
			rchild->parents.push_back(newNode);
	}
	return newNode->nodeNum;
}

bool hasAllInQue()
{
	for (int i = 0; i < dagGraph.size(); ++i) {
		if (dagGraph[i]->isLeaf == 0) {
			if (dagGraph[i]->hasInQue == -1)
				return false;
		}
	}
	return true;
}

void dagOpt()
{
	for (int i = 0; i < blockGraph.size(); ++i) {
		vector<pair<string, int> > nodeTab;
		//建立dag图
		for (int j = 0; j < blockGraph[i]->codes.size(); ++j) {
			struct intermedia outim = imTable.exprs[blockGraph[i]->codes[j]];
			if (outim.type == VARASS || outim.type == ARRASS) {
				struct intermedia* im = new struct intermedia;
				if (outim.type == ARRASS && outim.expr[1] == "[]") { //z [] x = y
					im->type = ARRASS;
					im->len = outim.len;
					im->expr[0] = outim.expr[0];
					im->expr[1] = outim.expr[2];
					im->expr[2] = "[]=";
					im->expr[3] = outim.expr[3];
				} else {
					im->type = outim.type;
					im->len = outim.len;
					im->expr[0] = outim.expr[0];
					im->expr[1] = outim.expr[1];
					im->expr[2] = outim.expr[2];
					im->expr[3] = outim.expr[3];
				}
				if (im->expr[3] != "") { //z = x op y
					int find_x = findNode(nodeTab, im->expr[1]);
					int nodeNum_x = -1;
					if(find_x != -1)
						nodeNum_x = nodeTab[find_x].second;
					else {//未找到x，则在dag图中新建一个叶结点
						nodeNum_x = genNewNode(im->expr[1], NULL, NULL, NULL, 1);
						nodeTab.push_back(make_pair(im->expr[1], nodeNum_x));
					}
					int find_y = findNode(nodeTab, im->expr[3]);
					int nodeNum_y = -1;
					if (find_y != -1)
						nodeNum_y = nodeTab[find_y].second;
					else {//未找到y，则在dag图中新建一个叶结点
						nodeNum_y = genNewNode(im->expr[3], NULL, NULL, NULL, 1);
						nodeTab.push_back(make_pair(im->expr[3], nodeNum_y));
					}
					int find_op = findOpNode(im->expr[2], nodeNum_x, nodeNum_y);
					int nodeNum_op = -1;
					if (find_op != -1)
						nodeNum_op = dagGraph[find_op]->nodeNum;
					else
						nodeNum_op = genNewNode(im->expr[2], dagGraph[nodeNum_x], dagGraph[nodeNum_y], NULL, 0);
					int find_z = findNode(nodeTab, im->expr[0]);
					if (find_z != -1)
						nodeTab[find_z].second = nodeNum_op;
					else
						nodeTab.push_back(make_pair(im->expr[0], nodeNum_op));
				} else { //z = x
					int find_x = findNode(nodeTab, im->expr[1]);
					int nodeNum_x = -1;
					if (find_x != -1)
						nodeNum_x = nodeTab[find_x].second;
					else {//未找到x，则在dag图中新建一个叶结点
						nodeNum_x = genNewNode(im->expr[1], NULL, NULL, NULL, 1);
						nodeTab.push_back(make_pair(im->expr[1], nodeNum_x));
					}
					int find_z = findNode(nodeTab, im->expr[0]);
					if (find_z != -1)
						nodeTab[find_z].second = nodeNum_x;
					else
						nodeTab.push_back(make_pair(im->expr[0], nodeNum_x));
				}
			}
		}
		//根据dag图导出中间节点队列
		vector<dagNode> opQueue;
		while (!hasAllInQue()) {
			for (int i = 0; i < dagGraph.size(); ++i) {
				if (dagGraph[i]->isLeaf == 0 && dagGraph[i]->hasInQue == -1) {
					if (dagGraph[i]->parents.size() == 0) { //没有父节点的中间节点
						dagGraph[i]->hasInQue = 1;
						opQueue.push_back(dagGraph[i]);
						break;
					}
					int flag = 0;
					for (int j = 0; j < dagGraph[i]->parents.size(); ++j) {
						if (dagGraph[i]->parents[j]->hasInQue == -1) {
							flag = 1;
							break;
						}
					}
					if (flag == 0) {
						dagGraph[i]->hasInQue = 1;
						opQueue.push_back(dagGraph[i]);
						break;
					}
				}
			}
			//第(4)步
			dagNode dg = opQueue.back();
			while (dg->lchild != NULL) {
				int isContinue = 0;
				dg = dg->lchild;
				if (dg->isLeaf == 0 && dg->hasInQue == -1) {
					if (dg->parents.size() == 0) { //没有父节点的中间节点
						dg->hasInQue = 1;
						opQueue.push_back(dg);
						isContinue = 1;
					} else {
						int flag = 0;
						for (int j = 0; j < dg->parents.size(); ++j) 
							if (dg->parents[j]->hasInQue == -1) 
								flag = 1;
						if (flag == 0) {
							dg->hasInQue = 1;
							opQueue.push_back(dg);
							isContinue = 1;
						}
					}				
				}
				if (isContinue == 0)
					break;
			}
		}
		//由队列整理中间代码

		//dag图清零
		dagGraph.clear();
	}
	
}

//临时寄存器池分配
inline void clearPool()
{
	for (int i = 0; i < 10; ++i)
		regPool[i].second = "";
}

int getFromPool(string varName, vector<int> willBeUse) //_TEMP和全局变量分配寄存器
{
	int regInd = -1;
	for (int i = 0; i < 10; ++i) {
		if (regPool[i].second == "") {
			regInd = i;
			break;
		}			
	}
	if (regInd != -1) {
		regPool[regInd].second = varName;
		return regInd;
	} else { //寄存器池已满
		//将当前生成代码中不会用到的寄存器对应的变量写回内存，然后再写入当前变量
		for (int i = 0; i < 10; ++i) {
			if (find(willBeUse.begin(), willBeUse.end(), i) == willBeUse.end()) {
				//将下标为i的寄存器的值写回内存，并用varName覆盖这个寄存器
				int isTEMP = 0;
				string str = regPool[i].second;
				if (str.size() > 5) {
					if (str.substr(0, 5) == "_TEMP") {
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						genOneCode("sw", regPool[i].first, to_string(tempNum * 4) + "($s1)", "");
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) {
					int find = searchAllLevel(regPool[i].second, 0);
					if (find == -1)
						cout << "在寄存器池相关操作中出现致命错误" << endl;
					else {
						int addr = symTable.syms[find].addr;
						genOneCode("sw", regPool[i].first, to_string(addr) + "($s0)", "");
					}
				}
				regPool[i].second = varName;
				return i;
			}
		}
		cout << "异常情况：寄存器池没有可以换出的变量" << endl;
		return -1;
	}
}

void writeBack() //将寄存器池中所有被占用的寄存器回写，并清空寄存器池
{
	for (int i = 0; i < 10; ++i) {
		if (regPool[i].second == "")
			continue;
		int isTEMP = 0;
		string str = regPool[i].second;
		if (str.size() > 5) {
			if (str.substr(0, 5) == "_TEMP") {
				str.erase(str.find("_TEMP"), 5);
				istringstream is(str);
				int tempNum;
				is >> tempNum;
				genOneCode("sw", regPool[i].first, to_string(tempNum * 4) + "($s1)", "");
				isTEMP = 1;
			}
		}
		if (isTEMP == 0) {
			int find = searchAllLevel(regPool[i].second, 0);
			if (find == -1)
				cout << "在寄存器池相关操作中出现致命错误" << endl;
			else {
				int addr = symTable.syms[find].addr;
				genOneCode("sw", regPool[i].first, to_string(addr) + "($s0)", "");
			}
		}
	}
	clearPool();
}

pair<int, int> getFuncBlocks(int blockInd) //返回包含blockInd的函数的起始和结束block在blockGraph的下标
{
	int is_find = 0;
	int startInd = -1, endInd = -1;
	for (int i = blockInd; i >= 0; --i) {
		if (imTable.exprs[blockGraph[i]->codes[0]].type == FUNC) {
			startInd = i;
			is_find = 1;
			break;
		}
	}
	if (is_find == 0) {
		cout << "寻找函数基本块时发生严重错误！" << endl;
		exit(0);
	}
	is_find = 0;
	for (int i = blockInd + 1; i < blockGraph.size(); ++i) {
		if (imTable.exprs[blockGraph[i]->codes[0]].type == FUNC) {
			endInd = i - 1;
			is_find = 1;
			break;
		}
	}
	if (is_find == 0)  //main函数没有后继函数
		endInd = blockGraph.size() - 1;
	return make_pair(startInd, endInd);
}

int getBlockLevel(int i)
{
	pair<int, int> p = getFuncBlocks(i);
	string currFuncName = imTable.exprs[blockGraph[p.first]->codes[0]].expr[1];
	int nowLevel = -1;
	for (int i = 0; i < symTable.funcInd.size(); ++i) {
		if (symTable.syms[symTable.funcInd[i]].name == currFuncName) {
			nowLevel = i + 1;
			break;
		}
	}
	return nowLevel;
}

string vecToBits(vector<pair<int, int> > v)
{
	string bits(imTable.ind, '0');
	for (int i = 0; i < v.size(); ++i) {
		int ind = blockGraph[v[i].first]->codes[v[i].second];
		bits[ind] = '1';
	}
	return bits;
}

string bitUnion(string a, string b)
{
	string ret(imTable.ind, '0');
	for (int i = 0; i < imTable.ind; ++i)
		ret[i] = ((a[i] - '0') | (b[i] - '0')) + '0';
	return ret;
}

string bitSub(string a, string b)
{
	string ret(imTable.ind, '0');
	for (int i = 0; i < imTable.ind; ++i) {
		if (a[i] == '1' && b[i] == '0')
			ret[i] = '1';
		else
			ret[i] = '0';
	}
	return ret;
}

pair<int, int> bitTopair(int ind)
{
	for (int i = 0; i < blockGraph.size() - 1; ++i) {
		if (blockGraph[i]->codes[0] <= ind && blockGraph[i + 1]->codes[0] > ind) {
			if (blockGraph[i]->codes[ind - blockGraph[i]->codes[0]] == ind)
				return make_pair(i, ind - blockGraph[i]->codes[0]);
			else
				cout << "bitTopair发生严重错误" << endl;
		}
	}
	if(blockGraph[blockGraph.size() - 1]->codes[ind - blockGraph[blockGraph.size() - 1]->codes[0]] != ind)
		cout << "bitTopair发生严重错误" << endl;
	return make_pair(blockGraph.size() - 1, ind - blockGraph[blockGraph.size() - 1]->codes[0]);
}

//活跃变量分析
void liveVarAnalyse()
{

}










void dataFlowAnalyse()
{
	//为每个块生成gen和kill集合
	for (int i = 0; i < blockGraph.size(); ++i) {
		int nowLevel = getBlockLevel(i);
		pair<int, int> funcPair = getFuncBlocks(i);
		for (int j = 0; j < blockGraph[i]->codes.size(); ++j) {
			intermedia im = imTable.exprs[blockGraph[i]->codes[j]];
			if (im.type == VARASS || im.type == ARRASS) {
				//生成gen集合
				blockGraph[i]->gen.push_back(make_pair(i, j));
				//生成kill集合
				int find = searchAllLevel(im.expr[0], nowLevel);
				if (find == -1) {
					cout << "生成kill时发生严重错误" << endl;
					exit(0);
				}
				string varName = im.expr[0];
				if (symTable.syms[find].spaceLv == 0) { //全局变量
					//kill所有函数全局expr[0]的赋值
					vector<int> isGlobal;
					for (int funcI = 0; funcI < symTable.funcInd.size(); ++funcI) {
						int find = searchAllLevel(varName, funcI + 1);
						if (find == -1) {
							cout << "在生成kill时发生严重错误" << endl;
							exit(0);
						}
						isGlobal.push_back(symTable.syms[find].spaceLv);
					}
					for (int blockI = 0; blockI < blockGraph.size(); ++blockI) {
						for (int codeI = 0; codeI < blockGraph[blockI]->codes.size(); ++codeI) {
							if (blockI == i && codeI == j)
								continue;
							intermedia im = imTable.exprs[blockGraph[blockI]->codes[codeI]];
							if ((im.type == VARASS || im.type == ARRASS) && im.expr[0] == varName) {
								if (isGlobal[getBlockLevel(blockI) - 1] == 0)
									blockGraph[i]->kill.push_back(make_pair(blockI, codeI));
							}
						}
					}
				} else { //局部变量
					for (int a = funcPair.first; a <= funcPair.second; ++a) {
						for (int b = 0; b < blockGraph[a]->codes.size(); ++b) {
							if (a == i && b == j)
								continue;
							intermedia im = imTable.exprs[blockGraph[a]->codes[b]];
							if ((im.type == VARASS || im.type == ARRASS) && im.expr[0] == varName)
								blockGraph[i]->kill.push_back(make_pair(a, b));
						}
					}
				}
			}
		}
	}
	//根据算法14.4计算每个块的in和out集合
	for (int i = 0; i < blockGraph.size(); ++i) { //初始化所有out集合
		string tempStr(imTable.ind, '0');
		blockGraph[i]->outStr = tempStr;
	}	
	while (true) {
		int flag = 0;
		for (int i = 0; i < blockGraph.size(); ++i) {
			string in(imTable.ind, '0');
			string out(imTable.ind, '0');
			string gen = vecToBits(blockGraph[i]->gen);
			string kill = vecToBits(blockGraph[i]->kill);
			for (int j = 0; j < blockGraph[i]->prevBlocks.size(); ++j)
				in = bitUnion(in, blockGraph[i]->prevBlocks[j]->outStr);
			blockGraph[i]->inStr = in;
			out = bitUnion(gen, bitSub(in, kill));
			if (out != blockGraph[i]->outStr)
				flag = 1;
			blockGraph[i]->outStr = out;
		}
		if (flag == 0)
			break;
	}	
	//由outStr转换到out集合
	for (int i = 0; i < blockGraph.size(); ++i) {
		for (int j = 0; j < imTable.ind; ++j) {
			if (blockGraph[i]->inStr[j] == '1')
				blockGraph[i]->in.push_back(bitTopair(j));
			if (blockGraph[i]->outStr[j] == '1')
				blockGraph[i]->out.push_back(bitTopair(j));
		}
	}
	//构建定义使用链和网

}