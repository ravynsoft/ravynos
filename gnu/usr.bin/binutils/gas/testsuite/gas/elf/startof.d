#name: .startof. / .sizeof.
#readelf: -s

Symbol table .*
   Num: .*
     0: 0+ .*
#...
     [1-8]: 0+ .* UND \.startof\.\.text
     [2-9]: 0+ .* UND \.sizeof\.\.text
 +[1-9][0-9]*: 0+ .* UND \.startof\.\.Text
 +[1-9][0-9]*: 0+ .* UND \.sizeof\.\.TEXT
#pass
