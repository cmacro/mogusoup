//
//  mlalloc.h
//  mornlight
//
//  Created by SunSeed on 17-04-21.
//  Copyright (c) 2017年 SunSeed. All rights reserved.
//
//  windows 内存分配
//

#ifndef _MLALLOC_H_
#define _MLALLOC_H_

#ifdef _WIN32
    #include <malloc.h>
#else
    // clang 
    #include <mm_malloc.h>
#endif

#include <string.h>
#include "mlconfig.h"



#define ml_alloc(size)          	malloc(size)
#define ml_calloc(size)             calloc(1, size)
#define ml_free            	        free
#define ml_realloc(ptr, newsize)	realloc(ptr, newsize)
#define ml_zeromem(ptr, size)       memset((void *)(ptr), 0, size)
#define ml_memzero(buf, n)          memset(buf, 0, n)
#define ml_memset(buf, c, n)        memset(buf, c, n)

#define MLCHECK(p)                          if (!(p)) return NULL
#define MLCHECKALLOC(memitem)              { if ((memitem) == NULL) return NULL; }
#define MLCHECKALLOCCHILD(child, parent)   { if ((child) == NULL) { ml_free(parent); return NULL; } }

void *ml_lalloc(size_t size);
void *ml_lcalloc(size_t size);


#define mlstr_lalloc(n)        ml_lalloc(MLSSIZE(n))
#define mlstr_lcalloc(n)       ml_lcalloc(MLSSIZE(n))        

#define ml_memalign(alignment, size)       ml_lalloc(size)

extern uint  ml_pagesize;
extern uint  ml_pagesize_shift;
extern uint  ml_cacheline_size;


#endif /* _MLALLOC_H_ */
