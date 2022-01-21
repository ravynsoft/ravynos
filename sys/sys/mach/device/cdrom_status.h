/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * codes and structures for audio cd commands.
 */

typedef struct {
        unsigned char   cdmsf_min0;	/* start minute */
        unsigned char   cdmsf_sec0;	/* start second */
        unsigned char   cdmsf_frame0;	/* start frame */
        unsigned char   cdmsf_min1;	/* end minute */
        unsigned char   cdmsf_sec1;	/* end second */
        unsigned char   cdmsf_frame1;	/* end frame */
} cd_msf_t;

typedef struct {
        unsigned char   cdti_track0;	/* start track */
        unsigned char   cdti_index0;	/* start index */
        unsigned char   cdti_track1;	/* end track */
        unsigned char   cdti_index1;	/* end index */
} cd_ti_t;


typedef struct {
	unsigned int	len;
	unsigned char	first_track;
	unsigned char	last_track;
} cd_toc_header_t;

typedef union {
	  struct {
	    unsigned char	minute;
	    unsigned char	second;
	    unsigned char	frame;
	  } msf;
	  int lba;
} cd_addr_t;

typedef struct {
	unsigned char	cdt_track;
	unsigned int	cdt_adr		:4;
	unsigned int	cdt_ctrl	:4;
	unsigned char	cdt_type;
	cd_addr_t	cdt_addr;
	unsigned char	cdt_datamode;
} cd_toc_t;

typedef struct {
	unsigned char	cdsc_format;
	unsigned char	cdsc_track;
	unsigned char	cdsc_index;
	unsigned char	cdsc_audiostatus;
	unsigned int	cdsc_adr	:4;
	unsigned int	cdsc_ctrl	:4;
	unsigned char	cdsc_type;
	cd_addr_t	cdsc_absolute_addr;
	cd_addr_t	cdsc_relative_addr;
} cd_subchnl_t;

typedef struct {
        u_char  channel0;
        u_char  channel1;
        u_char  channel2;
        u_char  channel3;
} cd_volctrl_t;


/*
 * CD-ROM address types (cdrom_tocentry.cdte_format)
 */
#define MACH_CDROM_LBA 0x01 /* "logical block": first frame is #0 */
#define MACH_CDROM_MSF 0x02 /* "minute-second-frame": binary, not bcd here! */

/*
 * bit to tell whether track is data or audio (cdrom_tocentry.cdte_ctrl)
 */
#define MACH_CDROM_DATA_TRACK        0x04

/*
 * The leadout track is always 0xAA, regardless of # of tracks on disc
 */
#define MACH_CDROM_LEADOUT   0xAA

/*
 * CD-ROM IOCTL commands
 * For IOCTL calls, we will commandeer byte 0x53, or 'S'.
 */

#define MACH_CDROMPAUSE              0x5301
#define MACH_CDROMRESUME             0x5302
#define MACH_CDROMPLAYMSF            0x5303  /* (struct cdrom_msf) */
#define MACH_CDROMPLAYTRKIND         0x5304  /* (struct cdrom_ti) */

#define MACH_CDROMSTOP               0x5307  /* stop the drive motor */
#define MACH_CDROMSTART              0x5308  /* turn the motor on */

#define MACH_CDROMEJECT              0x5309  /* eject CD-ROM media */

#define MACH_CDROMVOLCTRL            0x530a  /* (struct cdrom_volctrl) */
#define MACH_CDROMVOLREAD	     0x5313  /* read current volume */


#define MACH_SCMD_READ_TOC_MSF		0x5320	/* Toc in msf format */
#define MACH_SCMD_READ_TOC_LBA		0x5321	/* Toc in lba format */

#define MACH_SCMD_READ_SUBCHNL_MSF	0x5322	/* SC info in msf format */
#define MACH_SCMD_READ_SUBCHNL_LBA	0x5323	/* SC info in lba format */

/*
 * CD-ROM-specific SCSI command opcodes
 */

/*
 * Group 2 (10-byte).  All of these are called 'optional' by SCSI-II.
 */
#define MACH_SCMD_READ_TOC           0x43    /* read table of contents */
#define MACH_SCMD_PLAYAUDIO_MSF      0x47    /* play data at time offset */
#define MACH_SCMD_PLAYAUDIO_TI       0x48    /* play data at track/index */
#define MACH_SCMD_PAUSE_RESUME       0x4B    /* pause/resume audio */
#define MACH_SCMD_READ_SUBCHANNEL    0x42    /* read SC info on playing disc */
#define MACH_SCMD_PLAYAUDIO10        0x45    /* play data at logical block */
#define MACH_SCMD_READ_HEADER        0x44    /* read TOC header */

/*
 * audio states (from SCSI-2, but seen with other drives, too)
 */
#define MACH_CDROM_AUDIO_INVALID     0x00    /* audio status not supported */
#define MACH_CDROM_AUDIO_PLAY        0x11    /* audio play operation in progress */
#define MACH_CDROM_AUDIO_PAUSED      0x12    /* audio play operation paused */
#define MACH_CDROM_AUDIO_COMPLETED   0x13    /* audio play successfully completed */
#define MACH_CDROM_AUDIO_ERROR       0x14    /* audio play stopped due to error */
#define MACH_CDROM_AUDIO_NO_STATUS   0x15    /* no current audio status to return */

