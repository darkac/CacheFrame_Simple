// Last modified: 2012-11-12 10:06:53
 
/**
 * @file: qtca.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-10-26 16:21:36
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _QTCA_H_
#define _QTCA_H_

#include <map>
#include <vector>
#include <cstring>
#include "MemoryDict.h"
#include "CacheFrame.h"
#include "TM.h"

using namespace std;

#define REFTOLOAD 2
#define LOOKAHEAD 1
#define LIFECHANCE 1
#define TRAIN_TERM 10000

class CCacheFrame_QTCA : public CCacheFrame
{
private:
	string cor_file;
	//int sup, wnd;
	//float conf;
	FILE *fCOR;

	int suffix_supposed; // the num of suffixes which might be prefetched
	int suffix_indeed;   // the num of suffixes which are prefetched indeed
public:
	map<int, vector<int> > correlationM;
	map<int, int> suffix_LengthM;
	map<int, unsigned long long> suffix_OffsetM;
	int prefetch_thread_num;
	ThreadManager *prefetch_TM;
	
	CCacheFrame_QTCA(
			int _size,
			int intersect_thread_num,
			int _sup, int _wnd, double _conf,
			MemoryDict *_dict);
	
	~CCacheFrame_QTCA();

	void CacheListInsert(
			unsigned int termid,
			unsigned int listLen,
			unsigned long long file_offset,
			ItemType type);
	void CacheListUpdate(cachenode_t *pcur);
	void CacheListEvict(int cur_len);

	void Correlated_Load(unsigned int termid);

	void inc_suppose() {suffix_supposed++;}
	void inc_indeed() {suffix_indeed++;}
	int get_suppose() {return suffix_supposed;}
	int get_indeed() {return suffix_indeed;}

	friend void* Prefetch(void *arg);
};
 
typedef struct prefetch_info
{
	unsigned int m_termid;
	unsigned int m_listlen;
	unsigned long long m_file_offset;
	CCacheFrame_QTCA *m_cf;
} prefetch_info_t;

#endif
