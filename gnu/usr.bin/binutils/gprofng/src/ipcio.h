/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Defines the external interface between er_ipc and the routines */

#ifndef _IPCIO_H
#define _IPCIO_H
#include <pthread.h>
#include "gp-defs.h"
#include "StringBuilder.h"

class DbeThreadPool;
typedef long long DbeObj;
typedef void *Object;
typedef char *String;

#define BUFFER_SIZE_SMALL           512
#define BUFFER_SIZE_MEDIUM          512
#define BUFFER_SIZE_LARGE           1024*1024

#define REQUEST_HAS_NO_BODY         0xFFFFFFFF
#define RESPONSE_STATUS_DEFAULT     0
#define RESPONSE_STATUS_SUCCESS     1
#define RESPONSE_STATUS_FAILURE     2
#define RESPONSE_STATUS_CANCELLED   3

#define RESPONSE_TYPE_ACK           0
#define RESPONSE_TYPE_PROGRESS      1
#define RESPONSE_TYPE_COMPLETE      2
#define RESPONSE_TYPE_HANDSHAKE     3
#define HEADER_MARKER               0xff

#define REQUEST_TYPE_DEFAULT        0
#define REQUEST_TYPE_CANCEL         1
#define REQUEST_TYPE_HANDSHAKE      2

#define IPC_PROTOCOL_STR            "IPC_PROTOCOL_38"
#define IPC_VERSION_NUMBER          38

enum IPCrequestStatus
{
  INITIALIZED = 0,
  IN_PROGRESS,
  COMPLETED,
  CANCELLED_DEFAULT,
  CANCELLED_IMMEDIATE
};

enum IPCTraceLevel
{
  TRACE_LVL_0 = 0,
  TRACE_LVL_1,
  TRACE_LVL_2,
  TRACE_LVL_3,
  TRACE_LVL_4
};

class IPCrequest
{
  char *buf;
  int size;
  int idx;
  int requestID;
  int channelID;
  IPCrequestStatus status;
  bool cancelImmediate;
public:
  IPCrequest (int, int, int);
  ~IPCrequest ();
  IPCrequestStatus getStatus ();
  void setStatus (IPCrequestStatus);
  void read ();

  int getRequestID ()               { return requestID; }
  int getChannelID ()               { return channelID; }
  bool isCancelImmediate ()         { return cancelImmediate; }
  void setCancelImmediate ()        { cancelImmediate = true; }
  char rgetc ()                     { return buf[idx++]; }
};

class IPCresponse
{
public:
  IPCresponse (int sz);
  ~IPCresponse ();

  int getRequestID ()               { return requestID; }
  int getChannelID ()               { return channelID; }
  void setRequestID (int r)         { requestID = r; }
  void setChannelID (int c)         { channelID = c; }
  void setResponseType (int r)      { responseType = r; }
  void setResponseStatus (int s)    { responseStatus = s; }
  int getCurBufSize ()              { return sb->capacity (); }
  void sendByte (int);
  void sendIVal (int);
  void sendLVal (long long);
  void sendDVal (double);
  void sendSVal (const char *);
  void sendBVal (bool);
  void sendCVal (char);
  void sendAVal (void*);
  void print (void);
  void reset ();
  IPCresponse *next;

private:
  int requestID;
  int channelID;
  int responseType;
  int responseStatus;
  StringBuilder *sb;
};

class BufferPool
{
public:
  BufferPool ();
  ~BufferPool ();
  IPCresponse* getNewResponse (int);
  void recycle (IPCresponse *);
private:
  pthread_mutex_t p_mutex;
  IPCresponse *smallBuf;
  IPCresponse *largeBuf;
};

// Read from the wire
int readInt (IPCrequest*);
bool readBoolean (IPCrequest*);
long long readLong (IPCrequest*);
DbeObj readObject (IPCrequest*);
Object readArray (IPCrequest*);
String readString (IPCrequest*);
void readRequestHeader ();

// write to the wire
void writeString (const char *, IPCrequest*);
void writeBoolean (bool, IPCrequest*);
void writeInt (int, IPCrequest*);
void writeChar (char, IPCrequest*);
void writeLong (long long, IPCrequest*);
void writeDouble (double, IPCrequest*);
void writeArray (void *, IPCrequest*);
void writeObject (DbeObj, IPCrequest*);
void writeResponseGeneric (int, int, int);
int setProgress (int, const char *);    // Update the progress bar
int ipc_doWork (void *);                // The argument is an IPCrequest

extern int ipc_flags;
extern int ipc_single_threaded_mode;
extern DbeThreadPool *responseThreadPool;
extern DbeThreadPool *ipcThreadPool;
extern int cancelRequestedChannelID;
extern int cancellableChannelID;
extern int error_flag;

void ipc_default_log (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void ipc_response_log (IPCTraceLevel, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void ipc_request_log (IPCTraceLevel, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

#endif
