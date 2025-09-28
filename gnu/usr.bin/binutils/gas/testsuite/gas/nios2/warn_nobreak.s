.set nobreak , 2 # This should not cause warning for ba, bt to be turned off
add ba, r2, r2
add bt, r2, r2
.set nobreak     # this should turn the warnings off
add ba, r3, r4
add bt, r3, r4
.set break, 3     # this should not turn the warnings on
add ba, r3, r4
add bt, r3, r4
.set break      # this should turn the warnings on
add ba, r3, r4
add bt, r3, r4
