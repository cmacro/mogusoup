//
//  
//  mornlight
// 
//  Created by SunSeed on 17-04-22.
//  Copyright (c) 2017 SunSeed. All rights reserved.
//


#ifndef _ML_PALLOC_H_
#define _ML_PALLOC_H_

#include "mlconfig.h"
#include "mlalloc.h"



/*
 * ML_MAX_ALLOC_FROM_POOL should be (ml_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define ML_MAX_ALLOC_FROM_POOL  (ml_pagesize - 1)

#define ML_DEFAULT_POOL_SIZE    (16 * 1024)

#define ML_POOL_ALIGNMENT       16
#define ML_MIN_POOL_SIZE                                                \
            ml_align((sizeof(mlPool) + 2 * sizeof(mlPoolLarge)),        \
                     ML_POOL_ALIGNMENT)


typedef void (*mlpool_cleanupFunc)(void *data);

typedef struct _mlPool          mlPool;
typedef struct _mlPoolCleanup   mlPoolCleanup;
struct _mlPoolCleanup {
    mlpool_cleanupFunc      handler;
    void                   *data;
    mlPoolCleanup          *next;
};


typedef struct _mlPoolLarge  mlPoolLarge;
struct _mlPoolLarge {
    mlPoolLarge     *next;
    void            *alloc;
};

typedef struct {
    byte           *last;
    byte           *end;
    mlPool         *next;
    uint            failed;
} mlPoolData;


struct _mlPool {
    mlPoolData          d;
    size_t              max;
    mlPool             *current;
    // mlChain            *chain;
    mlPoolLarge        *large;
    mlPoolCleanup      *cleanup;
    // mlLog              *log;
};


void *  ml_lalloc(size_t size);
void *  ml_lcalloc(size_t size);

mlPool *mlpool_create(size_t size);
void    mlpool_destroy(mlPool *pool);
void    mlpool_reset(mlPool *pool);

void *  ml_palloc(mlPool *pool, size_t size);
void *  ml_pnalloc(mlPool *pool, size_t size);
void *  ml_pcalloc(mlPool *pool, size_t size);
void *  ml_pmemalign(mlPool *pool, size_t size, size_t alignment);
int     ml_pfree(mlPool *pool, void *p);

#define mlpool_alloc(p, type)  (type *)ml_palloc(p, sizeof(type))
#define mlpool_calloc(p, type)  (type *)ml_pcalloc(p, sizeof(type))

char * ml_pstrdup(mlPool *pool, const char * source);
char * ml_pstrndup(mlPool *pool, const char * source, size_t len);


mlPoolCleanup *mlpool_cleanup_add(mlPool *p, size_t size);
void    mlpool_cleanup_file(void *data);
void    mlpool_delete_file(void *data);




#endif /* _ML_PALLOC_H_ */
