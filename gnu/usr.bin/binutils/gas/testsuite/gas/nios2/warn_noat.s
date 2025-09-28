.set noat, 2 # This should not cause warning for at to be turned off
add at, r2, r2
.set noat  # this should turn the warnings off
add at, r2, r2
.set at, 3     # this should not turn the warnings on
add at, r2, r2
.set at      # this should turn the warnings on
add at, r2, r2
