#
# Copyright 1991-1998 by Open Software Foundation, Inc. 
#              All Rights Reserved 
#  
# Permission to use, copy, modify, and distribute this software and 
# its documentation for any purpose and without fee is hereby granted, 
# provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation. 
#  
# OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE. 
#  
# IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
# LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
# NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
#
#
# MkLinux

VPATH		= ..:../..

MIGFLAGS	= -MD ${IDENT}
MIGKSFLAGS	= -DKERNEL_SERVER
MIGKUFLAGS	= -DKERNEL_USER

DEVICE_FILES	= device_server.h device_server.c
DEVICE_REPLY_FILES = device_reply.h device_reply_user.c
DEVICE_PAGER_FILES = device_pager_server.h device_pager_server.c
NORMA_DEVICE_FILES = dev_forward.h dev_forward.c


OTHERS		= ${DEVICE_FILES} ${DEVICE_REPLY_FILES} ${DEVICE_PAGER_FILES} \
		  ${NORMA_DEVICE_FILES}

INCFLAGS	= -I.. -I../..
MDINCFLAGS	= -I.. -I../..

DEPENDENCIES	=

.include <${RULES_MK}>

.ORDER: ${DEVICE_FILES}

${DEVICE_FILES}: device/device.defs
	${_MIG_} ${_MIGFLAGS_} ${MIGKSFLAGS}		\
		-header /dev/null			\
		-user /dev/null				\
		-sheader device_server.h		\
		-server device_server.c			\
                ${device/device.defs:P}

.ORDER: ${DEVICE_REPLY_FILES}

${DEVICE_REPLY_FILES}: device/device_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${MIGKUFLAGS}		\
		-header device_reply.h			\
		-user device_reply_user.c		\
		-server /dev/null			\
		${device/device_reply.defs:P}

.ORDER: ${DEVICE_PAGER_FILES}

${DEVICE_PAGER_FILES}: mach/memory_object.defs
	${_MIG_} ${_MIGFLAGS_} ${MIGKSFLAGS}		\
		-header /dev/null			\
		-user /dev/null				\
		-sheader device_pager_server.h		\
		-server device_pager_server.c		\
		${mach/memory_object.defs:P}

.ORDER: ${NORMA_DEVICE_FILES}

${NORMA_DEVICE_FILES}: device/dev_forward.defs
	${_MIG_} ${_MIGFLAGS_} ${MIGKUFLAGS}		\
		-header dev_forward.h			\
		-user dev_forward.c			\
		-server /dev/null			\
                ${device/dev_forward.defs:P}

.if exists(depend.mk)
.include "depend.mk"
.endif
