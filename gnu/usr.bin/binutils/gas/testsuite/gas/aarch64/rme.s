/* Realm Management Extension.  */

/* Read from RME system registers.  */
mrs x0, mfar_el3
mrs x0, gpccr_el3
mrs x0, gptbr_el3

/* Write to RME system registers.  */
msr mfar_el3, x0
msr gpccr_el3, x0
msr gptbr_el3, x0

/* RME data cache maintenance operations.   */
dc cipapa, x0
dc cigdpapa, x0

/* RME instructions for maintenance of GPT entries cached in a TLB.  */
tlbi rpaos, x0
tlbi rpalos, x0
tlbi paallos
tlbi paall
