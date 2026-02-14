/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * NOTE: This header files defines a trimmed down version of the BSD sys/queue.h
 * macros for use on platforms which do not come with a sys/queue.h file.
 */

#ifndef __DISPATCH_SHIMS_SYS_QUEUE__
#define __DISPATCH_SHIMS_SYS_QUEUE__

#ifndef TRASHIT
#define TRASHIT(elem) (elem) = NULL;
#endif

#define TAILQ_HEAD(list_name, elem_type) \
	struct list_name { \
		struct elem_type *tq_first; \
		struct elem_type *tq_last; \
	}

#define TAILQ_ENTRY(elem_type) \
	struct { \
		struct elem_type *te_next; \
		struct elem_type *te_prev; \
	}

#define TAILQ_INIT(list) do { \
		(list)->tq_first = NULL; \
		(list)->tq_last = NULL; \
	} while (0)

#define TAILQ_EMPTY(list) ((list)->tq_first == NULL)

#define TAILQ_FIRST(list) ((list)->tq_first)

#define TAILQ_LAST(list) ((list)->tq_last)

#define TAILQ_NEXT(elem, field) ((elem)->field.te_next)

#define TAILQ_PREV(elem, list, field) ((elem)->field.te_prev)

#define TAILQ_FOREACH(var, list, field) \
	for ((var) = TAILQ_FIRST(list); \
	(var) != NULL; \
	(var) = TAILQ_NEXT(var, field))

#define TAILQ_REMOVE(list, elem, field) do { \
		if (TAILQ_NEXT(elem, field) != NULL) { \
			TAILQ_NEXT(elem, field)->field.te_prev = (elem)->field.te_prev; \
		} else { \
			(list)->tq_last = (elem)->field.te_prev; \
		} \
		if (TAILQ_PREV(elem, list, field) != NULL) { \
			TAILQ_PREV(elem, list, field)->field.te_next = (elem)->field.te_next; \
		} else { \
			(list)->tq_first = (elem)->field.te_next; \
		} \
		TRASHIT((elem)->field.te_next); \
		TRASHIT((elem)->field.te_prev); \
	} while(0)

#define TAILQ_INSERT_TAIL(list, elem, field) do { \
		if (TAILQ_EMPTY(list)) { \
			(list)->tq_first = (list)->tq_last = (elem); \
			(elem)->field.te_prev = (elem)->field.te_next = NULL; \
		} else { \
			(elem)->field.te_next = NULL; \
			(elem)->field.te_prev = (list)->tq_last; \
			TAILQ_LAST(list)->field.te_next = (elem); \
			(list)->tq_last = (elem); \
		} \
	} while(0)

#endif // __DISPATCH_SHIMS_SYS_QUEUE__
