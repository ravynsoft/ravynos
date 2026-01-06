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
workqueue_ptr_map = {};
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
	if buf.command ~= "kernel_task" then
		proc = buf.command
		workqueue_ptr_map[buf[1]] = buf.command
	elseif workqueue_ptr_map[buf[1]] ~= nil then
		proc = workqueue_ptr_map[buf[1]]
	else
		proc = "UNKNOWN"
	end

	return string.format("%s %6.9f %-17s [%05d.%06x] %-24s",
		prefix, secs, proc, buf.pid, buf.threadid, buf.debugname)
end

trace_codename("pthread_thread_create", function(buf)
    local prefix = get_prefix(buf)
    if trace.debugid_is_start(buf.debugid) then
        printf("%s\tthread creation request: 0x%x\n", prefix, buf[1])
    elseif trace.debugid_is_end(buf.debugid) then
        printf("%s\t  thread creation complete: pthread 0x%x (error: %d)\n", prefix, buf[2], buf[1])
    elseif buf[4] == 2 then
        printf("%s\t  thread stack created: 0x%x + 0x%x\n", prefix, buf[2], buf[1])
    elseif buf[4] == 3 then
        printf("%s\t  thread using custom stack")
    end
end)

trace_codename("pthread_thread_terminate", function(buf)
    local prefix = get_prefix(buf)
    if trace.debugid_is_start(buf.debugid) then
        printf("%s\tthread terminate: stack 0x%x + 0x%x, kthport 0x%x\n", prefix, buf[1], buf[2], buf[3])
    elseif trace.debugid_is_end(buf.debugid) then
        printf("%s\t  thread terminate: ret %d\n", prefix, buf[1])
    end
end)

-- The trace codes we need aren't enabled by default
darwin.sysctlbyname("kern.pthread_debug_tracing", 1)
completion_handler = function()
	darwin.sysctlbyname("kern.pthread_debug_tracing", 0)
end
trace.set_completion_handler(completion_handler)
