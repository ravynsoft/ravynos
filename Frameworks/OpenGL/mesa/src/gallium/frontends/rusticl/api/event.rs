use crate::api::icd::*;
use crate::api::types::*;
use crate::api::util::*;
use crate::core::event::*;
use crate::core::queue::*;

use rusticl_opencl_gen::*;
use rusticl_proc_macros::cl_entrypoint;
use rusticl_proc_macros::cl_info_entrypoint;

use std::collections::HashSet;
use std::mem::MaybeUninit;
use std::ptr;
use std::sync::Arc;

#[cl_info_entrypoint(cl_get_event_info)]
impl CLInfo<cl_event_info> for cl_event {
    fn query(&self, q: cl_event_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let event = self.get_ref()?;
        Ok(match *q {
            CL_EVENT_COMMAND_EXECUTION_STATUS => cl_prop::<cl_int>(event.status()),
            CL_EVENT_CONTEXT => {
                // Note we use as_ptr here which doesn't increase the reference count.
                let ptr = Arc::as_ptr(&event.context);
                cl_prop::<cl_context>(cl_context::from_ptr(ptr))
            }
            CL_EVENT_COMMAND_QUEUE => {
                let ptr = match event.queue.as_ref() {
                    // Note we use as_ptr here which doesn't increase the reference count.
                    Some(queue) => Arc::as_ptr(queue),
                    None => ptr::null_mut(),
                };
                cl_prop::<cl_command_queue>(cl_command_queue::from_ptr(ptr))
            }
            CL_EVENT_REFERENCE_COUNT => cl_prop::<cl_uint>(self.refcnt()?),
            CL_EVENT_COMMAND_TYPE => cl_prop::<cl_command_type>(event.cmd_type),
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

#[cl_info_entrypoint(cl_get_event_profiling_info)]
impl CLInfo<cl_profiling_info> for cl_event {
    fn query(&self, q: cl_profiling_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let event = self.get_ref()?;
        if event.cmd_type == CL_COMMAND_USER {
            // CL_PROFILING_INFO_NOT_AVAILABLE [...] if event is a user event object.
            return Err(CL_PROFILING_INFO_NOT_AVAILABLE);
        }

        Ok(match *q {
            CL_PROFILING_COMMAND_QUEUED => cl_prop::<cl_ulong>(event.get_time(EventTimes::Queued)),
            CL_PROFILING_COMMAND_SUBMIT => cl_prop::<cl_ulong>(event.get_time(EventTimes::Submit)),
            CL_PROFILING_COMMAND_START => cl_prop::<cl_ulong>(event.get_time(EventTimes::Start)),
            CL_PROFILING_COMMAND_END => cl_prop::<cl_ulong>(event.get_time(EventTimes::End)),
            // For now, we treat Complete the same as End
            CL_PROFILING_COMMAND_COMPLETE => cl_prop::<cl_ulong>(event.get_time(EventTimes::End)),
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

#[cl_entrypoint]
fn create_user_event(context: cl_context) -> CLResult<cl_event> {
    let c = context.get_arc()?;
    Ok(cl_event::from_arc(Event::new_user(c)))
}

#[cl_entrypoint]
fn retain_event(event: cl_event) -> CLResult<()> {
    event.retain()
}

#[cl_entrypoint]
fn release_event(event: cl_event) -> CLResult<()> {
    event.release()
}

#[cl_entrypoint]
fn wait_for_events(num_events: cl_uint, event_list: *const cl_event) -> CLResult<()> {
    let evs = cl_event::get_arc_vec_from_arr(event_list, num_events)?;

    // CL_INVALID_VALUE if num_events is zero or event_list is NULL.
    if evs.is_empty() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if events specified in event_list do not belong to the same context.
    let contexts: HashSet<_> = evs.iter().map(|e| &e.context).collect();
    if contexts.len() != 1 {
        return Err(CL_INVALID_CONTEXT);
    }

    // find all queues we have to flush
    for q in Event::deep_unflushed_queues(&evs) {
        q.flush(false)?;
    }

    // now wait on all events and check if we got any errors
    let mut err = false;
    for e in evs {
        err |= e.wait() < 0;
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the execution status of any of the events
    // in event_list is a negative integer value.
    if err {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    Ok(())
}

#[cl_entrypoint]
fn set_event_callback(
    event: cl_event,
    command_exec_callback_type: cl_int,
    pfn_event_notify: Option<FuncEventCB>,
    user_data: *mut ::std::os::raw::c_void,
) -> CLResult<()> {
    let e = event.get_ref()?;

    // CL_INVALID_VALUE [...] if command_exec_callback_type is not CL_SUBMITTED, CL_RUNNING, or CL_COMPLETE.
    if ![CL_SUBMITTED, CL_RUNNING, CL_COMPLETE].contains(&(command_exec_callback_type as cl_uint)) {
        return Err(CL_INVALID_VALUE);
    }

    // SAFETY: The requirements on `EventCB::new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb = unsafe { EventCB::new(pfn_event_notify, user_data)? };

    e.add_cb(command_exec_callback_type, cb);

    Ok(())
}

#[cl_entrypoint]
fn set_user_event_status(event: cl_event, execution_status: cl_int) -> CLResult<()> {
    let e = event.get_ref()?;

    // CL_INVALID_VALUE if the execution_status is not CL_COMPLETE or a negative integer value.
    if execution_status != CL_COMPLETE as cl_int && execution_status > 0 {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_OPERATION if the execution_status for event has already been changed by a
    // previous call to clSetUserEventStatus.
    if e.status() != CL_SUBMITTED as cl_int {
        return Err(CL_INVALID_OPERATION);
    }

    e.set_user_status(execution_status);
    Ok(())
}

pub fn create_and_queue(
    q: Arc<Queue>,
    cmd_type: cl_command_type,
    deps: Vec<Arc<Event>>,
    event: *mut cl_event,
    block: bool,
    work: EventSig,
) -> CLResult<()> {
    let e = Event::new(&q, cmd_type, deps, work);
    cl_event::leak_ref(event, &e);
    q.queue(e);
    if block {
        q.flush(true)?;
    }
    Ok(())
}
