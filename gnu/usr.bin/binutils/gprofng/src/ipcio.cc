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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <queue>
#include "vec.h"
#include "util.h"
#include "ipcio.h"
#include "DbeThread.h"
#include "Experiment.h"

#define ipc_trace               if (ipc_flags) ipc_default_log
#define ipc_request_trace       if (ipc_flags) ipc_request_log
#define ipc_response_trace      if (ipc_flags) ipc_response_log

using namespace std;

// IPC implementation
static const int L_PROGRESS = 0;
static const int L_INTEGER  = 1;
static const int L_BOOLEAN  = 2;
static const int L_LONG     = 3;
static const int L_STRING   = 4;
static const int L_DOUBLE   = 5;
static const int L_ARRAY    = 6;
static const int L_OBJECT   = 7;
static const int L_CHAR     = 8;

int currentRequestID;
int currentChannelID;
IPCresponse *IPCresponseGlobal;

BufferPool *responseBufferPool;

IPCrequest::IPCrequest (int sz, int reqID, int chID)
{
  size = sz;
  requestID = reqID;
  channelID = chID;
  status = INITIALIZED;
  idx = 0;
  buf = (char *) malloc (size);
  cancelImmediate = false;
}

IPCrequest::~IPCrequest ()
{
  free (buf);
}

void
IPCrequest::read (void)
{
  for (int i = 0; i < size; i++)
    {
      int c = getc (stdin);
      ipc_request_trace (TRACE_LVL_4, "  IPCrequest:getc(stdin): %02x\n", c);
      buf[i] = c;
    }
}

IPCrequestStatus
IPCrequest::getStatus (void)
{
  return status;
}

void
IPCrequest::setStatus (IPCrequestStatus newStatus)
{
  status = newStatus;
}

static int
readByte (IPCrequest* req)
{
  int c;
  int val = 0;
  for (int i = 0; i < 2; i++)
    {
      if (req == NULL)
	{
	  c = getc (stdin);
	  ipc_request_trace (TRACE_LVL_4, "  readByte:getc(stdin): %02x\n", c);
	}
      else
	c = req->rgetc ();
      switch (c)
	{
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9':
	  val = val * 16 + c - '0';
	  break;
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	  val = val * 16 + c - 'a' + 10;
	  break;
	case EOF:
	  val = EOF;
	  break;
	default:
	  fprintf (stderr, "readByte: Unknown byte: %d\n", c);
	  break;
	}
    }
  return val;
}

static int
readIVal (IPCrequest *req)
{
  int val = readByte (req);
  for (int i = 0; i < 3; i++)
    val = val * 256 + readByte (req);
  ipc_trace ("  readIVal: %d\n", val);
  return val;
}

static String
readSVal (IPCrequest *req)
{
  int len = readIVal (req);
  if (len == -1)
    {
      ipc_trace ("  readSVal: <NULL>\n");
      return NULL;
    }
  char *str = (char *) malloc (len + 1);
  char *s = str;
  *s = (char) 0;
  while (len--)
    *s++ = req->rgetc ();
  *s = (char) 0;
  ipc_trace ("  readSVal: '%s'\n", str);
  return str;
}

static long long
readLVal (IPCrequest *req)
{
  long long val = readByte (req);
  for (int i = 0; i < 7; i++)
    val = val * 256 + readByte (req);
  ipc_trace ("  readLVal: %lld\n", val);
  return val;
}

static bool
readBVal (IPCrequest *req)
{
  int val = readByte (req);
  ipc_trace ("  readBVal: %s\n", val == 0 ? "true" : "false");
  return val != 0;
}

static char
readCVal (IPCrequest *req)
{
  int val = readByte (req);
  ipc_trace ("  readCVal: %d\n", val);
  return (char) val;
}

static double
readDVal (IPCrequest *req)
{
  String s = readSVal (req);
  double d = atof (s);
  free (s);
  return d;
}

static Object
readAVal (IPCrequest *req)
{
  bool twoD = false;
  int type = readByte (req);
  if (type == L_ARRAY)
    {
      twoD = true;
      type = readByte (req);
    }
  ipc_trace ("readAVal: twoD=%s type=%d\n", twoD ? "true" : "false", type);

  int len = readIVal (req);
  if (len == -1)
    return NULL;
  switch (type)
    {
    case L_INTEGER:
      if (twoD)
	{
	  Vector<Vector<int>*> *array = new Vector<Vector<int>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<int>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<int> *array = new Vector<int>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readIVal (req));
	  return array;
	}
      //break;
    case L_LONG:
      if (twoD)
	{
	  Vector<Vector<long long>*> *array = new Vector<Vector<long long>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<long long>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<long long> *array = new Vector<long long>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readLVal (req));
	  return array;
	}
      //break;
    case L_DOUBLE:
      if (twoD)
	{
	  Vector<Vector<double>*> *array = new Vector<Vector<double>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<double>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<double> *array = new Vector<double>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readDVal (req));
	  return array;
	}
      //break;
    case L_BOOLEAN:
      if (twoD)
	{
	  Vector < Vector<bool>*> *array = new Vector < Vector<bool>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<bool>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<bool> *array = new Vector<bool>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readBVal (req));
	  return array;
	}
      //break;
    case L_CHAR:
      if (twoD)
	{
	  Vector<Vector<char>*> *array = new Vector<Vector<char>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<char>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<char> *array = new Vector<char>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readCVal (req));
	  return array;
	}
      //break;
    case L_STRING:
      if (twoD)
	{
	  Vector<Vector<String>*> *array = new Vector<Vector<String>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<String>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<String> *array = new Vector<String>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readSVal (req));
	  return array;
	}
      //break;
    case L_OBJECT:
      if (twoD)
	{
	  Vector<Vector<Object>*> *array = new Vector<Vector<Object>*>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, (Vector<Object>*)readAVal (req));
	  return array;
	}
      else
	{
	  Vector<Object> *array = new Vector<Object>(len);
	  for (int i = 0; i < len; i++)
	    array->store (i, readAVal (req));
	  return array;
	}
      //break;
    default:
      fprintf (stderr, "readAVal: Unknown code: %d\n", type);
      break;
    }
  return NULL;
}

static int iVal;
static bool bVal;
static long long lVal;
static String sVal;
static double dVal;
static Object aVal;

static void
readResult (int type, IPCrequest *req)
{
  int tVal = readByte (req);
  switch (tVal)
    {
    case L_INTEGER:
      iVal = readIVal (req);
      break;
    case L_LONG:
      lVal = readLVal (req);
      break;
    case L_BOOLEAN:
      bVal = readBVal (req);
      break;
    case L_DOUBLE:
      dVal = readDVal (req);
      break;
    case L_STRING:
      sVal = readSVal (req);
      break;
    case L_ARRAY:
      aVal = readAVal (req);
      break;
    case EOF:
      fprintf (stderr, "EOF read in readResult\n");
      sVal = NULL;
      return;
    default:
      fprintf (stderr, "Unknown code: %d\n", tVal);
      abort ();
    }
  if (type != tVal)
    {
      fprintf (stderr, "Internal error: readResult: parameter mismatch: type=%d should be %d\n", tVal, type);
      abort ();
    }
}

int
readInt (IPCrequest *req)
{
  readResult (L_INTEGER, req);
  return iVal;
}

String
readString (IPCrequest *req)
{
  readResult (L_STRING, req);
  return sVal;
}

long long
readLong (IPCrequest *req)
{
  readResult (L_LONG, req);
  return lVal;
}

double
readDouble (IPCrequest *req)
{
  readResult (L_DOUBLE, req);
  return dVal;
}

bool
readBoolean (IPCrequest *req)
{
  readResult (L_BOOLEAN, req);
  return bVal;
}

DbeObj
readObject (IPCrequest *req)
{
  readResult (L_LONG, req);
  return (DbeObj) lVal;
}

Object
readArray (IPCrequest *req)
{
  readResult (L_ARRAY, req);
  return aVal;
}

// Write
IPCresponse::IPCresponse (int sz)
{
  requestID = -1;
  channelID = -1;
  responseType = -1;
  responseStatus = RESPONSE_STATUS_SUCCESS;
  sb = new StringBuilder (sz);
  next = NULL;
}

IPCresponse::~IPCresponse ()
{
  delete sb;
}

void
IPCresponse::reset ()
{
  requestID = -1;
  channelID = -1;
  responseType = -1;
  responseStatus = RESPONSE_STATUS_SUCCESS;
  sb->setLength (0);
}

void
IPCresponse::sendByte (int b)
{
  ipc_trace ("sendByte: %02x %d\n", b, b);
  sb->appendf ("%02x", b);
}

void
IPCresponse::sendIVal (int i)
{
  ipc_trace ("sendIVal: %08x %d\n", i, i);
  sb->appendf ("%08x", i);
}

void
IPCresponse::sendLVal (long long l)
{
  ipc_trace ("sendLVal: %016llx %lld\n", l, l);
  sb->appendf ("%016llx", l);
}

void
IPCresponse::sendSVal (const char *s)
{
  if (s == NULL)
    {
      sendIVal (-1);
      return;
    }
  sendIVal ((int) strlen (s));
  ipc_trace ("sendSVal: %s\n", s);
  sb->appendf ("%s", s);
}

void
IPCresponse::sendBVal (bool b)
{
  sendByte (b ? 1 : 0);
}

void
IPCresponse::sendCVal (char c)
{
  sendByte (c);
}

void
IPCresponse::sendDVal (double d)
{
  char str[32];
  snprintf (str, sizeof (str), "%.12f", d);
  sendSVal (str);
}

void
IPCresponse::sendAVal (void *ptr)
{
  if (ptr == NULL)
    {
      sendByte (L_INTEGER);
      sendIVal (-1);
      return;
    }

  VecType type = ((Vector<void*>*)ptr)->type ();
  switch (type)
    {
    case VEC_INTEGER:
      {
	sendByte (L_INTEGER);
	Vector<int> *array = (Vector<int>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendIVal (array->fetch (i));
	break;
      }
    case VEC_BOOL:
      {
	sendByte (L_BOOLEAN);
	Vector<bool> *array = (Vector<bool>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendBVal (array->fetch (i));
	break;
      }
    case VEC_CHAR:
      {
	sendByte (L_CHAR);
	Vector<char> *array = (Vector<char>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendCVal (array->fetch (i));
	break;
      }
    case VEC_LLONG:
      {
	sendByte (L_LONG);
	Vector<long long> *array = (Vector<long long>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendLVal (array->fetch (i));
	break;
      }
    case VEC_DOUBLE:
      {
	sendByte (L_DOUBLE);
	Vector<double> *array = (Vector<double>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendDVal (array->fetch (i));
	break;
      }
    case VEC_STRING:
      {
	sendByte (L_STRING);
	Vector<String> *array = (Vector<String>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendSVal (array->fetch (i));
	break;
      }
    case VEC_STRINGARR:
      {
	sendByte (L_ARRAY);
	sendByte (L_STRING);
	Vector<void*> *array = (Vector<void*>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendAVal (array->fetch (i));
	break;
      }
    case VEC_INTARR:
      {
	sendByte (L_ARRAY);
	sendByte (L_INTEGER);
	Vector<void*> *array = (Vector<void*>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendAVal (array->fetch (i));
	break;
      }
    case VEC_LLONGARR:
      {
	sendByte (L_ARRAY);
	sendByte (L_LONG);
	Vector<void*> *array = (Vector<void*>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendAVal (array->fetch (i));
	break;
      }
    case VEC_VOIDARR:
      {
	sendByte (L_OBJECT);
	Vector<void*> *array = (Vector<void*>*)ptr;
	sendIVal (array->size ());
	for (int i = 0; i < array->size (); i++)
	  sendAVal (array->fetch (i));
	break;
      }
    default:
      fprintf (stderr, "sendAVal: Unknown type: %d\n", type);
      abort ();
    }
}

bool
cancelNeeded (int chID)
{
  if (chID == cancellableChannelID && chID == cancelRequestedChannelID)
    return true;
  else
    return false;
}

static void
writeResponseWithHeader (int requestID, int channelID, int responseType,
			 int responseStatus, IPCresponse* os)
{
  if (cancelNeeded (channelID))
    {
      responseStatus = RESPONSE_STATUS_CANCELLED;
      ipc_trace ("CANCELLING %d %d\n", requestID, channelID);
      // This is for gracefully cancelling regular ops like openExperiment - getFiles should never reach here
    }
  os->setRequestID (requestID);
  os->setChannelID (channelID);
  os->setResponseType (responseType);
  os->setResponseStatus (responseStatus);
  os->print ();
  os->reset ();
  responseBufferPool->recycle (os);
}

void
writeAck (int requestID, int channelID)
{
#if DEBUG
  char *s = getenv (NTXT ("SP_NO_IPC_ACK"));
#else /* ^DEBUG */
  char *s = NULL;
#endif /* ^DEBUG */
  if (s)
    {
      int i = requestID;
      int j = channelID;
      ipc_request_trace (TRACE_LVL_4, "ACK skipped: requestID=%d channelID=%d\n", i, j);
    }
  else
    {
      IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_SMALL);
      writeResponseWithHeader (requestID, channelID, RESPONSE_TYPE_ACK,
			       RESPONSE_STATUS_SUCCESS, OUTS);
    }
}

void
writeHandshake (int requestID, int channelID)
{
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_SMALL);
  writeResponseWithHeader (requestID, channelID, RESPONSE_TYPE_HANDSHAKE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeResponseGeneric (int responseStatus, int requestID, int channelID)
{
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_SMALL);
  writeResponseWithHeader (requestID, channelID, RESPONSE_TYPE_COMPLETE, responseStatus, OUTS);
}

BufferPool::BufferPool ()
{
  pthread_mutex_init (&p_mutex, NULL);
  smallBuf = NULL;
  largeBuf = NULL;
}

BufferPool::~BufferPool ()
{
  for (IPCresponse *p = smallBuf; p;)
    {
      IPCresponse *tmp = p;
      p = tmp->next;
      delete tmp;
    }
  for (IPCresponse *p = largeBuf; p;)
    {
      IPCresponse *tmp = p;
      p = tmp->next;
      delete tmp;
    }
}

IPCresponse*
BufferPool::getNewResponse (int size)
{
  pthread_mutex_lock (&p_mutex);
  if (ipc_single_threaded_mode && size < BUFFER_SIZE_LARGE)
    size = BUFFER_SIZE_LARGE;
  IPCresponse *newResponse = NULL;
  if (size >= BUFFER_SIZE_LARGE)
    {
      if (largeBuf)
	{
	  newResponse = largeBuf;
	  largeBuf = largeBuf->next;
	}
    }
  else if (smallBuf)
    {
      newResponse = smallBuf;
      smallBuf = smallBuf->next;
    }
  if (newResponse)
    newResponse->reset ();
  else
    {
      newResponse = new IPCresponse (size);
      ipc_trace ("GETNEWBUFFER %d\n", size);
    }
  pthread_mutex_unlock (&p_mutex);
  return newResponse;
}

void
BufferPool::recycle (IPCresponse *respB)
{
  pthread_mutex_lock (&p_mutex);
  if (respB->getCurBufSize () >= BUFFER_SIZE_LARGE)
    {
      respB->next = largeBuf;
      largeBuf = respB;
    }
  else
    {
      respB->next = smallBuf;
      smallBuf = respB;
    }
  pthread_mutex_unlock (&p_mutex);
}

void
writeArray (void *ptr, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_LARGE);
  OUTS->sendByte (L_ARRAY);
  OUTS->sendAVal (ptr);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (),
			   RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeString (const char *s, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_LARGE);
  OUTS->sendByte (L_STRING);
  OUTS->sendSVal (s);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (),
			   RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeObject (DbeObj obj, IPCrequest* req)
{
  writeLong ((long long) obj, req);
}

void
writeBoolean (bool b, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_MEDIUM);
  OUTS->sendByte (L_BOOLEAN);
  OUTS->sendBVal (b);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (),
			   RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeInt (int i, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_MEDIUM);
  OUTS->sendByte (L_INTEGER);
  OUTS->sendIVal (i);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (), RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeChar (char c, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_MEDIUM);
  OUTS->sendByte (L_CHAR);
  OUTS->sendCVal (c);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (), RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeLong (long long l, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE)
    return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_MEDIUM);
  OUTS->sendByte (L_LONG);
  OUTS->sendLVal (l);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (), RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

void
writeDouble (double d, IPCrequest* req)
{
  if (req->getStatus () == CANCELLED_IMMEDIATE) return;
  IPCresponse *OUTS = responseBufferPool->getNewResponse (BUFFER_SIZE_MEDIUM);
  OUTS->sendByte (L_DOUBLE);
  OUTS->sendDVal (d);
  writeResponseWithHeader (req->getRequestID (), req->getChannelID (), RESPONSE_TYPE_COMPLETE, RESPONSE_STATUS_SUCCESS, OUTS);
}

int
setProgress (int percentage, const char *proc_str)
{
  if (cancelNeeded (currentChannelID))
    {
      // ExperimentLoadCancelException *e1 = new ExperimentLoadCancelException();
      // throw (e1);
      return 1;
    }
  if (NULL == proc_str)
    return 1;
  int size = strlen (proc_str) + 100; // 100 bytes for additional data
  int bs = BUFFER_SIZE_MEDIUM;
  if (size > BUFFER_SIZE_MEDIUM)
    {
      if (size > BUFFER_SIZE_LARGE) return 1; // This should never happen
      bs = BUFFER_SIZE_LARGE;
    }
  IPCresponse *OUTS = responseBufferPool->getNewResponse (bs);
  OUTS->sendByte (L_PROGRESS);
  OUTS->sendIVal (percentage);
  OUTS->sendSVal (proc_str);
  writeResponseWithHeader (currentRequestID, currentChannelID, RESPONSE_TYPE_PROGRESS, RESPONSE_STATUS_SUCCESS, OUTS);
  return 0;
}

static pthread_mutex_t responce_lock = PTHREAD_MUTEX_INITIALIZER;

void
IPCresponse::print (void)
{
  char buf[23];
  int sz = responseType == RESPONSE_TYPE_HANDSHAKE ?
      IPC_VERSION_NUMBER : sb->length ();
  snprintf (buf, sizeof (buf), "%02x%08x%02x%02x%08x", HEADER_MARKER,
	    requestID, responseType, responseStatus, sz);
  pthread_mutex_lock (&responce_lock);
  ipc_response_trace (TRACE_LVL_1,
		      "IPCresponse: ID=%08x type=%02x status=%02x sz:%6d\n",
		      requestID, responseType, responseStatus, sz);
  write (1, buf, 22);
  sb->write (1);
  pthread_mutex_unlock (&responce_lock);
}

void
setCancelRequestedCh (int chID)
{
  cancelRequestedChannelID = chID;
}

void
readRequestHeader ()
{
  int marker = readByte (NULL);
  if (marker != HEADER_MARKER)
    {
      fprintf (stderr, "Internal error: received request (%d) without header marker\n", marker);
      error_flag = 1;
      return;
    }
  else
    ipc_request_trace (TRACE_LVL_1, "RequestHeaderBegin------------------------\n");
  int requestID = readIVal (NULL);
  int requestType = readByte (NULL);
  int channelID = readIVal (NULL);
  int nBytes = readIVal (NULL);
  if (requestType == REQUEST_TYPE_HANDSHAKE)
    {
      // write the ack directly to the wire, not through the response queue
      writeAck (requestID, channelID);
      writeHandshake (requestID, channelID);
      ipc_request_trace (TRACE_LVL_1, "RQ: HANDSHAKE --- %x ----- %x ---- %x --- %x -RequestHeaderEnd\n", requestID, requestType, channelID, nBytes);
    }
  else if (requestType == REQUEST_TYPE_CANCEL)
    {
      writeAck (requestID, channelID);
      ipc_request_trace (TRACE_LVL_1, "RQ: CANCEL --- RQ: %x ----- %x --- CH:  %x --- %x -RequestHeaderEnd\n", requestID, requestType, channelID, nBytes);
      if (channelID == cancellableChannelID)
	{
	  // we have worked on at least one request belonging to this channel
	  writeResponseGeneric (RESPONSE_STATUS_SUCCESS, requestID, channelID);
	  setCancelRequestedCh (channelID);
	  ipc_trace ("CANCELLABLE %x %x\n", channelID, currentChannelID);
	  if (channelID == currentChannelID)
	    //  request for this channel is currently in progress
	    ipc_request_trace (TRACE_LVL_1, "IN PROGRESS REQUEST NEEDS CANCELLATION");
	    //              ssp_post_cond(waitingToFinish);
	}
      else
	{
	  // FIXME:
	  // it is possible that a request for this channel is on the requestQ
	  // or has been submitted to the work group queue but is waiting for a thread to pick it up
	  writeResponseGeneric (RESPONSE_STATUS_FAILURE, requestID, channelID);
	  setCancelRequestedCh (channelID);
	  ipc_request_trace (TRACE_LVL_1, "RETURNING FAILURE TO CANCEL REQUEST channel %d\n", channelID);
	}
    }
  else
    {
      writeAck (requestID, channelID);
      ipc_request_trace (TRACE_LVL_1, "RQ: --- %x ----- %x ---- %x --- %x -RequestHeaderEnd\n", requestID, requestType, channelID, nBytes);
      IPCrequest *nreq = new IPCrequest (nBytes, requestID, channelID);
      nreq->read ();
      ipc_request_trace (TRACE_LVL_1, "RQ: --- %x Read from stream \n", requestID);
      if (cancelNeeded (channelID))
	{
	  ipc_request_trace (TRACE_LVL_1, "CANCELLABLE REQ RECVD %x %x\n", channelID, requestID);
	  writeResponseGeneric (RESPONSE_STATUS_CANCELLED, requestID, channelID);
	  delete nreq;
	  return;
	}
      DbeQueue *q = new DbeQueue (ipc_doWork, nreq);
      ipcThreadPool->put_queue (q);
    }
}
