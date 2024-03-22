
#ifndef I915_PUBLIC_H
#define I915_PUBLIC_H

struct i915_winsys;
struct pipe_screen;

/**
 * Create i915 pipe_screen.
 */
struct pipe_screen *i915_screen_create(struct i915_winsys *iws);

#endif
