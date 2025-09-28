.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vpnot

.endr

it eq
vpnoteq
vpnoteq
vpst
vpnoteq
vpnott
vpst
vpnot
