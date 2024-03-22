io.write("HELLO WORLD\n")

r = rnn.init("a630")

function start_cmdstream(name)
  io.write("START: " .. name .. "\n")
end

function draw(primtype, nindx)
  io.write("DRAW: " .. primtype .. ", " .. nindx .. "\n")
  -- io.write("GRAS_CL_VPORT_XOFFSET: " .. r.GRAS_CL_VPORT_XOFFSET .. "\n")
  io.write("RB_MRT[0].CONTROL.ROP_CODE: " .. r.RB_MRT[0].CONTROL.ROP_CODE .. "\n")
  io.write("SP_VS_OUT[0].A_COMPMASK: " .. r.SP_VS_OUT[0].A_COMPMASK .. "\n")
  --io.write("RB_DEPTH_CONTROL.Z_TEST_ENABLE: " .. tostring(r.RB_DEPTH_CONTROL.Z_TEST_ENABLE) .. "\n")
  io.write("0x2280: written=" .. regs.written(0x2280) .. ", lastval=" .. regs.lastval(0x2280) .. ", val=" .. regs.val(0x2280) .. "\n")
end

function A6XX_TEX_CONST(pkt, size)
  io.write("\n-------- " .. size .. "\n")
  io.write("-------- w=" .. pkt[1].WIDTH .. ", h=" .. pkt[1].HEIGHT .. "\n")
  io.write("\n");
end

function end_cmdstream()
  io.write("END\n")
end

function finish()
  io.write("FINISH\n")
end

