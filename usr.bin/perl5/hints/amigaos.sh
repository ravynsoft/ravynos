#
# AmigaOS 4
#
. amigaos4/config.sh
for f in amigaos4/*.h amigaos4/*.c
do
  cp -f $f .
done

ccflags="$ccflags -fno-stack-protector"
