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

void divideBlocks() //���ֻ�����
{
	//Ѱ��������
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
	//���ֻ�����
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
	//���ӻ�����
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
				if (i + 1 < blockGraph.size() && imTable.exprs[blockGraph[i + 1]->codes[0]].type != FUNC) {
					blockGraph[i]->nextBlocks.push_back(blockGraph[i + 1]);
					blockGraph[i + 1]->prevBlocks.push_back(blockGraph[i]);
				}
				break;
			case CALL:
				if (i + 1 < blockGraph.size() && imTable.exprs[blockGraph[i + 1]->codes[0]].type != FUNC) {
					blockGraph[i]->nextBlocks.push_back(blockGraph[i + 1]);
					blockGraph[i + 1]->prevBlocks.push_back(blockGraph[i]);
				}
				break;
			case RET:
				;
				break;
			default:
				if (i + 1 < blockGraph.size() && imTable.exprs[blockGraph[i + 1]->codes[0]].type != FUNC) {
					blockGraph[i]->nextBlocks.push_back(blockGraph[i + 1]);
					blockGraph[i + 1]->prevBlocks.push_back(blockGraph[i]);
				}
				break;
		}
	}
}

//dagͼɾ���������ڹ����ӱ��ʽ
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
	//Ϊ�����ӽڵ���Ӹ��ڵ�
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
		//����dagͼ
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
					else {//δ�ҵ�x������dagͼ���½�һ��Ҷ���
						nodeNum_x = genNewNode(im->expr[1], NULL, NULL, NULL, 1);
						nodeTab.push_back(make_pair(im->expr[1], nodeNum_x));
					}
					int find_y = findNode(nodeTab, im->expr[3]);
					int nodeNum_y = -1;
					if (find_y != -1)
						nodeNum_y = nodeTab[find_y].second;
					else {//δ�ҵ�y������dagͼ���½�һ��Ҷ���
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
					else {//δ�ҵ�x������dagͼ���½�һ��Ҷ���
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
		//����dagͼ�����м�ڵ����
		vector<dagNode> opQueue;
		while (!hasAllInQue()) {
			for (int i = 0; i < dagGraph.size(); ++i) {
				if (dagGraph[i]->isLeaf == 0 && dagGraph[i]->hasInQue == -1) {
					if (dagGraph[i]->parents.size() == 0) { //û�и��ڵ���м�ڵ�
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
			//��(4)��
			dagNode dg = opQueue.back();
			while (dg->lchild != NULL) {
				int isContinue = 0;
				dg = dg->lchild;
				if (dg->isLeaf == 0 && dg->hasInQue == -1) {
					if (dg->parents.size() == 0) { //û�и��ڵ���м�ڵ�
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
		//�ɶ��������м����
		int k = 0;
		//dagͼ����
		dagGraph.clear();
	}
	
}

//��ʱ�Ĵ����ط���
inline void clearPool()
{
	for (int i = 0; i < 10; ++i)
		regPool[i].second.first = "";
}

int getFromPool(string varName, int level, 
	vector<string> willBeUse, int varInd, int isTemp) //_TEMP��ȫ�ֱ�������Ĵ���
{
	int regInd = -1;
	for (int i = 0; i < 10; ++i) {
		if (regPool[i].second.first == "") {
			regInd = i;
			break;
		}			
	}
	if (regInd != -1) {
		regPool[regInd].second.first = varName;
		regPool[regInd].second.second = level;
		if (isTemp == 1) {
			tempRegTab[varInd] = regPool[regInd].first;
			genOneCode("lw", regPool[regInd].first, to_string(varInd * 4) + "($a3)", "");
		} else {
			symTable.syms[varInd].reg = regPool[regInd].first;
			if(symTable.syms[varInd].spaceLv == 0)
				genOneCode("lw", regPool[regInd].first, to_string(symTable.syms[varInd].addr) + "($a1)", "");
			else
				genOneCode("lw", regPool[regInd].first, to_string(symTable.syms[varInd].addr) + "($a2)", "");
		}
		return regInd;
	} else { //�Ĵ���������
		//����ǰ���ɴ����в����õ��ļĴ�����Ӧ�ı���д���ڴ棬Ȼ����д�뵱ǰ����
		for (int i = 0; i < 10; ++i) {
			if (find(willBeUse.begin(), willBeUse.end(), regPool[i].first) == willBeUse.end()) {
				//���±�Ϊi�ļĴ�����ֵд���ڴ棬����varName��������Ĵ���
				int isTEMP = 0;
				string str = regPool[i].second.first;
				if (str.size() > 5) {
					if (str.substr(0, 5) == "_TEMP") {
						str.erase(str.find("_TEMP"), 5);
						istringstream is(str);
						int tempNum;
						is >> tempNum;
						genOneCode("sw", regPool[i].first, to_string(tempNum * 4) + "($a3)", "");
						tempRegTab[tempNum] = "";
						isTEMP = 1;
					}
				}
				if (isTEMP == 0) {
					int find = searchAllLevel(regPool[i].second.first, regPool[i].second.second);
					if (find == -1)
						cout << "�ڼĴ�������ز����г�����������" << endl;
					else {
						int addr = symTable.syms[find].addr;
						if(symTable.syms[find].spaceLv == 0)
							genOneCode("sw", regPool[i].first, to_string(addr) + "($a1)", "");
						else
							genOneCode("sw", regPool[i].first, to_string(addr) + "($a2)", "");
						symTable.syms[find].reg = "";
					}
				}
				regPool[i].second.first = varName;
				regPool[i].second.second = level;
				if (isTemp == 1) {
					tempRegTab[varInd] = regPool[i].first;
					genOneCode("lw", regPool[i].first, to_string(varInd * 4) + "($a3)", "");
				} else {
					symTable.syms[varInd].reg = regPool[i].first;
					if(symTable.syms[varInd].spaceLv == 0)
						genOneCode("lw", regPool[i].first, to_string(symTable.syms[varInd].addr) + "($a1)", "");
					else
						genOneCode("lw", regPool[i].first, to_string(symTable.syms[varInd].addr) + "($a2)", "");
				}
				return i;
			}
		}
		cout << "�쳣������Ĵ�����û�п��Ի����ı���" << endl;
		return -1;
	}
}

void writeBack() //���Ĵ����������б�ռ�õļĴ�����д������ռĴ�����
{
	for (int i = 0; i < 10; ++i) {
		if (regPool[i].second.first == "")
			continue;
		int isTEMP = 0;
		string str = regPool[i].second.first;
		if (str.size() > 5) {
			if (str.substr(0, 5) == "_TEMP") {
				str.erase(str.find("_TEMP"), 5);
				istringstream is(str);
				int tempNum;
				is >> tempNum;
				genOneCode("sw", regPool[i].first, to_string(tempNum * 4) + "($a3)", "");
				tempRegTab[tempNum] = "";
				isTEMP = 1;
			}
		}
		if (isTEMP == 0) {
			int find = searchAllLevel(regPool[i].second.first, regPool[i].second.second);
			if (find == -1)
				cout << "�ڼĴ�������ز����г�����������" << endl;
			else {
				int addr = symTable.syms[find].addr;
				if(symTable.syms[find].spaceLv == 0)
					genOneCode("sw", regPool[i].first, to_string(addr) + "($a1)", "");
				else
					genOneCode("sw", regPool[i].first, to_string(addr) + "($a2)", "");
				symTable.syms[find].reg = "";
			}
		}
	}
	clearPool();
}

pair<int, int> getFuncBlocks(int blockInd) //���ذ���blockInd�ĺ�������ʼ�ͽ���block��blockGraph���±�
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
		cout << "Ѱ�Һ���������ʱ�������ش���" << endl;
	}
	is_find = 0;
	for (int i = blockInd + 1; i < blockGraph.size(); ++i) {
		if (imTable.exprs[blockGraph[i]->codes[0]].type == FUNC) {
			endInd = i - 1;
			is_find = 1;
			break;
		}
	}
	if (is_find == 0)  //main����û�к�̺���
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
				cout << "bitTopair�������ش���" << endl;
		}
	}
	if(blockGraph[blockGraph.size() - 1]->codes[ind - blockGraph[blockGraph.size() - 1]->codes[0]] != ind)
		cout << "bitTopair�������ش���" << endl;
	return make_pair(blockGraph.size() - 1, ind - blockGraph[blockGraph.size() - 1]->codes[0]);
}

//��Ծ��������
inline bool isNum(string str)
{
	if (str[0] == '+' || str[0] == '-' || str[0] == '\'' || (str[0] >= '0' && str[0] <= '9'))
		return true;
	else
		return false;
}

void insDefAndUse(int blockNum, string varName, int flag)
{
	if (find(blockGraph[blockNum]->def.begin(), blockGraph[blockNum]->def.end(), varName) == blockGraph[blockNum]->def.end() &&
		find(blockGraph[blockNum]->use.begin(), blockGraph[blockNum]->use.end(), varName) == blockGraph[blockNum]->use.end()) {
		if (flag == 0)
			blockGraph[blockNum]->def.push_back(varName);
		else
			blockGraph[blockNum]->use.push_back(varName);
	}
}

vector<string> setUnion(vector<string> v1, vector<string> v2)
{
	vector<string> ret(v1);
	for (int i = 0; i < v2.size(); ++i) {
		if (find(ret.begin(), ret.end(), v2[i]) == ret.end())
			ret.push_back(v2[i]);
	}
	return ret;
}

vector<string> setSub(vector<string> v1, vector<string> v2)
{
	vector<string> ret;
	for (int i = 0; i < v1.size(); ++i) {
		if (find(v2.begin(), v2.end(), v1[i]) == v2.end())
			ret.push_back(v1[i]);
	}
	return ret;
}

void liveVarAnalyse() //������ȫ�ֱ�����_TEMP���м��㣬Ȼ���ڽ���н�����ɾ��
{
	//Ϊÿ��������def��use����
	for (int i = 0; i < blockGraph.size(); ++i) {
		for (int j = 0; j < blockGraph[i]->codes.size(); ++j) {
			intermedia im = imTable.exprs[blockGraph[i]->codes[j]];
			switch (im.type) {
				case VARASS:
					if(!isNum(im.expr[1]))
						insDefAndUse(i, im.expr[1], 1); //����x
					if (im.expr[3] != "")
						if(!isNum(im.expr[3]))
							insDefAndUse(i, im.expr[3], 1); //����y
					insDefAndUse(i, im.expr[0], 0); //����z
					break;
				case ARRASS:
					if (im.expr[1] == "[]") { //����x
						if (!isNum(im.expr[2]))
							insDefAndUse(i, im.expr[2], 1);
					} else {
						insDefAndUse(i, im.expr[1], 1);
					}
					if (!isNum(im.expr[3]))
						insDefAndUse(i, im.expr[3], 1);
					insDefAndUse(i, im.expr[0], 0);					
					break;
				case COMPARE:
					if (!isNum(im.expr[0]))
						insDefAndUse(i, im.expr[0], 1);
					if (!isNum(im.expr[2]))
						insDefAndUse(i, im.expr[2], 1);
					break;
				case PUSH:
					if(im.expr[1][0] != '"' && !isNum(im.expr[1]))
						insDefAndUse(i, im.expr[1], 1);
					break;
				case RET:
					if(im.expr[1] != "")
						if (!isNum(im.expr[1]))
							insDefAndUse(i, im.expr[1], 1);
					break;
			}
		}
	}
	//����in��out����
	while (true) {
		int flag = 0;
		for (int i = 0; i < blockGraph.size(); ++i) {
			int lastNum = blockGraph[i]->in.size();
			blockGraph[i]->in.clear();
			blockGraph[i]->out.clear();
			for (int j = 0; j < blockGraph[i]->nextBlocks.size(); ++j)
				blockGraph[i]->out = setUnion(blockGraph[i]->out, blockGraph[i]->nextBlocks[j]->in);
			blockGraph[i]->in = setUnion(blockGraph[i]->use, setSub(blockGraph[i]->out, blockGraph[i]->def));
			int nowNum = blockGraph[i]->in.size();
			if (lastNum != nowNum)
				flag = 1;		
		}
		if (flag == 0)
			break;
	}
	//��in��out������ɾȥȫ�ֱ�����_TEMP�Լ��������
	for (int i = 0; i < blockGraph.size(); ++i) {
		if (i == 0) {
			if (imTable.exprs[blockGraph[0]->codes[0]].type != FUNC)
				continue;
		}
		int nowLevel = getBlockLevel(i);
		for (int j = 0; j < blockGraph[i]->in.size(); ++j) {
			string name = blockGraph[i]->in[j];
			int isDel = 0;
			if (name.size() > 5 && name.substr(0, 5) == "_TEMP")
				isDel = 1;
			if (name.size() >= 4 && name == "_RET")
				isDel = 1;
			if (searchAllLevel(name, 0) != -1)
				isDel = 1;
			if(searchWithLevel(name, 3, nowLevel) != -1)
				isDel = 1;
			if (isDel == 1) {
				blockGraph[i]->in.erase(blockGraph[i]->in.begin() + j);
				--j;
			}
		}
		for (int j = 0; j < blockGraph[i]->out.size(); ++j) {
			string name = blockGraph[i]->out[j];
			int isDel = 0;
			if (name.size() > 5 && name.substr(0, 5) == "_TEMP")
				isDel = 1;
			if (name.size() >= 4 && name == "_RET")
				isDel = 1;
			if (searchAllLevel(name, 0) != -1)
				isDel = 1;
			if (searchWithLevel(name, 3, nowLevel) != -1)
				isDel = 1;
			if (isDel == 1) {
				blockGraph[i]->out.erase(blockGraph[i]->out.begin() + j);
				--j;
			}
		}
		for (int j = 0; j < blockGraph[i]->def.size(); ++j) {
			string name = blockGraph[i]->def[j];
			int isDel = 0;
			if (name.size() > 5 && name.substr(0, 5) == "_TEMP")
				isDel = 1;
			if (name.size() >= 4 && name == "_RET")
				isDel = 1;
			if (searchAllLevel(name, 0) != -1)
				isDel = 1;
			if (searchWithLevel(name, 3, nowLevel) != -1)
				isDel = 1;
			if (isDel == 1) {
				blockGraph[i]->def.erase(blockGraph[i]->def.begin() + j);
				--j;
			}
		}
		for (int j = 0; j < blockGraph[i]->use.size(); ++j) {
			string name = blockGraph[i]->use[j];
			int isDel = 0;
			if (name.size() > 5 && name.substr(0, 5) == "_TEMP")
				isDel = 1;
			if (name.size() >= 4 && name == "_RET")
				isDel = 1;
			if (searchAllLevel(name, 0) != -1)
				isDel = 1;
			if (searchWithLevel(name, 3, nowLevel) != -1)
				isDel = 1;
			if (isDel == 1) {
				blockGraph[i]->use.erase(blockGraph[i]->use.begin() + j);
				--j;
			}
		}
	}
}

int findInConf(vector<confNode> funcConf, string name)
{
	for (int i = 0; i < funcConf.size(); ++i) {
		if (funcConf[i]->name == name)
			return i;
	}
	return -1;
}

void genConfGraph()
{
	for (int blockInd = 0; blockInd < blockGraph.size(); ++blockInd) {
		if (imTable.exprs[blockGraph[blockInd]->codes[0]].type == FUNC) {
			pair<int, int> stAndEd = getFuncBlocks(blockInd); //������ʼ�ͽ���block���±�
			vector<string> vars; //������������Ҫ�Ĵ����ķ�����ֲ�����
			for (int i = stAndEd.first; i <= stAndEd.second; ++i) {
				for (int j = 0; j < blockGraph[i]->in.size(); ++j) {
					if (find(vars.begin(), vars.end(), blockGraph[i]->in[j]) == vars.end())
						vars.push_back(blockGraph[i]->in[j]);
				}
				for (int j = 0; j < blockGraph[i]->out.size(); ++j) {
					if (find(vars.begin(), vars.end(), blockGraph[i]->out[j]) == vars.end())
						vars.push_back(blockGraph[i]->out[j]);
				}
			}
			//Ϊvars������б���������ͻͼ
			vector<confNode> funcConf;
			for (int i = 0; i < vars.size(); ++i) {
				confNode cn = new struct varNode;
				cn->name = vars[i];
				funcConf.push_back(cn);
			}
			//���ݻ�Ծ��������������Ӹ������
			for (int i = stAndEd.first; i <= stAndEd.second; ++i) {
				for (int ind1 = 0; ind1 < blockGraph[i]->in.size(); ++ind1) {
					for (int ind2 = ind1 + 1; ind2 < blockGraph[i]->in.size(); ++ind2) {
						int nodeInd1 = findInConf(funcConf, blockGraph[i]->in[ind1]);
						int nodeInd2 = findInConf(funcConf, blockGraph[i]->in[ind2]);
						int flag = 0;
						for (int a = 0; a < funcConf[nodeInd1]->confVars.size(); ++a) {
							if (funcConf[nodeInd1]->confVars[a]->name == funcConf[nodeInd2]->name) {
								flag = 1;
								break;
							}
						}
						if (flag == 0) { //�������
							funcConf[nodeInd1]->confVars.push_back(funcConf[nodeInd2]);
							funcConf[nodeInd2]->confVars.push_back(funcConf[nodeInd1]);
						}
					}
				}
			}
			confGraph.push_back(funcConf);
		}
	}
}

void delNode(vector<confNode> &tempGraph, int delInd)
{
	string name = tempGraph[delInd]->name;
	tempGraph.erase(tempGraph.begin() + delInd);
	for (int i = 0; i < tempGraph.size(); ++i) {
		for (int j = 0; j < tempGraph[i]->confVars.size(); ++j) {
			if (tempGraph[i]->confVars[j]->name == name) {
				tempGraph[i]->confVars.erase(tempGraph[i]->confVars.begin() + j);
				--j;
			}
		}
	}
}

void allocReg(vector<vector<confNode> > &graphAlign, int ind, string nodeName, string regName)
{
	for (int i = ind; i >= 0; --i) {
		int find;
		for (find = 0; find < graphAlign[i].size(); ++find) {
			if (graphAlign[i][find]->name == nodeName) {
				graphAlign[i][find]->reg = regName;
				break;
			}			
		}
	}
}

void globalRegAlloc() //ͼ��ɫ�㷨Ϊÿ�������ĳ�ͻͼ����ȫ�ּĴ���s0-s7
{
	int K = 8;
	for (int i = 0; i < confGraph.size(); ++i) {
		if (confGraph[i].size() == 0)
			continue;
		vector<vector<confNode> > graphAlign;
		vector<pair<string, int> > nodeAlign;
		vector<string> noReg;
		vector<confNode> tempGraph(confGraph[i]);
		graphAlign.push_back(tempGraph);
		while (tempGraph.size() > 1) {
			int maxLinks = 0, maxInd = -1;
			for (int j = 0; j < tempGraph.size(); ++j) {
				if (tempGraph[j]->confVars.size() < K) {
					if (tempGraph[j]->confVars.size() >= maxLinks) {
						maxLinks = tempGraph[j]->confVars.size();
						maxInd = j;
					}
				}
			}
			if (maxInd != -1) { //��������һ�����
				nodeAlign.push_back(make_pair(tempGraph[maxInd]->name, 1));
				delNode(tempGraph, maxInd);
				graphAlign.push_back(tempGraph);
			} else { //�Ѿ��޷����߽��
				//ѡȡһ���ʵ��Ľ���Ϊ������Ĵ����Ľ��(ȡ���ӱ����ٵĽ��)
				int minLink = 9999, minInd = -1;
				for (int j = 0; j < tempGraph.size(); ++j) {
					if (tempGraph[j]->confVars.size() <= minLink) {
						minLink = tempGraph[j]->confVars.size();
						minInd = j;
					}
				}
				noReg.push_back(tempGraph[minInd]->name);
				nodeAlign.push_back(make_pair(tempGraph[minInd]->name, -1));
				delNode(tempGraph, minInd);
				graphAlign.push_back(tempGraph);
			}
		}
		//����Ĵ���
		for (int j = graphAlign.size() - 1; j >= 0; --j) {
			if (j == graphAlign.size() - 1)
				allocReg(graphAlign, j, graphAlign[j][0]->name, "$s0");
			else {
				//��ȷ����û�мĴ���
				if(nodeAlign[j].second == -1)
					allocReg(graphAlign, j, nodeAlign[j].first, "");
				else {
					for (int regInd = 0; regInd < 8; ++regInd) {
						string str = "$s";
						string regName = str + to_string(regInd);
						int flag = 0;
						for (int x = 0; x < graphAlign[j].size(); ++x) {
							if (graphAlign[j][x]->name == nodeAlign[j].first) {
								for (int y = 0; y < graphAlign[j][x]->confVars.size(); ++y) {
									if (graphAlign[j][x]->confVars[y]->reg == regName) {
										flag = 1;
										break;
									}
								}
								break;
							}
						}
						if (flag == 0) {
							allocReg(graphAlign, j, nodeAlign[j].first, regName);
							break;
						}
						if (regInd == 7)
							cout << "ͼ��ɫ����Ĵ���ʱ������������" << endl;
					}
				}			
			}			
		}
		//���Ĵ���������д����ű�
		int startInd = symTable.funcInd[i];
		for (int j = 0; j < graphAlign[0].size(); ++j) {
			for (int x = startInd + 1; symTable.syms[x].object != 4; ++x) {
				if (symTable.syms[x].name == graphAlign[0][j]->name) {
					symTable.syms[x].reg = graphAlign[0][j]->reg;
					break;
				}
			}
		}
	}
}

void optimize()
{
	divideBlocks();
	dagOpt();
	liveVarAnalyse();
	genConfGraph();
	globalRegAlloc();
}






/*void dataFlowAnalyse()
{
	//Ϊÿ��������gen��kill����
	for (int i = 0; i < blockGraph.size(); ++i) {
		int nowLevel = getBlockLevel(i);
		pair<int, int> funcPair = getFuncBlocks(i);
		for (int j = 0; j < blockGraph[i]->codes.size(); ++j) {
			intermedia im = imTable.exprs[blockGraph[i]->codes[j]];
			if (im.type == VARASS || im.type == ARRASS) {
				//����gen����
				blockGraph[i]->gen.push_back(make_pair(i, j));
				//����kill����
				int find = searchAllLevel(im.expr[0], nowLevel);
				if (find == -1) {
					cout << "����killʱ�������ش���" << endl;
					exit(0);
				}
				string varName = im.expr[0];
				if (symTable.syms[find].spaceLv == 0) { //ȫ�ֱ���
					//kill���к���ȫ��expr[0]�ĸ�ֵ
					vector<int> isGlobal;
					for (int funcI = 0; funcI < symTable.funcInd.size(); ++funcI) {
						int find = searchAllLevel(varName, funcI + 1);
						if (find == -1) {
							cout << "������killʱ�������ش���" << endl;
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
				} else { //�ֲ�����
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
	//�����㷨14.4����ÿ�����in��out����
	for (int i = 0; i < blockGraph.size(); ++i) { //��ʼ������out����
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
	//��outStrת����out����
	for (int i = 0; i < blockGraph.size(); ++i) {
		for (int j = 0; j < imTable.ind; ++j) {
			if (blockGraph[i]->inStr[j] == '1')
				blockGraph[i]->in.push_back(bitTopair(j));
			if (blockGraph[i]->outStr[j] == '1')
				blockGraph[i]->out.push_back(bitTopair(j));
		}
	}
	//��������ʹ��������

}
*/