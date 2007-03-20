/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_heap.c: Heap allocation.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 * Copyright (c) 2005-2006 Theo Julienne <terminal -at- atheme.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Legal note: code devised from claro.base.block module r288 (Pre MPL)
 */

#include "mowgli.h"

/* expands a mowgli_heap_t by 1 block */
void mowgli_heap_expand(mowgli_heap_t *bh)
{
	mowgli_block_t *block;
	void *blp;
	mowgli_node_t *node;
	void *offset;
	int a;
	
	blp = mowgli_alloc(sizeof(mowgli_heap_t) + (bh->alloc_size * bh->mowgli_heap_elems));
	block = (mowgli_block_t *)blp;
	
	offset = blp + sizeof(mowgli_heap_t);
	block->data = offset;

	for (a = 0; a < bh->mowgli_heap_elems; a++)
	{
		node = (mowgli_node_t *)offset;
		node->prev = node->next = NULL;

		mowgli_node_add(offset + sizeof(mowgli_node_t), node, &block->free_list);

		offset += bh->alloc_size;
	}
	
	mowgli_node_add(block, (mowgli_node_t *) block, &bh->blocks);
	
	bh->free_elems += bh->mowgli_heap_elems;
}

/* creates a new mowgli_heap_t */
mowgli_heap_t *mowgli_heap_create(size_t elem_size, size_t mowgli_heap_elems, unsigned int flags)
{
	mowgli_heap_t *bh = mowgli_alloc(sizeof(mowgli_heap_t));
	
	bh->elem_size = elem_size;
	bh->mowgli_heap_elems = mowgli_heap_elems;
	bh->free_elems = 0;
	
	bh->alloc_size = bh->elem_size + sizeof(mowgli_node_t);
	
	bh->flags = flags;
	
	if ( flags & BH_NOW )
		mowgli_heap_expand( bh );
	
	return bh;
}

/* completely frees a mowgli_heap_t and all blocks */
void mowgli_heap_destroy(mowgli_heap_t *heap)
{
	mowgli_node_t *n, *tn;
	
	MOWGLI_LIST_FOREACH_SAFE(n, tn, heap->blocks.head)
	{
		mowgli_node_delete(n, &heap->blocks); 
		mowgli_free(n);
	}

	/* everything related to heap has gone, time for itself */
	mowgli_free(heap);
}

/* allocates a new item from a mowgli_heap_t */
void *mowgli_heap_alloc(mowgli_heap_t *heap)
{
	mowgli_node_t *n, *fn;
	mowgli_block_t *b;
	
	/* no free space? */
	if (heap->free_elems == 0)
	{
		mowgli_heap_expand(heap);

		return_val_if_fail(heap->free_elems != 0, NULL);
	}

	MOWGLI_LIST_FOREACH(n, heap->blocks.head)
	{
		b = (mowgli_block_t *) n->data;
		
		if (MOWGLI_LIST_LENGTH(&b->free_list) == 0)
			continue;
		
		/* pull the first free node from the list */
		fn = b->free_list.head;
		
		/* mark it as used */
		mowgli_node_move(fn, &b->free_list, &b->used_list);
		
		/* keep count */
		heap->free_elems--;
		
		/* debug */
		mowgli_log("mowgli_heap_alloc(heap = @%p) -> %p", heap, fn->data);
		
		/* return pointer to it */
		return fn->data;
	}
	
	/* this should never happen */
	mowgli_log("mowgli_heap_alloc() failed to allocate; heap free element count (%d) wrong", heap->free_elems);
	
	return NULL;
}

/* frees an item back to the mowgli_heap_t */
void mowgli_heap_free(mowgli_heap_t *heap, void *data)
{
	mowgli_node_t *n, *dn;
	mowgli_block_t *b;
	
	MOWGLI_LIST_FOREACH(n, heap->blocks.head)
	{
		b = (mowgli_block_t *) n->data;

		/* see if the element belongs to this list */
		dn = mowgli_node_find(data, &b->used_list);

		if (dn != NULL)
		{
			/* mark it as free */
			mowgli_node_move(dn, &b->used_list, &b->free_list);
			
			/* keep count */
			heap->free_elems++;
			
			/* debug */
			mowgli_log("mowgli_heap_free(heap = @%p, data = %p)", heap, data);
			
			/* we're done */
			return;
		}
		
		/* and just in case, check it's not already free'd */
		dn = mowgli_node_find(data, &b->free_list);
		
		return_if_fail(dn == NULL);
	}
	
	/* this should never happen */
	mowgli_log("attempt to mowgli_heap_free() data (%p) which doesn't belong to the heap passed (%p)", data, heap);
}
