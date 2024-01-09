# Bind to a binary operator keystroke for use with zcalc
# Not useful in RPN mode.

if [[ -n $ZCALC_ACTIVE && $ZCALC_ACTIVE != rpn ]]; then
  if [[ $CURSOR -eq 0 || $LBUFFER[-1] = "(" ]]; then
    LBUFFER+=${ZCALC_AUTO_INSERT_PREFIX:-"ans "}
  fi
fi
zle .self-insert
