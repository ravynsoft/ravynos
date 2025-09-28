# Use self-test mode to verify that all the expected control register
# names assemble correctly.
	
_start:
        wrctl ctl0,    r2, 0x1001703a
        wrctl status,  r2, 0x1001703a

        wrctl ctl1,    r2, 0x1001707a
        wrctl estatus, r2, 0x1001707a

        wrctl ctl2,    r2, 0x100170ba
        wrctl bstatus, r2, 0x100170ba

        wrctl ctl3,    r2, 0x100170fa
        wrctl ienable, r2, 0x100170fa

#        wrctl ctl4,     r2, 0x1001713a		# write-only register
#        wrctl ipending, r2, 0x1001713a		#

         wrctl ctl5,     r2, 0x1001717a
         wrctl cpuid,    r2, 0x1001717a

         wrctl ctl6,     r2, 0x100171ba

         wrctl ctl7,     r2, 0x100171fa
         wrctl exception,r2, 0x100171fa

         wrctl ctl8,     r2, 0x1001723a
         wrctl pteaddr,  r2, 0x1001723a

         wrctl ctl9,     r2, 0x1001727a
         wrctl tlbacc,   r2, 0x1001727a

         wrctl ctl10,    r2, 0x100172ba
         wrctl tlbmisc,  r2, 0x100172ba

         wrctl ctl11,    r2, 0x100172fa
         wrctl eccinj,  r2, 0x100172fa

        wrctl ctl12,     r2, 0x1001733a
        wrctl badaddr,   r2, 0x1001733a

        wrctl ctl13,     r2, 0x1001737a
        wrctl config,    r2, 0x1001737a

        wrctl ctl14,     r2, 0x100173ba
        wrctl mpubase,   r2, 0x100173ba

        wrctl ctl15,     r2, 0x100173fa
        wrctl mpuacc,    r2, 0x100173fa

        wrctl ctl16,     r2, 0x1001743a
        wrctl ctl17,     r2, 0x1001747a
        wrctl ctl18,     r2, 0x100174ba
        wrctl ctl19,     r2, 0x100174fa
        wrctl ctl20,     r2, 0x1001753a
        wrctl ctl21,     r2, 0x1001757a
        wrctl ctl22,     r2, 0x100175ba
        wrctl ctl23,     r2, 0x100175fa
        wrctl ctl24,     r2, 0x1001763a
        wrctl ctl25,     r2, 0x1001767a
        wrctl ctl26,     r2, 0x100176ba
        wrctl ctl27,     r2, 0x100176fa
        wrctl ctl28,     r2, 0x1001773a
        wrctl ctl29,     r2, 0x1001777a
        wrctl ctl30,     r2, 0x100177ba
        wrctl ctl31,     r2, 0x100177fa


        rdctl r2,ctl0,   0x0005303a
        rdctl r2,status, 0x0005303a

        rdctl r2,ctl1,    0x0005307a
        rdctl r2,estatus, 0x0005307a

        rdctl r2,ctl2,    0x000530ba
        rdctl r2,bstatus, 0x000530ba

        rdctl r2,ctl3,    0x000530fa
        rdctl r2,ienable, 0x000530fa

        rdctl r2,ctl4,     0x0005313a
        rdctl r2,ipending, 0x0005313a

        rdctl r2,ctl5,     0x0005317a
        rdctl r2,cpuid,    0x0005317a

        rdctl r2,ctl6,     0x000531ba

        rdctl r2,ctl7,     0x000531fa
        rdctl r2,exception,0x000531fa

        rdctl r2,ctl8,     0x0005323a
        rdctl r2,pteaddr,  0x0005323a

        rdctl r2,ctl9,     0x0005327a
        rdctl r2,tlbacc,   0x0005327a

        rdctl r2,ctl10,    0x000532ba
        rdctl r2,tlbmisc,  0x000532ba

        rdctl r2,ctl11,    0x000532fa
        rdctl r2,eccinj,  0x000532fa

        rdctl r2,ctl12,    0x0005333a
        rdctl r2,badaddr,  0x0005333a

        rdctl r2,ctl13,    0x0005337a
        rdctl r2,config,   0x0005337a

        rdctl r2,ctl14,    0x000533ba
        rdctl r2,mpubase,  0x000533ba

        rdctl r2,ctl15,    0x000533fa
        rdctl r2,mpuacc,   0x000533fa

        rdctl r2,ctl16,    0x0005343a
        rdctl r2,ctl17,    0x0005347a
        rdctl r2,ctl18,    0x000534ba
        rdctl r2,ctl19,    0x000534fa
        rdctl r2,ctl20,    0x0005353a
        rdctl r2,ctl21,    0x0005357a
        rdctl r2,ctl22,    0x000535ba
        rdctl r2,ctl23,    0x000535fa
        rdctl r2,ctl24,    0x0005363a
        rdctl r2,ctl25,    0x0005367a
        rdctl r2,ctl26,    0x000536ba
        rdctl r2,ctl27,    0x000536fa
        rdctl r2,ctl28,    0x0005373a
        rdctl r2,ctl29,    0x0005377a
        rdctl r2,ctl30,    0x000537ba
        rdctl r2,ctl31,    0x000537fa


