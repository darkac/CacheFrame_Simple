// Last modified: 2012-11-22 23:11:06
 
/**
 * @file: lfu.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-11-20 19:57:33
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "lfu.h"
#include "function.h"

CCacheFrame_LFU::CCacheFrame_LFU(int _size) : CCacheFrame(_size)
{
	policy_name = CP_LFU;
	_1Ref_InsertPoint = firstCache->c_next;
}

void CCacheFrame_LFU::CacheListInsert(
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
		newcnode->m_reference = 1;

		hashnode_t *newhnode = (hashnode_t *)malloc(sizeof(hashnode_t));
		checkPointer(newhnode, __LINE__);
		memset(newhnode, 0, sizeof(hashnode_t));
		newhnode->m_key = termid;
		newhnode->m_listLength = listLen;
		newhnode->m_cache_node = newcnode;
		newhnode->h_next = hashTable[slot]->h_next;
		hashTable[slot]->h_next = newhnode;
		
		/*
		// ------------ Original Implementation ------------
		if (firstCache == lastCache)
		{
			firstCache->c_next = newcnode;
			newcnode->c_prev = firstCache;
			newcnode->c_next = NULL;
			lastCache = newcnode;
		}
		else
		{
			cachenode_t *tmp = lastCache;
			while ((tmp != firstCache) && (tmp->m_reference == 1))
				tmp = tmp->c_prev;
			newcnode->c_next = tmp->c_next;
			if (tmp != lastCache)
				tmp->c_next->c_prev = newcnode;
			else
				lastCache = newcnode;
			newcnode->c_prev = tmp;
			tmp->c_next = newcnode;
		}
		// ------------ Original Implementation ------------
		*/

		// ------------ Revised Implementation -------------
		// A little tricky here.
		// The item is inserted in the left of the _1Ref_InsertPoint node
		// in order to avoid the traversing from the 'lastCache' node
		if (_1Ref_InsertPoint != NULL)
		{
			newcnode->c_next = _1Ref_InsertPoint;
			newcnode->c_prev = _1Ref_InsertPoint->c_prev;
			_1Ref_InsertPoint->c_prev->c_next = newcnode;
			_1Ref_InsertPoint->c_prev = newcnode;
		}
		else
		// 1. the first insertion, there is no item
		// 2. the items whose reference is 1 have all been evicted
		{
			newcnode->c_prev = lastCache;
			newcnode->c_next = NULL;
			lastCache->c_next = newcnode;
			lastCache = newcnode;
		}
		_1Ref_InsertPoint = newcnode;
		// ------------ Revised Implementation -------------

		cacheUnUsed -= listLen;

		newcnode->m_list_pointer = (int *)malloc(listLen * sizeof(int));
		checkPointer(newcnode->m_list_pointer, __LINE__);
		fseek(pIndex, file_offset, SEEK_SET);
		size_t nobj = fread(newcnode->m_list_pointer, sizeof(int), listLen, pIndex);
		assert((unsigned int)nobj == listLen);
		io_number++;
		io_amount += listLen;
		
	}
	ht_unlock(slot);

	return ;
}


void CCacheFrame_LFU::CacheListUpdate(cachenode_t *pcur)
{
	pcur->m_reference++;
	cachenode_t *tmp = pcur;
	while ((tmp = tmp->c_prev) != firstCache)
	{
		if (pcur->m_reference < tmp->m_reference)
			break;
	}
	
	if (tmp->c_next == pcur)
	{
		// nothing to do here
	}
	else
	{
		pcur->c_prev->c_next = pcur->c_next;
		if (pcur != lastCache)
			pcur->c_next->c_prev = pcur->c_prev;
		else
			lastCache = pcur->c_prev;

		pcur->c_next = tmp->c_next;
		pcur->c_prev = tmp;
		tmp->c_next->c_prev = pcur;
		tmp->c_next = pcur;
	}
	return ;
}

void CCacheFrame_LFU::CacheListEvict(int cur_len)
{
	unsigned int termid;
	int listlen;
	cachenode_t *temp;
	hashnode_t *hn, *cur_hnode;

	while (IsSpaceEnough(cur_len) == 0)
	{
		termid = lastCache->m_termid;
		listlen = lastCache->m_listlen;

		int slot = HashKey(termid);
		hn = hashTable[slot];

		ht_wrlock(slot);
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
			hn->h_next = cur_hnode->h_next;
			freeResource(cur_hnode);
			ht_unlock(slot);
			
			cacheUnUsed += listlen;
			freeResource(lastCache->m_list_pointer);
		}

		lastCache->c_prev->c_next = NULL;
		temp = lastCache->c_prev;
		if (_1Ref_InsertPoint == lastCache)
			_1Ref_InsertPoint = NULL;
		freeResource(lastCache);
		lastCache = temp;
	}

	return ;
}
