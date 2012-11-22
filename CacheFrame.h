// Last modified: 2012-11-22 23:26:45
 
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

#include <string>
#include <pthread.h>
#include "hash.h"

using namespace std;

#define QTCA

#define MEMORYSIZE 256        // in terms of Byte

enum ItemType {Requested, Prefetched};
enum PolicyType {CP_LRU = 0, CP_LFU, CP_QTFDF, CP_QTCA, CP_OPT};
extern string Policy_Name[5];
//char policy[5][10] = {"LRU", "LFU", "QTFDF", "QTCA", "OPT"};

typedef struct cachenode {
	int m_termid;
	int m_listlen;
	struct cachenode *c_prev;
	struct cachenode *c_next;
	int *m_list_pointer;
	int m_reference;
#ifdef QTCA
	int m_chance;
	ItemType m_type;
#endif
} cachenode_t;

class CCacheFrame
{
protected:
	cachenode_t *firstCache, *lastCache;
	pthread_mutex_t cache_mutex;

	PolicyType policy_name;

	unsigned long long cacheCapacity; // in terms of int
	unsigned long long cacheUnUsed;   // in terms of int
	
	double hits;
	double total;
	double io_number;
	double io_amount; // in terms of int

public:
	CCacheFrame(int _size);
	virtual ~CCacheFrame();

	void inc_hits()			{hits++;}
	void inc_total()		{total++;}
	double get_hits() const;
	double get_total() const;
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
