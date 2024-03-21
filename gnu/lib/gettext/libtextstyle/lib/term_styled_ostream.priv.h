/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "styled_ostream.priv.h"

/* Field layout of term_styled_ostream class.  */
struct term_styled_ostream_representation
{
  struct styled_ostream_representation base;
   
  term_ostream_t destination;
   
  char *css_filename;
   
  CRCascade *css_document;
   
  CRSelEng *css_engine;
   
  char *curr_classes;
  size_t curr_classes_length;
  size_t curr_classes_allocated;
   
  hash_table cache;
   
  attributes_t *curr_attr;
};
