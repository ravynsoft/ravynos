#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#undef __USE_INLINE__
#include <exec/types.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>

#include <rexx/rxslib.h>
#include <rexx/errors.h>
//#include "rexxmsgext.h" // this should change depening on the ultimate location of the structures

/* utils */

/*
 * Structure for the rexx host. Most of the code is inspired from Olaf
 * Barthel's sample ARexx code from the developer CD 2.1
 */


struct RexxHost
{
	struct MsgPort *Port;
	TEXT PortName[81];
} ;

struct ARexxMsg
{
	struct RexxMsg *rexxMsg;
	BOOL isReplied;
	struct RexxHost *rexxHost;
};

STRPTR dupstr(STRPTR src)
{
    STRPTR dest = NULL;
    ULONG len;
    if(src)
    {
        len = strlen(src);
        if((dest = IExec->AllocVec(len + 1, MEMF_ANY)))
        {
            strcpy(dest,src);
        }
    }
    return dest;
}


struct TimeRequest *
OpenTimer(void)
{
	struct MsgPort *port = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END);
	if (port == NULL)
	{
		return NULL;
	}

	struct TimeRequest *req = IExec->AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_Size, sizeof(struct TimeRequest),
		ASOIOR_ReplyPort, port,
		TAG_END);

	if (req == NULL)
	{
		IExec->FreeSysObject(ASOT_PORT, port);
		return NULL;
	}

	int8 deverr = IExec->OpenDevice("timer.device", UNIT_MICROHZ,
		&req->Request, 0);

	if (deverr != IOERR_SUCCESS)
	{
		IExec->FreeSysObject(ASOT_IOREQUEST, req);
		IExec->FreeSysObject(ASOT_PORT, port);
		return NULL;
	}

	return req;
}


void
CloseTimer(struct TimeRequest *req)
{
	if (req != NULL)
	{
		struct MsgPort *port = req->Request.io_Message.mn_ReplyPort;

		IExec->CloseDevice(&req->Request);
		IExec->FreeSysObject(ASOT_IOREQUEST, req);
		IExec->FreeSysObject(ASOT_PORT, port);
	}
}

LONG
ReturnRexxMsg(struct RexxMsg * Message, CONST_STRPTR Result)
{
	STRPTR ResultString = NULL;

	/* No error has occured yet. */
	int32 ErrorCode = 0;

	/* Set up the RexxMsg to return no error. */
	Message->rm_Result1 = RC_OK;
	Message->rm_Result2 = 0;

	/* Check if the command should return a result. */
	if((Message->rm_Action & RXFF_RESULT) && Result != NULL)
	{
		/* To return the result string we need to make
		 * a copy for ARexx to use.
		 */
		if((ResultString = IRexxSys->CreateArgstring(Result, strlen(Result))))
		{
			/* Put the string into the secondary
			 * result field.
			 */
			Message->rm_Result2 = (LONG)ResultString;
		}
		else
		{
			/* No memory available. */
			ErrorCode = ERR10_003;
		}
	}

	/* Reply the message, regardless of the error code. */
	IExec->ReplyMsg((struct Message *)Message);

	return(ErrorCode);
}


void
ReturnErrorMsg(struct RexxMsg *msg, CONST_STRPTR port, int32 rc, int32 rc2)
{
	/* To signal an error the rc_Result1
	 * entry of the RexxMsg needs to be set to
	 * RC_ERROR. Unfortunately, we cannot convey
	 * the more meaningful error code through
	 * this interface which is why we set a
	 * Rexx variable to the error number. The
	 * Rexx script can then take a look at this
	 * variable and decide which further steps
	 * it should take.
	 */
	msg->rm_Result1 = rc;
	msg->rm_Result2 = rc2;

	/* Turn the error number into a string as
	 * ARexx only deals with strings.
	 */
	char value[12];
	IUtility->SNPrintf(value, sizeof(value), "%ld", rc2);

	/* Build the name of the variable to set to
	 * the error number. We will use the name of
	 * the host name and append ".LASTERROR".
	 */
	IRexxSys->SetRexxVarFromMsg("RC2", value, msg);

	IExec->ReplyMsg(&msg->rm_Node);
}

BOOL
PutMsgTo(CONST_STRPTR name, struct Message *msg)
{
	BOOL done = FALSE;

	IExec->Forbid();

	struct MsgPort *port = IExec->FindPort(name);
	if (port != NULL)
	{
		IExec->PutMsg(port, msg);
		done = TRUE;
	}

	IExec->Permit();

	return done;
}


STRPTR DoRexx(STRPTR port, STRPTR command, int32 *rc, int32 *rc2)
{
	*rc = 0;
	*rc2 = 0;
	STRPTR result = NULL;
	STRPTR dup = NULL;

	struct MsgPort *replyPort = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END);
	if (replyPort == NULL)
	{
		return NULL;
	}

	struct RexxMsg *rexxMsg = IRexxSys->CreateRexxMsg(replyPort, NULL, NULL);
	((struct Node *)rexxMsg)->ln_Name = "REXX";
	if (rexxMsg == NULL)
	{
		IExec->FreeSysObject(ASOT_PORT, replyPort);
		return NULL;
	}
	BOOL sent = FALSE;


	rexxMsg->rm_Args[0] = IRexxSys->CreateArgstring(command, strlen(command));

	if (rexxMsg->rm_Args[0] != NULL)
	{
		rexxMsg->rm_Action = RXCOMM | RXFF_RESULT | RXFF_STRING;

		sent = PutMsgTo(port, (struct Message*)rexxMsg);

		if (sent)
		{
			IExec->WaitPort(replyPort);
			(void)IExec->GetMsg(replyPort);
		}
		else
		{

		}

		*rc = rexxMsg->rm_Result1;

		if (*rc == RC_OK)
		{
			if (rexxMsg->rm_Result2 != 0)
			{
				result = (STRPTR)rexxMsg->rm_Result2;
			}
		}
		else
		{
			*rc2 = rexxMsg->rm_Result2;
		}

		IRexxSys->DeleteArgstring(rexxMsg->rm_Args[0]);
		rexxMsg->rm_Args[0] = NULL;
	}

	IRexxSys->DeleteRexxMsg(rexxMsg);
	rexxMsg = NULL;

	IExec->FreeSysObject(ASOT_PORT, replyPort);
	replyPort = NULL;

	if (result != NULL)
	{
		dup = dupstr(result);

		IRexxSys->DeleteArgstring(result);
		result = NULL;
	}

	return dup;
}


struct RexxHost *CreateRexxHost(CONST_STRPTR PortName)
{
	struct RexxHost *newHost = IExec->AllocVecTags(sizeof(struct RexxHost),
	AVT_Type, MEMF_PRIVATE, AVT_ClearWithValue, 0, TAG_DONE);

	if (newHost == NULL)
	{
	return NULL;
	}

	IUtility->Strlcpy(newHost->PortName, PortName, sizeof(newHost->PortName));

	IExec->Forbid();

	/* Check if the name already exists */
	if (IExec->FindPort(PortName) != NULL)
	{
	int32 index = 1;
	do
	{
	IUtility->SNPrintf(newHost->PortName, sizeof(newHost->PortName), "%s.%ld", PortName, index);
	index++;

	if (IExec->FindPort(newHost->PortName) == NULL)
	{
	break;
	}
	} while (1);
	}

	newHost->Port = IExec->AllocSysObjectTags(ASOT_PORT,
	ASOPORT_Name, 	newHost->PortName,
	ASOPORT_Public, TRUE,
	TAG_DONE);

	IExec->Permit();

	if (newHost->Port == NULL)
	{
	IExec->FreeVec(newHost);
	return NULL;
	}

	return newHost;
}


void DeleteRexxHost(struct RexxHost *host)
{
	if (host)
	{
	if (host->Port)
	{
	struct RexxMsg *msg;

	IExec->Forbid();
	while ((msg = (struct RexxMsg *)IExec->GetMsg(host->Port)) != NULL)
	{
	msg->rm_Result1 = RC_FATAL;
	IExec->ReplyMsg((struct Message *)msg);
	}

	IExec->FreeSysObject(ASOT_PORT, host->Port);
	IExec->Permit();
	}

	IExec->FreeVec(host);
	}
}

void WaitRexxHost(struct RexxHost *rexxHost, int timeout)
{

	struct TimeRequest *req = NULL;
	uint32 timermask        = 0;

	if (timeout > 0)
	{
		req = OpenTimer();

		if (req != NULL)
		{
			timermask = 1L << req->Request.io_Message.mn_ReplyPort->mp_SigBit;

			req->Request.io_Command = TR_ADDREQUEST;
			req->Time.Seconds       = 0;
			req->Time.Microseconds  = timeout;

			IExec->SendIO(&req->Request);
		}
	}

	uint32 hostmask = 1L << rexxHost->Port->mp_SigBit;
	uint32 waitmask = timermask | hostmask | SIGBREAKF_CTRL_C;

	uint32 sigmask = IExec->Wait(waitmask);

	if (req != NULL)
	{
		IExec->AbortIO(&req->Request);
		IExec->WaitIO(&req->Request);
		CloseTimer(req);
	}

	if (sigmask & SIGBREAKF_CTRL_C)
	{
		return;
	}


}

struct ARexxMsg *GetMsgRexxHost(struct RexxHost *rexxHost)
{
	struct ARexxMsg *am = NULL;

	struct RexxMsg *rexxMsg = NULL;

	rexxMsg = (struct RexxMsg *)IExec->GetMsg(rexxHost->Port);
	if (rexxMsg != NULL)
	{
		if((am = IExec->AllocVecTags(sizeof(struct ARexxMsg),AVT_Type, MEMF_PRIVATE, AVT_ClearWithValue, 0, TAG_DONE)))
		{
			am->rexxMsg = rexxMsg;
			am->rexxHost = rexxHost;
			am->isReplied = FALSE;
		}

	}
	return am;
}

uint32 GetSignalRexxHost(struct RexxHost *rexxHost)
{
	return rexxHost->Port->mp_SigBit;
}


void ReplyARexxMsg(struct ARexxMsg *am, int rc, int rc2, STRPTR result)
{
	if(am)
	{
		if(!am->isReplied)
		{
			if(rc == 0)
			{
				ReturnRexxMsg(am->rexxMsg, result);
			}
			else
			{
				ReturnErrorMsg(am->rexxMsg, am->rexxHost->PortName,rc,rc2);
			}
			am->isReplied = TRUE;
		}
	}
}

STRPTR GetVarARexxMsg(struct ARexxMsg *am, STRPTR varname)
{
	STRPTR result = IExec->AllocVecTags(256,AVT_Type, MEMF_PRIVATE, AVT_ClearWithValue, 0, TAG_DONE);
	if(result)
	{
		IRexxSys->GetRexxVarFromMsg(varname, result, am->rexxMsg);
	}
	return result;
}

void SetVarARexxMsg(struct ARexxMsg *am, STRPTR varname, STRPTR value)
{
	IRexxSys->SetRexxVarFromMsg(varname, value, am->rexxMsg);
}

void DeleteARexxMsg(struct ARexxMsg *am)
{
	if(!am->isReplied)
	{
		IExec->ReplyMsg(&am->rexxMsg->rm_Node);
		am->isReplied = TRUE;
	}
	IExec->FreeVec(am);
}

STRPTR GetArgsARexxMsg(struct ARexxMsg *am)
{
	return am->rexxMsg->rm_Args[0];
}

MODULE = Amiga::ARexx              PACKAGE = Amiga::ARexx

PROTOTYPES: DISABLE


APTR Host_init(name)
    STRPTR name;
    CODE:
    	RETVAL = CreateRexxHost(name);
    OUTPUT:
    	RETVAL

void Host_delete(rexxhost)
	APTR rexxhost;
	CODE:
		DeleteRexxHost(rexxhost);

void Host_wait(rexxhost,timeout)
	APTR rexxhost
	int timeout
	CODE:
		WaitRexxHost(rexxhost,timeout);

uint32 Host_signal(rexxhost)
	APTR rexxhost
	CODE:
		RETVAL = GetSignalRexxHost(rexxhost);
	OUTPUT:
		RETVAL

APTR Host_getmsg(rexxhost)
	APTR rexxhost
	CODE:
		RETVAL = GetMsgRexxHost(rexxhost);
	OUTPUT:
		RETVAL

void Msg_reply(rexxmsg,rc,rc2,result)
	APTR rexxmsg
	int rc
	int rc2
	STRPTR result
	CODE:
		ReplyARexxMsg(rexxmsg,rc,rc2,result);

void Msg_delete(rexxmsg)
	APTR rexxmsg
	CODE:
		DeleteARexxMsg(rexxmsg);

STRPTR Msg_argstr(rexxmsg)
	APTR rexxmsg
	CODE:
		RETVAL = GetArgsARexxMsg(rexxmsg);
	OUTPUT:
		RETVAL

STRPTR Msg_getvar(rexxmsg,varname)
	APTR rexxmsg
	STRPTR varname
	PPCODE:
		RETVAL = GetVarARexxMsg(rexxmsg,varname);
		sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
		if (RETVAL) IExec->FreeVec(RETVAL);

void Msg_setvar(rexxmsg,varname,value)
	APTR rexxmsg
	STRPTR varname
	STRPTR value
	CODE:
		SetVarARexxMsg(rexxmsg,varname,value);

STRPTR _DoRexx(port,command,rc,rc2)
	STRPTR port
	STRPTR command
	int32 &rc
	int32 &rc2
	PPCODE:
		RETVAL = DoRexx(port,command,&rc,&rc2);
		sv_setiv(ST(2), (IV)rc);
		SvSETMAGIC(ST(2));
		sv_setiv(ST(3), (IV)rc2);
		SvSETMAGIC(ST(3));
		sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
		IExec->FreeVec(RETVAL);

