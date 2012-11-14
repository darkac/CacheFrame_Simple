// Last modified: 2012-11-13 00:50:43
 
/**
 * @file: qtca.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-10-26 16:25:16
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cassert>

#include "qtca.h"
#include "function.h"
 
using namespace std;

void* Prefetch(void *arg)
{
	prefetch_info_t *pinfo = (prefetch_info_t *)arg;

	pinfo->m_cf->cf_lock_mutex();
	//int slot = HashKey(pinfo->termid);
	//ht_rdlock(slot);
	if (getHashNode(pinfo->m_termid) != NULL)
	{
		//nothing to do, just leave this function
	}
	else
	{
		pinfo->m_cf->inc_indeed();
		//insert
		if (pinfo->m_cf->IsSpaceEnough(pinfo->m_listlen) == 0)
		{
			pinfo->m_cf->CacheListEvict(pinfo->m_listlen);
		}
		pinfo->m_cf->CacheListInsert(
				pinfo->m_termid, pinfo->m_listlen, pinfo->m_file_offset, Prefetched);
	}
	
	//ht_unlock(slot);
	pinfo->m_cf->cf_unlock_mutex();
	freeResource(pinfo);
	
	return (void *)0;
}

CCacheFrame_QTCA::CCacheFrame_QTCA(
		int _size,
		int intersect_thread_num,
		int _sup, int _wnd, double _conf,
		MemoryDict *_dict) : CCacheFrame(_size)
{
	cor_file.clear();
	cor_file.assign("correlated/cor");
	stringstream tmp;
	tmp << cor_file << "_" << _sup << "_" << _conf << "_" << _wnd;
	cor_file = tmp.str();
	
	fCOR = fopen(cor_file.c_str(), "r");
	if (fCOR == NULL)
	{
		cout << "Failed to open file `" << cor_file << "`" << endl;
		exit(-1);
	}
	
	correlationM.clear();
	suffix_LengthM.clear();
	suffix_OffsetM.clear();

	int co_fre, term1, term2;
	float co_conf;
	char line[128];
	while (fgets(line, 128, fCOR) > 0)
	{
		sscanf(line, "%d %f %d %d\n", &co_fre, &co_conf, &term1, &term2);
		suffix_LengthM[term2] = _dict->m_vecDict[term2].m_nFreq;
		suffix_OffsetM[term2] = _dict->m_vecDict[term2].m_nOffset;

		correlationM[term1].push_back(term2);
	}
	closeResource(fCOR);
	
#ifdef AC_DBG
	cout << correlationM.size() << endl;
	map<int, vector<int> >::iterator mit;
	for (mit = correlationM.begin(); mit != correlationM.end(); ++mit)
	{
		cout << mit->first << "\t" << mit->second.size();
		for (vector<int>::iterator vit = mit->second.begin(); vit != mit->second.end(); ++vit)
		{
			cout << "\t" << *vit;
		}
		cout << endl;
	}
#endif

	int prefetch_thread_num = intersect_thread_num / 2;
	prefetch_TM = new ThreadManager(prefetch_thread_num);
	cout << "prefetch thread num = " << prefetch_TM->thread_num << endl;

	suffix_supposed = 0;
	suffix_indeed = 0;
}

CCacheFrame_QTCA::~CCacheFrame_QTCA()
{
	prefetch_TM->sync();
	delete prefetch_TM;

	correlationM.clear();
	suffix_LengthM.clear();
	suffix_OffsetM.clear();

	cout << "suffix_supposed = " << suffix_supposed << endl;
	cout << "suffix_indeed = " << suffix_indeed << endl;
}

void CCacheFrame_QTCA::Correlated_Load(unsigned int termid)
{
	map<int, vector<int> >::iterator mit = correlationM.find(termid);
	if (mit != correlationM.end())
	{
		int NumOfPair = mit->second.size();
		for (int i = 0; i < NumOfPair; ++i)
		{
			int tid = mit->second[i];
			if (ExistInCache(tid) == 0)
			{
				inc_suppose();

				int len = suffix_LengthM[tid];
				unsigned long long oft = suffix_OffsetM[tid];
				// the parameter prefetch_info_t
				//prefetch_TM->append(Prefetch, );
				prefetch_info_t *pinfo = (prefetch_info_t *)malloc(sizeof(prefetch_info_t));
				pinfo->m_termid = tid;
				pinfo->m_listlen = len;
				pinfo->m_file_offset = oft;
				pinfo->m_cf = this;
				prefetch_TM->append(Prefetch, pinfo);
				// the pinfo is freed by the `Prefetch`
			}
		}
	}
}

void CCacheFrame_QTCA::CacheListInsert(
		unsigned int termid,
		unsigned int listLen,
		unsigned long long file_offset,
		ItemType type)
{
	int slot = HashKey(termid);
	ht_wrlock(slot); //TODO Is this necessary?
	hashnode_t *tmp = getHashNode(termid);
	if (tmp != NULL)
	{
		// nothing to do here
	}
	else
	{
		cachenode_t *newcnode  = (cachenode_t *)malloc(sizeof(cachenode_t));
		checkPointer(newcnode, __LINE__);
		memset(newcnode, 0, sizeof(cachenode_t));
		newcnode->m_termid = termid;
		newcnode->m_listlen = listLen;
		
		newcnode->m_reference = 1;
		newcnode->m_chance = 0;
		//newcnode->m_type = Requested;
		newcnode->m_type = type;

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
		assert(static_cast<unsigned int>(nobj) == listLen);
	}
	ht_unlock(slot);

	//ht_rdlock(slot);

	return;
}

void CCacheFrame_QTCA::CacheListUpdate(cachenode_t *pcur)
{
	unsigned isMined = 0;
	if (pcur->m_type == Prefetched)
	{
		isMined = 1;
		pcur->m_type = Requested;
	}

	pcur->m_chance = 0;
	//++pcur->m_reference;

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
		// nothing to do here
	}

	if (isMined == 0)
	{
		if (++pcur->m_reference == REFTOLOAD)
		{
			Correlated_Load(pcur->m_termid);
		}
	}

	return ;
}

void CCacheFrame_QTCA::CacheListEvict(int cur_len)
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
		freeResource(lastCache);
		lastCache = temp;
	}

	//TODO take `LIFECHANCE` and `LOOKAHEAD` into account

	return ;
}
