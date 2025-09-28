#as: -mevexrcig=rd
#objdump: -dw -Mintel
#name: x86_64 AVX512F rcig insns (Intel disassembly)
#source: x86-64-avx512f-rcig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 10[ 	]*vcmpeq_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 08[ 	]*vcmpeq_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 18[ 	]*vcmpeq_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1b[ 	]*vcmpfalse_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1d[ 	]*vcmpge_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1e[ 	]*vcmpgt_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 12[ 	]*vcmple_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 11[ 	]*vcmplt_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0c[ 	]*vcmpneq_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1c[ 	]*vcmpneq_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 14[ 	]*vcmpneq_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 19[ 	]*vcmpnge_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1a[ 	]*vcmpngt_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 16[ 	]*vcmpnle_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 15[ 	]*vcmpnlt_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 17[ 	]*vcmpord_spd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1f[ 	]*vcmptrue_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 13[ 	]*vcmpunord_spd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed ab[ 	]*vcmppd k5,zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 7b[ 	]*vcmppd k5,zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 10[ 	]*vcmpeq_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 08[ 	]*vcmpeq_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 18[ 	]*vcmpeq_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1b[ 	]*vcmpfalse_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1d[ 	]*vcmpge_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1e[ 	]*vcmpgt_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 12[ 	]*vcmple_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 11[ 	]*vcmplt_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0c[ 	]*vcmpneq_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1c[ 	]*vcmpneq_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 14[ 	]*vcmpneq_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 19[ 	]*vcmpnge_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1a[ 	]*vcmpngt_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 16[ 	]*vcmpnle_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 15[ 	]*vcmpnlt_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 17[ 	]*vcmpord_sps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1f[ 	]*vcmptrue_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 13[ 	]*vcmpunord_sps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed ab[ 	]*vcmpps k5,zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 7b[ 	]*vcmpps k5,zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 10[ 	]*vcmpeq_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 08[ 	]*vcmpeq_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 18[ 	]*vcmpeq_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1b[ 	]*vcmpfalse_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1d[ 	]*vcmpge_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1e[ 	]*vcmpgt_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 12[ 	]*vcmple_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 11[ 	]*vcmplt_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0c[ 	]*vcmpneq_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1c[ 	]*vcmpneq_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 14[ 	]*vcmpneq_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 19[ 	]*vcmpnge_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1a[ 	]*vcmpngt_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 16[ 	]*vcmpnle_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 15[ 	]*vcmpnlt_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 17[ 	]*vcmpord_ssd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1f[ 	]*vcmptrue_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 13[ 	]*vcmpunord_ssd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec ab[ 	]*vcmpsd k5,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 7b[ 	]*vcmpsd k5,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 10[ 	]*vcmpeq_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 08[ 	]*vcmpeq_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 18[ 	]*vcmpeq_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1b[ 	]*vcmpfalse_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1d[ 	]*vcmpge_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1e[ 	]*vcmpgt_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 12[ 	]*vcmple_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 11[ 	]*vcmplt_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0c[ 	]*vcmpneq_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1c[ 	]*vcmpneq_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 14[ 	]*vcmpneq_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 19[ 	]*vcmpnge_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1a[ 	]*vcmpngt_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 16[ 	]*vcmpnle_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 15[ 	]*vcmpnlt_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 17[ 	]*vcmpord_sss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1f[ 	]*vcmptrue_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 13[ 	]*vcmpunord_sss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec ab[ 	]*vcmpss k5,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 7b[ 	]*vcmpss k5,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2f f5[ 	]*vcomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2f f5[ 	]*vcomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 13 f5[ 	]*vcvtph2ps zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 5a f5[ 	]*vcvtps2pd zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee ab[ 	]*vcvtps2ph ymm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee 7b[ 	]*vcvtps2ph ymm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5a f4[ 	]*vcvtss2sd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 e6 f5[ 	]*vcvttpd2dq ymm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 38 5b f5[ 	]*vcvttps2dq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c c6[ 	]*vcvttsd2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c ee[ 	]*vcvttsd2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 2c ee[ 	]*vcvttsd2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 2c c6[ 	]*vcvttsd2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 2c c6[ 	]*vcvttsd2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c c6[ 	]*vcvttss2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c ee[ 	]*vcvttss2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 2c ee[ 	]*vcvttss2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 2c c6[ 	]*vcvttss2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 2c c6[ 	]*vcvttss2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 38 42 f5[ 	]*vgetexppd zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 42 f5[ 	]*vgetexpps zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 30 43 f4[ 	]*vgetexpsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 30 43 f4[ 	]*vgetexpss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 ab[ 	]*vgetmantpd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 7b[ 	]*vgetmantpd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 ab[ 	]*vgetmantps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 7b[ 	]*vgetmantps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 ab[ 	]*vgetmantsd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 7b[ 	]*vgetmantsd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 ab[ 	]*vgetmantss xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 7b[ 	]*vgetmantss xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5f f4[ 	]*vmaxpd zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5f f4[ 	]*vmaxps zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5f f4[ 	]*vmaxsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5f f4[ 	]*vmaxss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5d f4[ 	]*vminpd zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5d f4[ 	]*vminps zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5d f4[ 	]*vminsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5d f4[ 	]*vminss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2e f5[ 	]*vucomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2e f5[ 	]*vucomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 ab[ 	]*vfixupimmpd zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 7b[ 	]*vfixupimmpd zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 ab[ 	]*vfixupimmps zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 7b[ 	]*vfixupimmps zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 ab[ 	]*vfixupimmsd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 7b[ 	]*vfixupimmsd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 ab[ 	]*vfixupimmss xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 7b[ 	]*vfixupimmss xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 ab[ 	]*vrndscalepd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 7b[ 	]*vrndscalepd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 ab[ 	]*vrndscaleps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 7b[ 	]*vrndscaleps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 ab[ 	]*vrndscalesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 7b[ 	]*vrndscalesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 ab[ 	]*vrndscaless xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 7b[ 	]*vrndscaless xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 78 f5[ 	]*vcvttpd2udq ymm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 78 f5[ 	]*vcvttps2udq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 c6[ 	]*vcvttsd2usi eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 ee[ 	]*vcvttsd2usi ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 78 ee[ 	]*vcvttsd2usi r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 78 c6[ 	]*vcvttsd2usi rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 78 c6[ 	]*vcvttsd2usi r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 c6[ 	]*vcvttss2usi eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 ee[ 	]*vcvttss2usi ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 78 ee[ 	]*vcvttss2usi r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 78 c6[ 	]*vcvttss2usi rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 78 c6[ 	]*vcvttss2usi r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 10[ 	]*vcmpeq_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 08[ 	]*vcmpeq_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 18[ 	]*vcmpeq_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1b[ 	]*vcmpfalse_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1d[ 	]*vcmpge_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1e[ 	]*vcmpgt_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 12[ 	]*vcmple_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 11[ 	]*vcmplt_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0c[ 	]*vcmpneq_oqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1c[ 	]*vcmpneq_ospd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 14[ 	]*vcmpneq_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 19[ 	]*vcmpnge_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1a[ 	]*vcmpngt_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 16[ 	]*vcmpnle_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 15[ 	]*vcmpnlt_uqpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 17[ 	]*vcmpord_spd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1f[ 	]*vcmptrue_uspd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 13[ 	]*vcmpunord_spd k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed ab[ 	]*vcmppd k5,zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 7b[ 	]*vcmppd k5,zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 10[ 	]*vcmpeq_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 08[ 	]*vcmpeq_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 18[ 	]*vcmpeq_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1b[ 	]*vcmpfalse_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1d[ 	]*vcmpge_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1e[ 	]*vcmpgt_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 12[ 	]*vcmple_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 11[ 	]*vcmplt_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0c[ 	]*vcmpneq_oqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1c[ 	]*vcmpneq_osps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 14[ 	]*vcmpneq_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 19[ 	]*vcmpnge_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1a[ 	]*vcmpngt_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 16[ 	]*vcmpnle_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 15[ 	]*vcmpnlt_uqps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 17[ 	]*vcmpord_sps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1f[ 	]*vcmptrue_usps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 13[ 	]*vcmpunord_sps k5,zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed ab[ 	]*vcmpps k5,zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 7b[ 	]*vcmpps k5,zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 10[ 	]*vcmpeq_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 08[ 	]*vcmpeq_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 18[ 	]*vcmpeq_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1b[ 	]*vcmpfalse_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1d[ 	]*vcmpge_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1e[ 	]*vcmpgt_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 12[ 	]*vcmple_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 11[ 	]*vcmplt_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0c[ 	]*vcmpneq_oqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1c[ 	]*vcmpneq_ossd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 14[ 	]*vcmpneq_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 19[ 	]*vcmpnge_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1a[ 	]*vcmpngt_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 16[ 	]*vcmpnle_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 15[ 	]*vcmpnlt_uqsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 17[ 	]*vcmpord_ssd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1f[ 	]*vcmptrue_ussd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 13[ 	]*vcmpunord_ssd k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec ab[ 	]*vcmpsd k5,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 7b[ 	]*vcmpsd k5,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 10[ 	]*vcmpeq_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 08[ 	]*vcmpeq_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 18[ 	]*vcmpeq_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1b[ 	]*vcmpfalse_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1d[ 	]*vcmpge_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1e[ 	]*vcmpgt_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 12[ 	]*vcmple_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 11[ 	]*vcmplt_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0c[ 	]*vcmpneq_oqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1c[ 	]*vcmpneq_osss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 14[ 	]*vcmpneq_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 19[ 	]*vcmpnge_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1a[ 	]*vcmpngt_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 16[ 	]*vcmpnle_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 15[ 	]*vcmpnlt_uqss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 17[ 	]*vcmpord_sss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1f[ 	]*vcmptrue_usss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 13[ 	]*vcmpunord_sss k5,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec ab[ 	]*vcmpss k5,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 7b[ 	]*vcmpss k5,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2f f5[ 	]*vcomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2f f5[ 	]*vcomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 13 f5[ 	]*vcvtph2ps zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 5a f5[ 	]*vcvtps2pd zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee ab[ 	]*vcvtps2ph ymm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee 7b[ 	]*vcvtps2ph ymm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5a f4[ 	]*vcvtss2sd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 e6 f5[ 	]*vcvttpd2dq ymm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 38 5b f5[ 	]*vcvttps2dq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c c6[ 	]*vcvttsd2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c ee[ 	]*vcvttsd2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 2c ee[ 	]*vcvttsd2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 2c c6[ 	]*vcvttsd2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 2c c6[ 	]*vcvttsd2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c c6[ 	]*vcvttss2si eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c ee[ 	]*vcvttss2si ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 2c ee[ 	]*vcvttss2si r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 2c c6[ 	]*vcvttss2si rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 2c c6[ 	]*vcvttss2si r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 38 42 f5[ 	]*vgetexppd zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 42 f5[ 	]*vgetexpps zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 30 43 f4[ 	]*vgetexpsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 30 43 f4[ 	]*vgetexpss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 ab[ 	]*vgetmantpd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 7b[ 	]*vgetmantpd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 ab[ 	]*vgetmantps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 7b[ 	]*vgetmantps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 ab[ 	]*vgetmantsd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 7b[ 	]*vgetmantsd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 ab[ 	]*vgetmantss xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 7b[ 	]*vgetmantss xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5f f4[ 	]*vmaxpd zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5f f4[ 	]*vmaxps zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5f f4[ 	]*vmaxsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5f f4[ 	]*vmaxss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5d f4[ 	]*vminpd zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5d f4[ 	]*vminps zmm30,zmm29,zmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5d f4[ 	]*vminsd xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5d f4[ 	]*vminss xmm30,xmm29,xmm28\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2e f5[ 	]*vucomisd xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2e f5[ 	]*vucomiss xmm30,xmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 ab[ 	]*vfixupimmpd zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 7b[ 	]*vfixupimmpd zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 ab[ 	]*vfixupimmps zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 7b[ 	]*vfixupimmps zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 ab[ 	]*vfixupimmsd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 7b[ 	]*vfixupimmsd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 ab[ 	]*vfixupimmss xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 7b[ 	]*vfixupimmss xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 ab[ 	]*vrndscalepd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 7b[ 	]*vrndscalepd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 ab[ 	]*vrndscaleps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 7b[ 	]*vrndscaleps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 ab[ 	]*vrndscalesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 7b[ 	]*vrndscalesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 ab[ 	]*vrndscaless xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 7b[ 	]*vrndscaless xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 78 f5[ 	]*vcvttpd2udq ymm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 78 f5[ 	]*vcvttps2udq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 c6[ 	]*vcvttsd2usi eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 ee[ 	]*vcvttsd2usi ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 78 ee[ 	]*vcvttsd2usi r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 78 c6[ 	]*vcvttsd2usi rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 78 c6[ 	]*vcvttsd2usi r8,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 c6[ 	]*vcvttss2usi eax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 ee[ 	]*vcvttss2usi ebp,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 78 ee[ 	]*vcvttss2usi r13d,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 78 c6[ 	]*vcvttss2usi rax,xmm30\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 78 c6[ 	]*vcvttss2usi r8,xmm30\{sae\}
#pass
