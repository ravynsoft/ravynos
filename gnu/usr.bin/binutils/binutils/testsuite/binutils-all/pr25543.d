#source: pr25543.s
#readelf: -p.data
#notarget: rx-*

String dump of section '.data':
  \[     0\]  line1 : This is a line without a newline at the end
  \[    34\]  line2 : This is a line with a newline at the end\\n
  \[    66\]  line3 : This is a line with a \\n
            newline in the middle
  \[    9b\]  line4 : This is a line with a \^Mcontrol character
  \[    cd\]  line6 : The previous line was empty\\n
#pass
