// Last modified: 2012-11-21 03:15:55
 
/**
 * @file: testMM.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 18:23:34
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>

#include "hash.h"
#include "function.h"
#include "ListHandler.h"
#include "MemoryDict.h"
//#include "MemoryManager.h"
//#include "CacheFrame.h"
#include "lru.h"
#include "qtca.h"
#include "lfu.h"

using namespace std;

#define THREAD_NUM 1000
#define TERM_NUM 5000000

FILE *pIndex;
MemoryDict dict;
hashnode_t *hashTable[MAX_HASH];
CCacheFrame *CF;
pthread_mutex_t ot_mutex = PTHREAD_MUTEX_INITIALIZER;

void LoadDict()
{
	string strDictDir = "/home/fan/Index/ori/";
	string strDictName = "GOV2.Rand";
	bool bRet = dict.Load(strDictDir, strDictName);

	if(!bRet) {
		cout << "Error in reading the dict" << endl;
		return ;
	}
	string file =strDictDir + "/" + strDictName + ".index";
	pIndex = fopen(file.c_str(), "rb");
	assert(pIndex != NULL);
}

void *run(void *arg)
{
	unsigned int range = 1024;
	int tno = *(int *)arg;
	
	for (int i = 0; i < TERM_NUM / THREAD_NUM; ++i)
	{
		int termid =  TERM_NUM / THREAD_NUM * tno + i;
		int docid = 1023;
		ListHandler *LH = new ListHandler(termid, dict.m_vecDict[termid]);
		if (LH->GetLength() >= range)
		{
			pthread_mutex_lock(&ot_mutex);
			cout << termid << "\t" << docid << "\t-\t" 
				<< LH->GetItem(docid) << "\t" << LH->GetLength() << endl;
			pthread_mutex_unlock(&ot_mutex);
		}

		delete LH;
	}
	
	for (int i = 0; i < TERM_NUM / THREAD_NUM; ++i)
	{
		int termid =  TERM_NUM / THREAD_NUM * tno + i;
		ListHandler *LH = new ListHandler(termid, dict.m_vecDict[termid]);
		delete LH;
	}

	return (void *)0;
}

int main(int argc, char **argv)
{
	init_hash();

	LoadDict();
	
	int sup = 100, wnd = 10;
	double conf = 0.3;
	if (argc == 4)
	{
		sup = atoi(argv[1]);
		wnd = atoi(argv[3]);
		conf = atof(argv[2]);
	}
	//CCacheFrame_QTCA *CF_QTCA = new CCacheFrame_QTCA(MEMORYSIZE, 8, sup, wnd, conf, &dict);
	//CF = CF_QTCA;
	//CCacheFrame_LFU *CF_LFU = new CCacheFrame_LFU(MEMORYSIZE);
	//CF = CF_LFU;
	CCacheFrame_LRU *CF_LRU = new CCacheFrame_LRU(MEMORYSIZE);
	CF = CF_LRU;

	pthread_t tid[THREAD_NUM];
	int arg[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		arg[i] = i;
		pthread_create(&tid[i], NULL, run, &arg[i]);
	}
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		pthread_join(tid[i], NULL);
	}

	int range = 1024;
	int docid = 1023;
	int listLength, offset;
	int *space = (int *)malloc(1024);
	for (int termid = 0; termid < TERM_NUM; ++termid)
	{
		listLength = dict.m_vecDict[termid].m_nFreq;
		offset = dict.m_vecDict[termid].m_nOffset;
		space = (int *)realloc(space, listLength * sizeof(int));
		fseek(pIndex, offset, SEEK_SET);
		fread(space, sizeof(int), listLength, pIndex);
		if (listLength >= range)
		{
			cerr << termid << "\t" << docid << "\t-\t" 
				<< space[docid] << "\t" << listLength << endl;
		}

	}
	freeResource(space);

	delete CF;
	free_hash();

	return 0;
}
