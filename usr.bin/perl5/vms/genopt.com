$! generates options file for vms link
$! p1 is filename and mode to open file (filename/write or filename/append)
$! p2 is delimiter separating elements of list in p3
$! p3 is list of items to be written, one per line, into options file
$!
$ open file 'p1'
$ element=0
$loop:
$ x=f$element(element,p2,p3)
$ if x .eqs. p2 then goto out
$ y=f$edit(x,"COLLAPSE")  ! lose spaces
$! Expand potential name-only args so we find shareable images
$! either via a logical name or in the default location
$ if y .nes. "" .and. -
     f$locate("/SHARE",f$edit(y,"UPCASE")) .ne. f$length(y)
$ then
$   name = f$element(0,"/",y)
$   tail = f$extract(f$length(name),1024,y)
$   if f$trnlnm(name) .eqs. ""  ! If it's a logical name, assume it's OK as is
$   then
$     name = f$parse(name,"sys$share:.exe;")   ! Look where image activator will
$     name = f$search(name)                    ! Does it really exist?
$     if name .nes. ""
$     then
$       name = name - f$parse(name,,,"version")  ! Insist on current version
$       y = name + tail
$     endif
$  endif
$ endif
$ if y .nes. "" then write file y
$ element=element+1
$ goto loop
$!
$out:
$ close file
$ exit
