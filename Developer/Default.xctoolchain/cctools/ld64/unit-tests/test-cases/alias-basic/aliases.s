
  .globl _main
_main = _mymain

  .globl _bar
_bar = _mybar

  .globl _barAlt
_barAlt = _mybar

  .private_extern _barHidden
_barHidden = _mybar

  .globl _barExtra
_barExtra = _barAlt

  .globl _result
_result = _myresult

  .globl _resultHidden
_resultHidden = _myresult

#if UNUSED_ALIAS
  .globl _unusedAlias
_unusedAlias = _unusedUndefined
#endif
