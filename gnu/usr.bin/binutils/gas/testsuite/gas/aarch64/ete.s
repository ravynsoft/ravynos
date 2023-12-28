/* ETE System registers.  */

/* Read from system register.  */
mrs x0, trcextinselr0
mrs x0, trcextinselr1
mrs x0, trcextinselr2
mrs x0, trcextinselr3
mrs x0, trcrsr

/* Write to system register.  */
msr trcextinselr0, x0
msr trcextinselr1, x0
msr trcextinselr2, x0
msr trcextinselr3, x0
msr trcrsr, x0
