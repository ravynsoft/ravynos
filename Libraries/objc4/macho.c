/* FIXME: remove once we build libmacho into libSystem */

#include <mach-o/loader.h>
#include <uuid/uuid.h>
#include <string.h>

uint8_t *
getsectiondata(
const struct mach_header_64 *mhp,
const char *segname,
const char *sectname,
unsigned long *size)
{
    struct segment_command_64 *sgp;
    struct section_64 *sp;
    uint32_t i, j;
    intptr_t slide;

	slide = 0;
	sp = 0;
	sgp = (struct segment_command_64 *)
	      ((char *)mhp + sizeof(struct mach_header_64));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT_64){
		if(strcmp(sgp->segname, "__TEXT") == 0){
		    slide = (uintptr_t)mhp - sgp->vmaddr;
		}
		if(strncmp(sgp->segname, segname, sizeof(sgp->segname)) == 0){
		    sp = (struct section_64 *)((char *)sgp +
			 sizeof(struct segment_command_64));
		    for(j = 0; j < sgp->nsects; j++){
			if(strncmp(sp->sectname, sectname,
			   sizeof(sp->sectname)) == 0 &&
			   strncmp(sp->segname, segname,
			   sizeof(sp->segname)) == 0){
			    *size = sp->size;
			    return((uint8_t *)(sp->addr) + slide);
			}
			sp = (struct section_64 *)((char *)sp +
			     sizeof(struct section_64));
		    }
		}
	    }
	    sgp = (struct segment_command_64 *)((char *)sgp + sgp->cmdsize);
	}
	return(0);
}

uint8_t *
getsegmentdata(
const struct mach_header_64 *mhp,
const char *segname,
unsigned long *size)
{
    struct segment_command_64 *sgp;
    intptr_t slide;
    uint32_t i;

	slide = 0;
	sgp = (struct segment_command_64 *)
	      ((char *)mhp + sizeof(struct mach_header_64));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT_64){
		if(strcmp(sgp->segname, "__TEXT") == 0){
		    slide = (uintptr_t)mhp - sgp->vmaddr;
		}
		if(strncmp(sgp->segname, segname, sizeof(sgp->segname)) == 0){
		    *size = sgp->vmsize;
		    return((uint8_t *)(sgp->vmaddr + slide));
		}
	    }
	    sgp = (struct segment_command_64 *)((char *)sgp + sgp->cmdsize);
	}
	return(0);
}
