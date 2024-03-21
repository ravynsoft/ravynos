	.set noreorder
	.set noat

	.globl text_label .text
text_label:
	# All instructions have at least one error in suffix or in register
	# usage. All errors should be detected by the assembler.
	vabs.w		$vf0w, $vf0z
	vabs.w		$vf0z, $vf31w
	vabs.xw		$vf0xw, $vf0w
	vabs.xw		$vf0x, $vf31xw
	vabs.xyzw	$vf0xyz, $vf0xyzw
	vaddai.w	$ACCw, $vf0w, $Q
	vaddai.w	$ACCz, $vf0w, $Q
	vaddai.xyzw	$ACCxyw, $vf0xyzw, $I
	vaddaq.w	$ACCw, $vf31z, $Q
	vaddaq.x	$ACCx, $vf0w, $Q
	vaddaq.x	$ACCw, $vf1x, $Q
	vaddaq.xw	$ACCxw, $vf1z, $Q
	vaddaq.xw	$ACCw, $vf31xw, $Q
	vaddaq.xyw	$ACCxw, $vf0xyw, $Q
	vaddaq.xyw	$ACCxyw, $vf1yw, $Q
	vaddaq.xyzw	$ACCxyzw, $vf1yzw, $Q
	vaddaq.z	$ACCxz, $vf0xz, $Q
	vaddaq.x	$ACCxz, $vf1xz, $Q
	vaddaq.xzw	$ACCxw, $vf0xzw, $Q
	vaddaq.y	$ACCy, $vf0y, $R
	vaddaq.y	$ACCy, $vf1y, $I
	vaddaq.yw	$ACCxyw, $vf0yw, $Q
	vaddaq.yw	$ACCwy, $vf1yw, $Q
	vaddaq.yw	$ACCyw, $vf31wy, $Q
	vaddaq.yz	$ACCy, $vf0yz, $Q
	vaddaq.yzw	$ACCxyzw, $vf0yzw, $Q
	vaddaq.yzw	$ACCyzw, $vf1xyzw, $Q
	vaddaq.yzw	$ACCyzw, $vf31yzw, $Qyzw
	vadda.w		$ACCw, $vf1w, $vf2z
	vadda.w		$ACCw, $vf31w, $vf0x
	vaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vaddaw.xz	$ACCxyz, $vf0xz, $vf0w
	vaddaw.xz	$ACCxzw, $vf0xz, $vf31w
	vaddaw.xz	$ACCxz, $vf1xyz, $vf2w
	vaddaw.xz	$ACCxz, $vf31xzw, $vf0w
	vaddaw.xz	$ACCxz, $vf31z, $vf15w
	vaddaw.xzw	$ACCxyzw, $vf0xzw, $vf0w
	vaddaw.xzw	$ACC, $vf0, $vf31z
	vaddaw.xzw	$ACCxzw, $vf1xzw, $vf2z
	vaddaw.xzw	$ACCxzw, $vf31xzw, $vf0x
	vaddaw.y	$ACCy, $vf0y, $vf0z
	vaddax.w	$ACCw, $vf0w, $vf31z
	vaddax.w	$ACC, $vf1, $vf2z
	vaddax.w	$ACCw, $vf31w, $vf0w
	vaddax.w	$ACCw, $vf31w, $vf15y
	vadda.xw	$ACCxw, $vf0xw, $vf0xyw
	vadda.xw	$ACCxw, $vf0xw, $vf31wx
	vaddax.x	$ACCw, $vf0x, $vf0x
	vaddax.x	$ACCx, $vf0x, $vf31w
	vaddax.x	$ACCx, $vf1x, $vf2y
	vaddax.x	$ACCx, $vf31x, $vf0z
	vaddax.xw	$ACCxw, $vf0xw, $vf0y
	vaddax.xw	$ACCxw, $vf0xw, $vf31z
	vaddax.xw	$ACCxw, $vf1xw, $vf2w
	vaddax.xw	$ACCxw, $vf31wx, $vf0
	vaddax.xyzw	$ACC, $vf1, $vf2y
	vaddax.xyzw	$ACC, $vf31, $vf0z
	vadda.zw	$ACCzw, $vf0zw, $vf0wz
	vadda.zw	$ACCzw, $vf0w, $vf31zw
	vadda.zw	$ACCw, $vf1zw, $vf2zw
	vadda.zw	$ACCxzw, $vf31zw, $vf0zw
	vadda.zw	$ACCzw, $vf31xzw, $vf15zw
	vadda.zw	$ACCzw, $vf31zw, $vf31yzw
	vaddaz.x	$ACCx, $vf0x, $vf0x
	vaddaz.x	$ACCx, $vf0x, $vf31y
	vaddaz.xw	$ACCxw, $vf31w, $vf15z
	vaddaz.xw	$ACCx, $vf31xw, $vf31z
	vaddaz.xy	$ACCx, $vf0xy, $vf0z
	vaddaz.xy	$ACCxyz, $vf0xy, $vf31z
	vaddaz.y	$ACCx, $vf31y, $vf0z
	vaddaz.yw	$ACCyw, $vf0yw, $a0
	vaddi.w		$vf0w, $vf31w, $Q
	vaddi.w		$vf1w, $vf2w, $R
	vaddi.w		$vf31w, $vf0w, $ACC
	vaddi.w		$vf31w, $vf15w, $ACCw
	vaddi.xzw	$vf1xyzw, $vf2xzw, $I
	vaddi.xzw	$vf31xw, $vf0xzw, $I
	vaddi.xzw	$vf31xzw, $vf15xzw, $Ixzw
	vaddq.w		$vf1w, $vf2w, $Qw
	vaddq.w		$vf31w, $vf0w, $R
	vaddq.w		$vf31w, $vf15w, $ACCw
	vaddq.w		$vf31w, $vf31w, $ACC
	vaddq.xyzw	$vf31xyzw, $vf32xyzw, $Q
	vaddq.xyzw	$vf31xyzw, $32, $Q
	vaddq.xz	$vf0xz, $-1, $Q
	vaddw.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vaddx.yw	$vf31yw, $vf31yw, $vf31y
	vadd.xyz	$vf0xyz, $vf0xyz, $vf0xz
	vadd.xyz	$vf0xyz, $vf0xyz, $vf31xyzw
	vadd.xyz	$vf0xyz, $vf31xyzw, $vf0xyz
	vaddx.yz	$vf0yz, $vf0xyz, $vf0x
	vaddz.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vaddz.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vaddz.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vcallms		-1
	vcallms		-0x0080
	vcallms		0x1
	vcallms		0x7
	vcallms		0x4
	vcallms		0x2
	vcallms		0x40000
	vcallms		0x40008
	vclipw.xyz	$vf0xyz, $vf0x
	vclipw.xyz	$vf0xyz, $vf31y
	vclipw.xyz	$vf1xyz, $vf2z
	vdiv		$Q, $vf0x, $vf0xy
	vdiv		$Q, $vf0xyzw, $vf31y
	vdiv		$Q, $vf1, $vf2z
	vdiv		$Q, $vf31x, $vf15
	vdiv		$0, $vf31w, $vf31y
	vdiv		$Q, $vf32y, $vf0w
	vftoi0.w	$vf0w, $vf0x
	vftoi0.w	$vf0x, $vf31w
	vftoi0.w	$vf1xw, $vf2w
	vftoi0.w	$vf31wx, $vf0w
	vftoi0.w	$vf31w, $vf15wz
	vftoi12.xw	$vf0xw, $vf0w
	vftoi12.xw	$vf0x, $vf31xw
	vftoi15.xyz	$vf31xyzw, $vf15xyz
	vftoi15.xyz	$vf31xyz, $vf31xyzw
	vftoi15.xyzw	$vf0xyzw, $vf0xyz
	vftoi15.y	$vf1y, $vf2x
	vftoi15.y	$vf31y, $vf0w
	viaddi		$vi0, $vi0, -17
	viaddi		$vi1, $vi2, 16
	viaddi		$vi31, $vi0, 17
	viaddi		$vi31, $vi15, 32
	viaddi		$vi31, $vi31, 31
	viand		$vi0xyzw, $vi0, $vi0
	viand		$vi0, $vi0xyzw, $vi31
	viand		$vi0, $vi31, $vi0xyzw
	viand		$vi1, $vi2, $vi3x
	viand		$vi31, $vi0y, $vi0
	viand		$vi31w, $vi15, $vi7
	viand		$vi31, $vi31, $vi31x
	vilwr.w		$vi0, ($vi0x)
	vilwr.w		$vi0, ($vi31y)
	vilwr.w		$vi1, ($vi2z)
	vilwr.w		$vi31, ($vi0w)
	vilwr.w		$vi31, ($vi15xyzw)
	vilwr.w		$vi31x, ($vi31)
	vilwr.x		$vi0y, ($vi0)
	vilwr.x		$vi0z, ($vi31)
	vilwr.x		$vi1w, ($vi2)
	vilwr.x		$vi31xyzw, ($vi0)
	vilwr.x		$vi31xy, ($vi15)
	vilwr.x		$vi31zw, ($vi31)
	vilwr.y		$vi0wx, ($vi0)
	vilwr.y		$vi0xyzw, ($vi31)
	vilwr.y		$vi1y, ($vi2)
	vilwr.y		$vi31, ($vi0y)
	vilwr.z		$vi0z, ($vi0)
	vilwr.z		$vi0, ($vi31z)
	vior		$vi0x, $vi0, $vi0
	vior		$vi0, $vi0x, $vi31
	vior		$vi0, $vi31, $vi0x
	vior		$vi1y, $vi2, $vi3
	vior		$vi31, $vi0y, $vi0
	vior		$vi31, $vi15, $vi7y
	vior		$vi31xyzw, $vi31, $vi31
	visub		$vi0x, $vi0, $vi0
	visub		$vi0, $vi0y, $vi31
	visub		$vi0, $vi31, $vi0z
	visub		$vi1w, $vi2, $vi3
	visub		$vi31, $vi0xy, $vi0
	visub		$vi31, $vi15, $vi7zw
	visub		$vi31, $vi31, $vi31w
	viswr.w		$vi0, ($vi0w)
	viswr.w		$vi0w, ($vi31)
	viswr.x		$vi0x, ($vi31)
	viswr.x		$vi1, ($vi2x)
	viswr.x		$vi31x, ($vi0x)
	viswr.y		$vi31y, ($vi15)
	viswr.y		$vi31, ($vi31y)
	viswr.z		$vi0, ($vi0z)
	viswr.z		$vi0z, ($vi31)
	viswr.z		$vi1z, ($vi2z)
	vitof0.w	$vf1w, $vf2x
	vitof0.w	$vf31z, $vf0w
	vitof0.xw	$vf0xw, $vf0xyw
	vitof0.xw	$vf0xw, $vf31w
	vitof12.xw	$vf31xw, $vf0x
	vitof12.xzw	$vf0xzw, $vf31xz
	vitof12.xzw	$vf1xzw, $vf2xw
	vitof12.xzw	$vf31xzw, $vf0xyzw
	vitof12.xzw	$vf31xyzw, $vf15xzw
	vitof12.xzw	$vf31xw, $vf31xzw
	vitof12.y	$vf0y, $vf0w
	vitof12.y	$vf0x, $vf31y
	vitof15.xyw	$vf0xyw, $vf31xw
	vitof15.xyw	$vf1xyw, $vf2yxw
	vitof15.xyw	$vf31xwy, $vf15xyw
	vitof15.xyzw	$vf1.xyzw, $vf2xyzw
	vitof15.xyzw	$vf31xyzw, $vf0.xyzw
	vitof4.xw	$vf31xw, $31xw
	vitof4.xy	$0xy, $vf0xy
	vitof4.xyzw	$vf0yzw, $vf0xyzw
	vitof4.yzw	$vf1yzw, $vf2yw
	vlqd.w		$vf0, (--$vi0w)
	vlqd.w		$vf0, (--$vi31w)
	vlqd.w		$vf0x, (--$vi0)
	vlqd.x		$vf0w, (--$vi0x)
	vlqd.x		$vf0x, (--$vi31x)
	vlqd.x		$vf0w, (--$vi0)
	vlqd.xw		$vf0, (--$vi0xw)
	vlqd.xy		$vf0, (--$vi0xy)
	vlqd.xyw	$vf0, (--$vi0xyw)
	vlqd.xyz	$vf0, (--$vi0xyz)
	vlqd.xyzw	$vf0, (--$vi0xyzw)
	vlqd.xz		$vf0, (--$vi0xz)
	vlqd.xzw	$vf0, (--$vi0xzw)
	vlqd.y		$vf0, (--$vi0y)
	vlqd.yw		$vf0, (--$vi0yw)
	vlqd.yz		$vf0, (--$vi0yz)
	vlqd.yzw	$vf0, (--$vi0yzw)
	vlqd.z		$vf0, (--$vi0z)
	vlqd.zw		$vf0, (--$vi0zw)
	vlqi.w		$vf0, ($vi0w++)
	vlqi.x		$vf31, ($vi15x++)
	vlqi.xw		$vf0x, ($vi0++)
	vlqi.xw		$vf0, ($vi31xw++)
	vlqi.xy		$vf0, ($vi0xy++)
	vlqi.xy		$vf1, ($2xy++)
	vlqi.xyw	$vf0, ($vi0xyw++)
	vlqi.xyz	$vf0, ($vi0xyz++)
	vlqi.xyzw	$vf0, ($vi0xyzw++)
	vlqi.xz		$vf0, ($vi0xz++)
	vlqi.xzw	$vf0, ($vi0xzw++)
	vlqi.y		$vf0, ($vi0y++)
	vlqi.yw		$vf0, ($vi0yw++)
	vlqi.yz		$vf0yz, ($vi0yz++)
	vlqi.yzw	$vf0, ($vi0yzw++)
	vlqi.z		$vf0, ($vi0z++)
	vlqi.zw		$vf0, ($vi0zw++)
	vmaddai.w	$ACCw, $vf0w, $R
	vmaddai.w	$ACCw, $vf1w, $Iw
	vmaddai.w	$ACCw, $vf31w, $Q
	vmaddai.x	$ACCx, $vf0x, $ACC
	vmaddai.x	$ACCy, $vf1w, $I
	vmaddai.x	$ACCxy, $vf31x, $I
	vmaddai.xw	$ACCxw, $vf0xyw, $I
	vmaddai.xy	$ACCxy, $vf0xyw, $I
	vmaddai.xy	$ACCxy, $vf1xyz, $I
	vmaddai.xy	$ACCxyz, $vf31xy, $I
	vmaddai.xyw	$ACCxy, $vf0xyw, $I
	vmaddai.yw	$ACCyw, $vf1w, $I
	vmaddai.yw	$ACCyw, $vf31y, $I
	vmaddai.yz	$ACCyz, $vf0yz, $R
	vmaddaq.xyz	$ACCxyz, $vf0xyz, $R
	vmaddaq.xyz	$ACCxyz, $vf1xyz, $Qxyz
	vmaddaq.xzw	$ACCxzw, $vf31xzw, $Qxzw
	vmaddaq.y	$ACCy, $vf0y, $R
	vmaddaq.y	$ACCy, $vf1y, $ACCy
	vmaddaq.y	$ACCy, $vf31y, $ACC
	vmaddaw.z	$ACCz, $vf31z, $vf0x
	vmaddaw.zw	$ACCzw, $vf31zw, $vf15y
	vmaddax.w	$ACCw, $vf1w, $vf2w
	vmadda.xw	$ACCxw, $vf31xw, $vf31wx
	vmaddax.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vmaddax.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vmaddax.xz	$ACCxz, $vf0xz, $vf31xz
	vmaddax.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vmaddax.z	$ACCz, $vf31z, $vf15z
	vmaddax.zw	$ACCzw, $vf1zw, $vf2zw
	vmadday.w	$ACCw, $vf1w, $vf2w
	vmadday.w	$ACCw, $vf31y, $vf0y
	vmadday.w	$ACCy, $vf31w, $vf15y
	vmadday.w	$ACCy, $vf31y, $vf31y
	vmadday.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vmadday.xyzw	$ACCxyzw, $vf0y, $vf31y
	vmadday.xyzw	$ACCy, $vf1xyzw, $vf2y
	vmadday.xyzw	$ACCy, $vf31y, $vf0y
	vmaddi.x	$vf0x, $vf31x, $Ix
	vmaddi.xw	$vf1xw, $vf2xw, $Ixw
	vmaddi.xy	$vf31xy, $vf0xy, $Ixy
	vmaddi.xyw	$vf0xyw, $vf0xyw, $Ixyw
	vmaddi.xyzw	$vf1xyzw, $vf2xyzw, $Ixyzw
	vmaddi.y	$vf0y, $vf0y, $Iy
	vmaddi.yw	$vf0yw, $vf0yw, $Iyw
	vmaddi.zw	$vf0zw, $vf31zw, $0
	vmaddq.w	$vf0w, $vf0w, $0
	vmadd.w		$vf0w, $vf0w, $vf0y
	vmaddw.xyz	$vf31xyz, $vf15xyz
	vmaddw.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmaddx.yw	$vf1yw, $vf2yw, $vf3yw
	vmaddy.xy	$vf31xy, $vf15xy, $vf7xy
	vmadd.z		$vf1z, $vf2z, $vf3x
	vmadd.z		$vf31z, $vf0z, $vf0w
	vmaddz.xyw	$vf0xyw, $vf0xyw, $vf31x
	vmaddz.xz	$vf0xz, $vf31xz, $vf0xz
	vmaddz.y	$vf31y, $vf0y, $vf0y
	vmaxi.w		$vf31w, $vf15w, $Q
	vmaxi.w		$vf31w, $vf31w, $0
	vmax.w		$vf31w, $vf31w, $vf31x
	vmaxw.w		$vf0w, $vf0w, $vf0x
	vmaxw.x		$vf0x, $vf0x, $vf0x
	vmaxw.x		$vf0w, $vf0w, $vf31w
	vmaxw.xw	$vf0xw, $vf0xw, $vf0xw
	vmaxw.xw	$vf0w, $vf0w, $vf31w
	vmaxw.xy	$vf0xy, $vf31xy, $vf0xy
	vmaxw.xy	$vf1xy, $vf2w, $vf3w
	vmaxw.xy	$vf31w, $vf0xy, $vf0w
	vmax.x		$vf0x, $vf0x, $vf31w
	vmaxx.w		$vf0w, $vf0w, $vf31w
	vmaxx.w		$vf0x, $vf31x, $vf0x
	vmaxx.w		$vf31w, $vf0w, $vf0w
	vmaxx.w		$vf31x, $vf15x, $vf7x
	vmax.xw		$vf31xw, $vf15xw, $vf7w
	vmaxx.x		$vf0x, $vf0x, $vf0w
	vmaxx.x		$vf31w, $vf15x, $vf7x
	vmaxx.x		$vf31x, $vf31w, $vf31x
	vmaxx.xw	$vf31xw, $vf15xw, $vf7xw
	vmaxx.xy	$vf0xy, $vf31xy, $vf0xy
	vmaxx.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmaxx.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmaxx.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmaxx.xyzw	$vf0xyzw, $vf0x, $vf31x
	vmaxx.xyzw	$vf0x, $vf31xyzw, $vf0x
	vmaxx.xyzw	$vf1x, $vf2x, $vf3x
	vmaxx.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmaxx.y		$vf0y, $vf0y, $vf0y
	vmaxx.y		$vf0y, $vf0x, $vf31x
	vmaxx.y		$vf0x, $vf31y, $vf0x
	vmaxx.yw	$vf1yw, $vf2yw, $vf3yw
	vmaxx.yz	$vf0yz, $vf0yz, $vf0yz
	vmaxx.yz	$vf0x, $vf0x, $vf31x
	vmaxx.z		$vf31z, $vf0z, $vf0z
	vmaxx.z		$vf31z, $vf15x, $vf7x
	vmaxx.z		$vf31x, $vf31z, $vf31x
	vmaxx.zw	$vf1zw, $vf2zw, $vf3zw
	vmax.y		$vf0y, $vf0y, $vf31x
	vmax.yw		$vf0yw, $vf0yw, $vf31w
	vmax.yw		$vf0yw, $vf31yw, $vf0y
	vmaxy.xz	$vf31xz, $vf15xz, $vf7xz
	vmaxy.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vmaxy.y		$vf1y, $vf2y
	vmaxy.yz	$vf0yz, $vf31yz, $vf0yz
	vmaxy.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmaxy.yzw	$vf31y, $vf15y, $vf7y
	vmaxy.yzw	$vf31yzw, $vf31yw, $vf31y
	vmaxy.z		$vf0z, $vf0z, $vf0z
	vmaxy.z		$vf0z, $vf0y, $vf31y
	vmaxy.z		$vf0y, $vf31z, $vf0y
	vmaxz.xw	$vf31xw, $vf31xw, $vf31xw
	vmaxz.xy	$vf0xy, $vf0xy, $vf0xy
	vmaxz.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmaxz.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vmaxz.xyz	$vf31xyz, $vf0z, $vf0z
	vmaxz.xyz	$vf31z, $vf15xyz, $vf7z
	vmaxz.xyz	$vf31z, $vf31z, $vf31z
	vmaxz.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmaxz.xyzw	$vf0xyzw, $vf0z, $vf31z
	vmaxz.xyzw	$vf0z, $vf31z, $vf0z
	vmaxz.xyzw	$vf1xyzw, $vf2xyzw, $vfz
	vmaxz.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmaxz.xz	$vf0xz, $vf0xz, $vf0xz
	vmaxz.y		$vf31y, $vf15z, $vf7z
	vmaxz.y		$vf31y, $vf31y, $vf31y
	vmaxz.yw	$vf0yw, $vf0yw, $vf0yw
	vmaxz.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmaxz.yzw	$vf0yzw, $vf31z, $vf0z
	vmaxz.yzw	$vf1z, $vf2yzw, $vf3z
	vmaxz.yzw	$vf31z, $vf0z, $vf0z
	vmaxz.z		$vf31z, $vf31z, $vf31x
	vmfir.w		$vf0w, $vi0w
	vmfir.w		$vf0, $vi31w
	vmfir.x		$vf0x, $vi0x
	vmfir.x		$vf0, $vi31x
	vmfir.xw	$vf0xw, $vi31xw
	vmfir.xy	$vf1xy, $vi2xy
	vmfir.xy	$vf31, $vi0xy
	vmfir.xyw	$vf0xyw, $vi31xyw
	vmfir.xyw	$vf31xyw, $vi0x
	vmfir.xyz	$vf0xyz, $vi0xyz
	vmfir.xyzw	$vf1xyzw, $vi2xyzw
	vmfir.xz	$vf0xz, $vi31xz
	vmfir.xzw	$vf0xzw, $vi31xzw
	vmfir.y		$vf0y, $vi0y
	vmfir.yw	$vf0yw, $vi0yw
	vmfir.yz	$vf0yz, $vi31yz
	vmfir.yzw	$vf0yzw, $vi0yzw
	vmfir.z		$vf0z, $vi0z
	vmfir.z		$0z, $vi31
	vmfir.zw	$vf0zw, $vi0zw
	vminii.w	$vf0w, $vf0w, $Iw
	vminii.w	$vf0w, $vf31x, $I
	vminii.w	$vf1x, $vf2w, $I
	vminii.xw	$vf0xw, $vf31xw, $Ixw
	vminii.xw	$vf1xw, $vf2w, $I
	vminii.xw	$vf31x, $vf0xw, $I
	vminii.xyw	$vf31xw, $vf0xyw, $I
	vminii.xyz	$vf0xy, $vf0xyz, $I
	minii.xz	$vf31z, $vf15xz, $I
	vminii.xz	$vf31xz, $vf31x, $I
	vminii.xzw	$vf0xzw, $vf0xw, $I
	vminii.xzw	$vf0zw, $vf31xzw, $I
	vminii.xzw	$vf1xyzw, $vf2xzw, $I
	vminii.xzw	$vf31xzw, $vf0xyzw, $I
	vminii.yw	$vf31yw, $vf31yw, $R
	vminii.yz	$vf0yz, $vf0yz, $Q
	vminii.yz	$vf0yz, $vf31yz, $ACC
	vminii.yzw	$vf31yzw, $vf0yzw, $R
	vminii.yzw	$vf31yzw, $vf15yzw, $ACC
	vminii.yzw	$vf31yzw, $vf31yzw, $Q
	vmini.w		$vf0w, $vf0w, $vf0x
	vminiw.w	$vf31w, $vf31w, $vf31x
	vminiw.x	$vf0x, $vf0x, $vf0x
	vminiw.x	$vf0x, $vf0w, $vf31w
	vminiw.x	$vf0w, $vf31x, $vf0w
	vminiw.x	$vf1w, $vf2w, $vf3w
	vminiw.xw	$vf0xw, $vf31xw, $vf0xw
	vminiw.xw	$vf1w, $vf2w, $vf3w
	vminiw.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vminiw.xyzw	$vf0xyzw, $vf0xxyzw, $vf31w
	vminiw.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vminiw.xyzw	$vf1xyzw, $vf2xyzw, $vf3ww
	vminiw.xz	$vf31xz, $vf0xz, $vf0xz
	vminiw.yw	$vf0yw, $vf0yw, $vf0yw
	vminiw.yz	$vf31yz, $vf0yz, $vf0yz
	vminiw.z	$vf31z, $vf0z, $vf0z
	vminiw.z	$vf31z, $vf15w, $vf7w
	vminiw.z	$vf31w, $vf31z, $vf31w
	vminix.xw	$vf0xw, $vf31xw, $vf0xw
	vminix.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vminix.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vminix.yw	$vf31yw, $vf31yw, $vf31yw
	vminix.zw	$vf31zw, $vf31zw, $vf31zw
	vmini.y		$vf0y, $vf0x, $vf0y
	vminiy.w	$vf0w, $vf31w, $vf0w
	vminiy.x	$vf31x, $vf15x, $vf7x
	vminiy.x	$vf31x, $vf31y, $vf31y
	vminiy.xw	$vf0y, $vf0xw, $vf0y
	vminiy.xw	$vf0xw, $vf0y, $vf31y
	vminiy.xw	$vf0xw, $vf31xw, $vf0xw
	vminiy.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vminiy.xyzw	$vf0xyzw, $vf0y, $vf0y
	vminiy.xyzw	$vf0y, $vf0xyzw, $vf31y
	vminiy.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vminiy.yw	$vf1yw, $vf2yw, $vf3yw
	vminiy.zw	$vf1zw, $vf2zw, $vf3zw
	vmini.z		$vf0z, $vf0z, $vf0x
	vminiz.x	$vf0x, $vf31x, $vf0x
	vminiz.xw	$vf0xw, $vf31xw, $vf0xw
	vminiz.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vminiz.xyw	$vf31xyw, $vf15z, $vf7z
	vmove.xyw	$vf0xyw, $vf0xw
	vmove.y		$vf0y, $vf31x
	vmr32.xw	$vf0xw, $vf0w
	vmr32.xw	$vf0w, $vf31xw
	vmsubai.xy	$ACCxy, $vf31xy, $Q
	vmsubai.xyw	$ACCxyw, $vf0xyw, $0
	vmsubai.xyw	$ACCxyw, $vf1xyw, $ACC
	vmsubai.xyw	$ACCxyw, $vf31xw, $I
	vmsubaq.y	$ACCy, $vf31y, $Qy
	vmsubaq.yw	$ACCw, $vf0yw, $Q
	vmsubaq.yw	$ACCwy, $vf1yw, $Q
	vmsubaw.x	$ACCx, $vf31x, $vf0x
	vmsubaw.x	$ACCx, $vf31w, $vf15w
	vmsubaw.x	$ACCw, $vf31x, $vf31w
	vmsubaw.xw	$ACCw, $vf0xw, $vf0w
	vmsubaw.xw	$ACCxw, $vf0w, $vf31w
	vmsubaw.xw	$ACCxw, $vf1xw, $vf2xw
	vmsubax.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vmsubax.z	$ACCz, $vf31z, $vf0z
	vmsuba.y	$ACCy, $vf31y, $vf15a
	vmsuba.yw	$ACCyw, $vf31yw, $vf0w
	vmsubay.x	$ACCx, $vf31x, $vf15x
	vmsubay.x	$ACCx, $vf31y, $vf31y
	vmsubay.xw	$ACCxw, $vf0xw, $vf0xw
	vmsubaz.xy	$ACCxy, $vf0xy, $vf31xy
	vmsubaz.yw	$ACCyw, $vf31yw, $vf0yw
	vmsubi.xyzw	$vf31xyzw, $vf0xyzw, $R
	vmsubw.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmsubw.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmsubw.y	$vf31y, $vf31y, $vf31y
	vmsubw.yw	$vf0yw, $vf0y, $vf0w
	vmsubw.yw	$vf0w, $vf0yw, $vf31w
	vmsubw.zw	$vf0zw, $vf0zw, $vf31zw
	vmsubx.w	$vf0w, $vf0w, $vf0w
	vmsub.y		$vf31y, $vf15y, $vf7w
	vmsuby.x	$vf0x, $vf0x, $vf31x
	vmsuby.x	$vf0x, $vf31y, $vf0y
	vmsubz.x	$vf0x, $vf31x, $vf0x
	vmulai.xyz	$ACCxyz, $vf1xz, $I
	vmulaq.zw	$ACCzw, $vf31zw, $I
	vmula.w		$ACCw, $vf31w, $vf0x
	vmulax.xz	$ACCxz, $vf0xz, $vf31xz
	vmulax.xz	$ACCxz, $vf1x, $vf2x
	vmulax.xz	$ACCx, $vf31xz, $vf0x
	vmulay.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vmulaz.w	$ACCw, $vf0w, $vf31w
	mulaz.xy	$ACCz, $vf31xy, $vf0z
	vmulaz.xy	$ACCxy, $vf31z, $vf15z
	vmulaz.z	$ACCz, $vf1z, $vf2x
	vmuli.x		$vf31x, $vf15x, $ACC
	vmulq.x		$vf0x, $vf31x, $0
	vmulq.x		$vf1x, $vf2x, $ACC
	vmulq.x		$vf31x, $vf0x, $R
	vmulq.x		$vf31x, $vf15x, $I
	vmulw.z		$vf31z, $vf15z, $vf7z
	vmulw.z		$vf31z, $vf31w, $vf31w
	vmulw.zw	$vf0zw, $vf0zw, $vf0zw
	vmuly.xyzw	$vf0xyzw, $vf0y, $vf31y
	vmuly.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmuly.xyzw	$vf1xyzw, $vf2y, $vf3y
	vmuly.xyzw	$vf31y, $vf0xyzw, $vf0y
	vmulz.y		$vf0y, $vf31y, $vf0y
	vmulz.y		$vf1y, $vf2z, $vf3z
	vmulz.y		$vf31z, $vf0y, $vf0z
	vmulz.y		$vf31z, $vf15z, $vf7z
	vopmsub		$vf0x, $vf0, $vf31
	vopmsub		$vf0, $vf31x, $vf0
	vopmsub		$vf1, $vf2, $vf3x
	vopmsub		$ACC, $vf0, $vf0
	vopmsub		$vf31, $R, $vf7
	vopmsub		$vf31, $vf31, $I
	vopmsub.xyz	$vf0xyz, $vf0xyz, $vf0xy
	vopmula		$0, $vf0, $vf0
	vopmula		$Q, $vf0, $vf31
	vopmula		$R, $vf1, $vf2
	vopmula		$I, $vf31, $vf0
	vopmula		$ACCx, $vf31, $vf15
	vopmula		$ACCxyzw, $vf31, $vf31
	vopmula.xyz	$ACCxyzw, $vf0xyz, $vf0xyz
	vopmula.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vrget.w		$vf0w, $0
	vrget.w		$vf1w, $I
	vrget.w		$vf31w, $Q
	vrget.x		$vf0x, $ACC
	vrget.x		$vf1y, $R
	vrget.xy	$vf31x, $R
	vrget.xyw	$vf0xw, $R
	vrget.xyw	$vf1yw, $R
	vrget.xyw	$vf31xy, $R
	vrget.xyz	$vf0xy, $R
	vrget.xyz	$vf1xyzw, $R
	vrget.xyz	$vf31xyzw, $R
	vrget.xyzw	$vf0xyz, $R
	vrget.xyzw	$vf1xzw, $R
	vrget.xyzw	$vf31yzw, $R
	vrget.xz	$vf0xz, $0
	vrget.xz	$vf1z, $R
	vrget.xzw	$vf0xw, $R
	vrget.y		$vf0z, $R
	vrget.y		$vf1y, $I
	vrget.z		$vf31z, $Q
	vrget.zw	$vf0zw, $ACC
	vrnext.xyzw	$vf0xyz, $R
	vrnext.xyzw	$vf1xyzw, $0
	vrnext.xyzw	$vf31xyzw, $Rxyzw
	vrnext.yz	$vf31yz, $Ryz
	vrnext.z	$vf0z, $Rz
	vrsqrt		$Q, $vf0xz, $vf31y
	vrsqrt		$Q, $vf1z, $vf2xz
	vrsqrt		$Q, $vf31yx, $vf15w
	vrsqrt		$Qx, $vf31x, $vf31y
	vrsqrt		$0, $vf31y, $vf0w
	vrxor		$0, $vf0w
	vrxor		$R, $vf0xy
	vrxor		$R, $vf0zw
	vrxor		$R, $vf1yz
	vrxor		$ACC, $vf31x
	vrxor		$Q, $vf31y
	vsqd.w		$vf0, (--$vi0w)
	vsqd.w		$vf0, (--$vi31w)
	vsqd.x		$vf1, (--$vi2x)
	vsqd.xw		$vf0, (--$vi0xw)
	vsqd.xy		$vf0, (--$vi0xy)
	vsqd.xyw	$vf0, (--$vi0xyw)
	vsqd.xyz	$vf0, (--$vi31xyz)
	vsqd.xyzw	$vf0, (--$vi0xyzw)
	vsqd.xz		$vf0, (--$vi31xz)
	vsqd.xzw	$vf0, (--$vi0xzw)
	vsqd.y		$vf0, (--$vi0y)
	vsqd.yw		$vf0, (--$vi31yw)
	vsqd.yz		$vf0, (--$vi31yz)
	vsqd.yzw	$vf0, (--$vi31yzw)
	vsqd.yzw	$vf0yzw, (--$vi0x)
	vsqd.z		$vf1, (--$vi2z)
	vsqd.zw		$vf1, (--$vi2zw)
	vsqi.w		$vf0, ($vi0w++)
	vsqi.x		$vf0x, ($vi0x++)
	vsqi.xw		$vf0xw, ($vi0xw++)
	vsqi.xw		$vf1x, ($vi2++)
	vsqi.xw		$vf31w, ($vi0++)
	vsqi.xy		$vf0, ($vi31xy++)
	vsqi.xyw	$vf0x, ($vi0++)
	vsqi.xyw	$vf0, ($vi31xyw++)
	vsqi.xyz	$vf0xyz, ($vi0xyz++)
	vsqi.xyzw	$vf0, ($vi31xyzw++)
	vsqi.xz		$vf0xz, ($vi0xz++)
	vsqi.xzw	$vf0xzw, ($vi0xzw++)
	vsqi.y		$vf1, ($vi2y++)
	vsqi.yw		$vf0yw, ($vi0yw++)
	vsqi.yz		$vf1, ($vi2yz++)
	vsqi.yzw	$vf0yzw, ($vi0yzw++)
	vsqi.z		$vf0, ($vi31z++)
	vsqi.zw		$vf0zw, ($vi0zw++)
	vsqrt		$Q, $vf1zw
	vsqrt		$Q, $vf31xw
	vsqrt		$Q, $vf31xy
	vsubai.w	$ACCw, $vf0w, $0
	vsubai.w	$ACCw, $vf1x, $I
	vsubai.w	$ACCx, $vf31w, $I
	vsubai.x	$ACCw, $vf31x, $I
	vsubai.xw	$ACCw, $vf0xw, $I
	vsubai.xw	$ACCxw, $vf1x, $I
	vsubai.xw	$ACCxw, $vf31xw, $0
	vsubai.xy	$ACCxy, $vf0y, $I
	vsubai.xy	$ACCxy, $vf1x, $I
	vsubai.xy	$ACCxy, $vf311xy, $I
	vsubai.xyz	$ACCxyz, $vf1yz, $I
	vsubai.xyz	$ACCxyz, $vf31xyz, $ACC
	vsubai.xyzw	$ACCxyzw, $vf0xyzw, $R
	vsubai.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vsubai.xz	$ACCxz, $vf1z, $I
	vsubai.y	$ACCy, $vf31, $3
	vsubai.yw	$ACCyw, $vf0yw, $Iyw
	vsubai.zw	$ACCzw, $vf1zw, $Izw
	vsubai.zw	$ACCzw, $vf31w, $I
	vsubaq.w	$ACCw, $Q, $Q
	vsubaq.w	$ACCw, $I, $Q
	vsubaq.xyw	$ACCxyw, $vf0xw, $Q
	vsubaq.xyzw	$ACCxyzw, $vf0xyz, $Q
	vsubaq.xzw	$ACCxzw, $vf1xw, $Q
	vsubaq.yw	$ACCyw, $vf31y, $Q
	vsubaq.yz	$ACCyz, $vf0yz, $ACC
	vsubaq.yz	$ACCyz, $vf1yz, $I
	vsubax.w	$ACCw, $vf0w, $vf0w
	vsubax.w	$ACCw, $vf0x, $vf31x
	vsubax.w	$ACCx, $vf1w, $vf2x
	vsubax.w	$ACCx, $vf31x, $vf0x
	vsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vsubax.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vsubax.y	$ACCy, $vf31y, $vf0y
	vsubax.yw	$ACCyw, $vf0yw, $vf0yw
	vsubay.yw	$ACCyw, $vf0yw, $vf31yw
	vsubay.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vsubay.z	$ACCz, $vf0z, $vf31z
	vsubay.zw	$ACCzw, $vf0zw, $vf0zw
	vsubaz.w	$ACCw, $vf31w, $vf15w
	vsubaz.x	$ACCx, $vf0x, $vf31x
	vsubaz.xy	$ACCxy, $vf0xy, $vf0xy
	vsubaz.xz	$ACCxz, $vf31xz, $vf15xz
	vsubaz.xz	$ACCxz, $vf31z, $vf31z
	vsubaz.xzw	$ACCxw, $vf0xzw, $vf0z
	vsubaz.xzw	$ACCz, $vf0xzw, $vf31z
	vsubaz.xzw	$ACCxzw, $vf1z, $vf2z
	vsubaz.yw	$ACCyw, $vf1yw, $vf2yw
	vsubi.w		$vf31w, $vf15w, $0
	vsubi.w		$vf31w, $vf31w, $R
	vsubi.x		$vf0x, $vf0y, $I
	vsubi.x		$vf0x, $vf31x, $Ix
	vsubi.xy	$vf0xy, $vf31y, $I
	vsubi.xy	$vf1x, $vf2xy, $I
	vsubq.x		$vf31x, $vf15x, $Qx
	vsubq.x		$vf31x, $vf31y, $Q
	vsubq.xw	$vf0xw, $vf0xw, $0
	vsubq.xw	$vf0xw, $vf31xw, $2
	vsubq.xyzw	$vf1yzw, $vf2xyzw, $Q
	vsubq.yw	$vf31w, $vf15yw, $Q
	vsubq.yw	$vf31yw, $vf31y, $Q
	vsubx.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vsubx.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vsuby.xw	$vf0xw, $vf0xw, $vf0xw
	vsuby.zw	$vf0zw, $vf0zw, $vf0zw
	vsub.z		$vf0z, $vf31z, $vf0x
	vsubz.xyw	$vf31yw, $vf15xyw, $vf7z
	vsubz.xyw	$vf31xyw, $vf31yw, $vf31z
	vsubz.xyz	$vf0xyz, $vf0xyz, $vf0x
	vwaitq		$vf0x

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
