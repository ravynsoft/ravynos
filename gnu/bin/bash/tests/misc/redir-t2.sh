read line1

echo read line 1 \"$line1\"

exec 4<&0

exec 0</dev/tty

read line2

echo line read from tty = \"$line2\"

exec 0<&4

read line3

echo read line 3 \"$line3\"
