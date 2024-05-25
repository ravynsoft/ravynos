/*
 * HCLINK.C
 *
 * This module implements a simple remote control protocol
 *
 * $DragonFly: src/bin/cpdup/hclink.c,v 1.10 2008/05/24 17:21:36 dillon Exp $
 */

#include "cpdup.h"
#include "hclink.h"
#include "hcproto.h"

static void hcc_start_reply(hctransaction_t trans, struct HCHead *rhead);
static int hcc_finish_reply(hctransaction_t trans, struct HCHead *head);

int
hcc_connect(struct HostConf *hc, int readonly)
{
    int fdin[2];
    int fdout[2];
    const char *av[32];

    if (hc == NULL || hc->host == NULL)
	return(0);

    if (pipe(fdin) < 0)
	return(-1);
    if (pipe(fdout) < 0) {
	close(fdin[0]);
	close(fdin[1]);
	return(-1);
    }
    if ((hc->pid = fork()) == 0) {
	/*
	 * Child process
	 */
	int n, m;

	dup2(fdin[1], 1);
	close(fdin[0]);
	close(fdin[1]);
	dup2(fdout[0], 0);
	close(fdout[0]);
	close(fdout[1]);

	n = 0;
	av[n++] = "ssh";
	if (CompressOpt)
	    av[n++] = "-C";
	for (m = 0; m < ssh_argc; m++)
	    av[n++] = ssh_argv[m];
	av[n++] = "-T";
	av[n++] = hc->host;
	av[n++] = "cpdup";
	av[n++] = (readonly ? "-RS" : "-S");
	av[n++] = NULL;

	execv("/usr/bin/ssh", (void *)av);
	_exit(1);
    } else if (hc->pid < 0) {
	return(-1);
    } else {
	/*
	 * Parent process.  Do the initial handshake to make sure we are
	 * actually talking to a cpdup slave.
	 */
	close(fdin[1]);
	hc->fdin = fdin[0];
	close(fdout[0]);
	hc->fdout = fdout[1];
	return(0);
    }
}

static int
rc_badop(hctransaction_t trans __unused, struct HCHead *head)
{
    head->error = EOPNOTSUPP;
    return(0);
}

int
hcc_slave(int fdin, int fdout, struct HCDesc *descs, int count)
{
    struct HostConf hcslave;
    struct HCHead *head;
    struct HCTransaction trans;
    int (*dispatch[256])(hctransaction_t, struct HCHead *);
    int i;
    int r;

    bzero(&hcslave, sizeof(hcslave));
    bzero(&trans, sizeof(trans));
    bzero(dispatch, sizeof(dispatch));
    for (i = 0; i < count; ++i) {
	struct HCDesc *desc = &descs[i];
	assert(desc->cmd >= 0 && desc->cmd < 256);
	dispatch[desc->cmd] = desc->func;
    }
    for (i = 0; i < 256; ++i) {
	if (dispatch[i] == NULL)
	    dispatch[i] = rc_badop;
    }
    hcslave.fdin = fdin;
    hcslave.fdout = fdout;
    trans.hc = &hcslave;

    /*
     * Process commands on fdin and write out results on fdout
     */
    for (;;) {
	/*
	 * Get the command
	 */
	head = hcc_read_command(trans.hc, &trans);
	if (head == NULL)
	    break;

	/*
	 * Start the reply and dispatch, then process the return code.
	 */
	head->error = 0;
	hcc_start_reply(&trans, head);

	r = dispatch[head->cmd & 255](&trans, head);

	switch(r) {
	case -2:
		head->error = EINVAL;
		break;
	case -1:
		head->error = errno;
		break;
	case 0:
		break;
	default:
		assert(0);
		break;
	}

	if (!hcc_finish_reply(&trans, head))
	    break;
    }
    return(0);
}

/*
 * This reads a command from fdin, fixes up the byte ordering, and returns
 * a pointer to HCHead.
 */
struct HCHead *
hcc_read_command(struct HostConf *hc, hctransaction_t trans)
{
    struct HCHead tmp;
    int aligned_bytes;
    int need_swap;
    int n;
    int r;

    if (trans == NULL)
	fatal("cpdup hlink protocol error with %s", hc->host);

    n = 0;
    while (n < (int)sizeof(struct HCHead)) {
	r = read(hc->fdin, (char *)&tmp + n, sizeof(struct HCHead) - n);
	if (r <= 0)
	    goto fail;
	n += r;
    }

    if (tmp.magic == HCMAGIC) {
	need_swap = 0;
    } else {
	tmp.magic = hc_bswap32(tmp.magic);
	if (tmp.magic != HCMAGIC)
	    fatal("magic mismatch with %s (%04x)", hc->host, tmp.id);
	need_swap = 1;
	tmp.bytes = hc_bswap32(tmp.bytes);
	tmp.cmd   = hc_bswap16(tmp.cmd);
	tmp.id    = hc_bswap16(tmp.id);
	tmp.error = hc_bswap32(tmp.error);
    }

    assert(tmp.bytes >= (int)sizeof(tmp) && tmp.bytes < HC_BUFSIZE);

    trans->swap = need_swap;
    bcopy(&tmp, trans->rbuf, n);
    aligned_bytes = HCC_ALIGN(tmp.bytes);

    while (n < aligned_bytes) {
	r = read(hc->fdin, trans->rbuf + n, aligned_bytes - n);
	if (r <= 0)
	    goto fail;
	n += r;
    }
#ifdef DEBUG
    hcc_debug_dump(trans, head);
#endif
    trans->state = HCT_REPLIED;
    return((void *)trans->rbuf);
fail:
    trans->state = HCT_FAIL;
    return(NULL);
}

/*
 * Initialize for a new command
 */
hctransaction_t
hcc_start_command(struct HostConf *hc, int16_t cmd)
{
    struct HCHead *whead;
    hctransaction_t trans;

    trans = &hc->trans;

    whead = (void *)trans->wbuf;
    whead->magic = HCMAGIC;
    whead->bytes = 0;
    whead->cmd = cmd;
    whead->id = trans->id;
    whead->error = 0;

    trans->windex = sizeof(*whead);
    trans->hc = hc;
    trans->state = HCT_IDLE;

    return(trans);
}

static void
hcc_start_reply(hctransaction_t trans, struct HCHead *rhead)
{
    struct HCHead *whead = (void *)trans->wbuf;

    whead->magic = HCMAGIC;
    whead->bytes = 0;
    whead->cmd = rhead->cmd | HCF_REPLY;
    whead->id = rhead->id;
    whead->error = 0;

    trans->windex = sizeof(*whead);
}

/*
 * Finish constructing a command, transmit it, and await the reply.
 * Return the HCHead of the reply.
 */
struct HCHead *
hcc_finish_command(hctransaction_t trans)
{
    struct HostConf *hc;
    struct HCHead *whead;
    struct HCHead *rhead;
    int aligned_bytes;
    int16_t wcmd;

    hc = trans->hc;
    whead = (void *)trans->wbuf;
    whead->bytes = trans->windex;
    aligned_bytes = HCC_ALIGN(trans->windex);
    trans->windex = 0;	/* initialize for hcc_nextchaineditem() */

    trans->state = HCT_SENT;

    if (write(hc->fdout, whead, aligned_bytes) != aligned_bytes) {
#ifdef __error
	*__error = EIO;
#else
	errno = EIO;
#endif
	if (whead->cmd < 0x0010)
	    return(NULL);
	fatal("cpdup lost connection to %s", hc->host);
    }

    wcmd = whead->cmd;

    /*
     * whead is invalid when we call hcc_read_command() because
     * we may switch to another thread.
     */
    rhead = hcc_read_command(hc, trans);
    if (trans->state != HCT_REPLIED || rhead->id != trans->id) {
#ifdef __error
	*__error = EIO;
#else
	errno = EIO;
#endif
	if (wcmd < 0x0010)
		return(NULL);
	fatal("cpdup lost connection to %s", hc->host);
    }
    trans->state = HCT_DONE;

    if (rhead->error) {
#ifdef __error
	*__error = rhead->error;
#else
	errno = rhead->error;
#endif
    }
    return (rhead);
}

int
hcc_finish_reply(hctransaction_t trans, struct HCHead *head)
{
    struct HCHead *whead;
    int aligned_bytes;

    whead = (void *)trans->wbuf;
    whead->bytes = trans->windex;
    whead->error = head->error;
    aligned_bytes = HCC_ALIGN(trans->windex);
#ifdef DEBUG
    hcc_debug_dump(trans, whead);
#endif
    return (write(trans->hc->fdout, whead, aligned_bytes) == aligned_bytes);
}

void
hcc_leaf_string(hctransaction_t trans, int16_t leafid, const char *str)
{
    struct HCLeaf *item;
    int bytes = strlen(str) + 1;

    item = (void *)(trans->wbuf + trans->windex);
    assert(trans->windex + sizeof(*item) + bytes < HC_BUFSIZE);
    item->leafid = leafid;
    item->reserved = 0;
    item->bytes = sizeof(*item) + bytes;
    bcopy(str, item + 1, bytes);
    trans->windex = HCC_ALIGN(trans->windex + item->bytes);
}

void
hcc_leaf_data(hctransaction_t trans, int16_t leafid, const void *ptr, int bytes)
{
    struct HCLeaf *item;

    item = (void *)(trans->wbuf + trans->windex);
    assert(trans->windex + sizeof(*item) + bytes < HC_BUFSIZE);
    item->leafid = leafid;
    item->reserved = 0;
    item->bytes = sizeof(*item) + bytes;
    bcopy(ptr, item + 1, bytes);
    trans->windex = HCC_ALIGN(trans->windex + item->bytes);
}

void
hcc_leaf_int32(hctransaction_t trans, int16_t leafid, int32_t value)
{
    struct HCLeaf *item;

    item = (void *)(trans->wbuf + trans->windex);
    assert(trans->windex + sizeof(*item) + sizeof(value) < HC_BUFSIZE);
    item->leafid = leafid;
    item->reserved = 0;
    item->bytes = sizeof(*item) + sizeof(value);
    *(int32_t *)(item + 1) = value;
    trans->windex = HCC_ALIGN(trans->windex + item->bytes);
}

void
hcc_leaf_int64(hctransaction_t trans, int16_t leafid, int64_t value)
{
    struct HCLeaf *item;

    item = (void *)(trans->wbuf + trans->windex);
    assert(trans->windex + sizeof(*item) + sizeof(value) < HC_BUFSIZE);
    item->leafid = leafid;
    item->reserved = 0;
    item->bytes = sizeof(*item) + sizeof(value);
    *(int64_t *)(item + 1) = value;
    trans->windex = HCC_ALIGN(trans->windex + item->bytes);
}

/*
 * Check if there's enough space left in the write buffer for <n>
 * leaves with a total of <size> data bytes.
 * If not, the current packet will be sent with the HCF_CONTINUE flag,
 * then the transaction is initialized for another reply packet.
 *
 * Returns success status (boolean).
 */
int
hcc_check_space(hctransaction_t trans, struct HCHead *head, int n, int size)
{
    size = HCC_ALIGN(size) + n * sizeof(struct HCLeaf);
    if (size >= HC_BUFSIZE - trans->windex) {
	struct HCHead *whead = (void *)trans->wbuf;

	whead->cmd |= HCF_CONTINUE;
	if (!hcc_finish_reply(trans, head))
	    return (0);
	hcc_start_reply(trans, head);
    }
    return (1);
}

intptr_t
hcc_alloc_descriptor(struct HostConf *hc, void *ptr, int type)
{
    struct HCHostDesc *hd;
    struct HCHostDesc *hnew;

    hnew = malloc(sizeof(struct HCHostDesc));
    hnew->type = type;
    hnew->data = ptr;

    if ((hd = hc->hostdescs) != NULL) {
	hnew->desc = hd->desc + 1;
    } else {
	/* start at 2 because 1 has a special meaning in hc_open() */
	hnew->desc = 2;
    }
    hnew->next = hd;
    hc->hostdescs = hnew;
    return(hnew->desc);
}

void *
hcc_get_descriptor(struct HostConf *hc, intptr_t desc, int type)
{
    struct HCHostDesc *hd;

    for (hd = hc->hostdescs; hd; hd = hd->next) {
	if (hd->desc == desc && hd->type == type)
	    return(hd->data);
    }
    return(NULL);
}

void
hcc_set_descriptor(struct HostConf *hc, intptr_t desc, void *ptr, int type)
{
    struct HCHostDesc *hd;
    struct HCHostDesc **hdp;

    for (hdp = &hc->hostdescs; (hd = *hdp) != NULL; hdp = &hd->next) {
	if (hd->desc == desc) {
	    if (ptr) {
		hd->data = ptr;
		hd->type = type;
	    } else {
		*hdp = hd->next;
		free(hd);
	    }
	    return;
	}
    }
    if (ptr) {
	hd = malloc(sizeof(*hd));
	hd->desc = desc;
	hd->type = type;
	hd->data = ptr;
	hd->next = hc->hostdescs;
	hc->hostdescs = hd;
    }
}

struct HCLeaf *
hcc_nextitem(hctransaction_t trans, struct HCHead *head, struct HCLeaf *item)
{
    int offset;

    if (item == NULL)
	item = (void *)(head + 1);
    else
	item = (void *)((char *)item + HCC_ALIGN(item->bytes));
    offset = (char *)item - (char *)head;
    if (offset == head->bytes)
	return(NULL);
    if (trans->swap) {
	int64_t *i64ptr;
	int32_t *i32ptr;

	item->leafid = hc_bswap16(item->leafid);
	item->bytes  = hc_bswap32(item->bytes);
	switch (item->leafid & LCF_TYPEMASK) {
	    case LCF_INT32:
		i32ptr = (void *)(item + 1);
		*i32ptr = hc_bswap32(*i32ptr);
		break;
	    case LCF_INT64:
		i64ptr = (void *)(item + 1);
		*i64ptr = hc_bswap64(*i64ptr);
		break;
	}
    }
    assert(head->bytes >= offset + (int)sizeof(*item));
    assert(head->bytes >= offset + item->bytes);
    assert(item->bytes >= (int)sizeof(*item) && item->bytes < HC_BUFSIZE);
    return (item);
}

struct HCLeaf *
hcc_nextchaineditem(struct HostConf *hc, struct HCHead *head)
{
    hctransaction_t trans = &hc->trans;
    struct HCLeaf *item = hcc_currentchaineditem(hc, head);

    while ((item = hcc_nextitem(trans, head, item)) == NULL) {
	if (!(head->cmd & HCF_CONTINUE))
	    return (NULL);
	head = hcc_read_command(hc, trans);
	if (trans->state != HCT_REPLIED || head->id != trans->id)
	    return (NULL);
    }
    trans->windex = (char *)item - (char *)head;
    return (item);
}

struct HCLeaf *
hcc_currentchaineditem(struct HostConf *hc, struct HCHead *head)
{
    hctransaction_t trans = &hc->trans;

    if (trans->windex == 0)
	return (NULL);
    else
	return ((void *) ((char *)head + trans->windex));
}

#ifdef DEBUG

void
hcc_debug_dump(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    int aligned_bytes = HCC_ALIGN(head->bytes);

    fprintf(stderr, "DUMP %04x (%d)", (uint16_t)head->cmd, aligned_bytes);
    if (head->cmd & HCF_REPLY)
	fprintf(stderr, " error %d", head->error);
    fprintf(stderr, "\n");
    FOR_EACH_ITEM(item, trans, head) {
	fprintf(stderr, "    ITEM %04x DATA ", item->leafid);
	switch(item->leafid & LCF_TYPEMASK) {
	case LCF_INT32:
	    fprintf(stderr, "int32 %d\n", HCC_INT32(item));
	    break;
	case LCF_INT64:
	    fprintf(stderr, "int64 %lld\n", HCC_INT64(item));
	    break;
	case LCF_STRING:
	    fprintf(stderr, "\"%s\"\n", HCC_STRING(item));
	    break;
	case LCF_BINARY:
	    fprintf(stderr, "(binary)\n");
	    break;
	default:
	    printf("?\n");
	}
    }
}

#endif
