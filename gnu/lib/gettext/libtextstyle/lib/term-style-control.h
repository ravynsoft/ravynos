/* Terminal control for outputting styled text to a terminal.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _TERM_STYLE_CONTROL_H
#define _TERM_STYLE_CONTROL_H

/* The user of this file will define a macro 'term_style_user_data', such that
   'struct term_style_user_data' is a user-defined struct.  */

/* This file uses _GL_ASYNC_SAFE, HAVE_TCGETATTR.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif


/* The amount of control to take over the underlying tty in order to avoid
   garbled output on the screen, due to interleaved output of escape sequences
   and output from the kernel (such as when the kernel echoes user's input
   or when the kernel prints '^C' after the user pressed Ctrl-C).  */
typedef enum
{
  TTYCTL_AUTO = 0,  /* Automatic best-possible choice.  */
  TTYCTL_NONE,      /* No control.
                       Result: Garbled output can occur, and the terminal can
                       be left in any state when the program is interrupted.  */
  TTYCTL_PARTIAL,   /* Signal handling.
                       Result: Garbled output can occur, but the terminal will
                       be left in the default state when the program is
                       interrupted.  */
  TTYCTL_FULL       /* Signal handling and disabling echo and flush-upon-signal.
                       Result: No garbled output, and the terminal will
                       be left in the default state when the program is
                       interrupted.  */
} ttyctl_t;

/* This struct contains data, used by implementation of this module.
   You should not access the members of this struct; they may be renamed or
   removed without notice.  */
struct term_style_control_data
{
  int volatile fd;
  ttyctl_t volatile tty_control;     /* Signal handling and tty control.  */
  #if HAVE_TCGETATTR
  bool volatile same_as_stderr;
  #endif
  bool non_default_active;           /* True if activate_term_non_default_mode()
                                        is in effect.  */
};

/* Forward declaration.  */
struct term_style_user_data;

/* This struct contains function pointers.  You implement these functions
   in your application; this module invokes them when it needs to.  */
struct term_style_controller
{
  /* This function returns a pointer to the embedded
     'struct term_style_control_data' contained in a
     'struct term_style_user_data'.  */
  struct term_style_control_data * (*get_control_data) (struct term_style_user_data *);

  /* This function brings the terminal's state back to the default state
     (no styling attributes set).  It is invoked when the process terminates
     through exit().  */
  void (*restore) (struct term_style_user_data *);

  /* This function brings the terminal's state back to the default state
     (no styling attributes set).  It is async-safe (see gnulib-common.m4 for
     the precise definition).  It is invoked when the process receives a fatal
     or stopping signal.  */
  _GL_ASYNC_SAFE void (*async_restore) (struct term_style_user_data *);

  /* This function brings the terminal's state, from the default state, back
     to the state where it has the desired attributes set.  It is async-safe
     (see gnulib-common.m4 for the precise definition).  It is invoked when
     the process receives a SIGCONT signal.  */
  _GL_ASYNC_SAFE void (*async_set_attributes_from_default) (struct term_style_user_data *);
};


#ifdef __cplusplus
extern "C" {
#endif

/* This module is used as follows:
   1. You fill a 'struct term_style_controller' with function pointers.
      You create a 'struct term_style_user_data' that contains, among other
      members, a 'struct term_style_control_data'.
      You will pass these two objects to all API functions below.
   2. You call activate_term_style_controller to activate this controller.
      Activation of the controller is the prerequisite for activating
      the non-default mode, which in turn is the prerequisite for changing
      the terminal's attributes.
      When you are done with the styled output, you may deactivate the
      controller.  This is not required before exiting the program, but is
      required before activating a different controller.
      You cannot have more than one controller activated at the same time.
   3. Once the controller is activated, you may turn on the non-default mode.
      The non-default mode is the prerequisite for changing the terminal's
      attributes.  Once the terminal's attributes are in the default state
      again, you may turn off the non-default mode again.
      In other words:
        - In the default mode, the terminal's attributes MUST be in the default
          state; no styled output is possible.
        - In the non-default mode, the terminal's attributes MAY switch among
          the default state and other states.
      This module exercises a certain amount of control over the terminal
      during the non-default mode phases; see above (ttyctl_t) for details.
      You may switch between the default and the non-default modes any number
      of times.
      The idea is that you switch back to the default mode before doing large
      amounts of output of unstyled text.  However, this is not a requirement:
      You may leave the non-default mode turned on all the time until the
      the program exits.
   4. Once the non-default mode is activated, you may change the attributes
      (foreground color, background color, font weight, font posture, underline
      decoration, etc.) of the terminal.  On Unix, this is typically done by
      outputting appropriate escape sequences.
   5. Once attributes are set, text output to the terminal will be rendered
      with these attributes.
      Note: You MUST return the terminal to the default state before outputting
      a newline.
 */

/* Activates a controller.  The CONTROLLER and its USER_DATA controls the
   terminal associated with FD.  FD is usually STDOUT_FILENO.
   TTY_CONTROL specifies the amount of control to take over the underlying tty.
   The effects of this functions are undone by calling
   deactivate_term_style_controller.
   You cannot have more than one controller activated at the same time.
   You must not close FD while the controller is active.  */
extern void
       activate_term_style_controller (const struct term_style_controller *controller,
                                       struct term_style_user_data *user_data,
                                       int fd, ttyctl_t tty_control);

/* Activates the non-default mode.
   CONTROLLER and its USER_DATA must be a currently active controller.
   This function fiddles with the signals of the current process and with
   the underlying tty, to an extent described by TTY_CONTROL.
   This function is idempotent: When you call it twice in a row, the second
   invocation does nothing.
   The effects of this function are undone by calling
   deactivate_term_non_default_mode.  */
extern void
       activate_term_non_default_mode (const struct term_style_controller *controller,
                                       struct term_style_user_data *user_data);

/* Deactivates the non-default mode.
   CONTROLLER and its USER_DATA must be a currently active controller.
   This function is idempotent: When you call it twice in a row, the second
   invocation does nothing.
   Before invoking this function, you must put the terminal's attributes in
   the default state.  */
extern void
       deactivate_term_non_default_mode (const struct term_style_controller *controller,
                                         struct term_style_user_data *user_data);

/* Deactivates a controller.
   CONTROLLER and its USER_DATA must be a currently active controller.
   Before invoking this function, you must ensure that the non-default mode
   is deactivated.  */
extern void
       deactivate_term_style_controller (const struct term_style_controller *controller,
                                         struct term_style_user_data *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TERM_STYLE_CONTROL_H */
