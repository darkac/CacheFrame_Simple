// Last modified: 2013-04-24 20:38:22
 
/**
 * @file: qtfdf.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-11-30 15:11:54
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "qtfdf.h"
#include "function.h"
 
using namespace std;
 
CCacheFrame_QTFDF::CCacheFrame_QTFDF(
		int _size,
		const char *stat_file) : CCacheFrame(_size)
{
	policy_name = CP_QTFDF;
	
	string file_name;
	file_name.assign("stat/");
	file_name.append(stat_file);
	fSTAT = fopen(file_name.c_str(), "r");
	checkResource(fSTAT, file_name.c_str());
	
	char buf[128];
	int termid, fre, len;
	while (fgets(buf, 128, fSTAT) != NULL)
	{
		sscanf(buf, "%d\t%d\t%d", &termid, &fre, &len);
		mapFre[termid] = fre;
		mapLen[termid] = len;
		//qtQue.push(QtfDf(termid, static_cast<double>fre/len));
		//QtfDf *newQ = new QtfDf(termid, static_cast<double>(fre) / len);
		//qtQue_push(newQ);
	}
	closeResource(fSTAT);
}

CCacheFrame_QTFDF::~CCacheFrame_QTFDF()
{
	mapFre.clear();
	mapLen.clear();
	while(!qtQue_empty())
		qtQue_pop();
}

void CCacheFrame_QTFDF::CacheListInsert(
		unsigned int termid,
		unsigned int listLen,
		unsigned long long file_offset,
		ItemType type)
{
	int slot = HashKey(termid);
	ht_wrlock(slot);
	hashnode_t *tmp = getHashNode(termid);
	if (tmp != NULL)
	{
		// nothing to do here
	}
	else
	{
		cachenode_t *newcnode = (cachenode_t *)malloc(sizeof(cachenode_t));
		checkPointer(newcnode, __LINE__);
		memset(newcnode, 0, sizeof(cachenode_t));
		newcnode->m_termid = termid;
		newcnode->m_listlen = listLen;

		hashnode_t *newhnode = (hashnode_t *)malloc(sizeof(hashnode_t));
		checkPointer(newhnode, __LINE__);
		memset(newhnode, 0, sizeof(hashnode_t));
		newhnode->m_key = termid;
		newhnode->m_listLength = listLen;
		newhnode->m_cache_node = newcnode;
		newhnode->h_next = hashTable[slot]->h_next;
		hashTable[slot]->h_next = newhnode;
	
		if (firstCache == lastCache)
		{
			firstCache->c_next = newcnode;
			newcnode->c_prev = firstCache;
			newcnode->c_next = NULL;
			lastCache = newcnode;
		}
		else
		{
			firstCache->c_next->c_prev = newcnode;
			newcnode->c_next = firstCache->c_next;
			newcnode->c_prev = firstCache;
			firstCache->c_next = newcnode;
		}
		cacheUnUsed -= listLen;

		newcnode->m_list_pointer = (int *)malloc(listLen * sizeof(int));
		checkPointer(newcnode->m_list_pointer, __LINE__);
		fseek(pIndex, file_offset, SEEK_SET);
		size_t nobj = fread(newcnode->m_list_pointer, sizeof(int), listLen, pIndex);
		assert((unsigned int)nobj == listLen);
		io_number++;
		io_amount += listLen;

		QtfDf *newQ = new QtfDf(termid, static_cast<double>(mapFre[termid]) / mapLen[termid]);
		qtQue_push(newQ);
	}
	ht_unlock(slot);

	return ;
}

void CCacheFrame_QTFDF::CacheListUpdate(cachenode_t *pcur)
{
	mapFre[pcur->m_termid]++;
	return ;
}

void CCacheFrame_QTFDF::CacheListEvict(int cur_len)
{
	double newQtfDf, preQtfDf;
	while (IsSpaceEnough(cur_len) == 0)
	{
		if (qtQue_empty())
		{
			printf("qtQue Empty!\n");
			exit(-1);
		}

		const QtfDf* oldQ = qtQue_top();
		unsigned int termid = oldQ->termid;
		qtQue.pop();

		preQtfDf = oldQ->value;
		newQtfDf = static_cast<double>(mapFre[termid]) / mapLen[termid];
		if (newQtfDf > preQtfDf)
		{
			QtfDf *newQ = new QtfDf(termid, newQtfDf);
			qtQue_push(newQ);
			delete oldQ;
			continue;
		}

		int slot = HashKey(termid);
		hashnode_t *hn, *cur_hnode;

		hn = hashTable[slot];
		//ht_wrlock(slot);
		if (ht_trywrlock(slot) == EBUSY)
		{
			cout << "exceeed " << (cur_len - cacheUnUsed) * sizeof(int) << endl;
			break;
		}
		while ((cur_hnode = hn->h_next) != NULL)
		{
			if (cur_hnode->m_key == termid)
				break;
			hn = hn->h_next;
		}

		if (cur_hnode == NULL)
		{
			// In fact, this branch will never be reached
			ht_unlock(slot);
		}
		else
		{
			int listlen = cur_hnode->m_listLength;
			cachenode_t *cnode = cur_hnode->m_cache_node;
			hn->h_next = cur_hnode->h_next;
			freeResource(cur_hnode);
			ht_unlock(slot);

			freeResource(cnode->m_list_pointer);
			cacheUnUsed += listlen;
			
			cnode->c_prev->c_next = cnode->c_next;
			if (cnode == lastCache)
				lastCache = cnode->c_prev;
			else
				cnode->c_next->c_prev = cnode->c_prev;
			freeResource(cnode);
		}
		delete oldQ;
	}
}
