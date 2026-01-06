#ifndef _RELAX_H /* cctools-port */
#define _RELAX_H
/* The type used for a target address */
#ifdef ARCH64
typedef uint64_t relax_addressT;
#else
typedef uint32_t relax_addressT;
#endif

/*
 * relax_stateT is a fragment's type and stored in a struct frag's fr_type
 * field.
 */
typedef enum {
    rs_fill,	/* Variable chars to be repeated fr_offset */
		/* times. Fr_symbol unused. */
		/* Used with fr_offset == 0 for a constant */
		/* length frag. */
    rs_align,	/* Align: Fr_offset: power of 2. */
		/* 1 variable char: fill character. */
    rs_org,	/* Org: Fr_offset, fr_symbol: address. */
		/* 1 variable char: fill character. */
    rs_machine_dependent,
    rs_dwarf2dbg,
    rs_leb128	/* leb128 value, subtype is 0 for 1 for signed. */
} relax_stateT;

/*
 * For machine dependent fragments, a struct frag's who's fr_type field is
 * rs_machine_dependent it's substate is stored in the struct frag's fr_subtype;
 * field.  The substate is used to index in to md_relax_table by relax_section()
 * in layout.c to drive the span dependent branch algorithm of the assembler.
 * The substate is a machine dependent indication of what type of branch
 * instruction this fragment is.
 */
typedef uint32_t relax_substateT;

/*
 * relax_typeS is the structure that is the entry in the md_relax_table array.
 * It is indexed into by the substate of a fragment for machine depependent
 * branches that have variable sizes.  The entry tell how far this branch can
 * reach, rlx_forward and rlx_backward, as well as the size of branch,
 * rlx_length, and which substate go to, rlx_more, if this sized branch can't 
 * reach it's target.
 */
typedef struct relax_type {
    int32_t	    rlx_forward;  /* Forward  reach. Signed number. > 0. */
    int32_t	    rlx_backward; /* Backward reach. Signed number. < 0. */
    unsigned char   rlx_length;	  /* Bytes length of this address. */
    relax_substateT rlx_more;	  /* Next longer relax-state. */
				  /* 0 means there is no 'next' relax-state. */
} relax_typeS;
#endif /* _RELAX_H */
