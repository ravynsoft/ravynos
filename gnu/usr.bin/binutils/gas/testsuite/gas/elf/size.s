.text

.size foo1, 1
foo1:

.set bar1, foo1
.size bar1, 2
.size bar2, 2
.set bar2, foo1

.set bar3, foo2
.size bar3, 2
.size bar4, 2
.set bar4, foo2

.set bar5, foo1
.set bar6, foo2

.size foo2, 3
foo2:

.set bar7, foo1
.set bar7, foo2
