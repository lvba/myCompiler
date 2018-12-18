#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include<algorithm>
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
				cout << "在划分基本块时发生错误" << endl;
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
		//根据dag图导出中间代码
		vector<dagNode> opQueue;
		while (!hasAllInQue()) {
			for (int i = 0; i < dagGraph.size())
		}
		//dag图清零
		dagGraph.clear();
	}
	
}