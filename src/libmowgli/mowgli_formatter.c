/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_formatter.c: Reusable formatting.
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

static void mowgli_formatter_format_internal(char *buf, size_t bufstr, const char *fmtstr, const char *descstr, va_list va)
{
	char scratch[64];
	size_t pos = 0;
	char *i = buf;
	const char *fiter = fmtstr;
	mowgli_argstack_t *stack;

	return_if_fail(buf != NULL);
	return_if_fail(fmtstr != NULL);
	return_if_fail(descstr != NULL);

	*i = '\0';

	while (*fiter && pos <= bufstr)
	{
		int arg;
		mowgli_argstack_element_t *e;

		pos = strlen(buf);

		switch(*fiter)
		{
		case '%':
			fiter++;
			arg = atoi(fiter);
			e = mowgli_node_nth_data(&stack->stack, arg - 1);

			while (isdigit(*fiter)) fiter++;

			if (e == NULL)
			{
				arg = snprintf(i, bufstr - (i - buf), "(unknown)");
				i += arg;
				continue;
			}

			switch(e->type)
			{
			case MOWGLI_ARG_STRING:
				arg = snprintf(i, bufstr - (i - buf), "%s", e->data.string);
				i += arg;
				break;
			case MOWGLI_ARG_NUMERIC:
				arg = snprintf(i, bufstr - (i - buf), "%d", e->data.numeric);
				i += arg;
				break;
			case MOWGLI_ARG_POINTER:
				arg = snprintf(i, bufstr - (i - buf), "%p", e->data.pointer);
				i += arg;
				break;
			case MOWGLI_ARG_BOOLEAN:
				arg = snprintf(i, bufstr - (i - buf), "%s", e->data.boolean ? "TRUE" : "FALSE");
				i += arg;
				break;
			default:
				mowgli_throw_exception(mowgli.formatter.unhandled_type_exception);
				break;
			}

			continue;
			break;
		default:
			*i = *fiter;
		}

		i++;
		fiter++;
	}
}

void mowgli_formatter_format(char *buf, size_t bufstr, const char *fmtstr, const char *descstr, ...)
{
	va_list va;

	va_start(va, descstr);
	mowgli_formatter_format_internal(buf, bufstr, fmtstr, descstr, va);
	va_end(va);
}

void mowgli_formatter_print(const char *fmtstr, const char *descstr, ...)
{
	va_list va;
	char buf[65535];

	va_start(va, descstr);
	mowgli_formatter_format_internal(buf, 65535, fmtstr, descstr, va);
	va_end(va);

	printf("%s", buf);
}