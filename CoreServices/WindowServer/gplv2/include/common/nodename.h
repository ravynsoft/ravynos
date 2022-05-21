/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_NODENAME_H
#define __LABWC_NODENAME_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>

/**
 * nodename - give xml node an ascii name
 * @node: xml-node
 * @buf: buffer to receive the name
 * @len: size of buffer
 *
 * For example, the xml structure <a><b><c></c></b></a> would return the
 * name c.b.a
 */
char *nodename(xmlNode *node, char *buf, int len);

#endif /* __LABWC_NODENAME_H */
