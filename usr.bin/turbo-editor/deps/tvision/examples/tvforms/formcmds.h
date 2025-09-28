/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Formcmds.h: Support header file for TVFORMS Demo    */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(__FORMCMDS_H )
#define __FORMCMDS_H

// Misc UI commands

const ushort
   cmAboutBox    = 2000,
   cmChgDir      = 2001,
   cmVideoMode   = 2002;

// List & form-oriented commands
// (Cannot be disabled)

const ushort
   cmListOpen    = 3000,
   cmListSave    = 3001,
   cmFormEdit    = 3002,
   cmFormNew     = 3003,
   cmFormSave    = 3004,
   cmFormDel     = 3005;

// Broadcast commands

const ushort
   cmTopForm      = 3050,
   cmRegisterForm = 3051,
   cmEditingForm  = 3052,
   cmCanCloseForm = 3053,
   cmCloseForm    = 3054,
   cmTopList      = 3055,
   cmEditingFile  = 3056;

// History list IDs 

const ushort
   hlChangeDir   = 1,
   hlOpenListDlg = 2;

#endif  // __FORMCMDS_H
