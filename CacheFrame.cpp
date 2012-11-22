// Last modified: 2012-11-22 23:26:41
 
/**
 * @file: CacheFrame.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-09-26 19:29:11
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "hash.h"
#include "function.h"
#include "CacheFrame.h"

string Policy_Name[5] = {"LRU", "LFU", "QTFDF", "QTCA", "OPT"};

CCacheFrame::CCacheFrame(int _size)
{
	firstCache = (cachenode_t *)malloc(sizeof(cachenode_t));
	checkPointer(firstCache, __LINE__);
	memset(firstCache, 0, sizeof(cachenode_t));
	lastCache = firstCache;

	if (pthread_mutex_init(&cache_mutex, NULL) != 0)
	{
		printf("cache_mutex init failed.\n");
		exit(-1);
	}

	cacheCapacity = (unsigned long long)_size * 1024 * 1024 / sizeof(int);
	cacheUnUsed = cacheCapacity;
	
	hits = 0;
	total = 0;
	io_number = 0;
	io_amount = 0;
}

CCacheFrame::~CCacheFrame()
{
	printf("policy : %s\t", Policy_Name[policy_name].c_str());
	printf("hit_ratio = %f\n", get_ratio());
	printf("hits = %f\ttotal = %f\n", get_hits(), get_total());
	printf("io_number = %f, io_amount = %f\n", io_number, io_amount);

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

double CCacheFrame::get_hits() const
{
	return hits;
}

double CCacheFrame::get_total() const
{
	return total;
}

double CCacheFrame::get_ratio() const
{
	return hits / total;
}

cachenode_t *CCacheFrame::get_first() const
{
	return firstCache;
}

cachenode_t *CCacheFrame::get_last() const
{
	return lastCache;
}

bool CCacheFrame::ExistInCache(const unsigned int termid) const
{
	return existInHash(termid);
}

bool CCacheFrame::IsSpaceEnough(const unsigned int listLen) const
{
	if (listLen > cacheUnUsed)
		return 0;
	else
		return 1;
}

void CCacheFrame::cf_lock_mutex()
{
	pthread_mutex_lock(&cache_mutex);
}

void CCacheFrame::cf_unlock_mutex()
{
	pthread_mutex_unlock(&cache_mutex);
}
