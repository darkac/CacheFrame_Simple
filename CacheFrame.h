// Last modified: 2012-11-12 10:10:37
 
/**
 * @file: CacheFrame.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-09-26 16:00:21
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _CACHEFRAME_H_
#define _CACHEFRAME_H_

#include <pthread.h>
#include "hash.h"

#define QTCA

#define MEMORYSIZE 256        // in terms of Byte

enum ItemType {Requested, Prefetched};

typedef struct cachenode {
	int m_termid;
	int m_listlen;
	struct cachenode *c_prev;
	struct cachenode *c_next;
	int *m_list_pointer;
#ifdef QTCA
	int m_reference;
	int m_chance;
	ItemType m_type;
#endif
} cachenode_t;

class CCacheFrame
{
protected:
	cachenode_t *firstCache, *lastCache;
	pthread_mutex_t cache_mutex;

	unsigned long long cacheCapacity; // in terms of int
	unsigned long long cacheUnUsed;   // in terms of int

	double hits;
	double total;

public:
	CCacheFrame(int _size);
	virtual ~CCacheFrame();

	//double get_hits() const;
	//double get_total() const;
	void inc_hits()			{hits++;}
	void inc_total()		{total++;}
	double get_ratio() const;

	cachenode_t *get_first() const;
	cachenode_t *get_last() const;

	virtual bool ExistInCache(const unsigned int temrid) const;
	bool IsSpaceEnough(const unsigned int listLen) const;

	virtual void CacheListInsert(
			unsigned int termid, 
			unsigned int listLen, 
			unsigned long long file_offset,
			ItemType type) = 0;
	virtual void CacheListUpdate(cachenode_t *pcur) = 0;
	virtual void CacheListEvict(int cur_len) = 0;

	void cf_lock_mutex();
	void cf_unlock_mutex();

};
extern CCacheFrame *CF;
 
#endif
