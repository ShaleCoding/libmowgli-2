/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_queue.c: Double-linked queues.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
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
 */

#include "mowgli.h"

mowgli_queue_t *
mowgli_queue_append(mowgli_queue_t *head, void *data)
{
	mowgli_queue_t *out = mowgli_alloc(sizeof(mowgli_queue_t));

	out->next = head;
	out->data = data;

	if (head != NULL)
		head->prev = out;

	return out;
}

mowgli_queue_t *
mowgli_queue_remove(mowgli_queue_t *head)
{
	mowgli_queue_t *out;

	if (head->prev != NULL)
		head->prev->next = head->next;

	if (head->next != NULL)
		head->next->prev = head->prev;

	out = head->prev != NULL ? head->prev : head->next;

	free(head);

	return out;
}

mowgli_queue_t *
mowgli_queue_find(mowgli_queue_t *head, void *data)
{
	mowgli_queue_t *n;

	for (n = head; n != NULL; n = n->next)
		if (n->data == data)
			return n;

	return NULL;
}

mowgli_queue_t *
mowgli_queue_remove_data(mowgli_queue_t *head, void *data)
{
	mowgli_queue_t *n = mowgli_queue_find(head, data);

	if (n != NULL)
		return mowgli_queue_remove(n);

	return NULL;
}

void
mowgli_queue_free(mowgli_queue_t *head)
{
	mowgli_queue_t *n, *n2;

	for (n = head, n2 = n ? n->next : NULL; n != NULL; n = n2, n2 = n ? n->next : NULL)
		mowgli_queue_remove(n);
}