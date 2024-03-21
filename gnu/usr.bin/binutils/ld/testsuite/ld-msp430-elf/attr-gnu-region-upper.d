#source: attr-gnu-main.s -ml -mdata-region=upper
#source: attr-gnu-obj.s -ml -mdata-region=none
#ld: --data-region=either
#readelf: -A

Attribute Section: mspabi
File Attributes
  Tag_ISA: MSP430X
  Tag_Code_Model: Large
  Tag_Data_Model: Large
Attribute Section: gnu
File Attributes
  Tag_GNU_MSP430_Data_Region: Any Region
