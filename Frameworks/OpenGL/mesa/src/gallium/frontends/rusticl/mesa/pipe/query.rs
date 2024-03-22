use crate::pipe::context::*;

use mesa_rust_gen::*;

use std::marker::*;

// Callers create new queries using PipeQueryGen<QueryType>::new(...)
pub struct PipeQueryGen<const Q: pipe_query_type::Type> {}

// We record the type that the given Query type will return into
// a trait we associate with a PipeQuery object we return
// QueryResultTrait is the type we'd like our PipeQueryGen to return
// for a given query
pub trait QueryResultTrait {
    type ResType;
}

// Define this set of PipeQueryGen's for these queries
impl QueryResultTrait for PipeQueryGen<{ pipe_query_type::PIPE_QUERY_TIMESTAMP }> {
    type ResType = u64;
}

impl<const Q: pipe_query_type::Type> PipeQueryGen<Q>
where
    PipeQueryGen<Q>: QueryResultTrait,
{
    // The external interface to create a new query
    pub fn new(ctx: &PipeContext) -> Option<PipeQuery<<Self as QueryResultTrait>::ResType>> {
        PipeQuery::<<Self as QueryResultTrait>::ResType>::new(ctx, Q)
    }
}

// Our higher level view of a 'pipe_query', created by a call to the
// 'create_query' method on the pipe context
pub struct PipeQuery<'a, R> {
    query: *mut pipe_query,
    ctx: &'a PipeContext,

    _result_marker: PhantomData<R>,
}

impl<'a, R> PipeQuery<'a, R> {
    fn new(ctx: &'a PipeContext, query_type: u32) -> Option<Self> {
        let pq = ctx.create_query(query_type, 0);
        if pq.is_null() {
            return None;
        }
        // SAFETY: we are the only owner of that valid pointer
        unsafe {
            if !ctx.end_query(pq) {
                ctx.destroy_query(pq);
                return None;
            }
        }
        Some(Self {
            query: pq,
            ctx: ctx,
            _result_marker: Default::default(),
        })
    }
}

impl<'a, R> Drop for PipeQuery<'a, R> {
    fn drop(&mut self) {
        // SAFETY: we are the only owner of that valid pointer
        unsafe {
            self.ctx.destroy_query(self.query);
        }
    }
}

pub trait QueryReadTrait {
    type ResType;
    fn read(&mut self, wait: bool) -> Option<Self::ResType>;

    fn read_blocked(&mut self) -> Self::ResType {
        self.read(true).unwrap()
    }
}

impl QueryReadTrait for PipeQuery<'_, u64> {
    type ResType = u64;

    fn read(&mut self, wait: bool) -> Option<u64> {
        let mut raw_result = pipe_query_result::default();
        // SAFETY: we guarentee unique access through our `&mut self` reference above.
        if unsafe { self.ctx.get_query_result(self.query, wait, &mut raw_result) } {
            // SAFETY: We know this is the right type
            // because of the trait bound on PipeQueryGen binds the
            // query type with the result type.
            Some(unsafe { raw_result.u64_ })
        } else {
            None
        }
    }
}
