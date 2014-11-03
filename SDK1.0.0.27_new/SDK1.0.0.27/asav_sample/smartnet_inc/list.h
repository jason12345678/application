#ifndef __list_h__
#define	__list_h__

#include <stdlib.h>
#include <assert.h>


typedef struct __dlink dlink;
struct __dlink
{
	dlink *next, *prev;
}; 

typedef struct __d2list_head d2list_head;
struct __d2list_head
{
	dlink *front, *back;
};

static __inline void d2list_init(d2list_head *head)
{ head->front = head->back = 0; }

static __inline void d2list_cat(d2list_head *dest, d2list_head *src)
{
    if (NULL != dest->back) {
        dest->back->next = src->front;
    } else {
        dest->front = src->front;
    }
    dest->back = src->back;
}

static __inline boolean d2list_empty(const d2list_head *head)
{ return head->front == 0; }

static __inline dlink *d2list_front(const d2list_head *head)
{ return head->front; }

static __inline dlink *d2list_back(d2list_head *head)
{ return head->back; }

static __inline dlink *d2list_next(dlink *node)
{ return node->next; }

static __inline dlink *d2list_prev(dlink *node)
{ return node->prev; }

static __inline void d2list_insert_after(d2list_head *head, dlink *prev, dlink *node)
{
    if ((node->next = prev->next) != 0) {
		node->next->prev = node;
    } else {
		head->back = node;
    }
	prev->next = node;
	node->prev = prev;
}

static __inline void d2list_insert_before(d2list_head *head, dlink *next, dlink *node)
{
    if ((node->prev = next->prev) != 0) {
		node->prev->next = node;
    } else {
		head->front = node;
    }
	next->prev = node;
	node->next = next;
}

static __inline void d2list_remove(d2list_head *head, dlink *node)
{
    if (node->next != 0) {
		node->next->prev = node->prev;
    } else {
		head->back = node->prev;
    }
    if (node->prev != 0) {
		node->prev->next = node->next;
    } else {
		head->front = node->next;
    }
}

static __inline void d2list_push_front(d2list_head *head, dlink *node)
{
    if ((node->next = head->front) != 0) {
		head->front->prev = node;
    } else {
		head->back = node;
    }
	head->front = node;
	node->prev = 0;
}

static __inline void d2list_push_back(d2list_head *head, dlink *node)
{
    if ((node->prev = head->back) != 0) {
		head->back->next = node;
    } else {
		head->front = node;
    }
	head->back = node;
	node->next = 0;
}

static __inline void d2list_pop_front(d2list_head *head)
{
    if ((head->front = head->front->next) == 0) {
		head->back = 0;
    } else {
		head->front->prev = 0;
    }
}

static __inline void d2list_pop_back(d2list_head *head)
{
    if ((head->back = head->back->prev) == 0) {
		head->front = 0;
    } else {
		head->back->next = 0;
    }
}

#define MEMBER_ENTRY(ptr, type, field) \
            ((type*)((char*)(ptr)-(long long)(&((type *)0)->field)))

#define BIT_31      (1UL << 31)
#define BIT_30      (1L << 30)
#define BIT_29      (1L << 29)
#define BIT_28      (1L << 28)
#define BIT_27      (1L << 27)
#define BIT_26      (1L << 26)
#define BIT_25      (1L << 25)
#define BIT_24      (1L << 24)
#define BIT_23      (1L << 23)
#define BIT_22      (1L << 22)
#define BIT_21      (1L << 21)
#define BIT_20      (1L << 20)
#define BIT_19      (1L << 19)
#define BIT_18      (1L << 18)
#define BIT_17      (1L << 17)
#define BIT_16      (1L << 16)
#define BIT_15      (1L << 15)
#define BIT_14      (1L << 14)
#define BIT_13      (1L << 13)
#define BIT_12      (1L << 12)
#define BIT_11      (1L << 11)
#define BIT_10      (1L << 10)
#define BIT_9       (1L << 9)
#define BIT_8       (1L << 8)
#define BIT_7       (1L << 7)
#define BIT_6       (1L << 6)
#define BIT_5       (1L << 5)
#define BIT_4       (1L << 4)
#define BIT_3       (1L << 3)
#define BIT_2       (1L << 2)
#define BIT_1       (1L << 1)
#define BIT_0       1L

#define FLAG(_off)           ((UINT32)1 << (_off))
#define MASK(_off, _bits)     ((((UINT32)1 << (_bits)) - 1) << (_off))
#define FIELD(_off, _bits, _x) ((((UINT32)_x) << (_off)) & MASK((_off),(_bits)))

#define BIT_SET(_val, _off) \
    ( (_val) |= FLAG(_off) )
#define BIT_CLEAR(_val, _off) \
    ( (_val) &= ~FLAG(_off) )
#define BIT_TEST(_val, _off) \
    (0 != ( (_val) & FLAG(_off) ) ) 

#endif /* __list_h__ */

