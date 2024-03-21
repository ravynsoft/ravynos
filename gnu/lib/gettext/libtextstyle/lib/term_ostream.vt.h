/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Virtual function table layout of superclass.  */
#include "ostream.vt.h"

/* Virtual function table layout of term_ostream class.  */
        term_color_t (*rgb_to_color) (THIS_ARG,                              int red, int green, int blue);
        term_color_t (*get_color) (THIS_ARG);
   void         (*set_color) (THIS_ARG, term_color_t color);
        term_color_t (*get_bgcolor) (THIS_ARG);
   void         (*set_bgcolor) (THIS_ARG, term_color_t color);
        term_weight_t (*get_weight) (THIS_ARG);
   void          (*set_weight) (THIS_ARG, term_weight_t weight);
        term_posture_t (*get_posture) (THIS_ARG);
   void           (*set_posture) (THIS_ARG, term_posture_t posture);
        term_underline_t (*get_underline) (THIS_ARG);
   void             (*set_underline) (THIS_ARG,                                   term_underline_t underline);
        const char * (*get_hyperlink_ref) (THIS_ARG);
   const char * (*get_hyperlink_id) (THIS_ARG);
   void         (*set_hyperlink) (THIS_ARG,                               const char *ref, const char *id);
              void (*flush_to_current_style) (THIS_ARG);
        int          (*get_descriptor) (THIS_ARG);
   const char * (*get_filename) (THIS_ARG);
   ttyctl_t     (*get_tty_control) (THIS_ARG);
   ttyctl_t     (*get_effective_tty_control) (THIS_ARG);
