#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#undef __USE_INLINE__
#include <exec/types.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>



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



uint32 WaitTimeout(uint32 signalmask , int timeout)
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

	uint32 waitmask = timermask |  signalmask | SIGBREAKF_CTRL_C;

	uint32 sigmask = IExec->Wait(waitmask);

	if (req != NULL)
	{
		IExec->AbortIO(&req->Request);
		IExec->WaitIO(&req->Request);
		CloseTimer(req);
	}

	/* remove the timer mask bit */

	return sigmask & (~timermask );
}



MODULE = Amiga::Exec              PACKAGE = Amiga::Exec

PROTOTYPES: DISABLE


uint32 _Wait(signalmask,timeout)
    uint32 signalmask;
    uint32 timeout;
    CODE:
    	RETVAL = WaitTimeout(signalmask,timeout);
    OUTPUT:
    	RETVAL

