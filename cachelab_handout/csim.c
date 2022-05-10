// name: 김영인, loginID: 2017-14384
#include "cachelab.h"
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

void simulate(unsigned long long int address);

typedef struct {
    int valid;
    unsigned long long tag;
    unsigned long long lru;
} cacheline;

typedef cacheline* cacheset;
typedef cacheset* cache;

//global variables
int s = 0, E = 0, b = 0;
int hits = 0;
int misses = 0;
int evictions = 0;
unsigned long long int setidx;
cache c;

int main(int argc, char *argv[])
{
    char *t = NULL;
    FILE *file;
    int option;
    unsigned long long address;
    char kind;
    int size;
    //command line parsing
    while((option = getopt(argc, argv, "s:E:b:t:")) != -1) {
	switch (option) {
	    case 's':
		s = atoi(optarg);;
		break;
	    case 'E':
		E = atoi(optarg);
		break;
	    case 'b':
		b = atoi(optarg);
		break;
	    case 't':
		t = optarg;
		break;
	    default:
		exit(1);
	}
    }
    //command line에 있어야하는 값이 없으면 종료
    if(s==0 || E==0 || b==0 || t==NULL) {
        exit(1);
    }
    //Set 개수
    int S = pow(2, s);
    //나중에 set index 만을 추출하기 위해 아래와 같은 변수를 정의(s개 bit가 모두 1인 수)
    setidx = (unsigned long long int)(S - 1);

    // malloc 이용해서 cache 초기화
    c = (cacheset*)malloc(sizeof(cacheset) * S);
    for(int i=0; i<S; i++) {
	c[i] = (cacheline*)malloc(sizeof(cacheline)*E);
	for(int j=0; j<E; j++) {
	    c[i][j].valid = 0;
	    c[i][j].tag = 0;
	    c[i][j].lru = 0;
	}
    }

    // trace 파일 열어서 parsing 후, 경우에 따라 적절하게 처리
    // 'I' 는 무시
    // 'L'과 'S'는 메모리 주소에 있는 data에 한번 접근-> simulate 함수 한번 호출
    // 'M'은 load followed by a stroe to the same address로 간주 -> simulate 함수 두번 호출
    file = fopen(t, "r");
    while(fscanf(file, " %c %llx,%d", &kind, &address, &size) == 3) {
	switch(kind) {
	    case 'L':
		simulate(address);
	    	break;
	    case 'S':
		simulate(address);
		break;
	    case 'M':
		simulate(address);
		simulate(address);
		break;
	    default:
		break;
	}
    }
    fclose(file);

    // cache free    
    for(int i=0; i<S; i++) {
	free(c[i]);
    }
    free(c);

    //print
    printSummary(hits, misses, evictions);

    return 0;
}

// 메모리 주소에 있는 data에 접근하는 함수 
// 'L', 'S', 'M'의 역할을 직접적으로 수행하는 파트
void simulate(unsigned long long int address) {
    unsigned long long int seti = (setidx & (address >> b)); // setidx를 이용하여 set index만 추출
    cacheset cset = c[seti]; // 해당 set index를 가진 cache set 
    static unsigned long long int lrunum = 0; // LRU replacement policy를 위한 변수
    unsigned long long int tag = (address >> (s+b)); // tag만 추출
    unsigned long long int eviclru = ULONG_MAX; // 가장 작은 lru를 갖는 block을 찾기 위해 lru 값을 저장하는 변수
    unsigned int evicline = 0; // 가장 작은 lru 값을 가지는  block의 index

    for(int i=0; i<E; i++) {
	// valid bit이 1이고, tag가 동일하면 cache hit
	if((cset[i].valid) && (cset[i].tag == tag)) {
	     hits++;
	     // cache hit가 발생하면 lrunum을 배정해준다 -> 나중에 사용
	     cset[i].lru = ++lrunum; 
	     return;
	}
    }

    // 위 경우가 아니라면 cache miss
    misses++;
    
    // cache hit에 실패했으므로 빈 cache 자리를 찾기 위해 cache set을 순회(lru replacement policy 적용)
    // cache set 내부에서 가장 작은lru 값을 갖는 block을 찾는다
    for(int i=0; i<E; i++) {
	if(eviclru > cset[i].lru) {
	    eviclru = cset[i].lru;
	    evicline = i;
	}
    }

    // 위에서 찾은 index에 해당하는 block은 가장작은 lru 값을 가지고 있다. 만약 이 block의 valid bit가 1이면 eviction 진행
    if(cset[evicline].valid) {
	evictions++;
    }

    // 찾은 자리에 새로운 block을 setting 한다.
    cset[evicline].valid = 1;
    cset[evicline].tag = tag;
    cset[evicline].lru = lrunum++;
}



