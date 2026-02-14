#!/usr/local/bin/luatrace -s

trace_codename = function(codename, callback)
	local debugid = trace.debugid(codename)
	if debugid ~= 0 then
		trace.single(debugid,callback)
	else
		printf("WARNING: Cannot locate debugid for '%s'\n", codename)
	end
end

initial_timestamp = 0
get_prefix = function(buf)
	if initial_timestamp == 0 then
		initial_timestamp = buf.timestamp
	end
	local secs = trace.convert_timestamp_to_nanoseconds(buf.timestamp - initial_timestamp) / 1000000000

	local prefix
	if trace.debugid_is_start(buf.debugid) then
		prefix = "→"
	elseif trace.debugid_is_end(buf.debugid) then
		prefix = "←"
	else
		prefix = "↔"
	end

	local proc
	proc = buf.command

	return string.format("%s %6.9f %-17s [%05d.%06x] %-35s",
		prefix, secs, proc, buf.pid, buf.threadid, buf.debugname)
end

get_count = function(val)
	return ((val & 0xffffff00) >> 8)
end

get_kwq_type = function(val)
	if val & 0xff == 0x1 then
		return "MTX"
	elseif val & 0xff == 0x2 then
		return "CVAR"
	elseif val & 0xff == 0x4 then
		return "RWL"
	else
		return string.format("0x%04x", val)
	end
end

decode_lval = function(lval)
	local kbit = " "
	if lval & 0x1 ~= 0 then
		kbit = "K"
	end
	local ebit = " "
	if lval & 0x2 ~= 0 then
		ebit = "E"
	end
	local wbit = " "
	if lval & 0x4 ~= 0 then
		wbit = "W"
	end

	local count = lval >> 8
	return string.format("[0x%06x, %s%s%s]", count, wbit, ebit, kbit)
end

decode_sval = function(sval)
	local sbit = " "
	if sval & 0x1 ~= 0 then
		sbit = "S"
	end
	local ibit = " "
	if sval & 0x2 ~= 0 then
		ibit = "I"
	end

	local count = sval >> 8
	return string.format("[0x%06x,  %s%s]", count, ibit, sbit)
end

decode_cv_sval = function(sval)
	local sbit = " "
	if sval & 0x1 ~= 0 then
		sbit = "C"
	end
	local ibit = " "
	if sval & 0x2 ~= 0 then
		ibit = "P"
	end

	local count = sval >> 8
	return string.format("[0x%06x,  %s%s]", count, ibit, sbit)
end

trace_codename("psynch_mutex_lock_updatebits", function(buf)
	local prefix = get_prefix(buf)
	if buf[4] == 0 then
		printf("%s\tupdated lock bits, pre-kernel\taddr: 0x%016x\toldl: %s\tnewl: %s\n",
				prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]))
	else
		printf("%s\tupdated lock bits, post-kernel\taddr: 0x%016x\toldl: %s\tnewl: %s\n",
				prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]))
	end
end)

trace_codename("psynch_mutex_unlock_updatebits", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tupdated unlock bits\t\taddr: 0x%016x\toldl: %s\tnewl: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]))
end)

trace_codename("psynch_ffmutex_lock_updatebits", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tlock path, bits update\t\taddr: 0x%016x\toldl: %s\toldu: %s\twaiters: %d\n",
				prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), get_count(buf[2]) - get_count(buf[3]))
	else
		printf("%s\tlock path, bits update\t\taddr: 0x%016x\tnewl: %s\tnewu: %s\twaiters: %d\n",
				prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), get_count(buf[2]) - get_count(buf[3]))
	end
end)

trace_codename("psynch_ffmutex_unlock_updatebits", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tunlock path, update bits\taddr: 0x%016x\toldl: %s\tnewl: %s\tnewu: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]), decode_sval(buf[4]))
end)

trace_codename("psynch_ffmutex_wake", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tfirst fit kernel wake\t\taddr: 0x%016x\tlval: %s\tuval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]))
end)

trace_codename("psynch_ffmutex_wait", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tfirstfit kernel wait\t\taddr: 0x%016x\tlval: %s\tuval: %s\tflags: 0x%x\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), buf[4])
	else
		printf("%s\tfirstfit kernel wait\t\taddr: 0x%016x\trval: %s\n",
			prefix, buf[1], decode_lval(buf[2]))
	end
end)

trace_codename("psynch_mutex_ulock", function(buf)
	local prefix = get_prefix(buf)

	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tlock busy, waiting in kernel\taddr: 0x%016x\tlval: %s\tsval: %s\towner_tid: 0x%x\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), buf[4])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tlock acquired from kernel\taddr: 0x%016x\tupdt: %s\n",
			prefix, buf[1], decode_lval(buf[2]))
	else
		printf("%s\tlock taken userspace\t\taddr: 0x%016x\tlval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]))
	end
end)

trace_codename("psynch_mutex_utrylock_failed", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tmutex trybusy\t\t\taddr: 0x%016x\tlval: %s\tsval: %s\towner: 0x%x\n", prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), buf[4])
end)

trace_codename("psynch_mutex_uunlock", function(buf)
	local prefix = get_prefix(buf)

	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tunlock, signalling kernel\taddr: 0x%016x\tlval: %s\tsval: %s\towner_tid: 0x%x\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]), buf[4])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tunlock, waiters signalled\taddr: 0x%016x\tupdt: %s\n",
			prefix, buf[1], decode_lval(buf[2]))
	else
		printf("%s\tunlock, no kernel waiters\taddr: 0x%016x\tlval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]))
	end
end)

trace_codename("psynch_mutex_clearprepost", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tclear prepost\t\t\taddr: 0x%016x\tlval: %s\tsval: %s\n",
		prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]))
end)

trace_codename("psynch_mutex_markprepost", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tmark prepost\t\t\taddr: 0x%016x\tlval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_sval(buf[3]))
	else
		printf("%s\tmark prepost\t\t\taddr: 0x%016x\tcleared: %d\n",
			prefix, buf[1], buf[2])
	end
end)

trace_codename("psynch_mutex_kwqallocate", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tkernel kwq allocated\t\taddr: 0x%016x\ttype: %s\tkwq: 0x%016x\n",
			prefix, buf[1], get_kwq_type(buf[2]), buf[3])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tkernel kwq allocated\t\taddr: 0x%016x\tlval: %s\tuval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]), decode_sval(buf[4]))
	end
end)

trace_codename("psynch_mutex_kwqdeallocate", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tkernel kwq deallocated\t\taddr: 0x%016x\ttype: %s\tfreenow: %d\n",
			prefix, buf[1], get_kwq_type(buf[2]), buf[3])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tkernel kwq deallocated\t\taddr: 0x%016x\tlval: %s\tuval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]), decode_sval(buf[4]))
	end
end)

trace_codename("psynch_mutex_kwqprepost", function(buf)
	local prefix = get_prefix(buf)
	if buf[4] == 0 then
		printf("%s\tkernel prepost incremented\taddr: 0x%016x\tlval: %s\tinqueue: %d\n",
			prefix, buf[1], decode_lval(buf[2]), buf[3])
	elseif buf[4] == 1 then
		printf("%s\tkernel prepost decremented\taddr: 0x%016x\tlval: %s\tremaining: %d\n",
			prefix, buf[1], decode_lval(buf[2]), buf[3])
	elseif buf[4] == 2 then
		printf("%s\tkernel prepost cleared\t\taddr: 0x%016x\tlval: %s\n", prefix,
			buf[1], decode_lval(buf[2]))
	end
end)

trace_codename("psynch_mutex_kwqcollision", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tkernel kwq collision\t\taddr: 0x%016x\ttype: %d\n", prefix,
		buf[1], buf[2])
end)

trace_codename("psynch_mutex_kwqsignal", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tkernel mutex signal\t\taddr: 0x%016x\tkwe: 0x%16x\ttid: 0x%x\tinqueue: %d\n",
			prefix, buf[1], buf[2], buf[3], buf[4]);
	else
		printf("%s\tkernel mutex signal\t\taddr: 0x%016x\tkwe: 0x%16x\tret: 0x%x\n",
			prefix, buf[1], buf[2], buf[3]);
	end
end)

trace_codename("psynch_mutex_kwqwait", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tkernel mutex wait\t\taddr: 0x%016x\tinqueue: %d\tprepost: %d\tintr: %d\n",
		prefix, buf[1], buf[2], buf[3], buf[4])
end)

trace_codename("psynch_cvar_kwait", function(buf)
	local prefix = get_prefix(buf)
	if buf[4] == 0 then
		printf("%s\tkernel condvar wait\t\taddr: 0x%016x\tmutex: 0x%016x\tcgen: 0x%x\n",
			prefix, buf[1], buf[2], buf[3])
	elseif buf[4] == 1 then
		printf("%s\tkernel condvar sleep\t\taddr: 0x%016x\tflags: 0x%x\n",
			prefix, buf[1], buf[3])
	elseif buf[4] == 2 then
		printf("%s\tkernel condvar wait return\taddr: 0x%016x\terror: 0x%x\tupdt: 0x%x\n",
			prefix, buf[1], buf[2], buf[3])
	elseif buf[4] == 3 and (buf[2] & 0xff) == 60 then
		printf("%s\tkernel condvar timeout\t\taddr: 0x%016x\terror: 0x%x\n",
			prefix, buf[1], buf[2])
	elseif buf[4] == 3 then
		printf("%s\tkernel condvar wait error\taddr: 0x%016x\terror: 0x%x\n",
			prefix, buf[1], buf[2])
	elseif buf[4] == 4 then
		printf("%s\tkernel condvar wait return\taddr: 0x%016x\tupdt: 0x%x\n",
			prefix, buf[1], buf[2])
	end
end)

trace_codename("psynch_cvar_clrprepost", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tkernel condvar clear prepost:\taddr: 0x%016x\ttype: 0x%x\tprepost seq: %s\n",
		prefix, buf[1], buf[2], decode_lval(buf[3]))
end)

trace_codename("psynch_cvar_freeitems", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tcvar free fake/prepost items\taddr: 0x%016x\ttype: %d\t\t\tupto: %s\tall: %d\n",
			prefix, buf[1], buf[2], decode_lval(buf[3]), buf[4])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tcvar free fake/prepost items\taddr: 0x%016x\tfreed: %d\tsignaled: %d\tinqueue: %d\n",
			prefix, buf[1], buf[2], buf[3], buf[4])
	elseif buf[4] == 1 then
		printf("%s\tcvar free, signalling waiter\taddr: 0x%016x\tinqueue: %d\tkwe: 0x%016x\n",
			prefix, buf[1], buf[3], buf[2])
	elseif buf[4] == 2 then
		printf("%s\tcvar free, removing fake\taddr: 0x%016x\tinqueue: %d\tkwe: 0x%016x\n",
			prefix, buf[1], buf[3], buf[2])
	end
end)

trace_codename("psynch_cvar_signal", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tkernel cvar signal\t\taddr: 0x%016x\tfrom: %s\tupto: %s\tbroad: %d\n",
			prefix, buf[1], decode_lval(buf[2]), decode_lval(buf[3]), buf[4])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tkernel cvar signal\t\taddr: 0x%016x\tupdt: %s\n",
			prefix, buf[1], decode_cv_sval(buf[2]))
	else
		printf("%s\tkernel cvar signal\t\taddr: 0x%016x\tsignalled waiters (converted to broadcast: %d)\n",
			prefix, buf[1], buf[2])
	end
end)

trace_codename("psynch_cvar_broadcast", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tupto: %s\tinqueue: %d\n",
			prefix, buf[1], decode_lval(buf[2]), buf[3])
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tupdt: %s\n",
			prefix, buf[1], decode_lval(buf[2]))
	elseif buf[4] == 1 then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tsignalling: 0x%16x\n",
			prefix, buf[1], buf[2])
	elseif buf[4] == 2 then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tremoving fake: 0x%16x\tstate: %d\n",
			prefix, buf[1], buf[2], buf[3])
	elseif buf[4] == 3 then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tprepost\tlval: %s\tsval: %s\n",
			prefix, buf[1], decode_lval(buf[2]), decode_cv_sval(buf[3]))
	elseif buf[4] == 4 then
		printf("%s\tkernel cvar broadcast\t\taddr: 0x%016x\tbroadcast prepost: 0x%016x\n",
			prefix, buf[1], buf[2])
	end
end)

trace_codename("psynch_cvar_zeroed", function(buf)
	local prefix = get_prefix(buf)
	printf("%s\tkernel cvar zeroed\t\taddr: 0x%016x\tlval: %s\tsval: %s\tinqueue: %d\n",
		prefix, buf[1], decode_lval(buf[2]), decode_cv_sval(buf[3]), buf[4])
end)

trace_codename("psynch_cvar_updateval", function(buf)
	local prefix = get_prefix(buf)
	if trace.debugid_is_start(buf.debugid) then
		printf("%s\tcvar updateval\t\t\taddr: 0x%016x\tlval: %s\tsval: %s\tupdateval: %s\n",
			prefix, buf[1], decode_lval(buf[2] & 0xffffffff), decode_cv_sval(buf[2] >> 32), decode_cv_sval(buf[3]))
	elseif trace.debugid_is_end(buf.debugid) then
		printf("%s\tcvar updateval (updated)\taddr: 0x%016x\tlval: %s\tsval: %s\tdiffgen: %d\tneedsclear: %d\n",
			prefix, buf[1], decode_lval(buf[2] & 0xffffffff), decode_cv_sval(buf[2] >> 32), buf[3] >> 32, buf[3] & 0x1)
	end
end)

