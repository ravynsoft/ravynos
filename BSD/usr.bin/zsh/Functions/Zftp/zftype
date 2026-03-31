# function zftype {
local type
[[ $curcontext = :zf* ]] || local curcontext=:zftype

zfautocheck -d

if (( $# == 0 )); then
  type=$(zftp type)
  if [[ $type = I ]]; then
    print "Current type is image (binary)"
    return 0
  elif [[ $type = A ]]; then
    print "Current type is ASCII"
    return 0
  else
    return 1
  fi
else
  if [[ $1 == (#i)a(sc(ii|)|) ]]; then
    type=A
  elif [[ $1 == (#i)i(m(age|)|) || $1 == (#i)b(in(ary|)|) ]]; then
    type=I
  else
    print "Type not recognised:  $1" 2>&1
    return 1
  fi
  zftp type $type
fi
# }
