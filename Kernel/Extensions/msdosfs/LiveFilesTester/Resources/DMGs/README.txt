To create a dmg (of FAT16 or ExFat for example):
hdiutil create -size 4G -fs "MS-DOS FAT16" -volname test_fat16 usb_storgae_fat16.dmg
hdiutil create -size 1M -fs ExFAT -volname test_exfat usb_storgae_exfat.dmg

The maximum size for each FAT fs is:
FAT12 - 12MB
FAT16 - 4GB
FAT32 - 2TB (With max file size of 4GB)

In order to get info on dmg file:
hdiutil fsid <dmg_file>

