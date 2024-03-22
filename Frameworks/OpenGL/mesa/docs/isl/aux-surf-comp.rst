Auxiliary surface compression
=============================

Most lossless image compression on Intel hardware, be that CCS, MCS, or HiZ,
works by way of some chunk of auxiliary data (often a surface) which is used
together with the main surface to provide compression.  Even though this means
more memory is allocated, the scheme allows us to reduce our over-all memory
bandwidth since the auxiliary data is much smaller than the main surface.

The simplest example of this is single-sample fast clears
(:c:enumerator:`isl_aux_usage.ISL_AUX_USAGE_CCS_D`) on Ivy Bridge through
Broadwell and later.  For this scheme, the auxiliary surface stores a single
bit for each cache-line-pair in the main surface.  If that bit is set, then the
entire cache line pair contains only the clear color as provided in the
``RENDER_SURFACE_STATE`` for the image.  If the bit is unset, then it's not
clear and you should look at the main surface.  Since a cache line is 64B, this
yields a scale-down factor of 1:1024.

Even the simple fast-clear scheme saves us bandwidth in two places.  The first
is when we go to clear the surface.  If we're doing a full-surface clear or
clearing to the same color that was used to clear before, we don't have to
touch the main surface at all.  All we have to do is record the clear color and
smash the aux data to ``0xff``.  The hardware then knows to ignore whatever is
in the main surface and look at the clear color instead.  The second is when we
go to render.  Say we're doing some color blending.  Instead of the blend unit
having to read back actual surface contents to blend with, it looks at the
clear bit and blends with the clear color recorded with the surface state
instead.  Depending on the geometry and cache utilization, this can save as
much as one whole read of the surface worth of bandwidth.

The difficulty with a scheme like this comes when we want to do something else
with that surface.  What happens if the sampler doesn't support this fast-clear
scheme (it doesn't on IVB)?  In that case, we have to do a *resolve* where we
run a special pipeline that reads the auxiliary data and applies it to the main
surface.  In the case of fast clears, this means that, for every 1 bit in the
auxiliary surface, the corresponding pair of cache lines in the main surface
gets filled with the clear color.  At the end of the resolve operation, the
main surface contents are the actual contents of the surface.

Types of surface compression
----------------------------

Intel hardware has several different compression schemes that all work along
similar lines:

.. c:autoenum:: isl_aux_usage
   :file: src/intel/isl/isl.h
   :members:

.. c:autofunction:: isl_aux_usage_has_fast_clears

.. c:autofunction:: isl_aux_usage_has_compression

.. c:autofunction:: isl_aux_usage_has_hiz

.. c:autofunction:: isl_aux_usage_has_mcs

.. c:autofunction:: isl_aux_usage_has_ccs

Creating auxiliary surfaces
---------------------------

Each type of data compression requires some type of auxiliary data on the side.
For most, this involves a second auxiliary surface.  ISL provides helpers for
creating each of these types of surfaces:

.. c:autofunction:: isl_surf_get_hiz_surf

.. c:autofunction:: isl_surf_get_mcs_surf

.. c:autofunction:: isl_surf_supports_ccs

.. c:autofunction:: isl_surf_get_ccs_surf

Compression state tracking
--------------------------

All of the Intel auxiliary surface compression schemes share a common concept
of a main surface which may or may not contain correct up-to-date data and some
auxiliary data which says how to interpret it.  The main surface is divided
into blocks of some fixed size and some smaller block in the auxiliary data
controls how that main surface block is to be interpreted.  We then have to do
resolves depending on the different HW units which need to interact with a
given surface.

To help drivers keep track of what all is going on and when resolves need to be
inserted, ISL provides a finite state machine which tracks the current state of
the main surface and auxiliary data and their relationship to each other.  The
states are encoded with the :c:enum:`isl_aux_state` enum.  ISL also provides
helper functions for operating the state machine and determining what aux op
(if any) is required to get to the right state for a given operation.

.. c:autoenum:: isl_aux_state

.. c:autofunction:: isl_aux_state_has_valid_primary

.. c:autofunction:: isl_aux_state_has_valid_aux

.. c:autoenum:: isl_aux_op

.. c:autofunction:: isl_aux_prepare_access

.. c:autofunction:: isl_aux_state_transition_aux_op

.. c:autofunction:: isl_aux_state_transition_write
