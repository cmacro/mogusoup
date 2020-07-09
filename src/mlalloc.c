//
//  mlalloc.c
//  mornlight
//
//  Created by SunSeed on 17-04-21.
//  Copyright (c) 2017年 SunSeed. All rights reserved.
//
//  windows 内存分配
//

#include "mlconfig.h"
#include "mlalloc.h"

uint  ml_pagesize;
uint  ml_pagesize_shift;
uint  ml_cacheline_size;


void *ml_lalloc(size_t size)
{
    void  *p;
    p = malloc(size);
    if (p == NULL) {
        // mllog_error(MLLOG_EMERG, log, mlerr_no,
        //               MLS("malloc(%uz) failed"), size);
    }
    // mllog_debug2(MLLOG_DEBUG_ALLOC, log, 0, MLS("malloc: %p:%uz"), p, size);
    return p;
}


void *ml_lcalloc(size_t size)
{
    void  *p = ml_lalloc(size);
    if (p) {
        ml_memzero(p, size);
    }
    return p;
}
