/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#undef DEBUG
#include <string.h>
#include <stdio.h>
#include <libc.h>
#include <limits.h>
#include <sys/mman.h>
#include "stuff/errors.h"
#include "gprof.h"

static uint64_t text_min = 0;
static uint64_t text_max = 0;

struct Edgestruct { 
    struct Edgestruct *next1;
    struct Edgestruct *next2;
    struct Edgestruct *prev1;
    struct Edgestruct *prev2;
    struct Edgestruct **pqp;
    int v1;
    int v2;
    int w;
};
typedef struct Edgestruct Edge;

struct TreeNodestruct { 
    Edge *next1;
    Edge *next2; 
    int parent;
    int left;
    int right;
};
typedef struct TreeNodestruct TreeNode;

#define NONE -1
#define SEEN -2

static FILE *gmon = NULL;
static FILE *callf = NULL;
static FILE *callo = NULL;
static FILE *callt = NULL;
#ifdef notdef
static FILE *treefile = NULL;
#endif

static TreeNode *tree = NULL;
static int n_nodes = 0;
static Edge **pq = NULL;
static int n_pq = 0;
static char *whatsloaded = NULL;
static Edge *free_edges = NULL;
static Edge pqzero = { NULL, NULL, NULL, NULL, NULL, 0, 0, INT_MAX };

static void pqinsert(
    Edge *edge);
static Edge *pqremove(
    void);
static void pqdelete(
    Edge *edge);
static void upheap(
    int k);
static void downheap(
    int k);

static
void
do_what(
void)
{
    struct file *afile;
    char *start, *stop, *dest, *whatsloadedp, ar_name[16], *namep;

	for(afile = files; afile < &files[n_files]; afile++){
	    if((namep = strrchr(afile->name, '/')))
		namep++; 
	    else
		namep = afile->name;
	    strncpy(ar_name, namep, 15);
	    ar_name[15] = '\0';
	    whatsloadedp = whatsloaded;
	    while((start = stop = strstr(whatsloadedp, ar_name))){
		if((start == whatsloaded) || (*(start-1) == '(') ||
		   (*(start-1) == '/')) {
		    while((*start != '\n') && (start >= whatsloaded))
			start--;
		    while((*stop != '\n') && (*stop != ')'))
			stop++;
		    afile->what_name = dest = malloc(stop - start);
		    for(start++; start < stop; start++){
			*(dest++) = (*start == '(') ? ':' : *start;
		    }
		    *dest = '\0';
		    break;
		}
		whatsloadedp = start + 1;
	    }
	}
} 

static
char *
find_file(
uint64_t pc)
{
    struct file *afile;
    static int flag = 0;

	for(afile = files; afile < &files[n_files]; afile++){
	    if((pc >= afile->firstpc) && (pc < afile->lastpc)){
		return(afile->what_name);
	    }
	}
	if(flag == 0){
	    fprintf(stderr, "In producing order files can't find module name "
		    "for functions (make sure all modules are compiled -g and "
		    "the program is not already ordered)\n");
	    flag = 1;
	}
	return(NULL);
}

static
int
printp(
nltype *node,
FILE *f)
{
	if(f == callt){
	    if(zflag == FALSE && node->ncall == 0 && node->time == 0)
		return(0);
	    else
		return(1);
	}
	if(node->ncall == 0) /* call count 0 (not called) */
	    return(0);
	if(node->order == 0) /* not ordered (not called) */
	    return(0);
	if(node->value < text_min || node->value > text_max)
	    return(0);
	return(1);
/*
	return(!( (node->ncall == 0) &&
		  (node->selfcalls == 0) &&
		  (node->propself == 0) &&
		  (node->propchild == 0) &&
		  (node->value >= text_min && node->value < text_max) ) );
  return ! ((node->ncall == 0) &&
	    (node->selfcalls == 0) &&
	    (node->propself == 0) &&
	    (node->propchild == 0)
	 && (node->value >= text_min && node->value < text_max));
*/
}

/*
  void indent_node(int node, int level)
  for (i = 0; i < level; i++) fprintf(treefile, "  ");
  fprintf(treefile, "%d %s\n", node, (node < npe - nl) ? nl[node].name : "");
  */

static
void
print_node(
int node)
{
    nltype *nlp;
    char *file;

	if((node == NONE) || (tree[node].parent == SEEN))
	    return;
	if(node < (npe - nl)){
	    nlp = &nl[node];
	    if(printp(nlp, NULL)){
		file = find_file(nlp->value);
		fprintf(gmon, "%s:%s\n", file ? file : "", nlp->name);
	    }
	}
	tree[node].parent = SEEN;
	print_node(tree[node].left);
	print_node(tree[node].right);
}

static
void
print_nl(
FILE *f)
{
    nltype *nlp;
    char *file;

	for(nlp = npe-1; nlp >= nl; nlp--){
	    if(printp(nlp, f)){
		file = find_file(nlp->value);
		fprintf(f, "%s:%s\n", file ? file : "", nlp->name);
	    }
	}
}

static
void print_tree(
void)
{
    int i;

	for(i = n_nodes-1; i >= 0; i--){
	    if(tree[i].parent != SEEN)
		print_node(i);
	}
}

static
Edge *
find_edge(
int v1,
int v2,
int w)
{
    Edge *edge;

	for(edge = tree[v1].next1; edge; edge = edge->next1){
	    if(edge->v2 == v2){
		edge->w += w;
		return(edge);
	    }
	}
	return(NULL);
} 

static
Edge *
make_edge(
int v1,
int v2, 
int w)
{
    Edge *edge;

	if(free_edges){
	    edge = free_edges;
	    free_edges = free_edges->next1;
	}
	else
	    edge = (Edge *)malloc(sizeof(Edge));
	edge->v1 = v1;
	edge->v2 = v2; 
	edge->w = w;

	edge->next1 = tree[v1].next1;
	if(edge->next1)
	    edge->next1->prev1 = edge;
	tree[v1].next1 = edge;
	edge->prev1 = NULL;
	
	edge->next2 = tree[v2].next2; 
	if(edge->next2)
	    edge->next2->prev2 = edge;
	tree[v2].next2 = edge;
	edge->prev2 = NULL;

	edge->pqp = NULL;
	return(edge);
}

static
void
free_edge(
Edge *edge)
{
	if(edge->prev1)
	    edge->prev1->next1 = edge->next1;
	else
	    tree[edge->v1].next1 = edge->next1;

	if(edge->next1)
	    edge->next1->prev1 = edge->prev1;

	if(edge->prev2)
	    edge->prev2->next2 = edge->next2;
	else
	    tree[edge->v2].next2 = edge->next2;

	if(edge->next2)
	    edge->next2->prev2 = edge->prev2;

	edge->next1 = free_edges;
	free_edges = edge;
}

static
int
compare_file(
struct file *x,
struct file *y)
{
	if(x->firstpc < y->firstpc)
	    return(-1);
	else if(x->firstpc > y->firstpc)
	    return(1);
	else
	    return(0);
}

static
int
compare_nl(
nltype *x,
nltype *y)
{
	if(x->ncall < y->ncall)
	    return(-1);
	else if(x->ncall > y->ncall)
	    return(1);
	else
	    return(0);
}

static
int
compare_nl_order(
nltype *x,
nltype *y)
{
	if(x->order > y->order)
	    return(-1);
	else if(x->order < y->order)
	    return(1);
	else
	    return(0);
}

static
int
timecmp(
nltype *npp1,
nltype *npp2)
{
    double timediff;
    int32_t calldiff;

	timediff = npp2->time - npp1->time;
	if(timediff > 0.0)
	    return(-1);
	if(timediff < 0.0)
	    return(1);
	calldiff = npp2->ncall - npp1->ncall;
	if(calldiff > 0)
	    return(-1);
	if(calldiff < 0)
	    return(1);
	return(strcmp(npp2->name, npp1->name));
}

static
void
enum_arcs(
void(*proc)(arctype *arc,
	    arctype *backarc))
{
    nltype *nlp;
    arctype *arcp, *backarc;

	for(nlp = nl; nlp < npe; nlp++){
	    for(arcp = nlp->children; arcp ; arcp = arcp->arc_childlist){
		for(backarc = arcp->arc_childp->children;
		    backarc;
		    backarc = backarc->arc_childlist){
		    if(backarc->arc_childp == nlp){
			if(nlp < arcp->arc_childp)
			    proc(arcp, backarc);
			goto skip;
		    }
		}
		proc(arcp, NULL);
skip:
		continue;
	    }
	}
}

static int most_edges = 0;
static
void
most_edges_proc(
arctype *arc,
arctype *backarc)
{ 
	most_edges++; 
}

static
void
enum_edges(
int v,
int(*proc_e1)(Edge *),
int(*proc_e2)(Edge *))
{
    Edge *edge;
    Edge *next1, *next2;

	edge = tree[v].next1; 
	while(edge){
	    next1 = edge->next1; 
	    if(proc_e1(edge))
		free_edge(edge);
	    edge = next1;
	}
	edge = tree[v].next2; 
	while(edge){
	  next2 = edge->next2;
	  if(proc_e2(edge))
		free_edge(edge);
	  edge = next2;
	}
}

static
int
make_edge_e1(
Edge *edge)
{
	if(edge->pqp){
	    pqdelete(edge); 
	    make_edge(n_nodes, edge->v2, edge->w);
	    return(1);
	}
	else{
	    return(0);
	}
}

static
int
make_edge_e2(
Edge *edge)
{
	if(edge->pqp){
	    pqdelete(edge);
	    make_edge(n_nodes, edge->v1, edge->w);
	    return(1);
	}
	else{
	    return(0);
	}
}

static
int
find_edge_e1(
Edge *edge)
{
	if(edge->pqp){
	    pqdelete(edge); 
	    if(!find_edge(n_nodes, edge->v2, edge->w))
		make_edge(n_nodes, edge->v2, edge->w);
	    return(1);
	}
	else{
	    return(0);
	}
}

static
int
find_edge_e2(
Edge *edge)
{
	if(edge->pqp){
	    pqdelete(edge);
	    if(!find_edge(n_nodes, edge->v1, edge->w))
		make_edge(n_nodes, edge->v1, edge->w);
	    return(1);
	}
	else{
	    return(0);
	}
}

static
int
pqinsert_e1(
Edge *edge)
{
	if(!edge->pqp)
	    pqinsert(edge);
	return(0);
}

static
void
main_loop(
void)
{
    Edge *edge;
  
	while(n_pq > 0){
	    edge = pqremove();
	    
	    /* collapse endpoints into combined node */
	    tree[n_nodes].parent = NONE;
	    tree[n_nodes].left = edge->v1;
	    tree[n_nodes].right = edge->v2;
	    tree[edge->v1].parent = n_nodes;
	    tree[edge->v2].parent = n_nodes;
	    if(n_pq == 0)
		break;
	    
	    /* create edges for combined node */
	    enum_edges(edge->v1, make_edge_e1, make_edge_e2);
	    enum_edges(edge->v2, find_edge_e1, find_edge_e2);
	    enum_edges(n_nodes, pqinsert_e1, NULL);
	    
	    if((n_nodes++ % 50) == 0)
		putc('.', stderr);
	}
	putc(';', stderr);
	putc('\n', stderr);
}

static
void
pqinsert_proc(
arctype *arc,
arctype *backarc)
{
	pqinsert(make_edge(arc->arc_parentp - nl, 
			   arc->arc_childp - nl, 
			   (backarc) ? (arc->arc_count + backarc->arc_count) :
				       (arc->arc_count)));
}

static
void
scatter(
void)
{
    int max_nodes;
    TreeNode *tnode;

	if(xflag == FALSE){
	    max_nodes = 2 * (npe - nl);
	    tree = (TreeNode *)malloc(max_nodes * sizeof(TreeNode));
	    n_nodes = npe - nl;
	    for(tnode = tree; tnode < &tree[max_nodes]; tnode++){
		tnode->next1 = tnode->next2 = NULL;
		tnode->parent = tnode->left = tnode->right = NONE;
	    }
	    most_edges = 1;
	    enum_arcs(most_edges_proc);
	    pq = (Edge **)malloc(most_edges * sizeof(Edge *));
	    n_pq = 0;
	    pq[0] = &pqzero;
	    
	    enum_arcs(pqinsert_proc); /* create undirected graph */
	    main_loop();
	    print_tree();
	}

	/* call frequency order file */
	qsort(nl, npe - nl, sizeof(nltype),  
	      (int(*)(const void *, const void *))compare_nl);
	print_nl(callf);

	/* call order order file */
	qsort(nl, npe - nl, sizeof(nltype),  
	      (int(*)(const void *, const void *))compare_nl_order);
	print_nl(callo);

	/* time order file */
	qsort(nl, npe - nl, sizeof(nltype),  
	      (int (*)(const void *, const void *))timecmp);
	print_nl(callt);
}

void
printscatter(
void)
{
    int what_fd;
    struct stat buf;

	if(xflag == FALSE)
	    if((gmon = fopen("gmon.order", "w")) == NULL)
		system_fatal("can't create file: gmon.order");
	if((callf = fopen("callf.order", "w")) == NULL)
	    system_fatal("can't create file: callf.order");
	if((callo = fopen("callo.order", "w")) == NULL)
	    system_fatal("can't create file: callo.order");
	if((callt = fopen("time.order", "w")) == NULL)
	    system_fatal("can't create file: time.order");
	/*
	if((treefile = fopen("gmon.tree", "w")) == NULL)
	    system_fatal("can't create file: gmon.tree");
	*/
	if((what_fd = open("whatsloaded", O_RDONLY)) >= 0){
	    fstat(what_fd, &buf);
	    whatsloaded = mmap(0, buf.st_size, PROT_READ|PROT_WRITE,
			       MAP_FILE|MAP_PRIVATE, what_fd, 0);
	    do_what();
	}
	get_text_min_max(&text_min, &text_max);

	qsort(files, n_files, sizeof(struct file), 
	      (int(*)(const void *, const void *))compare_file);
	scatter();
	if(xflag == FALSE)
	    fclose(gmon);
	fclose(callf);
	fclose(callo);
	fclose(callt);
	if(what_fd >= 0)
	    close(what_fd);
}

#ifdef notdef
static
void
ugh(void)
{
}

static
voids
print_pq(
void)
{
    int i; Edge *edge;

	for(i = 1; i <= n_pq; i++){
	    edge = pq[i];
	    if(edge->pqp != &pq[i])
		ugh();
	    fprintf(stderr, "%4d %4d %4d\n", i, edge->v1, edge->v2);
	}
}
#endif

static
void
upheap(
int k)
{
    Edge *v;

	v = pq[k];
	while(pq[k/2]->w <= v->w){ 
	    pq[k] = pq[k/2]; 
	    pq[k]->pqp = &pq[k];
	    k = k/2;
	}
	pq[k] = v; 
	pq[k]->pqp = &pq[k];
}

static
void
downheap(
int k)
{
    Edge *v;
    int j;

	v = pq[k];
	while(k <= n_pq/2){
	    j = 2 * k;
	    if(j < n_pq)
		if(pq[j]->w < pq[j+1]->w)
		    j++;
	    if (v->w >= pq[j]->w)
		break;
	    pq[k] = pq[j];
	    pq[k]->pqp = &pq[k];
	    k = j;
	}
	pq[k] = v;
	pq[k]->pqp = &pq[k];
}

static
void
pqinsert(
Edge *v)
{
#ifdef DEBUG
	printf("pqinsert %d	%x  %d  %d\n",
	       n_pq, (unsigned int)v, v->v1, v->v2);
#endif
	pq[++n_pq] = v;
	upheap(n_pq);
}

static
Edge *
pqremove(
void)
{
    Edge *v = pq[1];
#ifdef DEBUG
	printf("pqremove %d	%x  %d  %d\n",
	       n_pq, (unsigned int)v, v->v1, v->v2);
#endif
	pq[1]->pqp = NULL;
	pq[1] = pq[n_pq--];
	downheap(1);
	return(v);
}

static
void
pqdelete(
Edge *v)
{
    Edge **peeq = v->pqp;
#ifdef DEBUG
	printf("pqdelete %d	%x  %d  %d\n",
	       n_pq, (unsigned int)v, v->v1, v->v2);
#endif
	v->pqp = NULL;
	if(n_pq == 0)
	    return;
	*peeq = pq[n_pq--];
	upheap(peeq - pq);
	downheap(peeq - pq);
}
