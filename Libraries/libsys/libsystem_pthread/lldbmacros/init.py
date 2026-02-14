from xnu import *
import struct

def GetSeqCount(seq):
	return (seq >> 8)

def GetLSeqBits(seq):
	rv = ""
	if seq & 0x1:
		rv += "K"
	if seq & 0x2:
		rv += "E"
	if seq & 0x4:
		rv += "W"
	if seq & 0x20:
		rv += "M"
	if seq & 0x40:
		rv += "U"
	if seq & 0x80:
		rv += "I"
	return rv

def GetSSeqBits(seq):
	rv = ""
	if seq & 0x1:
		rv += "S"
	if seq & 0x2:
		rv += "I"
	if seq & 0x4:
		rv += "Ws"
	return rv

def GetLSeqSummary(seq):
	return "{:d} {:s}".format(GetSeqCount(seq), GetLSeqBits(seq))

def GetSSeqSummary(seq):
	return "{:d} {:s}".format(GetSeqCount(seq), GetSSeqBits(seq))

@header("{0: <24s} {1: <16s} {2: <16s} {3: <16s} {4: <16s}".format('sig', 'tid', 'options', 'lseq', 'useq'))
def GetUserMutexSummary(task, uaddr):
	if int(task.t_flags) & 0x1:
		mtxlayout = "QIIhhIQIII"
		padoffset = 1
	else:
		mtxlayout = "QIIhhQIII"
		padoffset = 0

	data = GetUserDataAsString(task, unsigned(uaddr), struct.calcsize(mtxlayout))
	info = struct.unpack(mtxlayout, data)

	format = "{0: <24s} {1: <16s} {2: <16s} {3: <16s} {4: <16s}"
	sigstr = str("{0: <#020x}".format(info[0]))

	# the options field dictates whether we were created misaligned
	if info[2] & 0x800:
		lseq = info[7+padoffset]
		useq = info[8+padoffset]
	else:
		lseq = info[6+padoffset]
		useq = info[7+padoffset]

	return format.format(sigstr, hex(info[5+padoffset]), hex(info[2]), hex(lseq), hex(useq))

@lldb_command('showusermutex')
def PthreadShowUserMutex(cmd_args=None):
	"""
	display information about a userspace mutex at a given address
	Syntax: (lldb) showusermutex <task_t> <uaddr>
	"""
	if not cmd_args:
		raise ArgumentError("No arguments passed")
	task = kern.GetValueFromAddress(cmd_args[0], "task_t")
	uaddr = kern.GetValueFromAddress(cmd_args[1], "user_addr_t")

	print GetUserMutexSummary.header
	print GetUserMutexSummary(task, uaddr)

@lldb_type_summary(['ksyn_wait_queue *', 'ksyn_wait_queue_t'])
@header("{:<20s} {:<20s} {:<10s} {:<6s} {:<6s} {:<8s} {:<8s} {:<8s} {:<8s}".format('kwq', 'uaddr', 'type', 'pflags', 'kflags', 'refs', 'indrop', 'waiters', 'preposts'))
def GetKwqSummary(kwq):
	format = "{:<#20x} {:<#20x} {:<10s} {:<6s} {:<6s} {:<8d} {:<8d} {:<8d} {:<8d}\n"
	kwq = Cast(kwq, "ksyn_wait_queue_t")

	kwqtype = ""
	if kwq.kw_type & 0xff == 0x01:
		kwqtype = "mtx"
	if kwq.kw_type & 0xff == 0x02:
		kwqtype = "cvar"
	if kwq.kw_type & 0xff == 0x04:
		kwqtype = "rwl"
	if kwq.kw_type & 0xff == 0x05:
		kwqtype = "sema"

	if kwq.kw_type & 0x1000 == 0x1000:
		kwqtype += "W" # INWAIT
	if kwq.kw_type & 0x2000 == 0x2000:
		kwqtype += "D" # INDROP

	pflags = ""
	if kwq.kw_pflags & 0x2:
		pflags += "H" # INHASH
	if kwq.kw_pflags & 0x4:
		pflags += "S" # SHARED
	if kwq.kw_pflags & 0x8:
		pflags += "W" # WAITING
	if kwq.kw_pflags & 0x10:
		pflags += "F" # FREELIST

	kflags = ""
	if kwq.kw_kflags & 0x1:
		kflags += "C" # INITCLEARED
	if kwq.kw_kflags & 0x2:
		kflags += "Z" # ZEROED
	if kwq.kw_kflags & 0x4:
		kflags += "Q" # QOS APPLIED
	if kwq.kw_kflags & 0x8:
		kflags += "O" # OVERLAP

	rs = format.format(kwq, kwq.kw_addr, kwqtype, pflags, kflags, kwq.kw_iocount, kwq.kw_dropcount, kwq.kw_inqueue, kwq.kw_fakecount)

	rs += "\t{:<10s} {:<10s} {:<10s} {:<10s} {:<10s} {:<10s} {:<10s}\n".format('lowest', 'highest', 'lword', 'uword', 'sword', 'last', 'next')
	rs += "\t{:<10d} {:<10d} {:<10s} {:<10d} {:<10s} {:<10s} {:<10s}\n".format(
			GetSeqCount(kwq.kw_lowseq), GetSeqCount(kwq.kw_highseq),
			GetLSeqSummary(kwq.kw_lword), GetSeqCount(kwq.kw_uword),
			GetSSeqSummary(kwq.kw_sword), GetSSeqSummary(kwq.kw_lastseqword),
			GetSSeqSummary(kwq.kw_nextseqword))

	rs += "\t{:<10s} {:<10s} {:<10s} {:<10s} {:<10s} {:<10s} {:<10s}\n".format(
			'pposts', 'lseq', 'sseq', 'intr', 'count', 'seq', 'bits')

	intr_type = "NONE"
	if kwq.kw_intr.type == 0x1:
		intr_type = "READ"
	elif kwq.kw_intr.type == 0x2:
		intr_type = "WRITE"

	rs += "\t{:<10d} {:<10s} {:<10s} {:<10s} {:<10d} {:<10s} {:<10s}\n".format(
			kwq.kw_prepost.count,
			GetLSeqSummary(kwq.kw_prepost.lseq), GetSSeqSummary(kwq.kw_prepost.sseq),
			intr_type, kwq.kw_intr.count,
			GetSSeqSummary(kwq.kw_intr.seq), GetSSeqSummary(kwq.kw_intr.returnbits))

	rs += "\twaiting readers:\n"
	for kwe in IterateTAILQ_HEAD(kwq.kw_ksynqueues[0].ksynq_kwelist, 'kwe_list'):
		rs += "\t" + GetKweSummary.header + "\n"
		rs += "\t" + GetKweSummary(kwe) + "\n"

	rs += "\twaiting writers:\n"
	for kwe in IterateTAILQ_HEAD(kwq.kw_ksynqueues[1].ksynq_kwelist, 'kwe_list'):
		rs += "\t" + GetKweSummary.header + "\n"
		rs += "\t" + GetKweSummary(kwe) + "\n"

	if kwq.kw_turnstile:
		rs += GetTurnstileSummary.header + "\n"
		rs += GetTurnstileSummary(Cast(kwq.kw_turnstile, "struct turnstile *"))

	return rs

@lldb_type_summary(['ksyn_waitq_element *', 'ksyn_waitq_element_t'])
@header("{:<20s} {:<20s} {:<10s} {:<10s} {:<20s} {:<20s}".format('kwe', 'kwq', 'lseq', 'state', 'uthread', 'thread'))
def GetKweSummary(kwe):
	format = "{:<#20x} {:<#20x} {:<10s} {:<10s} {:<#20x} {:<#20x}"
	kwe = Cast(kwe, 'struct ksyn_waitq_element *')
	state = ""
	if kwe.kwe_state == 1:
		state = "INWAIT"
	elif kwe.kwe_state == 2:
		state = "PPOST"
	elif kwe.kwe_state == 3:
		state = "BROAD"
	else:
		state = "{:#10x}".format(kwe.kwe_state)
	return format.format(kwe, kwe.kwe_kwqqueue, GetLSeqSummary(kwe.kwe_lockseq), state, kwe.kwe_uth, kwe.kwe_thread)

@header("{0: <24s} {1: <24s} {2: <24s}".format('thread', 'thread_id', 'uthread'))
def GetPthreadSummary(thread):
	format = "{0: <24s} {1: <24s} {2: <24s}"

	threadstr = str("{0: <#020x}".format(thread))
	if int(thread.static_param):
		threadstr += "[WQ]"

	uthread = Cast(thread.uthread, "uthread_t")
	uthreadstr = str("{0: <#020x}".format(uthread))


	return format.format(threadstr, hex(thread.thread_id), uthreadstr)

@header("{0: <24s} {1: <24s} {2: <10s} {3: <10s} {4: <10s} {5: <10s} {6: <10s}".format('proc', 'wq', 'sched', 'req', 'idle', 'wq_flags', 'wq_lflags'))
def GetPthreadWorkqueueSummary(wq):
	format = "{0: <24s} {1: <24s} {2: <10d} {3: <10d} {4: <10d} {5: <10s} {6: <10s}"
	procstr = str("{0: <#020x}".format(wq.wq_proc))
	wqstr = str("{0: <#020x}".format(wq))
	
	flags = []
	if wq.wq_flags & 0x1:
		flags.append("I")
	if wq.wq_flags & 0x2:
		flags.append("R")
	if wq.wq_flags & 0x4:
		flags.append("E")
		
	wqflags = []
	if wq.wq_lflags & 0x1:
		wqflags.append("B")
	if wq.wq_lflags & 0x2:
		wqflags.append("W")
	if wq.wq_lflags & 0x4:
		wqflags.append("C")
	if wq.wq_lflags & 0x8:
		wqflags.append("L")
	
	return format.format(procstr, wqstr, wq.wq_threads_scheduled, wq.wq_reqcount, wq.wq_thidlecount, "".join(flags), "".join(wqflags))

@header("{0: <24s} {1: <5s} {2: <5s} {3: <5s} {4: <5s} {5: <5s} {6: <5s} {7: <5s}".format('category', 'uint', 'uinit', 'lgcy', 'util', 'bckgd', 'maint', 'event'))
def GetPthreadWorkqueueDetail(wq):
	format = "  {0: <22s} {1: <5d} {2: <5d} {3: <5d} {4: <5d} {5: <5d} {6: <5d} {7: <5d}"
	# requests
	schedstr = format.format('scheduled', wq.wq_thscheduled_count[0], wq.wq_thscheduled_count[1], wq.wq_thscheduled_count[2], wq.wq_thscheduled_count[3], wq.wq_thscheduled_count[4], wq.wq_thscheduled_count[5], wq.wq_thscheduled_count[6])
	activestr = format.format('active', wq.wq_thactive_count[0], wq.wq_thactive_count[1], wq.wq_thactive_count[2], wq.wq_thactive_count[3], wq.wq_thactive_count[4], wq.wq_thactive_count[5], wq.wq_thactive_count[6])
	return "\n".join([schedstr, activestr])

@lldb_command('showthreadpsynch')
def PthreadCurrentMutex(cmd_args=None):
	"""
	display information about a thread's pthread state
	Syntax: (lldb) showthreadpsync <thread_t>
	"""
	if not cmd_args:
		raise ArgumentError("No arguments passed")

	thread = kern.GetValueFromAddress(cmd_args[0], "thread_t")
	print GetPthreadSummary.header
	print GetPthreadSummary(thread)

	uthread = Cast(thread.uthread, "uthread_t")
	kwe = Cast(addressof(uthread.uu_save.uus_kwe), 'struct ksyn_waitq_element *')
	if not kwe or not kwe.kwe_kwqqueue:
		print GetKweSummary.header
		print GetKweSummary(kwe)
	else:
		print GetKwqSummary.header
		print GetKwqSummary(kwe.kwe_kwqqueue)

@lldb_command('showpthreadkwq')
def PthreadShowKsynQueue(cmd_args=None):
	"""
	display information about a pthread ksyn_wait_queue_t
	Syntax: (lldb) showpthreadkwq <ksyn_wait_queue_t>
	"""
	if not cmd_args:
		raise ArgumentError("No arguments passed")

	kwq = kern.GetValueFromAddress(cmd_args[0], "ksyn_wait_queue_t")
	print GetKwqSummary.header
	print GetKwqSummary(kwq)

@lldb_command('showpthreadkwe')
def PthreadShowKsynElement(cmd_args=None):
	"""
	display information about a thread's ksyn_waitq_element
	Syntax: (lldb) showpthreadkwe <ksyn_waitq_element_t>	
	"""
	if not cmd_args:
		raise ArgumentError("No arguments passed")

	kwe = kern.GetValueFromAddress(cmd_args[0], "struct ksyn_waitq_element *")
	print GetKweSummary.header
	print GetKweSummary(kwe)

@lldb_command('showpthreadworkqueue')
def ShowPthreadWorkqueue(cmd_args=None):
	"""
	display information about a processes' pthread workqueue
	Syntax: (lldb) showpthreadworkqueue <proc_t>
	"""
	
	if not cmd_args:
		raise ArgumentError("No arguments passed")
		
	proc = kern.GetValueFromAddress(cmd_args[0], "proc_t")
	wq = Cast(proc.p_wqptr, "struct workqueue *");
	
	print GetPthreadWorkqueueSummary.header
	print GetPthreadWorkqueueSummary(wq)
	
	print GetPthreadWorkqueueDetail.header
	print GetPthreadWorkqueueDetail(wq)

def IterateTAILQ_HEAD(headval, element_name):
    """ iterate over a TAILQ_HEAD in kernel. refer to bsd/sys/queue.h
        params:
            headval     - value : value object representing the head of the list
            element_name- str          :  string name of the field which holds the list links.
        returns:
            A generator does not return. It is used for iterating.
            value : an object that is of type as headval->tqh_first. Always a pointer object
        example usage:
          list_head = kern.GetGlobalVariable('mountlist')
          for entryobj in IterateTAILQ_HEAD(list_head, 'mnt_list'):
            print GetEntrySummary(entryobj)
    """
    iter_val = headval.tqh_first
    while unsigned(iter_val) != 0 :
        yield iter_val
        iter_val = iter_val.__getattr__(element_name).tqe_next
    #end of yield loop

def __lldb_init_module(debugger, internal_dict):
	pass
