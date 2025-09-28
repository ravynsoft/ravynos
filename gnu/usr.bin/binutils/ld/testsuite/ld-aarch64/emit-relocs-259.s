.global dummy
.text
dummy:
  ldr x0, .L1

.L1:
  .hword dummy
