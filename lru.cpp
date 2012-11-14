// Last modified: 2012-11-12 02:49:22
 
/**
 * @file: lru.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-10-26 14:09:42
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "lru.h"
#include "function.h"
 
using namespace std;
/*
CCacheFrame_LRU::~CCacheFrame_LRU()
{
	printf("hit_ratio = %f\n", get_ratio());

	cachenode_t *tmp = firstCache->c_next;
	cachenode_t *todel;
	while ((todel = tmp) != NULL)
	{
		tmp = tmp->c_next;
		freeResource(todel->m_list_pointer);
		freeResource(todel);
	}

	lastCache = firstCache;
	freeResource(firstCache);

	pthread_mutex_destroy(&cache_mutex);
}
*/
void CCacheFrame_LRU::CacheListInsert(
		unsigned int termid, 
		unsigned int listLen, 
		unsigned long long file_offset)
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
	
		//cf_lock_mutex();

		/*
		if (IsSpaceEnough(listLen) == 0)
		{
			//CacheListEvict(listLen, slot);
			CacheListEvict(listLen);
		}
		*/

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
		//cf_unlock_mutex();

		newcnode->m_list_pointer = (int *)malloc(listLen * sizeof(int));
		checkPointer(newcnode->m_list_pointer, __LINE__);
		fseek(pIndex, file_offset, SEEK_SET);
		size_t nobj = fread(newcnode->m_list_pointer, sizeof(int), listLen, pIndex);
		assert((unsigned int)nobj == listLen);
	}
	ht_unlock(slot);
	// In fact, here will be a tiny time gap,
	// but i don't think it will cause any problem.
	// Since the termid has *just* been inserted into the CacheFrame,
	// it won't be evicted at this time gap.
	//ht_rdlock(slot);

	return ;
}

void CCacheFrame_LRU::CacheListUpdate(cachenode_t *pcur)
{
	//cf_lock_mutex();
	if (firstCache->c_next != pcur)
	{
		if (pcur != lastCache)
			pcur->c_next->c_prev = pcur->c_prev;
		else
			lastCache = pcur->c_prev;
		pcur->c_prev->c_next = pcur->c_next;

		pcur->c_next = firstCache->c_next;
		firstCache->c_next->c_prev = pcur;
		firstCache->c_next = pcur;
		pcur->c_prev = firstCache;
	}
	else
	{
		/* firstCache -> pcur -> ... ... ... -> NULL
		 *                 ^              ^
		 *                 |      or      |
		 *             lastCache      lastCache
		 */
		// nothing to do here
	}
	//cf_unlock_mutex();
	return ;
}

void CCacheFrame_LRU::CacheListEvict(int cur_len)
{
	unsigned int termid;
	int listlen;
	cachenode_t *temp;
	hashnode_t *hn, *cur_hnode;

	//cf_lock_mutex();
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
		freeResource(lastCache);
		lastCache = temp;
	}
	//cf_unlock_mutex();

	return ;
}
