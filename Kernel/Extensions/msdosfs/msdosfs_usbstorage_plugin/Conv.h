/*
 
 Copyright © 2017 Apple Inc. All rights reserved.

 Conv.h
 usbstorage_plugin

 Created by Yakov Ben Zaken on 16/10/2017.

 */


#ifndef __Conv_h_
#define __Conv_h_

#include "Common.h"

 #define UTF_SFM_CONVERSIONS 0x0020

void     msdosfs_unix2dostime                    (struct timespec *tsp, u_int16_t *ddp, u_int16_t *dtp, u_int8_t *dhp);
void     msdosfs_dos2unixtime                    (u_int dd, u_int dt, u_int dh, struct timespec *tsp);
u_char   msdosfs_unicode2dos                     (u_int16_t uc);
size_t   msdosfs_dos2unicodefn                   (u_char dn[SHORT_NAME_LEN], u_int16_t *un, int lower);
int      msdosfs_winChkName                      (const u_int16_t *un, int ucslen, struct winentry *wep, int chksum, u_int16_t *found_name, boolean_t *case_folded);
int      msdosfs_unicode_to_dos_name             (const uint16_t *unicode, size_t unicode_length, uint8_t short_name[SHORT_NAME_LEN], u_int8_t *lower_case);
int      msdosfs_apply_generation_to_short_name  (uint8_t short_name[SHORT_NAME_LEN], unsigned generation);
int      msdosfs_unicode2winfn                   (const u_int16_t *un, int unlen, struct winentry *wep, int cnt, int chksum);
int      msdosfs_getunicodefn                    (struct winentry *wep, u_int16_t ucfn[WIN_MAXLEN], u_int16_t *unichars, int chksum);
u_int8_t msdosfs_winChksum                       (u_int8_t *name);
int      msdosfs_winSlotCnt                      (const u_int16_t *un, int unlen);

extern const uint8_t puLongNameOffset[13];


errno_t     CONV_UTF8ToUnistr255        ( const uint8_t *puUtf8Ch, size_t utf8len, struct unistr255 *unicode, uint32_t uFlags );
size_t      CONV_Unistr255ToUTF8        ( const struct unistr255 *utf16, char utf8[FAT_MAX_FILENAME_UTF8] );
void        CONV_Unistr255ToLowerCase   ( struct unistr255* psUnistr255 );
void        CONV_GetCurrentTime         ( struct timespec* psTS );
errno_t     CONV_UTF8ToLowerCase  ( char* pcFileNameUTF8, char pcFileNameUTF8LowerCase[FAT_MAX_FILENAME_UTF8]);
void        CONV_convert_to_fsm(struct unistr255* unicode);
void        CONV_DuplicateName(char** ppcNewUTF8Name, const char *pcUTF8Name);
#endif /* __Conv_h_ */
