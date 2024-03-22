use crate::api::icd::*;
use crate::api::types::*;
use crate::core::context::*;
use crate::core::queue::*;
use crate::impl_cl_type_trait;

use mesa_rust::pipe::query::*;
use mesa_rust_gen::*;
use mesa_rust_util::static_assert;
use rusticl_opencl_gen::*;

use std::collections::HashSet;
use std::slice;
use std::sync::Arc;
use std::sync::Condvar;
use std::sync::Mutex;
use std::sync::MutexGuard;
use std::time::Duration;

// we assert that those are a continous range of numbers so we won't have to use HashMaps
static_assert!(CL_COMPLETE == 0);
static_assert!(CL_RUNNING == 1);
static_assert!(CL_SUBMITTED == 2);
static_assert!(CL_QUEUED == 3);

pub type EventSig = Box<dyn FnOnce(&Arc<Queue>, &QueueContext) -> CLResult<()>>;

pub enum EventTimes {
    Queued = CL_PROFILING_COMMAND_QUEUED as isize,
    Submit = CL_PROFILING_COMMAND_SUBMIT as isize,
    Start = CL_PROFILING_COMMAND_START as isize,
    End = CL_PROFILING_COMMAND_END as isize,
}

#[derive(Default)]
struct EventMutState {
    status: cl_int,
    cbs: [Vec<EventCB>; 3],
    work: Option<EventSig>,
    time_queued: cl_ulong,
    time_submit: cl_ulong,
    time_start: cl_ulong,
    time_end: cl_ulong,
}

pub struct Event {
    pub base: CLObjectBase<CL_INVALID_EVENT>,
    pub context: Arc<Context>,
    pub queue: Option<Arc<Queue>>,
    pub cmd_type: cl_command_type,
    pub deps: Vec<Arc<Event>>,
    state: Mutex<EventMutState>,
    cv: Condvar,
}

impl_cl_type_trait!(cl_event, Event, CL_INVALID_EVENT);

// TODO shouldn't be needed, but... uff C pointers are annoying
unsafe impl Send for Event {}
unsafe impl Sync for Event {}

impl Event {
    pub fn new(
        queue: &Arc<Queue>,
        cmd_type: cl_command_type,
        deps: Vec<Arc<Event>>,
        work: EventSig,
    ) -> Arc<Event> {
        Arc::new(Self {
            base: CLObjectBase::new(),
            context: queue.context.clone(),
            queue: Some(queue.clone()),
            cmd_type: cmd_type,
            deps: deps,
            state: Mutex::new(EventMutState {
                status: CL_QUEUED as cl_int,
                work: Some(work),
                ..Default::default()
            }),
            cv: Condvar::new(),
        })
    }

    pub fn new_user(context: Arc<Context>) -> Arc<Event> {
        Arc::new(Self {
            base: CLObjectBase::new(),
            context: context,
            queue: None,
            cmd_type: CL_COMMAND_USER,
            deps: Vec::new(),
            state: Mutex::new(EventMutState {
                status: CL_SUBMITTED as cl_int,
                ..Default::default()
            }),
            cv: Condvar::new(),
        })
    }

    pub fn from_cl_arr(events: *const cl_event, num_events: u32) -> CLResult<Vec<Arc<Event>>> {
        let s = unsafe { slice::from_raw_parts(events, num_events as usize) };
        s.iter().map(|e| e.get_arc()).collect()
    }

    fn state(&self) -> MutexGuard<EventMutState> {
        self.state.lock().unwrap()
    }

    pub fn status(&self) -> cl_int {
        self.state().status
    }

    fn set_status(&self, lock: &mut MutexGuard<EventMutState>, new: cl_int) {
        lock.status = new;

        // signal on completion or an error
        if new <= CL_COMPLETE as cl_int {
            self.cv.notify_all();
        }

        // on error we need to call the CL_COMPLETE callbacks
        let cb_idx = if new < 0 { CL_COMPLETE } else { new as u32 };

        if [CL_COMPLETE, CL_RUNNING, CL_SUBMITTED].contains(&cb_idx) {
            if let Some(cbs) = lock.cbs.get_mut(cb_idx as usize) {
                cbs.drain(..).for_each(|cb| cb.call(self, new));
            }
        }
    }

    pub fn set_user_status(&self, status: cl_int) {
        let mut lock = self.state();
        self.set_status(&mut lock, status);
    }

    pub fn is_error(&self) -> bool {
        self.status() < 0
    }

    pub fn is_user(&self) -> bool {
        self.cmd_type == CL_COMMAND_USER
    }

    pub fn set_time(&self, which: EventTimes, value: cl_ulong) {
        let mut lock = self.state();
        match which {
            EventTimes::Queued => lock.time_queued = value,
            EventTimes::Submit => lock.time_submit = value,
            EventTimes::Start => lock.time_start = value,
            EventTimes::End => lock.time_end = value,
        }
    }

    pub fn get_time(&self, which: EventTimes) -> cl_ulong {
        let lock = self.state();

        match which {
            EventTimes::Queued => lock.time_queued,
            EventTimes::Submit => lock.time_submit,
            EventTimes::Start => lock.time_start,
            EventTimes::End => lock.time_end,
        }
    }

    pub fn add_cb(&self, state: cl_int, cb: EventCB) {
        let mut lock = self.state();
        let status = lock.status;

        // call cb if the status was already reached
        if state >= status {
            drop(lock);
            cb.call(self, state);
        } else {
            lock.cbs.get_mut(state as usize).unwrap().push(cb);
        }
    }

    pub(super) fn signal(&self) {
        let mut lock = self.state();

        self.set_status(&mut lock, CL_RUNNING as cl_int);
        self.set_status(&mut lock, CL_COMPLETE as cl_int);
    }

    pub fn wait(&self) -> cl_int {
        let mut lock = self.state();
        while lock.status >= CL_RUNNING as cl_int {
            lock = self
                .cv
                .wait_timeout(lock, Duration::from_secs(1))
                .unwrap()
                .0;
        }
        lock.status
    }

    // We always assume that work here simply submits stuff to the hardware even if it's just doing
    // sw emulation or nothing at all.
    // If anything requets waiting, we will update the status through fencing later.
    pub fn call(&self, ctx: &QueueContext) {
        let mut lock = self.state();
        let status = lock.status;
        let queue = self.queue.as_ref().unwrap();
        let profiling_enabled = queue.is_profiling_enabled();
        if status == CL_QUEUED as cl_int {
            if profiling_enabled {
                // We already have the lock so can't call set_time on the event
                lock.time_submit = queue.device.screen().get_timestamp();
            }
            let mut query_start = None;
            let mut query_end = None;
            let new = lock.work.take().map_or(
                // if there is no work
                CL_SUBMITTED as cl_int,
                |w| {
                    if profiling_enabled {
                        query_start =
                            PipeQueryGen::<{ pipe_query_type::PIPE_QUERY_TIMESTAMP }>::new(ctx);
                    }

                    let res = w(queue, ctx).err().map_or(
                        // if there is an error, negate it
                        CL_SUBMITTED as cl_int,
                        |e| e,
                    );
                    if profiling_enabled {
                        query_end =
                            PipeQueryGen::<{ pipe_query_type::PIPE_QUERY_TIMESTAMP }>::new(ctx);
                    }
                    res
                },
            );

            if profiling_enabled {
                lock.time_start = query_start.unwrap().read_blocked();
                lock.time_end = query_end.unwrap().read_blocked();
            }
            self.set_status(&mut lock, new);
        }
    }

    fn deep_unflushed_deps_impl<'a>(&'a self, result: &mut HashSet<&'a Event>) {
        if self.status() <= CL_SUBMITTED as i32 {
            return;
        }

        // only scan dependencies if it's a new one
        if result.insert(self) {
            for e in &self.deps {
                e.deep_unflushed_deps_impl(result);
            }
        }
    }

    /// does a deep search and returns a list of all dependencies including `events` which haven't
    /// been flushed out yet
    pub fn deep_unflushed_deps(events: &[Arc<Event>]) -> HashSet<&Event> {
        let mut result = HashSet::new();

        for e in events {
            e.deep_unflushed_deps_impl(&mut result);
        }

        result
    }

    /// does a deep search and returns a list of all queues which haven't been flushed yet
    pub fn deep_unflushed_queues(events: &[Arc<Event>]) -> HashSet<Arc<Queue>> {
        Event::deep_unflushed_deps(events)
            .iter()
            .filter_map(|e| e.queue.clone())
            .collect()
    }
}

// TODO worker thread per device
// Condvar to wait on new events to work on
// notify condvar when flushing queue events to worker
// attach fence to flushed events on context->flush
// store "newest" event for in-order queues per queue
// reordering/graph building done in worker
