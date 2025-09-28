	.cpu	archs
        add_s   r0,gp,@a@sda
        add     r0,pcl,@var@tlsgd
        bl      @a@plt
