/*
 * Copyright 2012 GRNET S.A. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GRNET S.A. ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GRNET S.A OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and
 * documentation are those of the authors and should not be
 * interpreted as representing official policies, either expressed
 * or implied, of GRNET S.A.
 */

#ifndef __PHASH_H__
#define __PHASH_H__

#ifndef __KERNEL__
#include <stdbool.h>
#endif

#include <sys/util.h>
#include <xtypes/domain.h>
/* based on: 
 *
 * python hash for C
 *  originally by gtsouk@cslab.ece.ntua.gr
 *  -- kkourt@cslab.ece.ntua.gr
 */

typedef uint64_t xhashidx;
#define Noxhashidx ((xhashidx) -1)

#define XHASH_ERESIZE 1
#define XHASH_EEXIST 2
#define XHASH_ENOSPC 3
#define XHASH_ENOENT 4

enum xhash_type {
	INTEGER = 0,	/* signed/unsigned integers, pointers, etc */
	STRING = 1	/* NULL terminated strings */
	//OBJECT = 2, to be used later with objects
};
#define NR_XHASH_TYPES 2

struct xhash {
    xhashidx size_shift;
    xhashidx minsize_shift;
    xhashidx used;
    xhashidx dummies;
    xhashidx defval;
    xhashidx limit;
    enum xhash_type type;
#ifdef PHASH_STATS
    xhashidx inserts;
    xhashidx deletes;
    xhashidx lookups;
    xhashidx bounces;
#endif
    XPTR_TYPE(xhashidx) kvs;
};
typedef struct xhash xhash_t;

static inline xhashidx
xhash_elements(xhash_t *xhash)
{
    return xhash->used;
}

static inline xhashidx
xhash_size(xhash_t *xhash)
{
    return 1UL<<(xhash->size_shift);
}

static inline xhashidx *
xhash_kvs(xhash_t *xhash)
{
    return XPTR(&xhash->kvs);
}

static inline xhashidx *
xhash_vals(xhash_t *xhash)
{
    return XPTR(&xhash->kvs) + xhash_size(xhash);
}

#ifdef PHASH_STATS
#define ZEROSTAT(stat) (stat) = 0
#define INCSTAT(stat) (stat) ++
#define DECSTAT(stat) (stat) --
#define REPSTAT(stat)  fprintf(stderr, "" # stat  " = %lu \n" , stat)
#else
#define ZEROSTAT(stat)
#define INCSTAT(stat)
#define DECSTAT(stat)
#define REPSTAT(stat)  do { } while (0)
#endif

#define REPSTATS(x) do {     \
    REPSTAT(x->inserts); \
    REPSTAT(x->deletes); \
    REPSTAT(x->lookups); \
    REPSTAT(x->bounces); \
} while (0)



/**
 * hash functions (dict)
 */

xhashidx xhash_grow_size_shift(xhash_t *xhash);
xhashidx xhash_shrink_size_shift(xhash_t *xhash);
ssize_t xhash_get_alloc_size(xhashidx size_shift);

xhash_t *xhash_new(xhashidx minsize_shift, xhashidx limit, enum xhash_type type);
void xhash_free(xhash_t *xhash); // pairs with _new()
void xhash_init(struct xhash *xhash, xhashidx minsize_shift, xhashidx limit,
		enum xhash_type type);

xhash_t * xhash_resize(xhash_t *xhash, xhashidx new_size_shift,
		xhashidx newlimit, xhash_t *newxhash);
int xhash_insert(xhash_t *xhash, xhashidx key, xhashidx val);
int xhash_update(xhash_t *xhash, xhashidx key, xhashidx val);
int xhash_freql_update(xhash_t *xhash, xhashidx key, xhashidx val);
int xhash_delete(struct xhash *xhash, xhashidx key);
int xhash_lookup(xhash_t *xhash, xhashidx key, xhashidx *val);

struct xhash_iter {
    xhashidx   loc;  /* location on the array */
    xhashidx   cnt;  /* returned items */
};
typedef struct xhash_iter xhash_iter_t;

/* The iterators are read-only */
void xhash_iter_init(xhash_t *xhash, xhash_iter_t *pi);
int  xhash_iterate(xhash_t *xhash, xhash_iter_t *pi, xhashidx *key, xhashidx *val);

void xhash_print(xhash_t *xhash);

/* no set functionality for now */
#if 0
/**
 * set funtions
 */
struct pset {
    xhash_t ph_;
};
typedef struct pset pset_t;

static inline xhashidx
pset_elements(pset_t *pset)
{
    return pset->ph_.used;
}

static inline xhashidx
pset_size(pset_t *pset)
{
    return 1UL<<(pset->ph_.size_shift);
}

pset_t *pset_new(xhashidx minsize_shift); // returns an initialized pset
void pset_free(pset_t *pset); // goes with _new()

void pset_init(pset_t *pset, xhashidx minsize_shift);
void pset_tfree(pset_t *pset); // goes with _init()

void pset_insert(pset_t *pset, xhashidx key);
int pset_delete(pset_t *pset, xhashidx key);
bool pset_lookup(pset_t *pset, xhashidx key);
int pset_iterate(pset_t *pset, xhash_iter_t *pi, xhashidx *key);
void pset_print(pset_t *pset);

typedef xhash_iter_t pset_iter_t;

static inline void
pset_iter_init(pset_t *pset, pset_iter_t *pi)
{
    xhash_iter_init(&pset->ph_, pi);
}

#endif /* if 0 */

#endif

// vim:expandtab:tabstop=8:shiftwidth=4:softtabstop=4
