/*
 * HCLINK.H
 *
 * $DragonFly: src/bin/cpdup/hclink.h,v 1.7 2008/05/24 17:21:36 dillon Exp $
 */

#ifndef _HCLINK_H_
#define _HCLINK_H_

/* Changing the buffer size breaks protocol compatibility! */
#define HC_BUFSIZE	65536

struct HCHostDesc {
    struct HCHostDesc *next;
    intptr_t desc;
    int type;
    void *data;
};

struct HostConf;

typedef struct HCTransaction {
    char	rbuf[HC_BUFSIZE];	/* input buffer */
    char	wbuf[HC_BUFSIZE];	/* output buffer */
    struct HostConf *hc;
    uint16_t	id;		/* assigned transaction id */
    int		swap;		/* have to swap byte order */
    int		windex;		/* output buffer index */
    enum { HCT_IDLE, HCT_SENT, HCT_REPLIED, HCT_DONE, HCT_FAIL } state;
} *hctransaction_t;

struct HostConf {
    char	*host;		/* [user@]host */
    int		fdin;		/* pipe */
    int		fdout;		/* pipe */
    int		error;		/* permanent failure code */
    pid_t	pid;
    int		version;	/* cpdup protocol version */
    struct HCHostDesc *hostdescs;
    struct HCTransaction trans;
};

struct HCHead {
    int32_t magic;		/* magic number / byte ordering */
    int32_t bytes;		/* size of packet */
    int16_t cmd;		/* command code */
    uint16_t id;		/* transaction id */
    int32_t error;		/* error code (response) */
} __aligned(8);	/* fix clang warning, not required for correct operation */

#define HCMAGIC		0x48435052	/* compatible byte ordering */
#define HCC_ALIGN(bytes)	(((bytes) + 7) & ~7)

struct HCLeaf {
    int16_t leafid;
    int16_t reserved;		/* reserved must be 0 */
    int32_t bytes;
} __aligned(8);	/* fix clang warning, not required for correct operation */

#define HCF_CONTINUE	0x4000		/* expect another reply */
#define HCF_REPLY	0x8000		/* reply */

#define LCF_TYPEMASK	0x0F00
#define LCF_INT32	0x0100		/* 4 byte integer */
#define LCF_INT64	0x0200		/* 8 byte integer */
#define LCF_STRING	0x0300		/* string, must be 0-terminated */
#define LCF_BINARY	0x0F00		/* binary data */

struct HCDesc {
    int16_t cmd;
    int (*func)(hctransaction_t, struct HCHead *);
};

/*
 * Item extraction macros
 */
#define HCC_STRING(item)	((const char *)((item) + 1))
#define HCC_INT32(item)		(*(int32_t *)((item) + 1))
#define HCC_INT64(item)		(*(int64_t *)((item) + 1))
#define HCC_BINARYDATA(item)	((void *)((item) + 1))

#define FOR_EACH_ITEM(item, trans, head)	\
		for (item = hcc_firstitem(trans, head); item; \
		     item = hcc_nextitem(trans, head, item))

/*
 * Prototypes
 */
int hcc_connect(struct HostConf *hc, int readonly);
int hcc_slave(int fdin, int fdout, struct HCDesc *descs, int count);

struct HCHead *hcc_read_command(struct HostConf *hc, hctransaction_t trans);
hctransaction_t hcc_start_command(struct HostConf *hc, int16_t cmd);
struct HCHead *hcc_finish_command(hctransaction_t trans);
void hcc_leaf_string(hctransaction_t trans, int16_t leafid, const char *str);
void hcc_leaf_data(hctransaction_t trans, int16_t leafid, const void *ptr, int bytes);
void hcc_leaf_int32(hctransaction_t trans, int16_t leafid, int32_t value);
void hcc_leaf_int64(hctransaction_t trans, int16_t leafid, int64_t value);
int hcc_check_space(hctransaction_t trans, struct HCHead *head, int n, int size);

intptr_t hcc_alloc_descriptor(struct HostConf *hc, void *ptr, int type);
void *hcc_get_descriptor(struct HostConf *hc, intptr_t desc, int type);
void hcc_set_descriptor(struct HostConf *hc, intptr_t desc, void *ptr, int type);

struct HCLeaf *hcc_nextitem(hctransaction_t trans, struct HCHead *head, struct HCLeaf *item);
#define hcc_firstitem(trans, head)	hcc_nextitem(trans, head, NULL)
struct HCLeaf *hcc_nextchaineditem(struct HostConf *hc, struct HCHead *head);
struct HCLeaf *hcc_currentchaineditem(struct HostConf *hc, struct HCHead *head);

void hcc_debug_dump(struct HCHead *head);

#endif

