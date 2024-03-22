/**************************************************************************
 *
 * Copyright 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/
#ifndef _PSP_DECRYPTION_PARAM_H_
#define _PSP_DECRYPTION_PARAM_H_

typedef struct _DECRYPT_PARAMETERS_
{
   uint32_t                frame_size;         // Size of encrypted frame
   uint8_t                 encrypted_iv[16];   // IV of the encrypted frame (clear)
   uint8_t                 encrypted_key[16];  // key to decrypt encrypted frame (encrypted with session key)
   uint8_t                 session_iv[16];     // IV to be used to decrypt encrypted_key

   union
   {
      struct
      {
         uint32_t    drm_id   : 4;	//DRM session ID
         uint32_t    ctr      : 1;
         uint32_t    cbc      : 1;
         uint32_t    reserved : 26;
      } s;
      uint32_t        value;
   } u;
} DECRYPT_PARAMETERS;

#endif //_PSP_DECRYPTION_PARAM_H_
