//
//  
//  mornlight
// 
//  Created by SunSeed on 17-04-22.
//  Copyright (c) 2017 SunSeed. All rights reserved.
//


#include "mlconfig.h"
#include "mlpalloc.h"

static void *ml_palloc_block(mlPool *pool, size_t size);
static void *ml_palloc_large(mlPool *pool, size_t size);


mlPool *
mlpool_create(size_t size)
{
    mlPool  *p;

    p = ml_memalign(ML_POOL_ALIGNMENT, size);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (byte *) p + sizeof(mlPool);
    p->d.end = (byte *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(mlPool);
    p->max = (size < ML_MAX_ALLOC_FROM_POOL) ? size : ML_MAX_ALLOC_FROM_POOL;

    p->current = p;
    // p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;

    return p;
}


void
mlpool_destroy(mlPool *pool)
{
    mlPool          *p, *n;
    mlPoolLarge    *l;
    mlPoolCleanup  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            // mllog_debug1(MLLOG_DEBUG_ALLOC, pool->log, 0,
                        //  MLS("run cleanup: %p"), c);
            c->handler(c->data);
        }
    }

    for (l = pool->large; l; l = l->next) {
        // mllog_debug1(MLLOG_DEBUG_ALLOC, pool->log, 0, MLS("free: %p"), l->alloc);
        if (l->alloc) {
            ml_free(l->alloc);
        }
    }

#if (ML_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        mllog_debug2(MLLOG_DEBUG_ALLOC, pool->log, 0,
                     MLS("free: %p, unused: %uz"), p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ml_free(p);

        if (n == NULL) {
            break;
        }
    }
}


void
mlpool_reset(mlPool *pool)
{
    mlPool        *p;
    mlPoolLarge  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ml_free(l->alloc);
        }
    }

    for (p = pool; p; p = p->d.next) {
        p->d.last = (uchar *) p + sizeof(mlPool);
        p->d.failed = 0;
    }

    pool->current = pool;
    // pool->chain = NULL;
    pool->large = NULL;
}


void *  
ml_palloc(mlPool *pool, size_t size) {
    uchar   *m;
    mlPool  *p;

    if (size <= pool->max) {
        p = pool->current;
        do {
            m = ml_align_ptr(p->d.last, ML_ALIGNMENT);

            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;
                return m;
            }
            p = p->d.next;

        } while (p);

        return ml_palloc_block(pool, size);
    }

    return ml_palloc_large(pool, size);
}


void *
ml_pnalloc(mlPool *pool, size_t size)
{
    uchar       *m;
    mlPool      *p;

    if (size <= pool->max) {
        p = pool->current;
        do {
            m = p->d.last;
            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;
                return m;
            }

            p = p->d.next;

        } while (p);
        return ml_palloc_block(pool, size);
    }

    return ml_palloc_large(pool, size);
}


static void *
ml_palloc_block(mlPool *pool, size_t size)
{
    uchar      *m;
    size_t       psize;
    mlPool  *p, *new;

    psize = (size_t) (pool->d.end - (uchar *) pool);

    m = ml_memalign(ML_POOL_ALIGNMENT, psize);
    if (m == NULL) {
        return NULL;
    }

    new = (mlPool *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(mlPoolData);
    m = ml_align_ptr(m, ML_ALIGNMENT);
    new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    p->d.next = new;

    return m;
}


static void *
ml_palloc_large(mlPool *pool, size_t size)
{
    void        *p;
    uint         n;
    mlPoolLarge *large;

    p = ml_lalloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = ml_palloc(pool, sizeof(mlPoolLarge));
    if (large == NULL) {
        ml_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


void *
ml_pmemalign(mlPool *pool, size_t size, size_t alignment)
{
    void         *p;
    mlPoolLarge  *large;

    p = ml_memalign(alignment, size);
    if (p == NULL) {
        return NULL;
    }

    large = ml_palloc(pool, sizeof(mlPoolLarge));
    if (large == NULL) {
        ml_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


int
ml_pfree(mlPool *pool, void *p)
{
    mlPoolLarge  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            // mllog_debug1(MLLOG_DEBUG_ALLOC, pool->log, 0,
            //              MLS("free: %p"), l->alloc);
            ml_free(l->alloc);
            l->alloc = NULL;

            return ML_OK;
        }
    }

    return ML_DECLINED;
}


void *
ml_pcalloc(mlPool *pool, size_t size)
{
    void *p;

    p = ml_palloc(pool, size);
    if (p) {
        ml_memzero(p, size);
    }

    return p;
}


mlPoolCleanup *
mlpool_cleanup_add(mlPool *p, size_t size)
{
    mlPoolCleanup  *c;

    c = ml_palloc(p, sizeof(mlPoolCleanup));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = ml_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }
    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;
    p->cleanup = c;

    // mllog_debug1(MLLOG_DEBUG_ALLOC, p->log, 0, MLS("add cleanup: %p"), c);

    return c;
}


#if 0

static void *
ml_get_cached_block(size_t size)
{
    void                     *p;
    ml_cached_block_slot_t  *slot;

    if (ml_cycle->cache == NULL) {
        return NULL;
    }

    slot = &ml_cycle->cache[(size + ml_pagesize - 1) / ml_pagesize];

    slot->tries++;

    if (slot->number) {
        p = slot->block;
        slot->block = slot->block->next;
        slot->number--;
        return p;
    }

    return NULL;
}

#endif


char * ml_pstrndup(mlPool *pool, const char * source, size_t len){
    char  *dst;
    if (len <= 0) {
        return NULL;
    }    
    dst = ml_pnalloc(pool, (len + 1) * sizeof(char));
    if (!dst) {
        return NULL;
    }
    memcpy(dst, source, len * sizeof(char));
    dst[len] = '\0';
    return dst;
}

char * ml_pstrdup(mlPool *pool, const char * source){
    char  *dst;
    size_t size = (strlen(source) + 1) * sizeof(char);
    
    dst = ml_pnalloc(pool, size);
    if (!dst) {
        return NULL;
    }
    memcpy(dst, source, size);
    return dst;
}



// mlchar *
// mlstr_pdup(mlPool *pool, mlStr *src) {
//     mlchar  *dst;

//     dst = ml_pnalloc(pool, src->len * sizeof(mlchar));
//     if (dst == NULL) {
//         return NULL;
//     }

//     mlstr_memcpy(dst, src->data, src->len);

//     return dst;
// }

