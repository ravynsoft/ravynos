#as: -mevexrcig=rd
#objdump: -dw
#name: x86_64 AVX512F rcig insns
#source: x86-64-avx512f-rcig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 10[ 	]*vcmpeq_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 08[ 	]*vcmpeq_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 18[ 	]*vcmpeq_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1b[ 	]*vcmpfalse_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1d[ 	]*vcmpge_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1e[ 	]*vcmpgt_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 12[ 	]*vcmple_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 11[ 	]*vcmplt_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0c[ 	]*vcmpneq_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1c[ 	]*vcmpneq_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 14[ 	]*vcmpneq_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 19[ 	]*vcmpnge_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1a[ 	]*vcmpngt_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 16[ 	]*vcmpnle_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 15[ 	]*vcmpnlt_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 17[ 	]*vcmpord_spd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1f[ 	]*vcmptrue_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 13[ 	]*vcmpunord_spd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed ab[ 	]*vcmppd \$0xab,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 7b[ 	]*vcmppd \$0x7b,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 10[ 	]*vcmpeq_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 08[ 	]*vcmpeq_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 18[ 	]*vcmpeq_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1b[ 	]*vcmpfalse_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1d[ 	]*vcmpge_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1e[ 	]*vcmpgt_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 12[ 	]*vcmple_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 11[ 	]*vcmplt_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0c[ 	]*vcmpneq_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1c[ 	]*vcmpneq_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 14[ 	]*vcmpneq_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 19[ 	]*vcmpnge_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1a[ 	]*vcmpngt_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 16[ 	]*vcmpnle_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 15[ 	]*vcmpnlt_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 17[ 	]*vcmpord_sps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1f[ 	]*vcmptrue_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 13[ 	]*vcmpunord_sps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed ab[ 	]*vcmpps \$0xab,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 7b[ 	]*vcmpps \$0x7b,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 10[ 	]*vcmpeq_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 08[ 	]*vcmpeq_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 18[ 	]*vcmpeq_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1b[ 	]*vcmpfalse_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1d[ 	]*vcmpge_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1e[ 	]*vcmpgt_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 12[ 	]*vcmple_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 11[ 	]*vcmplt_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0c[ 	]*vcmpneq_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1c[ 	]*vcmpneq_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 14[ 	]*vcmpneq_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 19[ 	]*vcmpnge_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1a[ 	]*vcmpngt_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 16[ 	]*vcmpnle_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 15[ 	]*vcmpnlt_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 17[ 	]*vcmpord_ssd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1f[ 	]*vcmptrue_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 13[ 	]*vcmpunord_ssd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec ab[ 	]*vcmpsd \$0xab,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 7b[ 	]*vcmpsd \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 10[ 	]*vcmpeq_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 08[ 	]*vcmpeq_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 18[ 	]*vcmpeq_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1b[ 	]*vcmpfalse_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1d[ 	]*vcmpge_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1e[ 	]*vcmpgt_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 12[ 	]*vcmple_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 11[ 	]*vcmplt_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0c[ 	]*vcmpneq_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1c[ 	]*vcmpneq_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 14[ 	]*vcmpneq_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 19[ 	]*vcmpnge_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1a[ 	]*vcmpngt_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 16[ 	]*vcmpnle_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 15[ 	]*vcmpnlt_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 17[ 	]*vcmpord_sss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1f[ 	]*vcmptrue_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 13[ 	]*vcmpunord_sss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec ab[ 	]*vcmpss \$0xab,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 7b[ 	]*vcmpss \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2f f5[ 	]*vcomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2f f5[ 	]*vcomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 13 f5[ 	]*vcvtph2ps \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 5a f5[ 	]*vcvtps2pd \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee ab[ 	]*vcvtps2ph \$0xab,\{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee 7b[ 	]*vcvtps2ph \$0x7b,\{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5a f4[ 	]*vcvtss2sd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 e6 f5[ 	]*vcvttpd2dq \{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 38 5b f5[ 	]*vcvttps2dq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c ee[ 	]*vcvttsd2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 2c ee[ 	]*vcvttsd2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c ee[ 	]*vcvttss2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 2c ee[ 	]*vcvttss2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 38 42 f5[ 	]*vgetexppd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 42 f5[ 	]*vgetexpps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 30 43 f4[ 	]*vgetexpsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 30 43 f4[ 	]*vgetexpss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 ab[ 	]*vgetmantpd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 7b[ 	]*vgetmantpd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 ab[ 	]*vgetmantps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 7b[ 	]*vgetmantps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 ab[ 	]*vgetmantsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 7b[ 	]*vgetmantsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 ab[ 	]*vgetmantss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 7b[ 	]*vgetmantss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5f f4[ 	]*vmaxpd \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5f f4[ 	]*vmaxps \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5f f4[ 	]*vmaxsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5f f4[ 	]*vmaxss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5d f4[ 	]*vminpd \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5d f4[ 	]*vminps \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5d f4[ 	]*vminsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5d f4[ 	]*vminss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2e f5[ 	]*vucomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2e f5[ 	]*vucomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 ab[ 	]*vfixupimmpd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 7b[ 	]*vfixupimmpd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 ab[ 	]*vfixupimmps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 7b[ 	]*vfixupimmps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 ab[ 	]*vfixupimmsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 7b[ 	]*vfixupimmsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 ab[ 	]*vfixupimmss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 7b[ 	]*vfixupimmss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 ab[ 	]*vrndscalepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 7b[ 	]*vrndscalepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 ab[ 	]*vrndscaleps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 7b[ 	]*vrndscaleps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 ab[ 	]*vrndscalesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 7b[ 	]*vrndscalesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 ab[ 	]*vrndscaless \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 7b[ 	]*vrndscaless \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 78 f5[ 	]*vcvttpd2udq \{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 78 f5[ 	]*vcvttps2udq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 ee[ 	]*vcvttss2usi \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 78 ee[ 	]*vcvttss2usi \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 00[ 	]*vcmpeqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 10[ 	]*vcmpeq_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 08[ 	]*vcmpeq_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 18[ 	]*vcmpeq_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0b[ 	]*vcmpfalsepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1b[ 	]*vcmpfalse_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1d[ 	]*vcmpge_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0d[ 	]*vcmpgepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1e[ 	]*vcmpgt_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0e[ 	]*vcmpgtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 12[ 	]*vcmple_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 02[ 	]*vcmplepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 11[ 	]*vcmplt_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 01[ 	]*vcmpltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0c[ 	]*vcmpneq_oqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1c[ 	]*vcmpneq_ospd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 04[ 	]*vcmpneqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 14[ 	]*vcmpneq_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 19[ 	]*vcmpnge_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 09[ 	]*vcmpngepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1a[ 	]*vcmpngt_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0a[ 	]*vcmpngtpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 16[ 	]*vcmpnle_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 06[ 	]*vcmpnlepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 15[ 	]*vcmpnlt_uqpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 05[ 	]*vcmpnltpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 07[ 	]*vcmpordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 17[ 	]*vcmpord_spd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 0f[ 	]*vcmptruepd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 1f[ 	]*vcmptrue_uspd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 03[ 	]*vcmpunordpd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 13[ 	]*vcmpunord_spd \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed ab[ 	]*vcmppd \$0xab,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 8d 30 c2 ed 7b[ 	]*vcmppd \$0x7b,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 00[ 	]*vcmpeqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 10[ 	]*vcmpeq_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 08[ 	]*vcmpeq_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 18[ 	]*vcmpeq_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0b[ 	]*vcmpfalseps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1b[ 	]*vcmpfalse_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1d[ 	]*vcmpge_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0d[ 	]*vcmpgeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1e[ 	]*vcmpgt_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0e[ 	]*vcmpgtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 12[ 	]*vcmple_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 02[ 	]*vcmpleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 11[ 	]*vcmplt_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 01[ 	]*vcmpltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0c[ 	]*vcmpneq_oqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1c[ 	]*vcmpneq_osps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 04[ 	]*vcmpneqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 14[ 	]*vcmpneq_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 19[ 	]*vcmpnge_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 09[ 	]*vcmpngeps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1a[ 	]*vcmpngt_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0a[ 	]*vcmpngtps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 16[ 	]*vcmpnle_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 06[ 	]*vcmpnleps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 15[ 	]*vcmpnlt_uqps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 05[ 	]*vcmpnltps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 07[ 	]*vcmpordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 17[ 	]*vcmpord_sps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 0f[ 	]*vcmptrueps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 1f[ 	]*vcmptrue_usps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 03[ 	]*vcmpunordps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 13[ 	]*vcmpunord_sps \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed ab[ 	]*vcmpps \$0xab,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 0c 30 c2 ed 7b[ 	]*vcmpps \$0x7b,\{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 00[ 	]*vcmpeqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 10[ 	]*vcmpeq_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 08[ 	]*vcmpeq_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 18[ 	]*vcmpeq_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0b[ 	]*vcmpfalsesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1b[ 	]*vcmpfalse_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1d[ 	]*vcmpge_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0d[ 	]*vcmpgesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1e[ 	]*vcmpgt_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0e[ 	]*vcmpgtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 12[ 	]*vcmple_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 02[ 	]*vcmplesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 11[ 	]*vcmplt_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 01[ 	]*vcmpltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0c[ 	]*vcmpneq_oqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1c[ 	]*vcmpneq_ossd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 04[ 	]*vcmpneqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 14[ 	]*vcmpneq_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 19[ 	]*vcmpnge_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 09[ 	]*vcmpngesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1a[ 	]*vcmpngt_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0a[ 	]*vcmpngtsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 16[ 	]*vcmpnle_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 06[ 	]*vcmpnlesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 15[ 	]*vcmpnlt_uqsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 05[ 	]*vcmpnltsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 07[ 	]*vcmpordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 17[ 	]*vcmpord_ssd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 0f[ 	]*vcmptruesd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 1f[ 	]*vcmptrue_ussd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 03[ 	]*vcmpunordsd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 13[ 	]*vcmpunord_ssd \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec ab[ 	]*vcmpsd \$0xab,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 97 30 c2 ec 7b[ 	]*vcmpsd \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 00[ 	]*vcmpeqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 10[ 	]*vcmpeq_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 08[ 	]*vcmpeq_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 18[ 	]*vcmpeq_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0b[ 	]*vcmpfalsess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1b[ 	]*vcmpfalse_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1d[ 	]*vcmpge_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0d[ 	]*vcmpgess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1e[ 	]*vcmpgt_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0e[ 	]*vcmpgtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 12[ 	]*vcmple_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 02[ 	]*vcmpless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 11[ 	]*vcmplt_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 01[ 	]*vcmpltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0c[ 	]*vcmpneq_oqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1c[ 	]*vcmpneq_osss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 04[ 	]*vcmpneqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 14[ 	]*vcmpneq_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 19[ 	]*vcmpnge_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 09[ 	]*vcmpngess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1a[ 	]*vcmpngt_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0a[ 	]*vcmpngtss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 16[ 	]*vcmpnle_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 06[ 	]*vcmpnless \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 15[ 	]*vcmpnlt_uqss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 05[ 	]*vcmpnltss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 07[ 	]*vcmpordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 17[ 	]*vcmpord_sss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 0f[ 	]*vcmptruess \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 1f[ 	]*vcmptrue_usss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 03[ 	]*vcmpunordss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 13[ 	]*vcmpunord_sss \{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec ab[ 	]*vcmpss \$0xab,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 91 16 30 c2 ec 7b[ 	]*vcmpss \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2f f5[ 	]*vcomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2f f5[ 	]*vcomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 13 f5[ 	]*vcvtph2ps \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 5a f5[ 	]*vcvtps2pd \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee ab[ 	]*vcvtps2ph \$0xab,\{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 1d ee 7b[ 	]*vcvtps2ph \$0x7b,\{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5a f4[ 	]*vcvtss2sd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 e6 f5[ 	]*vcvttpd2dq \{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 38 5b f5[ 	]*vcvttps2dq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 2c ee[ 	]*vcvttsd2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 2c ee[ 	]*vcvttsd2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 2c c6[ 	]*vcvttsd2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 2c ee[ 	]*vcvttss2si \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 2c ee[ 	]*vcvttss2si \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 2c c6[ 	]*vcvttss2si \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 38 42 f5[ 	]*vgetexppd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 38 42 f5[ 	]*vgetexpps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 30 43 f4[ 	]*vgetexpsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 30 43 f4[ 	]*vgetexpss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 ab[ 	]*vgetmantpd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 26 f5 7b[ 	]*vgetmantpd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 ab[ 	]*vgetmantps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 26 f5 7b[ 	]*vgetmantps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 ab[ 	]*vgetmantsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 27 f4 7b[ 	]*vgetmantsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 ab[ 	]*vgetmantss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 27 f4 7b[ 	]*vgetmantss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5f f4[ 	]*vmaxpd \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5f f4[ 	]*vmaxps \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5f f4[ 	]*vmaxsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5f f4[ 	]*vmaxss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 30 5d f4[ 	]*vminpd \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 30 5d f4[ 	]*vminps \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 97 30 5d f4[ 	]*vminsd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 16 30 5d f4[ 	]*vminss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 2e f5[ 	]*vucomisd \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 2e f5[ 	]*vucomiss \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 ab[ 	]*vfixupimmpd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 54 f4 7b[ 	]*vfixupimmpd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 ab[ 	]*vfixupimmps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 54 f4 7b[ 	]*vfixupimmps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 ab[ 	]*vfixupimmsd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 55 f4 7b[ 	]*vfixupimmsd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 ab[ 	]*vfixupimmss \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 55 f4 7b[ 	]*vfixupimmss \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 ab[ 	]*vrndscalepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 38 09 f5 7b[ 	]*vrndscalepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 ab[ 	]*vrndscaleps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 38 08 f5 7b[ 	]*vrndscaleps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 ab[ 	]*vrndscalesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 30 0b f4 7b[ 	]*vrndscalesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 ab[ 	]*vrndscaless \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 30 0a f4 7b[ 	]*vrndscaless \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 78 f5[ 	]*vcvttpd2udq \{sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7c 38 78 f5[ 	]*vcvttps2udq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7f 38 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7f 38 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 ff 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 ff 38 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm30,%r8
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%eax
[ 	]*[a-f0-9]+:[ 	]*62 91 7e 38 78 ee[ 	]*vcvttss2usi \{sae\},%xmm30,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 11 7e 38 78 ee[ 	]*vcvttss2usi \{sae\},%xmm30,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 91 fe 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%rax
[ 	]*[a-f0-9]+:[ 	]*62 11 fe 38 78 c6[ 	]*vcvttss2usi \{sae\},%xmm30,%r8
#pass
