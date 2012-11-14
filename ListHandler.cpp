// Last modified: 2012-11-13 20:46:19
 
/**
 * @file: ListHandler.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-29 00:55:58
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>

#include "hash.h"
#include "function.h"
#include "CacheFrame.h"
#include "ListHandler.h"

ListHandler::ListHandler(unsigned _termid, DictItem _item)
{
	termid = _termid;
	listLength = _item.m_nFreq;
	offset = _item.m_nOffset;

	CF->cf_lock_mutex();
	CF->inc_total();
	int slot = HashKey(termid);
	//ht_rdlock(slot); // will be unlock in the ~ListHandler()
	hashnode_t *tmp = getHashNode(termid);
	if (tmp != NULL)
	{
		//cout << "term " << _termid << " hited" << endl;
		CF->inc_hits();
		cachenode_t *pcur = tmp->m_cache_node;
		CF->CacheListUpdate(pcur);
		
		hnode = tmp;
	}
	else
	{
		//ht_unlock(slot);
		//CF->cf_lock_mutex();
		if (CF->IsSpaceEnough(listLength) == 0)
		{
			CF->CacheListEvict(listLength);
		}
		CF->CacheListInsert(termid, listLength, offset, Requested);
		//CF->cf_unlock_mutex();
		hnode = getHashNode(termid);
	}
	ht_rdlock(slot);
	CF->cf_unlock_mutex();
}

ListHandler::~ListHandler()
{
	hnode = NULL;
	int slot = HashKey(termid);
	ht_unlock(slot);
}

int ListHandler::GetItem(unsigned int itemID)
{
	//hashnode_t *tmp = getHashNode(termid);
	//return tmp->m_cache_node->m_list_pointer[itemID];
	return hnode->m_cache_node->m_list_pointer[itemID];
}

unsigned long long ListHandler::GetOffset()
{
	return offset;
}

unsigned long long ListHandler::GetLength()
{
	return listLength;
}
