	.set noreorder
	.set noat

	.globl text_label .text
text_label:
	vabs.w		$vf0w, $vf0w
	vabs.w		$vf0w, $vf31w
	vabs.w		$vf1w, $vf2w
	vabs.w		$vf31w, $vf0w
	vabs.w		$vf31w, $vf15w
	vabs.w		$vf31w, $vf31w
	vabs.x		$vf0x, $vf0x
	vabs.x		$vf0x, $vf31x
	vabs.x		$vf1x, $vf2x
	vabs.x		$vf31x, $vf0x
	vabs.x		$vf31x, $vf15x
	vabs.x		$vf31x, $vf31x
	vabs.xw		$vf0xw, $vf0xw
	vabs.xw		$vf0xw, $vf31xw
	vabs.xw		$vf1xw, $vf2xw
	vabs.xw		$vf31xw, $vf0xw
	vabs.xw		$vf31xw, $vf15xw
	vabs.xw		$vf31xw, $vf31xw
	vabs.xy		$vf0xy, $vf0xy
	vabs.xy		$vf0xy, $vf31xy
	vabs.xy		$vf1xy, $vf2xy
	vabs.xy		$vf31xy, $vf0xy
	vabs.xy		$vf31xy, $vf15xy
	vabs.xy		$vf31xy, $vf31xy
	vabs.xyw	$vf0xyw, $vf0xyw
	vabs.xyw	$vf0xyw, $vf31xyw
	vabs.xyw	$vf1xyw, $vf2xyw
	vabs.xyw	$vf31xyw, $vf0xyw
	vabs.xyw	$vf31xyw, $vf15xyw
	vabs.xyw	$vf31xyw, $vf31xyw
	vabs.xyz	$vf0xyz, $vf0xyz
	vabs.xyz	$vf0xyz, $vf31xyz
	vabs.xyz	$vf1xyz, $vf2xyz
	vabs.xyz	$vf31xyz, $vf0xyz
	vabs.xyz	$vf31xyz, $vf15xyz
	vabs.xyz	$vf31xyz, $vf31xyz
	vabs.xyzw	$vf0xyzw, $vf0xyzw
	vabs.xyzw	$vf0xyzw, $vf31xyzw
	vabs.xyzw	$vf1xyzw, $vf2xyzw
	vabs.xyzw	$vf31xyzw, $vf0xyzw
	vabs.xyzw	$vf31xyzw, $vf15xyzw
	vabs.xyzw	$vf31xyzw, $vf31xyzw
	vabs.xz		$vf0xz, $vf0xz
	vabs.xz		$vf0xz, $vf31xz
	vabs.xz		$vf1xz, $vf2xz
	vabs.xz		$vf31xz, $vf0xz
	vabs.xz		$vf31xz, $vf15xz
	vabs.xz		$vf31xz, $vf31xz
	vabs.xzw	$vf0xzw, $vf0xzw
	vabs.xzw	$vf0xzw, $vf31xzw
	vabs.xzw	$vf1xzw, $vf2xzw
	vabs.xzw	$vf31xzw, $vf0xzw
	vabs.xzw	$vf31xzw, $vf15xzw
	vabs.xzw	$vf31xzw, $vf31xzw
	vabs.y		$vf0y, $vf0y
	vabs.y		$vf0y, $vf31y
	vabs.y		$vf1y, $vf2y
	vabs.y		$vf31y, $vf0y
	vabs.y		$vf31y, $vf15y
	vabs.y		$vf31y, $vf31y
	vabs.yw		$vf0yw, $vf0yw
	vabs.yw		$vf0yw, $vf31yw
	vabs.yw		$vf1yw, $vf2yw
	vabs.yw		$vf31yw, $vf0yw
	vabs.yw		$vf31yw, $vf15yw
	vabs.yw		$vf31yw, $vf31yw
	vabs.yz		$vf0yz, $vf0yz
	vabs.yz		$vf0yz, $vf31yz
	vabs.yz		$vf1yz, $vf2yz
	vabs.yz		$vf31yz, $vf0yz
	vabs.yz		$vf31yz, $vf15yz
	vabs.yz		$vf31yz, $vf31yz
	vabs.yzw	$vf0yzw, $vf0yzw
	vabs.yzw	$vf0yzw, $vf31yzw
	vabs.yzw	$vf1yzw, $vf2yzw
	vabs.yzw	$vf31yzw, $vf0yzw
	vabs.yzw	$vf31yzw, $vf15yzw
	vabs.yzw	$vf31yzw, $vf31yzw
	vabs.z		$vf0z, $vf0z
	vabs.z		$vf0z, $vf31z
	vabs.z		$vf1z, $vf2z
	vabs.z		$vf31z, $vf0z
	vabs.z		$vf31z, $vf15z
	vabs.z		$vf31z, $vf31z
	vabs.zw		$vf0zw, $vf0zw
	vabs.zw		$vf0zw, $vf31zw
	vabs.zw		$vf1zw, $vf2zw
	vabs.zw		$vf31zw, $vf0zw
	vabs.zw		$vf31zw, $vf15zw
	vabs.zw		$vf31zw, $vf31zw
	vaddai.w	$ACCw, $vf0w, $I
	vaddai.w	$ACCw, $vf1w, $I
	vaddai.w	$ACCw, $vf31w, $I
	vaddai.x	$ACCx, $vf0x, $I
	vaddai.x	$ACCx, $vf1x, $I
	vaddai.x	$ACCx, $vf31x, $I
	vaddai.xw	$ACCxw, $vf0xw, $I
	vaddai.xw	$ACCxw, $vf1xw, $I
	vaddai.xw	$ACCxw, $vf31xw, $I
	vaddai.xy	$ACCxy, $vf0xy, $I
	vaddai.xy	$ACCxy, $vf1xy, $I
	vaddai.xy	$ACCxy, $vf31xy, $I
	vaddai.xyw	$ACCxyw, $vf0xyw, $I
	vaddai.xyw	$ACCxyw, $vf1xyw, $I
	vaddai.xyw	$ACCxyw, $vf31xyw, $I
	vaddai.xyz	$ACCxyz, $vf0xyz, $I
	vaddai.xyz	$ACCxyz, $vf1xyz, $I
	vaddai.xyz	$ACCxyz, $vf31xyz, $I
	vaddai.xyzw	$ACCxyzw, $vf0xyzw, $I
	vaddai.xyzw	$ACCxyzw, $vf1xyzw, $I
	vaddai.xyzw	$ACCxyzw, $vf31xyzw, $I
	vaddai.xz	$ACCxz, $vf0xz, $I
	vaddai.xz	$ACCxz, $vf1xz, $I
	vaddai.xz	$ACCxz, $vf31xz, $I
	vaddai.xzw	$ACCxzw, $vf0xzw, $I
	vaddai.xzw	$ACCxzw, $vf1xzw, $I
	vaddai.xzw	$ACCxzw, $vf31xzw, $I
	vaddai.y	$ACCy, $vf0y, $I
	vaddai.y	$ACCy, $vf1y, $I
	vaddai.y	$ACCy, $vf31y, $I
	vaddai.yw	$ACCyw, $vf0yw, $I
	vaddai.yw	$ACCyw, $vf1yw, $I
	vaddai.yw	$ACCyw, $vf31yw, $I
	vaddai.yz	$ACCyz, $vf0yz, $I
	vaddai.yz	$ACCyz, $vf1yz, $I
	vaddai.yz	$ACCyz, $vf31yz, $I
	vaddai.yzw	$ACCyzw, $vf0yzw, $I
	vaddai.yzw	$ACCyzw, $vf1yzw, $I
	vaddai.yzw	$ACCyzw, $vf31yzw, $I
	vaddai.z	$ACCz, $vf0z, $I
	vaddai.z	$ACCz, $vf1z, $I
	vaddai.z	$ACCz, $vf31z, $I
	vaddai.zw	$ACCzw, $vf0zw, $I
	vaddai.zw	$ACCzw, $vf1zw, $I
	vaddai.zw	$ACCzw, $vf31zw, $I
	vaddaq.w	$ACCw, $vf0w, $Q
	vaddaq.w	$ACCw, $vf1w, $Q
	vaddaq.w	$ACCw, $vf31w, $Q
	vaddaq.x	$ACCx, $vf0x, $Q
	vaddaq.x	$ACCx, $vf1x, $Q
	vaddaq.x	$ACCx, $vf31x, $Q
	vaddaq.xw	$ACCxw, $vf0xw, $Q
	vaddaq.xw	$ACCxw, $vf1xw, $Q
	vaddaq.xw	$ACCxw, $vf31xw, $Q
	vaddaq.xy	$ACCxy, $vf0xy, $Q
	vaddaq.xy	$ACCxy, $vf1xy, $Q
	vaddaq.xy	$ACCxy, $vf31xy, $Q
	vaddaq.xyw	$ACCxyw, $vf0xyw, $Q
	vaddaq.xyw	$ACCxyw, $vf1xyw, $Q
	vaddaq.xyw	$ACCxyw, $vf31xyw, $Q
	vaddaq.xyz	$ACCxyz, $vf0xyz, $Q
	vaddaq.xyz	$ACCxyz, $vf1xyz, $Q
	vaddaq.xyz	$ACCxyz, $vf31xyz, $Q
	vaddaq.xyzw	$ACCxyzw, $vf0xyzw, $Q
	vaddaq.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vaddaq.xyzw	$ACCxyzw, $vf31xyzw, $Q
	vaddaq.xz	$ACCxz, $vf0xz, $Q
	vaddaq.xz	$ACCxz, $vf1xz, $Q
	vaddaq.xz	$ACCxz, $vf31xz, $Q
	vaddaq.xzw	$ACCxzw, $vf0xzw, $Q
	vaddaq.xzw	$ACCxzw, $vf1xzw, $Q
	vaddaq.xzw	$ACCxzw, $vf31xzw, $Q
	vaddaq.y	$ACCy, $vf0y, $Q
	vaddaq.y	$ACCy, $vf1y, $Q
	vaddaq.y	$ACCy, $vf31y, $Q
	vaddaq.yw	$ACCyw, $vf0yw, $Q
	vaddaq.yw	$ACCyw, $vf1yw, $Q
	vaddaq.yw	$ACCyw, $vf31yw, $Q
	vaddaq.yz	$ACCyz, $vf0yz, $Q
	vaddaq.yz	$ACCyz, $vf1yz, $Q
	vaddaq.yz	$ACCyz, $vf31yz, $Q
	vaddaq.yzw	$ACCyzw, $vf0yzw, $Q
	vaddaq.yzw	$ACCyzw, $vf1yzw, $Q
	vaddaq.yzw	$ACCyzw, $vf31yzw, $Q
	vaddaq.z	$ACCz, $vf0z, $Q
	vaddaq.z	$ACCz, $vf1z, $Q
	vaddaq.z	$ACCz, $vf31z, $Q
	vaddaq.zw	$ACCzw, $vf0zw, $Q
	vaddaq.zw	$ACCzw, $vf1zw, $Q
	vaddaq.zw	$ACCzw, $vf31zw, $Q
	vadda.w		$ACCw, $vf0w, $vf0w
	vadda.w		$ACCw, $vf0w, $vf31w
	vadda.w		$ACCw, $vf1w, $vf2w
	vadda.w		$ACCw, $vf31w, $vf0w
	vadda.w		$ACCw, $vf31w, $vf15w
	vadda.w		$ACCw, $vf31w, $vf31w
	vaddaw.w	$ACCw, $vf0w, $vf0w
	vaddaw.w	$ACCw, $vf0w, $vf31w
	vaddaw.w	$ACCw, $vf1w, $vf2w
	vaddaw.w	$ACCw, $vf31w, $vf0w
	vaddaw.w	$ACCw, $vf31w, $vf15w
	vaddaw.w	$ACCw, $vf31w, $vf31w
	vaddaw.x	$ACCx, $vf0x, $vf0w
	vaddaw.x	$ACCx, $vf0x, $vf31w
	vaddaw.x	$ACCx, $vf1x, $vf2w
	vaddaw.x	$ACCx, $vf31x, $vf0w
	vaddaw.x	$ACCx, $vf31x, $vf15w
	vaddaw.x	$ACCx, $vf31x, $vf31w
	vaddaw.xw	$ACCxw, $vf0xw, $vf0w
	vaddaw.xw	$ACCxw, $vf0xw, $vf31w
	vaddaw.xw	$ACCxw, $vf1xw, $vf2w
	vaddaw.xw	$ACCxw, $vf31xw, $vf0w
	vaddaw.xw	$ACCxw, $vf31xw, $vf15w
	vaddaw.xw	$ACCxw, $vf31xw, $vf31w
	vaddaw.xy	$ACCxy, $vf0xy, $vf0w
	vaddaw.xy	$ACCxy, $vf0xy, $vf31w
	vaddaw.xy	$ACCxy, $vf1xy, $vf2w
	vaddaw.xy	$ACCxy, $vf31xy, $vf0w
	vaddaw.xy	$ACCxy, $vf31xy, $vf15w
	vaddaw.xy	$ACCxy, $vf31xy, $vf31w
	vaddaw.xyw	$ACCxyw, $vf0xyw, $vf0w
	vaddaw.xyw	$ACCxyw, $vf0xyw, $vf31w
	vaddaw.xyw	$ACCxyw, $vf1xyw, $vf2w
	vaddaw.xyw	$ACCxyw, $vf31xyw, $vf0w
	vaddaw.xyw	$ACCxyw, $vf31xyw, $vf15w
	vaddaw.xyw	$ACCxyw, $vf31xyw, $vf31w
	vaddaw.xyz	$ACCxyz, $vf0xyz, $vf0w
	vaddaw.xyz	$ACCxyz, $vf0xyz, $vf31w
	vaddaw.xyz	$ACCxyz, $vf1xyz, $vf2w
	vaddaw.xyz	$ACCxyz, $vf31xyz, $vf0w
	vaddaw.xyz	$ACCxyz, $vf31xyz, $vf15w
	vaddaw.xyz	$ACCxyz, $vf31xyz, $vf31w
	vaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0w
	vaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31w
	vaddaw.xyzw	$ACCxyzw, $vf1xyzw, $vf2w
	vaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf0w
	vaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf15w
	vaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf31w
	vaddaw.xz	$ACCxz, $vf0xz, $vf0w
	vaddaw.xz	$ACCxz, $vf0xz, $vf31w
	vaddaw.xz	$ACCxz, $vf1xz, $vf2w
	vaddaw.xz	$ACCxz, $vf31xz, $vf0w
	vaddaw.xz	$ACCxz, $vf31xz, $vf15w
	vaddaw.xz	$ACCxz, $vf31xz, $vf31w
	vaddaw.xzw	$ACCxzw, $vf0xzw, $vf0w
	vaddaw.xzw	$ACCxzw, $vf0xzw, $vf31w
	vaddaw.xzw	$ACCxzw, $vf1xzw, $vf2w
	vaddaw.xzw	$ACCxzw, $vf31xzw, $vf0w
	vaddaw.xzw	$ACCxzw, $vf31xzw, $vf15w
	vaddaw.xzw	$ACCxzw, $vf31xzw, $vf31w
	vaddaw.y	$ACCy, $vf0y, $vf0w
	vaddaw.y	$ACCy, $vf0y, $vf31w
	vaddaw.y	$ACCy, $vf1y, $vf2w
	vaddaw.y	$ACCy, $vf31y, $vf0w
	vaddaw.y	$ACCy, $vf31y, $vf15w
	vaddaw.y	$ACCy, $vf31y, $vf31w
	vaddaw.yw	$ACCyw, $vf0yw, $vf0w
	vaddaw.yw	$ACCyw, $vf0yw, $vf31w
	vaddaw.yw	$ACCyw, $vf1yw, $vf2w
	vaddaw.yw	$ACCyw, $vf31yw, $vf0w
	vaddaw.yw	$ACCyw, $vf31yw, $vf15w
	vaddaw.yw	$ACCyw, $vf31yw, $vf31w
	vaddaw.yz	$ACCyz, $vf0yz, $vf0w
	vaddaw.yz	$ACCyz, $vf0yz, $vf31w
	vaddaw.yz	$ACCyz, $vf1yz, $vf2w
	vaddaw.yz	$ACCyz, $vf31yz, $vf0w
	vaddaw.yz	$ACCyz, $vf31yz, $vf15w
	vaddaw.yz	$ACCyz, $vf31yz, $vf31w
	vaddaw.yzw	$ACCyzw, $vf0yzw, $vf0w
	vaddaw.yzw	$ACCyzw, $vf0yzw, $vf31w
	vaddaw.yzw	$ACCyzw, $vf1yzw, $vf2w
	vaddaw.yzw	$ACCyzw, $vf31yzw, $vf0w
	vaddaw.yzw	$ACCyzw, $vf31yzw, $vf15w
	vaddaw.yzw	$ACCyzw, $vf31yzw, $vf31w
	vaddaw.z	$ACCz, $vf0z, $vf0w
	vaddaw.z	$ACCz, $vf0z, $vf31w
	vaddaw.z	$ACCz, $vf1z, $vf2w
	vaddaw.z	$ACCz, $vf31z, $vf0w
	vaddaw.z	$ACCz, $vf31z, $vf15w
	vaddaw.z	$ACCz, $vf31z, $vf31w
	vaddaw.zw	$ACCzw, $vf0zw, $vf0w
	vaddaw.zw	$ACCzw, $vf0zw, $vf31w
	vaddaw.zw	$ACCzw, $vf1zw, $vf2w
	vaddaw.zw	$ACCzw, $vf31zw, $vf0w
	vaddaw.zw	$ACCzw, $vf31zw, $vf15w
	vaddaw.zw	$ACCzw, $vf31zw, $vf31w
	vadda.x		$ACCx, $vf0x, $vf0x
	vadda.x		$ACCx, $vf0x, $vf31x
	vadda.x		$ACCx, $vf1x, $vf2x
	vadda.x		$ACCx, $vf31x, $vf0x
	vadda.x		$ACCx, $vf31x, $vf15x
	vadda.x		$ACCx, $vf31x, $vf31x
	vaddax.w	$ACCw, $vf0w, $vf0x
	vaddax.w	$ACCw, $vf0w, $vf31x
	vaddax.w	$ACCw, $vf1w, $vf2x
	vaddax.w	$ACCw, $vf31w, $vf0x
	vaddax.w	$ACCw, $vf31w, $vf15x
	vaddax.w	$ACCw, $vf31w, $vf31x
	vadda.xw	$ACCxw, $vf0xw, $vf0xw
	vadda.xw	$ACCxw, $vf0xw, $vf31xw
	vadda.xw	$ACCxw, $vf1xw, $vf2xw
	vadda.xw	$ACCxw, $vf31xw, $vf0xw
	vadda.xw	$ACCxw, $vf31xw, $vf15xw
	vadda.xw	$ACCxw, $vf31xw, $vf31xw
	vaddax.x	$ACCx, $vf0x, $vf0x
	vaddax.x	$ACCx, $vf0x, $vf31x
	vaddax.x	$ACCx, $vf1x, $vf2x
	vaddax.x	$ACCx, $vf31x, $vf0x
	vaddax.x	$ACCx, $vf31x, $vf15x
	vaddax.x	$ACCx, $vf31x, $vf31x
	vaddax.xw	$ACCxw, $vf0xw, $vf0x
	vaddax.xw	$ACCxw, $vf0xw, $vf31x
	vaddax.xw	$ACCxw, $vf1xw, $vf2x
	vaddax.xw	$ACCxw, $vf31xw, $vf0x
	vaddax.xw	$ACCxw, $vf31xw, $vf15x
	vaddax.xw	$ACCxw, $vf31xw, $vf31x
	vaddax.xy	$ACCxy, $vf0xy, $vf0x
	vaddax.xy	$ACCxy, $vf0xy, $vf31x
	vaddax.xy	$ACCxy, $vf1xy, $vf2x
	vaddax.xy	$ACCxy, $vf31xy, $vf0x
	vaddax.xy	$ACCxy, $vf31xy, $vf15x
	vaddax.xy	$ACCxy, $vf31xy, $vf31x
	vaddax.xyw	$ACCxyw, $vf0xyw, $vf0x
	vaddax.xyw	$ACCxyw, $vf0xyw, $vf31x
	vaddax.xyw	$ACCxyw, $vf1xyw, $vf2x
	vaddax.xyw	$ACCxyw, $vf31xyw, $vf0x
	vaddax.xyw	$ACCxyw, $vf31xyw, $vf15x
	vaddax.xyw	$ACCxyw, $vf31xyw, $vf31x
	vaddax.xyz	$ACCxyz, $vf0xyz, $vf0x
	vaddax.xyz	$ACCxyz, $vf0xyz, $vf31x
	vaddax.xyz	$ACCxyz, $vf1xyz, $vf2x
	vaddax.xyz	$ACCxyz, $vf31xyz, $vf0x
	vaddax.xyz	$ACCxyz, $vf31xyz, $vf15x
	vaddax.xyz	$ACCxyz, $vf31xyz, $vf31x
	vaddax.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vaddax.xyzw	$ACCxyzw, $vf0xyzw, $vf31x
	vaddax.xyzw	$ACCxyzw, $vf1xyzw, $vf2x
	vaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf0x
	vaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf15x
	vaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf31x
	vaddax.xz	$ACCxz, $vf0xz, $vf0x
	vaddax.xz	$ACCxz, $vf0xz, $vf31x
	vaddax.xz	$ACCxz, $vf1xz, $vf2x
	vaddax.xz	$ACCxz, $vf31xz, $vf0x
	vaddax.xz	$ACCxz, $vf31xz, $vf15x
	vaddax.xz	$ACCxz, $vf31xz, $vf31x
	vaddax.xzw	$ACCxzw, $vf0xzw, $vf0x
	vaddax.xzw	$ACCxzw, $vf0xzw, $vf31x
	vaddax.xzw	$ACCxzw, $vf1xzw, $vf2x
	vaddax.xzw	$ACCxzw, $vf31xzw, $vf0x
	vaddax.xzw	$ACCxzw, $vf31xzw, $vf15x
	vaddax.xzw	$ACCxzw, $vf31xzw, $vf31x
	vadda.xy	$ACCxy, $vf0xy, $vf0xy
	vadda.xy	$ACCxy, $vf0xy, $vf31xy
	vadda.xy	$ACCxy, $vf1xy, $vf2xy
	vadda.xy	$ACCxy, $vf31xy, $vf0xy
	vadda.xy	$ACCxy, $vf31xy, $vf15xy
	vadda.xy	$ACCxy, $vf31xy, $vf31xy
	vaddax.y	$ACCy, $vf0y, $vf0x
	vaddax.y	$ACCy, $vf0y, $vf31x
	vaddax.y	$ACCy, $vf1y, $vf2x
	vaddax.y	$ACCy, $vf31y, $vf0x
	vaddax.y	$ACCy, $vf31y, $vf15x
	vaddax.y	$ACCy, $vf31y, $vf31x
	vadda.xyw	$ACCxyw, $vf0xyw, $vf0xyw
	vadda.xyw	$ACCxyw, $vf0xyw, $vf31xyw
	vadda.xyw	$ACCxyw, $vf1xyw, $vf2xyw
	vadda.xyw	$ACCxyw, $vf31xyw, $vf0xyw
	vadda.xyw	$ACCxyw, $vf31xyw, $vf15xyw
	vadda.xyw	$ACCxyw, $vf31xyw, $vf31xyw
	vaddax.yw	$ACCyw, $vf0yw, $vf0x
	vaddax.yw	$ACCyw, $vf0yw, $vf31x
	vaddax.yw	$ACCyw, $vf1yw, $vf2x
	vaddax.yw	$ACCyw, $vf31yw, $vf0x
	vaddax.yw	$ACCyw, $vf31yw, $vf15x
	vaddax.yw	$ACCyw, $vf31yw, $vf31x
	vadda.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vadda.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vadda.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vadda.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vadda.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vadda.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vaddax.yz	$ACCyz, $vf0yz, $vf0x
	vaddax.yz	$ACCyz, $vf0yz, $vf31x
	vaddax.yz	$ACCyz, $vf1yz, $vf2x
	vaddax.yz	$ACCyz, $vf31yz, $vf0x
	vaddax.yz	$ACCyz, $vf31yz, $vf15x
	vaddax.yz	$ACCyz, $vf31yz, $vf31x
	vadda.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vadda.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vadda.xyzw	$ACCxyzw, $vf1xyzw, $vf2xyzw
	vadda.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vadda.xyzw	$ACCxyzw, $vf31xyzw, $vf15xyzw
	vadda.xyzw	$ACCxyzw, $vf31xyzw, $vf31xyzw
	vaddax.yzw	$ACCyzw, $vf0yzw, $vf0x
	vaddax.yzw	$ACCyzw, $vf0yzw, $vf31x
	vaddax.yzw	$ACCyzw, $vf1yzw, $vf2x
	vaddax.yzw	$ACCyzw, $vf31yzw, $vf0x
	vaddax.yzw	$ACCyzw, $vf31yzw, $vf15x
	vaddax.yzw	$ACCyzw, $vf31yzw, $vf31x
	vadda.xz	$ACCxz, $vf0xz, $vf0xz
	vadda.xz	$ACCxz, $vf0xz, $vf31xz
	vadda.xz	$ACCxz, $vf1xz, $vf2xz
	vadda.xz	$ACCxz, $vf31xz, $vf0xz
	vadda.xz	$ACCxz, $vf31xz, $vf15xz
	vadda.xz	$ACCxz, $vf31xz, $vf31xz
	vaddax.z	$ACCz, $vf0z, $vf0x
	vaddax.z	$ACCz, $vf0z, $vf31x
	vaddax.z	$ACCz, $vf1z, $vf2x
	vaddax.z	$ACCz, $vf31z, $vf0x
	vaddax.z	$ACCz, $vf31z, $vf15x
	vaddax.z	$ACCz, $vf31z, $vf31x
	vadda.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vadda.xzw	$ACCxzw, $vf0xzw, $vf31xzw
	vadda.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vadda.xzw	$ACCxzw, $vf31xzw, $vf0xzw
	vadda.xzw	$ACCxzw, $vf31xzw, $vf15xzw
	vadda.xzw	$ACCxzw, $vf31xzw, $vf31xzw
	vaddax.zw	$ACCzw, $vf0zw, $vf0x
	vaddax.zw	$ACCzw, $vf0zw, $vf31x
	vaddax.zw	$ACCzw, $vf1zw, $vf2x
	vaddax.zw	$ACCzw, $vf31zw, $vf0x
	vaddax.zw	$ACCzw, $vf31zw, $vf15x
	vaddax.zw	$ACCzw, $vf31zw, $vf31x
	vadda.y		$ACCy, $vf0y, $vf0y
	vadda.y		$ACCy, $vf0y, $vf31y
	vadda.y		$ACCy, $vf1y, $vf2y
	vadda.y		$ACCy, $vf31y, $vf0y
	vadda.y		$ACCy, $vf31y, $vf15y
	vadda.y		$ACCy, $vf31y, $vf31y
	vadday.w	$ACCw, $vf0w, $vf0y
	vadday.w	$ACCw, $vf0w, $vf31y
	vadday.w	$ACCw, $vf1w, $vf2y
	vadday.w	$ACCw, $vf31w, $vf0y
	vadday.w	$ACCw, $vf31w, $vf15y
	vadday.w	$ACCw, $vf31w, $vf31y
	vadda.yw	$ACCyw, $vf0yw, $vf0yw
	vadda.yw	$ACCyw, $vf0yw, $vf31yw
	vadda.yw	$ACCyw, $vf1yw, $vf2yw
	vadda.yw	$ACCyw, $vf31yw, $vf0yw
	vadda.yw	$ACCyw, $vf31yw, $vf15yw
	vadda.yw	$ACCyw, $vf31yw, $vf31yw
	vadday.x	$ACCx, $vf0x, $vf0y
	vadday.x	$ACCx, $vf0x, $vf31y
	vadday.x	$ACCx, $vf1x, $vf2y
	vadday.x	$ACCx, $vf31x, $vf0y
	vadday.x	$ACCx, $vf31x, $vf15y
	vadday.x	$ACCx, $vf31x, $vf31y
	vadday.xw	$ACCxw, $vf0xw, $vf0y
	vadday.xw	$ACCxw, $vf0xw, $vf31y
	vadday.xw	$ACCxw, $vf1xw, $vf2y
	vadday.xw	$ACCxw, $vf31xw, $vf0y
	vadday.xw	$ACCxw, $vf31xw, $vf15y
	vadday.xw	$ACCxw, $vf31xw, $vf31y
	vadday.xy	$ACCxy, $vf0xy, $vf0y
	vadday.xy	$ACCxy, $vf0xy, $vf31y
	vadday.xy	$ACCxy, $vf1xy, $vf2y
	vadday.xy	$ACCxy, $vf31xy, $vf0y
	vadday.xy	$ACCxy, $vf31xy, $vf15y
	vadday.xy	$ACCxy, $vf31xy, $vf31y
	vadday.xyw	$ACCxyw, $vf0xyw, $vf0y
	vadday.xyw	$ACCxyw, $vf0xyw, $vf31y
	vadday.xyw	$ACCxyw, $vf1xyw, $vf2y
	vadday.xyw	$ACCxyw, $vf31xyw, $vf0y
	vadday.xyw	$ACCxyw, $vf31xyw, $vf15y
	vadday.xyw	$ACCxyw, $vf31xyw, $vf31y
	vadday.xyz	$ACCxyz, $vf0xyz, $vf0y
	vadday.xyz	$ACCxyz, $vf0xyz, $vf31y
	vadday.xyz	$ACCxyz, $vf1xyz, $vf2y
	vadday.xyz	$ACCxyz, $vf31xyz, $vf0y
	vadday.xyz	$ACCxyz, $vf31xyz, $vf15y
	vadday.xyz	$ACCxyz, $vf31xyz, $vf31y
	vadday.xyzw	$ACCxyzw, $vf0xyzw, $vf0y
	vadday.xyzw	$ACCxyzw, $vf0xyzw, $vf31y
	vadday.xyzw	$ACCxyzw, $vf1xyzw, $vf2y
	vadday.xyzw	$ACCxyzw, $vf31xyzw, $vf0y
	vadday.xyzw	$ACCxyzw, $vf31xyzw, $vf15y
	vadday.xyzw	$ACCxyzw, $vf31xyzw, $vf31y
	vadday.xz	$ACCxz, $vf0xz, $vf0y
	vadday.xz	$ACCxz, $vf0xz, $vf31y
	vadday.xz	$ACCxz, $vf1xz, $vf2y
	vadday.xz	$ACCxz, $vf31xz, $vf0y
	vadday.xz	$ACCxz, $vf31xz, $vf15y
	vadday.xz	$ACCxz, $vf31xz, $vf31y
	vadday.xzw	$ACCxzw, $vf0xzw, $vf0y
	vadday.xzw	$ACCxzw, $vf0xzw, $vf31y
	vadday.xzw	$ACCxzw, $vf1xzw, $vf2y
	vadday.xzw	$ACCxzw, $vf31xzw, $vf0y
	vadday.xzw	$ACCxzw, $vf31xzw, $vf15y
	vadday.xzw	$ACCxzw, $vf31xzw, $vf31y
	vadday.y	$ACCy, $vf0y, $vf0y
	vadday.y	$ACCy, $vf0y, $vf31y
	vadday.y	$ACCy, $vf1y, $vf2y
	vadday.y	$ACCy, $vf31y, $vf0y
	vadday.y	$ACCy, $vf31y, $vf15y
	vadday.y	$ACCy, $vf31y, $vf31y
	vadday.yw	$ACCyw, $vf0yw, $vf0y
	vadday.yw	$ACCyw, $vf0yw, $vf31y
	vadday.yw	$ACCyw, $vf1yw, $vf2y
	vadday.yw	$ACCyw, $vf31yw, $vf0y
	vadday.yw	$ACCyw, $vf31yw, $vf15y
	vadday.yw	$ACCyw, $vf31yw, $vf31y
	vadday.yz	$ACCyz, $vf0yz, $vf0y
	vadday.yz	$ACCyz, $vf0yz, $vf31y
	vadday.yz	$ACCyz, $vf1yz, $vf2y
	vadday.yz	$ACCyz, $vf31yz, $vf0y
	vadday.yz	$ACCyz, $vf31yz, $vf15y
	vadday.yz	$ACCyz, $vf31yz, $vf31y
	vadday.yzw	$ACCyzw, $vf0yzw, $vf0y
	vadday.yzw	$ACCyzw, $vf0yzw, $vf31y
	vadday.yzw	$ACCyzw, $vf1yzw, $vf2y
	vadday.yzw	$ACCyzw, $vf31yzw, $vf0y
	vadday.yzw	$ACCyzw, $vf31yzw, $vf15y
	vadday.yzw	$ACCyzw, $vf31yzw, $vf31y
	vadda.yz	$ACCyz, $vf0yz, $vf0yz
	vadda.yz	$ACCyz, $vf0yz, $vf31yz
	vadda.yz	$ACCyz, $vf1yz, $vf2yz
	vadda.yz	$ACCyz, $vf31yz, $vf0yz
	vadda.yz	$ACCyz, $vf31yz, $vf15yz
	vadda.yz	$ACCyz, $vf31yz, $vf31yz
	vadday.z	$ACCz, $vf0z, $vf0y
	vadday.z	$ACCz, $vf0z, $vf31y
	vadday.z	$ACCz, $vf1z, $vf2y
	vadday.z	$ACCz, $vf31z, $vf0y
	vadday.z	$ACCz, $vf31z, $vf15y
	vadday.z	$ACCz, $vf31z, $vf31y
	vadda.yzw	$ACCyzw, $vf0yzw, $vf0yzw
	vadda.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vadda.yzw	$ACCyzw, $vf1yzw, $vf2yzw
	vadda.yzw	$ACCyzw, $vf31yzw, $vf0yzw
	vadda.yzw	$ACCyzw, $vf31yzw, $vf15yzw
	vadda.yzw	$ACCyzw, $vf31yzw, $vf31yzw
	vadday.zw	$ACCzw, $vf0zw, $vf0y
	vadday.zw	$ACCzw, $vf0zw, $vf31y
	vadday.zw	$ACCzw, $vf1zw, $vf2y
	vadday.zw	$ACCzw, $vf31zw, $vf0y
	vadday.zw	$ACCzw, $vf31zw, $vf15y
	vadday.zw	$ACCzw, $vf31zw, $vf31y
	vadda.z		$ACCz, $vf0z, $vf0z
	vadda.z		$ACCz, $vf0z, $vf31z
	vadda.z		$ACCz, $vf1z, $vf2z
	vadda.z		$ACCz, $vf31z, $vf0z
	vadda.z		$ACCz, $vf31z, $vf15z
	vadda.z		$ACCz, $vf31z, $vf31z
	vaddaz.w	$ACCw, $vf0w, $vf0z
	vaddaz.w	$ACCw, $vf0w, $vf31z
	vaddaz.w	$ACCw, $vf1w, $vf2z
	vaddaz.w	$ACCw, $vf31w, $vf0z
	vaddaz.w	$ACCw, $vf31w, $vf15z
	vaddaz.w	$ACCw, $vf31w, $vf31z
	vadda.zw	$ACCzw, $vf0zw, $vf0zw
	vadda.zw	$ACCzw, $vf0zw, $vf31zw
	vadda.zw	$ACCzw, $vf1zw, $vf2zw
	vadda.zw	$ACCzw, $vf31zw, $vf0zw
	vadda.zw	$ACCzw, $vf31zw, $vf15zw
	vadda.zw	$ACCzw, $vf31zw, $vf31zw
	vaddaz.x	$ACCx, $vf0x, $vf0z
	vaddaz.x	$ACCx, $vf0x, $vf31z
	vaddaz.x	$ACCx, $vf1x, $vf2z
	vaddaz.x	$ACCx, $vf31x, $vf0z
	vaddaz.x	$ACCx, $vf31x, $vf15z
	vaddaz.x	$ACCx, $vf31x, $vf31z
	vaddaz.xw	$ACCxw, $vf0xw, $vf0z
	vaddaz.xw	$ACCxw, $vf0xw, $vf31z
	vaddaz.xw	$ACCxw, $vf1xw, $vf2z
	vaddaz.xw	$ACCxw, $vf31xw, $vf0z
	vaddaz.xw	$ACCxw, $vf31xw, $vf15z
	vaddaz.xw	$ACCxw, $vf31xw, $vf31z
	vaddaz.xy	$ACCxy, $vf0xy, $vf0z
	vaddaz.xy	$ACCxy, $vf0xy, $vf31z
	vaddaz.xy	$ACCxy, $vf1xy, $vf2z
	vaddaz.xy	$ACCxy, $vf31xy, $vf0z
	vaddaz.xy	$ACCxy, $vf31xy, $vf15z
	vaddaz.xy	$ACCxy, $vf31xy, $vf31z
	vaddaz.xyw	$ACCxyw, $vf0xyw, $vf0z
	vaddaz.xyw	$ACCxyw, $vf0xyw, $vf31z
	vaddaz.xyw	$ACCxyw, $vf1xyw, $vf2z
	vaddaz.xyw	$ACCxyw, $vf31xyw, $vf0z
	vaddaz.xyw	$ACCxyw, $vf31xyw, $vf15z
	vaddaz.xyw	$ACCxyw, $vf31xyw, $vf31z
	vaddaz.xyz	$ACCxyz, $vf0xyz, $vf0z
	vaddaz.xyz	$ACCxyz, $vf0xyz, $vf31z
	vaddaz.xyz	$ACCxyz, $vf1xyz, $vf2z
	vaddaz.xyz	$ACCxyz, $vf31xyz, $vf0z
	vaddaz.xyz	$ACCxyz, $vf31xyz, $vf15z
	vaddaz.xyz	$ACCxyz, $vf31xyz, $vf31z
	vaddaz.xyzw	$ACCxyzw, $vf0xyzw, $vf0z
	vaddaz.xyzw	$ACCxyzw, $vf0xyzw, $vf31z
	vaddaz.xyzw	$ACCxyzw, $vf1xyzw, $vf2z
	vaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf0z
	vaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf15z
	vaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf31z
	vaddaz.xz	$ACCxz, $vf0xz, $vf0z
	vaddaz.xz	$ACCxz, $vf0xz, $vf31z
	vaddaz.xz	$ACCxz, $vf1xz, $vf2z
	vaddaz.xz	$ACCxz, $vf31xz, $vf0z
	vaddaz.xz	$ACCxz, $vf31xz, $vf15z
	vaddaz.xz	$ACCxz, $vf31xz, $vf31z
	vaddaz.xzw	$ACCxzw, $vf0xzw, $vf0z
	vaddaz.xzw	$ACCxzw, $vf0xzw, $vf31z
	vaddaz.xzw	$ACCxzw, $vf1xzw, $vf2z
	vaddaz.xzw	$ACCxzw, $vf31xzw, $vf0z
	vaddaz.xzw	$ACCxzw, $vf31xzw, $vf15z
	vaddaz.xzw	$ACCxzw, $vf31xzw, $vf31z
	vaddaz.y	$ACCy, $vf0y, $vf0z
	vaddaz.y	$ACCy, $vf0y, $vf31z
	vaddaz.y	$ACCy, $vf1y, $vf2z
	vaddaz.y	$ACCy, $vf31y, $vf0z
	vaddaz.y	$ACCy, $vf31y, $vf15z
	vaddaz.y	$ACCy, $vf31y, $vf31z
	vaddaz.yw	$ACCyw, $vf0yw, $vf0z
	vaddaz.yw	$ACCyw, $vf0yw, $vf31z
	vaddaz.yw	$ACCyw, $vf1yw, $vf2z
	vaddaz.yw	$ACCyw, $vf31yw, $vf0z
	vaddaz.yw	$ACCyw, $vf31yw, $vf15z
	vaddaz.yw	$ACCyw, $vf31yw, $vf31z
	vaddaz.yz	$ACCyz, $vf0yz, $vf0z
	vaddaz.yz	$ACCyz, $vf0yz, $vf31z
	vaddaz.yz	$ACCyz, $vf1yz, $vf2z
	vaddaz.yz	$ACCyz, $vf31yz, $vf0z
	vaddaz.yz	$ACCyz, $vf31yz, $vf15z
	vaddaz.yz	$ACCyz, $vf31yz, $vf31z
	vaddaz.yzw	$ACCyzw, $vf0yzw, $vf0z
	vaddaz.yzw	$ACCyzw, $vf0yzw, $vf31z
	vaddaz.yzw	$ACCyzw, $vf1yzw, $vf2z
	vaddaz.yzw	$ACCyzw, $vf31yzw, $vf0z
	vaddaz.yzw	$ACCyzw, $vf31yzw, $vf15z
	vaddaz.yzw	$ACCyzw, $vf31yzw, $vf31z
	vaddaz.z	$ACCz, $vf0z, $vf0z
	vaddaz.z	$ACCz, $vf0z, $vf31z
	vaddaz.z	$ACCz, $vf1z, $vf2z
	vaddaz.z	$ACCz, $vf31z, $vf0z
	vaddaz.z	$ACCz, $vf31z, $vf15z
	vaddaz.z	$ACCz, $vf31z, $vf31z
	vaddaz.zw	$ACCzw, $vf0zw, $vf0z
	vaddaz.zw	$ACCzw, $vf0zw, $vf31z
	vaddaz.zw	$ACCzw, $vf1zw, $vf2z
	vaddaz.zw	$ACCzw, $vf31zw, $vf0z
	vaddaz.zw	$ACCzw, $vf31zw, $vf15z
	vaddaz.zw	$ACCzw, $vf31zw, $vf31z
	vaddi.w		$vf0w, $vf0w, $I
	vaddi.w		$vf0w, $vf31w, $I
	vaddi.w		$vf1w, $vf2w, $I
	vaddi.w		$vf31w, $vf0w, $I
	vaddi.w		$vf31w, $vf15w, $I
	vaddi.w		$vf31w, $vf31w, $I
	vaddi.x		$vf0x, $vf0x, $I
	vaddi.x		$vf0x, $vf31x, $I
	vaddi.x		$vf1x, $vf2x, $I
	vaddi.x		$vf31x, $vf0x, $I
	vaddi.x		$vf31x, $vf15x, $I
	vaddi.x		$vf31x, $vf31x, $I
	vaddi.xw	$vf0xw, $vf0xw, $I
	vaddi.xw	$vf0xw, $vf31xw, $I
	vaddi.xw	$vf1xw, $vf2xw, $I
	vaddi.xw	$vf31xw, $vf0xw, $I
	vaddi.xw	$vf31xw, $vf15xw, $I
	vaddi.xw	$vf31xw, $vf31xw, $I
	vaddi.xy	$vf0xy, $vf0xy, $I
	vaddi.xy	$vf0xy, $vf31xy, $I
	vaddi.xy	$vf1xy, $vf2xy, $I
	vaddi.xy	$vf31xy, $vf0xy, $I
	vaddi.xy	$vf31xy, $vf15xy, $I
	vaddi.xy	$vf31xy, $vf31xy, $I
	vaddi.xyw	$vf0xyw, $vf0xyw, $I
	vaddi.xyw	$vf0xyw, $vf31xyw, $I
	vaddi.xyw	$vf1xyw, $vf2xyw, $I
	vaddi.xyw	$vf31xyw, $vf0xyw, $I
	vaddi.xyw	$vf31xyw, $vf15xyw, $I
	vaddi.xyw	$vf31xyw, $vf31xyw, $I
	vaddi.xyz	$vf0xyz, $vf0xyz, $I
	vaddi.xyz	$vf0xyz, $vf31xyz, $I
	vaddi.xyz	$vf1xyz, $vf2xyz, $I
	vaddi.xyz	$vf31xyz, $vf0xyz, $I
	vaddi.xyz	$vf31xyz, $vf15xyz, $I
	vaddi.xyz	$vf31xyz, $vf31xyz, $I
	vaddi.xyzw	$vf0xyzw, $vf0xyzw, $I
	vaddi.xyzw	$vf0xyzw, $vf31xyzw, $I
	vaddi.xyzw	$vf1xyzw, $vf2xyzw, $I
	vaddi.xyzw	$vf31xyzw, $vf0xyzw, $I
	vaddi.xyzw	$vf31xyzw, $vf15xyzw, $I
	vaddi.xyzw	$vf31xyzw, $vf31xyzw, $I
	vaddi.xz	$vf0xz, $vf0xz, $I
	vaddi.xz	$vf0xz, $vf31xz, $I
	vaddi.xz	$vf1xz, $vf2xz, $I
	vaddi.xz	$vf31xz, $vf0xz, $I
	vaddi.xz	$vf31xz, $vf15xz, $I
	vaddi.xz	$vf31xz, $vf31xz, $I
	vaddi.xzw	$vf0xzw, $vf0xzw, $I
	vaddi.xzw	$vf0xzw, $vf31xzw, $I
	vaddi.xzw	$vf1xzw, $vf2xzw, $I
	vaddi.xzw	$vf31xzw, $vf0xzw, $I
	vaddi.xzw	$vf31xzw, $vf15xzw, $I
	vaddi.xzw	$vf31xzw, $vf31xzw, $I
	vaddi.y		$vf0y, $vf0y, $I
	vaddi.y		$vf0y, $vf31y, $I
	vaddi.y		$vf1y, $vf2y, $I
	vaddi.y		$vf31y, $vf0y, $I
	vaddi.y		$vf31y, $vf15y, $I
	vaddi.y		$vf31y, $vf31y, $I
	vaddi.yw	$vf0yw, $vf0yw, $I
	vaddi.yw	$vf0yw, $vf31yw, $I
	vaddi.yw	$vf1yw, $vf2yw, $I
	vaddi.yw	$vf31yw, $vf0yw, $I
	vaddi.yw	$vf31yw, $vf15yw, $I
	vaddi.yw	$vf31yw, $vf31yw, $I
	vaddi.yz	$vf0yz, $vf0yz, $I
	vaddi.yz	$vf0yz, $vf31yz, $I
	vaddi.yz	$vf1yz, $vf2yz, $I
	vaddi.yz	$vf31yz, $vf0yz, $I
	vaddi.yz	$vf31yz, $vf15yz, $I
	vaddi.yz	$vf31yz, $vf31yz, $I
	vaddi.yzw	$vf0yzw, $vf0yzw, $I
	vaddi.yzw	$vf0yzw, $vf31yzw, $I
	vaddi.yzw	$vf1yzw, $vf2yzw, $I
	vaddi.yzw	$vf31yzw, $vf0yzw, $I
	vaddi.yzw	$vf31yzw, $vf15yzw, $I
	vaddi.yzw	$vf31yzw, $vf31yzw, $I
	vaddi.z		$vf0z, $vf0z, $I
	vaddi.z		$vf0z, $vf31z, $I
	vaddi.z		$vf1z, $vf2z, $I
	vaddi.z		$vf31z, $vf0z, $I
	vaddi.z		$vf31z, $vf15z, $I
	vaddi.z		$vf31z, $vf31z, $I
	vaddi.zw	$vf0zw, $vf0zw, $I
	vaddi.zw	$vf0zw, $vf31zw, $I
	vaddi.zw	$vf1zw, $vf2zw, $I
	vaddi.zw	$vf31zw, $vf0zw, $I
	vaddi.zw	$vf31zw, $vf15zw, $I
	vaddi.zw	$vf31zw, $vf31zw, $I
	vaddq.w		$vf0w, $vf0w, $Q
	vaddq.w		$vf0w, $vf31w, $Q
	vaddq.w		$vf1w, $vf2w, $Q
	vaddq.w		$vf31w, $vf0w, $Q
	vaddq.w		$vf31w, $vf15w, $Q
	vaddq.w		$vf31w, $vf31w, $Q
	vaddq.x		$vf0x, $vf0x, $Q
	vaddq.x		$vf0x, $vf31x, $Q
	vaddq.x		$vf1x, $vf2x, $Q
	vaddq.x		$vf31x, $vf0x, $Q
	vaddq.x		$vf31x, $vf15x, $Q
	vaddq.x		$vf31x, $vf31x, $Q
	vaddq.xw	$vf0xw, $vf0xw, $Q
	vaddq.xw	$vf0xw, $vf31xw, $Q
	vaddq.xw	$vf1xw, $vf2xw, $Q
	vaddq.xw	$vf31xw, $vf0xw, $Q
	vaddq.xw	$vf31xw, $vf15xw, $Q
	vaddq.xw	$vf31xw, $vf31xw, $Q
	vaddq.xy	$vf0xy, $vf0xy, $Q
	vaddq.xy	$vf0xy, $vf31xy, $Q
	vaddq.xy	$vf1xy, $vf2xy, $Q
	vaddq.xy	$vf31xy, $vf0xy, $Q
	vaddq.xy	$vf31xy, $vf15xy, $Q
	vaddq.xy	$vf31xy, $vf31xy, $Q
	vaddq.xyw	$vf0xyw, $vf0xyw, $Q
	vaddq.xyw	$vf0xyw, $vf31xyw, $Q
	vaddq.xyw	$vf1xyw, $vf2xyw, $Q
	vaddq.xyw	$vf31xyw, $vf0xyw, $Q
	vaddq.xyw	$vf31xyw, $vf15xyw, $Q
	vaddq.xyw	$vf31xyw, $vf31xyw, $Q
	vaddq.xyz	$vf0xyz, $vf0xyz, $Q
	vaddq.xyz	$vf0xyz, $vf31xyz, $Q
	vaddq.xyz	$vf1xyz, $vf2xyz, $Q
	vaddq.xyz	$vf31xyz, $vf0xyz, $Q
	vaddq.xyz	$vf31xyz, $vf15xyz, $Q
	vaddq.xyz	$vf31xyz, $vf31xyz, $Q
	vaddq.xyzw	$vf0xyzw, $vf0xyzw, $Q
	vaddq.xyzw	$vf0xyzw, $vf31xyzw, $Q
	vaddq.xyzw	$vf1xyzw, $vf2xyzw, $Q
	vaddq.xyzw	$vf31xyzw, $vf0xyzw, $Q
	vaddq.xyzw	$vf31xyzw, $vf15xyzw, $Q
	vaddq.xyzw	$vf31xyzw, $vf31xyzw, $Q
	vaddq.xz	$vf0xz, $vf0xz, $Q
	vaddq.xz	$vf0xz, $vf31xz, $Q
	vaddq.xz	$vf1xz, $vf2xz, $Q
	vaddq.xz	$vf31xz, $vf0xz, $Q
	vaddq.xz	$vf31xz, $vf15xz, $Q
	vaddq.xz	$vf31xz, $vf31xz, $Q
	vaddq.xzw	$vf0xzw, $vf0xzw, $Q
	vaddq.xzw	$vf0xzw, $vf31xzw, $Q
	vaddq.xzw	$vf1xzw, $vf2xzw, $Q
	vaddq.xzw	$vf31xzw, $vf0xzw, $Q
	vaddq.xzw	$vf31xzw, $vf15xzw, $Q
	vaddq.xzw	$vf31xzw, $vf31xzw, $Q
	vaddq.y		$vf0y, $vf0y, $Q
	vaddq.y		$vf0y, $vf31y, $Q
	vaddq.y		$vf1y, $vf2y, $Q
	vaddq.y		$vf31y, $vf0y, $Q
	vaddq.y		$vf31y, $vf15y, $Q
	vaddq.y		$vf31y, $vf31y, $Q
	vaddq.yw	$vf0yw, $vf0yw, $Q
	vaddq.yw	$vf0yw, $vf31yw, $Q
	vaddq.yw	$vf1yw, $vf2yw, $Q
	vaddq.yw	$vf31yw, $vf0yw, $Q
	vaddq.yw	$vf31yw, $vf15yw, $Q
	vaddq.yw	$vf31yw, $vf31yw, $Q
	vaddq.yz	$vf0yz, $vf0yz, $Q
	vaddq.yz	$vf0yz, $vf31yz, $Q
	vaddq.yz	$vf1yz, $vf2yz, $Q
	vaddq.yz	$vf31yz, $vf0yz, $Q
	vaddq.yz	$vf31yz, $vf15yz, $Q
	vaddq.yz	$vf31yz, $vf31yz, $Q
	vaddq.yzw	$vf0yzw, $vf0yzw, $Q
	vaddq.yzw	$vf0yzw, $vf31yzw, $Q
	vaddq.yzw	$vf1yzw, $vf2yzw, $Q
	vaddq.yzw	$vf31yzw, $vf0yzw, $Q
	vaddq.yzw	$vf31yzw, $vf15yzw, $Q
	vaddq.yzw	$vf31yzw, $vf31yzw, $Q
	vaddq.z		$vf0z, $vf0z, $Q
	vaddq.z		$vf0z, $vf31z, $Q
	vaddq.z		$vf1z, $vf2z, $Q
	vaddq.z		$vf31z, $vf0z, $Q
	vaddq.z		$vf31z, $vf15z, $Q
	vaddq.z		$vf31z, $vf31z, $Q
	vaddq.zw	$vf0zw, $vf0zw, $Q
	vaddq.zw	$vf0zw, $vf31zw, $Q
	vaddq.zw	$vf1zw, $vf2zw, $Q
	vaddq.zw	$vf31zw, $vf0zw, $Q
	vaddq.zw	$vf31zw, $vf15zw, $Q
	vaddq.zw	$vf31zw, $vf31zw, $Q
	vadd.w		$vf0w, $vf0w, $vf0w
	vadd.w		$vf0w, $vf0w, $vf31w
	vadd.w		$vf0w, $vf31w, $vf0w
	vadd.w		$vf1w, $vf2w, $vf3w
	vadd.w		$vf31w, $vf0w, $vf0w
	vadd.w		$vf31w, $vf15w, $vf7w
	vadd.w		$vf31w, $vf31w, $vf31w
	vaddw.w		$vf0w, $vf0w, $vf0w
	vaddw.w		$vf0w, $vf0w, $vf31w
	vaddw.w		$vf0w, $vf31w, $vf0w
	vaddw.w		$vf1w, $vf2w, $vf3w
	vaddw.w		$vf31w, $vf0w, $vf0w
	vaddw.w		$vf31w, $vf15w, $vf7w
	vaddw.w		$vf31w, $vf31w, $vf31w
	vaddw.x		$vf0x, $vf0x, $vf0w
	vaddw.x		$vf0x, $vf0x, $vf31w
	vaddw.x		$vf0x, $vf31x, $vf0w
	vaddw.x		$vf1x, $vf2x, $vf3w
	vaddw.x		$vf31x, $vf0x, $vf0w
	vaddw.x		$vf31x, $vf15x, $vf7w
	vaddw.x		$vf31x, $vf31x, $vf31w
	vaddw.xw	$vf0xw, $vf0xw, $vf0w
	vaddw.xw	$vf0xw, $vf0xw, $vf31w
	vaddw.xw	$vf0xw, $vf31xw, $vf0w
	vaddw.xw	$vf1xw, $vf2xw, $vf3w
	vaddw.xw	$vf31xw, $vf0xw, $vf0w
	vaddw.xw	$vf31xw, $vf15xw, $vf7w
	vaddw.xw	$vf31xw, $vf31xw, $vf31w
	vaddw.xy	$vf0xy, $vf0xy, $vf0w
	vaddw.xy	$vf0xy, $vf0xy, $vf31w
	vaddw.xy	$vf0xy, $vf31xy, $vf0w
	vaddw.xy	$vf1xy, $vf2xy, $vf3w
	vaddw.xy	$vf31xy, $vf0xy, $vf0w
	vaddw.xy	$vf31xy, $vf15xy, $vf7w
	vaddw.xy	$vf31xy, $vf31xy, $vf31w
	vaddw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vaddw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vaddw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vaddw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vaddw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vaddw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vaddw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vaddw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vaddw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vaddw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vaddw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vaddw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vaddw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vaddw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vaddw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vaddw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vaddw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vaddw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vaddw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vaddw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vaddw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vaddw.xz	$vf0xz, $vf0xz, $vf0w
	vaddw.xz	$vf0xz, $vf0xz, $vf31w
	vaddw.xz	$vf0xz, $vf31xz, $vf0w
	vaddw.xz	$vf1xz, $vf2xz, $vf3w
	vaddw.xz	$vf31xz, $vf0xz, $vf0w
	vaddw.xz	$vf31xz, $vf15xz, $vf7w
	vaddw.xz	$vf31xz, $vf31xz, $vf31w
	vaddw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vaddw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vaddw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vaddw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vaddw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vaddw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vaddw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vaddw.y		$vf0y, $vf0y, $vf0w
	vaddw.y		$vf0y, $vf0y, $vf31w
	vaddw.y		$vf0y, $vf31y, $vf0w
	vaddw.y		$vf1y, $vf2y, $vf3w
	vaddw.y		$vf31y, $vf0y, $vf0w
	vaddw.y		$vf31y, $vf15y, $vf7w
	vaddw.y		$vf31y, $vf31y, $vf31w
	vaddw.yw	$vf0yw, $vf0yw, $vf0w
	vaddw.yw	$vf0yw, $vf0yw, $vf31w
	vaddw.yw	$vf0yw, $vf31yw, $vf0w
	vaddw.yw	$vf1yw, $vf2yw, $vf3w
	vaddw.yw	$vf31yw, $vf0yw, $vf0w
	vaddw.yw	$vf31yw, $vf15yw, $vf7w
	vaddw.yw	$vf31yw, $vf31yw, $vf31w
	vaddw.yz	$vf0yz, $vf0yz, $vf0w
	vaddw.yz	$vf0yz, $vf0yz, $vf31w
	vaddw.yz	$vf0yz, $vf31yz, $vf0w
	vaddw.yz	$vf1yz, $vf2yz, $vf3w
	vaddw.yz	$vf31yz, $vf0yz, $vf0w
	vaddw.yz	$vf31yz, $vf15yz, $vf7w
	vaddw.yz	$vf31yz, $vf31yz, $vf31w
	vaddw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vaddw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vaddw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vaddw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vaddw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vaddw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vaddw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vaddw.z		$vf0z, $vf0z, $vf0w
	vaddw.z		$vf0z, $vf0z, $vf31w
	vaddw.z		$vf0z, $vf31z, $vf0w
	vaddw.z		$vf1z, $vf2z, $vf3w
	vaddw.z		$vf31z, $vf0z, $vf0w
	vaddw.z		$vf31z, $vf15z, $vf7w
	vaddw.z		$vf31z, $vf31z, $vf31w
	vaddw.zw	$vf0zw, $vf0zw, $vf0w
	vaddw.zw	$vf0zw, $vf0zw, $vf31w
	vaddw.zw	$vf0zw, $vf31zw, $vf0w
	vaddw.zw	$vf1zw, $vf2zw, $vf3w
	vaddw.zw	$vf31zw, $vf0zw, $vf0w
	vaddw.zw	$vf31zw, $vf15zw, $vf7w
	vaddw.zw	$vf31zw, $vf31zw, $vf31w
	vadd.x		$vf0x, $vf0x, $vf0x
	vadd.x		$vf0x, $vf0x, $vf31x
	vadd.x		$vf0x, $vf31x, $vf0x
	vadd.x		$vf1x, $vf2x, $vf3x
	vadd.x		$vf31x, $vf0x, $vf0x
	vadd.x		$vf31x, $vf15x, $vf7x
	vadd.x		$vf31x, $vf31x, $vf31x
	vaddx.w		$vf0w, $vf0w, $vf0x
	vaddx.w		$vf0w, $vf0w, $vf31x
	vaddx.w		$vf0w, $vf31w, $vf0x
	vadd.xw		$vf0xw, $vf0xw, $vf0xw
	vadd.xw		$vf0xw, $vf0xw, $vf31xw
	vadd.xw		$vf0xw, $vf31xw, $vf0xw
	vaddx.w		$vf1w, $vf2w, $vf3x
	vadd.xw		$vf1xw, $vf2xw, $vf3xw
	vaddx.w		$vf31w, $vf0w, $vf0x
	vaddx.w		$vf31w, $vf15w, $vf7x
	vaddx.w		$vf31w, $vf31w, $vf31x
	vadd.xw		$vf31xw, $vf0xw, $vf0xw
	vadd.xw		$vf31xw, $vf15xw, $vf7xw
	vadd.xw		$vf31xw, $vf31xw, $vf31xw
	vaddx.x		$vf0x, $vf0x, $vf0x
	vaddx.x		$vf0x, $vf0x, $vf31x
	vaddx.x		$vf0x, $vf31x, $vf0x
	vaddx.x		$vf1x, $vf2x, $vf3x
	vaddx.x		$vf31x, $vf0x, $vf0x
	vaddx.x		$vf31x, $vf15x, $vf7x
	vaddx.x		$vf31x, $vf31x, $vf31x
	vaddx.xw	$vf0xw, $vf0xw, $vf0x
	vaddx.xw	$vf0xw, $vf0xw, $vf31x
	vaddx.xw	$vf0xw, $vf31xw, $vf0x
	vaddx.xw	$vf1xw, $vf2xw, $vf3x
	vaddx.xw	$vf31xw, $vf0xw, $vf0x
	vaddx.xw	$vf31xw, $vf15xw, $vf7x
	vaddx.xw	$vf31xw, $vf31xw, $vf31x
	vaddx.xy	$vf0xy, $vf0xy, $vf0x
	vaddx.xy	$vf0xy, $vf0xy, $vf31x
	vaddx.xy	$vf0xy, $vf31xy, $vf0x
	vaddx.xy	$vf1xy, $vf2xy, $vf3x
	vaddx.xy	$vf31xy, $vf0xy, $vf0x
	vaddx.xy	$vf31xy, $vf15xy, $vf7x
	vaddx.xy	$vf31xy, $vf31xy, $vf31x
	vaddx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vaddx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vaddx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vaddx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vaddx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vaddx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vaddx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vaddx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vaddx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vaddx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vaddx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vaddx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vaddx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vaddx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vaddx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vaddx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vaddx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vaddx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vaddx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vaddx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vaddx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vaddx.xz	$vf0xz, $vf0xz, $vf0x
	vaddx.xz	$vf0xz, $vf0xz, $vf31x
	vaddx.xz	$vf0xz, $vf31xz, $vf0x
	vaddx.xz	$vf1xz, $vf2xz, $vf3x
	vaddx.xz	$vf31xz, $vf0xz, $vf0x
	vaddx.xz	$vf31xz, $vf15xz, $vf7x
	vaddx.xz	$vf31xz, $vf31xz, $vf31x
	vaddx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vaddx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vaddx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vaddx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vaddx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vaddx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vaddx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vadd.xy		$vf0xy, $vf0xy, $vf0xy
	vadd.xy		$vf0xy, $vf0xy, $vf31xy
	vadd.xy		$vf0xy, $vf31xy, $vf0xy
	vaddx.y		$vf0y, $vf0y, $vf0x
	vaddx.y		$vf0y, $vf0y, $vf31x
	vaddx.y		$vf0y, $vf31y, $vf0x
	vadd.xy		$vf1xy, $vf2xy, $vf3xy
	vaddx.y		$vf1y, $vf2y, $vf3x
	vadd.xy		$vf31xy, $vf0xy, $vf0xy
	vadd.xy		$vf31xy, $vf15xy, $vf7xy
	vadd.xy		$vf31xy, $vf31xy, $vf31xy
	vaddx.y		$vf31y, $vf0y, $vf0x
	vaddx.y		$vf31y, $vf15y, $vf7x
	vaddx.y		$vf31y, $vf31y, $vf31x
	vadd.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vadd.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vadd.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vaddx.yw	$vf0yw, $vf0yw, $vf0x
	vaddx.yw	$vf0yw, $vf0yw, $vf31x
	vaddx.yw	$vf0yw, $vf31yw, $vf0x
	vadd.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vaddx.yw	$vf1yw, $vf2yw, $vf3x
	vadd.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vadd.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vadd.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vaddx.yw	$vf31yw, $vf0yw, $vf0x
	vaddx.yw	$vf31yw, $vf15yw, $vf7x
	vaddx.yw	$vf31yw, $vf31yw, $vf31x
	vadd.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vadd.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vadd.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vaddx.yz	$vf0yz, $vf0yz, $vf0x
	vaddx.yz	$vf0yz, $vf0yz, $vf31x
	vaddx.yz	$vf0yz, $vf31yz, $vf0x
	vadd.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vaddx.yz	$vf1yz, $vf2yz, $vf3x
	vadd.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vadd.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vadd.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vaddx.yz	$vf31yz, $vf0yz, $vf0x
	vaddx.yz	$vf31yz, $vf15yz, $vf7x
	vaddx.yz	$vf31yz, $vf31yz, $vf31x
	vadd.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vadd.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vadd.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vaddx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vaddx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vaddx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vadd.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vaddx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vadd.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vadd.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vadd.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vaddx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vaddx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vaddx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vadd.xz		$vf0xz, $vf0xz, $vf0xz
	vadd.xz		$vf0xz, $vf0xz, $vf31xz
	vadd.xz		$vf0xz, $vf31xz, $vf0xz
	vaddx.z		$vf0z, $vf0z, $vf0x
	vaddx.z		$vf0z, $vf0z, $vf31x
	vaddx.z		$vf0z, $vf31z, $vf0x
	vadd.xz		$vf1xz, $vf2xz, $vf3xz
	vaddx.z		$vf1z, $vf2z, $vf3x
	vadd.xz		$vf31xz, $vf0xz, $vf0xz
	vadd.xz		$vf31xz, $vf15xz, $vf7xz
	vadd.xz		$vf31xz, $vf31xz, $vf31xz
	vaddx.z		$vf31z, $vf0z, $vf0x
	vaddx.z		$vf31z, $vf15z, $vf7x
	vaddx.z		$vf31z, $vf31z, $vf31x
	vadd.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vadd.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vadd.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vaddx.zw	$vf0zw, $vf0zw, $vf0x
	vaddx.zw	$vf0zw, $vf0zw, $vf31x
	vaddx.zw	$vf0zw, $vf31zw, $vf0x
	vadd.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vaddx.zw	$vf1zw, $vf2zw, $vf3x
	vadd.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vadd.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vadd.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vaddx.zw	$vf31zw, $vf0zw, $vf0x
	vaddx.zw	$vf31zw, $vf15zw, $vf7x
	vaddx.zw	$vf31zw, $vf31zw, $vf31x
	vadd.y		$vf0y, $vf0y, $vf0y
	vadd.y		$vf0y, $vf0y, $vf31y
	vadd.y		$vf0y, $vf31y, $vf0y
	vadd.y		$vf1y, $vf2y, $vf3y
	vadd.y		$vf31y, $vf0y, $vf0y
	vadd.y		$vf31y, $vf15y, $vf7y
	vadd.y		$vf31y, $vf31y, $vf31y
	vaddy.w		$vf0w, $vf0w, $vf0y
	vaddy.w		$vf0w, $vf0w, $vf31y
	vaddy.w		$vf0w, $vf31w, $vf0y
	vadd.yw		$vf0yw, $vf0yw, $vf0yw
	vadd.yw		$vf0yw, $vf0yw, $vf31yw
	vadd.yw		$vf0yw, $vf31yw, $vf0yw
	vaddy.w		$vf1w, $vf2w, $vf3y
	vadd.yw		$vf1yw, $vf2yw, $vf3yw
	vaddy.w		$vf31w, $vf0w, $vf0y
	vaddy.w		$vf31w, $vf15w, $vf7y
	vaddy.w		$vf31w, $vf31w, $vf31y
	vadd.yw		$vf31yw, $vf0yw, $vf0yw
	vadd.yw		$vf31yw, $vf15yw, $vf7yw
	vadd.yw		$vf31yw, $vf31yw, $vf31yw
	vaddy.x		$vf0x, $vf0x, $vf0y
	vaddy.x		$vf0x, $vf0x, $vf31y
	vaddy.x		$vf0x, $vf31x, $vf0y
	vaddy.x		$vf1x, $vf2x, $vf3y
	vaddy.x		$vf31x, $vf0x, $vf0y
	vaddy.x		$vf31x, $vf15x, $vf7y
	vaddy.x		$vf31x, $vf31x, $vf31y
	vaddy.xw	$vf0xw, $vf0xw, $vf0y
	vaddy.xw	$vf0xw, $vf0xw, $vf31y
	vaddy.xw	$vf0xw, $vf31xw, $vf0y
	vaddy.xw	$vf1xw, $vf2xw, $vf3y
	vaddy.xw	$vf31xw, $vf0xw, $vf0y
	vaddy.xw	$vf31xw, $vf15xw, $vf7y
	vaddy.xw	$vf31xw, $vf31xw, $vf31y
	vaddy.xy	$vf0xy, $vf0xy, $vf0y
	vaddy.xy	$vf0xy, $vf0xy, $vf31y
	vaddy.xy	$vf0xy, $vf31xy, $vf0y
	vaddy.xy	$vf1xy, $vf2xy, $vf3y
	vaddy.xy	$vf31xy, $vf0xy, $vf0y
	vaddy.xy	$vf31xy, $vf15xy, $vf7y
	vaddy.xy	$vf31xy, $vf31xy, $vf31y
	vaddy.xyw	$vf0xyw, $vf0xyw, $vf0y
	vaddy.xyw	$vf0xyw, $vf0xyw, $vf31y
	vaddy.xyw	$vf0xyw, $vf31xyw, $vf0y
	vaddy.xyw	$vf1xyw, $vf2xyw, $vf3y
	vaddy.xyw	$vf31xyw, $vf0xyw, $vf0y
	vaddy.xyw	$vf31xyw, $vf15xyw, $vf7y
	vaddy.xyw	$vf31xyw, $vf31xyw, $vf31y
	vaddy.xyz	$vf0xyz, $vf0xyz, $vf0y
	vaddy.xyz	$vf0xyz, $vf0xyz, $vf31y
	vaddy.xyz	$vf0xyz, $vf31xyz, $vf0y
	vaddy.xyz	$vf1xyz, $vf2xyz, $vf3y
	vaddy.xyz	$vf31xyz, $vf0xyz, $vf0y
	vaddy.xyz	$vf31xyz, $vf15xyz, $vf7y
	vaddy.xyz	$vf31xyz, $vf31xyz, $vf31y
	vaddy.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vaddy.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vaddy.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vaddy.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vaddy.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vaddy.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vaddy.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vaddy.xz	$vf0xz, $vf0xz, $vf0y
	vaddy.xz	$vf0xz, $vf0xz, $vf31y
	vaddy.xz	$vf0xz, $vf31xz, $vf0y
	vaddy.xz	$vf1xz, $vf2xz, $vf3y
	vaddy.xz	$vf31xz, $vf0xz, $vf0y
	vaddy.xz	$vf31xz, $vf15xz, $vf7y
	vaddy.xz	$vf31xz, $vf31xz, $vf31y
	vaddy.xzw	$vf0xzw, $vf0xzw, $vf0y
	vaddy.xzw	$vf0xzw, $vf0xzw, $vf31y
	vaddy.xzw	$vf0xzw, $vf31xzw, $vf0y
	vaddy.xzw	$vf1xzw, $vf2xzw, $vf3y
	vaddy.xzw	$vf31xzw, $vf0xzw, $vf0y
	vaddy.xzw	$vf31xzw, $vf15xzw, $vf7y
	vaddy.xzw	$vf31xzw, $vf31xzw, $vf31y
	vaddy.y		$vf0y, $vf0y, $vf0y
	vaddy.y		$vf0y, $vf0y, $vf31y
	vaddy.y		$vf0y, $vf31y, $vf0y
	vaddy.y		$vf1y, $vf2y, $vf3y
	vaddy.y		$vf31y, $vf0y, $vf0y
	vaddy.y		$vf31y, $vf15y, $vf7y
	vaddy.y		$vf31y, $vf31y, $vf31y
	vaddy.yw	$vf0yw, $vf0yw, $vf0y
	vaddy.yw	$vf0yw, $vf0yw, $vf31y
	vaddy.yw	$vf0yw, $vf31yw, $vf0y
	vaddy.yw	$vf1yw, $vf2yw, $vf3y
	vaddy.yw	$vf31yw, $vf0yw, $vf0y
	vaddy.yw	$vf31yw, $vf15yw, $vf7y
	vaddy.yw	$vf31yw, $vf31yw, $vf31y
	vaddy.yz	$vf0yz, $vf0yz, $vf0y
	vaddy.yz	$vf0yz, $vf0yz, $vf31y
	vaddy.yz	$vf0yz, $vf31yz, $vf0y
	vaddy.yz	$vf1yz, $vf2yz, $vf3y
	vaddy.yz	$vf31yz, $vf0yz, $vf0y
	vaddy.yz	$vf31yz, $vf15yz, $vf7y
	vaddy.yz	$vf31yz, $vf31yz, $vf31y
	vaddy.yzw	$vf0yzw, $vf0yzw, $vf0y
	vaddy.yzw	$vf0yzw, $vf0yzw, $vf31y
	vaddy.yzw	$vf0yzw, $vf31yzw, $vf0y
	vaddy.yzw	$vf1yzw, $vf2yzw, $vf3y
	vaddy.yzw	$vf31yzw, $vf0yzw, $vf0y
	vaddy.yzw	$vf31yzw, $vf15yzw, $vf7y
	vaddy.yzw	$vf31yzw, $vf31yzw, $vf31y
	vadd.yz		$vf0yz, $vf0yz, $vf0yz
	vadd.yz		$vf0yz, $vf0yz, $vf31yz
	vadd.yz		$vf0yz, $vf31yz, $vf0yz
	vaddy.z		$vf0z, $vf0z, $vf0y
	vaddy.z		$vf0z, $vf0z, $vf31y
	vaddy.z		$vf0z, $vf31z, $vf0y
	vadd.yz		$vf1yz, $vf2yz, $vf3yz
	vaddy.z		$vf1z, $vf2z, $vf3y
	vadd.yz		$vf31yz, $vf0yz, $vf0yz
	vadd.yz		$vf31yz, $vf15yz, $vf7yz
	vadd.yz		$vf31yz, $vf31yz, $vf31yz
	vaddy.z		$vf31z, $vf0z, $vf0y
	vaddy.z		$vf31z, $vf15z, $vf7y
	vaddy.z		$vf31z, $vf31z, $vf31y
	vadd.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vadd.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vadd.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vaddy.zw	$vf0zw, $vf0zw, $vf0y
	vaddy.zw	$vf0zw, $vf0zw, $vf31y
	vaddy.zw	$vf0zw, $vf31zw, $vf0y
	vadd.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vaddy.zw	$vf1zw, $vf2zw, $vf3y
	vadd.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vadd.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vadd.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vaddy.zw	$vf31zw, $vf0zw, $vf0y
	vaddy.zw	$vf31zw, $vf15zw, $vf7y
	vaddy.zw	$vf31zw, $vf31zw, $vf31y
	vadd.z		$vf0z, $vf0z, $vf0z
	vadd.z		$vf0z, $vf0z, $vf31z
	vadd.z		$vf0z, $vf31z, $vf0z
	vadd.z		$vf1z, $vf2z, $vf3z
	vadd.z		$vf31z, $vf0z, $vf0z
	vadd.z		$vf31z, $vf15z, $vf7z
	vadd.z		$vf31z, $vf31z, $vf31z
	vaddz.w		$vf0w, $vf0w, $vf0z
	vaddz.w		$vf0w, $vf0w, $vf31z
	vaddz.w		$vf0w, $vf31w, $vf0z
	vadd.zw		$vf0zw, $vf0zw, $vf0zw
	vadd.zw		$vf0zw, $vf0zw, $vf31zw
	vadd.zw		$vf0zw, $vf31zw, $vf0zw
	vaddz.w		$vf1w, $vf2w, $vf3z
	vadd.zw		$vf1zw, $vf2zw, $vf3zw
	vaddz.w		$vf31w, $vf0w, $vf0z
	vaddz.w		$vf31w, $vf15w, $vf7z
	vaddz.w		$vf31w, $vf31w, $vf31z
	vadd.zw		$vf31zw, $vf0zw, $vf0zw
	vadd.zw		$vf31zw, $vf15zw, $vf7zw
	vadd.zw		$vf31zw, $vf31zw, $vf31zw
	vaddz.x		$vf0x, $vf0x, $vf0z
	vaddz.x		$vf0x, $vf0x, $vf31z
	vaddz.x		$vf0x, $vf31x, $vf0z
	vaddz.x		$vf1x, $vf2x, $vf3z
	vaddz.x		$vf31x, $vf0x, $vf0z
	vaddz.x		$vf31x, $vf15x, $vf7z
	vaddz.x		$vf31x, $vf31x, $vf31z
	vaddz.xw	$vf0xw, $vf0xw, $vf0z
	vaddz.xw	$vf0xw, $vf0xw, $vf31z
	vaddz.xw	$vf0xw, $vf31xw, $vf0z
	vaddz.xw	$vf1xw, $vf2xw, $vf3z
	vaddz.xw	$vf31xw, $vf0xw, $vf0z
	vaddz.xw	$vf31xw, $vf15xw, $vf7z
	vaddz.xw	$vf31xw, $vf31xw, $vf31z
	vaddz.xy	$vf0xy, $vf0xy, $vf0z
	vaddz.xy	$vf0xy, $vf0xy, $vf31z
	vaddz.xy	$vf0xy, $vf31xy, $vf0z
	vaddz.xy	$vf1xy, $vf2xy, $vf3z
	vaddz.xy	$vf31xy, $vf0xy, $vf0z
	vaddz.xy	$vf31xy, $vf15xy, $vf7z
	vaddz.xy	$vf31xy, $vf31xy, $vf31z
	vaddz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vaddz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vaddz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vaddz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vaddz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vaddz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vaddz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vaddz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vaddz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vaddz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vaddz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vaddz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vaddz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vaddz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vaddz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vaddz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vaddz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vaddz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vaddz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vaddz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vaddz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vaddz.xz	$vf0xz, $vf0xz, $vf0z
	vaddz.xz	$vf0xz, $vf0xz, $vf31z
	vaddz.xz	$vf0xz, $vf31xz, $vf0z
	vaddz.xz	$vf1xz, $vf2xz, $vf3z
	vaddz.xz	$vf31xz, $vf0xz, $vf0z
	vaddz.xz	$vf31xz, $vf15xz, $vf7z
	vaddz.xz	$vf31xz, $vf31xz, $vf31z
	vaddz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vaddz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vaddz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vaddz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vaddz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vaddz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vaddz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vaddz.y		$vf0y, $vf0y, $vf0z
	vaddz.y		$vf0y, $vf0y, $vf31z
	vaddz.y		$vf0y, $vf31y, $vf0z
	vaddz.y		$vf1y, $vf2y, $vf3z
	vaddz.y		$vf31y, $vf0y, $vf0z
	vaddz.y		$vf31y, $vf15y, $vf7z
	vaddz.y		$vf31y, $vf31y, $vf31z
	vaddz.yw	$vf0yw, $vf0yw, $vf0z
	vaddz.yw	$vf0yw, $vf0yw, $vf31z
	vaddz.yw	$vf0yw, $vf31yw, $vf0z
	vaddz.yw	$vf1yw, $vf2yw, $vf3z
	vaddz.yw	$vf31yw, $vf0yw, $vf0z
	vaddz.yw	$vf31yw, $vf15yw, $vf7z
	vaddz.yw	$vf31yw, $vf31yw, $vf31z
	vaddz.yz	$vf0yz, $vf0yz, $vf0z
	vaddz.yz	$vf0yz, $vf0yz, $vf31z
	vaddz.yz	$vf0yz, $vf31yz, $vf0z
	vaddz.yz	$vf1yz, $vf2yz, $vf3z
	vaddz.yz	$vf31yz, $vf0yz, $vf0z
	vaddz.yz	$vf31yz, $vf15yz, $vf7z
	vaddz.yz	$vf31yz, $vf31yz, $vf31z
	vaddz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vaddz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vaddz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vaddz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vaddz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vaddz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vaddz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vaddz.z		$vf0z, $vf0z, $vf0z
	vaddz.z		$vf0z, $vf0z, $vf31z
	vaddz.z		$vf0z, $vf31z, $vf0z
	vaddz.z		$vf1z, $vf2z, $vf3z
	vaddz.z		$vf31z, $vf0z, $vf0z
	vaddz.z		$vf31z, $vf15z, $vf7z
	vaddz.z		$vf31z, $vf31z, $vf31z
	vaddz.zw	$vf0zw, $vf0zw, $vf0z
	vaddz.zw	$vf0zw, $vf0zw, $vf31z
	vaddz.zw	$vf0zw, $vf31zw, $vf0z
	vaddz.zw	$vf1zw, $vf2zw, $vf3z
	vaddz.zw	$vf31zw, $vf0zw, $vf0z
	vaddz.zw	$vf31zw, $vf15zw, $vf7z
	vaddz.zw	$vf31zw, $vf31zw, $vf31z
	vcallms		0x0
	vcallms		0x0080
	vcallms		0x00a0
	vcallms		0x00c0
	vcallms		0x4000
	vcallms		0x7FF8
	vcallms		0x8
	vcallmsr	$vi27
	vclipw.xyz	$vf0xyz, $vf0w
	vclipw.xyz	$vf0xyz, $vf31w
	vclipw.xyz	$vf1xyz, $vf2w
	vclipw.xyz	$vf31xyz, $vf0w
	vclipw.xyz	$vf31xyz, $vf15w
	vclipw.xyz	$vf31xyz, $vf31w
	vdiv		$Q, $vf0w, $vf0z
	vdiv		$Q, $vf0x, $vf0x
	vdiv		$Q, $vf0z, $vf31y
	vdiv		$Q, $vf1z, $vf2z
	vdiv		$Q, $vf31x, $vf15w
	vdiv		$Q, $vf31x, $vf31y
	vdiv		$Q, $vf31y, $vf0w
	vftoi0.w	$vf0w, $vf0w
	vftoi0.w	$vf0w, $vf31w
	vftoi0.w	$vf1w, $vf2w
	vftoi0.w	$vf31w, $vf0w
	vftoi0.w	$vf31w, $vf15w
	vftoi0.w	$vf31w, $vf31w
	vftoi0.x	$vf0x, $vf0x
	vftoi0.x	$vf0x, $vf31x
	vftoi0.x	$vf1x, $vf2x
	vftoi0.x	$vf31x, $vf0x
	vftoi0.x	$vf31x, $vf15x
	vftoi0.x	$vf31x, $vf31x
	vftoi0.xw	$vf0xw, $vf0xw
	vftoi0.xw	$vf0xw, $vf31xw
	vftoi0.xw	$vf1xw, $vf2xw
	vftoi0.xw	$vf31xw, $vf0xw
	vftoi0.xw	$vf31xw, $vf15xw
	vftoi0.xw	$vf31xw, $vf31xw
	vftoi0.xy	$vf0xy, $vf0xy
	vftoi0.xy	$vf0xy, $vf31xy
	vftoi0.xy	$vf1xy, $vf2xy
	vftoi0.xy	$vf31xy, $vf0xy
	vftoi0.xy	$vf31xy, $vf15xy
	vftoi0.xy	$vf31xy, $vf31xy
	vftoi0.xyw	$vf0xyw, $vf0xyw
	vftoi0.xyw	$vf0xyw, $vf31xyw
	vftoi0.xyw	$vf1xyw, $vf2xyw
	vftoi0.xyw	$vf31xyw, $vf0xyw
	vftoi0.xyw	$vf31xyw, $vf15xyw
	vftoi0.xyw	$vf31xyw, $vf31xyw
	vftoi0.xyz	$vf0xyz, $vf0xyz
	vftoi0.xyz	$vf0xyz, $vf31xyz
	vftoi0.xyz	$vf1xyz, $vf2xyz
	vftoi0.xyz	$vf31xyz, $vf0xyz
	vftoi0.xyz	$vf31xyz, $vf15xyz
	vftoi0.xyz	$vf31xyz, $vf31xyz
	vftoi0.xyzw	$vf0xyzw, $vf0xyzw
	vftoi0.xyzw	$vf0xyzw, $vf31xyzw
	vftoi0.xyzw	$vf1xyzw, $vf2xyzw
	vftoi0.xyzw	$vf31xyzw, $vf0xyzw
	vftoi0.xyzw	$vf31xyzw, $vf15xyzw
	vftoi0.xyzw	$vf31xyzw, $vf31xyzw
	vftoi0.xz	$vf0xz, $vf0xz
	vftoi0.xz	$vf0xz, $vf31xz
	vftoi0.xz	$vf1xz, $vf2xz
	vftoi0.xz	$vf31xz, $vf0xz
	vftoi0.xz	$vf31xz, $vf15xz
	vftoi0.xz	$vf31xz, $vf31xz
	vftoi0.xzw	$vf0xzw, $vf0xzw
	vftoi0.xzw	$vf0xzw, $vf31xzw
	vftoi0.xzw	$vf1xzw, $vf2xzw
	vftoi0.xzw	$vf31xzw, $vf0xzw
	vftoi0.xzw	$vf31xzw, $vf15xzw
	vftoi0.xzw	$vf31xzw, $vf31xzw
	vftoi0.y	$vf0y, $vf0y
	vftoi0.y	$vf0y, $vf31y
	vftoi0.y	$vf1y, $vf2y
	vftoi0.y	$vf31y, $vf0y
	vftoi0.y	$vf31y, $vf15y
	vftoi0.y	$vf31y, $vf31y
	vftoi0.yw	$vf0yw, $vf0yw
	vftoi0.yw	$vf0yw, $vf31yw
	vftoi0.yw	$vf1yw, $vf2yw
	vftoi0.yw	$vf31yw, $vf0yw
	vftoi0.yw	$vf31yw, $vf15yw
	vftoi0.yw	$vf31yw, $vf31yw
	vftoi0.yz	$vf0yz, $vf0yz
	vftoi0.yz	$vf0yz, $vf31yz
	vftoi0.yz	$vf1yz, $vf2yz
	vftoi0.yz	$vf31yz, $vf0yz
	vftoi0.yz	$vf31yz, $vf15yz
	vftoi0.yz	$vf31yz, $vf31yz
	vftoi0.yzw	$vf0yzw, $vf0yzw
	vftoi0.yzw	$vf0yzw, $vf31yzw
	vftoi0.yzw	$vf1yzw, $vf2yzw
	vftoi0.yzw	$vf31yzw, $vf0yzw
	vftoi0.yzw	$vf31yzw, $vf15yzw
	vftoi0.yzw	$vf31yzw, $vf31yzw
	vftoi0.z	$vf0z, $vf0z
	vftoi0.z	$vf0z, $vf31z
	vftoi0.z	$vf1z, $vf2z
	vftoi0.z	$vf31z, $vf0z
	vftoi0.z	$vf31z, $vf15z
	vftoi0.z	$vf31z, $vf31z
	vftoi0.zw	$vf0zw, $vf0zw
	vftoi0.zw	$vf0zw, $vf31zw
	vftoi0.zw	$vf1zw, $vf2zw
	vftoi0.zw	$vf31zw, $vf0zw
	vftoi0.zw	$vf31zw, $vf15zw
	vftoi0.zw	$vf31zw, $vf31zw
	vftoi12.w	$vf0w, $vf0w
	vftoi12.w	$vf0w, $vf31w
	vftoi12.w	$vf1w, $vf2w
	vftoi12.w	$vf31w, $vf0w
	vftoi12.w	$vf31w, $vf15w
	vftoi12.w	$vf31w, $vf31w
	vftoi12.x	$vf0x, $vf0x
	vftoi12.x	$vf0x, $vf31x
	vftoi12.x	$vf1x, $vf2x
	vftoi12.x	$vf31x, $vf0x
	vftoi12.x	$vf31x, $vf15x
	vftoi12.x	$vf31x, $vf31x
	vftoi12.xw	$vf0xw, $vf0xw
	vftoi12.xw	$vf0xw, $vf31xw
	vftoi12.xw	$vf1xw, $vf2xw
	vftoi12.xw	$vf31xw, $vf0xw
	vftoi12.xw	$vf31xw, $vf15xw
	vftoi12.xw	$vf31xw, $vf31xw
	vftoi12.xy	$vf0xy, $vf0xy
	vftoi12.xy	$vf0xy, $vf31xy
	vftoi12.xy	$vf1xy, $vf2xy
	vftoi12.xy	$vf31xy, $vf0xy
	vftoi12.xy	$vf31xy, $vf15xy
	vftoi12.xy	$vf31xy, $vf31xy
	vftoi12.xyw	$vf0xyw, $vf0xyw
	vftoi12.xyw	$vf0xyw, $vf31xyw
	vftoi12.xyw	$vf1xyw, $vf2xyw
	vftoi12.xyw	$vf31xyw, $vf0xyw
	vftoi12.xyw	$vf31xyw, $vf15xyw
	vftoi12.xyw	$vf31xyw, $vf31xyw
	vftoi12.xyz	$vf0xyz, $vf0xyz
	vftoi12.xyz	$vf0xyz, $vf31xyz
	vftoi12.xyz	$vf1xyz, $vf2xyz
	vftoi12.xyz	$vf31xyz, $vf0xyz
	vftoi12.xyz	$vf31xyz, $vf15xyz
	vftoi12.xyz	$vf31xyz, $vf31xyz
	vftoi12.xyzw	$vf0xyzw, $vf0xyzw
	vftoi12.xyzw	$vf0xyzw, $vf31xyzw
	vftoi12.xyzw	$vf1xyzw, $vf2xyzw
	vftoi12.xyzw	$vf31xyzw, $vf0xyzw
	vftoi12.xyzw	$vf31xyzw, $vf15xyzw
	vftoi12.xyzw	$vf31xyzw, $vf31xyzw
	vftoi12.xz	$vf0xz, $vf0xz
	vftoi12.xz	$vf0xz, $vf31xz
	vftoi12.xz	$vf1xz, $vf2xz
	vftoi12.xz	$vf31xz, $vf0xz
	vftoi12.xz	$vf31xz, $vf15xz
	vftoi12.xz	$vf31xz, $vf31xz
	vftoi12.xzw	$vf0xzw, $vf0xzw
	vftoi12.xzw	$vf0xzw, $vf31xzw
	vftoi12.xzw	$vf1xzw, $vf2xzw
	vftoi12.xzw	$vf31xzw, $vf0xzw
	vftoi12.xzw	$vf31xzw, $vf15xzw
	vftoi12.xzw	$vf31xzw, $vf31xzw
	vftoi12.y	$vf0y, $vf0y
	vftoi12.y	$vf0y, $vf31y
	vftoi12.y	$vf1y, $vf2y
	vftoi12.y	$vf31y, $vf0y
	vftoi12.y	$vf31y, $vf15y
	vftoi12.y	$vf31y, $vf31y
	vftoi12.yw	$vf0yw, $vf0yw
	vftoi12.yw	$vf0yw, $vf31yw
	vftoi12.yw	$vf1yw, $vf2yw
	vftoi12.yw	$vf31yw, $vf0yw
	vftoi12.yw	$vf31yw, $vf15yw
	vftoi12.yw	$vf31yw, $vf31yw
	vftoi12.yz	$vf0yz, $vf0yz
	vftoi12.yz	$vf0yz, $vf31yz
	vftoi12.yz	$vf1yz, $vf2yz
	vftoi12.yz	$vf31yz, $vf0yz
	vftoi12.yz	$vf31yz, $vf15yz
	vftoi12.yz	$vf31yz, $vf31yz
	vftoi12.yzw	$vf0yzw, $vf0yzw
	vftoi12.yzw	$vf0yzw, $vf31yzw
	vftoi12.yzw	$vf1yzw, $vf2yzw
	vftoi12.yzw	$vf31yzw, $vf0yzw
	vftoi12.yzw	$vf31yzw, $vf15yzw
	vftoi12.yzw	$vf31yzw, $vf31yzw
	vftoi12.z	$vf0z, $vf0z
	vftoi12.z	$vf0z, $vf31z
	vftoi12.z	$vf1z, $vf2z
	vftoi12.z	$vf31z, $vf0z
	vftoi12.z	$vf31z, $vf15z
	vftoi12.z	$vf31z, $vf31z
	vftoi12.zw	$vf0zw, $vf0zw
	vftoi12.zw	$vf0zw, $vf31zw
	vftoi12.zw	$vf1zw, $vf2zw
	vftoi12.zw	$vf31zw, $vf0zw
	vftoi12.zw	$vf31zw, $vf15zw
	vftoi12.zw	$vf31zw, $vf31zw
	vftoi15.w	$vf0w, $vf0w
	vftoi15.w	$vf0w, $vf31w
	vftoi15.w	$vf1w, $vf2w
	vftoi15.w	$vf31w, $vf0w
	vftoi15.w	$vf31w, $vf15w
	vftoi15.w	$vf31w, $vf31w
	vftoi15.x	$vf0x, $vf0x
	vftoi15.x	$vf0x, $vf31x
	vftoi15.x	$vf1x, $vf2x
	vftoi15.x	$vf31x, $vf0x
	vftoi15.x	$vf31x, $vf15x
	vftoi15.x	$vf31x, $vf31x
	vftoi15.xw	$vf0xw, $vf0xw
	vftoi15.xw	$vf0xw, $vf31xw
	vftoi15.xw	$vf1xw, $vf2xw
	vftoi15.xw	$vf31xw, $vf0xw
	vftoi15.xw	$vf31xw, $vf15xw
	vftoi15.xw	$vf31xw, $vf31xw
	vftoi15.xy	$vf0xy, $vf0xy
	vftoi15.xy	$vf0xy, $vf31xy
	vftoi15.xy	$vf1xy, $vf2xy
	vftoi15.xy	$vf31xy, $vf0xy
	vftoi15.xy	$vf31xy, $vf15xy
	vftoi15.xy	$vf31xy, $vf31xy
	vftoi15.xyw	$vf0xyw, $vf0xyw
	vftoi15.xyw	$vf0xyw, $vf31xyw
	vftoi15.xyw	$vf1xyw, $vf2xyw
	vftoi15.xyw	$vf31xyw, $vf0xyw
	vftoi15.xyw	$vf31xyw, $vf15xyw
	vftoi15.xyw	$vf31xyw, $vf31xyw
	vftoi15.xyz	$vf0xyz, $vf0xyz
	vftoi15.xyz	$vf0xyz, $vf31xyz
	vftoi15.xyz	$vf1xyz, $vf2xyz
	vftoi15.xyz	$vf31xyz, $vf0xyz
	vftoi15.xyz	$vf31xyz, $vf15xyz
	vftoi15.xyz	$vf31xyz, $vf31xyz
	vftoi15.xyzw	$vf0xyzw, $vf0xyzw
	vftoi15.xyzw	$vf0xyzw, $vf31xyzw
	vftoi15.xyzw	$vf1xyzw, $vf2xyzw
	vftoi15.xyzw	$vf31xyzw, $vf0xyzw
	vftoi15.xyzw	$vf31xyzw, $vf15xyzw
	vftoi15.xyzw	$vf31xyzw, $vf31xyzw
	vftoi15.xz	$vf0xz, $vf0xz
	vftoi15.xz	$vf0xz, $vf31xz
	vftoi15.xz	$vf1xz, $vf2xz
	vftoi15.xz	$vf31xz, $vf0xz
	vftoi15.xz	$vf31xz, $vf15xz
	vftoi15.xz	$vf31xz, $vf31xz
	vftoi15.xzw	$vf0xzw, $vf0xzw
	vftoi15.xzw	$vf0xzw, $vf31xzw
	vftoi15.xzw	$vf1xzw, $vf2xzw
	vftoi15.xzw	$vf31xzw, $vf0xzw
	vftoi15.xzw	$vf31xzw, $vf15xzw
	vftoi15.xzw	$vf31xzw, $vf31xzw
	vftoi15.y	$vf0y, $vf0y
	vftoi15.y	$vf0y, $vf31y
	vftoi15.y	$vf1y, $vf2y
	vftoi15.y	$vf31y, $vf0y
	vftoi15.y	$vf31y, $vf15y
	vftoi15.y	$vf31y, $vf31y
	vftoi15.yw	$vf0yw, $vf0yw
	vftoi15.yw	$vf0yw, $vf31yw
	vftoi15.yw	$vf1yw, $vf2yw
	vftoi15.yw	$vf31yw, $vf0yw
	vftoi15.yw	$vf31yw, $vf15yw
	vftoi15.yw	$vf31yw, $vf31yw
	vftoi15.yz	$vf0yz, $vf0yz
	vftoi15.yz	$vf0yz, $vf31yz
	vftoi15.yz	$vf1yz, $vf2yz
	vftoi15.yz	$vf31yz, $vf0yz
	vftoi15.yz	$vf31yz, $vf15yz
	vftoi15.yz	$vf31yz, $vf31yz
	vftoi15.yzw	$vf0yzw, $vf0yzw
	vftoi15.yzw	$vf0yzw, $vf31yzw
	vftoi15.yzw	$vf1yzw, $vf2yzw
	vftoi15.yzw	$vf31yzw, $vf0yzw
	vftoi15.yzw	$vf31yzw, $vf15yzw
	vftoi15.yzw	$vf31yzw, $vf31yzw
	vftoi15.z	$vf0z, $vf0z
	vftoi15.z	$vf0z, $vf31z
	vftoi15.z	$vf1z, $vf2z
	vftoi15.z	$vf31z, $vf0z
	vftoi15.z	$vf31z, $vf15z
	vftoi15.z	$vf31z, $vf31z
	vftoi15.zw	$vf0zw, $vf0zw
	vftoi15.zw	$vf0zw, $vf31zw
	vftoi15.zw	$vf1zw, $vf2zw
	vftoi15.zw	$vf31zw, $vf0zw
	vftoi15.zw	$vf31zw, $vf15zw
	vftoi15.zw	$vf31zw, $vf31zw
	vftoi4.w	$vf0w, $vf0w
	vftoi4.w	$vf0w, $vf31w
	vftoi4.w	$vf1w, $vf2w
	vftoi4.w	$vf31w, $vf0w
	vftoi4.w	$vf31w, $vf15w
	vftoi4.w	$vf31w, $vf31w
	vftoi4.x	$vf0x, $vf0x
	vftoi4.x	$vf0x, $vf31x
	vftoi4.x	$vf1x, $vf2x
	vftoi4.x	$vf31x, $vf0x
	vftoi4.x	$vf31x, $vf15x
	vftoi4.x	$vf31x, $vf31x
	vftoi4.xw	$vf0xw, $vf0xw
	vftoi4.xw	$vf0xw, $vf31xw
	vftoi4.xw	$vf1xw, $vf2xw
	vftoi4.xw	$vf31xw, $vf0xw
	vftoi4.xw	$vf31xw, $vf15xw
	vftoi4.xw	$vf31xw, $vf31xw
	vftoi4.xy	$vf0xy, $vf0xy
	vftoi4.xy	$vf0xy, $vf31xy
	vftoi4.xy	$vf1xy, $vf2xy
	vftoi4.xy	$vf31xy, $vf0xy
	vftoi4.xy	$vf31xy, $vf15xy
	vftoi4.xy	$vf31xy, $vf31xy
	vftoi4.xyw	$vf0xyw, $vf0xyw
	vftoi4.xyw	$vf0xyw, $vf31xyw
	vftoi4.xyw	$vf1xyw, $vf2xyw
	vftoi4.xyw	$vf31xyw, $vf0xyw
	vftoi4.xyw	$vf31xyw, $vf15xyw
	vftoi4.xyw	$vf31xyw, $vf31xyw
	vftoi4.xyz	$vf0xyz, $vf0xyz
	vftoi4.xyz	$vf0xyz, $vf31xyz
	vftoi4.xyz	$vf1xyz, $vf2xyz
	vftoi4.xyz	$vf31xyz, $vf0xyz
	vftoi4.xyz	$vf31xyz, $vf15xyz
	vftoi4.xyz	$vf31xyz, $vf31xyz
	vftoi4.xyzw	$vf0xyzw, $vf0xyzw
	vftoi4.xyzw	$vf0xyzw, $vf31xyzw
	vftoi4.xyzw	$vf1xyzw, $vf2xyzw
	vftoi4.xyzw	$vf31xyzw, $vf0xyzw
	vftoi4.xyzw	$vf31xyzw, $vf15xyzw
	vftoi4.xyzw	$vf31xyzw, $vf31xyzw
	vftoi4.xz	$vf0xz, $vf0xz
	vftoi4.xz	$vf0xz, $vf31xz
	vftoi4.xz	$vf1xz, $vf2xz
	vftoi4.xz	$vf31xz, $vf0xz
	vftoi4.xz	$vf31xz, $vf15xz
	vftoi4.xz	$vf31xz, $vf31xz
	vftoi4.xzw	$vf0xzw, $vf0xzw
	vftoi4.xzw	$vf0xzw, $vf31xzw
	vftoi4.xzw	$vf1xzw, $vf2xzw
	vftoi4.xzw	$vf31xzw, $vf0xzw
	vftoi4.xzw	$vf31xzw, $vf15xzw
	vftoi4.xzw	$vf31xzw, $vf31xzw
	vftoi4.y	$vf0y, $vf0y
	vftoi4.y	$vf0y, $vf31y
	vftoi4.y	$vf1y, $vf2y
	vftoi4.y	$vf31y, $vf0y
	vftoi4.y	$vf31y, $vf15y
	vftoi4.y	$vf31y, $vf31y
	vftoi4.yw	$vf0yw, $vf0yw
	vftoi4.yw	$vf0yw, $vf31yw
	vftoi4.yw	$vf1yw, $vf2yw
	vftoi4.yw	$vf31yw, $vf0yw
	vftoi4.yw	$vf31yw, $vf15yw
	vftoi4.yw	$vf31yw, $vf31yw
	vftoi4.yz	$vf0yz, $vf0yz
	vftoi4.yz	$vf0yz, $vf31yz
	vftoi4.yz	$vf1yz, $vf2yz
	vftoi4.yz	$vf31yz, $vf0yz
	vftoi4.yz	$vf31yz, $vf15yz
	vftoi4.yz	$vf31yz, $vf31yz
	vftoi4.yzw	$vf0yzw, $vf0yzw
	vftoi4.yzw	$vf0yzw, $vf31yzw
	vftoi4.yzw	$vf1yzw, $vf2yzw
	vftoi4.yzw	$vf31yzw, $vf0yzw
	vftoi4.yzw	$vf31yzw, $vf15yzw
	vftoi4.yzw	$vf31yzw, $vf31yzw
	vftoi4.z	$vf0z, $vf0z
	vftoi4.z	$vf0z, $vf31z
	vftoi4.z	$vf1z, $vf2z
	vftoi4.z	$vf31z, $vf0z
	vftoi4.z	$vf31z, $vf15z
	vftoi4.z	$vf31z, $vf31z
	vftoi4.zw	$vf0zw, $vf0zw
	vftoi4.zw	$vf0zw, $vf31zw
	vftoi4.zw	$vf1zw, $vf2zw
	vftoi4.zw	$vf31zw, $vf0zw
	vftoi4.zw	$vf31zw, $vf15zw
	vftoi4.zw	$vf31zw, $vf31zw
	viaddi		$vi0, $vi0, 0
	viaddi		$vi0, $vi0, -15
	viaddi		$vi0, $vi31, 0
	viaddi		$vi1, $vi2, 3
	viaddi		$vi31, $vi0, 0
	viaddi		$vi31, $vi15, 7
	viaddi		$vi31, $vi31, -15
	viadd		$vi0, $vi0, $vi0
	viadd		$vi0, $vi0, $vi31
	viadd		$vi0, $vi31, $vi0
	viadd		$vi1, $vi2, $vi3
	viadd		$vi31, $vi0, $vi0
	viadd		$vi31, $vi15, $vi7
	viadd		$vi31, $vi31, $vi31
	viand		$vi0, $vi0, $vi0
	viand		$vi0, $vi0, $vi31
	viand		$vi0, $vi31, $vi0
	viand		$vi1, $vi2, $vi3
	viand		$vi31, $vi0, $vi0
	viand		$vi31, $vi15, $vi7
	viand		$vi31, $vi31, $vi31
	vilwr.w		$vi0, ($vi0)
	vilwr.w		$vi0, ($vi31)
	vilwr.w		$vi1, ($vi2)
	vilwr.w		$vi31, ($vi0)
	vilwr.w		$vi31, ($vi15)
	vilwr.w		$vi31, ($vi31)
	vilwr.x		$vi0, ($vi0)
	vilwr.x		$vi0, ($vi31)
	vilwr.x		$vi1, ($vi2)
	vilwr.x		$vi31, ($vi0)
	vilwr.x		$vi31, ($vi15)
	vilwr.x		$vi31, ($vi31)
	vilwr.y		$vi0, ($vi0)
	vilwr.y		$vi0, ($vi31)
	vilwr.y		$vi1, ($vi2)
	vilwr.y		$vi31, ($vi0)
	vilwr.y		$vi31, ($vi15)
	vilwr.y		$vi31, ($vi31)
	vilwr.z		$vi0, ($vi0)
	vilwr.z		$vi0, ($vi31)
	vilwr.z		$vi1, ($vi2)
	vilwr.z		$vi31, ($vi0)
	vilwr.z		$vi31, ($vi15)
	vilwr.z		$vi31, ($vi31)
	vior		$vi0, $vi0, $vi0
	vior		$vi0, $vi0, $vi31
	vior		$vi0, $vi31, $vi0
	vior		$vi1, $vi2, $vi3
	vior		$vi31, $vi0, $vi0
	vior		$vi31, $vi15, $vi7
	vior		$vi31, $vi31, $vi31
	visub		$vi0, $vi0, $vi0
	visub		$vi0, $vi0, $vi31
	visub		$vi0, $vi31, $vi0
	visub		$vi1, $vi2, $vi3
	visub		$vi31, $vi0, $vi0
	visub		$vi31, $vi15, $vi7
	visub		$vi31, $vi31, $vi31
	viswr.w		$vi0, ($vi0)
	viswr.w		$vi0, ($vi31)
	viswr.w		$vi1, ($vi2)
	viswr.w		$vi31, ($vi0)
	viswr.w		$vi31, ($vi15)
	viswr.w		$vi31, ($vi31)
	viswr.x		$vi0, ($vi0)
	viswr.x		$vi0, ($vi31)
	viswr.x		$vi1, ($vi2)
	viswr.x		$vi31, ($vi0)
	viswr.x		$vi31, ($vi15)
	viswr.x		$vi31, ($vi31)
	viswr.y		$vi0, ($vi0)
	viswr.y		$vi0, ($vi31)
	viswr.y		$vi1, ($vi2)
	viswr.y		$vi31, ($vi0)
	viswr.y		$vi31, ($vi15)
	viswr.y		$vi31, ($vi31)
	viswr.z		$vi0, ($vi0)
	viswr.z		$vi0, ($vi31)
	viswr.z		$vi1, ($vi2)
	viswr.z		$vi31, ($vi0)
	viswr.z		$vi31, ($vi15)
	viswr.z		$vi31, ($vi31)
	vitof0.w	$vf0w, $vf0w
	vitof0.w	$vf0w, $vf31w
	vitof0.w	$vf1w, $vf2w
	vitof0.w	$vf31w, $vf0w
	vitof0.w	$vf31w, $vf15w
	vitof0.w	$vf31w, $vf31w
	vitof0.x	$vf0x, $vf0x
	vitof0.x	$vf0x, $vf31x
	vitof0.x	$vf1x, $vf2x
	vitof0.x	$vf31x, $vf0x
	vitof0.x	$vf31x, $vf15x
	vitof0.x	$vf31x, $vf31x
	vitof0.xw	$vf0xw, $vf0xw
	vitof0.xw	$vf0xw, $vf31xw
	vitof0.xw	$vf1xw, $vf2xw
	vitof0.xw	$vf31xw, $vf0xw
	vitof0.xw	$vf31xw, $vf15xw
	vitof0.xw	$vf31xw, $vf31xw
	vitof0.xy	$vf0xy, $vf0xy
	vitof0.xy	$vf0xy, $vf31xy
	vitof0.xy	$vf1xy, $vf2xy
	vitof0.xy	$vf31xy, $vf0xy
	vitof0.xy	$vf31xy, $vf15xy
	vitof0.xy	$vf31xy, $vf31xy
	vitof0.xyw	$vf0xyw, $vf0xyw
	vitof0.xyw	$vf0xyw, $vf31xyw
	vitof0.xyw	$vf1xyw, $vf2xyw
	vitof0.xyw	$vf31xyw, $vf0xyw
	vitof0.xyw	$vf31xyw, $vf15xyw
	vitof0.xyw	$vf31xyw, $vf31xyw
	vitof0.xyz	$vf0xyz, $vf0xyz
	vitof0.xyz	$vf0xyz, $vf31xyz
	vitof0.xyz	$vf1xyz, $vf2xyz
	vitof0.xyz	$vf31xyz, $vf0xyz
	vitof0.xyz	$vf31xyz, $vf15xyz
	vitof0.xyz	$vf31xyz, $vf31xyz
	vitof0.xyzw	$vf0xyzw, $vf0xyzw
	vitof0.xyzw	$vf0xyzw, $vf31xyzw
	vitof0.xyzw	$vf1xyzw, $vf2xyzw
	vitof0.xyzw	$vf31xyzw, $vf0xyzw
	vitof0.xyzw	$vf31xyzw, $vf15xyzw
	vitof0.xyzw	$vf31xyzw, $vf31xyzw
	vitof0.xz	$vf0xz, $vf0xz
	vitof0.xz	$vf0xz, $vf31xz
	vitof0.xz	$vf1xz, $vf2xz
	vitof0.xz	$vf31xz, $vf0xz
	vitof0.xz	$vf31xz, $vf15xz
	vitof0.xz	$vf31xz, $vf31xz
	vitof0.xzw	$vf0xzw, $vf0xzw
	vitof0.xzw	$vf0xzw, $vf31xzw
	vitof0.xzw	$vf1xzw, $vf2xzw
	vitof0.xzw	$vf31xzw, $vf0xzw
	vitof0.xzw	$vf31xzw, $vf15xzw
	vitof0.xzw	$vf31xzw, $vf31xzw
	vitof0.y	$vf0y, $vf0y
	vitof0.y	$vf0y, $vf31y
	vitof0.y	$vf1y, $vf2y
	vitof0.y	$vf31y, $vf0y
	vitof0.y	$vf31y, $vf15y
	vitof0.y	$vf31y, $vf31y
	vitof0.yw	$vf0yw, $vf0yw
	vitof0.yw	$vf0yw, $vf31yw
	vitof0.yw	$vf1yw, $vf2yw
	vitof0.yw	$vf31yw, $vf0yw
	vitof0.yw	$vf31yw, $vf15yw
	vitof0.yw	$vf31yw, $vf31yw
	vitof0.yz	$vf0yz, $vf0yz
	vitof0.yz	$vf0yz, $vf31yz
	vitof0.yz	$vf1yz, $vf2yz
	vitof0.yz	$vf31yz, $vf0yz
	vitof0.yz	$vf31yz, $vf15yz
	vitof0.yz	$vf31yz, $vf31yz
	vitof0.yzw	$vf0yzw, $vf0yzw
	vitof0.yzw	$vf0yzw, $vf31yzw
	vitof0.yzw	$vf1yzw, $vf2yzw
	vitof0.yzw	$vf31yzw, $vf0yzw
	vitof0.yzw	$vf31yzw, $vf15yzw
	vitof0.yzw	$vf31yzw, $vf31yzw
	vitof0.z	$vf0z, $vf0z
	vitof0.z	$vf0z, $vf31z
	vitof0.z	$vf1z, $vf2z
	vitof0.z	$vf31z, $vf0z
	vitof0.z	$vf31z, $vf15z
	vitof0.z	$vf31z, $vf31z
	vitof0.zw	$vf0zw, $vf0zw
	vitof0.zw	$vf0zw, $vf31zw
	vitof0.zw	$vf1zw, $vf2zw
	vitof0.zw	$vf31zw, $vf0zw
	vitof0.zw	$vf31zw, $vf15zw
	vitof0.zw	$vf31zw, $vf31zw
	vitof12.w	$vf0w, $vf0w
	vitof12.w	$vf0w, $vf31w
	vitof12.w	$vf1w, $vf2w
	vitof12.w	$vf31w, $vf0w
	vitof12.w	$vf31w, $vf15w
	vitof12.w	$vf31w, $vf31w
	vitof12.x	$vf0x, $vf0x
	vitof12.x	$vf0x, $vf31x
	vitof12.x	$vf1x, $vf2x
	vitof12.x	$vf31x, $vf0x
	vitof12.x	$vf31x, $vf15x
	vitof12.x	$vf31x, $vf31x
	vitof12.xw	$vf0xw, $vf0xw
	vitof12.xw	$vf0xw, $vf31xw
	vitof12.xw	$vf1xw, $vf2xw
	vitof12.xw	$vf31xw, $vf0xw
	vitof12.xw	$vf31xw, $vf15xw
	vitof12.xw	$vf31xw, $vf31xw
	vitof12.xy	$vf0xy, $vf0xy
	vitof12.xy	$vf0xy, $vf31xy
	vitof12.xy	$vf1xy, $vf2xy
	vitof12.xy	$vf31xy, $vf0xy
	vitof12.xy	$vf31xy, $vf15xy
	vitof12.xy	$vf31xy, $vf31xy
	vitof12.xyw	$vf0xyw, $vf0xyw
	vitof12.xyw	$vf0xyw, $vf31xyw
	vitof12.xyw	$vf1xyw, $vf2xyw
	vitof12.xyw	$vf31xyw, $vf0xyw
	vitof12.xyw	$vf31xyw, $vf15xyw
	vitof12.xyw	$vf31xyw, $vf31xyw
	vitof12.xyz	$vf0xyz, $vf0xyz
	vitof12.xyz	$vf0xyz, $vf31xyz
	vitof12.xyz	$vf1xyz, $vf2xyz
	vitof12.xyz	$vf31xyz, $vf0xyz
	vitof12.xyz	$vf31xyz, $vf15xyz
	vitof12.xyz	$vf31xyz, $vf31xyz
	vitof12.xyzw	$vf0xyzw, $vf0xyzw
	vitof12.xyzw	$vf0xyzw, $vf31xyzw
	vitof12.xyzw	$vf1xyzw, $vf2xyzw
	vitof12.xyzw	$vf31xyzw, $vf0xyzw
	vitof12.xyzw	$vf31xyzw, $vf15xyzw
	vitof12.xyzw	$vf31xyzw, $vf31xyzw
	vitof12.xz	$vf0xz, $vf0xz
	vitof12.xz	$vf0xz, $vf31xz
	vitof12.xz	$vf1xz, $vf2xz
	vitof12.xz	$vf31xz, $vf0xz
	vitof12.xz	$vf31xz, $vf15xz
	vitof12.xz	$vf31xz, $vf31xz
	vitof12.xzw	$vf0xzw, $vf0xzw
	vitof12.xzw	$vf0xzw, $vf31xzw
	vitof12.xzw	$vf1xzw, $vf2xzw
	vitof12.xzw	$vf31xzw, $vf0xzw
	vitof12.xzw	$vf31xzw, $vf15xzw
	vitof12.xzw	$vf31xzw, $vf31xzw
	vitof12.y	$vf0y, $vf0y
	vitof12.y	$vf0y, $vf31y
	vitof12.y	$vf1y, $vf2y
	vitof12.y	$vf31y, $vf0y
	vitof12.y	$vf31y, $vf15y
	vitof12.y	$vf31y, $vf31y
	vitof12.yw	$vf0yw, $vf0yw
	vitof12.yw	$vf0yw, $vf31yw
	vitof12.yw	$vf1yw, $vf2yw
	vitof12.yw	$vf31yw, $vf0yw
	vitof12.yw	$vf31yw, $vf15yw
	vitof12.yw	$vf31yw, $vf31yw
	vitof12.yz	$vf0yz, $vf0yz
	vitof12.yz	$vf0yz, $vf31yz
	vitof12.yz	$vf1yz, $vf2yz
	vitof12.yz	$vf31yz, $vf0yz
	vitof12.yz	$vf31yz, $vf15yz
	vitof12.yz	$vf31yz, $vf31yz
	vitof12.yzw	$vf0yzw, $vf0yzw
	vitof12.yzw	$vf0yzw, $vf31yzw
	vitof12.yzw	$vf1yzw, $vf2yzw
	vitof12.yzw	$vf31yzw, $vf0yzw
	vitof12.yzw	$vf31yzw, $vf15yzw
	vitof12.yzw	$vf31yzw, $vf31yzw
	vitof12.z	$vf0z, $vf0z
	vitof12.z	$vf0z, $vf31z
	vitof12.z	$vf1z, $vf2z
	vitof12.z	$vf31z, $vf0z
	vitof12.z	$vf31z, $vf15z
	vitof12.z	$vf31z, $vf31z
	vitof12.zw	$vf0zw, $vf0zw
	vitof12.zw	$vf0zw, $vf31zw
	vitof12.zw	$vf1zw, $vf2zw
	vitof12.zw	$vf31zw, $vf0zw
	vitof12.zw	$vf31zw, $vf15zw
	vitof12.zw	$vf31zw, $vf31zw
	vitof15.w	$vf0w, $vf0w
	vitof15.w	$vf0w, $vf31w
	vitof15.w	$vf1w, $vf2w
	vitof15.w	$vf31w, $vf0w
	vitof15.w	$vf31w, $vf15w
	vitof15.w	$vf31w, $vf31w
	vitof15.x	$vf0x, $vf0x
	vitof15.x	$vf0x, $vf31x
	vitof15.x	$vf1x, $vf2x
	vitof15.x	$vf31x, $vf0x
	vitof15.x	$vf31x, $vf15x
	vitof15.x	$vf31x, $vf31x
	vitof15.xw	$vf0xw, $vf0xw
	vitof15.xw	$vf0xw, $vf31xw
	vitof15.xw	$vf1xw, $vf2xw
	vitof15.xw	$vf31xw, $vf0xw
	vitof15.xw	$vf31xw, $vf15xw
	vitof15.xw	$vf31xw, $vf31xw
	vitof15.xy	$vf0xy, $vf0xy
	vitof15.xy	$vf0xy, $vf31xy
	vitof15.xy	$vf1xy, $vf2xy
	vitof15.xy	$vf31xy, $vf0xy
	vitof15.xy	$vf31xy, $vf15xy
	vitof15.xy	$vf31xy, $vf31xy
	vitof15.xyw	$vf0xyw, $vf0xyw
	vitof15.xyw	$vf0xyw, $vf31xyw
	vitof15.xyw	$vf1xyw, $vf2xyw
	vitof15.xyw	$vf31xyw, $vf0xyw
	vitof15.xyw	$vf31xyw, $vf15xyw
	vitof15.xyw	$vf31xyw, $vf31xyw
	vitof15.xyz	$vf0xyz, $vf0xyz
	vitof15.xyz	$vf0xyz, $vf31xyz
	vitof15.xyz	$vf1xyz, $vf2xyz
	vitof15.xyz	$vf31xyz, $vf0xyz
	vitof15.xyz	$vf31xyz, $vf15xyz
	vitof15.xyz	$vf31xyz, $vf31xyz
	vitof15.xyzw	$vf0xyzw, $vf0xyzw
	vitof15.xyzw	$vf0xyzw, $vf31xyzw
	vitof15.xyzw	$vf1xyzw, $vf2xyzw
	vitof15.xyzw	$vf31xyzw, $vf0xyzw
	vitof15.xyzw	$vf31xyzw, $vf15xyzw
	vitof15.xyzw	$vf31xyzw, $vf31xyzw
	vitof15.xz	$vf0xz, $vf0xz
	vitof15.xz	$vf0xz, $vf31xz
	vitof15.xz	$vf1xz, $vf2xz
	vitof15.xz	$vf31xz, $vf0xz
	vitof15.xz	$vf31xz, $vf15xz
	vitof15.xz	$vf31xz, $vf31xz
	vitof15.xzw	$vf0xzw, $vf0xzw
	vitof15.xzw	$vf0xzw, $vf31xzw
	vitof15.xzw	$vf1xzw, $vf2xzw
	vitof15.xzw	$vf31xzw, $vf0xzw
	vitof15.xzw	$vf31xzw, $vf15xzw
	vitof15.xzw	$vf31xzw, $vf31xzw
	vitof15.y	$vf0y, $vf0y
	vitof15.y	$vf0y, $vf31y
	vitof15.y	$vf1y, $vf2y
	vitof15.y	$vf31y, $vf0y
	vitof15.y	$vf31y, $vf15y
	vitof15.y	$vf31y, $vf31y
	vitof15.yw	$vf0yw, $vf0yw
	vitof15.yw	$vf0yw, $vf31yw
	vitof15.yw	$vf1yw, $vf2yw
	vitof15.yw	$vf31yw, $vf0yw
	vitof15.yw	$vf31yw, $vf15yw
	vitof15.yw	$vf31yw, $vf31yw
	vitof15.yz	$vf0yz, $vf0yz
	vitof15.yz	$vf0yz, $vf31yz
	vitof15.yz	$vf1yz, $vf2yz
	vitof15.yz	$vf31yz, $vf0yz
	vitof15.yz	$vf31yz, $vf15yz
	vitof15.yz	$vf31yz, $vf31yz
	vitof15.yzw	$vf0yzw, $vf0yzw
	vitof15.yzw	$vf0yzw, $vf31yzw
	vitof15.yzw	$vf1yzw, $vf2yzw
	vitof15.yzw	$vf31yzw, $vf0yzw
	vitof15.yzw	$vf31yzw, $vf15yzw
	vitof15.yzw	$vf31yzw, $vf31yzw
	vitof15.z	$vf0z, $vf0z
	vitof15.z	$vf0z, $vf31z
	vitof15.z	$vf1z, $vf2z
	vitof15.z	$vf31z, $vf0z
	vitof15.z	$vf31z, $vf15z
	vitof15.z	$vf31z, $vf31z
	vitof15.zw	$vf0zw, $vf0zw
	vitof15.zw	$vf0zw, $vf31zw
	vitof15.zw	$vf1zw, $vf2zw
	vitof15.zw	$vf31zw, $vf0zw
	vitof15.zw	$vf31zw, $vf15zw
	vitof15.zw	$vf31zw, $vf31zw
	vitof4.w	$vf0w, $vf0w
	vitof4.w	$vf0w, $vf31w
	vitof4.w	$vf1w, $vf2w
	vitof4.w	$vf31w, $vf0w
	vitof4.w	$vf31w, $vf15w
	vitof4.w	$vf31w, $vf31w
	vitof4.x	$vf0x, $vf0x
	vitof4.x	$vf0x, $vf31x
	vitof4.x	$vf1x, $vf2x
	vitof4.x	$vf31x, $vf0x
	vitof4.x	$vf31x, $vf15x
	vitof4.x	$vf31x, $vf31x
	vitof4.xw	$vf0xw, $vf0xw
	vitof4.xw	$vf0xw, $vf31xw
	vitof4.xw	$vf1xw, $vf2xw
	vitof4.xw	$vf31xw, $vf0xw
	vitof4.xw	$vf31xw, $vf15xw
	vitof4.xw	$vf31xw, $vf31xw
	vitof4.xy	$vf0xy, $vf0xy
	vitof4.xy	$vf0xy, $vf31xy
	vitof4.xy	$vf1xy, $vf2xy
	vitof4.xy	$vf31xy, $vf0xy
	vitof4.xy	$vf31xy, $vf15xy
	vitof4.xy	$vf31xy, $vf31xy
	vitof4.xyw	$vf0xyw, $vf0xyw
	vitof4.xyw	$vf0xyw, $vf31xyw
	vitof4.xyw	$vf1xyw, $vf2xyw
	vitof4.xyw	$vf31xyw, $vf0xyw
	vitof4.xyw	$vf31xyw, $vf15xyw
	vitof4.xyw	$vf31xyw, $vf31xyw
	vitof4.xyz	$vf0xyz, $vf0xyz
	vitof4.xyz	$vf0xyz, $vf31xyz
	vitof4.xyz	$vf1xyz, $vf2xyz
	vitof4.xyz	$vf31xyz, $vf0xyz
	vitof4.xyz	$vf31xyz, $vf15xyz
	vitof4.xyz	$vf31xyz, $vf31xyz
	vitof4.xyzw	$vf0xyzw, $vf0xyzw
	vitof4.xyzw	$vf0xyzw, $vf31xyzw
	vitof4.xyzw	$vf1xyzw, $vf2xyzw
	vitof4.xyzw	$vf31xyzw, $vf0xyzw
	vitof4.xyzw	$vf31xyzw, $vf15xyzw
	vitof4.xyzw	$vf31xyzw, $vf31xyzw
	vitof4.xz	$vf0xz, $vf0xz
	vitof4.xz	$vf0xz, $vf31xz
	vitof4.xz	$vf1xz, $vf2xz
	vitof4.xz	$vf31xz, $vf0xz
	vitof4.xz	$vf31xz, $vf15xz
	vitof4.xz	$vf31xz, $vf31xz
	vitof4.xzw	$vf0xzw, $vf0xzw
	vitof4.xzw	$vf0xzw, $vf31xzw
	vitof4.xzw	$vf1xzw, $vf2xzw
	vitof4.xzw	$vf31xzw, $vf0xzw
	vitof4.xzw	$vf31xzw, $vf15xzw
	vitof4.xzw	$vf31xzw, $vf31xzw
	vitof4.y	$vf0y, $vf0y
	vitof4.y	$vf0y, $vf31y
	vitof4.y	$vf1y, $vf2y
	vitof4.y	$vf31y, $vf0y
	vitof4.y	$vf31y, $vf15y
	vitof4.y	$vf31y, $vf31y
	vitof4.yw	$vf0yw, $vf0yw
	vitof4.yw	$vf0yw, $vf31yw
	vitof4.yw	$vf1yw, $vf2yw
	vitof4.yw	$vf31yw, $vf0yw
	vitof4.yw	$vf31yw, $vf15yw
	vitof4.yw	$vf31yw, $vf31yw
	vitof4.yz	$vf0yz, $vf0yz
	vitof4.yz	$vf0yz, $vf31yz
	vitof4.yz	$vf1yz, $vf2yz
	vitof4.yz	$vf31yz, $vf0yz
	vitof4.yz	$vf31yz, $vf15yz
	vitof4.yz	$vf31yz, $vf31yz
	vitof4.yzw	$vf0yzw, $vf0yzw
	vitof4.yzw	$vf0yzw, $vf31yzw
	vitof4.yzw	$vf1yzw, $vf2yzw
	vitof4.yzw	$vf31yzw, $vf0yzw
	vitof4.yzw	$vf31yzw, $vf15yzw
	vitof4.yzw	$vf31yzw, $vf31yzw
	vitof4.z	$vf0z, $vf0z
	vitof4.z	$vf0z, $vf31z
	vitof4.z	$vf1z, $vf2z
	vitof4.z	$vf31z, $vf0z
	vitof4.z	$vf31z, $vf15z
	vitof4.z	$vf31z, $vf31z
	vitof4.zw	$vf0zw, $vf0zw
	vitof4.zw	$vf0zw, $vf31zw
	vitof4.zw	$vf1zw, $vf2zw
	vitof4.zw	$vf31zw, $vf0zw
	vitof4.zw	$vf31zw, $vf15zw
	vitof4.zw	$vf31zw, $vf31zw
	vlqd.w		$vf0, (--$vi0)
	vlqd.w		$vf0, (--$vi31)
	vlqd.w		$vf0w, (--$vi0)
	vlqd.w		$vf1, (--$vi2)
	vlqd.w		$vf31, (--$vi0)
	vlqd.w		$vf31, (--$vi15)
	vlqd.w		$vf31, (--$vi31)
	vlqd.x		$vf0, (--$vi0)
	vlqd.x		$vf0, (--$vi31)
	vlqd.x		$vf0x, (--$vi0)
	vlqd.x		$vf1, (--$vi2)
	vlqd.x		$vf31, (--$vi0)
	vlqd.x		$vf31, (--$vi15)
	vlqd.x		$vf31, (--$vi31)
	vlqd.xw		$vf0, (--$vi0)
	vlqd.xw		$vf0, (--$vi31)
	vlqd.xw		$vf0xw, (--$vi0)
	vlqd.xw		$vf1, (--$vi2)
	vlqd.xw		$vf31, (--$vi0)
	vlqd.xw		$vf31, (--$vi15)
	vlqd.xw		$vf31, (--$vi31)
	vlqd.xy		$vf0, (--$vi0)
	vlqd.xy		$vf0, (--$vi31)
	vlqd.xy		$vf0xy, (--$vi0)
	vlqd.xy		$vf1, (--$vi2)
	vlqd.xy		$vf31, (--$vi0)
	vlqd.xy		$vf31, (--$vi15)
	vlqd.xy		$vf31, (--$vi31)
	vlqd.xyw	$vf0, (--$vi0)
	vlqd.xyw	$vf0, (--$vi31)
	vlqd.xyw	$vf0xyw, (--$vi0)
	vlqd.xyw	$vf1, (--$vi2)
	vlqd.xyw	$vf31, (--$vi0)
	vlqd.xyw	$vf31, (--$vi15)
	vlqd.xyw	$vf31, (--$vi31)
	vlqd.xyz	$vf0, (--$vi0)
	vlqd.xyz	$vf0, (--$vi31)
	vlqd.xyz	$vf0xyz, (--$vi0)
	vlqd.xyz	$vf1, (--$vi2)
	vlqd.xyz	$vf31, (--$vi0)
	vlqd.xyz	$vf31, (--$vi15)
	vlqd.xyz	$vf31, (--$vi31)
	vlqd.xyzw	$vf0, (--$vi0)
	vlqd.xyzw	$vf0, (--$vi31)
	vlqd.xyzw	$vf0xyzw, (--$vi0)
	vlqd.xyzw	$vf1, (--$vi2)
	vlqd.xyzw	$vf31, (--$vi0)
	vlqd.xyzw	$vf31, (--$vi15)
	vlqd.xyzw	$vf31, (--$vi31)
	vlqd.xz		$vf0, (--$vi0)
	vlqd.xz		$vf0, (--$vi31)
	vlqd.xz		$vf0xz, (--$vi0)
	vlqd.xz		$vf1, (--$vi2)
	vlqd.xz		$vf31, (--$vi0)
	vlqd.xz		$vf31, (--$vi15)
	vlqd.xz		$vf31, (--$vi31)
	vlqd.xzw	$vf0, (--$vi0)
	vlqd.xzw	$vf0, (--$vi31)
	vlqd.xzw	$vf0xzw, (--$vi0)
	vlqd.xzw	$vf1, (--$vi2)
	vlqd.xzw	$vf31, (--$vi0)
	vlqd.xzw	$vf31, (--$vi15)
	vlqd.xzw	$vf31, (--$vi31)
	vlqd.y		$vf0, (--$vi0)
	vlqd.y		$vf0, (--$vi31)
	vlqd.y		$vf0y, (--$vi0)
	vlqd.y		$vf1, (--$vi2)
	vlqd.y		$vf31, (--$vi0)
	vlqd.y		$vf31, (--$vi15)
	vlqd.y		$vf31, (--$vi31)
	vlqd.yw		$vf0, (--$vi0)
	vlqd.yw		$vf0, (--$vi31)
	vlqd.yw		$vf0yw, (--$vi0)
	vlqd.yw		$vf1, (--$vi2)
	vlqd.yw		$vf31, (--$vi0)
	vlqd.yw		$vf31, (--$vi15)
	vlqd.yw		$vf31, (--$vi31)
	vlqd.yz		$vf0, (--$vi0)
	vlqd.yz		$vf0, (--$vi31)
	vlqd.yz		$vf0yz, (--$vi0)
	vlqd.yz		$vf1, (--$vi2)
	vlqd.yz		$vf31, (--$vi0)
	vlqd.yz		$vf31, (--$vi15)
	vlqd.yz		$vf31, (--$vi31)
	vlqd.yzw	$vf0, (--$vi0)
	vlqd.yzw	$vf0, (--$vi31)
	vlqd.yzw	$vf0yzw, (--$vi0)
	vlqd.yzw	$vf1, (--$vi2)
	vlqd.yzw	$vf31, (--$vi0)
	vlqd.yzw	$vf31, (--$vi15)
	vlqd.yzw	$vf31, (--$vi31)
	vlqd.z		$vf0, (--$vi0)
	vlqd.z		$vf0, (--$vi31)
	vlqd.z		$vf0z, (--$vi0)
	vlqd.z		$vf1, (--$vi2)
	vlqd.z		$vf31, (--$vi0)
	vlqd.z		$vf31, (--$vi15)
	vlqd.z		$vf31, (--$vi31)
	vlqd.zw		$vf0, (--$vi0)
	vlqd.zw		$vf0, (--$vi31)
	vlqd.zw		$vf0zw, (--$vi0)
	vlqd.zw		$vf1, (--$vi2)
	vlqd.zw		$vf31, (--$vi0)
	vlqd.zw		$vf31, (--$vi15)
	vlqd.zw		$vf31, (--$vi31)
	vlqi.w		$vf0, ($vi0++)
	vlqi.w		$vf0, ($vi31++)
	vlqi.w		$vf0w, ($vi0++)
	vlqi.w		$vf1, ($vi2++)
	vlqi.w		$vf31, ($vi0++)
	vlqi.w		$vf31, ($vi15++)
	vlqi.w		$vf31, ($vi31++)
	vlqi.x		$vf0, ($vi0++)
	vlqi.x		$vf0, ($vi31++)
	vlqi.x		$vf0x, ($vi0++)
	vlqi.x		$vf1, ($vi2++)
	vlqi.x		$vf31, ($vi0++)
	vlqi.x		$vf31, ($vi15++)
	vlqi.x		$vf31, ($vi31++)
	vlqi.xw		$vf0, ($vi0++)
	vlqi.xw		$vf0, ($vi31++)
	vlqi.xw		$vf0xw, ($vi0++)
	vlqi.xw		$vf1, ($vi2++)
	vlqi.xw		$vf31, ($vi0++)
	vlqi.xw		$vf31, ($vi15++)
	vlqi.xw		$vf31, ($vi31++)
	vlqi.xy		$vf0, ($vi0++)
	vlqi.xy		$vf0, ($vi31++)
	vlqi.xy		$vf0xy, ($vi0++)
	vlqi.xy		$vf1, ($vi2++)
	vlqi.xy		$vf31, ($vi0++)
	vlqi.xy		$vf31, ($vi15++)
	vlqi.xy		$vf31, ($vi31++)
	vlqi.xyw	$vf0, ($vi0++)
	vlqi.xyw	$vf0, ($vi31++)
	vlqi.xyw	$vf0xyw, ($vi0++)
	vlqi.xyw	$vf1, ($vi2++)
	vlqi.xyw	$vf31, ($vi0++)
	vlqi.xyw	$vf31, ($vi15++)
	vlqi.xyw	$vf31, ($vi31++)
	vlqi.xyz	$vf0, ($vi0++)
	vlqi.xyz	$vf0, ($vi31++)
	vlqi.xyz	$vf0xyz, ($vi0++)
	vlqi.xyz	$vf1, ($vi2++)
	vlqi.xyz	$vf31, ($vi0++)
	vlqi.xyz	$vf31, ($vi15++)
	vlqi.xyz	$vf31, ($vi31++)
	vlqi.xyzw	$vf0, ($vi0++)
	vlqi.xyzw	$vf0, ($vi31++)
	vlqi.xyzw	$vf0xyzw, ($vi0++)
	vlqi.xyzw	$vf1, ($vi2++)
	vlqi.xyzw	$vf31, ($vi0++)
	vlqi.xyzw	$vf31, ($vi15++)
	vlqi.xyzw	$vf31, ($vi31++)
	vlqi.xz		$vf0, ($vi0++)
	vlqi.xz		$vf0, ($vi31++)
	vlqi.xz		$vf0xz, ($vi0++)
	vlqi.xz		$vf1, ($vi2++)
	vlqi.xz		$vf31, ($vi0++)
	vlqi.xz		$vf31, ($vi15++)
	vlqi.xz		$vf31, ($vi31++)
	vlqi.xzw	$vf0, ($vi0++)
	vlqi.xzw	$vf0, ($vi31++)
	vlqi.xzw	$vf0xzw, ($vi0++)
	vlqi.xzw	$vf1, ($vi2++)
	vlqi.xzw	$vf31, ($vi0++)
	vlqi.xzw	$vf31, ($vi15++)
	vlqi.xzw	$vf31, ($vi31++)
	vlqi.y		$vf0, ($vi0++)
	vlqi.y		$vf0, ($vi31++)
	vlqi.y		$vf0y, ($vi0++)
	vlqi.y		$vf1, ($vi2++)
	vlqi.y		$vf31, ($vi0++)
	vlqi.y		$vf31, ($vi15++)
	vlqi.y		$vf31, ($vi31++)
	vlqi.yw		$vf0, ($vi0++)
	vlqi.yw		$vf0, ($vi31++)
	vlqi.yw		$vf0yw, ($vi0++)
	vlqi.yw		$vf1, ($vi2++)
	vlqi.yw		$vf31, ($vi0++)
	vlqi.yw		$vf31, ($vi15++)
	vlqi.yw		$vf31, ($vi31++)
	vlqi.yz		$vf0, ($vi0++)
	vlqi.yz		$vf0, ($vi31++)
	vlqi.yz		$vf0yz, ($vi0++)
	vlqi.yz		$vf1, ($vi2++)
	vlqi.yz		$vf31, ($vi0++)
	vlqi.yz		$vf31, ($vi15++)
	vlqi.yz		$vf31, ($vi31++)
	vlqi.yzw	$vf0, ($vi0++)
	vlqi.yzw	$vf0, ($vi31++)
	vlqi.yzw	$vf0yzw, ($vi0++)
	vlqi.yzw	$vf1, ($vi2++)
	vlqi.yzw	$vf31, ($vi0++)
	vlqi.yzw	$vf31, ($vi15++)
	vlqi.yzw	$vf31, ($vi31++)
	vlqi.z		$vf0, ($vi0++)
	vlqi.z		$vf0, ($vi31++)
	vlqi.z		$vf0z, ($vi0++)
	vlqi.z		$vf1, ($vi2++)
	vlqi.z		$vf31, ($vi0++)
	vlqi.z		$vf31, ($vi15++)
	vlqi.z		$vf31, ($vi31++)
	vlqi.zw		$vf0, ($vi0++)
	vlqi.zw		$vf0, ($vi31++)
	vlqi.zw		$vf0zw, ($vi0++)
	vlqi.zw		$vf1, ($vi2++)
	vlqi.zw		$vf31, ($vi0++)
	vlqi.zw		$vf31, ($vi15++)
	vlqi.zw		$vf31, ($vi31++)
	vmaddai.w	$ACCw, $vf0w, $I
	vmaddai.w	$ACCw, $vf1w, $I
	vmaddai.w	$ACCw, $vf31w, $I
	vmaddai.x	$ACCx, $vf0x, $I
	vmaddai.x	$ACCx, $vf1x, $I
	vmaddai.x	$ACCx, $vf31x, $I
	vmaddai.xw	$ACCxw, $vf0xw, $I
	vmaddai.xw	$ACCxw, $vf1xw, $I
	vmaddai.xw	$ACCxw, $vf31xw, $I
	vmaddai.xy	$ACCxy, $vf0xy, $I
	vmaddai.xy	$ACCxy, $vf1xy, $I
	vmaddai.xy	$ACCxy, $vf31xy, $I
	vmaddai.xyw	$ACCxyw, $vf0xyw, $I
	vmaddai.xyw	$ACCxyw, $vf1xyw, $I
	vmaddai.xyw	$ACCxyw, $vf31xyw, $I
	vmaddai.xyz	$ACCxyz, $vf0xyz, $I
	vmaddai.xyz	$ACCxyz, $vf1xyz, $I
	vmaddai.xyz	$ACCxyz, $vf31xyz, $I
	vmaddai.xyzw	$ACCxyzw, $vf0xyzw, $I
	vmaddai.xyzw	$ACCxyzw, $vf1xyzw, $I
	vmaddai.xyzw	$ACCxyzw, $vf31xyzw, $I
	vmaddai.xz	$ACCxz, $vf0xz, $I
	vmaddai.xz	$ACCxz, $vf1xz, $I
	vmaddai.xz	$ACCxz, $vf31xz, $I
	vmaddai.xzw	$ACCxzw, $vf0xzw, $I
	vmaddai.xzw	$ACCxzw, $vf1xzw, $I
	vmaddai.xzw	$ACCxzw, $vf31xzw, $I
	vmaddai.y	$ACCy, $vf0y, $I
	vmaddai.y	$ACCy, $vf1y, $I
	vmaddai.y	$ACCy, $vf31y, $I
	vmaddai.yw	$ACCyw, $vf0yw, $I
	vmaddai.yw	$ACCyw, $vf1yw, $I
	vmaddai.yw	$ACCyw, $vf31yw, $I
	vmaddai.yz	$ACCyz, $vf0yz, $I
	vmaddai.yz	$ACCyz, $vf1yz, $I
	vmaddai.yz	$ACCyz, $vf31yz, $I
	vmaddai.yzw	$ACCyzw, $vf0yzw, $I
	vmaddai.yzw	$ACCyzw, $vf1yzw, $I
	vmaddai.yzw	$ACCyzw, $vf31yzw, $I
	vmaddai.z	$ACCz, $vf0z, $I
	vmaddai.z	$ACCz, $vf1z, $I
	vmaddai.z	$ACCz, $vf31z, $I
	vmaddai.zw	$ACCzw, $vf0zw, $I
	vmaddai.zw	$ACCzw, $vf1zw, $I
	vmaddai.zw	$ACCzw, $vf31zw, $I
	vmaddaq.w	$ACCw, $vf0w, $Q
	vmaddaq.w	$ACCw, $vf1w, $Q
	vmaddaq.w	$ACCw, $vf31w, $Q
	vmaddaq.x	$ACCx, $vf0x, $Q
	vmaddaq.x	$ACCx, $vf1x, $Q
	vmaddaq.x	$ACCx, $vf31x, $Q
	vmaddaq.xw	$ACCxw, $vf0xw, $Q
	vmaddaq.xw	$ACCxw, $vf1xw, $Q
	vmaddaq.xw	$ACCxw, $vf31xw, $Q
	vmaddaq.xy	$ACCxy, $vf0xy, $Q
	vmaddaq.xy	$ACCxy, $vf1xy, $Q
	vmaddaq.xy	$ACCxy, $vf31xy, $Q
	vmaddaq.xyw	$ACCxyw, $vf0xyw, $Q
	vmaddaq.xyw	$ACCxyw, $vf1xyw, $Q
	vmaddaq.xyw	$ACCxyw, $vf31xyw, $Q
	vmaddaq.xyz	$ACCxyz, $vf0xyz, $Q
	vmaddaq.xyz	$ACCxyz, $vf1xyz, $Q
	vmaddaq.xyz	$ACCxyz, $vf31xyz, $Q
	vmaddaq.xyzw	$ACCxyzw, $vf0xyzw, $Q
	vmaddaq.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vmaddaq.xyzw	$ACCxyzw, $vf31xyzw, $Q
	vmaddaq.xz	$ACCxz, $vf0xz, $Q
	vmaddaq.xz	$ACCxz, $vf1xz, $Q
	vmaddaq.xz	$ACCxz, $vf31xz, $Q
	vmaddaq.xzw	$ACCxzw, $vf0xzw, $Q
	vmaddaq.xzw	$ACCxzw, $vf1xzw, $Q
	vmaddaq.xzw	$ACCxzw, $vf31xzw, $Q
	vmaddaq.y	$ACCy, $vf0y, $Q
	vmaddaq.y	$ACCy, $vf1y, $Q
	vmaddaq.y	$ACCy, $vf31y, $Q
	vmaddaq.yw	$ACCyw, $vf0yw, $Q
	vmaddaq.yw	$ACCyw, $vf1yw, $Q
	vmaddaq.yw	$ACCyw, $vf31yw, $Q
	vmaddaq.yz	$ACCyz, $vf0yz, $Q
	vmaddaq.yz	$ACCyz, $vf1yz, $Q
	vmaddaq.yz	$ACCyz, $vf31yz, $Q
	vmaddaq.yzw	$ACCyzw, $vf0yzw, $Q
	vmaddaq.yzw	$ACCyzw, $vf1yzw, $Q
	vmaddaq.yzw	$ACCyzw, $vf31yzw, $Q
	vmaddaq.z	$ACCz, $vf0z, $Q
	vmaddaq.z	$ACCz, $vf1z, $Q
	vmaddaq.z	$ACCz, $vf31z, $Q
	vmaddaq.zw	$ACCzw, $vf0zw, $Q
	vmaddaq.zw	$ACCzw, $vf1zw, $Q
	vmaddaq.zw	$ACCzw, $vf31zw, $Q
	vmadda.w	$ACCw, $vf0w, $vf0w
	vmadda.w	$ACCw, $vf0w, $vf31w
	vmadda.w	$ACCw, $vf1w, $vf2w
	vmadda.w	$ACCw, $vf31w, $vf0w
	vmadda.w	$ACCw, $vf31w, $vf15w
	vmadda.w	$ACCw, $vf31w, $vf31w
	vmaddaw.w	$ACCw, $vf0w, $vf0w
	vmaddaw.w	$ACCw, $vf0w, $vf31w
	vmaddaw.w	$ACCw, $vf1w, $vf2w
	vmaddaw.w	$ACCw, $vf31w, $vf0w
	vmaddaw.w	$ACCw, $vf31w, $vf15w
	vmaddaw.w	$ACCw, $vf31w, $vf31w
	vmaddaw.x	$ACCx, $vf0x, $vf0w
	vmaddaw.x	$ACCx, $vf0x, $vf31w
	vmaddaw.x	$ACCx, $vf1x, $vf2w
	vmaddaw.x	$ACCx, $vf31x, $vf0w
	vmaddaw.x	$ACCx, $vf31x, $vf15w
	vmaddaw.x	$ACCx, $vf31x, $vf31w
	vmaddaw.xw	$ACCxw, $vf0xw, $vf0w
	vmaddaw.xw	$ACCxw, $vf0xw, $vf31w
	vmaddaw.xw	$ACCxw, $vf1xw, $vf2w
	vmaddaw.xw	$ACCxw, $vf31xw, $vf0w
	vmaddaw.xw	$ACCxw, $vf31xw, $vf15w
	vmaddaw.xw	$ACCxw, $vf31xw, $vf31w
	vmaddaw.xy	$ACCxy, $vf0xy, $vf0w
	vmaddaw.xy	$ACCxy, $vf0xy, $vf31w
	vmaddaw.xy	$ACCxy, $vf1xy, $vf2w
	vmaddaw.xy	$ACCxy, $vf31xy, $vf0w
	vmaddaw.xy	$ACCxy, $vf31xy, $vf15w
	vmaddaw.xy	$ACCxy, $vf31xy, $vf31w
	vmaddaw.xyw	$ACCxyw, $vf0xyw, $vf0w
	vmaddaw.xyw	$ACCxyw, $vf0xyw, $vf31w
	vmaddaw.xyw	$ACCxyw, $vf1xyw, $vf2w
	vmaddaw.xyw	$ACCxyw, $vf31xyw, $vf0w
	vmaddaw.xyw	$ACCxyw, $vf31xyw, $vf15w
	vmaddaw.xyw	$ACCxyw, $vf31xyw, $vf31w
	vmaddaw.xyz	$ACCxyz, $vf0xyz, $vf0w
	vmaddaw.xyz	$ACCxyz, $vf0xyz, $vf31w
	vmaddaw.xyz	$ACCxyz, $vf1xyz, $vf2w
	vmaddaw.xyz	$ACCxyz, $vf31xyz, $vf0w
	vmaddaw.xyz	$ACCxyz, $vf31xyz, $vf15w
	vmaddaw.xyz	$ACCxyz, $vf31xyz, $vf31w
	vmaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0w
	vmaddaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31w
	vmaddaw.xyzw	$ACCxyzw, $vf1xyzw, $vf2w
	vmaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf0w
	vmaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf15w
	vmaddaw.xyzw	$ACCxyzw, $vf31xyzw, $vf31w
	vmaddaw.xz	$ACCxz, $vf0xz, $vf0w
	vmaddaw.xz	$ACCxz, $vf0xz, $vf31w
	vmaddaw.xz	$ACCxz, $vf1xz, $vf2w
	vmaddaw.xz	$ACCxz, $vf31xz, $vf0w
	vmaddaw.xz	$ACCxz, $vf31xz, $vf15w
	vmaddaw.xz	$ACCxz, $vf31xz, $vf31w
	vmaddaw.xzw	$ACCxzw, $vf0xzw, $vf0w
	vmaddaw.xzw	$ACCxzw, $vf0xzw, $vf31w
	vmaddaw.xzw	$ACCxzw, $vf1xzw, $vf2w
	vmaddaw.xzw	$ACCxzw, $vf31xzw, $vf0w
	vmaddaw.xzw	$ACCxzw, $vf31xzw, $vf15w
	vmaddaw.xzw	$ACCxzw, $vf31xzw, $vf31w
	vmaddaw.y	$ACCy, $vf0y, $vf0w
	vmaddaw.y	$ACCy, $vf0y, $vf31w
	vmaddaw.y	$ACCy, $vf1y, $vf2w
	vmaddaw.y	$ACCy, $vf31y, $vf0w
	vmaddaw.y	$ACCy, $vf31y, $vf15w
	vmaddaw.y	$ACCy, $vf31y, $vf31w
	vmaddaw.yw	$ACCyw, $vf0yw, $vf0w
	vmaddaw.yw	$ACCyw, $vf0yw, $vf31w
	vmaddaw.yw	$ACCyw, $vf1yw, $vf2w
	vmaddaw.yw	$ACCyw, $vf31yw, $vf0w
	vmaddaw.yw	$ACCyw, $vf31yw, $vf15w
	vmaddaw.yw	$ACCyw, $vf31yw, $vf31w
	vmaddaw.yz	$ACCyz, $vf0yz, $vf0w
	vmaddaw.yz	$ACCyz, $vf0yz, $vf31w
	vmaddaw.yz	$ACCyz, $vf1yz, $vf2w
	vmaddaw.yz	$ACCyz, $vf31yz, $vf0w
	vmaddaw.yz	$ACCyz, $vf31yz, $vf15w
	vmaddaw.yz	$ACCyz, $vf31yz, $vf31w
	vmaddaw.yzw	$ACCyzw, $vf0yzw, $vf0w
	vmaddaw.yzw	$ACCyzw, $vf0yzw, $vf31w
	vmaddaw.yzw	$ACCyzw, $vf1yzw, $vf2w
	vmaddaw.yzw	$ACCyzw, $vf31yzw, $vf0w
	vmaddaw.yzw	$ACCyzw, $vf31yzw, $vf15w
	vmaddaw.yzw	$ACCyzw, $vf31yzw, $vf31w
	vmaddaw.z	$ACCz, $vf0z, $vf0w
	vmaddaw.z	$ACCz, $vf0z, $vf31w
	vmaddaw.z	$ACCz, $vf1z, $vf2w
	vmaddaw.z	$ACCz, $vf31z, $vf0w
	vmaddaw.z	$ACCz, $vf31z, $vf15w
	vmaddaw.z	$ACCz, $vf31z, $vf31w
	vmaddaw.zw	$ACCzw, $vf0zw, $vf0w
	vmaddaw.zw	$ACCzw, $vf0zw, $vf31w
	vmaddaw.zw	$ACCzw, $vf1zw, $vf2w
	vmaddaw.zw	$ACCzw, $vf31zw, $vf0w
	vmaddaw.zw	$ACCzw, $vf31zw, $vf15w
	vmaddaw.zw	$ACCzw, $vf31zw, $vf31w
	vmadda.x	$ACCx, $vf0x, $vf0x
	vmadda.x	$ACCx, $vf0x, $vf31x
	vmadda.x	$ACCx, $vf1x, $vf2x
	vmadda.x	$ACCx, $vf31x, $vf0x
	vmadda.x	$ACCx, $vf31x, $vf15x
	vmadda.x	$ACCx, $vf31x, $vf31x
	vmaddax.w	$ACCw, $vf0w, $vf0x
	vmaddax.w	$ACCw, $vf0w, $vf31x
	vmaddax.w	$ACCw, $vf1w, $vf2x
	vmaddax.w	$ACCw, $vf31w, $vf0x
	vmaddax.w	$ACCw, $vf31w, $vf15x
	vmaddax.w	$ACCw, $vf31w, $vf31x
	vmadda.xw	$ACCxw, $vf0xw, $vf0xw
	vmadda.xw	$ACCxw, $vf0xw, $vf31xw
	vmadda.xw	$ACCxw, $vf1xw, $vf2xw
	vmadda.xw	$ACCxw, $vf31xw, $vf0xw
	vmadda.xw	$ACCxw, $vf31xw, $vf15xw
	vmadda.xw	$ACCxw, $vf31xw, $vf31xw
	vmaddax.x	$ACCx, $vf0x, $vf0x
	vmaddax.x	$ACCx, $vf0x, $vf31x
	vmaddax.x	$ACCx, $vf1x, $vf2x
	vmaddax.x	$ACCx, $vf31x, $vf0x
	vmaddax.x	$ACCx, $vf31x, $vf15x
	vmaddax.x	$ACCx, $vf31x, $vf31x
	vmaddax.xw	$ACCxw, $vf0xw, $vf0x
	vmaddax.xw	$ACCxw, $vf0xw, $vf31x
	vmaddax.xw	$ACCxw, $vf1xw, $vf2x
	vmaddax.xw	$ACCxw, $vf31xw, $vf0x
	vmaddax.xw	$ACCxw, $vf31xw, $vf15x
	vmaddax.xw	$ACCxw, $vf31xw, $vf31x
	vmaddax.xy	$ACCxy, $vf0xy, $vf0x
	vmaddax.xy	$ACCxy, $vf0xy, $vf31x
	vmaddax.xy	$ACCxy, $vf1xy, $vf2x
	vmaddax.xy	$ACCxy, $vf31xy, $vf0x
	vmaddax.xy	$ACCxy, $vf31xy, $vf15x
	vmaddax.xy	$ACCxy, $vf31xy, $vf31x
	vmaddax.xyw	$ACCxyw, $vf0xyw, $vf0x
	vmaddax.xyw	$ACCxyw, $vf0xyw, $vf31x
	vmaddax.xyw	$ACCxyw, $vf1xyw, $vf2x
	vmaddax.xyw	$ACCxyw, $vf31xyw, $vf0x
	vmaddax.xyw	$ACCxyw, $vf31xyw, $vf15x
	vmaddax.xyw	$ACCxyw, $vf31xyw, $vf31x
	vmaddax.xyz	$ACCxyz, $vf0xyz, $vf0x
	vmaddax.xyz	$ACCxyz, $vf0xyz, $vf31x
	vmaddax.xyz	$ACCxyz, $vf1xyz, $vf2x
	vmaddax.xyz	$ACCxyz, $vf31xyz, $vf0x
	vmaddax.xyz	$ACCxyz, $vf31xyz, $vf15x
	vmaddax.xyz	$ACCxyz, $vf31xyz, $vf31x
	vmaddax.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vmaddax.xyzw	$ACCxyzw, $vf0xyzw, $vf31x
	vmaddax.xyzw	$ACCxyzw, $vf1xyzw, $vf2x
	vmaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf0x
	vmaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf15x
	vmaddax.xyzw	$ACCxyzw, $vf31xyzw, $vf31x
	vmaddax.xz	$ACCxz, $vf0xz, $vf0x
	vmaddax.xz	$ACCxz, $vf0xz, $vf31x
	vmaddax.xz	$ACCxz, $vf1xz, $vf2x
	vmaddax.xz	$ACCxz, $vf31xz, $vf0x
	vmaddax.xz	$ACCxz, $vf31xz, $vf15x
	vmaddax.xz	$ACCxz, $vf31xz, $vf31x
	vmaddax.xzw	$ACCxzw, $vf0xzw, $vf0x
	vmaddax.xzw	$ACCxzw, $vf0xzw, $vf31x
	vmaddax.xzw	$ACCxzw, $vf1xzw, $vf2x
	vmaddax.xzw	$ACCxzw, $vf31xzw, $vf0x
	vmaddax.xzw	$ACCxzw, $vf31xzw, $vf15x
	vmaddax.xzw	$ACCxzw, $vf31xzw, $vf31x
	vmadda.xy	$ACCxy, $vf0xy, $vf0xy
	vmadda.xy	$ACCxy, $vf0xy, $vf31xy
	vmadda.xy	$ACCxy, $vf1xy, $vf2xy
	vmadda.xy	$ACCxy, $vf31xy, $vf0xy
	vmadda.xy	$ACCxy, $vf31xy, $vf15xy
	vmadda.xy	$ACCxy, $vf31xy, $vf31xy
	vmaddax.y	$ACCy, $vf0y, $vf0x
	vmaddax.y	$ACCy, $vf0y, $vf31x
	vmaddax.y	$ACCy, $vf1y, $vf2x
	vmaddax.y	$ACCy, $vf31y, $vf0x
	vmaddax.y	$ACCy, $vf31y, $vf15x
	vmaddax.y	$ACCy, $vf31y, $vf31x
	vmadda.xyw	$ACCxyw, $vf0xyw, $vf0xyw
	vmadda.xyw	$ACCxyw, $vf0xyw, $vf31xyw
	vmadda.xyw	$ACCxyw, $vf1xyw, $vf2xyw
	vmadda.xyw	$ACCxyw, $vf31xyw, $vf0xyw
	vmadda.xyw	$ACCxyw, $vf31xyw, $vf15xyw
	vmadda.xyw	$ACCxyw, $vf31xyw, $vf31xyw
	vmaddax.yw	$ACCyw, $vf0yw, $vf0x
	vmaddax.yw	$ACCyw, $vf0yw, $vf31x
	vmaddax.yw	$ACCyw, $vf1yw, $vf2x
	vmaddax.yw	$ACCyw, $vf31yw, $vf0x
	vmaddax.yw	$ACCyw, $vf31yw, $vf15x
	vmaddax.yw	$ACCyw, $vf31yw, $vf31x
	vmadda.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vmadda.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vmadda.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vmadda.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vmadda.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vmadda.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vmaddax.yz	$ACCyz, $vf0yz, $vf0x
	vmaddax.yz	$ACCyz, $vf0yz, $vf31x
	vmaddax.yz	$ACCyz, $vf1yz, $vf2x
	vmaddax.yz	$ACCyz, $vf31yz, $vf0x
	vmaddax.yz	$ACCyz, $vf31yz, $vf15x
	vmaddax.yz	$ACCyz, $vf31yz, $vf31x
	vmadda.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vmadda.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vmadda.xyzw	$ACCxyzw, $vf1xyzw, $vf2xyzw
	vmadda.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vmadda.xyzw	$ACCxyzw, $vf31xyzw, $vf15xyzw
	vmadda.xyzw	$ACCxyzw, $vf31xyzw, $vf31xyzw
	vmaddax.yzw	$ACCyzw, $vf0yzw, $vf0x
	vmaddax.yzw	$ACCyzw, $vf0yzw, $vf31x
	vmaddax.yzw	$ACCyzw, $vf1yzw, $vf2x
	vmaddax.yzw	$ACCyzw, $vf31yzw, $vf0x
	vmaddax.yzw	$ACCyzw, $vf31yzw, $vf15x
	vmaddax.yzw	$ACCyzw, $vf31yzw, $vf31x
	vmadda.xz	$ACCxz, $vf0xz, $vf0xz
	vmadda.xz	$ACCxz, $vf0xz, $vf31xz
	vmadda.xz	$ACCxz, $vf1xz, $vf2xz
	vmadda.xz	$ACCxz, $vf31xz, $vf0xz
	vmadda.xz	$ACCxz, $vf31xz, $vf15xz
	vmadda.xz	$ACCxz, $vf31xz, $vf31xz
	vmaddax.z	$ACCz, $vf0z, $vf0x
	vmaddax.z	$ACCz, $vf0z, $vf31x
	vmaddax.z	$ACCz, $vf1z, $vf2x
	vmaddax.z	$ACCz, $vf31z, $vf0x
	vmaddax.z	$ACCz, $vf31z, $vf15x
	vmaddax.z	$ACCz, $vf31z, $vf31x
	vmadda.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vmadda.xzw	$ACCxzw, $vf0xzw, $vf31xzw
	vmadda.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vmadda.xzw	$ACCxzw, $vf31xzw, $vf0xzw
	vmadda.xzw	$ACCxzw, $vf31xzw, $vf15xzw
	vmadda.xzw	$ACCxzw, $vf31xzw, $vf31xzw
	vmaddax.zw	$ACCzw, $vf0zw, $vf0x
	vmaddax.zw	$ACCzw, $vf0zw, $vf31x
	vmaddax.zw	$ACCzw, $vf1zw, $vf2x
	vmaddax.zw	$ACCzw, $vf31zw, $vf0x
	vmaddax.zw	$ACCzw, $vf31zw, $vf15x
	vmaddax.zw	$ACCzw, $vf31zw, $vf31x
	vmadda.y	$ACCy, $vf0y, $vf0y
	vmadda.y	$ACCy, $vf0y, $vf31y
	vmadda.y	$ACCy, $vf1y, $vf2y
	vmadda.y	$ACCy, $vf31y, $vf0y
	vmadda.y	$ACCy, $vf31y, $vf15y
	vmadda.y	$ACCy, $vf31y, $vf31y
	vmadday.w	$ACCw, $vf0w, $vf0y
	vmadday.w	$ACCw, $vf0w, $vf31y
	vmadday.w	$ACCw, $vf1w, $vf2y
	vmadday.w	$ACCw, $vf31w, $vf0y
	vmadday.w	$ACCw, $vf31w, $vf15y
	vmadday.w	$ACCw, $vf31w, $vf31y
	vmadda.yw	$ACCyw, $vf0yw, $vf0yw
	vmadda.yw	$ACCyw, $vf0yw, $vf31yw
	vmadda.yw	$ACCyw, $vf1yw, $vf2yw
	vmadda.yw	$ACCyw, $vf31yw, $vf0yw
	vmadda.yw	$ACCyw, $vf31yw, $vf15yw
	vmadda.yw	$ACCyw, $vf31yw, $vf31yw
	vmadday.x	$ACCx, $vf0x, $vf0y
	vmadday.x	$ACCx, $vf0x, $vf31y
	vmadday.x	$ACCx, $vf1x, $vf2y
	vmadday.x	$ACCx, $vf31x, $vf0y
	vmadday.x	$ACCx, $vf31x, $vf15y
	vmadday.x	$ACCx, $vf31x, $vf31y
	vmadday.xw	$ACCxw, $vf0xw, $vf0y
	vmadday.xw	$ACCxw, $vf0xw, $vf31y
	vmadday.xw	$ACCxw, $vf1xw, $vf2y
	vmadday.xw	$ACCxw, $vf31xw, $vf0y
	vmadday.xw	$ACCxw, $vf31xw, $vf15y
	vmadday.xw	$ACCxw, $vf31xw, $vf31y
	vmadday.xy	$ACCxy, $vf0xy, $vf0y
	vmadday.xy	$ACCxy, $vf0xy, $vf31y
	vmadday.xy	$ACCxy, $vf1xy, $vf2y
	vmadday.xy	$ACCxy, $vf31xy, $vf0y
	vmadday.xy	$ACCxy, $vf31xy, $vf15y
	vmadday.xy	$ACCxy, $vf31xy, $vf31y
	vmadday.xyw	$ACCxyw, $vf0xyw, $vf0y
	vmadday.xyw	$ACCxyw, $vf0xyw, $vf31y
	vmadday.xyw	$ACCxyw, $vf1xyw, $vf2y
	vmadday.xyw	$ACCxyw, $vf31xyw, $vf0y
	vmadday.xyw	$ACCxyw, $vf31xyw, $vf15y
	vmadday.xyw	$ACCxyw, $vf31xyw, $vf31y
	vmadday.xyz	$ACCxyz, $vf0xyz, $vf0y
	vmadday.xyz	$ACCxyz, $vf0xyz, $vf31y
	vmadday.xyz	$ACCxyz, $vf1xyz, $vf2y
	vmadday.xyz	$ACCxyz, $vf31xyz, $vf0y
	vmadday.xyz	$ACCxyz, $vf31xyz, $vf15y
	vmadday.xyz	$ACCxyz, $vf31xyz, $vf31y
	vmadday.xyzw	$ACCxyzw, $vf0xyzw, $vf0y
	vmadday.xyzw	$ACCxyzw, $vf0xyzw, $vf31y
	vmadday.xyzw	$ACCxyzw, $vf1xyzw, $vf2y
	vmadday.xyzw	$ACCxyzw, $vf31xyzw, $vf0y
	vmadday.xyzw	$ACCxyzw, $vf31xyzw, $vf15y
	vmadday.xyzw	$ACCxyzw, $vf31xyzw, $vf31y
	vmadday.xz	$ACCxz, $vf0xz, $vf0y
	vmadday.xz	$ACCxz, $vf0xz, $vf31y
	vmadday.xz	$ACCxz, $vf1xz, $vf2y
	vmadday.xz	$ACCxz, $vf31xz, $vf0y
	vmadday.xz	$ACCxz, $vf31xz, $vf15y
	vmadday.xz	$ACCxz, $vf31xz, $vf31y
	vmadday.xzw	$ACCxzw, $vf0xzw, $vf0y
	vmadday.xzw	$ACCxzw, $vf0xzw, $vf31y
	vmadday.xzw	$ACCxzw, $vf1xzw, $vf2y
	vmadday.xzw	$ACCxzw, $vf31xzw, $vf0y
	vmadday.xzw	$ACCxzw, $vf31xzw, $vf15y
	vmadday.xzw	$ACCxzw, $vf31xzw, $vf31y
	vmadday.y	$ACCy, $vf0y, $vf0y
	vmadday.y	$ACCy, $vf0y, $vf31y
	vmadday.y	$ACCy, $vf1y, $vf2y
	vmadday.y	$ACCy, $vf31y, $vf0y
	vmadday.y	$ACCy, $vf31y, $vf15y
	vmadday.y	$ACCy, $vf31y, $vf31y
	vmadday.yw	$ACCyw, $vf0yw, $vf0y
	vmadday.yw	$ACCyw, $vf0yw, $vf31y
	vmadday.yw	$ACCyw, $vf1yw, $vf2y
	vmadday.yw	$ACCyw, $vf31yw, $vf0y
	vmadday.yw	$ACCyw, $vf31yw, $vf15y
	vmadday.yw	$ACCyw, $vf31yw, $vf31y
	vmadday.yz	$ACCyz, $vf0yz, $vf0y
	vmadday.yz	$ACCyz, $vf0yz, $vf31y
	vmadday.yz	$ACCyz, $vf1yz, $vf2y
	vmadday.yz	$ACCyz, $vf31yz, $vf0y
	vmadday.yz	$ACCyz, $vf31yz, $vf15y
	vmadday.yz	$ACCyz, $vf31yz, $vf31y
	vmadday.yzw	$ACCyzw, $vf0yzw, $vf0y
	vmadday.yzw	$ACCyzw, $vf0yzw, $vf31y
	vmadday.yzw	$ACCyzw, $vf1yzw, $vf2y
	vmadday.yzw	$ACCyzw, $vf31yzw, $vf0y
	vmadday.yzw	$ACCyzw, $vf31yzw, $vf15y
	vmadday.yzw	$ACCyzw, $vf31yzw, $vf31y
	vmadda.yz	$ACCyz, $vf0yz, $vf0yz
	vmadda.yz	$ACCyz, $vf0yz, $vf31yz
	vmadda.yz	$ACCyz, $vf1yz, $vf2yz
	vmadda.yz	$ACCyz, $vf31yz, $vf0yz
	vmadda.yz	$ACCyz, $vf31yz, $vf15yz
	vmadda.yz	$ACCyz, $vf31yz, $vf31yz
	vmadday.z	$ACCz, $vf0z, $vf0y
	vmadday.z	$ACCz, $vf0z, $vf31y
	vmadday.z	$ACCz, $vf1z, $vf2y
	vmadday.z	$ACCz, $vf31z, $vf0y
	vmadday.z	$ACCz, $vf31z, $vf15y
	vmadday.z	$ACCz, $vf31z, $vf31y
	vmadda.yzw	$ACCyzw, $vf0yzw, $vf0yzw
	vmadda.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vmadda.yzw	$ACCyzw, $vf1yzw, $vf2yzw
	vmadda.yzw	$ACCyzw, $vf31yzw, $vf0yzw
	vmadda.yzw	$ACCyzw, $vf31yzw, $vf15yzw
	vmadda.yzw	$ACCyzw, $vf31yzw, $vf31yzw
	vmadday.zw	$ACCzw, $vf0zw, $vf0y
	vmadday.zw	$ACCzw, $vf0zw, $vf31y
	vmadday.zw	$ACCzw, $vf1zw, $vf2y
	vmadday.zw	$ACCzw, $vf31zw, $vf0y
	vmadday.zw	$ACCzw, $vf31zw, $vf15y
	vmadday.zw	$ACCzw, $vf31zw, $vf31y
	vmadda.z	$ACCz, $vf0z, $vf0z
	vmadda.z	$ACCz, $vf0z, $vf31z
	vmadda.z	$ACCz, $vf1z, $vf2z
	vmadda.z	$ACCz, $vf31z, $vf0z
	vmadda.z	$ACCz, $vf31z, $vf15z
	vmadda.z	$ACCz, $vf31z, $vf31z
	vmaddaz.w	$ACCw, $vf0w, $vf0z
	vmaddaz.w	$ACCw, $vf0w, $vf31z
	vmaddaz.w	$ACCw, $vf1w, $vf2z
	vmaddaz.w	$ACCw, $vf31w, $vf0z
	vmaddaz.w	$ACCw, $vf31w, $vf15z
	vmaddaz.w	$ACCw, $vf31w, $vf31z
	vmadda.zw	$ACCzw, $vf0zw, $vf0zw
	vmadda.zw	$ACCzw, $vf0zw, $vf31zw
	vmadda.zw	$ACCzw, $vf1zw, $vf2zw
	vmadda.zw	$ACCzw, $vf31zw, $vf0zw
	vmadda.zw	$ACCzw, $vf31zw, $vf15zw
	vmadda.zw	$ACCzw, $vf31zw, $vf31zw
	vmaddaz.x	$ACCx, $vf0x, $vf0z
	vmaddaz.x	$ACCx, $vf0x, $vf31z
	vmaddaz.x	$ACCx, $vf1x, $vf2z
	vmaddaz.x	$ACCx, $vf31x, $vf0z
	vmaddaz.x	$ACCx, $vf31x, $vf15z
	vmaddaz.x	$ACCx, $vf31x, $vf31z
	vmaddaz.xw	$ACCxw, $vf0xw, $vf0z
	vmaddaz.xw	$ACCxw, $vf0xw, $vf31z
	vmaddaz.xw	$ACCxw, $vf1xw, $vf2z
	vmaddaz.xw	$ACCxw, $vf31xw, $vf0z
	vmaddaz.xw	$ACCxw, $vf31xw, $vf15z
	vmaddaz.xw	$ACCxw, $vf31xw, $vf31z
	vmaddaz.xy	$ACCxy, $vf0xy, $vf0z
	vmaddaz.xy	$ACCxy, $vf0xy, $vf31z
	vmaddaz.xy	$ACCxy, $vf1xy, $vf2z
	vmaddaz.xy	$ACCxy, $vf31xy, $vf0z
	vmaddaz.xy	$ACCxy, $vf31xy, $vf15z
	vmaddaz.xy	$ACCxy, $vf31xy, $vf31z
	vmaddaz.xyw	$ACCxyw, $vf0xyw, $vf0z
	vmaddaz.xyw	$ACCxyw, $vf0xyw, $vf31z
	vmaddaz.xyw	$ACCxyw, $vf1xyw, $vf2z
	vmaddaz.xyw	$ACCxyw, $vf31xyw, $vf0z
	vmaddaz.xyw	$ACCxyw, $vf31xyw, $vf15z
	vmaddaz.xyw	$ACCxyw, $vf31xyw, $vf31z
	vmaddaz.xyz	$ACCxyz, $vf0xyz, $vf0z
	vmaddaz.xyz	$ACCxyz, $vf0xyz, $vf31z
	vmaddaz.xyz	$ACCxyz, $vf1xyz, $vf2z
	vmaddaz.xyz	$ACCxyz, $vf31xyz, $vf0z
	vmaddaz.xyz	$ACCxyz, $vf31xyz, $vf15z
	vmaddaz.xyz	$ACCxyz, $vf31xyz, $vf31z
	vmaddaz.xyzw	$ACCxyzw, $vf0xyzw, $vf0z
	vmaddaz.xyzw	$ACCxyzw, $vf0xyzw, $vf31z
	vmaddaz.xyzw	$ACCxyzw, $vf1xyzw, $vf2z
	vmaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf0z
	vmaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf15z
	vmaddaz.xyzw	$ACCxyzw, $vf31xyzw, $vf31z
	vmaddaz.xz	$ACCxz, $vf0xz, $vf0z
	vmaddaz.xz	$ACCxz, $vf0xz, $vf31z
	vmaddaz.xz	$ACCxz, $vf1xz, $vf2z
	vmaddaz.xz	$ACCxz, $vf31xz, $vf0z
	vmaddaz.xz	$ACCxz, $vf31xz, $vf15z
	vmaddaz.xz	$ACCxz, $vf31xz, $vf31z
	vmaddaz.xzw	$ACCxzw, $vf0xzw, $vf0z
	vmaddaz.xzw	$ACCxzw, $vf0xzw, $vf31z
	vmaddaz.xzw	$ACCxzw, $vf1xzw, $vf2z
	vmaddaz.xzw	$ACCxzw, $vf31xzw, $vf0z
	vmaddaz.xzw	$ACCxzw, $vf31xzw, $vf15z
	vmaddaz.xzw	$ACCxzw, $vf31xzw, $vf31z
	vmaddaz.y	$ACCy, $vf0y, $vf0z
	vmaddaz.y	$ACCy, $vf0y, $vf31z
	vmaddaz.y	$ACCy, $vf1y, $vf2z
	vmaddaz.y	$ACCy, $vf31y, $vf0z
	vmaddaz.y	$ACCy, $vf31y, $vf15z
	vmaddaz.y	$ACCy, $vf31y, $vf31z
	vmaddaz.yw	$ACCyw, $vf0yw, $vf0z
	vmaddaz.yw	$ACCyw, $vf0yw, $vf31z
	vmaddaz.yw	$ACCyw, $vf1yw, $vf2z
	vmaddaz.yw	$ACCyw, $vf31yw, $vf0z
	vmaddaz.yw	$ACCyw, $vf31yw, $vf15z
	vmaddaz.yw	$ACCyw, $vf31yw, $vf31z
	vmaddaz.yz	$ACCyz, $vf0yz, $vf0z
	vmaddaz.yz	$ACCyz, $vf0yz, $vf31z
	vmaddaz.yz	$ACCyz, $vf1yz, $vf2z
	vmaddaz.yz	$ACCyz, $vf31yz, $vf0z
	vmaddaz.yz	$ACCyz, $vf31yz, $vf15z
	vmaddaz.yz	$ACCyz, $vf31yz, $vf31z
	vmaddaz.yzw	$ACCyzw, $vf0yzw, $vf0z
	vmaddaz.yzw	$ACCyzw, $vf0yzw, $vf31z
	vmaddaz.yzw	$ACCyzw, $vf1yzw, $vf2z
	vmaddaz.yzw	$ACCyzw, $vf31yzw, $vf0z
	vmaddaz.yzw	$ACCyzw, $vf31yzw, $vf15z
	vmaddaz.yzw	$ACCyzw, $vf31yzw, $vf31z
	vmaddaz.z	$ACCz, $vf0z, $vf0z
	vmaddaz.z	$ACCz, $vf0z, $vf31z
	vmaddaz.z	$ACCz, $vf1z, $vf2z
	vmaddaz.z	$ACCz, $vf31z, $vf0z
	vmaddaz.z	$ACCz, $vf31z, $vf15z
	vmaddaz.z	$ACCz, $vf31z, $vf31z
	vmaddaz.zw	$ACCzw, $vf0zw, $vf0z
	vmaddaz.zw	$ACCzw, $vf0zw, $vf31z
	vmaddaz.zw	$ACCzw, $vf1zw, $vf2z
	vmaddaz.zw	$ACCzw, $vf31zw, $vf0z
	vmaddaz.zw	$ACCzw, $vf31zw, $vf15z
	vmaddaz.zw	$ACCzw, $vf31zw, $vf31z
	vmaddi.w	$vf0w, $vf0w, $I
	vmaddi.w	$vf0w, $vf31w, $I
	vmaddi.w	$vf1w, $vf2w, $I
	vmaddi.w	$vf31w, $vf0w, $I
	vmaddi.w	$vf31w, $vf15w, $I
	vmaddi.w	$vf31w, $vf31w, $I
	vmaddi.x	$vf0x, $vf0x, $I
	vmaddi.x	$vf0x, $vf31x, $I
	vmaddi.x	$vf1x, $vf2x, $I
	vmaddi.x	$vf31x, $vf0x, $I
	vmaddi.x	$vf31x, $vf15x, $I
	vmaddi.x	$vf31x, $vf31x, $I
	vmaddi.xw	$vf0xw, $vf0xw, $I
	vmaddi.xw	$vf0xw, $vf31xw, $I
	vmaddi.xw	$vf1xw, $vf2xw, $I
	vmaddi.xw	$vf31xw, $vf0xw, $I
	vmaddi.xw	$vf31xw, $vf15xw, $I
	vmaddi.xw	$vf31xw, $vf31xw, $I
	vmaddi.xy	$vf0xy, $vf0xy, $I
	vmaddi.xy	$vf0xy, $vf31xy, $I
	vmaddi.xy	$vf1xy, $vf2xy, $I
	vmaddi.xy	$vf31xy, $vf0xy, $I
	vmaddi.xy	$vf31xy, $vf15xy, $I
	vmaddi.xy	$vf31xy, $vf31xy, $I
	vmaddi.xyw	$vf0xyw, $vf0xyw, $I
	vmaddi.xyw	$vf0xyw, $vf31xyw, $I
	vmaddi.xyw	$vf1xyw, $vf2xyw, $I
	vmaddi.xyw	$vf31xyw, $vf0xyw, $I
	vmaddi.xyw	$vf31xyw, $vf15xyw, $I
	vmaddi.xyw	$vf31xyw, $vf31xyw, $I
	vmaddi.xyz	$vf0xyz, $vf0xyz, $I
	vmaddi.xyz	$vf0xyz, $vf31xyz, $I
	vmaddi.xyz	$vf1xyz, $vf2xyz, $I
	vmaddi.xyz	$vf31xyz, $vf0xyz, $I
	vmaddi.xyz	$vf31xyz, $vf15xyz, $I
	vmaddi.xyz	$vf31xyz, $vf31xyz, $I
	vmaddi.xyzw	$vf0xyzw, $vf0xyzw, $I
	vmaddi.xyzw	$vf0xyzw, $vf31xyzw, $I
	vmaddi.xyzw	$vf1xyzw, $vf2xyzw, $I
	vmaddi.xyzw	$vf31xyzw, $vf0xyzw, $I
	vmaddi.xyzw	$vf31xyzw, $vf15xyzw, $I
	vmaddi.xyzw	$vf31xyzw, $vf31xyzw, $I
	vmaddi.xz	$vf0xz, $vf0xz, $I
	vmaddi.xz	$vf0xz, $vf31xz, $I
	vmaddi.xz	$vf1xz, $vf2xz, $I
	vmaddi.xz	$vf31xz, $vf0xz, $I
	vmaddi.xz	$vf31xz, $vf15xz, $I
	vmaddi.xz	$vf31xz, $vf31xz, $I
	vmaddi.xzw	$vf0xzw, $vf0xzw, $I
	vmaddi.xzw	$vf0xzw, $vf31xzw, $I
	vmaddi.xzw	$vf1xzw, $vf2xzw, $I
	vmaddi.xzw	$vf31xzw, $vf0xzw, $I
	vmaddi.xzw	$vf31xzw, $vf15xzw, $I
	vmaddi.xzw	$vf31xzw, $vf31xzw, $I
	vmaddi.y	$vf0y, $vf0y, $I
	vmaddi.y	$vf0y, $vf31y, $I
	vmaddi.y	$vf1y, $vf2y, $I
	vmaddi.y	$vf31y, $vf0y, $I
	vmaddi.y	$vf31y, $vf15y, $I
	vmaddi.y	$vf31y, $vf31y, $I
	vmaddi.yw	$vf0yw, $vf0yw, $I
	vmaddi.yw	$vf0yw, $vf31yw, $I
	vmaddi.yw	$vf1yw, $vf2yw, $I
	vmaddi.yw	$vf31yw, $vf0yw, $I
	vmaddi.yw	$vf31yw, $vf15yw, $I
	vmaddi.yw	$vf31yw, $vf31yw, $I
	vmaddi.yz	$vf0yz, $vf0yz, $I
	vmaddi.yz	$vf0yz, $vf31yz, $I
	vmaddi.yz	$vf1yz, $vf2yz, $I
	vmaddi.yz	$vf31yz, $vf0yz, $I
	vmaddi.yz	$vf31yz, $vf15yz, $I
	vmaddi.yz	$vf31yz, $vf31yz, $I
	vmaddi.yzw	$vf0yzw, $vf0yzw, $I
	vmaddi.yzw	$vf0yzw, $vf31yzw, $I
	vmaddi.yzw	$vf1yzw, $vf2yzw, $I
	vmaddi.yzw	$vf31yzw, $vf0yzw, $I
	vmaddi.yzw	$vf31yzw, $vf15yzw, $I
	vmaddi.yzw	$vf31yzw, $vf31yzw, $I
	vmaddi.z	$vf0z, $vf0z, $I
	vmaddi.z	$vf0z, $vf31z, $I
	vmaddi.z	$vf1z, $vf2z, $I
	vmaddi.z	$vf31z, $vf0z, $I
	vmaddi.z	$vf31z, $vf15z, $I
	vmaddi.z	$vf31z, $vf31z, $I
	vmaddi.zw	$vf0zw, $vf0zw, $I
	vmaddi.zw	$vf0zw, $vf31zw, $I
	vmaddi.zw	$vf1zw, $vf2zw, $I
	vmaddi.zw	$vf31zw, $vf0zw, $I
	vmaddi.zw	$vf31zw, $vf15zw, $I
	vmaddi.zw	$vf31zw, $vf31zw, $I
	vmaddq.w	$vf0w, $vf0w, $Q
	vmaddq.w	$vf0w, $vf31w, $Q
	vmaddq.w	$vf1w, $vf2w, $Q
	vmaddq.w	$vf31w, $vf0w, $Q
	vmaddq.w	$vf31w, $vf15w, $Q
	vmaddq.w	$vf31w, $vf31w, $Q
	vmaddq.x	$vf0x, $vf0x, $Q
	vmaddq.x	$vf0x, $vf31x, $Q
	vmaddq.x	$vf1x, $vf2x, $Q
	vmaddq.x	$vf31x, $vf0x, $Q
	vmaddq.x	$vf31x, $vf15x, $Q
	vmaddq.x	$vf31x, $vf31x, $Q
	vmaddq.xw	$vf0xw, $vf0xw, $Q
	vmaddq.xw	$vf0xw, $vf31xw, $Q
	vmaddq.xw	$vf1xw, $vf2xw, $Q
	vmaddq.xw	$vf31xw, $vf0xw, $Q
	vmaddq.xw	$vf31xw, $vf15xw, $Q
	vmaddq.xw	$vf31xw, $vf31xw, $Q
	vmaddq.xy	$vf0xy, $vf0xy, $Q
	vmaddq.xy	$vf0xy, $vf31xy, $Q
	vmaddq.xy	$vf1xy, $vf2xy, $Q
	vmaddq.xy	$vf31xy, $vf0xy, $Q
	vmaddq.xy	$vf31xy, $vf15xy, $Q
	vmaddq.xy	$vf31xy, $vf31xy, $Q
	vmaddq.xyw	$vf0xyw, $vf0xyw, $Q
	vmaddq.xyw	$vf0xyw, $vf31xyw, $Q
	vmaddq.xyw	$vf1xyw, $vf2xyw, $Q
	vmaddq.xyw	$vf31xyw, $vf0xyw, $Q
	vmaddq.xyw	$vf31xyw, $vf15xyw, $Q
	vmaddq.xyw	$vf31xyw, $vf31xyw, $Q
	vmaddq.xyz	$vf0xyz, $vf0xyz, $Q
	vmaddq.xyz	$vf0xyz, $vf31xyz, $Q
	vmaddq.xyz	$vf1xyz, $vf2xyz, $Q
	vmaddq.xyz	$vf31xyz, $vf0xyz, $Q
	vmaddq.xyz	$vf31xyz, $vf15xyz, $Q
	vmaddq.xyz	$vf31xyz, $vf31xyz, $Q
	vmaddq.xyzw	$vf0xyzw, $vf0xyzw, $Q
	vmaddq.xyzw	$vf0xyzw, $vf31xyzw, $Q
	vmaddq.xyzw	$vf1xyzw, $vf2xyzw, $Q
	vmaddq.xyzw	$vf31xyzw, $vf0xyzw, $Q
	vmaddq.xyzw	$vf31xyzw, $vf15xyzw, $Q
	vmaddq.xyzw	$vf31xyzw, $vf31xyzw, $Q
	vmaddq.xz	$vf0xz, $vf0xz, $Q
	vmaddq.xz	$vf0xz, $vf31xz, $Q
	vmaddq.xz	$vf1xz, $vf2xz, $Q
	vmaddq.xz	$vf31xz, $vf0xz, $Q
	vmaddq.xz	$vf31xz, $vf15xz, $Q
	vmaddq.xz	$vf31xz, $vf31xz, $Q
	vmaddq.xzw	$vf0xzw, $vf0xzw, $Q
	vmaddq.xzw	$vf0xzw, $vf31xzw, $Q
	vmaddq.xzw	$vf1xzw, $vf2xzw, $Q
	vmaddq.xzw	$vf31xzw, $vf0xzw, $Q
	vmaddq.xzw	$vf31xzw, $vf15xzw, $Q
	vmaddq.xzw	$vf31xzw, $vf31xzw, $Q
	vmaddq.y	$vf0y, $vf0y, $Q
	vmaddq.y	$vf0y, $vf31y, $Q
	vmaddq.y	$vf1y, $vf2y, $Q
	vmaddq.y	$vf31y, $vf0y, $Q
	vmaddq.y	$vf31y, $vf15y, $Q
	vmaddq.y	$vf31y, $vf31y, $Q
	vmaddq.yw	$vf0yw, $vf0yw, $Q
	vmaddq.yw	$vf0yw, $vf31yw, $Q
	vmaddq.yw	$vf1yw, $vf2yw, $Q
	vmaddq.yw	$vf31yw, $vf0yw, $Q
	vmaddq.yw	$vf31yw, $vf15yw, $Q
	vmaddq.yw	$vf31yw, $vf31yw, $Q
	vmaddq.yz	$vf0yz, $vf0yz, $Q
	vmaddq.yz	$vf0yz, $vf31yz, $Q
	vmaddq.yz	$vf1yz, $vf2yz, $Q
	vmaddq.yz	$vf31yz, $vf0yz, $Q
	vmaddq.yz	$vf31yz, $vf15yz, $Q
	vmaddq.yz	$vf31yz, $vf31yz, $Q
	vmaddq.yzw	$vf0yzw, $vf0yzw, $Q
	vmaddq.yzw	$vf0yzw, $vf31yzw, $Q
	vmaddq.yzw	$vf1yzw, $vf2yzw, $Q
	vmaddq.yzw	$vf31yzw, $vf0yzw, $Q
	vmaddq.yzw	$vf31yzw, $vf15yzw, $Q
	vmaddq.yzw	$vf31yzw, $vf31yzw, $Q
	vmaddq.z	$vf0z, $vf0z, $Q
	vmaddq.z	$vf0z, $vf31z, $Q
	vmaddq.z	$vf1z, $vf2z, $Q
	vmaddq.z	$vf31z, $vf0z, $Q
	vmaddq.z	$vf31z, $vf15z, $Q
	vmaddq.z	$vf31z, $vf31z, $Q
	vmaddq.zw	$vf0zw, $vf0zw, $Q
	vmaddq.zw	$vf0zw, $vf31zw, $Q
	vmaddq.zw	$vf1zw, $vf2zw, $Q
	vmaddq.zw	$vf31zw, $vf0zw, $Q
	vmaddq.zw	$vf31zw, $vf15zw, $Q
	vmaddq.zw	$vf31zw, $vf31zw, $Q
	vmadd.w		$vf0w, $vf0w, $vf0w
	vmadd.w		$vf0w, $vf0w, $vf31w
	vmadd.w		$vf0w, $vf31w, $vf0w
	vmadd.w		$vf1w, $vf2w, $vf3w
	vmadd.w		$vf31w, $vf0w, $vf0w
	vmadd.w		$vf31w, $vf15w, $vf7w
	vmadd.w		$vf31w, $vf31w, $vf31w
	vmaddw.w	$vf0w, $vf0w, $vf0w
	vmaddw.w	$vf0w, $vf0w, $vf31w
	vmaddw.w	$vf0w, $vf31w, $vf0w
	vmaddw.w	$vf1w, $vf2w, $vf3w
	vmaddw.w	$vf31w, $vf0w, $vf0w
	vmaddw.w	$vf31w, $vf15w, $vf7w
	vmaddw.w	$vf31w, $vf31w, $vf31w
	vmaddw.x	$vf0x, $vf0x, $vf0w
	vmaddw.x	$vf0x, $vf0x, $vf31w
	vmaddw.x	$vf0x, $vf31x, $vf0w
	vmaddw.x	$vf1x, $vf2x, $vf3w
	vmaddw.x	$vf31x, $vf0x, $vf0w
	vmaddw.x	$vf31x, $vf15x, $vf7w
	vmaddw.x	$vf31x, $vf31x, $vf31w
	vmaddw.xw	$vf0xw, $vf0xw, $vf0w
	vmaddw.xw	$vf0xw, $vf0xw, $vf31w
	vmaddw.xw	$vf0xw, $vf31xw, $vf0w
	vmaddw.xw	$vf1xw, $vf2xw, $vf3w
	vmaddw.xw	$vf31xw, $vf0xw, $vf0w
	vmaddw.xw	$vf31xw, $vf15xw, $vf7w
	vmaddw.xw	$vf31xw, $vf31xw, $vf31w
	vmaddw.xy	$vf0xy, $vf0xy, $vf0w
	vmaddw.xy	$vf0xy, $vf0xy, $vf31w
	vmaddw.xy	$vf0xy, $vf31xy, $vf0w
	vmaddw.xy	$vf1xy, $vf2xy, $vf3w
	vmaddw.xy	$vf31xy, $vf0xy, $vf0w
	vmaddw.xy	$vf31xy, $vf15xy, $vf7w
	vmaddw.xy	$vf31xy, $vf31xy, $vf31w
	vmaddw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vmaddw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vmaddw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vmaddw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vmaddw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vmaddw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vmaddw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vmaddw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vmaddw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vmaddw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vmaddw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vmaddw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vmaddw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vmaddw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vmaddw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vmaddw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vmaddw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vmaddw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vmaddw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vmaddw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vmaddw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vmaddw.xz	$vf0xz, $vf0xz, $vf0w
	vmaddw.xz	$vf0xz, $vf0xz, $vf31w
	vmaddw.xz	$vf0xz, $vf31xz, $vf0w
	vmaddw.xz	$vf1xz, $vf2xz, $vf3w
	vmaddw.xz	$vf31xz, $vf0xz, $vf0w
	vmaddw.xz	$vf31xz, $vf15xz, $vf7w
	vmaddw.xz	$vf31xz, $vf31xz, $vf31w
	vmaddw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vmaddw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vmaddw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vmaddw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vmaddw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vmaddw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vmaddw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vmaddw.y	$vf0y, $vf0y, $vf0w
	vmaddw.y	$vf0y, $vf0y, $vf31w
	vmaddw.y	$vf0y, $vf31y, $vf0w
	vmaddw.y	$vf1y, $vf2y, $vf3w
	vmaddw.y	$vf31y, $vf0y, $vf0w
	vmaddw.y	$vf31y, $vf15y, $vf7w
	vmaddw.y	$vf31y, $vf31y, $vf31w
	vmaddw.yw	$vf0yw, $vf0yw, $vf0w
	vmaddw.yw	$vf0yw, $vf0yw, $vf31w
	vmaddw.yw	$vf0yw, $vf31yw, $vf0w
	vmaddw.yw	$vf1yw, $vf2yw, $vf3w
	vmaddw.yw	$vf31yw, $vf0yw, $vf0w
	vmaddw.yw	$vf31yw, $vf15yw, $vf7w
	vmaddw.yw	$vf31yw, $vf31yw, $vf31w
	vmaddw.yz	$vf0yz, $vf0yz, $vf0w
	vmaddw.yz	$vf0yz, $vf0yz, $vf31w
	vmaddw.yz	$vf0yz, $vf31yz, $vf0w
	vmaddw.yz	$vf1yz, $vf2yz, $vf3w
	vmaddw.yz	$vf31yz, $vf0yz, $vf0w
	vmaddw.yz	$vf31yz, $vf15yz, $vf7w
	vmaddw.yz	$vf31yz, $vf31yz, $vf31w
	vmaddw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vmaddw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vmaddw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vmaddw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vmaddw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vmaddw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vmaddw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vmaddw.z	$vf0z, $vf0z, $vf0w
	vmaddw.z	$vf0z, $vf0z, $vf31w
	vmaddw.z	$vf0z, $vf31z, $vf0w
	vmaddw.z	$vf1z, $vf2z, $vf3w
	vmaddw.z	$vf31z, $vf0z, $vf0w
	vmaddw.z	$vf31z, $vf15z, $vf7w
	vmaddw.z	$vf31z, $vf31z, $vf31w
	vmaddw.zw	$vf0zw, $vf0zw, $vf0w
	vmaddw.zw	$vf0zw, $vf0zw, $vf31w
	vmaddw.zw	$vf0zw, $vf31zw, $vf0w
	vmaddw.zw	$vf1zw, $vf2zw, $vf3w
	vmaddw.zw	$vf31zw, $vf0zw, $vf0w
	vmaddw.zw	$vf31zw, $vf15zw, $vf7w
	vmaddw.zw	$vf31zw, $vf31zw, $vf31w
	vmadd.x		$vf0x, $vf0x, $vf0x
	vmadd.x		$vf0x, $vf0x, $vf31x
	vmadd.x		$vf0x, $vf31x, $vf0x
	vmadd.x		$vf1x, $vf2x, $vf3x
	vmadd.x		$vf31x, $vf0x, $vf0x
	vmadd.x		$vf31x, $vf15x, $vf7x
	vmadd.x		$vf31x, $vf31x, $vf31x
	vmaddx.w	$vf0w, $vf0w, $vf0x
	vmaddx.w	$vf0w, $vf0w, $vf31x
	vmaddx.w	$vf0w, $vf31w, $vf0x
	vmadd.xw	$vf0xw, $vf0xw, $vf0xw
	vmadd.xw	$vf0xw, $vf0xw, $vf31xw
	vmadd.xw	$vf0xw, $vf31xw, $vf0xw
	vmaddx.w	$vf1w, $vf2w, $vf3x
	vmadd.xw	$vf1xw, $vf2xw, $vf3xw
	vmaddx.w	$vf31w, $vf0w, $vf0x
	vmaddx.w	$vf31w, $vf15w, $vf7x
	vmaddx.w	$vf31w, $vf31w, $vf31x
	vmadd.xw	$vf31xw, $vf0xw, $vf0xw
	vmadd.xw	$vf31xw, $vf15xw, $vf7xw
	vmadd.xw	$vf31xw, $vf31xw, $vf31xw
	vmaddx.x	$vf0x, $vf0x, $vf0x
	vmaddx.x	$vf0x, $vf0x, $vf31x
	vmaddx.x	$vf0x, $vf31x, $vf0x
	vmaddx.x	$vf1x, $vf2x, $vf3x
	vmaddx.x	$vf31x, $vf0x, $vf0x
	vmaddx.x	$vf31x, $vf15x, $vf7x
	vmaddx.x	$vf31x, $vf31x, $vf31x
	vmaddx.xw	$vf0xw, $vf0xw, $vf0x
	vmaddx.xw	$vf0xw, $vf0xw, $vf31x
	vmaddx.xw	$vf0xw, $vf31xw, $vf0x
	vmaddx.xw	$vf1xw, $vf2xw, $vf3x
	vmaddx.xw	$vf31xw, $vf0xw, $vf0x
	vmaddx.xw	$vf31xw, $vf15xw, $vf7x
	vmaddx.xw	$vf31xw, $vf31xw, $vf31x
	vmaddx.xy	$vf0xy, $vf0xy, $vf0x
	vmaddx.xy	$vf0xy, $vf0xy, $vf31x
	vmaddx.xy	$vf0xy, $vf31xy, $vf0x
	vmaddx.xy	$vf1xy, $vf2xy, $vf3x
	vmaddx.xy	$vf31xy, $vf0xy, $vf0x
	vmaddx.xy	$vf31xy, $vf15xy, $vf7x
	vmaddx.xy	$vf31xy, $vf31xy, $vf31x
	vmaddx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vmaddx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vmaddx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vmaddx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vmaddx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vmaddx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vmaddx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vmaddx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vmaddx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vmaddx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vmaddx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vmaddx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vmaddx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vmaddx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vmaddx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vmaddx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vmaddx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vmaddx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vmaddx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vmaddx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vmaddx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vmaddx.xz	$vf0xz, $vf0xz, $vf0x
	vmaddx.xz	$vf0xz, $vf0xz, $vf31x
	vmaddx.xz	$vf0xz, $vf31xz, $vf0x
	vmaddx.xz	$vf1xz, $vf2xz, $vf3x
	vmaddx.xz	$vf31xz, $vf0xz, $vf0x
	vmaddx.xz	$vf31xz, $vf15xz, $vf7x
	vmaddx.xz	$vf31xz, $vf31xz, $vf31x
	vmaddx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vmaddx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vmaddx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vmaddx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vmaddx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vmaddx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vmaddx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vmadd.xy	$vf0xy, $vf0xy, $vf0xy
	vmadd.xy	$vf0xy, $vf0xy, $vf31xy
	vmadd.xy	$vf0xy, $vf31xy, $vf0xy
	vmaddx.y	$vf0y, $vf0y, $vf0x
	vmaddx.y	$vf0y, $vf0y, $vf31x
	vmaddx.y	$vf0y, $vf31y, $vf0x
	vmadd.xy	$vf1xy, $vf2xy, $vf3xy
	vmaddx.y	$vf1y, $vf2y, $vf3x
	vmadd.xy	$vf31xy, $vf0xy, $vf0xy
	vmadd.xy	$vf31xy, $vf15xy, $vf7xy
	vmadd.xy	$vf31xy, $vf31xy, $vf31xy
	vmaddx.y	$vf31y, $vf0y, $vf0x
	vmaddx.y	$vf31y, $vf15y, $vf7x
	vmaddx.y	$vf31y, $vf31y, $vf31x
	vmadd.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmadd.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vmadd.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vmaddx.yw	$vf0yw, $vf0yw, $vf0x
	vmaddx.yw	$vf0yw, $vf0yw, $vf31x
	vmaddx.yw	$vf0yw, $vf31yw, $vf0x
	vmadd.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vmaddx.yw	$vf1yw, $vf2yw, $vf3x
	vmadd.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vmadd.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vmadd.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vmaddx.yw	$vf31yw, $vf0yw, $vf0x
	vmaddx.yw	$vf31yw, $vf15yw, $vf7x
	vmaddx.yw	$vf31yw, $vf31yw, $vf31x
	vmadd.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmadd.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vmadd.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vmaddx.yz	$vf0yz, $vf0yz, $vf0x
	vmaddx.yz	$vf0yz, $vf0yz, $vf31x
	vmaddx.yz	$vf0yz, $vf31yz, $vf0x
	vmadd.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vmaddx.yz	$vf1yz, $vf2yz, $vf3x
	vmadd.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vmadd.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vmadd.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vmaddx.yz	$vf31yz, $vf0yz, $vf0x
	vmaddx.yz	$vf31yz, $vf15yz, $vf7x
	vmaddx.yz	$vf31yz, $vf31yz, $vf31x
	vmadd.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmadd.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vmadd.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmaddx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vmaddx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vmaddx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vmadd.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vmaddx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vmadd.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmadd.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vmadd.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vmaddx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vmaddx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vmaddx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vmadd.xz	$vf0xz, $vf0xz, $vf0xz
	vmadd.xz	$vf0xz, $vf0xz, $vf31xz
	vmadd.xz	$vf0xz, $vf31xz, $vf0xz
	vmaddx.z	$vf0z, $vf0z, $vf0x
	vmaddx.z	$vf0z, $vf0z, $vf31x
	vmaddx.z	$vf0z, $vf31z, $vf0x
	vmadd.xz	$vf1xz, $vf2xz, $vf3xz
	vmaddx.z	$vf1z, $vf2z, $vf3x
	vmadd.xz	$vf31xz, $vf0xz, $vf0xz
	vmadd.xz	$vf31xz, $vf15xz, $vf7xz
	vmadd.xz	$vf31xz, $vf31xz, $vf31xz
	vmaddx.z	$vf31z, $vf0z, $vf0x
	vmaddx.z	$vf31z, $vf15z, $vf7x
	vmaddx.z	$vf31z, $vf31z, $vf31x
	vmadd.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vmadd.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmadd.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vmaddx.zw	$vf0zw, $vf0zw, $vf0x
	vmaddx.zw	$vf0zw, $vf0zw, $vf31x
	vmaddx.zw	$vf0zw, $vf31zw, $vf0x
	vmadd.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vmaddx.zw	$vf1zw, $vf2zw, $vf3x
	vmadd.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vmadd.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmadd.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vmaddx.zw	$vf31zw, $vf0zw, $vf0x
	vmaddx.zw	$vf31zw, $vf15zw, $vf7x
	vmaddx.zw	$vf31zw, $vf31zw, $vf31x
	vmadd.y		$vf0y, $vf0y, $vf0y
	vmadd.y		$vf0y, $vf0y, $vf31y
	vmadd.y		$vf0y, $vf31y, $vf0y
	vmadd.y		$vf1y, $vf2y, $vf3y
	vmadd.y		$vf31y, $vf0y, $vf0y
	vmadd.y		$vf31y, $vf15y, $vf7y
	vmadd.y		$vf31y, $vf31y, $vf31y
	vmaddy.w	$vf0w, $vf0w, $vf0y
	vmaddy.w	$vf0w, $vf0w, $vf31y
	vmaddy.w	$vf0w, $vf31w, $vf0y
	vmadd.yw	$vf0yw, $vf0yw, $vf0yw
	vmadd.yw	$vf0yw, $vf0yw, $vf31yw
	vmadd.yw	$vf0yw, $vf31yw, $vf0yw
	vmaddy.w	$vf1w, $vf2w, $vf3y
	vmadd.yw	$vf1yw, $vf2yw, $vf3yw
	vmaddy.w	$vf31w, $vf0w, $vf0y
	vmaddy.w	$vf31w, $vf15w, $vf7y
	vmaddy.w	$vf31w, $vf31w, $vf31y
	vmadd.yw	$vf31yw, $vf0yw, $vf0yw
	vmadd.yw	$vf31yw, $vf15yw, $vf7yw
	vmadd.yw	$vf31yw, $vf31yw, $vf31yw
	vmaddy.x	$vf0x, $vf0x, $vf0y
	vmaddy.x	$vf0x, $vf0x, $vf31y
	vmaddy.x	$vf0x, $vf31x, $vf0y
	vmaddy.x	$vf1x, $vf2x, $vf3y
	vmaddy.x	$vf31x, $vf0x, $vf0y
	vmaddy.x	$vf31x, $vf15x, $vf7y
	vmaddy.x	$vf31x, $vf31x, $vf31y
	vmaddy.xw	$vf0xw, $vf0xw, $vf0y
	vmaddy.xw	$vf0xw, $vf0xw, $vf31y
	vmaddy.xw	$vf0xw, $vf31xw, $vf0y
	vmaddy.xw	$vf1xw, $vf2xw, $vf3y
	vmaddy.xw	$vf31xw, $vf0xw, $vf0y
	vmaddy.xw	$vf31xw, $vf15xw, $vf7y
	vmaddy.xw	$vf31xw, $vf31xw, $vf31y
	vmaddy.xy	$vf0xy, $vf0xy, $vf0y
	vmaddy.xy	$vf0xy, $vf0xy, $vf31y
	vmaddy.xy	$vf0xy, $vf31xy, $vf0y
	vmaddy.xy	$vf1xy, $vf2xy, $vf3y
	vmaddy.xy	$vf31xy, $vf0xy, $vf0y
	vmaddy.xy	$vf31xy, $vf15xy, $vf7y
	vmaddy.xy	$vf31xy, $vf31xy, $vf31y
	vmaddy.xyw	$vf0xyw, $vf0xyw, $vf0y
	vmaddy.xyw	$vf0xyw, $vf0xyw, $vf31y
	vmaddy.xyw	$vf0xyw, $vf31xyw, $vf0y
	vmaddy.xyw	$vf1xyw, $vf2xyw, $vf3y
	vmaddy.xyw	$vf31xyw, $vf0xyw, $vf0y
	vmaddy.xyw	$vf31xyw, $vf15xyw, $vf7y
	vmaddy.xyw	$vf31xyw, $vf31xyw, $vf31y
	vmaddy.xyz	$vf0xyz, $vf0xyz, $vf0y
	vmaddy.xyz	$vf0xyz, $vf0xyz, $vf31y
	vmaddy.xyz	$vf0xyz, $vf31xyz, $vf0y
	vmaddy.xyz	$vf1xyz, $vf2xyz, $vf3y
	vmaddy.xyz	$vf31xyz, $vf0xyz, $vf0y
	vmaddy.xyz	$vf31xyz, $vf15xyz, $vf7y
	vmaddy.xyz	$vf31xyz, $vf31xyz, $vf31y
	vmaddy.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vmaddy.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vmaddy.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vmaddy.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vmaddy.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vmaddy.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vmaddy.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vmaddy.xz	$vf0xz, $vf0xz, $vf0y
	vmaddy.xz	$vf0xz, $vf0xz, $vf31y
	vmaddy.xz	$vf0xz, $vf31xz, $vf0y
	vmaddy.xz	$vf1xz, $vf2xz, $vf3y
	vmaddy.xz	$vf31xz, $vf0xz, $vf0y
	vmaddy.xz	$vf31xz, $vf15xz, $vf7y
	vmaddy.xz	$vf31xz, $vf31xz, $vf31y
	vmaddy.xzw	$vf0xzw, $vf0xzw, $vf0y
	vmaddy.xzw	$vf0xzw, $vf0xzw, $vf31y
	vmaddy.xzw	$vf0xzw, $vf31xzw, $vf0y
	vmaddy.xzw	$vf1xzw, $vf2xzw, $vf3y
	vmaddy.xzw	$vf31xzw, $vf0xzw, $vf0y
	vmaddy.xzw	$vf31xzw, $vf15xzw, $vf7y
	vmaddy.xzw	$vf31xzw, $vf31xzw, $vf31y
	vmaddy.y	$vf0y, $vf0y, $vf0y
	vmaddy.y	$vf0y, $vf0y, $vf31y
	vmaddy.y	$vf0y, $vf31y, $vf0y
	vmaddy.y	$vf1y, $vf2y, $vf3y
	vmaddy.y	$vf31y, $vf0y, $vf0y
	vmaddy.y	$vf31y, $vf15y, $vf7y
	vmaddy.y	$vf31y, $vf31y, $vf31y
	vmaddy.yw	$vf0yw, $vf0yw, $vf0y
	vmaddy.yw	$vf0yw, $vf0yw, $vf31y
	vmaddy.yw	$vf0yw, $vf31yw, $vf0y
	vmaddy.yw	$vf1yw, $vf2yw, $vf3y
	vmaddy.yw	$vf31yw, $vf0yw, $vf0y
	vmaddy.yw	$vf31yw, $vf15yw, $vf7y
	vmaddy.yw	$vf31yw, $vf31yw, $vf31y
	vmaddy.yz	$vf0yz, $vf0yz, $vf0y
	vmaddy.yz	$vf0yz, $vf0yz, $vf31y
	vmaddy.yz	$vf0yz, $vf31yz, $vf0y
	vmaddy.yz	$vf1yz, $vf2yz, $vf3y
	vmaddy.yz	$vf31yz, $vf0yz, $vf0y
	vmaddy.yz	$vf31yz, $vf15yz, $vf7y
	vmaddy.yz	$vf31yz, $vf31yz, $vf31y
	vmaddy.yzw	$vf0yzw, $vf0yzw, $vf0y
	vmaddy.yzw	$vf0yzw, $vf0yzw, $vf31y
	vmaddy.yzw	$vf0yzw, $vf31yzw, $vf0y
	vmaddy.yzw	$vf1yzw, $vf2yzw, $vf3y
	vmaddy.yzw	$vf31yzw, $vf0yzw, $vf0y
	vmaddy.yzw	$vf31yzw, $vf15yzw, $vf7y
	vmaddy.yzw	$vf31yzw, $vf31yzw, $vf31y
	vmadd.yz	$vf0yz, $vf0yz, $vf0yz
	vmadd.yz	$vf0yz, $vf0yz, $vf31yz
	vmadd.yz	$vf0yz, $vf31yz, $vf0yz
	vmaddy.z	$vf0z, $vf0z, $vf0y
	vmaddy.z	$vf0z, $vf0z, $vf31y
	vmaddy.z	$vf0z, $vf31z, $vf0y
	vmadd.yz	$vf1yz, $vf2yz, $vf3yz
	vmaddy.z	$vf1z, $vf2z, $vf3y
	vmadd.yz	$vf31yz, $vf0yz, $vf0yz
	vmadd.yz	$vf31yz, $vf15yz, $vf7yz
	vmadd.yz	$vf31yz, $vf31yz, $vf31yz
	vmaddy.z	$vf31z, $vf0z, $vf0y
	vmaddy.z	$vf31z, $vf15z, $vf7y
	vmaddy.z	$vf31z, $vf31z, $vf31y
	vmadd.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmadd.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmadd.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vmaddy.zw	$vf0zw, $vf0zw, $vf0y
	vmaddy.zw	$vf0zw, $vf0zw, $vf31y
	vmaddy.zw	$vf0zw, $vf31zw, $vf0y
	vmadd.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vmaddy.zw	$vf1zw, $vf2zw, $vf3y
	vmadd.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vmadd.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vmadd.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vmaddy.zw	$vf31zw, $vf0zw, $vf0y
	vmaddy.zw	$vf31zw, $vf15zw, $vf7y
	vmaddy.zw	$vf31zw, $vf31zw, $vf31y
	vmadd.z		$vf0z, $vf0z, $vf0z
	vmadd.z		$vf0z, $vf0z, $vf31z
	vmadd.z		$vf0z, $vf31z, $vf0z
	vmadd.z		$vf1z, $vf2z, $vf3z
	vmadd.z		$vf31z, $vf0z, $vf0z
	vmadd.z		$vf31z, $vf15z, $vf7z
	vmadd.z		$vf31z, $vf31z, $vf31z
	vmaddz.w	$vf0w, $vf0w, $vf0z
	vmaddz.w	$vf0w, $vf0w, $vf31z
	vmaddz.w	$vf0w, $vf31w, $vf0z
	vmadd.zw	$vf0zw, $vf0zw, $vf0zw
	vmadd.zw	$vf0zw, $vf0zw, $vf31zw
	vmadd.zw	$vf0zw, $vf31zw, $vf0zw
	vmaddz.w	$vf1w, $vf2w, $vf3z
	vmadd.zw	$vf1zw, $vf2zw, $vf3zw
	vmaddz.w	$vf31w, $vf0w, $vf0z
	vmaddz.w	$vf31w, $vf15w, $vf7z
	vmaddz.w	$vf31w, $vf31w, $vf31z
	vmadd.zw	$vf31zw, $vf0zw, $vf0zw
	vmadd.zw	$vf31zw, $vf15zw, $vf7zw
	vmadd.zw	$vf31zw, $vf31zw, $vf31zw
	vmaddz.x	$vf0x, $vf0x, $vf0z
	vmaddz.x	$vf0x, $vf0x, $vf31z
	vmaddz.x	$vf0x, $vf31x, $vf0z
	vmaddz.x	$vf1x, $vf2x, $vf3z
	vmaddz.x	$vf31x, $vf0x, $vf0z
	vmaddz.x	$vf31x, $vf15x, $vf7z
	vmaddz.x	$vf31x, $vf31x, $vf31z
	vmaddz.xw	$vf0xw, $vf0xw, $vf0z
	vmaddz.xw	$vf0xw, $vf0xw, $vf31z
	vmaddz.xw	$vf0xw, $vf31xw, $vf0z
	vmaddz.xw	$vf1xw, $vf2xw, $vf3z
	vmaddz.xw	$vf31xw, $vf0xw, $vf0z
	vmaddz.xw	$vf31xw, $vf15xw, $vf7z
	vmaddz.xw	$vf31xw, $vf31xw, $vf31z
	vmaddz.xy	$vf0xy, $vf0xy, $vf0z
	vmaddz.xy	$vf0xy, $vf0xy, $vf31z
	vmaddz.xy	$vf0xy, $vf31xy, $vf0z
	vmaddz.xy	$vf1xy, $vf2xy, $vf3z
	vmaddz.xy	$vf31xy, $vf0xy, $vf0z
	vmaddz.xy	$vf31xy, $vf15xy, $vf7z
	vmaddz.xy	$vf31xy, $vf31xy, $vf31z
	vmaddz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vmaddz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vmaddz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vmaddz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vmaddz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vmaddz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vmaddz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vmaddz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vmaddz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vmaddz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vmaddz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vmaddz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vmaddz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vmaddz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vmaddz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vmaddz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vmaddz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vmaddz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vmaddz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vmaddz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vmaddz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vmaddz.xz	$vf0xz, $vf0xz, $vf0z
	vmaddz.xz	$vf0xz, $vf0xz, $vf31z
	vmaddz.xz	$vf0xz, $vf31xz, $vf0z
	vmaddz.xz	$vf1xz, $vf2xz, $vf3z
	vmaddz.xz	$vf31xz, $vf0xz, $vf0z
	vmaddz.xz	$vf31xz, $vf15xz, $vf7z
	vmaddz.xz	$vf31xz, $vf31xz, $vf31z
	vmaddz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vmaddz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vmaddz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vmaddz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vmaddz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vmaddz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vmaddz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vmaddz.y	$vf0y, $vf0y, $vf0z
	vmaddz.y	$vf0y, $vf0y, $vf31z
	vmaddz.y	$vf0y, $vf31y, $vf0z
	vmaddz.y	$vf1y, $vf2y, $vf3z
	vmaddz.y	$vf31y, $vf0y, $vf0z
	vmaddz.y	$vf31y, $vf15y, $vf7z
	vmaddz.y	$vf31y, $vf31y, $vf31z
	vmaddz.yw	$vf0yw, $vf0yw, $vf0z
	vmaddz.yw	$vf0yw, $vf0yw, $vf31z
	vmaddz.yw	$vf0yw, $vf31yw, $vf0z
	vmaddz.yw	$vf1yw, $vf2yw, $vf3z
	vmaddz.yw	$vf31yw, $vf0yw, $vf0z
	vmaddz.yw	$vf31yw, $vf15yw, $vf7z
	vmaddz.yw	$vf31yw, $vf31yw, $vf31z
	vmaddz.yz	$vf0yz, $vf0yz, $vf0z
	vmaddz.yz	$vf0yz, $vf0yz, $vf31z
	vmaddz.yz	$vf0yz, $vf31yz, $vf0z
	vmaddz.yz	$vf1yz, $vf2yz, $vf3z
	vmaddz.yz	$vf31yz, $vf0yz, $vf0z
	vmaddz.yz	$vf31yz, $vf15yz, $vf7z
	vmaddz.yz	$vf31yz, $vf31yz, $vf31z
	vmaddz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vmaddz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vmaddz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vmaddz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vmaddz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vmaddz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vmaddz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vmaddz.z	$vf0z, $vf0z, $vf0z
	vmaddz.z	$vf0z, $vf0z, $vf31z
	vmaddz.z	$vf0z, $vf31z, $vf0z
	vmaddz.z	$vf1z, $vf2z, $vf3z
	vmaddz.z	$vf31z, $vf0z, $vf0z
	vmaddz.z	$vf31z, $vf15z, $vf7z
	vmaddz.z	$vf31z, $vf31z, $vf31z
	vmaddz.zw	$vf0zw, $vf0zw, $vf0z
	vmaddz.zw	$vf0zw, $vf0zw, $vf31z
	vmaddz.zw	$vf0zw, $vf31zw, $vf0z
	vmaddz.zw	$vf1zw, $vf2zw, $vf3z
	vmaddz.zw	$vf31zw, $vf0zw, $vf0z
	vmaddz.zw	$vf31zw, $vf15zw, $vf7z
	vmaddz.zw	$vf31zw, $vf31zw, $vf31z
	vmaxi.w		$vf0w, $vf0w, $I
	vmaxi.w		$vf0w, $vf31w, $I
	vmaxi.w		$vf1w, $vf2w, $I
	vmaxi.w		$vf31w, $vf0w, $I
	vmaxi.w		$vf31w, $vf15w, $I
	vmaxi.w		$vf31w, $vf31w, $I
	vmaxi.x		$vf0x, $vf0x, $I
	vmaxi.x		$vf0x, $vf31x, $I
	vmaxi.x		$vf1x, $vf2x, $I
	vmaxi.x		$vf31x, $vf0x, $I
	vmaxi.x		$vf31x, $vf15x, $I
	vmaxi.x		$vf31x, $vf31x, $I
	vmaxi.xw	$vf0xw, $vf0xw, $I
	vmaxi.xw	$vf0xw, $vf31xw, $I
	vmaxi.xw	$vf1xw, $vf2xw, $I
	vmaxi.xw	$vf31xw, $vf0xw, $I
	vmaxi.xw	$vf31xw, $vf15xw, $I
	vmaxi.xw	$vf31xw, $vf31xw, $I
	vmaxi.xy	$vf0xy, $vf0xy, $I
	vmaxi.xy	$vf0xy, $vf31xy, $I
	vmaxi.xy	$vf1xy, $vf2xy, $I
	vmaxi.xy	$vf31xy, $vf0xy, $I
	vmaxi.xy	$vf31xy, $vf15xy, $I
	vmaxi.xy	$vf31xy, $vf31xy, $I
	vmaxi.xyw	$vf0xyw, $vf0xyw, $I
	vmaxi.xyw	$vf0xyw, $vf31xyw, $I
	vmaxi.xyw	$vf1xyw, $vf2xyw, $I
	vmaxi.xyw	$vf31xyw, $vf0xyw, $I
	vmaxi.xyw	$vf31xyw, $vf15xyw, $I
	vmaxi.xyw	$vf31xyw, $vf31xyw, $I
	vmaxi.xyz	$vf0xyz, $vf0xyz, $I
	vmaxi.xyz	$vf0xyz, $vf31xyz, $I
	vmaxi.xyz	$vf1xyz, $vf2xyz, $I
	vmaxi.xyz	$vf31xyz, $vf0xyz, $I
	vmaxi.xyz	$vf31xyz, $vf15xyz, $I
	vmaxi.xyz	$vf31xyz, $vf31xyz, $I
	vmaxi.xyzw	$vf0xyzw, $vf0xyzw, $I
	vmaxi.xyzw	$vf0xyzw, $vf31xyzw, $I
	vmaxi.xyzw	$vf1xyzw, $vf2xyzw, $I
	vmaxi.xyzw	$vf31xyzw, $vf0xyzw, $I
	vmaxi.xyzw	$vf31xyzw, $vf15xyzw, $I
	vmaxi.xyzw	$vf31xyzw, $vf31xyzw, $I
	vmaxi.xz	$vf0xz, $vf0xz, $I
	vmaxi.xz	$vf0xz, $vf31xz, $I
	vmaxi.xz	$vf1xz, $vf2xz, $I
	vmaxi.xz	$vf31xz, $vf0xz, $I
	vmaxi.xz	$vf31xz, $vf15xz, $I
	vmaxi.xz	$vf31xz, $vf31xz, $I
	vmaxi.xzw	$vf0xzw, $vf0xzw, $I
	vmaxi.xzw	$vf0xzw, $vf31xzw, $I
	vmaxi.xzw	$vf1xzw, $vf2xzw, $I
	vmaxi.xzw	$vf31xzw, $vf0xzw, $I
	vmaxi.xzw	$vf31xzw, $vf15xzw, $I
	vmaxi.xzw	$vf31xzw, $vf31xzw, $I
	vmaxi.y		$vf0y, $vf0y, $I
	vmaxi.y		$vf0y, $vf31y, $I
	vmaxi.y		$vf1y, $vf2y, $I
	vmaxi.y		$vf31y, $vf0y, $I
	vmaxi.y		$vf31y, $vf15y, $I
	vmaxi.y		$vf31y, $vf31y, $I
	vmaxi.yw	$vf0yw, $vf0yw, $I
	vmaxi.yw	$vf0yw, $vf31yw, $I
	vmaxi.yw	$vf1yw, $vf2yw, $I
	vmaxi.yw	$vf31yw, $vf0yw, $I
	vmaxi.yw	$vf31yw, $vf15yw, $I
	vmaxi.yw	$vf31yw, $vf31yw, $I
	vmaxi.yz	$vf0yz, $vf0yz, $I
	vmaxi.yz	$vf0yz, $vf31yz, $I
	vmaxi.yz	$vf1yz, $vf2yz, $I
	vmaxi.yz	$vf31yz, $vf0yz, $I
	vmaxi.yz	$vf31yz, $vf15yz, $I
	vmaxi.yz	$vf31yz, $vf31yz, $I
	vmaxi.yzw	$vf0yzw, $vf0yzw, $I
	vmaxi.yzw	$vf0yzw, $vf31yzw, $I
	vmaxi.yzw	$vf1yzw, $vf2yzw, $I
	vmaxi.yzw	$vf31yzw, $vf0yzw, $I
	vmaxi.yzw	$vf31yzw, $vf15yzw, $I
	vmaxi.yzw	$vf31yzw, $vf31yzw, $I
	vmaxi.z		$vf0z, $vf0z, $I
	vmaxi.z		$vf0z, $vf31z, $I
	vmaxi.z		$vf1z, $vf2z, $I
	vmaxi.z		$vf31z, $vf0z, $I
	vmaxi.z		$vf31z, $vf15z, $I
	vmaxi.z		$vf31z, $vf31z, $I
	vmaxi.zw	$vf0zw, $vf0zw, $I
	vmaxi.zw	$vf0zw, $vf31zw, $I
	vmaxi.zw	$vf1zw, $vf2zw, $I
	vmaxi.zw	$vf31zw, $vf0zw, $I
	vmaxi.zw	$vf31zw, $vf15zw, $I
	vmaxi.zw	$vf31zw, $vf31zw, $I
	vmax.w		$vf0w, $vf0w, $vf0w
	vmax.w		$vf0w, $vf0w, $vf31w
	vmax.w		$vf0w, $vf31w, $vf0w
	vmax.w		$vf1w, $vf2w, $vf3w
	vmax.w		$vf31w, $vf0w, $vf0w
	vmax.w		$vf31w, $vf15w, $vf7w
	vmax.w		$vf31w, $vf31w, $vf31w
	vmaxw.w		$vf0w, $vf0w, $vf0w
	vmaxw.w		$vf0w, $vf0w, $vf31w
	vmaxw.w		$vf0w, $vf31w, $vf0w
	vmaxw.w		$vf1w, $vf2w, $vf3w
	vmaxw.w		$vf31w, $vf0w, $vf0w
	vmaxw.w		$vf31w, $vf15w, $vf7w
	vmaxw.w		$vf31w, $vf31w, $vf31w
	vmaxw.x		$vf0x, $vf0x, $vf0w
	vmaxw.x		$vf0x, $vf0x, $vf31w
	vmaxw.x		$vf0x, $vf31x, $vf0w
	vmaxw.x		$vf1x, $vf2x, $vf3w
	vmaxw.x		$vf31x, $vf0x, $vf0w
	vmaxw.x		$vf31x, $vf15x, $vf7w
	vmaxw.x		$vf31x, $vf31x, $vf31w
	vmaxw.xw	$vf0xw, $vf0xw, $vf0w
	vmaxw.xw	$vf0xw, $vf0xw, $vf31w
	vmaxw.xw	$vf0xw, $vf31xw, $vf0w
	vmaxw.xw	$vf1xw, $vf2xw, $vf3w
	vmaxw.xw	$vf31xw, $vf0xw, $vf0w
	vmaxw.xw	$vf31xw, $vf15xw, $vf7w
	vmaxw.xw	$vf31xw, $vf31xw, $vf31w
	vmaxw.xy	$vf0xy, $vf0xy, $vf0w
	vmaxw.xy	$vf0xy, $vf0xy, $vf31w
	vmaxw.xy	$vf0xy, $vf31xy, $vf0w
	vmaxw.xy	$vf1xy, $vf2xy, $vf3w
	vmaxw.xy	$vf31xy, $vf0xy, $vf0w
	vmaxw.xy	$vf31xy, $vf15xy, $vf7w
	vmaxw.xy	$vf31xy, $vf31xy, $vf31w
	vmaxw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vmaxw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vmaxw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vmaxw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vmaxw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vmaxw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vmaxw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vmaxw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vmaxw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vmaxw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vmaxw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vmaxw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vmaxw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vmaxw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vmaxw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vmaxw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vmaxw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vmaxw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vmaxw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vmaxw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vmaxw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vmaxw.xz	$vf0xz, $vf0xz, $vf0w
	vmaxw.xz	$vf0xz, $vf0xz, $vf31w
	vmaxw.xz	$vf0xz, $vf31xz, $vf0w
	vmaxw.xz	$vf1xz, $vf2xz, $vf3w
	vmaxw.xz	$vf31xz, $vf0xz, $vf0w
	vmaxw.xz	$vf31xz, $vf15xz, $vf7w
	vmaxw.xz	$vf31xz, $vf31xz, $vf31w
	vmaxw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vmaxw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vmaxw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vmaxw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vmaxw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vmaxw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vmaxw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vmaxw.y		$vf0y, $vf0y, $vf0w
	vmaxw.y		$vf0y, $vf0y, $vf31w
	vmaxw.y		$vf0y, $vf31y, $vf0w
	vmaxw.y		$vf1y, $vf2y, $vf3w
	vmaxw.y		$vf31y, $vf0y, $vf0w
	vmaxw.y		$vf31y, $vf15y, $vf7w
	vmaxw.y		$vf31y, $vf31y, $vf31w
	vmaxw.yw	$vf0yw, $vf0yw, $vf0w
	vmaxw.yw	$vf0yw, $vf0yw, $vf31w
	vmaxw.yw	$vf0yw, $vf31yw, $vf0w
	vmaxw.yw	$vf1yw, $vf2yw, $vf3w
	vmaxw.yw	$vf31yw, $vf0yw, $vf0w
	vmaxw.yw	$vf31yw, $vf15yw, $vf7w
	vmaxw.yw	$vf31yw, $vf31yw, $vf31w
	vmaxw.yz	$vf0yz, $vf0yz, $vf0w
	vmaxw.yz	$vf0yz, $vf0yz, $vf31w
	vmaxw.yz	$vf0yz, $vf31yz, $vf0w
	vmaxw.yz	$vf1yz, $vf2yz, $vf3w
	vmaxw.yz	$vf31yz, $vf0yz, $vf0w
	vmaxw.yz	$vf31yz, $vf15yz, $vf7w
	vmaxw.yz	$vf31yz, $vf31yz, $vf31w
	vmaxw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vmaxw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vmaxw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vmaxw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vmaxw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vmaxw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vmaxw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vmaxw.z		$vf0z, $vf0z, $vf0w
	vmaxw.z		$vf0z, $vf0z, $vf31w
	vmaxw.z		$vf0z, $vf31z, $vf0w
	vmaxw.z		$vf1z, $vf2z, $vf3w
	vmaxw.z		$vf31z, $vf0z, $vf0w
	vmaxw.z		$vf31z, $vf15z, $vf7w
	vmaxw.z		$vf31z, $vf31z, $vf31w
	vmaxw.zw	$vf0zw, $vf0zw, $vf0w
	vmaxw.zw	$vf0zw, $vf0zw, $vf31w
	vmaxw.zw	$vf0zw, $vf31zw, $vf0w
	vmaxw.zw	$vf1zw, $vf2zw, $vf3w
	vmaxw.zw	$vf31zw, $vf0zw, $vf0w
	vmaxw.zw	$vf31zw, $vf15zw, $vf7w
	vmaxw.zw	$vf31zw, $vf31zw, $vf31w
	vmax.x		$vf0x, $vf0x, $vf0x
	vmax.x		$vf0x, $vf0x, $vf31x
	vmax.x		$vf0x, $vf31x, $vf0x
	vmax.x		$vf1x, $vf2x, $vf3x
	vmax.x		$vf31x, $vf0x, $vf0x
	vmax.x		$vf31x, $vf15x, $vf7x
	vmax.x		$vf31x, $vf31x, $vf31x
	vmaxx.w		$vf0w, $vf0w, $vf0x
	vmaxx.w		$vf0w, $vf0w, $vf31x
	vmaxx.w		$vf0w, $vf31w, $vf0x
	vmax.xw		$vf0xw, $vf0xw, $vf0xw
	vmax.xw		$vf0xw, $vf0xw, $vf31xw
	vmax.xw		$vf0xw, $vf31xw, $vf0xw
	vmaxx.w		$vf1w, $vf2w, $vf3x
	vmax.xw		$vf1xw, $vf2xw, $vf3xw
	vmaxx.w		$vf31w, $vf0w, $vf0x
	vmaxx.w		$vf31w, $vf15w, $vf7x
	vmaxx.w		$vf31w, $vf31w, $vf31x
	vmax.xw		$vf31xw, $vf0xw, $vf0xw
	vmax.xw		$vf31xw, $vf15xw, $vf7xw
	vmax.xw		$vf31xw, $vf31xw, $vf31xw
	vmaxx.x		$vf0x, $vf0x, $vf0x
	vmaxx.x		$vf0x, $vf0x, $vf31x
	vmaxx.x		$vf0x, $vf31x, $vf0x
	vmaxx.x		$vf1x, $vf2x, $vf3x
	vmaxx.x		$vf31x, $vf0x, $vf0x
	vmaxx.x		$vf31x, $vf15x, $vf7x
	vmaxx.x		$vf31x, $vf31x, $vf31x
	vmaxx.xw	$vf0xw, $vf0xw, $vf0x
	vmaxx.xw	$vf0xw, $vf0xw, $vf31x
	vmaxx.xw	$vf0xw, $vf31xw, $vf0x
	vmaxx.xw	$vf1xw, $vf2xw, $vf3x
	vmaxx.xw	$vf31xw, $vf0xw, $vf0x
	vmaxx.xw	$vf31xw, $vf15xw, $vf7x
	vmaxx.xw	$vf31xw, $vf31xw, $vf31x
	vmaxx.xy	$vf0xy, $vf0xy, $vf0x
	vmaxx.xy	$vf0xy, $vf0xy, $vf31x
	vmaxx.xy	$vf0xy, $vf31xy, $vf0x
	vmaxx.xy	$vf1xy, $vf2xy, $vf3x
	vmaxx.xy	$vf31xy, $vf0xy, $vf0x
	vmaxx.xy	$vf31xy, $vf15xy, $vf7x
	vmaxx.xy	$vf31xy, $vf31xy, $vf31x
	vmaxx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vmaxx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vmaxx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vmaxx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vmaxx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vmaxx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vmaxx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vmaxx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vmaxx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vmaxx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vmaxx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vmaxx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vmaxx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vmaxx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vmaxx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vmaxx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vmaxx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vmaxx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vmaxx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vmaxx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vmaxx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vmaxx.xz	$vf0xz, $vf0xz, $vf0x
	vmaxx.xz	$vf0xz, $vf0xz, $vf31x
	vmaxx.xz	$vf0xz, $vf31xz, $vf0x
	vmaxx.xz	$vf1xz, $vf2xz, $vf3x
	vmaxx.xz	$vf31xz, $vf0xz, $vf0x
	vmaxx.xz	$vf31xz, $vf15xz, $vf7x
	vmaxx.xz	$vf31xz, $vf31xz, $vf31x
	vmaxx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vmaxx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vmaxx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vmaxx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vmaxx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vmaxx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vmaxx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vmax.xy		$vf0xy, $vf0xy, $vf0xy
	vmax.xy		$vf0xy, $vf0xy, $vf31xy
	vmax.xy		$vf0xy, $vf31xy, $vf0xy
	vmaxx.y		$vf0y, $vf0y, $vf0x
	vmaxx.y		$vf0y, $vf0y, $vf31x
	vmaxx.y		$vf0y, $vf31y, $vf0x
	vmax.xy		$vf1xy, $vf2xy, $vf3xy
	vmaxx.y		$vf1y, $vf2y, $vf3x
	vmax.xy		$vf31xy, $vf0xy, $vf0xy
	vmax.xy		$vf31xy, $vf15xy, $vf7xy
	vmax.xy		$vf31xy, $vf31xy, $vf31xy
	vmaxx.y		$vf31y, $vf0y, $vf0x
	vmaxx.y		$vf31y, $vf15y, $vf7x
	vmaxx.y		$vf31y, $vf31y, $vf31x
	vmax.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmax.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vmax.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vmaxx.yw	$vf0yw, $vf0yw, $vf0x
	vmaxx.yw	$vf0yw, $vf0yw, $vf31x
	vmaxx.yw	$vf0yw, $vf31yw, $vf0x
	vmax.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vmaxx.yw	$vf1yw, $vf2yw, $vf3x
	vmax.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vmax.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vmax.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vmaxx.yw	$vf31yw, $vf0yw, $vf0x
	vmaxx.yw	$vf31yw, $vf15yw, $vf7x
	vmaxx.yw	$vf31yw, $vf31yw, $vf31x
	vmax.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmax.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vmax.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vmaxx.yz	$vf0yz, $vf0yz, $vf0x
	vmaxx.yz	$vf0yz, $vf0yz, $vf31x
	vmaxx.yz	$vf0yz, $vf31yz, $vf0x
	vmax.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vmaxx.yz	$vf1yz, $vf2yz, $vf3x
	vmax.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vmax.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vmax.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vmaxx.yz	$vf31yz, $vf0yz, $vf0x
	vmaxx.yz	$vf31yz, $vf15yz, $vf7x
	vmaxx.yz	$vf31yz, $vf31yz, $vf31x
	vmax.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmax.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vmax.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmaxx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vmaxx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vmaxx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vmax.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vmaxx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vmax.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmax.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vmax.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vmaxx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vmaxx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vmaxx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vmax.xz		$vf0xz, $vf0xz, $vf0xz
	vmax.xz		$vf0xz, $vf0xz, $vf31xz
	vmax.xz		$vf0xz, $vf31xz, $vf0xz
	vmaxx.z		$vf0z, $vf0z, $vf0x
	vmaxx.z		$vf0z, $vf0z, $vf31x
	vmaxx.z		$vf0z, $vf31z, $vf0x
	vmax.xz		$vf1xz, $vf2xz, $vf3xz
	vmaxx.z		$vf1z, $vf2z, $vf3x
	vmax.xz		$vf31xz, $vf0xz, $vf0xz
	vmax.xz		$vf31xz, $vf15xz, $vf7xz
	vmax.xz		$vf31xz, $vf31xz, $vf31xz
	vmaxx.z		$vf31z, $vf0z, $vf0x
	vmaxx.z		$vf31z, $vf15z, $vf7x
	vmaxx.z		$vf31z, $vf31z, $vf31x
	vmax.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vmax.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmax.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vmaxx.zw	$vf0zw, $vf0zw, $vf0x
	vmaxx.zw	$vf0zw, $vf0zw, $vf31x
	vmaxx.zw	$vf0zw, $vf31zw, $vf0x
	vmax.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vmaxx.zw	$vf1zw, $vf2zw, $vf3x
	vmax.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vmax.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmax.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vmaxx.zw	$vf31zw, $vf0zw, $vf0x
	vmaxx.zw	$vf31zw, $vf15zw, $vf7x
	vmaxx.zw	$vf31zw, $vf31zw, $vf31x
	vmax.y		$vf0y, $vf0y, $vf0y
	vmax.y		$vf0y, $vf0y, $vf31y
	vmax.y		$vf0y, $vf31y, $vf0y
	vmax.y		$vf1y, $vf2y, $vf3y
	vmax.y		$vf31y, $vf0y, $vf0y
	vmax.y		$vf31y, $vf15y, $vf7y
	vmax.y		$vf31y, $vf31y, $vf31y
	vmaxy.w		$vf0w, $vf0w, $vf0y
	vmaxy.w		$vf0w, $vf0w, $vf31y
	vmaxy.w		$vf0w, $vf31w, $vf0y
	vmax.yw		$vf0yw, $vf0yw, $vf0yw
	vmax.yw		$vf0yw, $vf0yw, $vf31yw
	vmax.yw		$vf0yw, $vf31yw, $vf0yw
	vmaxy.w		$vf1w, $vf2w, $vf3y
	vmax.yw		$vf1yw, $vf2yw, $vf3yw
	vmaxy.w		$vf31w, $vf0w, $vf0y
	vmaxy.w		$vf31w, $vf15w, $vf7y
	vmaxy.w		$vf31w, $vf31w, $vf31y
	vmax.yw		$vf31yw, $vf0yw, $vf0yw
	vmax.yw		$vf31yw, $vf15yw, $vf7yw
	vmax.yw		$vf31yw, $vf31yw, $vf31yw
	vmaxy.x		$vf0x, $vf0x, $vf0y
	vmaxy.x		$vf0x, $vf0x, $vf31y
	vmaxy.x		$vf0x, $vf31x, $vf0y
	vmaxy.x		$vf1x, $vf2x, $vf3y
	vmaxy.x		$vf31x, $vf0x, $vf0y
	vmaxy.x		$vf31x, $vf15x, $vf7y
	vmaxy.x		$vf31x, $vf31x, $vf31y
	vmaxy.xw	$vf0xw, $vf0xw, $vf0y
	vmaxy.xw	$vf0xw, $vf0xw, $vf31y
	vmaxy.xw	$vf0xw, $vf31xw, $vf0y
	vmaxy.xw	$vf1xw, $vf2xw, $vf3y
	vmaxy.xw	$vf31xw, $vf0xw, $vf0y
	vmaxy.xw	$vf31xw, $vf15xw, $vf7y
	vmaxy.xw	$vf31xw, $vf31xw, $vf31y
	vmaxy.xy	$vf0xy, $vf0xy, $vf0y
	vmaxy.xy	$vf0xy, $vf0xy, $vf31y
	vmaxy.xy	$vf0xy, $vf31xy, $vf0y
	vmaxy.xy	$vf1xy, $vf2xy, $vf3y
	vmaxy.xy	$vf31xy, $vf0xy, $vf0y
	vmaxy.xy	$vf31xy, $vf15xy, $vf7y
	vmaxy.xy	$vf31xy, $vf31xy, $vf31y
	vmaxy.xyw	$vf0xyw, $vf0xyw, $vf0y
	vmaxy.xyw	$vf0xyw, $vf0xyw, $vf31y
	vmaxy.xyw	$vf0xyw, $vf31xyw, $vf0y
	vmaxy.xyw	$vf1xyw, $vf2xyw, $vf3y
	vmaxy.xyw	$vf31xyw, $vf0xyw, $vf0y
	vmaxy.xyw	$vf31xyw, $vf15xyw, $vf7y
	vmaxy.xyw	$vf31xyw, $vf31xyw, $vf31y
	vmaxy.xyz	$vf0xyz, $vf0xyz, $vf0y
	vmaxy.xyz	$vf0xyz, $vf0xyz, $vf31y
	vmaxy.xyz	$vf0xyz, $vf31xyz, $vf0y
	vmaxy.xyz	$vf1xyz, $vf2xyz, $vf3y
	vmaxy.xyz	$vf31xyz, $vf0xyz, $vf0y
	vmaxy.xyz	$vf31xyz, $vf15xyz, $vf7y
	vmaxy.xyz	$vf31xyz, $vf31xyz, $vf31y
	vmaxy.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vmaxy.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vmaxy.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vmaxy.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vmaxy.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vmaxy.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vmaxy.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vmaxy.xz	$vf0xz, $vf0xz, $vf0y
	vmaxy.xz	$vf0xz, $vf0xz, $vf31y
	vmaxy.xz	$vf0xz, $vf31xz, $vf0y
	vmaxy.xz	$vf1xz, $vf2xz, $vf3y
	vmaxy.xz	$vf31xz, $vf0xz, $vf0y
	vmaxy.xz	$vf31xz, $vf15xz, $vf7y
	vmaxy.xz	$vf31xz, $vf31xz, $vf31y
	vmaxy.xzw	$vf0xzw, $vf0xzw, $vf0y
	vmaxy.xzw	$vf0xzw, $vf0xzw, $vf31y
	vmaxy.xzw	$vf0xzw, $vf31xzw, $vf0y
	vmaxy.xzw	$vf1xzw, $vf2xzw, $vf3y
	vmaxy.xzw	$vf31xzw, $vf0xzw, $vf0y
	vmaxy.xzw	$vf31xzw, $vf15xzw, $vf7y
	vmaxy.xzw	$vf31xzw, $vf31xzw, $vf31y
	vmaxy.y		$vf0y, $vf0y, $vf0y
	vmaxy.y		$vf0y, $vf0y, $vf31y
	vmaxy.y		$vf0y, $vf31y, $vf0y
	vmaxy.y		$vf1y, $vf2y, $vf3y
	vmaxy.y		$vf31y, $vf0y, $vf0y
	vmaxy.y		$vf31y, $vf15y, $vf7y
	vmaxy.y		$vf31y, $vf31y, $vf31y
	vmaxy.yw	$vf0yw, $vf0yw, $vf0y
	vmaxy.yw	$vf0yw, $vf0yw, $vf31y
	vmaxy.yw	$vf0yw, $vf31yw, $vf0y
	vmaxy.yw	$vf1yw, $vf2yw, $vf3y
	vmaxy.yw	$vf31yw, $vf0yw, $vf0y
	vmaxy.yw	$vf31yw, $vf15yw, $vf7y
	vmaxy.yw	$vf31yw, $vf31yw, $vf31y
	vmaxy.yz	$vf0yz, $vf0yz, $vf0y
	vmaxy.yz	$vf0yz, $vf0yz, $vf31y
	vmaxy.yz	$vf0yz, $vf31yz, $vf0y
	vmaxy.yz	$vf1yz, $vf2yz, $vf3y
	vmaxy.yz	$vf31yz, $vf0yz, $vf0y
	vmaxy.yz	$vf31yz, $vf15yz, $vf7y
	vmaxy.yz	$vf31yz, $vf31yz, $vf31y
	vmaxy.yzw	$vf0yzw, $vf0yzw, $vf0y
	vmaxy.yzw	$vf0yzw, $vf0yzw, $vf31y
	vmaxy.yzw	$vf0yzw, $vf31yzw, $vf0y
	vmaxy.yzw	$vf1yzw, $vf2yzw, $vf3y
	vmaxy.yzw	$vf31yzw, $vf0yzw, $vf0y
	vmaxy.yzw	$vf31yzw, $vf15yzw, $vf7y
	vmaxy.yzw	$vf31yzw, $vf31yzw, $vf31y
	vmax.yz		$vf0yz, $vf0yz, $vf0yz
	vmax.yz		$vf0yz, $vf0yz, $vf31yz
	vmax.yz		$vf0yz, $vf31yz, $vf0yz
	vmaxy.z		$vf0z, $vf0z, $vf0y
	vmaxy.z		$vf0z, $vf0z, $vf31y
	vmaxy.z		$vf0z, $vf31z, $vf0y
	vmax.yz		$vf1yz, $vf2yz, $vf3yz
	vmaxy.z		$vf1z, $vf2z, $vf3y
	vmax.yz		$vf31yz, $vf0yz, $vf0yz
	vmax.yz		$vf31yz, $vf15yz, $vf7yz
	vmax.yz		$vf31yz, $vf31yz, $vf31yz
	vmaxy.z		$vf31z, $vf0z, $vf0y
	vmaxy.z		$vf31z, $vf15z, $vf7y
	vmaxy.z		$vf31z, $vf31z, $vf31y
	vmax.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmax.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmax.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vmaxy.zw	$vf0zw, $vf0zw, $vf0y
	vmaxy.zw	$vf0zw, $vf0zw, $vf31y
	vmaxy.zw	$vf0zw, $vf31zw, $vf0y
	vmax.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vmaxy.zw	$vf1zw, $vf2zw, $vf3y
	vmax.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vmax.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vmax.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vmaxy.zw	$vf31zw, $vf0zw, $vf0y
	vmaxy.zw	$vf31zw, $vf15zw, $vf7y
	vmaxy.zw	$vf31zw, $vf31zw, $vf31y
	vmax.z		$vf0z, $vf0z, $vf0z
	vmax.z		$vf0z, $vf0z, $vf31z
	vmax.z		$vf0z, $vf31z, $vf0z
	vmax.z		$vf1z, $vf2z, $vf3z
	vmax.z		$vf31z, $vf0z, $vf0z
	vmax.z		$vf31z, $vf15z, $vf7z
	vmax.z		$vf31z, $vf31z, $vf31z
	vmaxz.w		$vf0w, $vf0w, $vf0z
	vmaxz.w		$vf0w, $vf0w, $vf31z
	vmaxz.w		$vf0w, $vf31w, $vf0z
	vmax.zw		$vf0zw, $vf0zw, $vf0zw
	vmax.zw		$vf0zw, $vf0zw, $vf31zw
	vmax.zw		$vf0zw, $vf31zw, $vf0zw
	vmaxz.w		$vf1w, $vf2w, $vf3z
	vmax.zw		$vf1zw, $vf2zw, $vf3zw
	vmaxz.w		$vf31w, $vf0w, $vf0z
	vmaxz.w		$vf31w, $vf15w, $vf7z
	vmaxz.w		$vf31w, $vf31w, $vf31z
	vmax.zw		$vf31zw, $vf0zw, $vf0zw
	vmax.zw		$vf31zw, $vf15zw, $vf7zw
	vmax.zw		$vf31zw, $vf31zw, $vf31zw
	vmaxz.x		$vf0x, $vf0x, $vf0z
	vmaxz.x		$vf0x, $vf0x, $vf31z
	vmaxz.x		$vf0x, $vf31x, $vf0z
	vmaxz.x		$vf1x, $vf2x, $vf3z
	vmaxz.x		$vf31x, $vf0x, $vf0z
	vmaxz.x		$vf31x, $vf15x, $vf7z
	vmaxz.x		$vf31x, $vf31x, $vf31z
	vmaxz.xw	$vf0xw, $vf0xw, $vf0z
	vmaxz.xw	$vf0xw, $vf0xw, $vf31z
	vmaxz.xw	$vf0xw, $vf31xw, $vf0z
	vmaxz.xw	$vf1xw, $vf2xw, $vf3z
	vmaxz.xw	$vf31xw, $vf0xw, $vf0z
	vmaxz.xw	$vf31xw, $vf15xw, $vf7z
	vmaxz.xw	$vf31xw, $vf31xw, $vf31z
	vmaxz.xy	$vf0xy, $vf0xy, $vf0z
	vmaxz.xy	$vf0xy, $vf0xy, $vf31z
	vmaxz.xy	$vf0xy, $vf31xy, $vf0z
	vmaxz.xy	$vf1xy, $vf2xy, $vf3z
	vmaxz.xy	$vf31xy, $vf0xy, $vf0z
	vmaxz.xy	$vf31xy, $vf15xy, $vf7z
	vmaxz.xy	$vf31xy, $vf31xy, $vf31z
	vmaxz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vmaxz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vmaxz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vmaxz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vmaxz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vmaxz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vmaxz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vmaxz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vmaxz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vmaxz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vmaxz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vmaxz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vmaxz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vmaxz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vmaxz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vmaxz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vmaxz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vmaxz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vmaxz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vmaxz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vmaxz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vmaxz.xz	$vf0xz, $vf0xz, $vf0z
	vmaxz.xz	$vf0xz, $vf0xz, $vf31z
	vmaxz.xz	$vf0xz, $vf31xz, $vf0z
	vmaxz.xz	$vf1xz, $vf2xz, $vf3z
	vmaxz.xz	$vf31xz, $vf0xz, $vf0z
	vmaxz.xz	$vf31xz, $vf15xz, $vf7z
	vmaxz.xz	$vf31xz, $vf31xz, $vf31z
	vmaxz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vmaxz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vmaxz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vmaxz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vmaxz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vmaxz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vmaxz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vmaxz.y		$vf0y, $vf0y, $vf0z
	vmaxz.y		$vf0y, $vf0y, $vf31z
	vmaxz.y		$vf0y, $vf31y, $vf0z
	vmaxz.y		$vf1y, $vf2y, $vf3z
	vmaxz.y		$vf31y, $vf0y, $vf0z
	vmaxz.y		$vf31y, $vf15y, $vf7z
	vmaxz.y		$vf31y, $vf31y, $vf31z
	vmaxz.yw	$vf0yw, $vf0yw, $vf0z
	vmaxz.yw	$vf0yw, $vf0yw, $vf31z
	vmaxz.yw	$vf0yw, $vf31yw, $vf0z
	vmaxz.yw	$vf1yw, $vf2yw, $vf3z
	vmaxz.yw	$vf31yw, $vf0yw, $vf0z
	vmaxz.yw	$vf31yw, $vf15yw, $vf7z
	vmaxz.yw	$vf31yw, $vf31yw, $vf31z
	vmaxz.yz	$vf0yz, $vf0yz, $vf0z
	vmaxz.yz	$vf0yz, $vf0yz, $vf31z
	vmaxz.yz	$vf0yz, $vf31yz, $vf0z
	vmaxz.yz	$vf1yz, $vf2yz, $vf3z
	vmaxz.yz	$vf31yz, $vf0yz, $vf0z
	vmaxz.yz	$vf31yz, $vf15yz, $vf7z
	vmaxz.yz	$vf31yz, $vf31yz, $vf31z
	vmaxz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vmaxz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vmaxz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vmaxz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vmaxz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vmaxz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vmaxz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vmaxz.z		$vf0z, $vf0z, $vf0z
	vmaxz.z		$vf0z, $vf0z, $vf31z
	vmaxz.z		$vf0z, $vf31z, $vf0z
	vmaxz.z		$vf1z, $vf2z, $vf3z
	vmaxz.z		$vf31z, $vf0z, $vf0z
	vmaxz.z		$vf31z, $vf15z, $vf7z
	vmaxz.z		$vf31z, $vf31z, $vf31z
	vmaxz.zw	$vf0zw, $vf0zw, $vf0z
	vmaxz.zw	$vf0zw, $vf0zw, $vf31z
	vmaxz.zw	$vf0zw, $vf31zw, $vf0z
	vmaxz.zw	$vf1zw, $vf2zw, $vf3z
	vmaxz.zw	$vf31zw, $vf0zw, $vf0z
	vmaxz.zw	$vf31zw, $vf15zw, $vf7z
	vmaxz.zw	$vf31zw, $vf31zw, $vf31z
	vmfir.w		$vf0w, $vi0
	vmfir.w		$vf0w, $vi31
	vmfir.w		$vf1w, $vi2
	vmfir.w		$vf31w, $vi0
	vmfir.w		$vf31w, $vi15
	vmfir.w		$vf31w, $vi31
	vmfir.x		$vf0x, $vi0
	vmfir.x		$vf0x, $vi31
	vmfir.x		$vf1x, $vi2
	vmfir.x		$vf31x, $vi0
	vmfir.x		$vf31x, $vi15
	vmfir.x		$vf31x, $vi31
	vmfir.xw	$vf0xw, $vi0
	vmfir.xw	$vf0xw, $vi31
	vmfir.xw	$vf1xw, $vi2
	vmfir.xw	$vf31xw, $vi0
	vmfir.xw	$vf31xw, $vi15
	vmfir.xw	$vf31xw, $vi31
	vmfir.xy	$vf0xy, $vi0
	vmfir.xy	$vf0xy, $vi31
	vmfir.xy	$vf1xy, $vi2
	vmfir.xy	$vf31xy, $vi0
	vmfir.xy	$vf31xy, $vi15
	vmfir.xy	$vf31xy, $vi31
	vmfir.xyw	$vf0xyw, $vi0
	vmfir.xyw	$vf0xyw, $vi31
	vmfir.xyw	$vf1xyw, $vi2
	vmfir.xyw	$vf31xyw, $vi0
	vmfir.xyw	$vf31xyw, $vi15
	vmfir.xyw	$vf31xyw, $vi31
	vmfir.xyz	$vf0xyz, $vi0
	vmfir.xyz	$vf0xyz, $vi31
	vmfir.xyz	$vf1xyz, $vi2
	vmfir.xyz	$vf31xyz, $vi0
	vmfir.xyz	$vf31xyz, $vi15
	vmfir.xyz	$vf31xyz, $vi31
	vmfir.xyzw	$vf0xyzw, $vi0
	vmfir.xyzw	$vf0xyzw, $vi31
	vmfir.xyzw	$vf1xyzw, $vi2
	vmfir.xyzw	$vf31xyzw, $vi0
	vmfir.xyzw	$vf31xyzw, $vi15
	vmfir.xyzw	$vf31xyzw, $vi31
	vmfir.xz	$vf0xz, $vi0
	vmfir.xz	$vf0xz, $vi31
	vmfir.xz	$vf1xz, $vi2
	vmfir.xz	$vf31xz, $vi0
	vmfir.xz	$vf31xz, $vi15
	vmfir.xz	$vf31xz, $vi31
	vmfir.xzw	$vf0xzw, $vi0
	vmfir.xzw	$vf0xzw, $vi31
	vmfir.xzw	$vf1xzw, $vi2
	vmfir.xzw	$vf31xzw, $vi0
	vmfir.xzw	$vf31xzw, $vi15
	vmfir.xzw	$vf31xzw, $vi31
	vmfir.y		$vf0y, $vi0
	vmfir.y		$vf0y, $vi31
	vmfir.y		$vf1y, $vi2
	vmfir.y		$vf31y, $vi0
	vmfir.y		$vf31y, $vi15
	vmfir.y		$vf31y, $vi31
	vmfir.yw	$vf0yw, $vi0
	vmfir.yw	$vf0yw, $vi31
	vmfir.yw	$vf1yw, $vi2
	vmfir.yw	$vf31yw, $vi0
	vmfir.yw	$vf31yw, $vi15
	vmfir.yw	$vf31yw, $vi31
	vmfir.yz	$vf0yz, $vi0
	vmfir.yz	$vf0yz, $vi31
	vmfir.yz	$vf1yz, $vi2
	vmfir.yz	$vf31yz, $vi0
	vmfir.yz	$vf31yz, $vi15
	vmfir.yz	$vf31yz, $vi31
	vmfir.yzw	$vf0yzw, $vi0
	vmfir.yzw	$vf0yzw, $vi31
	vmfir.yzw	$vf1yzw, $vi2
	vmfir.yzw	$vf31yzw, $vi0
	vmfir.yzw	$vf31yzw, $vi15
	vmfir.yzw	$vf31yzw, $vi31
	vmfir.z		$vf0z, $vi0
	vmfir.z		$vf0z, $vi31
	vmfir.z		$vf1z, $vi2
	vmfir.z		$vf31z, $vi0
	vmfir.z		$vf31z, $vi15
	vmfir.z		$vf31z, $vi31
	vmfir.zw	$vf0zw, $vi0
	vmfir.zw	$vf0zw, $vi31
	vmfir.zw	$vf1zw, $vi2
	vmfir.zw	$vf31zw, $vi0
	vmfir.zw	$vf31zw, $vi15
	vmfir.zw	$vf31zw, $vi31
	vminii.w	$vf0w, $vf0w, $I
	vminii.w	$vf0w, $vf31w, $I
	vminii.w	$vf1w, $vf2w, $I
	vminii.w	$vf31w, $vf0w, $I
	vminii.w	$vf31w, $vf15w, $I
	vminii.w	$vf31w, $vf31w, $I
	vminii.x	$vf0x, $vf0x, $I
	vminii.x	$vf0x, $vf31x, $I
	vminii.x	$vf1x, $vf2x, $I
	vminii.x	$vf31x, $vf0x, $I
	vminii.x	$vf31x, $vf15x, $I
	vminii.x	$vf31x, $vf31x, $I
	vminii.xw	$vf0xw, $vf0xw, $I
	vminii.xw	$vf0xw, $vf31xw, $I
	vminii.xw	$vf1xw, $vf2xw, $I
	vminii.xw	$vf31xw, $vf0xw, $I
	vminii.xw	$vf31xw, $vf15xw, $I
	vminii.xw	$vf31xw, $vf31xw, $I
	vminii.xy	$vf0xy, $vf0xy, $I
	vminii.xy	$vf0xy, $vf31xy, $I
	vminii.xy	$vf1xy, $vf2xy, $I
	vminii.xy	$vf31xy, $vf0xy, $I
	vminii.xy	$vf31xy, $vf15xy, $I
	vminii.xy	$vf31xy, $vf31xy, $I
	vminii.xyw	$vf0xyw, $vf0xyw, $I
	vminii.xyw	$vf0xyw, $vf31xyw, $I
	vminii.xyw	$vf1xyw, $vf2xyw, $I
	vminii.xyw	$vf31xyw, $vf0xyw, $I
	vminii.xyw	$vf31xyw, $vf15xyw, $I
	vminii.xyw	$vf31xyw, $vf31xyw, $I
	vminii.xyz	$vf0xyz, $vf0xyz, $I
	vminii.xyz	$vf0xyz, $vf31xyz, $I
	vminii.xyz	$vf1xyz, $vf2xyz, $I
	vminii.xyz	$vf31xyz, $vf0xyz, $I
	vminii.xyz	$vf31xyz, $vf15xyz, $I
	vminii.xyz	$vf31xyz, $vf31xyz, $I
	vminii.xyzw	$vf0xyzw, $vf0xyzw, $I
	vminii.xyzw	$vf0xyzw, $vf31xyzw, $I
	vminii.xyzw	$vf1xyzw, $vf2xyzw, $I
	vminii.xyzw	$vf31xyzw, $vf0xyzw, $I
	vminii.xyzw	$vf31xyzw, $vf15xyzw, $I
	vminii.xyzw	$vf31xyzw, $vf31xyzw, $I
	vminii.xz	$vf0xz, $vf0xz, $I
	vminii.xz	$vf0xz, $vf31xz, $I
	vminii.xz	$vf1xz, $vf2xz, $I
	vminii.xz	$vf31xz, $vf0xz, $I
	vminii.xz	$vf31xz, $vf15xz, $I
	vminii.xz	$vf31xz, $vf31xz, $I
	vminii.xzw	$vf0xzw, $vf0xzw, $I
	vminii.xzw	$vf0xzw, $vf31xzw, $I
	vminii.xzw	$vf1xzw, $vf2xzw, $I
	vminii.xzw	$vf31xzw, $vf0xzw, $I
	vminii.xzw	$vf31xzw, $vf15xzw, $I
	vminii.xzw	$vf31xzw, $vf31xzw, $I
	vminii.y	$vf0y, $vf0y, $I
	vminii.y	$vf0y, $vf31y, $I
	vminii.y	$vf1y, $vf2y, $I
	vminii.y	$vf31y, $vf0y, $I
	vminii.y	$vf31y, $vf15y, $I
	vminii.y	$vf31y, $vf31y, $I
	vminii.yw	$vf0yw, $vf0yw, $I
	vminii.yw	$vf0yw, $vf31yw, $I
	vminii.yw	$vf1yw, $vf2yw, $I
	vminii.yw	$vf31yw, $vf0yw, $I
	vminii.yw	$vf31yw, $vf15yw, $I
	vminii.yw	$vf31yw, $vf31yw, $I
	vminii.yz	$vf0yz, $vf0yz, $I
	vminii.yz	$vf0yz, $vf31yz, $I
	vminii.yz	$vf1yz, $vf2yz, $I
	vminii.yz	$vf31yz, $vf0yz, $I
	vminii.yz	$vf31yz, $vf15yz, $I
	vminii.yz	$vf31yz, $vf31yz, $I
	vminii.yzw	$vf0yzw, $vf0yzw, $I
	vminii.yzw	$vf0yzw, $vf31yzw, $I
	vminii.yzw	$vf1yzw, $vf2yzw, $I
	vminii.yzw	$vf31yzw, $vf0yzw, $I
	vminii.yzw	$vf31yzw, $vf15yzw, $I
	vminii.yzw	$vf31yzw, $vf31yzw, $I
	vminii.z	$vf0z, $vf0z, $I
	vminii.z	$vf0z, $vf31z, $I
	vminii.z	$vf1z, $vf2z, $I
	vminii.z	$vf31z, $vf0z, $I
	vminii.z	$vf31z, $vf15z, $I
	vminii.z	$vf31z, $vf31z, $I
	vminii.zw	$vf0zw, $vf0zw, $I
	vminii.zw	$vf0zw, $vf31zw, $I
	vminii.zw	$vf1zw, $vf2zw, $I
	vminii.zw	$vf31zw, $vf0zw, $I
	vminii.zw	$vf31zw, $vf15zw, $I
	vminii.zw	$vf31zw, $vf31zw, $I
	vmini.w		$vf0w, $vf0w, $vf0w
	vmini.w		$vf0w, $vf0w, $vf31w
	vmini.w		$vf0w, $vf31w, $vf0w
	vmini.w		$vf1w, $vf2w, $vf3w
	vmini.w		$vf31w, $vf0w, $vf0w
	vmini.w		$vf31w, $vf15w, $vf7w
	vmini.w		$vf31w, $vf31w, $vf31w
	vminiw.w	$vf0w, $vf0w, $vf0w
	vminiw.w	$vf0w, $vf0w, $vf31w
	vminiw.w	$vf0w, $vf31w, $vf0w
	vminiw.w	$vf1w, $vf2w, $vf3w
	vminiw.w	$vf31w, $vf0w, $vf0w
	vminiw.w	$vf31w, $vf15w, $vf7w
	vminiw.w	$vf31w, $vf31w, $vf31w
	vminiw.x	$vf0x, $vf0x, $vf0w
	vminiw.x	$vf0x, $vf0x, $vf31w
	vminiw.x	$vf0x, $vf31x, $vf0w
	vminiw.x	$vf1x, $vf2x, $vf3w
	vminiw.x	$vf31x, $vf0x, $vf0w
	vminiw.x	$vf31x, $vf15x, $vf7w
	vminiw.x	$vf31x, $vf31x, $vf31w
	vminiw.xw	$vf0xw, $vf0xw, $vf0w
	vminiw.xw	$vf0xw, $vf0xw, $vf31w
	vminiw.xw	$vf0xw, $vf31xw, $vf0w
	vminiw.xw	$vf1xw, $vf2xw, $vf3w
	vminiw.xw	$vf31xw, $vf0xw, $vf0w
	vminiw.xw	$vf31xw, $vf15xw, $vf7w
	vminiw.xw	$vf31xw, $vf31xw, $vf31w
	vminiw.xy	$vf0xy, $vf0xy, $vf0w
	vminiw.xy	$vf0xy, $vf0xy, $vf31w
	vminiw.xy	$vf0xy, $vf31xy, $vf0w
	vminiw.xy	$vf1xy, $vf2xy, $vf3w
	vminiw.xy	$vf31xy, $vf0xy, $vf0w
	vminiw.xy	$vf31xy, $vf15xy, $vf7w
	vminiw.xy	$vf31xy, $vf31xy, $vf31w
	vminiw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vminiw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vminiw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vminiw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vminiw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vminiw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vminiw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vminiw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vminiw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vminiw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vminiw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vminiw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vminiw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vminiw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vminiw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vminiw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vminiw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vminiw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vminiw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vminiw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vminiw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vminiw.xz	$vf0xz, $vf0xz, $vf0w
	vminiw.xz	$vf0xz, $vf0xz, $vf31w
	vminiw.xz	$vf0xz, $vf31xz, $vf0w
	vminiw.xz	$vf1xz, $vf2xz, $vf3w
	vminiw.xz	$vf31xz, $vf0xz, $vf0w
	vminiw.xz	$vf31xz, $vf15xz, $vf7w
	vminiw.xz	$vf31xz, $vf31xz, $vf31w
	vminiw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vminiw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vminiw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vminiw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vminiw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vminiw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vminiw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vminiw.y	$vf0y, $vf0y, $vf0w
	vminiw.y	$vf0y, $vf0y, $vf31w
	vminiw.y	$vf0y, $vf31y, $vf0w
	vminiw.y	$vf1y, $vf2y, $vf3w
	vminiw.y	$vf31y, $vf0y, $vf0w
	vminiw.y	$vf31y, $vf15y, $vf7w
	vminiw.y	$vf31y, $vf31y, $vf31w
	vminiw.yw	$vf0yw, $vf0yw, $vf0w
	vminiw.yw	$vf0yw, $vf0yw, $vf31w
	vminiw.yw	$vf0yw, $vf31yw, $vf0w
	vminiw.yw	$vf1yw, $vf2yw, $vf3w
	vminiw.yw	$vf31yw, $vf0yw, $vf0w
	vminiw.yw	$vf31yw, $vf15yw, $vf7w
	vminiw.yw	$vf31yw, $vf31yw, $vf31w
	vminiw.yz	$vf0yz, $vf0yz, $vf0w
	vminiw.yz	$vf0yz, $vf0yz, $vf31w
	vminiw.yz	$vf0yz, $vf31yz, $vf0w
	vminiw.yz	$vf1yz, $vf2yz, $vf3w
	vminiw.yz	$vf31yz, $vf0yz, $vf0w
	vminiw.yz	$vf31yz, $vf15yz, $vf7w
	vminiw.yz	$vf31yz, $vf31yz, $vf31w
	vminiw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vminiw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vminiw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vminiw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vminiw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vminiw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vminiw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vminiw.z	$vf0z, $vf0z, $vf0w
	vminiw.z	$vf0z, $vf0z, $vf31w
	vminiw.z	$vf0z, $vf31z, $vf0w
	vminiw.z	$vf1z, $vf2z, $vf3w
	vminiw.z	$vf31z, $vf0z, $vf0w
	vminiw.z	$vf31z, $vf15z, $vf7w
	vminiw.z	$vf31z, $vf31z, $vf31w
	vminiw.zw	$vf0zw, $vf0zw, $vf0w
	vminiw.zw	$vf0zw, $vf0zw, $vf31w
	vminiw.zw	$vf0zw, $vf31zw, $vf0w
	vminiw.zw	$vf1zw, $vf2zw, $vf3w
	vminiw.zw	$vf31zw, $vf0zw, $vf0w
	vminiw.zw	$vf31zw, $vf15zw, $vf7w
	vminiw.zw	$vf31zw, $vf31zw, $vf31w
	vmini.x		$vf0x, $vf0x, $vf0x
	vmini.x		$vf0x, $vf0x, $vf31x
	vmini.x		$vf0x, $vf31x, $vf0x
	vmini.x		$vf1x, $vf2x, $vf3x
	vmini.x		$vf31x, $vf0x, $vf0x
	vmini.x		$vf31x, $vf15x, $vf7x
	vmini.x		$vf31x, $vf31x, $vf31x
	vminix.w	$vf0w, $vf0w, $vf0x
	vminix.w	$vf0w, $vf0w, $vf31x
	vminix.w	$vf0w, $vf31w, $vf0x
	vmini.xw	$vf0xw, $vf0xw, $vf0xw
	vmini.xw	$vf0xw, $vf0xw, $vf31xw
	vmini.xw	$vf0xw, $vf31xw, $vf0xw
	vminix.w	$vf1w, $vf2w, $vf3x
	vmini.xw	$vf1xw, $vf2xw, $vf3xw
	vminix.w	$vf31w, $vf0w, $vf0x
	vminix.w	$vf31w, $vf15w, $vf7x
	vminix.w	$vf31w, $vf31w, $vf31x
	vmini.xw	$vf31xw, $vf0xw, $vf0xw
	vmini.xw	$vf31xw, $vf15xw, $vf7xw
	vmini.xw	$vf31xw, $vf31xw, $vf31xw
	vminix.x	$vf0x, $vf0x, $vf0x
	vminix.x	$vf0x, $vf0x, $vf31x
	vminix.x	$vf0x, $vf31x, $vf0x
	vminix.x	$vf1x, $vf2x, $vf3x
	vminix.x	$vf31x, $vf0x, $vf0x
	vminix.x	$vf31x, $vf15x, $vf7x
	vminix.x	$vf31x, $vf31x, $vf31x
	vminix.xw	$vf0xw, $vf0xw, $vf0x
	vminix.xw	$vf0xw, $vf0xw, $vf31x
	vminix.xw	$vf0xw, $vf31xw, $vf0x
	vminix.xw	$vf1xw, $vf2xw, $vf3x
	vminix.xw	$vf31xw, $vf0xw, $vf0x
	vminix.xw	$vf31xw, $vf15xw, $vf7x
	vminix.xw	$vf31xw, $vf31xw, $vf31x
	vminix.xy	$vf0xy, $vf0xy, $vf0x
	vminix.xy	$vf0xy, $vf0xy, $vf31x
	vminix.xy	$vf0xy, $vf31xy, $vf0x
	vminix.xy	$vf1xy, $vf2xy, $vf3x
	vminix.xy	$vf31xy, $vf0xy, $vf0x
	vminix.xy	$vf31xy, $vf15xy, $vf7x
	vminix.xy	$vf31xy, $vf31xy, $vf31x
	vminix.xyw	$vf0xyw, $vf0xyw, $vf0x
	vminix.xyw	$vf0xyw, $vf0xyw, $vf31x
	vminix.xyw	$vf0xyw, $vf31xyw, $vf0x
	vminix.xyw	$vf1xyw, $vf2xyw, $vf3x
	vminix.xyw	$vf31xyw, $vf0xyw, $vf0x
	vminix.xyw	$vf31xyw, $vf15xyw, $vf7x
	vminix.xyw	$vf31xyw, $vf31xyw, $vf31x
	vminix.xyz	$vf0xyz, $vf0xyz, $vf0x
	vminix.xyz	$vf0xyz, $vf0xyz, $vf31x
	vminix.xyz	$vf0xyz, $vf31xyz, $vf0x
	vminix.xyz	$vf1xyz, $vf2xyz, $vf3x
	vminix.xyz	$vf31xyz, $vf0xyz, $vf0x
	vminix.xyz	$vf31xyz, $vf15xyz, $vf7x
	vminix.xyz	$vf31xyz, $vf31xyz, $vf31x
	vminix.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vminix.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vminix.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vminix.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vminix.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vminix.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vminix.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vminix.xz	$vf0xz, $vf0xz, $vf0x
	vminix.xz	$vf0xz, $vf0xz, $vf31x
	vminix.xz	$vf0xz, $vf31xz, $vf0x
	vminix.xz	$vf1xz, $vf2xz, $vf3x
	vminix.xz	$vf31xz, $vf0xz, $vf0x
	vminix.xz	$vf31xz, $vf15xz, $vf7x
	vminix.xz	$vf31xz, $vf31xz, $vf31x
	vminix.xzw	$vf0xzw, $vf0xzw, $vf0x
	vminix.xzw	$vf0xzw, $vf0xzw, $vf31x
	vminix.xzw	$vf0xzw, $vf31xzw, $vf0x
	vminix.xzw	$vf1xzw, $vf2xzw, $vf3x
	vminix.xzw	$vf31xzw, $vf0xzw, $vf0x
	vminix.xzw	$vf31xzw, $vf15xzw, $vf7x
	vminix.xzw	$vf31xzw, $vf31xzw, $vf31x
	vmini.xy	$vf0xy, $vf0xy, $vf0xy
	vmini.xy	$vf0xy, $vf0xy, $vf31xy
	vmini.xy	$vf0xy, $vf31xy, $vf0xy
	vminix.y	$vf0y, $vf0y, $vf0x
	vminix.y	$vf0y, $vf0y, $vf31x
	vminix.y	$vf0y, $vf31y, $vf0x
	vmini.xy	$vf1xy, $vf2xy, $vf3xy
	vminix.y	$vf1y, $vf2y, $vf3x
	vmini.xy	$vf31xy, $vf0xy, $vf0xy
	vmini.xy	$vf31xy, $vf15xy, $vf7xy
	vmini.xy	$vf31xy, $vf31xy, $vf31xy
	vminix.y	$vf31y, $vf0y, $vf0x
	vminix.y	$vf31y, $vf15y, $vf7x
	vminix.y	$vf31y, $vf31y, $vf31x
	vmini.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmini.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vmini.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vminix.yw	$vf0yw, $vf0yw, $vf0x
	vminix.yw	$vf0yw, $vf0yw, $vf31x
	vminix.yw	$vf0yw, $vf31yw, $vf0x
	vmini.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vminix.yw	$vf1yw, $vf2yw, $vf3x
	vmini.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vmini.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vmini.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vminix.yw	$vf31yw, $vf0yw, $vf0x
	vminix.yw	$vf31yw, $vf15yw, $vf7x
	vminix.yw	$vf31yw, $vf31yw, $vf31x
	vmini.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmini.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vmini.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vminix.yz	$vf0yz, $vf0yz, $vf0x
	vminix.yz	$vf0yz, $vf0yz, $vf31x
	vminix.yz	$vf0yz, $vf31yz, $vf0x
	vmini.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vminix.yz	$vf1yz, $vf2yz, $vf3x
	vmini.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vmini.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vmini.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vminix.yz	$vf31yz, $vf0yz, $vf0x
	vminix.yz	$vf31yz, $vf15yz, $vf7x
	vminix.yz	$vf31yz, $vf31yz, $vf31x
	vmini.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmini.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vmini.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vminix.yzw	$vf0yzw, $vf0yzw, $vf0x
	vminix.yzw	$vf0yzw, $vf0yzw, $vf31x
	vminix.yzw	$vf0yzw, $vf31yzw, $vf0x
	vmini.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vminix.yzw	$vf1yzw, $vf2yzw, $vf3x
	vmini.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmini.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vmini.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vminix.yzw	$vf31yzw, $vf0yzw, $vf0x
	vminix.yzw	$vf31yzw, $vf15yzw, $vf7x
	vminix.yzw	$vf31yzw, $vf31yzw, $vf31x
	vmini.xz	$vf0xz, $vf0xz, $vf0xz
	vmini.xz	$vf0xz, $vf0xz, $vf31xz
	vmini.xz	$vf0xz, $vf31xz, $vf0xz
	vminix.z	$vf0z, $vf0z, $vf0x
	vminix.z	$vf0z, $vf0z, $vf31x
	vminix.z	$vf0z, $vf31z, $vf0x
	vmini.xz	$vf1xz, $vf2xz, $vf3xz
	vminix.z	$vf1z, $vf2z, $vf3x
	vmini.xz	$vf31xz, $vf0xz, $vf0xz
	vmini.xz	$vf31xz, $vf15xz, $vf7xz
	vmini.xz	$vf31xz, $vf31xz, $vf31xz
	vminix.z	$vf31z, $vf0z, $vf0x
	vminix.z	$vf31z, $vf15z, $vf7x
	vminix.z	$vf31z, $vf31z, $vf31x
	vmini.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vmini.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmini.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vminix.zw	$vf0zw, $vf0zw, $vf0x
	vminix.zw	$vf0zw, $vf0zw, $vf31x
	vminix.zw	$vf0zw, $vf31zw, $vf0x
	vmini.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vminix.zw	$vf1zw, $vf2zw, $vf3x
	vmini.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vmini.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmini.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vminix.zw	$vf31zw, $vf0zw, $vf0x
	vminix.zw	$vf31zw, $vf15zw, $vf7x
	vminix.zw	$vf31zw, $vf31zw, $vf31x
	vmini.y		$vf0y, $vf0y, $vf0y
	vmini.y		$vf0y, $vf0y, $vf31y
	vmini.y		$vf0y, $vf31y, $vf0y
	vmini.y		$vf1y, $vf2y, $vf3y
	vmini.y		$vf31y, $vf0y, $vf0y
	vmini.y		$vf31y, $vf15y, $vf7y
	vmini.y		$vf31y, $vf31y, $vf31y
	vminiy.w	$vf0w, $vf0w, $vf0y
	vminiy.w	$vf0w, $vf0w, $vf31y
	vminiy.w	$vf0w, $vf31w, $vf0y
	vmini.yw	$vf0yw, $vf0yw, $vf0yw
	vmini.yw	$vf0yw, $vf0yw, $vf31yw
	vmini.yw	$vf0yw, $vf31yw, $vf0yw
	vminiy.w	$vf1w, $vf2w, $vf3y
	vmini.yw	$vf1yw, $vf2yw, $vf3yw
	vminiy.w	$vf31w, $vf0w, $vf0y
	vminiy.w	$vf31w, $vf15w, $vf7y
	vminiy.w	$vf31w, $vf31w, $vf31y
	vmini.yw	$vf31yw, $vf0yw, $vf0yw
	vmini.yw	$vf31yw, $vf15yw, $vf7yw
	vmini.yw	$vf31yw, $vf31yw, $vf31yw
	vminiy.x	$vf0x, $vf0x, $vf0y
	vminiy.x	$vf0x, $vf0x, $vf31y
	vminiy.x	$vf0x, $vf31x, $vf0y
	vminiy.x	$vf1x, $vf2x, $vf3y
	vminiy.x	$vf31x, $vf0x, $vf0y
	vminiy.x	$vf31x, $vf15x, $vf7y
	vminiy.x	$vf31x, $vf31x, $vf31y
	vminiy.xw	$vf0xw, $vf0xw, $vf0y
	vminiy.xw	$vf0xw, $vf0xw, $vf31y
	vminiy.xw	$vf0xw, $vf31xw, $vf0y
	vminiy.xw	$vf1xw, $vf2xw, $vf3y
	vminiy.xw	$vf31xw, $vf0xw, $vf0y
	vminiy.xw	$vf31xw, $vf15xw, $vf7y
	vminiy.xw	$vf31xw, $vf31xw, $vf31y
	vminiy.xy	$vf0xy, $vf0xy, $vf0y
	vminiy.xy	$vf0xy, $vf0xy, $vf31y
	vminiy.xy	$vf0xy, $vf31xy, $vf0y
	vminiy.xy	$vf1xy, $vf2xy, $vf3y
	vminiy.xy	$vf31xy, $vf0xy, $vf0y
	vminiy.xy	$vf31xy, $vf15xy, $vf7y
	vminiy.xy	$vf31xy, $vf31xy, $vf31y
	vminiy.xyw	$vf0xyw, $vf0xyw, $vf0y
	vminiy.xyw	$vf0xyw, $vf0xyw, $vf31y
	vminiy.xyw	$vf0xyw, $vf31xyw, $vf0y
	vminiy.xyw	$vf1xyw, $vf2xyw, $vf3y
	vminiy.xyw	$vf31xyw, $vf0xyw, $vf0y
	vminiy.xyw	$vf31xyw, $vf15xyw, $vf7y
	vminiy.xyw	$vf31xyw, $vf31xyw, $vf31y
	vminiy.xyz	$vf0xyz, $vf0xyz, $vf0y
	vminiy.xyz	$vf0xyz, $vf0xyz, $vf31y
	vminiy.xyz	$vf0xyz, $vf31xyz, $vf0y
	vminiy.xyz	$vf1xyz, $vf2xyz, $vf3y
	vminiy.xyz	$vf31xyz, $vf0xyz, $vf0y
	vminiy.xyz	$vf31xyz, $vf15xyz, $vf7y
	vminiy.xyz	$vf31xyz, $vf31xyz, $vf31y
	vminiy.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vminiy.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vminiy.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vminiy.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vminiy.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vminiy.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vminiy.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vminiy.xz	$vf0xz, $vf0xz, $vf0y
	vminiy.xz	$vf0xz, $vf0xz, $vf31y
	vminiy.xz	$vf0xz, $vf31xz, $vf0y
	vminiy.xz	$vf1xz, $vf2xz, $vf3y
	vminiy.xz	$vf31xz, $vf0xz, $vf0y
	vminiy.xz	$vf31xz, $vf15xz, $vf7y
	vminiy.xz	$vf31xz, $vf31xz, $vf31y
	vminiy.xzw	$vf0xzw, $vf0xzw, $vf0y
	vminiy.xzw	$vf0xzw, $vf0xzw, $vf31y
	vminiy.xzw	$vf0xzw, $vf31xzw, $vf0y
	vminiy.xzw	$vf1xzw, $vf2xzw, $vf3y
	vminiy.xzw	$vf31xzw, $vf0xzw, $vf0y
	vminiy.xzw	$vf31xzw, $vf15xzw, $vf7y
	vminiy.xzw	$vf31xzw, $vf31xzw, $vf31y
	vminiy.y	$vf0y, $vf0y, $vf0y
	vminiy.y	$vf0y, $vf0y, $vf31y
	vminiy.y	$vf0y, $vf31y, $vf0y
	vminiy.y	$vf1y, $vf2y, $vf3y
	vminiy.y	$vf31y, $vf0y, $vf0y
	vminiy.y	$vf31y, $vf15y, $vf7y
	vminiy.y	$vf31y, $vf31y, $vf31y
	vminiy.yw	$vf0yw, $vf0yw, $vf0y
	vminiy.yw	$vf0yw, $vf0yw, $vf31y
	vminiy.yw	$vf0yw, $vf31yw, $vf0y
	vminiy.yw	$vf1yw, $vf2yw, $vf3y
	vminiy.yw	$vf31yw, $vf0yw, $vf0y
	vminiy.yw	$vf31yw, $vf15yw, $vf7y
	vminiy.yw	$vf31yw, $vf31yw, $vf31y
	vminiy.yz	$vf0yz, $vf0yz, $vf0y
	vminiy.yz	$vf0yz, $vf0yz, $vf31y
	vminiy.yz	$vf0yz, $vf31yz, $vf0y
	vminiy.yz	$vf1yz, $vf2yz, $vf3y
	vminiy.yz	$vf31yz, $vf0yz, $vf0y
	vminiy.yz	$vf31yz, $vf15yz, $vf7y
	vminiy.yz	$vf31yz, $vf31yz, $vf31y
	vminiy.yzw	$vf0yzw, $vf0yzw, $vf0y
	vminiy.yzw	$vf0yzw, $vf0yzw, $vf31y
	vminiy.yzw	$vf0yzw, $vf31yzw, $vf0y
	vminiy.yzw	$vf1yzw, $vf2yzw, $vf3y
	vminiy.yzw	$vf31yzw, $vf0yzw, $vf0y
	vminiy.yzw	$vf31yzw, $vf15yzw, $vf7y
	vminiy.yzw	$vf31yzw, $vf31yzw, $vf31y
	vmini.yz	$vf0yz, $vf0yz, $vf0yz
	vmini.yz	$vf0yz, $vf0yz, $vf31yz
	vmini.yz	$vf0yz, $vf31yz, $vf0yz
	vminiy.z	$vf0z, $vf0z, $vf0y
	vminiy.z	$vf0z, $vf0z, $vf31y
	vminiy.z	$vf0z, $vf31z, $vf0y
	vmini.yz	$vf1yz, $vf2yz, $vf3yz
	vminiy.z	$vf1z, $vf2z, $vf3y
	vmini.yz	$vf31yz, $vf0yz, $vf0yz
	vmini.yz	$vf31yz, $vf15yz, $vf7yz
	vmini.yz	$vf31yz, $vf31yz, $vf31yz
	vminiy.z	$vf31z, $vf0z, $vf0y
	vminiy.z	$vf31z, $vf15z, $vf7y
	vminiy.z	$vf31z, $vf31z, $vf31y
	vmini.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmini.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmini.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vminiy.zw	$vf0zw, $vf0zw, $vf0y
	vminiy.zw	$vf0zw, $vf0zw, $vf31y
	vminiy.zw	$vf0zw, $vf31zw, $vf0y
	vmini.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vminiy.zw	$vf1zw, $vf2zw, $vf3y
	vmini.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vmini.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vmini.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vminiy.zw	$vf31zw, $vf0zw, $vf0y
	vminiy.zw	$vf31zw, $vf15zw, $vf7y
	vminiy.zw	$vf31zw, $vf31zw, $vf31y
	vmini.z		$vf0z, $vf0z, $vf0z
	vmini.z		$vf0z, $vf0z, $vf31z
	vmini.z		$vf0z, $vf31z, $vf0z
	vmini.z		$vf1z, $vf2z, $vf3z
	vmini.z		$vf31z, $vf0z, $vf0z
	vmini.z		$vf31z, $vf15z, $vf7z
	vmini.z		$vf31z, $vf31z, $vf31z
	vminiz.w	$vf0w, $vf0w, $vf0z
	vminiz.w	$vf0w, $vf0w, $vf31z
	vminiz.w	$vf0w, $vf31w, $vf0z
	vmini.zw	$vf0zw, $vf0zw, $vf0zw
	vmini.zw	$vf0zw, $vf0zw, $vf31zw
	vmini.zw	$vf0zw, $vf31zw, $vf0zw
	vminiz.w	$vf1w, $vf2w, $vf3z
	vmini.zw	$vf1zw, $vf2zw, $vf3zw
	vminiz.w	$vf31w, $vf0w, $vf0z
	vminiz.w	$vf31w, $vf15w, $vf7z
	vminiz.w	$vf31w, $vf31w, $vf31z
	vmini.zw	$vf31zw, $vf0zw, $vf0zw
	vmini.zw	$vf31zw, $vf15zw, $vf7zw
	vmini.zw	$vf31zw, $vf31zw, $vf31zw
	vminiz.x	$vf0x, $vf0x, $vf0z
	vminiz.x	$vf0x, $vf0x, $vf31z
	vminiz.x	$vf0x, $vf31x, $vf0z
	vminiz.x	$vf1x, $vf2x, $vf3z
	vminiz.x	$vf31x, $vf0x, $vf0z
	vminiz.x	$vf31x, $vf15x, $vf7z
	vminiz.x	$vf31x, $vf31x, $vf31z
	vminiz.xw	$vf0xw, $vf0xw, $vf0z
	vminiz.xw	$vf0xw, $vf0xw, $vf31z
	vminiz.xw	$vf0xw, $vf31xw, $vf0z
	vminiz.xw	$vf1xw, $vf2xw, $vf3z
	vminiz.xw	$vf31xw, $vf0xw, $vf0z
	vminiz.xw	$vf31xw, $vf15xw, $vf7z
	vminiz.xw	$vf31xw, $vf31xw, $vf31z
	vminiz.xy	$vf0xy, $vf0xy, $vf0z
	vminiz.xy	$vf0xy, $vf0xy, $vf31z
	vminiz.xy	$vf0xy, $vf31xy, $vf0z
	vminiz.xy	$vf1xy, $vf2xy, $vf3z
	vminiz.xy	$vf31xy, $vf0xy, $vf0z
	vminiz.xy	$vf31xy, $vf15xy, $vf7z
	vminiz.xy	$vf31xy, $vf31xy, $vf31z
	vminiz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vminiz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vminiz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vminiz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vminiz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vminiz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vminiz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vminiz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vminiz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vminiz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vminiz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vminiz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vminiz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vminiz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vminiz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vminiz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vminiz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vminiz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vminiz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vminiz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vminiz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vminiz.xz	$vf0xz, $vf0xz, $vf0z
	vminiz.xz	$vf0xz, $vf0xz, $vf31z
	vminiz.xz	$vf0xz, $vf31xz, $vf0z
	vminiz.xz	$vf1xz, $vf2xz, $vf3z
	vminiz.xz	$vf31xz, $vf0xz, $vf0z
	vminiz.xz	$vf31xz, $vf15xz, $vf7z
	vminiz.xz	$vf31xz, $vf31xz, $vf31z
	vminiz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vminiz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vminiz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vminiz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vminiz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vminiz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vminiz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vminiz.y	$vf0y, $vf0y, $vf0z
	vminiz.y	$vf0y, $vf0y, $vf31z
	vminiz.y	$vf0y, $vf31y, $vf0z
	vminiz.y	$vf1y, $vf2y, $vf3z
	vminiz.y	$vf31y, $vf0y, $vf0z
	vminiz.y	$vf31y, $vf15y, $vf7z
	vminiz.y	$vf31y, $vf31y, $vf31z
	vminiz.yw	$vf0yw, $vf0yw, $vf0z
	vminiz.yw	$vf0yw, $vf0yw, $vf31z
	vminiz.yw	$vf0yw, $vf31yw, $vf0z
	vminiz.yw	$vf1yw, $vf2yw, $vf3z
	vminiz.yw	$vf31yw, $vf0yw, $vf0z
	vminiz.yw	$vf31yw, $vf15yw, $vf7z
	vminiz.yw	$vf31yw, $vf31yw, $vf31z
	vminiz.yz	$vf0yz, $vf0yz, $vf0z
	vminiz.yz	$vf0yz, $vf0yz, $vf31z
	vminiz.yz	$vf0yz, $vf31yz, $vf0z
	vminiz.yz	$vf1yz, $vf2yz, $vf3z
	vminiz.yz	$vf31yz, $vf0yz, $vf0z
	vminiz.yz	$vf31yz, $vf15yz, $vf7z
	vminiz.yz	$vf31yz, $vf31yz, $vf31z
	vminiz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vminiz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vminiz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vminiz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vminiz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vminiz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vminiz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vminiz.z	$vf0z, $vf0z, $vf0z
	vminiz.z	$vf0z, $vf0z, $vf31z
	vminiz.z	$vf0z, $vf31z, $vf0z
	vminiz.z	$vf1z, $vf2z, $vf3z
	vminiz.z	$vf31z, $vf0z, $vf0z
	vminiz.z	$vf31z, $vf15z, $vf7z
	vminiz.z	$vf31z, $vf31z, $vf31z
	vminiz.zw	$vf0zw, $vf0zw, $vf0z
	vminiz.zw	$vf0zw, $vf0zw, $vf31z
	vminiz.zw	$vf0zw, $vf31zw, $vf0z
	vminiz.zw	$vf1zw, $vf2zw, $vf3z
	vminiz.zw	$vf31zw, $vf0zw, $vf0z
	vminiz.zw	$vf31zw, $vf15zw, $vf7z
	vminiz.zw	$vf31zw, $vf31zw, $vf31z
	vmove.w		$vf0w, $vf0w
	vmove.w		$vf0w, $vf31w
	vmove.w		$vf1w, $vf2w
	vmove.w		$vf31w, $vf0w
	vmove.w		$vf31w, $vf15w
	vmove.w		$vf31w, $vf31w
	vmove.x		$vf0x, $vf0x
	vmove.x		$vf0x, $vf31x
	vmove.x		$vf1x, $vf2x
	vmove.x		$vf31x, $vf0x
	vmove.x		$vf31x, $vf15x
	vmove.x		$vf31x, $vf31x
	vmove.xw	$vf0xw, $vf0xw
	vmove.xw	$vf0xw, $vf31xw
	vmove.xw	$vf1xw, $vf2xw
	vmove.xw	$vf31xw, $vf0xw
	vmove.xw	$vf31xw, $vf15xw
	vmove.xw	$vf31xw, $vf31xw
	vmove.xy	$vf0xy, $vf0xy
	vmove.xy	$vf0xy, $vf31xy
	vmove.xy	$vf1xy, $vf2xy
	vmove.xy	$vf31xy, $vf0xy
	vmove.xy	$vf31xy, $vf15xy
	vmove.xy	$vf31xy, $vf31xy
	vmove.xyw	$vf0xyw, $vf0xyw
	vmove.xyw	$vf0xyw, $vf31xyw
	vmove.xyw	$vf1xyw, $vf2xyw
	vmove.xyw	$vf31xyw, $vf0xyw
	vmove.xyw	$vf31xyw, $vf15xyw
	vmove.xyw	$vf31xyw, $vf31xyw
	vmove.xyz	$vf0xyz, $vf0xyz
	vmove.xyz	$vf0xyz, $vf31xyz
	vmove.xyz	$vf1xyz, $vf2xyz
	vmove.xyz	$vf31xyz, $vf0xyz
	vmove.xyz	$vf31xyz, $vf15xyz
	vmove.xyz	$vf31xyz, $vf31xyz
	vmove.xyzw	$vf0xyzw, $vf0xyzw
	vmove.xyzw	$vf0xyzw, $vf31xyzw
	vmove.xyzw	$vf1xyzw, $vf2xyzw
	vmove.xyzw	$vf31xyzw, $vf0xyzw
	vmove.xyzw	$vf31xyzw, $vf15xyzw
	vmove.xyzw	$vf31xyzw, $vf31xyzw
	vmove.xz	$vf0xz, $vf0xz
	vmove.xz	$vf0xz, $vf31xz
	vmove.xz	$vf1xz, $vf2xz
	vmove.xz	$vf31xz, $vf0xz
	vmove.xz	$vf31xz, $vf15xz
	vmove.xz	$vf31xz, $vf31xz
	vmove.xzw	$vf0xzw, $vf0xzw
	vmove.xzw	$vf0xzw, $vf31xzw
	vmove.xzw	$vf1xzw, $vf2xzw
	vmove.xzw	$vf31xzw, $vf0xzw
	vmove.xzw	$vf31xzw, $vf15xzw
	vmove.xzw	$vf31xzw, $vf31xzw
	vmove.y		$vf0y, $vf0y
	vmove.y		$vf0y, $vf31y
	vmove.y		$vf1y, $vf2y
	vmove.y		$vf31y, $vf0y
	vmove.y		$vf31y, $vf15y
	vmove.y		$vf31y, $vf31y
	vmove.yw	$vf0yw, $vf0yw
	vmove.yw	$vf0yw, $vf31yw
	vmove.yw	$vf1yw, $vf2yw
	vmove.yw	$vf31yw, $vf0yw
	vmove.yw	$vf31yw, $vf15yw
	vmove.yw	$vf31yw, $vf31yw
	vmove.yz	$vf0yz, $vf0yz
	vmove.yz	$vf0yz, $vf31yz
	vmove.yz	$vf1yz, $vf2yz
	vmove.yz	$vf31yz, $vf0yz
	vmove.yz	$vf31yz, $vf15yz
	vmove.yz	$vf31yz, $vf31yz
	vmove.yzw	$vf0yzw, $vf0yzw
	vmove.yzw	$vf0yzw, $vf31yzw
	vmove.yzw	$vf1yzw, $vf2yzw
	vmove.yzw	$vf31yzw, $vf0yzw
	vmove.yzw	$vf31yzw, $vf15yzw
	vmove.yzw	$vf31yzw, $vf31yzw
	vmove.z		$vf0z, $vf0z
	vmove.z		$vf0z, $vf31z
	vmove.z		$vf1z, $vf2z
	vmove.z		$vf31z, $vf0z
	vmove.z		$vf31z, $vf15z
	vmove.z		$vf31z, $vf31z
	vmove.zw	$vf0zw, $vf0zw
	vmove.zw	$vf0zw, $vf31zw
	vmove.zw	$vf1zw, $vf2zw
	vmove.zw	$vf31zw, $vf0zw
	vmove.zw	$vf31zw, $vf15zw
	vmove.zw	$vf31zw, $vf31zw
	vmr32.w		$vf0w, $vf0w
	vmr32.w		$vf0w, $vf31w
	vmr32.w		$vf1w, $vf2w
	vmr32.w		$vf31w, $vf0w
	vmr32.w		$vf31w, $vf15w
	vmr32.w		$vf31w, $vf31w
	vmr32.x		$vf0x, $vf0x
	vmr32.x		$vf0x, $vf31x
	vmr32.x		$vf1x, $vf2x
	vmr32.x		$vf31x, $vf0x
	vmr32.x		$vf31x, $vf15x
	vmr32.x		$vf31x, $vf31x
	vmr32.xw	$vf0xw, $vf0xw
	vmr32.xw	$vf0xw, $vf31xw
	vmr32.xw	$vf1xw, $vf2xw
	vmr32.xw	$vf31xw, $vf0xw
	vmr32.xw	$vf31xw, $vf15xw
	vmr32.xw	$vf31xw, $vf31xw
	vmr32.xy	$vf0xy, $vf0xy
	vmr32.xy	$vf0xy, $vf31xy
	vmr32.xy	$vf1xy, $vf2xy
	vmr32.xy	$vf31xy, $vf0xy
	vmr32.xy	$vf31xy, $vf15xy
	vmr32.xy	$vf31xy, $vf31xy
	vmr32.xyw	$vf0xyw, $vf0xyw
	vmr32.xyw	$vf0xyw, $vf31xyw
	vmr32.xyw	$vf1xyw, $vf2xyw
	vmr32.xyw	$vf31xyw, $vf0xyw
	vmr32.xyw	$vf31xyw, $vf15xyw
	vmr32.xyw	$vf31xyw, $vf31xyw
	vmr32.xyz	$vf0xyz, $vf0xyz
	vmr32.xyz	$vf0xyz, $vf31xyz
	vmr32.xyz	$vf1xyz, $vf2xyz
	vmr32.xyz	$vf31xyz, $vf0xyz
	vmr32.xyz	$vf31xyz, $vf15xyz
	vmr32.xyz	$vf31xyz, $vf31xyz
	vmr32.xyzw	$vf0xyzw, $vf0xyzw
	vmr32.xyzw	$vf0xyzw, $vf31xyzw
	vmr32.xyzw	$vf1xyzw, $vf2xyzw
	vmr32.xyzw	$vf31xyzw, $vf0xyzw
	vmr32.xyzw	$vf31xyzw, $vf15xyzw
	vmr32.xyzw	$vf31xyzw, $vf31xyzw
	vmr32.xz	$vf0xz, $vf0xz
	vmr32.xz	$vf0xz, $vf31xz
	vmr32.xz	$vf1xz, $vf2xz
	vmr32.xz	$vf31xz, $vf0xz
	vmr32.xz	$vf31xz, $vf15xz
	vmr32.xz	$vf31xz, $vf31xz
	vmr32.xzw	$vf0xzw, $vf0xzw
	vmr32.xzw	$vf0xzw, $vf31xzw
	vmr32.xzw	$vf1xzw, $vf2xzw
	vmr32.xzw	$vf31xzw, $vf0xzw
	vmr32.xzw	$vf31xzw, $vf15xzw
	vmr32.xzw	$vf31xzw, $vf31xzw
	vmr32.y		$vf0y, $vf0y
	vmr32.y		$vf0y, $vf31y
	vmr32.y		$vf1y, $vf2y
	vmr32.y		$vf31y, $vf0y
	vmr32.y		$vf31y, $vf15y
	vmr32.y		$vf31y, $vf31y
	vmr32.yw	$vf0yw, $vf0yw
	vmr32.yw	$vf0yw, $vf31yw
	vmr32.yw	$vf1yw, $vf2yw
	vmr32.yw	$vf31yw, $vf0yw
	vmr32.yw	$vf31yw, $vf15yw
	vmr32.yw	$vf31yw, $vf31yw
	vmr32.yz	$vf0yz, $vf0yz
	vmr32.yz	$vf0yz, $vf31yz
	vmr32.yz	$vf1yz, $vf2yz
	vmr32.yz	$vf31yz, $vf0yz
	vmr32.yz	$vf31yz, $vf15yz
	vmr32.yz	$vf31yz, $vf31yz
	vmr32.yzw	$vf0yzw, $vf0yzw
	vmr32.yzw	$vf0yzw, $vf31yzw
	vmr32.yzw	$vf1yzw, $vf2yzw
	vmr32.yzw	$vf31yzw, $vf0yzw
	vmr32.yzw	$vf31yzw, $vf15yzw
	vmr32.yzw	$vf31yzw, $vf31yzw
	vmr32.z		$vf0z, $vf0z
	vmr32.z		$vf0z, $vf31z
	vmr32.z		$vf1z, $vf2z
	vmr32.z		$vf31z, $vf0z
	vmr32.z		$vf31z, $vf15z
	vmr32.z		$vf31z, $vf31z
	vmr32.zw	$vf0zw, $vf0zw
	vmr32.zw	$vf0zw, $vf31zw
	vmr32.zw	$vf1zw, $vf2zw
	vmr32.zw	$vf31zw, $vf0zw
	vmr32.zw	$vf31zw, $vf15zw
	vmr32.zw	$vf31zw, $vf31zw
	vmsubai.w	$ACCw, $vf0w, $I
	vmsubai.w	$ACCw, $vf1w, $I
	vmsubai.w	$ACCw, $vf31w, $I
	vmsubai.x	$ACCx, $vf0x, $I
	vmsubai.x	$ACCx, $vf1x, $I
	vmsubai.x	$ACCx, $vf31x, $I
	vmsubai.xw	$ACCxw, $vf0xw, $I
	vmsubai.xw	$ACCxw, $vf1xw, $I
	vmsubai.xw	$ACCxw, $vf31xw, $I
	vmsubai.xy	$ACCxy, $vf0xy, $I
	vmsubai.xy	$ACCxy, $vf1xy, $I
	vmsubai.xy	$ACCxy, $vf31xy, $I
	vmsubai.xyw	$ACCxyw, $vf0xyw, $I
	vmsubai.xyw	$ACCxyw, $vf1xyw, $I
	vmsubai.xyw	$ACCxyw, $vf31xyw, $I
	vmsubai.xyz	$ACCxyz, $vf0xyz, $I
	vmsubai.xyz	$ACCxyz, $vf1xyz, $I
	vmsubai.xyz	$ACCxyz, $vf31xyz, $I
	vmsubai.xyzw	$ACCxyzw, $vf0xyzw, $I
	vmsubai.xyzw	$ACCxyzw, $vf1xyzw, $I
	vmsubai.xyzw	$ACCxyzw, $vf31xyzw, $I
	vmsubai.xz	$ACCxz, $vf0xz, $I
	vmsubai.xz	$ACCxz, $vf1xz, $I
	vmsubai.xz	$ACCxz, $vf31xz, $I
	vmsubai.xzw	$ACCxzw, $vf0xzw, $I
	vmsubai.xzw	$ACCxzw, $vf1xzw, $I
	vmsubai.xzw	$ACCxzw, $vf31xzw, $I
	vmsubai.y	$ACCy, $vf0y, $I
	vmsubai.y	$ACCy, $vf1y, $I
	vmsubai.y	$ACCy, $vf31y, $I
	vmsubai.yw	$ACCyw, $vf0yw, $I
	vmsubai.yw	$ACCyw, $vf1yw, $I
	vmsubai.yw	$ACCyw, $vf31yw, $I
	vmsubai.yz	$ACCyz, $vf0yz, $I
	vmsubai.yz	$ACCyz, $vf1yz, $I
	vmsubai.yz	$ACCyz, $vf31yz, $I
	vmsubai.yzw	$ACCyzw, $vf0yzw, $I
	vmsubai.yzw	$ACCyzw, $vf1yzw, $I
	vmsubai.yzw	$ACCyzw, $vf31yzw, $I
	vmsubai.z	$ACCz, $vf0z, $I
	vmsubai.z	$ACCz, $vf1z, $I
	vmsubai.z	$ACCz, $vf31z, $I
	vmsubai.zw	$ACCzw, $vf0zw, $I
	vmsubai.zw	$ACCzw, $vf1zw, $I
	vmsubai.zw	$ACCzw, $vf31zw, $I
	vmsubaq.w	$ACCw, $vf0w, $Q
	vmsubaq.w	$ACCw, $vf1w, $Q
	vmsubaq.w	$ACCw, $vf31w, $Q
	vmsubaq.x	$ACCx, $vf0x, $Q
	vmsubaq.x	$ACCx, $vf1x, $Q
	vmsubaq.x	$ACCx, $vf31x, $Q
	vmsubaq.xw	$ACCxw, $vf0xw, $Q
	vmsubaq.xw	$ACCxw, $vf1xw, $Q
	vmsubaq.xw	$ACCxw, $vf31xw, $Q
	vmsubaq.xy	$ACCxy, $vf0xy, $Q
	vmsubaq.xy	$ACCxy, $vf1xy, $Q
	vmsubaq.xy	$ACCxy, $vf31xy, $Q
	vmsubaq.xyw	$ACCxyw, $vf0xyw, $Q
	vmsubaq.xyw	$ACCxyw, $vf1xyw, $Q
	vmsubaq.xyw	$ACCxyw, $vf31xyw, $Q
	vmsubaq.xyz	$ACCxyz, $vf0xyz, $Q
	vmsubaq.xyz	$ACCxyz, $vf1xyz, $Q
	vmsubaq.xyz	$ACCxyz, $vf31xyz, $Q
	vmsubaq.xyzw	$ACCxyzw, $vf0xyzw, $Q
	vmsubaq.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vmsubaq.xyzw	$ACCxyzw, $vf31xyzw, $Q
	vmsubaq.xz	$ACCxz, $vf0xz, $Q
	vmsubaq.xz	$ACCxz, $vf1xz, $Q
	vmsubaq.xz	$ACCxz, $vf31xz, $Q
	vmsubaq.xzw	$ACCxzw, $vf0xzw, $Q
	vmsubaq.xzw	$ACCxzw, $vf1xzw, $Q
	vmsubaq.xzw	$ACCxzw, $vf31xzw, $Q
	vmsubaq.y	$ACCy, $vf0y, $Q
	vmsubaq.y	$ACCy, $vf1y, $Q
	vmsubaq.y	$ACCy, $vf31y, $Q
	vmsubaq.yw	$ACCyw, $vf0yw, $Q
	vmsubaq.yw	$ACCyw, $vf1yw, $Q
	vmsubaq.yw	$ACCyw, $vf31yw, $Q
	vmsubaq.yz	$ACCyz, $vf0yz, $Q
	vmsubaq.yz	$ACCyz, $vf1yz, $Q
	vmsubaq.yz	$ACCyz, $vf31yz, $Q
	vmsubaq.yzw	$ACCyzw, $vf0yzw, $Q
	vmsubaq.yzw	$ACCyzw, $vf1yzw, $Q
	vmsubaq.yzw	$ACCyzw, $vf31yzw, $Q
	vmsubaq.z	$ACCz, $vf0z, $Q
	vmsubaq.z	$ACCz, $vf1z, $Q
	vmsubaq.z	$ACCz, $vf31z, $Q
	vmsubaq.zw	$ACCzw, $vf0zw, $Q
	vmsubaq.zw	$ACCzw, $vf1zw, $Q
	vmsubaq.zw	$ACCzw, $vf31zw, $Q
	vmsuba.w	$ACCw, $vf0w, $vf0w
	vmsuba.w	$ACCw, $vf0w, $vf31w
	vmsuba.w	$ACCw, $vf1w, $vf2w
	vmsuba.w	$ACCw, $vf31w, $vf0w
	vmsuba.w	$ACCw, $vf31w, $vf15w
	vmsuba.w	$ACCw, $vf31w, $vf31w
	vmsubaw.w	$ACCw, $vf0w, $vf0w
	vmsubaw.w	$ACCw, $vf0w, $vf31w
	vmsubaw.w	$ACCw, $vf1w, $vf2w
	vmsubaw.w	$ACCw, $vf31w, $vf0w
	vmsubaw.w	$ACCw, $vf31w, $vf15w
	vmsubaw.w	$ACCw, $vf31w, $vf31w
	vmsubaw.x	$ACCx, $vf0x, $vf0w
	vmsubaw.x	$ACCx, $vf0x, $vf31w
	vmsubaw.x	$ACCx, $vf1x, $vf2w
	vmsubaw.x	$ACCx, $vf31x, $vf0w
	vmsubaw.x	$ACCx, $vf31x, $vf15w
	vmsubaw.x	$ACCx, $vf31x, $vf31w
	vmsubaw.xw	$ACCxw, $vf0xw, $vf0w
	vmsubaw.xw	$ACCxw, $vf0xw, $vf31w
	vmsubaw.xw	$ACCxw, $vf1xw, $vf2w
	vmsubaw.xw	$ACCxw, $vf31xw, $vf0w
	vmsubaw.xw	$ACCxw, $vf31xw, $vf15w
	vmsubaw.xw	$ACCxw, $vf31xw, $vf31w
	vmsubaw.xy	$ACCxy, $vf0xy, $vf0w
	vmsubaw.xy	$ACCxy, $vf0xy, $vf31w
	vmsubaw.xy	$ACCxy, $vf1xy, $vf2w
	vmsubaw.xy	$ACCxy, $vf31xy, $vf0w
	vmsubaw.xy	$ACCxy, $vf31xy, $vf15w
	vmsubaw.xy	$ACCxy, $vf31xy, $vf31w
	vmsubaw.xyw	$ACCxyw, $vf0xyw, $vf0w
	vmsubaw.xyw	$ACCxyw, $vf0xyw, $vf31w
	vmsubaw.xyw	$ACCxyw, $vf1xyw, $vf2w
	vmsubaw.xyw	$ACCxyw, $vf31xyw, $vf0w
	vmsubaw.xyw	$ACCxyw, $vf31xyw, $vf15w
	vmsubaw.xyw	$ACCxyw, $vf31xyw, $vf31w
	vmsubaw.xyz	$ACCxyz, $vf0xyz, $vf0w
	vmsubaw.xyz	$ACCxyz, $vf0xyz, $vf31w
	vmsubaw.xyz	$ACCxyz, $vf1xyz, $vf2w
	vmsubaw.xyz	$ACCxyz, $vf31xyz, $vf0w
	vmsubaw.xyz	$ACCxyz, $vf31xyz, $vf15w
	vmsubaw.xyz	$ACCxyz, $vf31xyz, $vf31w
	vmsubaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0w
	vmsubaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31w
	vmsubaw.xyzw	$ACCxyzw, $vf1xyzw, $vf2w
	vmsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf0w
	vmsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf15w
	vmsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf31w
	vmsubaw.xz	$ACCxz, $vf0xz, $vf0w
	vmsubaw.xz	$ACCxz, $vf0xz, $vf31w
	vmsubaw.xz	$ACCxz, $vf1xz, $vf2w
	vmsubaw.xz	$ACCxz, $vf31xz, $vf0w
	vmsubaw.xz	$ACCxz, $vf31xz, $vf15w
	vmsubaw.xz	$ACCxz, $vf31xz, $vf31w
	vmsubaw.xzw	$ACCxzw, $vf0xzw, $vf0w
	vmsubaw.xzw	$ACCxzw, $vf0xzw, $vf31w
	vmsubaw.xzw	$ACCxzw, $vf1xzw, $vf2w
	vmsubaw.xzw	$ACCxzw, $vf31xzw, $vf0w
	vmsubaw.xzw	$ACCxzw, $vf31xzw, $vf15w
	vmsubaw.xzw	$ACCxzw, $vf31xzw, $vf31w
	vmsubaw.y	$ACCy, $vf0y, $vf0w
	vmsubaw.y	$ACCy, $vf0y, $vf31w
	vmsubaw.y	$ACCy, $vf1y, $vf2w
	vmsubaw.y	$ACCy, $vf31y, $vf0w
	vmsubaw.y	$ACCy, $vf31y, $vf15w
	vmsubaw.y	$ACCy, $vf31y, $vf31w
	vmsubaw.yw	$ACCyw, $vf0yw, $vf0w
	vmsubaw.yw	$ACCyw, $vf0yw, $vf31w
	vmsubaw.yw	$ACCyw, $vf1yw, $vf2w
	vmsubaw.yw	$ACCyw, $vf31yw, $vf0w
	vmsubaw.yw	$ACCyw, $vf31yw, $vf15w
	vmsubaw.yw	$ACCyw, $vf31yw, $vf31w
	vmsubaw.yz	$ACCyz, $vf0yz, $vf0w
	vmsubaw.yz	$ACCyz, $vf0yz, $vf31w
	vmsubaw.yz	$ACCyz, $vf1yz, $vf2w
	vmsubaw.yz	$ACCyz, $vf31yz, $vf0w
	vmsubaw.yz	$ACCyz, $vf31yz, $vf15w
	vmsubaw.yz	$ACCyz, $vf31yz, $vf31w
	vmsubaw.yzw	$ACCyzw, $vf0yzw, $vf0w
	vmsubaw.yzw	$ACCyzw, $vf0yzw, $vf31w
	vmsubaw.yzw	$ACCyzw, $vf1yzw, $vf2w
	vmsubaw.yzw	$ACCyzw, $vf31yzw, $vf0w
	vmsubaw.yzw	$ACCyzw, $vf31yzw, $vf15w
	vmsubaw.yzw	$ACCyzw, $vf31yzw, $vf31w
	vmsubaw.z	$ACCz, $vf0z, $vf0w
	vmsubaw.z	$ACCz, $vf0z, $vf31w
	vmsubaw.z	$ACCz, $vf1z, $vf2w
	vmsubaw.z	$ACCz, $vf31z, $vf0w
	vmsubaw.z	$ACCz, $vf31z, $vf15w
	vmsubaw.z	$ACCz, $vf31z, $vf31w
	vmsubaw.zw	$ACCzw, $vf0zw, $vf0w
	vmsubaw.zw	$ACCzw, $vf0zw, $vf31w
	vmsubaw.zw	$ACCzw, $vf1zw, $vf2w
	vmsubaw.zw	$ACCzw, $vf31zw, $vf0w
	vmsubaw.zw	$ACCzw, $vf31zw, $vf15w
	vmsubaw.zw	$ACCzw, $vf31zw, $vf31w
	vmsuba.x	$ACCx, $vf0x, $vf0x
	vmsuba.x	$ACCx, $vf0x, $vf31x
	vmsuba.x	$ACCx, $vf1x, $vf2x
	vmsuba.x	$ACCx, $vf31x, $vf0x
	vmsuba.x	$ACCx, $vf31x, $vf15x
	vmsuba.x	$ACCx, $vf31x, $vf31x
	vmsubax.w	$ACCw, $vf0w, $vf0x
	vmsubax.w	$ACCw, $vf0w, $vf31x
	vmsubax.w	$ACCw, $vf1w, $vf2x
	vmsubax.w	$ACCw, $vf31w, $vf0x
	vmsubax.w	$ACCw, $vf31w, $vf15x
	vmsubax.w	$ACCw, $vf31w, $vf31x
	vmsuba.xw	$ACCxw, $vf0xw, $vf0xw
	vmsuba.xw	$ACCxw, $vf0xw, $vf31xw
	vmsuba.xw	$ACCxw, $vf1xw, $vf2xw
	vmsuba.xw	$ACCxw, $vf31xw, $vf0xw
	vmsuba.xw	$ACCxw, $vf31xw, $vf15xw
	vmsuba.xw	$ACCxw, $vf31xw, $vf31xw
	vmsubax.x	$ACCx, $vf0x, $vf0x
	vmsubax.x	$ACCx, $vf0x, $vf31x
	vmsubax.x	$ACCx, $vf1x, $vf2x
	vmsubax.x	$ACCx, $vf31x, $vf0x
	vmsubax.x	$ACCx, $vf31x, $vf15x
	vmsubax.x	$ACCx, $vf31x, $vf31x
	vmsubax.xw	$ACCxw, $vf0xw, $vf0x
	vmsubax.xw	$ACCxw, $vf0xw, $vf31x
	vmsubax.xw	$ACCxw, $vf1xw, $vf2x
	vmsubax.xw	$ACCxw, $vf31xw, $vf0x
	vmsubax.xw	$ACCxw, $vf31xw, $vf15x
	vmsubax.xw	$ACCxw, $vf31xw, $vf31x
	vmsubax.xy	$ACCxy, $vf0xy, $vf0x
	vmsubax.xy	$ACCxy, $vf0xy, $vf31x
	vmsubax.xy	$ACCxy, $vf1xy, $vf2x
	vmsubax.xy	$ACCxy, $vf31xy, $vf0x
	vmsubax.xy	$ACCxy, $vf31xy, $vf15x
	vmsubax.xy	$ACCxy, $vf31xy, $vf31x
	vmsubax.xyw	$ACCxyw, $vf0xyw, $vf0x
	vmsubax.xyw	$ACCxyw, $vf0xyw, $vf31x
	vmsubax.xyw	$ACCxyw, $vf1xyw, $vf2x
	vmsubax.xyw	$ACCxyw, $vf31xyw, $vf0x
	vmsubax.xyw	$ACCxyw, $vf31xyw, $vf15x
	vmsubax.xyw	$ACCxyw, $vf31xyw, $vf31x
	vmsubax.xyz	$ACCxyz, $vf0xyz, $vf0x
	vmsubax.xyz	$ACCxyz, $vf0xyz, $vf31x
	vmsubax.xyz	$ACCxyz, $vf1xyz, $vf2x
	vmsubax.xyz	$ACCxyz, $vf31xyz, $vf0x
	vmsubax.xyz	$ACCxyz, $vf31xyz, $vf15x
	vmsubax.xyz	$ACCxyz, $vf31xyz, $vf31x
	vmsubax.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vmsubax.xyzw	$ACCxyzw, $vf0xyzw, $vf31x
	vmsubax.xyzw	$ACCxyzw, $vf1xyzw, $vf2x
	vmsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf0x
	vmsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf15x
	vmsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf31x
	vmsubax.xz	$ACCxz, $vf0xz, $vf0x
	vmsubax.xz	$ACCxz, $vf0xz, $vf31x
	vmsubax.xz	$ACCxz, $vf1xz, $vf2x
	vmsubax.xz	$ACCxz, $vf31xz, $vf0x
	vmsubax.xz	$ACCxz, $vf31xz, $vf15x
	vmsubax.xz	$ACCxz, $vf31xz, $vf31x
	vmsubax.xzw	$ACCxzw, $vf0xzw, $vf0x
	vmsubax.xzw	$ACCxzw, $vf0xzw, $vf31x
	vmsubax.xzw	$ACCxzw, $vf1xzw, $vf2x
	vmsubax.xzw	$ACCxzw, $vf31xzw, $vf0x
	vmsubax.xzw	$ACCxzw, $vf31xzw, $vf15x
	vmsubax.xzw	$ACCxzw, $vf31xzw, $vf31x
	vmsuba.xy	$ACCxy, $vf0xy, $vf0xy
	vmsuba.xy	$ACCxy, $vf0xy, $vf31xy
	vmsuba.xy	$ACCxy, $vf1xy, $vf2xy
	vmsuba.xy	$ACCxy, $vf31xy, $vf0xy
	vmsuba.xy	$ACCxy, $vf31xy, $vf15xy
	vmsuba.xy	$ACCxy, $vf31xy, $vf31xy
	vmsubax.y	$ACCy, $vf0y, $vf0x
	vmsubax.y	$ACCy, $vf0y, $vf31x
	vmsubax.y	$ACCy, $vf1y, $vf2x
	vmsubax.y	$ACCy, $vf31y, $vf0x
	vmsubax.y	$ACCy, $vf31y, $vf15x
	vmsubax.y	$ACCy, $vf31y, $vf31x
	vmsuba.xyw	$ACCxyw, $vf0xyw, $vf0xyw
	vmsuba.xyw	$ACCxyw, $vf0xyw, $vf31xyw
	vmsuba.xyw	$ACCxyw, $vf1xyw, $vf2xyw
	vmsuba.xyw	$ACCxyw, $vf31xyw, $vf0xyw
	vmsuba.xyw	$ACCxyw, $vf31xyw, $vf15xyw
	vmsuba.xyw	$ACCxyw, $vf31xyw, $vf31xyw
	vmsubax.yw	$ACCyw, $vf0yw, $vf0x
	vmsubax.yw	$ACCyw, $vf0yw, $vf31x
	vmsubax.yw	$ACCyw, $vf1yw, $vf2x
	vmsubax.yw	$ACCyw, $vf31yw, $vf0x
	vmsubax.yw	$ACCyw, $vf31yw, $vf15x
	vmsubax.yw	$ACCyw, $vf31yw, $vf31x
	vmsuba.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vmsuba.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vmsuba.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vmsuba.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vmsuba.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vmsuba.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vmsubax.yz	$ACCyz, $vf0yz, $vf0x
	vmsubax.yz	$ACCyz, $vf0yz, $vf31x
	vmsubax.yz	$ACCyz, $vf1yz, $vf2x
	vmsubax.yz	$ACCyz, $vf31yz, $vf0x
	vmsubax.yz	$ACCyz, $vf31yz, $vf15x
	vmsubax.yz	$ACCyz, $vf31yz, $vf31x
	vmsuba.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vmsuba.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vmsuba.xyzw	$ACCxyzw, $vf1xyzw, $vf2xyzw
	vmsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vmsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf15xyzw
	vmsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf31xyzw
	vmsubax.yzw	$ACCyzw, $vf0yzw, $vf0x
	vmsubax.yzw	$ACCyzw, $vf0yzw, $vf31x
	vmsubax.yzw	$ACCyzw, $vf1yzw, $vf2x
	vmsubax.yzw	$ACCyzw, $vf31yzw, $vf0x
	vmsubax.yzw	$ACCyzw, $vf31yzw, $vf15x
	vmsubax.yzw	$ACCyzw, $vf31yzw, $vf31x
	vmsuba.xz	$ACCxz, $vf0xz, $vf0xz
	vmsuba.xz	$ACCxz, $vf0xz, $vf31xz
	vmsuba.xz	$ACCxz, $vf1xz, $vf2xz
	vmsuba.xz	$ACCxz, $vf31xz, $vf0xz
	vmsuba.xz	$ACCxz, $vf31xz, $vf15xz
	vmsuba.xz	$ACCxz, $vf31xz, $vf31xz
	vmsubax.z	$ACCz, $vf0z, $vf0x
	vmsubax.z	$ACCz, $vf0z, $vf31x
	vmsubax.z	$ACCz, $vf1z, $vf2x
	vmsubax.z	$ACCz, $vf31z, $vf0x
	vmsubax.z	$ACCz, $vf31z, $vf15x
	vmsubax.z	$ACCz, $vf31z, $vf31x
	vmsuba.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vmsuba.xzw	$ACCxzw, $vf0xzw, $vf31xzw
	vmsuba.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vmsuba.xzw	$ACCxzw, $vf31xzw, $vf0xzw
	vmsuba.xzw	$ACCxzw, $vf31xzw, $vf15xzw
	vmsuba.xzw	$ACCxzw, $vf31xzw, $vf31xzw
	vmsubax.zw	$ACCzw, $vf0zw, $vf0x
	vmsubax.zw	$ACCzw, $vf0zw, $vf31x
	vmsubax.zw	$ACCzw, $vf1zw, $vf2x
	vmsubax.zw	$ACCzw, $vf31zw, $vf0x
	vmsubax.zw	$ACCzw, $vf31zw, $vf15x
	vmsubax.zw	$ACCzw, $vf31zw, $vf31x
	vmsuba.y	$ACCy, $vf0y, $vf0y
	vmsuba.y	$ACCy, $vf0y, $vf31y
	vmsuba.y	$ACCy, $vf1y, $vf2y
	vmsuba.y	$ACCy, $vf31y, $vf0y
	vmsuba.y	$ACCy, $vf31y, $vf15y
	vmsuba.y	$ACCy, $vf31y, $vf31y
	vmsubay.w	$ACCw, $vf0w, $vf0y
	vmsubay.w	$ACCw, $vf0w, $vf31y
	vmsubay.w	$ACCw, $vf1w, $vf2y
	vmsubay.w	$ACCw, $vf31w, $vf0y
	vmsubay.w	$ACCw, $vf31w, $vf15y
	vmsubay.w	$ACCw, $vf31w, $vf31y
	vmsuba.yw	$ACCyw, $vf0yw, $vf0yw
	vmsuba.yw	$ACCyw, $vf0yw, $vf31yw
	vmsuba.yw	$ACCyw, $vf1yw, $vf2yw
	vmsuba.yw	$ACCyw, $vf31yw, $vf0yw
	vmsuba.yw	$ACCyw, $vf31yw, $vf15yw
	vmsuba.yw	$ACCyw, $vf31yw, $vf31yw
	vmsubay.x	$ACCx, $vf0x, $vf0y
	vmsubay.x	$ACCx, $vf0x, $vf31y
	vmsubay.x	$ACCx, $vf1x, $vf2y
	vmsubay.x	$ACCx, $vf31x, $vf0y
	vmsubay.x	$ACCx, $vf31x, $vf15y
	vmsubay.x	$ACCx, $vf31x, $vf31y
	vmsubay.xw	$ACCxw, $vf0xw, $vf0y
	vmsubay.xw	$ACCxw, $vf0xw, $vf31y
	vmsubay.xw	$ACCxw, $vf1xw, $vf2y
	vmsubay.xw	$ACCxw, $vf31xw, $vf0y
	vmsubay.xw	$ACCxw, $vf31xw, $vf15y
	vmsubay.xw	$ACCxw, $vf31xw, $vf31y
	vmsubay.xy	$ACCxy, $vf0xy, $vf0y
	vmsubay.xy	$ACCxy, $vf0xy, $vf31y
	vmsubay.xy	$ACCxy, $vf1xy, $vf2y
	vmsubay.xy	$ACCxy, $vf31xy, $vf0y
	vmsubay.xy	$ACCxy, $vf31xy, $vf15y
	vmsubay.xy	$ACCxy, $vf31xy, $vf31y
	vmsubay.xyw	$ACCxyw, $vf0xyw, $vf0y
	vmsubay.xyw	$ACCxyw, $vf0xyw, $vf31y
	vmsubay.xyw	$ACCxyw, $vf1xyw, $vf2y
	vmsubay.xyw	$ACCxyw, $vf31xyw, $vf0y
	vmsubay.xyw	$ACCxyw, $vf31xyw, $vf15y
	vmsubay.xyw	$ACCxyw, $vf31xyw, $vf31y
	vmsubay.xyz	$ACCxyz, $vf0xyz, $vf0y
	vmsubay.xyz	$ACCxyz, $vf0xyz, $vf31y
	vmsubay.xyz	$ACCxyz, $vf1xyz, $vf2y
	vmsubay.xyz	$ACCxyz, $vf31xyz, $vf0y
	vmsubay.xyz	$ACCxyz, $vf31xyz, $vf15y
	vmsubay.xyz	$ACCxyz, $vf31xyz, $vf31y
	vmsubay.xyzw	$ACCxyzw, $vf0xyzw, $vf0y
	vmsubay.xyzw	$ACCxyzw, $vf0xyzw, $vf31y
	vmsubay.xyzw	$ACCxyzw, $vf1xyzw, $vf2y
	vmsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf0y
	vmsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf15y
	vmsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf31y
	vmsubay.xz	$ACCxz, $vf0xz, $vf0y
	vmsubay.xz	$ACCxz, $vf0xz, $vf31y
	vmsubay.xz	$ACCxz, $vf1xz, $vf2y
	vmsubay.xz	$ACCxz, $vf31xz, $vf0y
	vmsubay.xz	$ACCxz, $vf31xz, $vf15y
	vmsubay.xz	$ACCxz, $vf31xz, $vf31y
	vmsubay.xzw	$ACCxzw, $vf0xzw, $vf0y
	vmsubay.xzw	$ACCxzw, $vf0xzw, $vf31y
	vmsubay.xzw	$ACCxzw, $vf1xzw, $vf2y
	vmsubay.xzw	$ACCxzw, $vf31xzw, $vf0y
	vmsubay.xzw	$ACCxzw, $vf31xzw, $vf15y
	vmsubay.xzw	$ACCxzw, $vf31xzw, $vf31y
	vmsubay.y	$ACCy, $vf0y, $vf0y
	vmsubay.y	$ACCy, $vf0y, $vf31y
	vmsubay.y	$ACCy, $vf1y, $vf2y
	vmsubay.y	$ACCy, $vf31y, $vf0y
	vmsubay.y	$ACCy, $vf31y, $vf15y
	vmsubay.y	$ACCy, $vf31y, $vf31y
	vmsubay.yw	$ACCyw, $vf0yw, $vf0y
	vmsubay.yw	$ACCyw, $vf0yw, $vf31y
	vmsubay.yw	$ACCyw, $vf1yw, $vf2y
	vmsubay.yw	$ACCyw, $vf31yw, $vf0y
	vmsubay.yw	$ACCyw, $vf31yw, $vf15y
	vmsubay.yw	$ACCyw, $vf31yw, $vf31y
	vmsubay.yz	$ACCyz, $vf0yz, $vf0y
	vmsubay.yz	$ACCyz, $vf0yz, $vf31y
	vmsubay.yz	$ACCyz, $vf1yz, $vf2y
	vmsubay.yz	$ACCyz, $vf31yz, $vf0y
	vmsubay.yz	$ACCyz, $vf31yz, $vf15y
	vmsubay.yz	$ACCyz, $vf31yz, $vf31y
	vmsubay.yzw	$ACCyzw, $vf0yzw, $vf0y
	vmsubay.yzw	$ACCyzw, $vf0yzw, $vf31y
	vmsubay.yzw	$ACCyzw, $vf1yzw, $vf2y
	vmsubay.yzw	$ACCyzw, $vf31yzw, $vf0y
	vmsubay.yzw	$ACCyzw, $vf31yzw, $vf15y
	vmsubay.yzw	$ACCyzw, $vf31yzw, $vf31y
	vmsuba.yz	$ACCyz, $vf0yz, $vf0yz
	vmsuba.yz	$ACCyz, $vf0yz, $vf31yz
	vmsuba.yz	$ACCyz, $vf1yz, $vf2yz
	vmsuba.yz	$ACCyz, $vf31yz, $vf0yz
	vmsuba.yz	$ACCyz, $vf31yz, $vf15yz
	vmsuba.yz	$ACCyz, $vf31yz, $vf31yz
	vmsubay.z	$ACCz, $vf0z, $vf0y
	vmsubay.z	$ACCz, $vf0z, $vf31y
	vmsubay.z	$ACCz, $vf1z, $vf2y
	vmsubay.z	$ACCz, $vf31z, $vf0y
	vmsubay.z	$ACCz, $vf31z, $vf15y
	vmsubay.z	$ACCz, $vf31z, $vf31y
	vmsuba.yzw	$ACCyzw, $vf0yzw, $vf0yzw
	vmsuba.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vmsuba.yzw	$ACCyzw, $vf1yzw, $vf2yzw
	vmsuba.yzw	$ACCyzw, $vf31yzw, $vf0yzw
	vmsuba.yzw	$ACCyzw, $vf31yzw, $vf15yzw
	vmsuba.yzw	$ACCyzw, $vf31yzw, $vf31yzw
	vmsubay.zw	$ACCzw, $vf0zw, $vf0y
	vmsubay.zw	$ACCzw, $vf0zw, $vf31y
	vmsubay.zw	$ACCzw, $vf1zw, $vf2y
	vmsubay.zw	$ACCzw, $vf31zw, $vf0y
	vmsubay.zw	$ACCzw, $vf31zw, $vf15y
	vmsubay.zw	$ACCzw, $vf31zw, $vf31y
	vmsuba.z	$ACCz, $vf0z, $vf0z
	vmsuba.z	$ACCz, $vf0z, $vf31z
	vmsuba.z	$ACCz, $vf1z, $vf2z
	vmsuba.z	$ACCz, $vf31z, $vf0z
	vmsuba.z	$ACCz, $vf31z, $vf15z
	vmsuba.z	$ACCz, $vf31z, $vf31z
	vmsubaz.w	$ACCw, $vf0w, $vf0z
	vmsubaz.w	$ACCw, $vf0w, $vf31z
	vmsubaz.w	$ACCw, $vf1w, $vf2z
	vmsubaz.w	$ACCw, $vf31w, $vf0z
	vmsubaz.w	$ACCw, $vf31w, $vf15z
	vmsubaz.w	$ACCw, $vf31w, $vf31z
	vmsuba.zw	$ACCzw, $vf0zw, $vf0zw
	vmsuba.zw	$ACCzw, $vf0zw, $vf31zw
	vmsuba.zw	$ACCzw, $vf1zw, $vf2zw
	vmsuba.zw	$ACCzw, $vf31zw, $vf0zw
	vmsuba.zw	$ACCzw, $vf31zw, $vf15zw
	vmsuba.zw	$ACCzw, $vf31zw, $vf31zw
	vmsubaz.x	$ACCx, $vf0x, $vf0z
	vmsubaz.x	$ACCx, $vf0x, $vf31z
	vmsubaz.x	$ACCx, $vf1x, $vf2z
	vmsubaz.x	$ACCx, $vf31x, $vf0z
	vmsubaz.x	$ACCx, $vf31x, $vf15z
	vmsubaz.x	$ACCx, $vf31x, $vf31z
	vmsubaz.xw	$ACCxw, $vf0xw, $vf0z
	vmsubaz.xw	$ACCxw, $vf0xw, $vf31z
	vmsubaz.xw	$ACCxw, $vf1xw, $vf2z
	vmsubaz.xw	$ACCxw, $vf31xw, $vf0z
	vmsubaz.xw	$ACCxw, $vf31xw, $vf15z
	vmsubaz.xw	$ACCxw, $vf31xw, $vf31z
	vmsubaz.xy	$ACCxy, $vf0xy, $vf0z
	vmsubaz.xy	$ACCxy, $vf0xy, $vf31z
	vmsubaz.xy	$ACCxy, $vf1xy, $vf2z
	vmsubaz.xy	$ACCxy, $vf31xy, $vf0z
	vmsubaz.xy	$ACCxy, $vf31xy, $vf15z
	vmsubaz.xy	$ACCxy, $vf31xy, $vf31z
	vmsubaz.xyw	$ACCxyw, $vf0xyw, $vf0z
	vmsubaz.xyw	$ACCxyw, $vf0xyw, $vf31z
	vmsubaz.xyw	$ACCxyw, $vf1xyw, $vf2z
	vmsubaz.xyw	$ACCxyw, $vf31xyw, $vf0z
	vmsubaz.xyw	$ACCxyw, $vf31xyw, $vf15z
	vmsubaz.xyw	$ACCxyw, $vf31xyw, $vf31z
	vmsubaz.xyz	$ACCxyz, $vf0xyz, $vf0z
	vmsubaz.xyz	$ACCxyz, $vf0xyz, $vf31z
	vmsubaz.xyz	$ACCxyz, $vf1xyz, $vf2z
	vmsubaz.xyz	$ACCxyz, $vf31xyz, $vf0z
	vmsubaz.xyz	$ACCxyz, $vf31xyz, $vf15z
	vmsubaz.xyz	$ACCxyz, $vf31xyz, $vf31z
	vmsubaz.xyzw	$ACCxyzw, $vf0xyzw, $vf0z
	vmsubaz.xyzw	$ACCxyzw, $vf0xyzw, $vf31z
	vmsubaz.xyzw	$ACCxyzw, $vf1xyzw, $vf2z
	vmsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf0z
	vmsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf15z
	vmsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf31z
	vmsubaz.xz	$ACCxz, $vf0xz, $vf0z
	vmsubaz.xz	$ACCxz, $vf0xz, $vf31z
	vmsubaz.xz	$ACCxz, $vf1xz, $vf2z
	vmsubaz.xz	$ACCxz, $vf31xz, $vf0z
	vmsubaz.xz	$ACCxz, $vf31xz, $vf15z
	vmsubaz.xz	$ACCxz, $vf31xz, $vf31z
	vmsubaz.xzw	$ACCxzw, $vf0xzw, $vf0z
	vmsubaz.xzw	$ACCxzw, $vf0xzw, $vf31z
	vmsubaz.xzw	$ACCxzw, $vf1xzw, $vf2z
	vmsubaz.xzw	$ACCxzw, $vf31xzw, $vf0z
	vmsubaz.xzw	$ACCxzw, $vf31xzw, $vf15z
	vmsubaz.xzw	$ACCxzw, $vf31xzw, $vf31z
	vmsubaz.y	$ACCy, $vf0y, $vf0z
	vmsubaz.y	$ACCy, $vf0y, $vf31z
	vmsubaz.y	$ACCy, $vf1y, $vf2z
	vmsubaz.y	$ACCy, $vf31y, $vf0z
	vmsubaz.y	$ACCy, $vf31y, $vf15z
	vmsubaz.y	$ACCy, $vf31y, $vf31z
	vmsubaz.yw	$ACCyw, $vf0yw, $vf0z
	vmsubaz.yw	$ACCyw, $vf0yw, $vf31z
	vmsubaz.yw	$ACCyw, $vf1yw, $vf2z
	vmsubaz.yw	$ACCyw, $vf31yw, $vf0z
	vmsubaz.yw	$ACCyw, $vf31yw, $vf15z
	vmsubaz.yw	$ACCyw, $vf31yw, $vf31z
	vmsubaz.yz	$ACCyz, $vf0yz, $vf0z
	vmsubaz.yz	$ACCyz, $vf0yz, $vf31z
	vmsubaz.yz	$ACCyz, $vf1yz, $vf2z
	vmsubaz.yz	$ACCyz, $vf31yz, $vf0z
	vmsubaz.yz	$ACCyz, $vf31yz, $vf15z
	vmsubaz.yz	$ACCyz, $vf31yz, $vf31z
	vmsubaz.yzw	$ACCyzw, $vf0yzw, $vf0z
	vmsubaz.yzw	$ACCyzw, $vf0yzw, $vf31z
	vmsubaz.yzw	$ACCyzw, $vf1yzw, $vf2z
	vmsubaz.yzw	$ACCyzw, $vf31yzw, $vf0z
	vmsubaz.yzw	$ACCyzw, $vf31yzw, $vf15z
	vmsubaz.yzw	$ACCyzw, $vf31yzw, $vf31z
	vmsubaz.z	$ACCz, $vf0z, $vf0z
	vmsubaz.z	$ACCz, $vf0z, $vf31z
	vmsubaz.z	$ACCz, $vf1z, $vf2z
	vmsubaz.z	$ACCz, $vf31z, $vf0z
	vmsubaz.z	$ACCz, $vf31z, $vf15z
	vmsubaz.z	$ACCz, $vf31z, $vf31z
	vmsubaz.zw	$ACCzw, $vf0zw, $vf0z
	vmsubaz.zw	$ACCzw, $vf0zw, $vf31z
	vmsubaz.zw	$ACCzw, $vf1zw, $vf2z
	vmsubaz.zw	$ACCzw, $vf31zw, $vf0z
	vmsubaz.zw	$ACCzw, $vf31zw, $vf15z
	vmsubaz.zw	$ACCzw, $vf31zw, $vf31z
	vmsubi.w	$vf0w, $vf0w, $I
	vmsubi.w	$vf0w, $vf31w, $I
	vmsubi.w	$vf1w, $vf2w, $I
	vmsubi.w	$vf31w, $vf0w, $I
	vmsubi.w	$vf31w, $vf15w, $I
	vmsubi.w	$vf31w, $vf31w, $I
	vmsubi.x	$vf0x, $vf0x, $I
	vmsubi.x	$vf0x, $vf31x, $I
	vmsubi.x	$vf1x, $vf2x, $I
	vmsubi.x	$vf31x, $vf0x, $I
	vmsubi.x	$vf31x, $vf15x, $I
	vmsubi.x	$vf31x, $vf31x, $I
	vmsubi.xw	$vf0xw, $vf0xw, $I
	vmsubi.xw	$vf0xw, $vf31xw, $I
	vmsubi.xw	$vf1xw, $vf2xw, $I
	vmsubi.xw	$vf31xw, $vf0xw, $I
	vmsubi.xw	$vf31xw, $vf15xw, $I
	vmsubi.xw	$vf31xw, $vf31xw, $I
	vmsubi.xy	$vf0xy, $vf0xy, $I
	vmsubi.xy	$vf0xy, $vf31xy, $I
	vmsubi.xy	$vf1xy, $vf2xy, $I
	vmsubi.xy	$vf31xy, $vf0xy, $I
	vmsubi.xy	$vf31xy, $vf15xy, $I
	vmsubi.xy	$vf31xy, $vf31xy, $I
	vmsubi.xyw	$vf0xyw, $vf0xyw, $I
	vmsubi.xyw	$vf0xyw, $vf31xyw, $I
	vmsubi.xyw	$vf1xyw, $vf2xyw, $I
	vmsubi.xyw	$vf31xyw, $vf0xyw, $I
	vmsubi.xyw	$vf31xyw, $vf15xyw, $I
	vmsubi.xyw	$vf31xyw, $vf31xyw, $I
	vmsubi.xyz	$vf0xyz, $vf0xyz, $I
	vmsubi.xyz	$vf0xyz, $vf31xyz, $I
	vmsubi.xyz	$vf1xyz, $vf2xyz, $I
	vmsubi.xyz	$vf31xyz, $vf0xyz, $I
	vmsubi.xyz	$vf31xyz, $vf15xyz, $I
	vmsubi.xyz	$vf31xyz, $vf31xyz, $I
	vmsubi.xyzw	$vf0xyzw, $vf0xyzw, $I
	vmsubi.xyzw	$vf0xyzw, $vf31xyzw, $I
	vmsubi.xyzw	$vf1xyzw, $vf2xyzw, $I
	vmsubi.xyzw	$vf31xyzw, $vf0xyzw, $I
	vmsubi.xyzw	$vf31xyzw, $vf15xyzw, $I
	vmsubi.xyzw	$vf31xyzw, $vf31xyzw, $I
	vmsubi.xz	$vf0xz, $vf0xz, $I
	vmsubi.xz	$vf0xz, $vf31xz, $I
	vmsubi.xz	$vf1xz, $vf2xz, $I
	vmsubi.xz	$vf31xz, $vf0xz, $I
	vmsubi.xz	$vf31xz, $vf15xz, $I
	vmsubi.xz	$vf31xz, $vf31xz, $I
	vmsubi.xzw	$vf0xzw, $vf0xzw, $I
	vmsubi.xzw	$vf0xzw, $vf31xzw, $I
	vmsubi.xzw	$vf1xzw, $vf2xzw, $I
	vmsubi.xzw	$vf31xzw, $vf0xzw, $I
	vmsubi.xzw	$vf31xzw, $vf15xzw, $I
	vmsubi.xzw	$vf31xzw, $vf31xzw, $I
	vmsubi.y	$vf0y, $vf0y, $I
	vmsubi.y	$vf0y, $vf31y, $I
	vmsubi.y	$vf1y, $vf2y, $I
	vmsubi.y	$vf31y, $vf0y, $I
	vmsubi.y	$vf31y, $vf15y, $I
	vmsubi.y	$vf31y, $vf31y, $I
	vmsubi.yw	$vf0yw, $vf0yw, $I
	vmsubi.yw	$vf0yw, $vf31yw, $I
	vmsubi.yw	$vf1yw, $vf2yw, $I
	vmsubi.yw	$vf31yw, $vf0yw, $I
	vmsubi.yw	$vf31yw, $vf15yw, $I
	vmsubi.yw	$vf31yw, $vf31yw, $I
	vmsubi.yz	$vf0yz, $vf0yz, $I
	vmsubi.yz	$vf0yz, $vf31yz, $I
	vmsubi.yz	$vf1yz, $vf2yz, $I
	vmsubi.yz	$vf31yz, $vf0yz, $I
	vmsubi.yz	$vf31yz, $vf15yz, $I
	vmsubi.yz	$vf31yz, $vf31yz, $I
	vmsubi.yzw	$vf0yzw, $vf0yzw, $I
	vmsubi.yzw	$vf0yzw, $vf31yzw, $I
	vmsubi.yzw	$vf1yzw, $vf2yzw, $I
	vmsubi.yzw	$vf31yzw, $vf0yzw, $I
	vmsubi.yzw	$vf31yzw, $vf15yzw, $I
	vmsubi.yzw	$vf31yzw, $vf31yzw, $I
	vmsubi.z	$vf0z, $vf0z, $I
	vmsubi.z	$vf0z, $vf31z, $I
	vmsubi.z	$vf1z, $vf2z, $I
	vmsubi.z	$vf31z, $vf0z, $I
	vmsubi.z	$vf31z, $vf15z, $I
	vmsubi.z	$vf31z, $vf31z, $I
	vmsubi.zw	$vf0zw, $vf0zw, $I
	vmsubi.zw	$vf0zw, $vf31zw, $I
	vmsubi.zw	$vf1zw, $vf2zw, $I
	vmsubi.zw	$vf31zw, $vf0zw, $I
	vmsubi.zw	$vf31zw, $vf15zw, $I
	vmsubi.zw	$vf31zw, $vf31zw, $I
	vmsubq.w	$vf0w, $vf0w, $Q
	vmsubq.w	$vf0w, $vf31w, $Q
	vmsubq.w	$vf1w, $vf2w, $Q
	vmsubq.w	$vf31w, $vf0w, $Q
	vmsubq.w	$vf31w, $vf15w, $Q
	vmsubq.w	$vf31w, $vf31w, $Q
	vmsubq.x	$vf0x, $vf0x, $Q
	vmsubq.x	$vf0x, $vf31x, $Q
	vmsubq.x	$vf1x, $vf2x, $Q
	vmsubq.x	$vf31x, $vf0x, $Q
	vmsubq.x	$vf31x, $vf15x, $Q
	vmsubq.x	$vf31x, $vf31x, $Q
	vmsubq.xw	$vf0xw, $vf0xw, $Q
	vmsubq.xw	$vf0xw, $vf31xw, $Q
	vmsubq.xw	$vf1xw, $vf2xw, $Q
	vmsubq.xw	$vf31xw, $vf0xw, $Q
	vmsubq.xw	$vf31xw, $vf15xw, $Q
	vmsubq.xw	$vf31xw, $vf31xw, $Q
	vmsubq.xy	$vf0xy, $vf0xy, $Q
	vmsubq.xy	$vf0xy, $vf31xy, $Q
	vmsubq.xy	$vf1xy, $vf2xy, $Q
	vmsubq.xy	$vf31xy, $vf0xy, $Q
	vmsubq.xy	$vf31xy, $vf15xy, $Q
	vmsubq.xy	$vf31xy, $vf31xy, $Q
	vmsubq.xyw	$vf0xyw, $vf0xyw, $Q
	vmsubq.xyw	$vf0xyw, $vf31xyw, $Q
	vmsubq.xyw	$vf1xyw, $vf2xyw, $Q
	vmsubq.xyw	$vf31xyw, $vf0xyw, $Q
	vmsubq.xyw	$vf31xyw, $vf15xyw, $Q
	vmsubq.xyw	$vf31xyw, $vf31xyw, $Q
	vmsubq.xyz	$vf0xyz, $vf0xyz, $Q
	vmsubq.xyz	$vf0xyz, $vf31xyz, $Q
	vmsubq.xyz	$vf1xyz, $vf2xyz, $Q
	vmsubq.xyz	$vf31xyz, $vf0xyz, $Q
	vmsubq.xyz	$vf31xyz, $vf15xyz, $Q
	vmsubq.xyz	$vf31xyz, $vf31xyz, $Q
	vmsubq.xyzw	$vf0xyzw, $vf0xyzw, $Q
	vmsubq.xyzw	$vf0xyzw, $vf31xyzw, $Q
	vmsubq.xyzw	$vf1xyzw, $vf2xyzw, $Q
	vmsubq.xyzw	$vf31xyzw, $vf0xyzw, $Q
	vmsubq.xyzw	$vf31xyzw, $vf15xyzw, $Q
	vmsubq.xyzw	$vf31xyzw, $vf31xyzw, $Q
	vmsubq.xz	$vf0xz, $vf0xz, $Q
	vmsubq.xz	$vf0xz, $vf31xz, $Q
	vmsubq.xz	$vf1xz, $vf2xz, $Q
	vmsubq.xz	$vf31xz, $vf0xz, $Q
	vmsubq.xz	$vf31xz, $vf15xz, $Q
	vmsubq.xz	$vf31xz, $vf31xz, $Q
	vmsubq.xzw	$vf0xzw, $vf0xzw, $Q
	vmsubq.xzw	$vf0xzw, $vf31xzw, $Q
	vmsubq.xzw	$vf1xzw, $vf2xzw, $Q
	vmsubq.xzw	$vf31xzw, $vf0xzw, $Q
	vmsubq.xzw	$vf31xzw, $vf15xzw, $Q
	vmsubq.xzw	$vf31xzw, $vf31xzw, $Q
	vmsubq.y	$vf0y, $vf0y, $Q
	vmsubq.y	$vf0y, $vf31y, $Q
	vmsubq.y	$vf1y, $vf2y, $Q
	vmsubq.y	$vf31y, $vf0y, $Q
	vmsubq.y	$vf31y, $vf15y, $Q
	vmsubq.y	$vf31y, $vf31y, $Q
	vmsubq.yw	$vf0yw, $vf0yw, $Q
	vmsubq.yw	$vf0yw, $vf31yw, $Q
	vmsubq.yw	$vf1yw, $vf2yw, $Q
	vmsubq.yw	$vf31yw, $vf0yw, $Q
	vmsubq.yw	$vf31yw, $vf15yw, $Q
	vmsubq.yw	$vf31yw, $vf31yw, $Q
	vmsubq.yz	$vf0yz, $vf0yz, $Q
	vmsubq.yz	$vf0yz, $vf31yz, $Q
	vmsubq.yz	$vf1yz, $vf2yz, $Q
	vmsubq.yz	$vf31yz, $vf0yz, $Q
	vmsubq.yz	$vf31yz, $vf15yz, $Q
	vmsubq.yz	$vf31yz, $vf31yz, $Q
	vmsubq.yzw	$vf0yzw, $vf0yzw, $Q
	vmsubq.yzw	$vf0yzw, $vf31yzw, $Q
	vmsubq.yzw	$vf1yzw, $vf2yzw, $Q
	vmsubq.yzw	$vf31yzw, $vf0yzw, $Q
	vmsubq.yzw	$vf31yzw, $vf15yzw, $Q
	vmsubq.yzw	$vf31yzw, $vf31yzw, $Q
	vmsubq.z	$vf0z, $vf0z, $Q
	vmsubq.z	$vf0z, $vf31z, $Q
	vmsubq.z	$vf1z, $vf2z, $Q
	vmsubq.z	$vf31z, $vf0z, $Q
	vmsubq.z	$vf31z, $vf15z, $Q
	vmsubq.z	$vf31z, $vf31z, $Q
	vmsubq.zw	$vf0zw, $vf0zw, $Q
	vmsubq.zw	$vf0zw, $vf31zw, $Q
	vmsubq.zw	$vf1zw, $vf2zw, $Q
	vmsubq.zw	$vf31zw, $vf0zw, $Q
	vmsubq.zw	$vf31zw, $vf15zw, $Q
	vmsubq.zw	$vf31zw, $vf31zw, $Q
	vmsub.w		$vf0w, $vf0w, $vf0w
	vmsub.w		$vf0w, $vf0w, $vf31w
	vmsub.w		$vf0w, $vf31w, $vf0w
	vmsub.w		$vf1w, $vf2w, $vf3w
	vmsub.w		$vf31w, $vf0w, $vf0w
	vmsub.w		$vf31w, $vf15w, $vf7w
	vmsub.w		$vf31w, $vf31w, $vf31w
	vmsubw.w	$vf0w, $vf0w, $vf0w
	vmsubw.w	$vf0w, $vf0w, $vf31w
	vmsubw.w	$vf0w, $vf31w, $vf0w
	vmsubw.w	$vf1w, $vf2w, $vf3w
	vmsubw.w	$vf31w, $vf0w, $vf0w
	vmsubw.w	$vf31w, $vf15w, $vf7w
	vmsubw.w	$vf31w, $vf31w, $vf31w
	vmsubw.x	$vf0x, $vf0x, $vf0w
	vmsubw.x	$vf0x, $vf0x, $vf31w
	vmsubw.x	$vf0x, $vf31x, $vf0w
	vmsubw.x	$vf1x, $vf2x, $vf3w
	vmsubw.x	$vf31x, $vf0x, $vf0w
	vmsubw.x	$vf31x, $vf15x, $vf7w
	vmsubw.x	$vf31x, $vf31x, $vf31w
	vmsubw.xw	$vf0xw, $vf0xw, $vf0w
	vmsubw.xw	$vf0xw, $vf0xw, $vf31w
	vmsubw.xw	$vf0xw, $vf31xw, $vf0w
	vmsubw.xw	$vf1xw, $vf2xw, $vf3w
	vmsubw.xw	$vf31xw, $vf0xw, $vf0w
	vmsubw.xw	$vf31xw, $vf15xw, $vf7w
	vmsubw.xw	$vf31xw, $vf31xw, $vf31w
	vmsubw.xy	$vf0xy, $vf0xy, $vf0w
	vmsubw.xy	$vf0xy, $vf0xy, $vf31w
	vmsubw.xy	$vf0xy, $vf31xy, $vf0w
	vmsubw.xy	$vf1xy, $vf2xy, $vf3w
	vmsubw.xy	$vf31xy, $vf0xy, $vf0w
	vmsubw.xy	$vf31xy, $vf15xy, $vf7w
	vmsubw.xy	$vf31xy, $vf31xy, $vf31w
	vmsubw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vmsubw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vmsubw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vmsubw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vmsubw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vmsubw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vmsubw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vmsubw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vmsubw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vmsubw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vmsubw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vmsubw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vmsubw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vmsubw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vmsubw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vmsubw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vmsubw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vmsubw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vmsubw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vmsubw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vmsubw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vmsubw.xz	$vf0xz, $vf0xz, $vf0w
	vmsubw.xz	$vf0xz, $vf0xz, $vf31w
	vmsubw.xz	$vf0xz, $vf31xz, $vf0w
	vmsubw.xz	$vf1xz, $vf2xz, $vf3w
	vmsubw.xz	$vf31xz, $vf0xz, $vf0w
	vmsubw.xz	$vf31xz, $vf15xz, $vf7w
	vmsubw.xz	$vf31xz, $vf31xz, $vf31w
	vmsubw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vmsubw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vmsubw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vmsubw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vmsubw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vmsubw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vmsubw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vmsubw.y	$vf0y, $vf0y, $vf0w
	vmsubw.y	$vf0y, $vf0y, $vf31w
	vmsubw.y	$vf0y, $vf31y, $vf0w
	vmsubw.y	$vf1y, $vf2y, $vf3w
	vmsubw.y	$vf31y, $vf0y, $vf0w
	vmsubw.y	$vf31y, $vf15y, $vf7w
	vmsubw.y	$vf31y, $vf31y, $vf31w
	vmsubw.yw	$vf0yw, $vf0yw, $vf0w
	vmsubw.yw	$vf0yw, $vf0yw, $vf31w
	vmsubw.yw	$vf0yw, $vf31yw, $vf0w
	vmsubw.yw	$vf1yw, $vf2yw, $vf3w
	vmsubw.yw	$vf31yw, $vf0yw, $vf0w
	vmsubw.yw	$vf31yw, $vf15yw, $vf7w
	vmsubw.yw	$vf31yw, $vf31yw, $vf31w
	vmsubw.yz	$vf0yz, $vf0yz, $vf0w
	vmsubw.yz	$vf0yz, $vf0yz, $vf31w
	vmsubw.yz	$vf0yz, $vf31yz, $vf0w
	vmsubw.yz	$vf1yz, $vf2yz, $vf3w
	vmsubw.yz	$vf31yz, $vf0yz, $vf0w
	vmsubw.yz	$vf31yz, $vf15yz, $vf7w
	vmsubw.yz	$vf31yz, $vf31yz, $vf31w
	vmsubw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vmsubw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vmsubw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vmsubw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vmsubw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vmsubw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vmsubw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vmsubw.z	$vf0z, $vf0z, $vf0w
	vmsubw.z	$vf0z, $vf0z, $vf31w
	vmsubw.z	$vf0z, $vf31z, $vf0w
	vmsubw.z	$vf1z, $vf2z, $vf3w
	vmsubw.z	$vf31z, $vf0z, $vf0w
	vmsubw.z	$vf31z, $vf15z, $vf7w
	vmsubw.z	$vf31z, $vf31z, $vf31w
	vmsubw.zw	$vf0zw, $vf0zw, $vf0w
	vmsubw.zw	$vf0zw, $vf0zw, $vf31w
	vmsubw.zw	$vf0zw, $vf31zw, $vf0w
	vmsubw.zw	$vf1zw, $vf2zw, $vf3w
	vmsubw.zw	$vf31zw, $vf0zw, $vf0w
	vmsubw.zw	$vf31zw, $vf15zw, $vf7w
	vmsubw.zw	$vf31zw, $vf31zw, $vf31w
	vmsub.x		$vf0x, $vf0x, $vf0x
	vmsub.x		$vf0x, $vf0x, $vf31x
	vmsub.x		$vf0x, $vf31x, $vf0x
	vmsub.x		$vf1x, $vf2x, $vf3x
	vmsub.x		$vf31x, $vf0x, $vf0x
	vmsub.x		$vf31x, $vf15x, $vf7x
	vmsub.x		$vf31x, $vf31x, $vf31x
	vmsubx.w	$vf0w, $vf0w, $vf0x
	vmsubx.w	$vf0w, $vf0w, $vf31x
	vmsubx.w	$vf0w, $vf31w, $vf0x
	vmsub.xw	$vf0xw, $vf0xw, $vf0xw
	vmsub.xw	$vf0xw, $vf0xw, $vf31xw
	vmsub.xw	$vf0xw, $vf31xw, $vf0xw
	vmsubx.w	$vf1w, $vf2w, $vf3x
	vmsub.xw	$vf1xw, $vf2xw, $vf3xw
	vmsubx.w	$vf31w, $vf0w, $vf0x
	vmsubx.w	$vf31w, $vf15w, $vf7x
	vmsubx.w	$vf31w, $vf31w, $vf31x
	vmsub.xw	$vf31xw, $vf0xw, $vf0xw
	vmsub.xw	$vf31xw, $vf15xw, $vf7xw
	vmsub.xw	$vf31xw, $vf31xw, $vf31xw
	vmsubx.x	$vf0x, $vf0x, $vf0x
	vmsubx.x	$vf0x, $vf0x, $vf31x
	vmsubx.x	$vf0x, $vf31x, $vf0x
	vmsubx.x	$vf1x, $vf2x, $vf3x
	vmsubx.x	$vf31x, $vf0x, $vf0x
	vmsubx.x	$vf31x, $vf15x, $vf7x
	vmsubx.x	$vf31x, $vf31x, $vf31x
	vmsubx.xw	$vf0xw, $vf0xw, $vf0x
	vmsubx.xw	$vf0xw, $vf0xw, $vf31x
	vmsubx.xw	$vf0xw, $vf31xw, $vf0x
	vmsubx.xw	$vf1xw, $vf2xw, $vf3x
	vmsubx.xw	$vf31xw, $vf0xw, $vf0x
	vmsubx.xw	$vf31xw, $vf15xw, $vf7x
	vmsubx.xw	$vf31xw, $vf31xw, $vf31x
	vmsubx.xy	$vf0xy, $vf0xy, $vf0x
	vmsubx.xy	$vf0xy, $vf0xy, $vf31x
	vmsubx.xy	$vf0xy, $vf31xy, $vf0x
	vmsubx.xy	$vf1xy, $vf2xy, $vf3x
	vmsubx.xy	$vf31xy, $vf0xy, $vf0x
	vmsubx.xy	$vf31xy, $vf15xy, $vf7x
	vmsubx.xy	$vf31xy, $vf31xy, $vf31x
	vmsubx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vmsubx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vmsubx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vmsubx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vmsubx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vmsubx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vmsubx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vmsubx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vmsubx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vmsubx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vmsubx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vmsubx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vmsubx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vmsubx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vmsubx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vmsubx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vmsubx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vmsubx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vmsubx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vmsubx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vmsubx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vmsubx.xz	$vf0xz, $vf0xz, $vf0x
	vmsubx.xz	$vf0xz, $vf0xz, $vf31x
	vmsubx.xz	$vf0xz, $vf31xz, $vf0x
	vmsubx.xz	$vf1xz, $vf2xz, $vf3x
	vmsubx.xz	$vf31xz, $vf0xz, $vf0x
	vmsubx.xz	$vf31xz, $vf15xz, $vf7x
	vmsubx.xz	$vf31xz, $vf31xz, $vf31x
	vmsubx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vmsubx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vmsubx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vmsubx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vmsubx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vmsubx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vmsubx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vmsub.xy	$vf0xy, $vf0xy, $vf0xy
	vmsub.xy	$vf0xy, $vf0xy, $vf31xy
	vmsub.xy	$vf0xy, $vf31xy, $vf0xy
	vmsubx.y	$vf0y, $vf0y, $vf0x
	vmsubx.y	$vf0y, $vf0y, $vf31x
	vmsubx.y	$vf0y, $vf31y, $vf0x
	vmsub.xy	$vf1xy, $vf2xy, $vf3xy
	vmsubx.y	$vf1y, $vf2y, $vf3x
	vmsub.xy	$vf31xy, $vf0xy, $vf0xy
	vmsub.xy	$vf31xy, $vf15xy, $vf7xy
	vmsub.xy	$vf31xy, $vf31xy, $vf31xy
	vmsubx.y	$vf31y, $vf0y, $vf0x
	vmsubx.y	$vf31y, $vf15y, $vf7x
	vmsubx.y	$vf31y, $vf31y, $vf31x
	vmsub.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmsub.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vmsub.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vmsubx.yw	$vf0yw, $vf0yw, $vf0x
	vmsubx.yw	$vf0yw, $vf0yw, $vf31x
	vmsubx.yw	$vf0yw, $vf31yw, $vf0x
	vmsub.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vmsubx.yw	$vf1yw, $vf2yw, $vf3x
	vmsub.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vmsub.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vmsub.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vmsubx.yw	$vf31yw, $vf0yw, $vf0x
	vmsubx.yw	$vf31yw, $vf15yw, $vf7x
	vmsubx.yw	$vf31yw, $vf31yw, $vf31x
	vmsub.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmsub.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vmsub.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vmsubx.yz	$vf0yz, $vf0yz, $vf0x
	vmsubx.yz	$vf0yz, $vf0yz, $vf31x
	vmsubx.yz	$vf0yz, $vf31yz, $vf0x
	vmsub.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vmsubx.yz	$vf1yz, $vf2yz, $vf3x
	vmsub.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vmsub.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vmsub.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vmsubx.yz	$vf31yz, $vf0yz, $vf0x
	vmsubx.yz	$vf31yz, $vf15yz, $vf7x
	vmsubx.yz	$vf31yz, $vf31yz, $vf31x
	vmsub.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmsub.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vmsub.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmsubx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vmsubx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vmsubx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vmsub.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vmsubx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vmsub.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmsub.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vmsub.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vmsubx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vmsubx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vmsubx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vmsub.xz	$vf0xz, $vf0xz, $vf0xz
	vmsub.xz	$vf0xz, $vf0xz, $vf31xz
	vmsub.xz	$vf0xz, $vf31xz, $vf0xz
	vmsubx.z	$vf0z, $vf0z, $vf0x
	vmsubx.z	$vf0z, $vf0z, $vf31x
	vmsubx.z	$vf0z, $vf31z, $vf0x
	vmsub.xz	$vf1xz, $vf2xz, $vf3xz
	vmsubx.z	$vf1z, $vf2z, $vf3x
	vmsub.xz	$vf31xz, $vf0xz, $vf0xz
	vmsub.xz	$vf31xz, $vf15xz, $vf7xz
	vmsub.xz	$vf31xz, $vf31xz, $vf31xz
	vmsubx.z	$vf31z, $vf0z, $vf0x
	vmsubx.z	$vf31z, $vf15z, $vf7x
	vmsubx.z	$vf31z, $vf31z, $vf31x
	vmsub.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vmsub.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmsub.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vmsubx.zw	$vf0zw, $vf0zw, $vf0x
	vmsubx.zw	$vf0zw, $vf0zw, $vf31x
	vmsubx.zw	$vf0zw, $vf31zw, $vf0x
	vmsub.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vmsubx.zw	$vf1zw, $vf2zw, $vf3x
	vmsub.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vmsub.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmsub.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vmsubx.zw	$vf31zw, $vf0zw, $vf0x
	vmsubx.zw	$vf31zw, $vf15zw, $vf7x
	vmsubx.zw	$vf31zw, $vf31zw, $vf31x
	vmsub.y		$vf0y, $vf0y, $vf0y
	vmsub.y		$vf0y, $vf0y, $vf31y
	vmsub.y		$vf0y, $vf31y, $vf0y
	vmsub.y		$vf1y, $vf2y, $vf3y
	vmsub.y		$vf31y, $vf0y, $vf0y
	vmsub.y		$vf31y, $vf15y, $vf7y
	vmsub.y		$vf31y, $vf31y, $vf31y
	vmsuby.w	$vf0w, $vf0w, $vf0y
	vmsuby.w	$vf0w, $vf0w, $vf31y
	vmsuby.w	$vf0w, $vf31w, $vf0y
	vmsub.yw	$vf0yw, $vf0yw, $vf0yw
	vmsub.yw	$vf0yw, $vf0yw, $vf31yw
	vmsub.yw	$vf0yw, $vf31yw, $vf0yw
	vmsuby.w	$vf1w, $vf2w, $vf3y
	vmsub.yw	$vf1yw, $vf2yw, $vf3yw
	vmsuby.w	$vf31w, $vf0w, $vf0y
	vmsuby.w	$vf31w, $vf15w, $vf7y
	vmsuby.w	$vf31w, $vf31w, $vf31y
	vmsub.yw	$vf31yw, $vf0yw, $vf0yw
	vmsub.yw	$vf31yw, $vf15yw, $vf7yw
	vmsub.yw	$vf31yw, $vf31yw, $vf31yw
	vmsuby.x	$vf0x, $vf0x, $vf0y
	vmsuby.x	$vf0x, $vf0x, $vf31y
	vmsuby.x	$vf0x, $vf31x, $vf0y
	vmsuby.x	$vf1x, $vf2x, $vf3y
	vmsuby.x	$vf31x, $vf0x, $vf0y
	vmsuby.x	$vf31x, $vf15x, $vf7y
	vmsuby.x	$vf31x, $vf31x, $vf31y
	vmsuby.xw	$vf0xw, $vf0xw, $vf0y
	vmsuby.xw	$vf0xw, $vf0xw, $vf31y
	vmsuby.xw	$vf0xw, $vf31xw, $vf0y
	vmsuby.xw	$vf1xw, $vf2xw, $vf3y
	vmsuby.xw	$vf31xw, $vf0xw, $vf0y
	vmsuby.xw	$vf31xw, $vf15xw, $vf7y
	vmsuby.xw	$vf31xw, $vf31xw, $vf31y
	vmsuby.xy	$vf0xy, $vf0xy, $vf0y
	vmsuby.xy	$vf0xy, $vf0xy, $vf31y
	vmsuby.xy	$vf0xy, $vf31xy, $vf0y
	vmsuby.xy	$vf1xy, $vf2xy, $vf3y
	vmsuby.xy	$vf31xy, $vf0xy, $vf0y
	vmsuby.xy	$vf31xy, $vf15xy, $vf7y
	vmsuby.xy	$vf31xy, $vf31xy, $vf31y
	vmsuby.xyw	$vf0xyw, $vf0xyw, $vf0y
	vmsuby.xyw	$vf0xyw, $vf0xyw, $vf31y
	vmsuby.xyw	$vf0xyw, $vf31xyw, $vf0y
	vmsuby.xyw	$vf1xyw, $vf2xyw, $vf3y
	vmsuby.xyw	$vf31xyw, $vf0xyw, $vf0y
	vmsuby.xyw	$vf31xyw, $vf15xyw, $vf7y
	vmsuby.xyw	$vf31xyw, $vf31xyw, $vf31y
	vmsuby.xyz	$vf0xyz, $vf0xyz, $vf0y
	vmsuby.xyz	$vf0xyz, $vf0xyz, $vf31y
	vmsuby.xyz	$vf0xyz, $vf31xyz, $vf0y
	vmsuby.xyz	$vf1xyz, $vf2xyz, $vf3y
	vmsuby.xyz	$vf31xyz, $vf0xyz, $vf0y
	vmsuby.xyz	$vf31xyz, $vf15xyz, $vf7y
	vmsuby.xyz	$vf31xyz, $vf31xyz, $vf31y
	vmsuby.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vmsuby.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vmsuby.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vmsuby.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vmsuby.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vmsuby.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vmsuby.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vmsuby.xz	$vf0xz, $vf0xz, $vf0y
	vmsuby.xz	$vf0xz, $vf0xz, $vf31y
	vmsuby.xz	$vf0xz, $vf31xz, $vf0y
	vmsuby.xz	$vf1xz, $vf2xz, $vf3y
	vmsuby.xz	$vf31xz, $vf0xz, $vf0y
	vmsuby.xz	$vf31xz, $vf15xz, $vf7y
	vmsuby.xz	$vf31xz, $vf31xz, $vf31y
	vmsuby.xzw	$vf0xzw, $vf0xzw, $vf0y
	vmsuby.xzw	$vf0xzw, $vf0xzw, $vf31y
	vmsuby.xzw	$vf0xzw, $vf31xzw, $vf0y
	vmsuby.xzw	$vf1xzw, $vf2xzw, $vf3y
	vmsuby.xzw	$vf31xzw, $vf0xzw, $vf0y
	vmsuby.xzw	$vf31xzw, $vf15xzw, $vf7y
	vmsuby.xzw	$vf31xzw, $vf31xzw, $vf31y
	vmsuby.y	$vf0y, $vf0y, $vf0y
	vmsuby.y	$vf0y, $vf0y, $vf31y
	vmsuby.y	$vf0y, $vf31y, $vf0y
	vmsuby.y	$vf1y, $vf2y, $vf3y
	vmsuby.y	$vf31y, $vf0y, $vf0y
	vmsuby.y	$vf31y, $vf15y, $vf7y
	vmsuby.y	$vf31y, $vf31y, $vf31y
	vmsuby.yw	$vf0yw, $vf0yw, $vf0y
	vmsuby.yw	$vf0yw, $vf0yw, $vf31y
	vmsuby.yw	$vf0yw, $vf31yw, $vf0y
	vmsuby.yw	$vf1yw, $vf2yw, $vf3y
	vmsuby.yw	$vf31yw, $vf0yw, $vf0y
	vmsuby.yw	$vf31yw, $vf15yw, $vf7y
	vmsuby.yw	$vf31yw, $vf31yw, $vf31y
	vmsuby.yz	$vf0yz, $vf0yz, $vf0y
	vmsuby.yz	$vf0yz, $vf0yz, $vf31y
	vmsuby.yz	$vf0yz, $vf31yz, $vf0y
	vmsuby.yz	$vf1yz, $vf2yz, $vf3y
	vmsuby.yz	$vf31yz, $vf0yz, $vf0y
	vmsuby.yz	$vf31yz, $vf15yz, $vf7y
	vmsuby.yz	$vf31yz, $vf31yz, $vf31y
	vmsuby.yzw	$vf0yzw, $vf0yzw, $vf0y
	vmsuby.yzw	$vf0yzw, $vf0yzw, $vf31y
	vmsuby.yzw	$vf0yzw, $vf31yzw, $vf0y
	vmsuby.yzw	$vf1yzw, $vf2yzw, $vf3y
	vmsuby.yzw	$vf31yzw, $vf0yzw, $vf0y
	vmsuby.yzw	$vf31yzw, $vf15yzw, $vf7y
	vmsuby.yzw	$vf31yzw, $vf31yzw, $vf31y
	vmsub.yz	$vf0yz, $vf0yz, $vf0yz
	vmsub.yz	$vf0yz, $vf0yz, $vf31yz
	vmsub.yz	$vf0yz, $vf31yz, $vf0yz
	vmsuby.z	$vf0z, $vf0z, $vf0y
	vmsuby.z	$vf0z, $vf0z, $vf31y
	vmsuby.z	$vf0z, $vf31z, $vf0y
	vmsub.yz	$vf1yz, $vf2yz, $vf3yz
	vmsuby.z	$vf1z, $vf2z, $vf3y
	vmsub.yz	$vf31yz, $vf0yz, $vf0yz
	vmsub.yz	$vf31yz, $vf15yz, $vf7yz
	vmsub.yz	$vf31yz, $vf31yz, $vf31yz
	vmsuby.z	$vf31z, $vf0z, $vf0y
	vmsuby.z	$vf31z, $vf15z, $vf7y
	vmsuby.z	$vf31z, $vf31z, $vf31y
	vmsub.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmsub.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmsub.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vmsuby.zw	$vf0zw, $vf0zw, $vf0y
	vmsuby.zw	$vf0zw, $vf0zw, $vf31y
	vmsuby.zw	$vf0zw, $vf31zw, $vf0y
	vmsub.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vmsuby.zw	$vf1zw, $vf2zw, $vf3y
	vmsub.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vmsub.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vmsub.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vmsuby.zw	$vf31zw, $vf0zw, $vf0y
	vmsuby.zw	$vf31zw, $vf15zw, $vf7y
	vmsuby.zw	$vf31zw, $vf31zw, $vf31y
	vmsub.z		$vf0z, $vf0z, $vf0z
	vmsub.z		$vf0z, $vf0z, $vf31z
	vmsub.z		$vf0z, $vf31z, $vf0z
	vmsub.z		$vf1z, $vf2z, $vf3z
	vmsub.z		$vf31z, $vf0z, $vf0z
	vmsub.z		$vf31z, $vf15z, $vf7z
	vmsub.z		$vf31z, $vf31z, $vf31z
	vmsubz.w	$vf0w, $vf0w, $vf0z
	vmsubz.w	$vf0w, $vf0w, $vf31z
	vmsubz.w	$vf0w, $vf31w, $vf0z
	vmsub.zw	$vf0zw, $vf0zw, $vf0zw
	vmsub.zw	$vf0zw, $vf0zw, $vf31zw
	vmsub.zw	$vf0zw, $vf31zw, $vf0zw
	vmsubz.w	$vf1w, $vf2w, $vf3z
	vmsub.zw	$vf1zw, $vf2zw, $vf3zw
	vmsubz.w	$vf31w, $vf0w, $vf0z
	vmsubz.w	$vf31w, $vf15w, $vf7z
	vmsubz.w	$vf31w, $vf31w, $vf31z
	vmsub.zw	$vf31zw, $vf0zw, $vf0zw
	vmsub.zw	$vf31zw, $vf15zw, $vf7zw
	vmsub.zw	$vf31zw, $vf31zw, $vf31zw
	vmsubz.x	$vf0x, $vf0x, $vf0z
	vmsubz.x	$vf0x, $vf0x, $vf31z
	vmsubz.x	$vf0x, $vf31x, $vf0z
	vmsubz.x	$vf1x, $vf2x, $vf3z
	vmsubz.x	$vf31x, $vf0x, $vf0z
	vmsubz.x	$vf31x, $vf15x, $vf7z
	vmsubz.x	$vf31x, $vf31x, $vf31z
	vmsubz.xw	$vf0xw, $vf0xw, $vf0z
	vmsubz.xw	$vf0xw, $vf0xw, $vf31z
	vmsubz.xw	$vf0xw, $vf31xw, $vf0z
	vmsubz.xw	$vf1xw, $vf2xw, $vf3z
	vmsubz.xw	$vf31xw, $vf0xw, $vf0z
	vmsubz.xw	$vf31xw, $vf15xw, $vf7z
	vmsubz.xw	$vf31xw, $vf31xw, $vf31z
	vmsubz.xy	$vf0xy, $vf0xy, $vf0z
	vmsubz.xy	$vf0xy, $vf0xy, $vf31z
	vmsubz.xy	$vf0xy, $vf31xy, $vf0z
	vmsubz.xy	$vf1xy, $vf2xy, $vf3z
	vmsubz.xy	$vf31xy, $vf0xy, $vf0z
	vmsubz.xy	$vf31xy, $vf15xy, $vf7z
	vmsubz.xy	$vf31xy, $vf31xy, $vf31z
	vmsubz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vmsubz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vmsubz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vmsubz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vmsubz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vmsubz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vmsubz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vmsubz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vmsubz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vmsubz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vmsubz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vmsubz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vmsubz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vmsubz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vmsubz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vmsubz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vmsubz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vmsubz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vmsubz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vmsubz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vmsubz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vmsubz.xz	$vf0xz, $vf0xz, $vf0z
	vmsubz.xz	$vf0xz, $vf0xz, $vf31z
	vmsubz.xz	$vf0xz, $vf31xz, $vf0z
	vmsubz.xz	$vf1xz, $vf2xz, $vf3z
	vmsubz.xz	$vf31xz, $vf0xz, $vf0z
	vmsubz.xz	$vf31xz, $vf15xz, $vf7z
	vmsubz.xz	$vf31xz, $vf31xz, $vf31z
	vmsubz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vmsubz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vmsubz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vmsubz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vmsubz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vmsubz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vmsubz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vmsubz.y	$vf0y, $vf0y, $vf0z
	vmsubz.y	$vf0y, $vf0y, $vf31z
	vmsubz.y	$vf0y, $vf31y, $vf0z
	vmsubz.y	$vf1y, $vf2y, $vf3z
	vmsubz.y	$vf31y, $vf0y, $vf0z
	vmsubz.y	$vf31y, $vf15y, $vf7z
	vmsubz.y	$vf31y, $vf31y, $vf31z
	vmsubz.yw	$vf0yw, $vf0yw, $vf0z
	vmsubz.yw	$vf0yw, $vf0yw, $vf31z
	vmsubz.yw	$vf0yw, $vf31yw, $vf0z
	vmsubz.yw	$vf1yw, $vf2yw, $vf3z
	vmsubz.yw	$vf31yw, $vf0yw, $vf0z
	vmsubz.yw	$vf31yw, $vf15yw, $vf7z
	vmsubz.yw	$vf31yw, $vf31yw, $vf31z
	vmsubz.yz	$vf0yz, $vf0yz, $vf0z
	vmsubz.yz	$vf0yz, $vf0yz, $vf31z
	vmsubz.yz	$vf0yz, $vf31yz, $vf0z
	vmsubz.yz	$vf1yz, $vf2yz, $vf3z
	vmsubz.yz	$vf31yz, $vf0yz, $vf0z
	vmsubz.yz	$vf31yz, $vf15yz, $vf7z
	vmsubz.yz	$vf31yz, $vf31yz, $vf31z
	vmsubz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vmsubz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vmsubz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vmsubz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vmsubz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vmsubz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vmsubz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vmsubz.z	$vf0z, $vf0z, $vf0z
	vmsubz.z	$vf0z, $vf0z, $vf31z
	vmsubz.z	$vf0z, $vf31z, $vf0z
	vmsubz.z	$vf1z, $vf2z, $vf3z
	vmsubz.z	$vf31z, $vf0z, $vf0z
	vmsubz.z	$vf31z, $vf15z, $vf7z
	vmsubz.z	$vf31z, $vf31z, $vf31z
	vmsubz.zw	$vf0zw, $vf0zw, $vf0z
	vmsubz.zw	$vf0zw, $vf0zw, $vf31z
	vmsubz.zw	$vf0zw, $vf31zw, $vf0z
	vmsubz.zw	$vf1zw, $vf2zw, $vf3z
	vmsubz.zw	$vf31zw, $vf0zw, $vf0z
	vmsubz.zw	$vf31zw, $vf15zw, $vf7z
	vmsubz.zw	$vf31zw, $vf31zw, $vf31z
	vmtir		$vi0, $vf0w
	vmtir		$vi0, $vf0x
	vmtir		$vi0, $vf31z
	vmtir		$vi1, $vf2z
	vmtir		$vi31, $vf0y
	vmtir		$vi31, $vf15x
	vmtir		$vi31, $vf31x
	vmulai.w	$ACCw, $vf0w, $I
	vmulai.w	$ACCw, $vf1w, $I
	vmulai.w	$ACCw, $vf31w, $I
	vmulai.x	$ACCx, $vf0x, $I
	vmulai.x	$ACCx, $vf1x, $I
	vmulai.x	$ACCx, $vf31x, $I
	vmulai.xw	$ACCxw, $vf0xw, $I
	vmulai.xw	$ACCxw, $vf1xw, $I
	vmulai.xw	$ACCxw, $vf31xw, $I
	vmulai.xy	$ACCxy, $vf0xy, $I
	vmulai.xy	$ACCxy, $vf1xy, $I
	vmulai.xy	$ACCxy, $vf31xy, $I
	vmulai.xyw	$ACCxyw, $vf0xyw, $I
	vmulai.xyw	$ACCxyw, $vf1xyw, $I
	vmulai.xyw	$ACCxyw, $vf31xyw, $I
	vmulai.xyz	$ACCxyz, $vf0xyz, $I
	vmulai.xyz	$ACCxyz, $vf1xyz, $I
	vmulai.xyz	$ACCxyz, $vf31xyz, $I
	vmulai.xyzw	$ACCxyzw, $vf0xyzw, $I
	vmulai.xyzw	$ACCxyzw, $vf1xyzw, $I
	vmulai.xyzw	$ACCxyzw, $vf31xyzw, $I
	vmulai.xz	$ACCxz, $vf0xz, $I
	vmulai.xz	$ACCxz, $vf1xz, $I
	vmulai.xz	$ACCxz, $vf31xz, $I
	vmulai.xzw	$ACCxzw, $vf0xzw, $I
	vmulai.xzw	$ACCxzw, $vf1xzw, $I
	vmulai.xzw	$ACCxzw, $vf31xzw, $I
	vmulai.y	$ACCy, $vf0y, $I
	vmulai.y	$ACCy, $vf1y, $I
	vmulai.y	$ACCy, $vf31y, $I
	vmulai.yw	$ACCyw, $vf0yw, $I
	vmulai.yw	$ACCyw, $vf1yw, $I
	vmulai.yw	$ACCyw, $vf31yw, $I
	vmulai.yz	$ACCyz, $vf0yz, $I
	vmulai.yz	$ACCyz, $vf1yz, $I
	vmulai.yz	$ACCyz, $vf31yz, $I
	vmulai.yzw	$ACCyzw, $vf0yzw, $I
	vmulai.yzw	$ACCyzw, $vf1yzw, $I
	vmulai.yzw	$ACCyzw, $vf31yzw, $I
	vmulai.z	$ACCz, $vf0z, $I
	vmulai.z	$ACCz, $vf1z, $I
	vmulai.z	$ACCz, $vf31z, $I
	vmulai.zw	$ACCzw, $vf0zw, $I
	vmulai.zw	$ACCzw, $vf1zw, $I
	vmulai.zw	$ACCzw, $vf31zw, $I
	vmulaq.w	$ACCw, $vf0w, $Q
	vmulaq.w	$ACCw, $vf1w, $Q
	vmulaq.w	$ACCw, $vf31w, $Q
	vmulaq.x	$ACCx, $vf0x, $Q
	vmulaq.x	$ACCx, $vf1x, $Q
	vmulaq.x	$ACCx, $vf31x, $Q
	vmulaq.xw	$ACCxw, $vf0xw, $Q
	vmulaq.xw	$ACCxw, $vf1xw, $Q
	vmulaq.xw	$ACCxw, $vf31xw, $Q
	vmulaq.xy	$ACCxy, $vf0xy, $Q
	vmulaq.xy	$ACCxy, $vf1xy, $Q
	vmulaq.xy	$ACCxy, $vf31xy, $Q
	vmulaq.xyw	$ACCxyw, $vf0xyw, $Q
	vmulaq.xyw	$ACCxyw, $vf1xyw, $Q
	vmulaq.xyw	$ACCxyw, $vf31xyw, $Q
	vmulaq.xyz	$ACCxyz, $vf0xyz, $Q
	vmulaq.xyz	$ACCxyz, $vf1xyz, $Q
	vmulaq.xyz	$ACCxyz, $vf31xyz, $Q
	vmulaq.xyzw	$ACCxyzw, $vf0xyzw, $Q
	vmulaq.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vmulaq.xyzw	$ACCxyzw, $vf31xyzw, $Q
	vmulaq.xz	$ACCxz, $vf0xz, $Q
	vmulaq.xz	$ACCxz, $vf1xz, $Q
	vmulaq.xz	$ACCxz, $vf31xz, $Q
	vmulaq.xzw	$ACCxzw, $vf0xzw, $Q
	vmulaq.xzw	$ACCxzw, $vf1xzw, $Q
	vmulaq.xzw	$ACCxzw, $vf31xzw, $Q
	vmulaq.y	$ACCy, $vf0y, $Q
	vmulaq.y	$ACCy, $vf1y, $Q
	vmulaq.y	$ACCy, $vf31y, $Q
	vmulaq.yw	$ACCyw, $vf0yw, $Q
	vmulaq.yw	$ACCyw, $vf1yw, $Q
	vmulaq.yw	$ACCyw, $vf31yw, $Q
	vmulaq.yz	$ACCyz, $vf0yz, $Q
	vmulaq.yz	$ACCyz, $vf1yz, $Q
	vmulaq.yz	$ACCyz, $vf31yz, $Q
	vmulaq.yzw	$ACCyzw, $vf0yzw, $Q
	vmulaq.yzw	$ACCyzw, $vf1yzw, $Q
	vmulaq.yzw	$ACCyzw, $vf31yzw, $Q
	vmulaq.z	$ACCz, $vf0z, $Q
	vmulaq.z	$ACCz, $vf1z, $Q
	vmulaq.z	$ACCz, $vf31z, $Q
	vmulaq.zw	$ACCzw, $vf0zw, $Q
	vmulaq.zw	$ACCzw, $vf1zw, $Q
	vmulaq.zw	$ACCzw, $vf31zw, $Q
	vmula.w		$ACCw, $vf0w, $vf0w
	vmula.w		$ACCw, $vf0w, $vf31w
	vmula.w		$ACCw, $vf1w, $vf2w
	vmula.w		$ACCw, $vf31w, $vf0w
	vmula.w		$ACCw, $vf31w, $vf15w
	vmula.w		$ACCw, $vf31w, $vf31w
	vmulaw.w	$ACCw, $vf0w, $vf0w
	vmulaw.w	$ACCw, $vf0w, $vf31w
	vmulaw.w	$ACCw, $vf1w, $vf2w
	vmulaw.w	$ACCw, $vf31w, $vf0w
	vmulaw.w	$ACCw, $vf31w, $vf15w
	vmulaw.w	$ACCw, $vf31w, $vf31w
	vmulaw.x	$ACCx, $vf0x, $vf0w
	vmulaw.x	$ACCx, $vf0x, $vf31w
	vmulaw.x	$ACCx, $vf1x, $vf2w
	vmulaw.x	$ACCx, $vf31x, $vf0w
	vmulaw.x	$ACCx, $vf31x, $vf15w
	vmulaw.x	$ACCx, $vf31x, $vf31w
	vmulaw.xw	$ACCxw, $vf0xw, $vf0w
	vmulaw.xw	$ACCxw, $vf0xw, $vf31w
	vmulaw.xw	$ACCxw, $vf1xw, $vf2w
	vmulaw.xw	$ACCxw, $vf31xw, $vf0w
	vmulaw.xw	$ACCxw, $vf31xw, $vf15w
	vmulaw.xw	$ACCxw, $vf31xw, $vf31w
	vmulaw.xy	$ACCxy, $vf0xy, $vf0w
	vmulaw.xy	$ACCxy, $vf0xy, $vf31w
	vmulaw.xy	$ACCxy, $vf1xy, $vf2w
	vmulaw.xy	$ACCxy, $vf31xy, $vf0w
	vmulaw.xy	$ACCxy, $vf31xy, $vf15w
	vmulaw.xy	$ACCxy, $vf31xy, $vf31w
	vmulaw.xyw	$ACCxyw, $vf0xyw, $vf0w
	vmulaw.xyw	$ACCxyw, $vf0xyw, $vf31w
	vmulaw.xyw	$ACCxyw, $vf1xyw, $vf2w
	vmulaw.xyw	$ACCxyw, $vf31xyw, $vf0w
	vmulaw.xyw	$ACCxyw, $vf31xyw, $vf15w
	vmulaw.xyw	$ACCxyw, $vf31xyw, $vf31w
	vmulaw.xyz	$ACCxyz, $vf0xyz, $vf0w
	vmulaw.xyz	$ACCxyz, $vf0xyz, $vf31w
	vmulaw.xyz	$ACCxyz, $vf1xyz, $vf2w
	vmulaw.xyz	$ACCxyz, $vf31xyz, $vf0w
	vmulaw.xyz	$ACCxyz, $vf31xyz, $vf15w
	vmulaw.xyz	$ACCxyz, $vf31xyz, $vf31w
	vmulaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0w
	vmulaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31w
	vmulaw.xyzw	$ACCxyzw, $vf1xyzw, $vf2w
	vmulaw.xyzw	$ACCxyzw, $vf31xyzw, $vf0w
	vmulaw.xyzw	$ACCxyzw, $vf31xyzw, $vf15w
	vmulaw.xyzw	$ACCxyzw, $vf31xyzw, $vf31w
	vmulaw.xz	$ACCxz, $vf0xz, $vf0w
	vmulaw.xz	$ACCxz, $vf0xz, $vf31w
	vmulaw.xz	$ACCxz, $vf1xz, $vf2w
	vmulaw.xz	$ACCxz, $vf31xz, $vf0w
	vmulaw.xz	$ACCxz, $vf31xz, $vf15w
	vmulaw.xz	$ACCxz, $vf31xz, $vf31w
	vmulaw.xzw	$ACCxzw, $vf0xzw, $vf0w
	vmulaw.xzw	$ACCxzw, $vf0xzw, $vf31w
	vmulaw.xzw	$ACCxzw, $vf1xzw, $vf2w
	vmulaw.xzw	$ACCxzw, $vf31xzw, $vf0w
	vmulaw.xzw	$ACCxzw, $vf31xzw, $vf15w
	vmulaw.xzw	$ACCxzw, $vf31xzw, $vf31w
	vmulaw.y	$ACCy, $vf0y, $vf0w
	vmulaw.y	$ACCy, $vf0y, $vf31w
	vmulaw.y	$ACCy, $vf1y, $vf2w
	vmulaw.y	$ACCy, $vf31y, $vf0w
	vmulaw.y	$ACCy, $vf31y, $vf15w
	vmulaw.y	$ACCy, $vf31y, $vf31w
	vmulaw.yw	$ACCyw, $vf0yw, $vf0w
	vmulaw.yw	$ACCyw, $vf0yw, $vf31w
	vmulaw.yw	$ACCyw, $vf1yw, $vf2w
	vmulaw.yw	$ACCyw, $vf31yw, $vf0w
	vmulaw.yw	$ACCyw, $vf31yw, $vf15w
	vmulaw.yw	$ACCyw, $vf31yw, $vf31w
	vmulaw.yz	$ACCyz, $vf0yz, $vf0w
	vmulaw.yz	$ACCyz, $vf0yz, $vf31w
	vmulaw.yz	$ACCyz, $vf1yz, $vf2w
	vmulaw.yz	$ACCyz, $vf31yz, $vf0w
	vmulaw.yz	$ACCyz, $vf31yz, $vf15w
	vmulaw.yz	$ACCyz, $vf31yz, $vf31w
	vmulaw.yzw	$ACCyzw, $vf0yzw, $vf0w
	vmulaw.yzw	$ACCyzw, $vf0yzw, $vf31w
	vmulaw.yzw	$ACCyzw, $vf1yzw, $vf2w
	vmulaw.yzw	$ACCyzw, $vf31yzw, $vf0w
	vmulaw.yzw	$ACCyzw, $vf31yzw, $vf15w
	vmulaw.yzw	$ACCyzw, $vf31yzw, $vf31w
	vmulaw.z	$ACCz, $vf0z, $vf0w
	vmulaw.z	$ACCz, $vf0z, $vf31w
	vmulaw.z	$ACCz, $vf1z, $vf2w
	vmulaw.z	$ACCz, $vf31z, $vf0w
	vmulaw.z	$ACCz, $vf31z, $vf15w
	vmulaw.z	$ACCz, $vf31z, $vf31w
	vmulaw.zw	$ACCzw, $vf0zw, $vf0w
	vmulaw.zw	$ACCzw, $vf0zw, $vf31w
	vmulaw.zw	$ACCzw, $vf1zw, $vf2w
	vmulaw.zw	$ACCzw, $vf31zw, $vf0w
	vmulaw.zw	$ACCzw, $vf31zw, $vf15w
	vmulaw.zw	$ACCzw, $vf31zw, $vf31w
	vmula.x		$ACCx, $vf0x, $vf0x
	vmula.x		$ACCx, $vf0x, $vf31x
	vmula.x		$ACCx, $vf1x, $vf2x
	vmula.x		$ACCx, $vf31x, $vf0x
	vmula.x		$ACCx, $vf31x, $vf15x
	vmula.x		$ACCx, $vf31x, $vf31x
	vmulax.w	$ACCw, $vf0w, $vf0x
	vmulax.w	$ACCw, $vf0w, $vf31x
	vmulax.w	$ACCw, $vf1w, $vf2x
	vmulax.w	$ACCw, $vf31w, $vf0x
	vmulax.w	$ACCw, $vf31w, $vf15x
	vmulax.w	$ACCw, $vf31w, $vf31x
	vmula.xw	$ACCxw, $vf0xw, $vf0xw
	vmula.xw	$ACCxw, $vf0xw, $vf31xw
	vmula.xw	$ACCxw, $vf1xw, $vf2xw
	vmula.xw	$ACCxw, $vf31xw, $vf0xw
	vmula.xw	$ACCxw, $vf31xw, $vf15xw
	vmula.xw	$ACCxw, $vf31xw, $vf31xw
	vmulax.x	$ACCx, $vf0x, $vf0x
	vmulax.x	$ACCx, $vf0x, $vf31x
	vmulax.x	$ACCx, $vf1x, $vf2x
	vmulax.x	$ACCx, $vf31x, $vf0x
	vmulax.x	$ACCx, $vf31x, $vf15x
	vmulax.x	$ACCx, $vf31x, $vf31x
	vmulax.xw	$ACCxw, $vf0xw, $vf0x
	vmulax.xw	$ACCxw, $vf0xw, $vf31x
	vmulax.xw	$ACCxw, $vf1xw, $vf2x
	vmulax.xw	$ACCxw, $vf31xw, $vf0x
	vmulax.xw	$ACCxw, $vf31xw, $vf15x
	vmulax.xw	$ACCxw, $vf31xw, $vf31x
	vmulax.xy	$ACCxy, $vf0xy, $vf0x
	vmulax.xy	$ACCxy, $vf0xy, $vf31x
	vmulax.xy	$ACCxy, $vf1xy, $vf2x
	vmulax.xy	$ACCxy, $vf31xy, $vf0x
	vmulax.xy	$ACCxy, $vf31xy, $vf15x
	vmulax.xy	$ACCxy, $vf31xy, $vf31x
	vmulax.xyw	$ACCxyw, $vf0xyw, $vf0x
	vmulax.xyw	$ACCxyw, $vf0xyw, $vf31x
	vmulax.xyw	$ACCxyw, $vf1xyw, $vf2x
	vmulax.xyw	$ACCxyw, $vf31xyw, $vf0x
	vmulax.xyw	$ACCxyw, $vf31xyw, $vf15x
	vmulax.xyw	$ACCxyw, $vf31xyw, $vf31x
	vmulax.xyz	$ACCxyz, $vf0xyz, $vf0x
	vmulax.xyz	$ACCxyz, $vf0xyz, $vf31x
	vmulax.xyz	$ACCxyz, $vf1xyz, $vf2x
	vmulax.xyz	$ACCxyz, $vf31xyz, $vf0x
	vmulax.xyz	$ACCxyz, $vf31xyz, $vf15x
	vmulax.xyz	$ACCxyz, $vf31xyz, $vf31x
	vmulax.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vmulax.xyzw	$ACCxyzw, $vf0xyzw, $vf31x
	vmulax.xyzw	$ACCxyzw, $vf1xyzw, $vf2x
	vmulax.xyzw	$ACCxyzw, $vf31xyzw, $vf0x
	vmulax.xyzw	$ACCxyzw, $vf31xyzw, $vf15x
	vmulax.xyzw	$ACCxyzw, $vf31xyzw, $vf31x
	vmulax.xz	$ACCxz, $vf0xz, $vf0x
	vmulax.xz	$ACCxz, $vf0xz, $vf31x
	vmulax.xz	$ACCxz, $vf1xz, $vf2x
	vmulax.xz	$ACCxz, $vf31xz, $vf0x
	vmulax.xz	$ACCxz, $vf31xz, $vf15x
	vmulax.xz	$ACCxz, $vf31xz, $vf31x
	vmulax.xzw	$ACCxzw, $vf0xzw, $vf0x
	vmulax.xzw	$ACCxzw, $vf0xzw, $vf31x
	vmulax.xzw	$ACCxzw, $vf1xzw, $vf2x
	vmulax.xzw	$ACCxzw, $vf31xzw, $vf0x
	vmulax.xzw	$ACCxzw, $vf31xzw, $vf15x
	vmulax.xzw	$ACCxzw, $vf31xzw, $vf31x
	vmula.xy	$ACCxy, $vf0xy, $vf0xy
	vmula.xy	$ACCxy, $vf0xy, $vf31xy
	vmula.xy	$ACCxy, $vf1xy, $vf2xy
	vmula.xy	$ACCxy, $vf31xy, $vf0xy
	vmula.xy	$ACCxy, $vf31xy, $vf15xy
	vmula.xy	$ACCxy, $vf31xy, $vf31xy
	vmulax.y	$ACCy, $vf0y, $vf0x
	vmulax.y	$ACCy, $vf0y, $vf31x
	vmulax.y	$ACCy, $vf1y, $vf2x
	vmulax.y	$ACCy, $vf31y, $vf0x
	vmulax.y	$ACCy, $vf31y, $vf15x
	vmulax.y	$ACCy, $vf31y, $vf31x
	vmula.xyw	$ACCxyw, $vf0xyw, $vf0xyw
	vmula.xyw	$ACCxyw, $vf0xyw, $vf31xyw
	vmula.xyw	$ACCxyw, $vf1xyw, $vf2xyw
	vmula.xyw	$ACCxyw, $vf31xyw, $vf0xyw
	vmula.xyw	$ACCxyw, $vf31xyw, $vf15xyw
	vmula.xyw	$ACCxyw, $vf31xyw, $vf31xyw
	vmulax.yw	$ACCyw, $vf0yw, $vf0x
	vmulax.yw	$ACCyw, $vf0yw, $vf31x
	vmulax.yw	$ACCyw, $vf1yw, $vf2x
	vmulax.yw	$ACCyw, $vf31yw, $vf0x
	vmulax.yw	$ACCyw, $vf31yw, $vf15x
	vmulax.yw	$ACCyw, $vf31yw, $vf31x
	vmula.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vmula.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vmula.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vmula.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vmula.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vmula.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vmulax.yz	$ACCyz, $vf0yz, $vf0x
	vmulax.yz	$ACCyz, $vf0yz, $vf31x
	vmulax.yz	$ACCyz, $vf1yz, $vf2x
	vmulax.yz	$ACCyz, $vf31yz, $vf0x
	vmulax.yz	$ACCyz, $vf31yz, $vf15x
	vmulax.yz	$ACCyz, $vf31yz, $vf31x
	vmula.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vmula.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vmula.xyzw	$ACCxyzw, $vf1xyzw, $vf2xyzw
	vmula.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vmula.xyzw	$ACCxyzw, $vf31xyzw, $vf15xyzw
	vmula.xyzw	$ACCxyzw, $vf31xyzw, $vf31xyzw
	vmulax.yzw	$ACCyzw, $vf0yzw, $vf0x
	vmulax.yzw	$ACCyzw, $vf0yzw, $vf31x
	vmulax.yzw	$ACCyzw, $vf1yzw, $vf2x
	vmulax.yzw	$ACCyzw, $vf31yzw, $vf0x
	vmulax.yzw	$ACCyzw, $vf31yzw, $vf15x
	vmulax.yzw	$ACCyzw, $vf31yzw, $vf31x
	vmula.xz	$ACCxz, $vf0xz, $vf0xz
	vmula.xz	$ACCxz, $vf0xz, $vf31xz
	vmula.xz	$ACCxz, $vf1xz, $vf2xz
	vmula.xz	$ACCxz, $vf31xz, $vf0xz
	vmula.xz	$ACCxz, $vf31xz, $vf15xz
	vmula.xz	$ACCxz, $vf31xz, $vf31xz
	vmulax.z	$ACCz, $vf0z, $vf0x
	vmulax.z	$ACCz, $vf0z, $vf31x
	vmulax.z	$ACCz, $vf1z, $vf2x
	vmulax.z	$ACCz, $vf31z, $vf0x
	vmulax.z	$ACCz, $vf31z, $vf15x
	vmulax.z	$ACCz, $vf31z, $vf31x
	vmula.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vmula.xzw	$ACCxzw, $vf0xzw, $vf31xzw
	vmula.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vmula.xzw	$ACCxzw, $vf31xzw, $vf0xzw
	vmula.xzw	$ACCxzw, $vf31xzw, $vf15xzw
	vmula.xzw	$ACCxzw, $vf31xzw, $vf31xzw
	vmulax.zw	$ACCzw, $vf0zw, $vf0x
	vmulax.zw	$ACCzw, $vf0zw, $vf31x
	vmulax.zw	$ACCzw, $vf1zw, $vf2x
	vmulax.zw	$ACCzw, $vf31zw, $vf0x
	vmulax.zw	$ACCzw, $vf31zw, $vf15x
	vmulax.zw	$ACCzw, $vf31zw, $vf31x
	vmula.y		$ACCy, $vf0y, $vf0y
	vmula.y		$ACCy, $vf0y, $vf31y
	vmula.y		$ACCy, $vf1y, $vf2y
	vmula.y		$ACCy, $vf31y, $vf0y
	vmula.y		$ACCy, $vf31y, $vf15y
	vmula.y		$ACCy, $vf31y, $vf31y
	vmulay.w	$ACCw, $vf0w, $vf0y
	vmulay.w	$ACCw, $vf0w, $vf31y
	vmulay.w	$ACCw, $vf1w, $vf2y
	vmulay.w	$ACCw, $vf31w, $vf0y
	vmulay.w	$ACCw, $vf31w, $vf15y
	vmulay.w	$ACCw, $vf31w, $vf31y
	vmula.yw	$ACCyw, $vf0yw, $vf0yw
	vmula.yw	$ACCyw, $vf0yw, $vf31yw
	vmula.yw	$ACCyw, $vf1yw, $vf2yw
	vmula.yw	$ACCyw, $vf31yw, $vf0yw
	vmula.yw	$ACCyw, $vf31yw, $vf15yw
	vmula.yw	$ACCyw, $vf31yw, $vf31yw
	vmulay.x	$ACCx, $vf0x, $vf0y
	vmulay.x	$ACCx, $vf0x, $vf31y
	vmulay.x	$ACCx, $vf1x, $vf2y
	vmulay.x	$ACCx, $vf31x, $vf0y
	vmulay.x	$ACCx, $vf31x, $vf15y
	vmulay.x	$ACCx, $vf31x, $vf31y
	vmulay.xw	$ACCxw, $vf0xw, $vf0y
	vmulay.xw	$ACCxw, $vf0xw, $vf31y
	vmulay.xw	$ACCxw, $vf1xw, $vf2y
	vmulay.xw	$ACCxw, $vf31xw, $vf0y
	vmulay.xw	$ACCxw, $vf31xw, $vf15y
	vmulay.xw	$ACCxw, $vf31xw, $vf31y
	vmulay.xy	$ACCxy, $vf0xy, $vf0y
	vmulay.xy	$ACCxy, $vf0xy, $vf31y
	vmulay.xy	$ACCxy, $vf1xy, $vf2y
	vmulay.xy	$ACCxy, $vf31xy, $vf0y
	vmulay.xy	$ACCxy, $vf31xy, $vf15y
	vmulay.xy	$ACCxy, $vf31xy, $vf31y
	vmulay.xyw	$ACCxyw, $vf0xyw, $vf0y
	vmulay.xyw	$ACCxyw, $vf0xyw, $vf31y
	vmulay.xyw	$ACCxyw, $vf1xyw, $vf2y
	vmulay.xyw	$ACCxyw, $vf31xyw, $vf0y
	vmulay.xyw	$ACCxyw, $vf31xyw, $vf15y
	vmulay.xyw	$ACCxyw, $vf31xyw, $vf31y
	vmulay.xyz	$ACCxyz, $vf0xyz, $vf0y
	vmulay.xyz	$ACCxyz, $vf0xyz, $vf31y
	vmulay.xyz	$ACCxyz, $vf1xyz, $vf2y
	vmulay.xyz	$ACCxyz, $vf31xyz, $vf0y
	vmulay.xyz	$ACCxyz, $vf31xyz, $vf15y
	vmulay.xyz	$ACCxyz, $vf31xyz, $vf31y
	vmulay.xyzw	$ACCxyzw, $vf0xyzw, $vf0y
	vmulay.xyzw	$ACCxyzw, $vf0xyzw, $vf31y
	vmulay.xyzw	$ACCxyzw, $vf1xyzw, $vf2y
	vmulay.xyzw	$ACCxyzw, $vf31xyzw, $vf0y
	vmulay.xyzw	$ACCxyzw, $vf31xyzw, $vf15y
	vmulay.xyzw	$ACCxyzw, $vf31xyzw, $vf31y
	vmulay.xz	$ACCxz, $vf0xz, $vf0y
	vmulay.xz	$ACCxz, $vf0xz, $vf31y
	vmulay.xz	$ACCxz, $vf1xz, $vf2y
	vmulay.xz	$ACCxz, $vf31xz, $vf0y
	vmulay.xz	$ACCxz, $vf31xz, $vf15y
	vmulay.xz	$ACCxz, $vf31xz, $vf31y
	vmulay.xzw	$ACCxzw, $vf0xzw, $vf0y
	vmulay.xzw	$ACCxzw, $vf0xzw, $vf31y
	vmulay.xzw	$ACCxzw, $vf1xzw, $vf2y
	vmulay.xzw	$ACCxzw, $vf31xzw, $vf0y
	vmulay.xzw	$ACCxzw, $vf31xzw, $vf15y
	vmulay.xzw	$ACCxzw, $vf31xzw, $vf31y
	vmulay.y	$ACCy, $vf0y, $vf0y
	vmulay.y	$ACCy, $vf0y, $vf31y
	vmulay.y	$ACCy, $vf1y, $vf2y
	vmulay.y	$ACCy, $vf31y, $vf0y
	vmulay.y	$ACCy, $vf31y, $vf15y
	vmulay.y	$ACCy, $vf31y, $vf31y
	vmulay.yw	$ACCyw, $vf0yw, $vf0y
	vmulay.yw	$ACCyw, $vf0yw, $vf31y
	vmulay.yw	$ACCyw, $vf1yw, $vf2y
	vmulay.yw	$ACCyw, $vf31yw, $vf0y
	vmulay.yw	$ACCyw, $vf31yw, $vf15y
	vmulay.yw	$ACCyw, $vf31yw, $vf31y
	vmulay.yz	$ACCyz, $vf0yz, $vf0y
	vmulay.yz	$ACCyz, $vf0yz, $vf31y
	vmulay.yz	$ACCyz, $vf1yz, $vf2y
	vmulay.yz	$ACCyz, $vf31yz, $vf0y
	vmulay.yz	$ACCyz, $vf31yz, $vf15y
	vmulay.yz	$ACCyz, $vf31yz, $vf31y
	vmulay.yzw	$ACCyzw, $vf0yzw, $vf0y
	vmulay.yzw	$ACCyzw, $vf0yzw, $vf31y
	vmulay.yzw	$ACCyzw, $vf1yzw, $vf2y
	vmulay.yzw	$ACCyzw, $vf31yzw, $vf0y
	vmulay.yzw	$ACCyzw, $vf31yzw, $vf15y
	vmulay.yzw	$ACCyzw, $vf31yzw, $vf31y
	vmula.yz	$ACCyz, $vf0yz, $vf0yz
	vmula.yz	$ACCyz, $vf0yz, $vf31yz
	vmula.yz	$ACCyz, $vf1yz, $vf2yz
	vmula.yz	$ACCyz, $vf31yz, $vf0yz
	vmula.yz	$ACCyz, $vf31yz, $vf15yz
	vmula.yz	$ACCyz, $vf31yz, $vf31yz
	vmulay.z	$ACCz, $vf0z, $vf0y
	vmulay.z	$ACCz, $vf0z, $vf31y
	vmulay.z	$ACCz, $vf1z, $vf2y
	vmulay.z	$ACCz, $vf31z, $vf0y
	vmulay.z	$ACCz, $vf31z, $vf15y
	vmulay.z	$ACCz, $vf31z, $vf31y
	vmula.yzw	$ACCyzw, $vf0yzw, $vf0yzw
	vmula.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vmula.yzw	$ACCyzw, $vf1yzw, $vf2yzw
	vmula.yzw	$ACCyzw, $vf31yzw, $vf0yzw
	vmula.yzw	$ACCyzw, $vf31yzw, $vf15yzw
	vmula.yzw	$ACCyzw, $vf31yzw, $vf31yzw
	vmulay.zw	$ACCzw, $vf0zw, $vf0y
	vmulay.zw	$ACCzw, $vf0zw, $vf31y
	vmulay.zw	$ACCzw, $vf1zw, $vf2y
	vmulay.zw	$ACCzw, $vf31zw, $vf0y
	vmulay.zw	$ACCzw, $vf31zw, $vf15y
	vmulay.zw	$ACCzw, $vf31zw, $vf31y
	vmula.z		$ACCz, $vf0z, $vf0z
	vmula.z		$ACCz, $vf0z, $vf31z
	vmula.z		$ACCz, $vf1z, $vf2z
	vmula.z		$ACCz, $vf31z, $vf0z
	vmula.z		$ACCz, $vf31z, $vf15z
	vmula.z		$ACCz, $vf31z, $vf31z
	vmulaz.w	$ACCw, $vf0w, $vf0z
	vmulaz.w	$ACCw, $vf0w, $vf31z
	vmulaz.w	$ACCw, $vf1w, $vf2z
	vmulaz.w	$ACCw, $vf31w, $vf0z
	vmulaz.w	$ACCw, $vf31w, $vf15z
	vmulaz.w	$ACCw, $vf31w, $vf31z
	vmula.zw	$ACCzw, $vf0zw, $vf0zw
	vmula.zw	$ACCzw, $vf0zw, $vf31zw
	vmula.zw	$ACCzw, $vf1zw, $vf2zw
	vmula.zw	$ACCzw, $vf31zw, $vf0zw
	vmula.zw	$ACCzw, $vf31zw, $vf15zw
	vmula.zw	$ACCzw, $vf31zw, $vf31zw
	vmulaz.x	$ACCx, $vf0x, $vf0z
	vmulaz.x	$ACCx, $vf0x, $vf31z
	vmulaz.x	$ACCx, $vf1x, $vf2z
	vmulaz.x	$ACCx, $vf31x, $vf0z
	vmulaz.x	$ACCx, $vf31x, $vf15z
	vmulaz.x	$ACCx, $vf31x, $vf31z
	vmulaz.xw	$ACCxw, $vf0xw, $vf0z
	vmulaz.xw	$ACCxw, $vf0xw, $vf31z
	vmulaz.xw	$ACCxw, $vf1xw, $vf2z
	vmulaz.xw	$ACCxw, $vf31xw, $vf0z
	vmulaz.xw	$ACCxw, $vf31xw, $vf15z
	vmulaz.xw	$ACCxw, $vf31xw, $vf31z
	vmulaz.xy	$ACCxy, $vf0xy, $vf0z
	vmulaz.xy	$ACCxy, $vf0xy, $vf31z
	vmulaz.xy	$ACCxy, $vf1xy, $vf2z
	vmulaz.xy	$ACCxy, $vf31xy, $vf0z
	vmulaz.xy	$ACCxy, $vf31xy, $vf15z
	vmulaz.xy	$ACCxy, $vf31xy, $vf31z
	vmulaz.xyw	$ACCxyw, $vf0xyw, $vf0z
	vmulaz.xyw	$ACCxyw, $vf0xyw, $vf31z
	vmulaz.xyw	$ACCxyw, $vf1xyw, $vf2z
	vmulaz.xyw	$ACCxyw, $vf31xyw, $vf0z
	vmulaz.xyw	$ACCxyw, $vf31xyw, $vf15z
	vmulaz.xyw	$ACCxyw, $vf31xyw, $vf31z
	vmulaz.xyz	$ACCxyz, $vf0xyz, $vf0z
	vmulaz.xyz	$ACCxyz, $vf0xyz, $vf31z
	vmulaz.xyz	$ACCxyz, $vf1xyz, $vf2z
	vmulaz.xyz	$ACCxyz, $vf31xyz, $vf0z
	vmulaz.xyz	$ACCxyz, $vf31xyz, $vf15z
	vmulaz.xyz	$ACCxyz, $vf31xyz, $vf31z
	vmulaz.xyzw	$ACCxyzw, $vf0xyzw, $vf0z
	vmulaz.xyzw	$ACCxyzw, $vf0xyzw, $vf31z
	vmulaz.xyzw	$ACCxyzw, $vf1xyzw, $vf2z
	vmulaz.xyzw	$ACCxyzw, $vf31xyzw, $vf0z
	vmulaz.xyzw	$ACCxyzw, $vf31xyzw, $vf15z
	vmulaz.xyzw	$ACCxyzw, $vf31xyzw, $vf31z
	vmulaz.xz	$ACCxz, $vf0xz, $vf0z
	vmulaz.xz	$ACCxz, $vf0xz, $vf31z
	vmulaz.xz	$ACCxz, $vf1xz, $vf2z
	vmulaz.xz	$ACCxz, $vf31xz, $vf0z
	vmulaz.xz	$ACCxz, $vf31xz, $vf15z
	vmulaz.xz	$ACCxz, $vf31xz, $vf31z
	vmulaz.xzw	$ACCxzw, $vf0xzw, $vf0z
	vmulaz.xzw	$ACCxzw, $vf0xzw, $vf31z
	vmulaz.xzw	$ACCxzw, $vf1xzw, $vf2z
	vmulaz.xzw	$ACCxzw, $vf31xzw, $vf0z
	vmulaz.xzw	$ACCxzw, $vf31xzw, $vf15z
	vmulaz.xzw	$ACCxzw, $vf31xzw, $vf31z
	vmulaz.y	$ACCy, $vf0y, $vf0z
	vmulaz.y	$ACCy, $vf0y, $vf31z
	vmulaz.y	$ACCy, $vf1y, $vf2z
	vmulaz.y	$ACCy, $vf31y, $vf0z
	vmulaz.y	$ACCy, $vf31y, $vf15z
	vmulaz.y	$ACCy, $vf31y, $vf31z
	vmulaz.yw	$ACCyw, $vf0yw, $vf0z
	vmulaz.yw	$ACCyw, $vf0yw, $vf31z
	vmulaz.yw	$ACCyw, $vf1yw, $vf2z
	vmulaz.yw	$ACCyw, $vf31yw, $vf0z
	vmulaz.yw	$ACCyw, $vf31yw, $vf15z
	vmulaz.yw	$ACCyw, $vf31yw, $vf31z
	vmulaz.yz	$ACCyz, $vf0yz, $vf0z
	vmulaz.yz	$ACCyz, $vf0yz, $vf31z
	vmulaz.yz	$ACCyz, $vf1yz, $vf2z
	vmulaz.yz	$ACCyz, $vf31yz, $vf0z
	vmulaz.yz	$ACCyz, $vf31yz, $vf15z
	vmulaz.yz	$ACCyz, $vf31yz, $vf31z
	vmulaz.yzw	$ACCyzw, $vf0yzw, $vf0z
	vmulaz.yzw	$ACCyzw, $vf0yzw, $vf31z
	vmulaz.yzw	$ACCyzw, $vf1yzw, $vf2z
	vmulaz.yzw	$ACCyzw, $vf31yzw, $vf0z
	vmulaz.yzw	$ACCyzw, $vf31yzw, $vf15z
	vmulaz.yzw	$ACCyzw, $vf31yzw, $vf31z
	vmulaz.z	$ACCz, $vf0z, $vf0z
	vmulaz.z	$ACCz, $vf0z, $vf31z
	vmulaz.z	$ACCz, $vf1z, $vf2z
	vmulaz.z	$ACCz, $vf31z, $vf0z
	vmulaz.z	$ACCz, $vf31z, $vf15z
	vmulaz.z	$ACCz, $vf31z, $vf31z
	vmulaz.zw	$ACCzw, $vf0zw, $vf0z
	vmulaz.zw	$ACCzw, $vf0zw, $vf31z
	vmulaz.zw	$ACCzw, $vf1zw, $vf2z
	vmulaz.zw	$ACCzw, $vf31zw, $vf0z
	vmulaz.zw	$ACCzw, $vf31zw, $vf15z
	vmulaz.zw	$ACCzw, $vf31zw, $vf31z
	vmuli.w		$vf0w, $vf0w, $I
	vmuli.w		$vf0w, $vf31w, $I
	vmuli.w		$vf1w, $vf2w, $I
	vmuli.w		$vf31w, $vf0w, $I
	vmuli.w		$vf31w, $vf15w, $I
	vmuli.w		$vf31w, $vf31w, $I
	vmuli.x		$vf0x, $vf0x, $I
	vmuli.x		$vf0x, $vf31x, $I
	vmuli.x		$vf1x, $vf2x, $I
	vmuli.x		$vf31x, $vf0x, $I
	vmuli.x		$vf31x, $vf15x, $I
	vmuli.x		$vf31x, $vf31x, $I
	vmuli.xw	$vf0xw, $vf0xw, $I
	vmuli.xw	$vf0xw, $vf31xw, $I
	vmuli.xw	$vf1xw, $vf2xw, $I
	vmuli.xw	$vf31xw, $vf0xw, $I
	vmuli.xw	$vf31xw, $vf15xw, $I
	vmuli.xw	$vf31xw, $vf31xw, $I
	vmuli.xy	$vf0xy, $vf0xy, $I
	vmuli.xy	$vf0xy, $vf31xy, $I
	vmuli.xy	$vf1xy, $vf2xy, $I
	vmuli.xy	$vf31xy, $vf0xy, $I
	vmuli.xy	$vf31xy, $vf15xy, $I
	vmuli.xy	$vf31xy, $vf31xy, $I
	vmuli.xyw	$vf0xyw, $vf0xyw, $I
	vmuli.xyw	$vf0xyw, $vf31xyw, $I
	vmuli.xyw	$vf1xyw, $vf2xyw, $I
	vmuli.xyw	$vf31xyw, $vf0xyw, $I
	vmuli.xyw	$vf31xyw, $vf15xyw, $I
	vmuli.xyw	$vf31xyw, $vf31xyw, $I
	vmuli.xyz	$vf0xyz, $vf0xyz, $I
	vmuli.xyz	$vf0xyz, $vf31xyz, $I
	vmuli.xyz	$vf1xyz, $vf2xyz, $I
	vmuli.xyz	$vf31xyz, $vf0xyz, $I
	vmuli.xyz	$vf31xyz, $vf15xyz, $I
	vmuli.xyz	$vf31xyz, $vf31xyz, $I
	vmuli.xyzw	$vf0xyzw, $vf0xyzw, $I
	vmuli.xyzw	$vf0xyzw, $vf31xyzw, $I
	vmuli.xyzw	$vf1xyzw, $vf2xyzw, $I
	vmuli.xyzw	$vf31xyzw, $vf0xyzw, $I
	vmuli.xyzw	$vf31xyzw, $vf15xyzw, $I
	vmuli.xyzw	$vf31xyzw, $vf31xyzw, $I
	vmuli.xz	$vf0xz, $vf0xz, $I
	vmuli.xz	$vf0xz, $vf31xz, $I
	vmuli.xz	$vf1xz, $vf2xz, $I
	vmuli.xz	$vf31xz, $vf0xz, $I
	vmuli.xz	$vf31xz, $vf15xz, $I
	vmuli.xz	$vf31xz, $vf31xz, $I
	vmuli.xzw	$vf0xzw, $vf0xzw, $I
	vmuli.xzw	$vf0xzw, $vf31xzw, $I
	vmuli.xzw	$vf1xzw, $vf2xzw, $I
	vmuli.xzw	$vf31xzw, $vf0xzw, $I
	vmuli.xzw	$vf31xzw, $vf15xzw, $I
	vmuli.xzw	$vf31xzw, $vf31xzw, $I
	vmuli.y		$vf0y, $vf0y, $I
	vmuli.y		$vf0y, $vf31y, $I
	vmuli.y		$vf1y, $vf2y, $I
	vmuli.y		$vf31y, $vf0y, $I
	vmuli.y		$vf31y, $vf15y, $I
	vmuli.y		$vf31y, $vf31y, $I
	vmuli.yw	$vf0yw, $vf0yw, $I
	vmuli.yw	$vf0yw, $vf31yw, $I
	vmuli.yw	$vf1yw, $vf2yw, $I
	vmuli.yw	$vf31yw, $vf0yw, $I
	vmuli.yw	$vf31yw, $vf15yw, $I
	vmuli.yw	$vf31yw, $vf31yw, $I
	vmuli.yz	$vf0yz, $vf0yz, $I
	vmuli.yz	$vf0yz, $vf31yz, $I
	vmuli.yz	$vf1yz, $vf2yz, $I
	vmuli.yz	$vf31yz, $vf0yz, $I
	vmuli.yz	$vf31yz, $vf15yz, $I
	vmuli.yz	$vf31yz, $vf31yz, $I
	vmuli.yzw	$vf0yzw, $vf0yzw, $I
	vmuli.yzw	$vf0yzw, $vf31yzw, $I
	vmuli.yzw	$vf1yzw, $vf2yzw, $I
	vmuli.yzw	$vf31yzw, $vf0yzw, $I
	vmuli.yzw	$vf31yzw, $vf15yzw, $I
	vmuli.yzw	$vf31yzw, $vf31yzw, $I
	vmuli.z		$vf0z, $vf0z, $I
	vmuli.z		$vf0z, $vf31z, $I
	vmuli.z		$vf1z, $vf2z, $I
	vmuli.z		$vf31z, $vf0z, $I
	vmuli.z		$vf31z, $vf15z, $I
	vmuli.z		$vf31z, $vf31z, $I
	vmuli.zw	$vf0zw, $vf0zw, $I
	vmuli.zw	$vf0zw, $vf31zw, $I
	vmuli.zw	$vf1zw, $vf2zw, $I
	vmuli.zw	$vf31zw, $vf0zw, $I
	vmuli.zw	$vf31zw, $vf15zw, $I
	vmuli.zw	$vf31zw, $vf31zw, $I
	vmulq.w		$vf0w, $vf0w, $Q
	vmulq.w		$vf0w, $vf31w, $Q
	vmulq.w		$vf1w, $vf2w, $Q
	vmulq.w		$vf31w, $vf0w, $Q
	vmulq.w		$vf31w, $vf15w, $Q
	vmulq.w		$vf31w, $vf31w, $Q
	vmulq.x		$vf0x, $vf0x, $Q
	vmulq.x		$vf0x, $vf31x, $Q
	vmulq.x		$vf1x, $vf2x, $Q
	vmulq.x		$vf31x, $vf0x, $Q
	vmulq.x		$vf31x, $vf15x, $Q
	vmulq.x		$vf31x, $vf31x, $Q
	vmulq.xw	$vf0xw, $vf0xw, $Q
	vmulq.xw	$vf0xw, $vf31xw, $Q
	vmulq.xw	$vf1xw, $vf2xw, $Q
	vmulq.xw	$vf31xw, $vf0xw, $Q
	vmulq.xw	$vf31xw, $vf15xw, $Q
	vmulq.xw	$vf31xw, $vf31xw, $Q
	vmulq.xy	$vf0xy, $vf0xy, $Q
	vmulq.xy	$vf0xy, $vf31xy, $Q
	vmulq.xy	$vf1xy, $vf2xy, $Q
	vmulq.xy	$vf31xy, $vf0xy, $Q
	vmulq.xy	$vf31xy, $vf15xy, $Q
	vmulq.xy	$vf31xy, $vf31xy, $Q
	vmulq.xyw	$vf0xyw, $vf0xyw, $Q
	vmulq.xyw	$vf0xyw, $vf31xyw, $Q
	vmulq.xyw	$vf1xyw, $vf2xyw, $Q
	vmulq.xyw	$vf31xyw, $vf0xyw, $Q
	vmulq.xyw	$vf31xyw, $vf15xyw, $Q
	vmulq.xyw	$vf31xyw, $vf31xyw, $Q
	vmulq.xyz	$vf0xyz, $vf0xyz, $Q
	vmulq.xyz	$vf0xyz, $vf31xyz, $Q
	vmulq.xyz	$vf1xyz, $vf2xyz, $Q
	vmulq.xyz	$vf31xyz, $vf0xyz, $Q
	vmulq.xyz	$vf31xyz, $vf15xyz, $Q
	vmulq.xyz	$vf31xyz, $vf31xyz, $Q
	vmulq.xyzw	$vf0xyzw, $vf0xyzw, $Q
	vmulq.xyzw	$vf0xyzw, $vf31xyzw, $Q
	vmulq.xyzw	$vf1xyzw, $vf2xyzw, $Q
	vmulq.xyzw	$vf31xyzw, $vf0xyzw, $Q
	vmulq.xyzw	$vf31xyzw, $vf15xyzw, $Q
	vmulq.xyzw	$vf31xyzw, $vf31xyzw, $Q
	vmulq.xz	$vf0xz, $vf0xz, $Q
	vmulq.xz	$vf0xz, $vf31xz, $Q
	vmulq.xz	$vf1xz, $vf2xz, $Q
	vmulq.xz	$vf31xz, $vf0xz, $Q
	vmulq.xz	$vf31xz, $vf15xz, $Q
	vmulq.xz	$vf31xz, $vf31xz, $Q
	vmulq.xzw	$vf0xzw, $vf0xzw, $Q
	vmulq.xzw	$vf0xzw, $vf31xzw, $Q
	vmulq.xzw	$vf1xzw, $vf2xzw, $Q
	vmulq.xzw	$vf31xzw, $vf0xzw, $Q
	vmulq.xzw	$vf31xzw, $vf15xzw, $Q
	vmulq.xzw	$vf31xzw, $vf31xzw, $Q
	vmulq.y		$vf0y, $vf0y, $Q
	vmulq.y		$vf0y, $vf31y, $Q
	vmulq.y		$vf1y, $vf2y, $Q
	vmulq.y		$vf31y, $vf0y, $Q
	vmulq.y		$vf31y, $vf15y, $Q
	vmulq.y		$vf31y, $vf31y, $Q
	vmulq.yw	$vf0yw, $vf0yw, $Q
	vmulq.yw	$vf0yw, $vf31yw, $Q
	vmulq.yw	$vf1yw, $vf2yw, $Q
	vmulq.yw	$vf31yw, $vf0yw, $Q
	vmulq.yw	$vf31yw, $vf15yw, $Q
	vmulq.yw	$vf31yw, $vf31yw, $Q
	vmulq.yz	$vf0yz, $vf0yz, $Q
	vmulq.yz	$vf0yz, $vf31yz, $Q
	vmulq.yz	$vf1yz, $vf2yz, $Q
	vmulq.yz	$vf31yz, $vf0yz, $Q
	vmulq.yz	$vf31yz, $vf15yz, $Q
	vmulq.yz	$vf31yz, $vf31yz, $Q
	vmulq.yzw	$vf0yzw, $vf0yzw, $Q
	vmulq.yzw	$vf0yzw, $vf31yzw, $Q
	vmulq.yzw	$vf1yzw, $vf2yzw, $Q
	vmulq.yzw	$vf31yzw, $vf0yzw, $Q
	vmulq.yzw	$vf31yzw, $vf15yzw, $Q
	vmulq.yzw	$vf31yzw, $vf31yzw, $Q
	vmulq.z		$vf0z, $vf0z, $Q
	vmulq.z		$vf0z, $vf31z, $Q
	vmulq.z		$vf1z, $vf2z, $Q
	vmulq.z		$vf31z, $vf0z, $Q
	vmulq.z		$vf31z, $vf15z, $Q
	vmulq.z		$vf31z, $vf31z, $Q
	vmulq.zw	$vf0zw, $vf0zw, $Q
	vmulq.zw	$vf0zw, $vf31zw, $Q
	vmulq.zw	$vf1zw, $vf2zw, $Q
	vmulq.zw	$vf31zw, $vf0zw, $Q
	vmulq.zw	$vf31zw, $vf15zw, $Q
	vmulq.zw	$vf31zw, $vf31zw, $Q
	vmul.w		$vf0w, $vf0w, $vf0w
	vmul.w		$vf0w, $vf0w, $vf31w
	vmul.w		$vf0w, $vf31w, $vf0w
	vmul.w		$vf1w, $vf2w, $vf3w
	vmul.w		$vf31w, $vf0w, $vf0w
	vmul.w		$vf31w, $vf15w, $vf7w
	vmul.w		$vf31w, $vf31w, $vf31w
	vmulw.w		$vf0w, $vf0w, $vf0w
	vmulw.w		$vf0w, $vf0w, $vf31w
	vmulw.w		$vf0w, $vf31w, $vf0w
	vmulw.w		$vf1w, $vf2w, $vf3w
	vmulw.w		$vf31w, $vf0w, $vf0w
	vmulw.w		$vf31w, $vf15w, $vf7w
	vmulw.w		$vf31w, $vf31w, $vf31w
	vmulw.x		$vf0x, $vf0x, $vf0w
	vmulw.x		$vf0x, $vf0x, $vf31w
	vmulw.x		$vf0x, $vf31x, $vf0w
	vmulw.x		$vf1x, $vf2x, $vf3w
	vmulw.x		$vf31x, $vf0x, $vf0w
	vmulw.x		$vf31x, $vf15x, $vf7w
	vmulw.x		$vf31x, $vf31x, $vf31w
	vmulw.xw	$vf0xw, $vf0xw, $vf0w
	vmulw.xw	$vf0xw, $vf0xw, $vf31w
	vmulw.xw	$vf0xw, $vf31xw, $vf0w
	vmulw.xw	$vf1xw, $vf2xw, $vf3w
	vmulw.xw	$vf31xw, $vf0xw, $vf0w
	vmulw.xw	$vf31xw, $vf15xw, $vf7w
	vmulw.xw	$vf31xw, $vf31xw, $vf31w
	vmulw.xy	$vf0xy, $vf0xy, $vf0w
	vmulw.xy	$vf0xy, $vf0xy, $vf31w
	vmulw.xy	$vf0xy, $vf31xy, $vf0w
	vmulw.xy	$vf1xy, $vf2xy, $vf3w
	vmulw.xy	$vf31xy, $vf0xy, $vf0w
	vmulw.xy	$vf31xy, $vf15xy, $vf7w
	vmulw.xy	$vf31xy, $vf31xy, $vf31w
	vmulw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vmulw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vmulw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vmulw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vmulw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vmulw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vmulw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vmulw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vmulw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vmulw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vmulw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vmulw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vmulw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vmulw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vmulw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vmulw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vmulw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vmulw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vmulw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vmulw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vmulw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vmulw.xz	$vf0xz, $vf0xz, $vf0w
	vmulw.xz	$vf0xz, $vf0xz, $vf31w
	vmulw.xz	$vf0xz, $vf31xz, $vf0w
	vmulw.xz	$vf1xz, $vf2xz, $vf3w
	vmulw.xz	$vf31xz, $vf0xz, $vf0w
	vmulw.xz	$vf31xz, $vf15xz, $vf7w
	vmulw.xz	$vf31xz, $vf31xz, $vf31w
	vmulw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vmulw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vmulw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vmulw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vmulw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vmulw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vmulw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vmulw.y		$vf0y, $vf0y, $vf0w
	vmulw.y		$vf0y, $vf0y, $vf31w
	vmulw.y		$vf0y, $vf31y, $vf0w
	vmulw.y		$vf1y, $vf2y, $vf3w
	vmulw.y		$vf31y, $vf0y, $vf0w
	vmulw.y		$vf31y, $vf15y, $vf7w
	vmulw.y		$vf31y, $vf31y, $vf31w
	vmulw.yw	$vf0yw, $vf0yw, $vf0w
	vmulw.yw	$vf0yw, $vf0yw, $vf31w
	vmulw.yw	$vf0yw, $vf31yw, $vf0w
	vmulw.yw	$vf1yw, $vf2yw, $vf3w
	vmulw.yw	$vf31yw, $vf0yw, $vf0w
	vmulw.yw	$vf31yw, $vf15yw, $vf7w
	vmulw.yw	$vf31yw, $vf31yw, $vf31w
	vmulw.yz	$vf0yz, $vf0yz, $vf0w
	vmulw.yz	$vf0yz, $vf0yz, $vf31w
	vmulw.yz	$vf0yz, $vf31yz, $vf0w
	vmulw.yz	$vf1yz, $vf2yz, $vf3w
	vmulw.yz	$vf31yz, $vf0yz, $vf0w
	vmulw.yz	$vf31yz, $vf15yz, $vf7w
	vmulw.yz	$vf31yz, $vf31yz, $vf31w
	vmulw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vmulw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vmulw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vmulw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vmulw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vmulw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vmulw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vmulw.z		$vf0z, $vf0z, $vf0w
	vmulw.z		$vf0z, $vf0z, $vf31w
	vmulw.z		$vf0z, $vf31z, $vf0w
	vmulw.z		$vf1z, $vf2z, $vf3w
	vmulw.z		$vf31z, $vf0z, $vf0w
	vmulw.z		$vf31z, $vf15z, $vf7w
	vmulw.z		$vf31z, $vf31z, $vf31w
	vmulw.zw	$vf0zw, $vf0zw, $vf0w
	vmulw.zw	$vf0zw, $vf0zw, $vf31w
	vmulw.zw	$vf0zw, $vf31zw, $vf0w
	vmulw.zw	$vf1zw, $vf2zw, $vf3w
	vmulw.zw	$vf31zw, $vf0zw, $vf0w
	vmulw.zw	$vf31zw, $vf15zw, $vf7w
	vmulw.zw	$vf31zw, $vf31zw, $vf31w
	vmul.x		$vf0x, $vf0x, $vf0x
	vmul.x		$vf0x, $vf0x, $vf31x
	vmul.x		$vf0x, $vf31x, $vf0x
	vmul.x		$vf1x, $vf2x, $vf3x
	vmul.x		$vf31x, $vf0x, $vf0x
	vmul.x		$vf31x, $vf15x, $vf7x
	vmul.x		$vf31x, $vf31x, $vf31x
	vmulx.w		$vf0w, $vf0w, $vf0x
	vmulx.w		$vf0w, $vf0w, $vf31x
	vmulx.w		$vf0w, $vf31w, $vf0x
	vmul.xw		$vf0xw, $vf0xw, $vf0xw
	vmul.xw		$vf0xw, $vf0xw, $vf31xw
	vmul.xw		$vf0xw, $vf31xw, $vf0xw
	vmulx.w		$vf1w, $vf2w, $vf3x
	vmul.xw		$vf1xw, $vf2xw, $vf3xw
	vmulx.w		$vf31w, $vf0w, $vf0x
	vmulx.w		$vf31w, $vf15w, $vf7x
	vmulx.w		$vf31w, $vf31w, $vf31x
	vmul.xw		$vf31xw, $vf0xw, $vf0xw
	vmul.xw		$vf31xw, $vf15xw, $vf7xw
	vmul.xw		$vf31xw, $vf31xw, $vf31xw
	vmulx.x		$vf0x, $vf0x, $vf0x
	vmulx.x		$vf0x, $vf0x, $vf31x
	vmulx.x		$vf0x, $vf31x, $vf0x
	vmulx.x		$vf1x, $vf2x, $vf3x
	vmulx.x		$vf31x, $vf0x, $vf0x
	vmulx.x		$vf31x, $vf15x, $vf7x
	vmulx.x		$vf31x, $vf31x, $vf31x
	vmulx.xw	$vf0xw, $vf0xw, $vf0x
	vmulx.xw	$vf0xw, $vf0xw, $vf31x
	vmulx.xw	$vf0xw, $vf31xw, $vf0x
	vmulx.xw	$vf1xw, $vf2xw, $vf3x
	vmulx.xw	$vf31xw, $vf0xw, $vf0x
	vmulx.xw	$vf31xw, $vf15xw, $vf7x
	vmulx.xw	$vf31xw, $vf31xw, $vf31x
	vmulx.xy	$vf0xy, $vf0xy, $vf0x
	vmulx.xy	$vf0xy, $vf0xy, $vf31x
	vmulx.xy	$vf0xy, $vf31xy, $vf0x
	vmulx.xy	$vf1xy, $vf2xy, $vf3x
	vmulx.xy	$vf31xy, $vf0xy, $vf0x
	vmulx.xy	$vf31xy, $vf15xy, $vf7x
	vmulx.xy	$vf31xy, $vf31xy, $vf31x
	vmulx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vmulx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vmulx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vmulx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vmulx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vmulx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vmulx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vmulx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vmulx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vmulx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vmulx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vmulx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vmulx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vmulx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vmulx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vmulx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vmulx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vmulx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vmulx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vmulx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vmulx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vmulx.xz	$vf0xz, $vf0xz, $vf0x
	vmulx.xz	$vf0xz, $vf0xz, $vf31x
	vmulx.xz	$vf0xz, $vf31xz, $vf0x
	vmulx.xz	$vf1xz, $vf2xz, $vf3x
	vmulx.xz	$vf31xz, $vf0xz, $vf0x
	vmulx.xz	$vf31xz, $vf15xz, $vf7x
	vmulx.xz	$vf31xz, $vf31xz, $vf31x
	vmulx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vmulx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vmulx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vmulx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vmulx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vmulx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vmulx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vmul.xy		$vf0xy, $vf0xy, $vf0xy
	vmul.xy		$vf0xy, $vf0xy, $vf31xy
	vmul.xy		$vf0xy, $vf31xy, $vf0xy
	vmulx.y		$vf0y, $vf0y, $vf0x
	vmulx.y		$vf0y, $vf0y, $vf31x
	vmulx.y		$vf0y, $vf31y, $vf0x
	vmul.xy		$vf1xy, $vf2xy, $vf3xy
	vmulx.y		$vf1y, $vf2y, $vf3x
	vmul.xy		$vf31xy, $vf0xy, $vf0xy
	vmul.xy		$vf31xy, $vf15xy, $vf7xy
	vmul.xy		$vf31xy, $vf31xy, $vf31xy
	vmulx.y		$vf31y, $vf0y, $vf0x
	vmulx.y		$vf31y, $vf15y, $vf7x
	vmulx.y		$vf31y, $vf31y, $vf31x
	vmul.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vmul.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vmul.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vmulx.yw	$vf0yw, $vf0yw, $vf0x
	vmulx.yw	$vf0yw, $vf0yw, $vf31x
	vmulx.yw	$vf0yw, $vf31yw, $vf0x
	vmul.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vmulx.yw	$vf1yw, $vf2yw, $vf3x
	vmul.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vmul.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vmul.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vmulx.yw	$vf31yw, $vf0yw, $vf0x
	vmulx.yw	$vf31yw, $vf15yw, $vf7x
	vmulx.yw	$vf31yw, $vf31yw, $vf31x
	vmul.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vmul.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vmul.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vmulx.yz	$vf0yz, $vf0yz, $vf0x
	vmulx.yz	$vf0yz, $vf0yz, $vf31x
	vmulx.yz	$vf0yz, $vf31yz, $vf0x
	vmul.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vmulx.yz	$vf1yz, $vf2yz, $vf3x
	vmul.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vmul.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vmul.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vmulx.yz	$vf31yz, $vf0yz, $vf0x
	vmulx.yz	$vf31yz, $vf15yz, $vf7x
	vmulx.yz	$vf31yz, $vf31yz, $vf31x
	vmul.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vmul.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vmul.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vmulx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vmulx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vmulx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vmul.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vmulx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vmul.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vmul.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vmul.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vmulx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vmulx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vmulx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vmul.xz		$vf0xz, $vf0xz, $vf0xz
	vmul.xz		$vf0xz, $vf0xz, $vf31xz
	vmul.xz		$vf0xz, $vf31xz, $vf0xz
	vmulx.z		$vf0z, $vf0z, $vf0x
	vmulx.z		$vf0z, $vf0z, $vf31x
	vmulx.z		$vf0z, $vf31z, $vf0x
	vmul.xz		$vf1xz, $vf2xz, $vf3xz
	vmulx.z		$vf1z, $vf2z, $vf3x
	vmul.xz		$vf31xz, $vf0xz, $vf0xz
	vmul.xz		$vf31xz, $vf15xz, $vf7xz
	vmul.xz		$vf31xz, $vf31xz, $vf31xz
	vmulx.z		$vf31z, $vf0z, $vf0x
	vmulx.z		$vf31z, $vf15z, $vf7x
	vmulx.z		$vf31z, $vf31z, $vf31x
	vmul.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vmul.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vmul.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vmulx.zw	$vf0zw, $vf0zw, $vf0x
	vmulx.zw	$vf0zw, $vf0zw, $vf31x
	vmulx.zw	$vf0zw, $vf31zw, $vf0x
	vmul.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vmulx.zw	$vf1zw, $vf2zw, $vf3x
	vmul.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vmul.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vmul.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vmulx.zw	$vf31zw, $vf0zw, $vf0x
	vmulx.zw	$vf31zw, $vf15zw, $vf7x
	vmulx.zw	$vf31zw, $vf31zw, $vf31x
	vmul.y		$vf0y, $vf0y, $vf0y
	vmul.y		$vf0y, $vf0y, $vf31y
	vmul.y		$vf0y, $vf31y, $vf0y
	vmul.y		$vf1y, $vf2y, $vf3y
	vmul.y		$vf31y, $vf0y, $vf0y
	vmul.y		$vf31y, $vf15y, $vf7y
	vmul.y		$vf31y, $vf31y, $vf31y
	vmuly.w		$vf0w, $vf0w, $vf0y
	vmuly.w		$vf0w, $vf0w, $vf31y
	vmuly.w		$vf0w, $vf31w, $vf0y
	vmul.yw		$vf0yw, $vf0yw, $vf0yw
	vmul.yw		$vf0yw, $vf0yw, $vf31yw
	vmul.yw		$vf0yw, $vf31yw, $vf0yw
	vmuly.w		$vf1w, $vf2w, $vf3y
	vmul.yw		$vf1yw, $vf2yw, $vf3yw
	vmuly.w		$vf31w, $vf0w, $vf0y
	vmuly.w		$vf31w, $vf15w, $vf7y
	vmuly.w		$vf31w, $vf31w, $vf31y
	vmul.yw		$vf31yw, $vf0yw, $vf0yw
	vmul.yw		$vf31yw, $vf15yw, $vf7yw
	vmul.yw		$vf31yw, $vf31yw, $vf31yw
	vmuly.x		$vf0x, $vf0x, $vf0y
	vmuly.x		$vf0x, $vf0x, $vf31y
	vmuly.x		$vf0x, $vf31x, $vf0y
	vmuly.x		$vf1x, $vf2x, $vf3y
	vmuly.x		$vf31x, $vf0x, $vf0y
	vmuly.x		$vf31x, $vf15x, $vf7y
	vmuly.x		$vf31x, $vf31x, $vf31y
	vmuly.xw	$vf0xw, $vf0xw, $vf0y
	vmuly.xw	$vf0xw, $vf0xw, $vf31y
	vmuly.xw	$vf0xw, $vf31xw, $vf0y
	vmuly.xw	$vf1xw, $vf2xw, $vf3y
	vmuly.xw	$vf31xw, $vf0xw, $vf0y
	vmuly.xw	$vf31xw, $vf15xw, $vf7y
	vmuly.xw	$vf31xw, $vf31xw, $vf31y
	vmuly.xy	$vf0xy, $vf0xy, $vf0y
	vmuly.xy	$vf0xy, $vf0xy, $vf31y
	vmuly.xy	$vf0xy, $vf31xy, $vf0y
	vmuly.xy	$vf1xy, $vf2xy, $vf3y
	vmuly.xy	$vf31xy, $vf0xy, $vf0y
	vmuly.xy	$vf31xy, $vf15xy, $vf7y
	vmuly.xy	$vf31xy, $vf31xy, $vf31y
	vmuly.xyw	$vf0xyw, $vf0xyw, $vf0y
	vmuly.xyw	$vf0xyw, $vf0xyw, $vf31y
	vmuly.xyw	$vf0xyw, $vf31xyw, $vf0y
	vmuly.xyw	$vf1xyw, $vf2xyw, $vf3y
	vmuly.xyw	$vf31xyw, $vf0xyw, $vf0y
	vmuly.xyw	$vf31xyw, $vf15xyw, $vf7y
	vmuly.xyw	$vf31xyw, $vf31xyw, $vf31y
	vmuly.xyz	$vf0xyz, $vf0xyz, $vf0y
	vmuly.xyz	$vf0xyz, $vf0xyz, $vf31y
	vmuly.xyz	$vf0xyz, $vf31xyz, $vf0y
	vmuly.xyz	$vf1xyz, $vf2xyz, $vf3y
	vmuly.xyz	$vf31xyz, $vf0xyz, $vf0y
	vmuly.xyz	$vf31xyz, $vf15xyz, $vf7y
	vmuly.xyz	$vf31xyz, $vf31xyz, $vf31y
	vmuly.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vmuly.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vmuly.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vmuly.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vmuly.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vmuly.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vmuly.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vmuly.xz	$vf0xz, $vf0xz, $vf0y
	vmuly.xz	$vf0xz, $vf0xz, $vf31y
	vmuly.xz	$vf0xz, $vf31xz, $vf0y
	vmuly.xz	$vf1xz, $vf2xz, $vf3y
	vmuly.xz	$vf31xz, $vf0xz, $vf0y
	vmuly.xz	$vf31xz, $vf15xz, $vf7y
	vmuly.xz	$vf31xz, $vf31xz, $vf31y
	vmuly.xzw	$vf0xzw, $vf0xzw, $vf0y
	vmuly.xzw	$vf0xzw, $vf0xzw, $vf31y
	vmuly.xzw	$vf0xzw, $vf31xzw, $vf0y
	vmuly.xzw	$vf1xzw, $vf2xzw, $vf3y
	vmuly.xzw	$vf31xzw, $vf0xzw, $vf0y
	vmuly.xzw	$vf31xzw, $vf15xzw, $vf7y
	vmuly.xzw	$vf31xzw, $vf31xzw, $vf31y
	vmuly.y		$vf0y, $vf0y, $vf0y
	vmuly.y		$vf0y, $vf0y, $vf31y
	vmuly.y		$vf0y, $vf31y, $vf0y
	vmuly.y		$vf1y, $vf2y, $vf3y
	vmuly.y		$vf31y, $vf0y, $vf0y
	vmuly.y		$vf31y, $vf15y, $vf7y
	vmuly.y		$vf31y, $vf31y, $vf31y
	vmuly.yw	$vf0yw, $vf0yw, $vf0y
	vmuly.yw	$vf0yw, $vf0yw, $vf31y
	vmuly.yw	$vf0yw, $vf31yw, $vf0y
	vmuly.yw	$vf1yw, $vf2yw, $vf3y
	vmuly.yw	$vf31yw, $vf0yw, $vf0y
	vmuly.yw	$vf31yw, $vf15yw, $vf7y
	vmuly.yw	$vf31yw, $vf31yw, $vf31y
	vmuly.yz	$vf0yz, $vf0yz, $vf0y
	vmuly.yz	$vf0yz, $vf0yz, $vf31y
	vmuly.yz	$vf0yz, $vf31yz, $vf0y
	vmuly.yz	$vf1yz, $vf2yz, $vf3y
	vmuly.yz	$vf31yz, $vf0yz, $vf0y
	vmuly.yz	$vf31yz, $vf15yz, $vf7y
	vmuly.yz	$vf31yz, $vf31yz, $vf31y
	vmuly.yzw	$vf0yzw, $vf0yzw, $vf0y
	vmuly.yzw	$vf0yzw, $vf0yzw, $vf31y
	vmuly.yzw	$vf0yzw, $vf31yzw, $vf0y
	vmuly.yzw	$vf1yzw, $vf2yzw, $vf3y
	vmuly.yzw	$vf31yzw, $vf0yzw, $vf0y
	vmuly.yzw	$vf31yzw, $vf15yzw, $vf7y
	vmuly.yzw	$vf31yzw, $vf31yzw, $vf31y
	vmul.yz		$vf0yz, $vf0yz, $vf0yz
	vmul.yz		$vf0yz, $vf0yz, $vf31yz
	vmul.yz		$vf0yz, $vf31yz, $vf0yz
	vmuly.z		$vf0z, $vf0z, $vf0y
	vmuly.z		$vf0z, $vf0z, $vf31y
	vmuly.z		$vf0z, $vf31z, $vf0y
	vmul.yz		$vf1yz, $vf2yz, $vf3yz
	vmuly.z		$vf1z, $vf2z, $vf3y
	vmul.yz		$vf31yz, $vf0yz, $vf0yz
	vmul.yz		$vf31yz, $vf15yz, $vf7yz
	vmul.yz		$vf31yz, $vf31yz, $vf31yz
	vmuly.z		$vf31z, $vf0z, $vf0y
	vmuly.z		$vf31z, $vf15z, $vf7y
	vmuly.z		$vf31z, $vf31z, $vf31y
	vmul.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vmul.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vmul.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vmuly.zw	$vf0zw, $vf0zw, $vf0y
	vmuly.zw	$vf0zw, $vf0zw, $vf31y
	vmuly.zw	$vf0zw, $vf31zw, $vf0y
	vmul.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vmuly.zw	$vf1zw, $vf2zw, $vf3y
	vmul.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vmul.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vmul.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vmuly.zw	$vf31zw, $vf0zw, $vf0y
	vmuly.zw	$vf31zw, $vf15zw, $vf7y
	vmuly.zw	$vf31zw, $vf31zw, $vf31y
	vmul.z		$vf0z, $vf0z, $vf0z
	vmul.z		$vf0z, $vf0z, $vf31z
	vmul.z		$vf0z, $vf31z, $vf0z
	vmul.z		$vf1z, $vf2z, $vf3z
	vmul.z		$vf31z, $vf0z, $vf0z
	vmul.z		$vf31z, $vf15z, $vf7z
	vmul.z		$vf31z, $vf31z, $vf31z
	vmulz.w		$vf0w, $vf0w, $vf0z
	vmulz.w		$vf0w, $vf0w, $vf31z
	vmulz.w		$vf0w, $vf31w, $vf0z
	vmul.zw		$vf0zw, $vf0zw, $vf0zw
	vmul.zw		$vf0zw, $vf0zw, $vf31zw
	vmul.zw		$vf0zw, $vf31zw, $vf0zw
	vmulz.w		$vf1w, $vf2w, $vf3z
	vmul.zw		$vf1zw, $vf2zw, $vf3zw
	vmulz.w		$vf31w, $vf0w, $vf0z
	vmulz.w		$vf31w, $vf15w, $vf7z
	vmulz.w		$vf31w, $vf31w, $vf31z
	vmul.zw		$vf31zw, $vf0zw, $vf0zw
	vmul.zw		$vf31zw, $vf15zw, $vf7zw
	vmul.zw		$vf31zw, $vf31zw, $vf31zw
	vmulz.x		$vf0x, $vf0x, $vf0z
	vmulz.x		$vf0x, $vf0x, $vf31z
	vmulz.x		$vf0x, $vf31x, $vf0z
	vmulz.x		$vf1x, $vf2x, $vf3z
	vmulz.x		$vf31x, $vf0x, $vf0z
	vmulz.x		$vf31x, $vf15x, $vf7z
	vmulz.x		$vf31x, $vf31x, $vf31z
	vmulz.xw	$vf0xw, $vf0xw, $vf0z
	vmulz.xw	$vf0xw, $vf0xw, $vf31z
	vmulz.xw	$vf0xw, $vf31xw, $vf0z
	vmulz.xw	$vf1xw, $vf2xw, $vf3z
	vmulz.xw	$vf31xw, $vf0xw, $vf0z
	vmulz.xw	$vf31xw, $vf15xw, $vf7z
	vmulz.xw	$vf31xw, $vf31xw, $vf31z
	vmulz.xy	$vf0xy, $vf0xy, $vf0z
	vmulz.xy	$vf0xy, $vf0xy, $vf31z
	vmulz.xy	$vf0xy, $vf31xy, $vf0z
	vmulz.xy	$vf1xy, $vf2xy, $vf3z
	vmulz.xy	$vf31xy, $vf0xy, $vf0z
	vmulz.xy	$vf31xy, $vf15xy, $vf7z
	vmulz.xy	$vf31xy, $vf31xy, $vf31z
	vmulz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vmulz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vmulz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vmulz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vmulz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vmulz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vmulz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vmulz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vmulz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vmulz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vmulz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vmulz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vmulz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vmulz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vmulz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vmulz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vmulz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vmulz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vmulz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vmulz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vmulz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vmulz.xz	$vf0xz, $vf0xz, $vf0z
	vmulz.xz	$vf0xz, $vf0xz, $vf31z
	vmulz.xz	$vf0xz, $vf31xz, $vf0z
	vmulz.xz	$vf1xz, $vf2xz, $vf3z
	vmulz.xz	$vf31xz, $vf0xz, $vf0z
	vmulz.xz	$vf31xz, $vf15xz, $vf7z
	vmulz.xz	$vf31xz, $vf31xz, $vf31z
	vmulz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vmulz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vmulz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vmulz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vmulz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vmulz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vmulz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vmulz.y		$vf0y, $vf0y, $vf0z
	vmulz.y		$vf0y, $vf0y, $vf31z
	vmulz.y		$vf0y, $vf31y, $vf0z
	vmulz.y		$vf1y, $vf2y, $vf3z
	vmulz.y		$vf31y, $vf0y, $vf0z
	vmulz.y		$vf31y, $vf15y, $vf7z
	vmulz.y		$vf31y, $vf31y, $vf31z
	vmulz.yw	$vf0yw, $vf0yw, $vf0z
	vmulz.yw	$vf0yw, $vf0yw, $vf31z
	vmulz.yw	$vf0yw, $vf31yw, $vf0z
	vmulz.yw	$vf1yw, $vf2yw, $vf3z
	vmulz.yw	$vf31yw, $vf0yw, $vf0z
	vmulz.yw	$vf31yw, $vf15yw, $vf7z
	vmulz.yw	$vf31yw, $vf31yw, $vf31z
	vmulz.yz	$vf0yz, $vf0yz, $vf0z
	vmulz.yz	$vf0yz, $vf0yz, $vf31z
	vmulz.yz	$vf0yz, $vf31yz, $vf0z
	vmulz.yz	$vf1yz, $vf2yz, $vf3z
	vmulz.yz	$vf31yz, $vf0yz, $vf0z
	vmulz.yz	$vf31yz, $vf15yz, $vf7z
	vmulz.yz	$vf31yz, $vf31yz, $vf31z
	vmulz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vmulz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vmulz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vmulz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vmulz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vmulz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vmulz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vmulz.z		$vf0z, $vf0z, $vf0z
	vmulz.z		$vf0z, $vf0z, $vf31z
	vmulz.z		$vf0z, $vf31z, $vf0z
	vmulz.z		$vf1z, $vf2z, $vf3z
	vmulz.z		$vf31z, $vf0z, $vf0z
	vmulz.z		$vf31z, $vf15z, $vf7z
	vmulz.z		$vf31z, $vf31z, $vf31z
	vmulz.zw	$vf0zw, $vf0zw, $vf0z
	vmulz.zw	$vf0zw, $vf0zw, $vf31z
	vmulz.zw	$vf0zw, $vf31zw, $vf0z
	vmulz.zw	$vf1zw, $vf2zw, $vf3z
	vmulz.zw	$vf31zw, $vf0zw, $vf0z
	vmulz.zw	$vf31zw, $vf15zw, $vf7z
	vmulz.zw	$vf31zw, $vf31zw, $vf31z
	vnop
	vopmsub		$vf0, $vf0, $vf0
	vopmsub		$vf0, $vf0, $vf31
	vopmsub		$vf0, $vf31, $vf0
	vopmsub		$vf1, $vf2, $vf3
	vopmsub		$vf31, $vf0, $vf0
	vopmsub		$vf31, $vf15, $vf7
	vopmsub		$vf31, $vf31, $vf31
	vopmsub.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vopmsub.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vopmsub.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vopmsub.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vopmsub.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vopmsub.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vopmsub.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vopmula		$ACC, $vf0, $vf0
	vopmula		$ACC, $vf0, $vf31
	vopmula		$ACC, $vf1, $vf2
	vopmula		$ACC, $vf31, $vf0
	vopmula		$ACC, $vf31, $vf15
	vopmula		$ACC, $vf31, $vf31
	vopmula.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vopmula.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vopmula.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vopmula.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vopmula.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vopmula.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vrget.w		$vf0w, $R
	vrget.w		$vf1w, $R
	vrget.w		$vf31w, $R
	vrget.x		$vf0x, $R
	vrget.x		$vf1x, $R
	vrget.x		$vf31x, $R
	vrget.xw	$vf0xw, $R
	vrget.xw	$vf1xw, $R
	vrget.xw	$vf31xw, $R
	vrget.xy	$vf0xy, $R
	vrget.xy	$vf1xy, $R
	vrget.xy	$vf31xy, $R
	vrget.xyw	$vf0xyw, $R
	vrget.xyw	$vf1xyw, $R
	vrget.xyw	$vf31xyw, $R
	vrget.xyz	$vf0xyz, $R
	vrget.xyz	$vf1xyz, $R
	vrget.xyz	$vf31xyz, $R
	vrget.xyzw	$vf0xyzw, $R
	vrget.xyzw	$vf1xyzw, $R
	vrget.xyzw	$vf31xyzw, $R
	vrget.xz	$vf0xz, $R
	vrget.xz	$vf1xz, $R
	vrget.xz	$vf31xz, $R
	vrget.xzw	$vf0xzw, $R
	vrget.xzw	$vf1xzw, $R
	vrget.xzw	$vf31xzw, $R
	vrget.y		$vf0y, $R
	vrget.y		$vf1y, $R
	vrget.y		$vf31y, $R
	vrget.yw	$vf0yw, $R
	vrget.yw	$vf1yw, $R
	vrget.yw	$vf31yw, $R
	vrget.yz	$vf0yz, $R
	vrget.yz	$vf1yz, $R
	vrget.yz	$vf31yz, $R
	vrget.yzw	$vf0yzw, $R
	vrget.yzw	$vf1yzw, $R
	vrget.yzw	$vf31yzw, $R
	vrget.z		$vf0z, $R
	vrget.z		$vf1z, $R
	vrget.z		$vf31z, $R
	vrget.zw	$vf0zw, $R
	vrget.zw	$vf1zw, $R
	vrget.zw	$vf31zw, $R
	vrinit		$R, $vf0w
	vrinit		$R, $vf0x
	vrinit		$R, $vf0z
	vrinit		$R, $vf1z
	vrinit		$R, $vf31x
	vrinit		$R, $vf31y
	vrnext.w	$vf0w, $R
	vrnext.w	$vf1w, $R
	vrnext.w	$vf31w, $R
	vrnext.x	$vf0x, $R
	vrnext.x	$vf1x, $R
	vrnext.x	$vf31x, $R
	vrnext.xw	$vf0xw, $R
	vrnext.xw	$vf1xw, $R
	vrnext.xw	$vf31xw, $R
	vrnext.xy	$vf0xy, $R
	vrnext.xy	$vf1xy, $R
	vrnext.xy	$vf31xy, $R
	vrnext.xyw	$vf0xyw, $R
	vrnext.xyw	$vf1xyw, $R
	vrnext.xyw	$vf31xyw, $R
	vrnext.xyz	$vf0xyz, $R
	vrnext.xyz	$vf1xyz, $R
	vrnext.xyz	$vf31xyz, $R
	vrnext.xyzw	$vf0xyzw, $R
	vrnext.xyzw	$vf1xyzw, $R
	vrnext.xyzw	$vf31xyzw, $R
	vrnext.xz	$vf0xz, $R
	vrnext.xz	$vf1xz, $R
	vrnext.xz	$vf31xz, $R
	vrnext.xzw	$vf0xzw, $R
	vrnext.xzw	$vf1xzw, $R
	vrnext.xzw	$vf31xzw, $R
	vrnext.y	$vf0y, $R
	vrnext.y	$vf1y, $R
	vrnext.y	$vf31y, $R
	vrnext.yw	$vf0yw, $R
	vrnext.yw	$vf1yw, $R
	vrnext.yw	$vf31yw, $R
	vrnext.yz	$vf0yz, $R
	vrnext.yz	$vf1yz, $R
	vrnext.yz	$vf31yz, $R
	vrnext.yzw	$vf0yzw, $R
	vrnext.yzw	$vf1yzw, $R
	vrnext.yzw	$vf31yzw, $R
	vrnext.z	$vf0z, $R
	vrnext.z	$vf1z, $R
	vrnext.z	$vf31z, $R
	vrnext.zw	$vf0zw, $R
	vrnext.zw	$vf1zw, $R
	vrnext.zw	$vf31zw, $R
	vrsqrt		$Q, $vf0w, $vf0z
	vrsqrt		$Q, $vf0x, $vf0x
	vrsqrt		$Q, $vf0z, $vf31y
	vrsqrt		$Q, $vf1z, $vf2z
	vrsqrt		$Q, $vf31x, $vf15w
	vrsqrt		$Q, $vf31x, $vf31y
	vrsqrt		$Q, $vf31y, $vf0w
	vrxor		$R, $vf0w
	vrxor		$R, $vf0x
	vrxor		$R, $vf0z
	vrxor		$R, $vf1z
	vrxor		$R, $vf31x
	vrxor		$R, $vf31y
	vsqd.w		$vf0, (--$vi0)
	vsqd.w		$vf0, (--$vi31)
	vsqd.w		$vf0w, (--$vi0)
	vsqd.w		$vf1, (--$vi2)
	vsqd.w		$vf31, (--$vi0)
	vsqd.w		$vf31, (--$vi15)
	vsqd.w		$vf31, (--$vi31)
	vsqd.x		$vf0, (--$vi0)
	vsqd.x		$vf0, (--$vi31)
	vsqd.x		$vf0x, (--$vi0)
	vsqd.x		$vf1, (--$vi2)
	vsqd.x		$vf31, (--$vi0)
	vsqd.x		$vf31, (--$vi15)
	vsqd.x		$vf31, (--$vi31)
	vsqd.xw		$vf0, (--$vi0)
	vsqd.xw		$vf0, (--$vi31)
	vsqd.xw		$vf0xw, (--$vi0)
	vsqd.xw		$vf1, (--$vi2)
	vsqd.xw		$vf31, (--$vi0)
	vsqd.xw		$vf31, (--$vi15)
	vsqd.xw		$vf31, (--$vi31)
	vsqd.xy		$vf0, (--$vi0)
	vsqd.xy		$vf0, (--$vi31)
	vsqd.xy		$vf0xy, (--$vi0)
	vsqd.xy		$vf1, (--$vi2)
	vsqd.xy		$vf31, (--$vi0)
	vsqd.xy		$vf31, (--$vi15)
	vsqd.xy		$vf31, (--$vi31)
	vsqd.xyw	$vf0, (--$vi0)
	vsqd.xyw	$vf0, (--$vi31)
	vsqd.xyw	$vf0xyw, (--$vi0)
	vsqd.xyw	$vf1, (--$vi2)
	vsqd.xyw	$vf31, (--$vi0)
	vsqd.xyw	$vf31, (--$vi15)
	vsqd.xyw	$vf31, (--$vi31)
	vsqd.xyz	$vf0, (--$vi0)
	vsqd.xyz	$vf0, (--$vi31)
	vsqd.xyz	$vf0xyz, (--$vi0)
	vsqd.xyz	$vf1, (--$vi2)
	vsqd.xyz	$vf31, (--$vi0)
	vsqd.xyz	$vf31, (--$vi15)
	vsqd.xyz	$vf31, (--$vi31)
	vsqd.xyzw	$vf0, (--$vi0)
	vsqd.xyzw	$vf0, (--$vi31)
	vsqd.xyzw	$vf0xyzw, (--$vi0)
	vsqd.xyzw	$vf1, (--$vi2)
	vsqd.xyzw	$vf31, (--$vi0)
	vsqd.xyzw	$vf31, (--$vi15)
	vsqd.xyzw	$vf31, (--$vi31)
	vsqd.xz		$vf0, (--$vi0)
	vsqd.xz		$vf0, (--$vi31)
	vsqd.xz		$vf0xz, (--$vi0)
	vsqd.xz		$vf1, (--$vi2)
	vsqd.xz		$vf31, (--$vi0)
	vsqd.xz		$vf31, (--$vi15)
	vsqd.xz		$vf31, (--$vi31)
	vsqd.xzw	$vf0, (--$vi0)
	vsqd.xzw	$vf0, (--$vi31)
	vsqd.xzw	$vf0xzw, (--$vi0)
	vsqd.xzw	$vf1, (--$vi2)
	vsqd.xzw	$vf31, (--$vi0)
	vsqd.xzw	$vf31, (--$vi15)
	vsqd.xzw	$vf31, (--$vi31)
	vsqd.y		$vf0, (--$vi0)
	vsqd.y		$vf0, (--$vi31)
	vsqd.y		$vf0y, (--$vi0)
	vsqd.y		$vf1, (--$vi2)
	vsqd.y		$vf31, (--$vi0)
	vsqd.y		$vf31, (--$vi15)
	vsqd.y		$vf31, (--$vi31)
	vsqd.yw		$vf0, (--$vi0)
	vsqd.yw		$vf0, (--$vi31)
	vsqd.yw		$vf0yw, (--$vi0)
	vsqd.yw		$vf1, (--$vi2)
	vsqd.yw		$vf31, (--$vi0)
	vsqd.yw		$vf31, (--$vi15)
	vsqd.yw		$vf31, (--$vi31)
	vsqd.yz		$vf0, (--$vi0)
	vsqd.yz		$vf0, (--$vi31)
	vsqd.yz		$vf0yz, (--$vi0)
	vsqd.yz		$vf1, (--$vi2)
	vsqd.yz		$vf31, (--$vi0)
	vsqd.yz		$vf31, (--$vi15)
	vsqd.yz		$vf31, (--$vi31)
	vsqd.yzw	$vf0, (--$vi0)
	vsqd.yzw	$vf0, (--$vi31)
	vsqd.yzw	$vf0yzw, (--$vi0)
	vsqd.yzw	$vf1, (--$vi2)
	vsqd.yzw	$vf31, (--$vi0)
	vsqd.yzw	$vf31, (--$vi15)
	vsqd.yzw	$vf31, (--$vi31)
	vsqd.z		$vf0, (--$vi0)
	vsqd.z		$vf0, (--$vi31)
	vsqd.z		$vf0z, (--$vi0)
	vsqd.z		$vf1, (--$vi2)
	vsqd.z		$vf31, (--$vi0)
	vsqd.z		$vf31, (--$vi15)
	vsqd.z		$vf31, (--$vi31)
	vsqd.zw		$vf0, (--$vi0)
	vsqd.zw		$vf0, (--$vi31)
	vsqd.zw		$vf0zw, (--$vi0)
	vsqd.zw		$vf1, (--$vi2)
	vsqd.zw		$vf31, (--$vi0)
	vsqd.zw		$vf31, (--$vi15)
	vsqd.zw		$vf31, (--$vi31)
	vsqi.w		$vf0, ($vi0++)
	vsqi.w		$vf0, ($vi31++)
	vsqi.w		$vf0w, ($vi0++)
	vsqi.w		$vf1, ($vi2++)
	vsqi.w		$vf31, ($vi0++)
	vsqi.w		$vf31, ($vi15++)
	vsqi.w		$vf31, ($vi31++)
	vsqi.x		$vf0, ($vi0++)
	vsqi.x		$vf0, ($vi31++)
	vsqi.x		$vf0x, ($vi0++)
	vsqi.x		$vf1, ($vi2++)
	vsqi.x		$vf31, ($vi0++)
	vsqi.x		$vf31, ($vi15++)
	vsqi.x		$vf31, ($vi31++)
	vsqi.xw		$vf0, ($vi0++)
	vsqi.xw		$vf0, ($vi31++)
	vsqi.xw		$vf0xw, ($vi0++)
	vsqi.xw		$vf1, ($vi2++)
	vsqi.xw		$vf31, ($vi0++)
	vsqi.xw		$vf31, ($vi15++)
	vsqi.xw		$vf31, ($vi31++)
	vsqi.xy		$vf0, ($vi0++)
	vsqi.xy		$vf0, ($vi31++)
	vsqi.xy		$vf0xy, ($vi0++)
	vsqi.xy		$vf1, ($vi2++)
	vsqi.xy		$vf31, ($vi0++)
	vsqi.xy		$vf31, ($vi15++)
	vsqi.xy		$vf31, ($vi31++)
	vsqi.xyw	$vf0, ($vi0++)
	vsqi.xyw	$vf0, ($vi31++)
	vsqi.xyw	$vf0xyw, ($vi0++)
	vsqi.xyw	$vf1, ($vi2++)
	vsqi.xyw	$vf31, ($vi0++)
	vsqi.xyw	$vf31, ($vi15++)
	vsqi.xyw	$vf31, ($vi31++)
	vsqi.xyz	$vf0, ($vi0++)
	vsqi.xyz	$vf0, ($vi31++)
	vsqi.xyz	$vf0xyz, ($vi0++)
	vsqi.xyz	$vf1, ($vi2++)
	vsqi.xyz	$vf31, ($vi0++)
	vsqi.xyz	$vf31, ($vi15++)
	vsqi.xyz	$vf31, ($vi31++)
	vsqi.xyzw	$vf0, ($vi0++)
	vsqi.xyzw	$vf0, ($vi31++)
	vsqi.xyzw	$vf0xyzw, ($vi0++)
	vsqi.xyzw	$vf1, ($vi2++)
	vsqi.xyzw	$vf31, ($vi0++)
	vsqi.xyzw	$vf31, ($vi15++)
	vsqi.xyzw	$vf31, ($vi31++)
	vsqi.xz		$vf0, ($vi0++)
	vsqi.xz		$vf0, ($vi31++)
	vsqi.xz		$vf0xz, ($vi0++)
	vsqi.xz		$vf1, ($vi2++)
	vsqi.xz		$vf31, ($vi0++)
	vsqi.xz		$vf31, ($vi15++)
	vsqi.xz		$vf31, ($vi31++)
	vsqi.xzw	$vf0, ($vi0++)
	vsqi.xzw	$vf0, ($vi31++)
	vsqi.xzw	$vf0xzw, ($vi0++)
	vsqi.xzw	$vf1, ($vi2++)
	vsqi.xzw	$vf31, ($vi0++)
	vsqi.xzw	$vf31, ($vi15++)
	vsqi.xzw	$vf31, ($vi31++)
	vsqi.y		$vf0, ($vi0++)
	vsqi.y		$vf0, ($vi31++)
	vsqi.y		$vf0y, ($vi0++)
	vsqi.y		$vf1, ($vi2++)
	vsqi.y		$vf31, ($vi0++)
	vsqi.y		$vf31, ($vi15++)
	vsqi.y		$vf31, ($vi31++)
	vsqi.yw		$vf0, ($vi0++)
	vsqi.yw		$vf0, ($vi31++)
	vsqi.yw		$vf0yw, ($vi0++)
	vsqi.yw		$vf1, ($vi2++)
	vsqi.yw		$vf31, ($vi0++)
	vsqi.yw		$vf31, ($vi15++)
	vsqi.yw		$vf31, ($vi31++)
	vsqi.yz		$vf0, ($vi0++)
	vsqi.yz		$vf0, ($vi31++)
	vsqi.yz		$vf0yz, ($vi0++)
	vsqi.yz		$vf1, ($vi2++)
	vsqi.yz		$vf31, ($vi0++)
	vsqi.yz		$vf31, ($vi15++)
	vsqi.yz		$vf31, ($vi31++)
	vsqi.yzw	$vf0, ($vi0++)
	vsqi.yzw	$vf0, ($vi31++)
	vsqi.yzw	$vf0yzw, ($vi0++)
	vsqi.yzw	$vf1, ($vi2++)
	vsqi.yzw	$vf31, ($vi0++)
	vsqi.yzw	$vf31, ($vi15++)
	vsqi.yzw	$vf31, ($vi31++)
	vsqi.z		$vf0, ($vi0++)
	vsqi.z		$vf0, ($vi31++)
	vsqi.z		$vf0z, ($vi0++)
	vsqi.z		$vf1, ($vi2++)
	vsqi.z		$vf31, ($vi0++)
	vsqi.z		$vf31, ($vi15++)
	vsqi.z		$vf31, ($vi31++)
	vsqi.zw		$vf0, ($vi0++)
	vsqi.zw		$vf0, ($vi31++)
	vsqi.zw		$vf0zw, ($vi0++)
	vsqi.zw		$vf1, ($vi2++)
	vsqi.zw		$vf31, ($vi0++)
	vsqi.zw		$vf31, ($vi15++)
	vsqi.zw		$vf31, ($vi31++)
	vsqrt		$Q, $vf0x
	vsqrt		$Q, $vf0y
	vsqrt		$Q, $vf0z
	vsqrt		$Q, $vf1z
	vsqrt		$Q, $vf31w
	vsqrt		$Q, $vf31y
	vsubai.w	$ACCw, $vf0w, $I
	vsubai.w	$ACCw, $vf1w, $I
	vsubai.w	$ACCw, $vf31w, $I
	vsubai.x	$ACCx, $vf0x, $I
	vsubai.x	$ACCx, $vf1x, $I
	vsubai.x	$ACCx, $vf31x, $I
	vsubai.xw	$ACCxw, $vf0xw, $I
	vsubai.xw	$ACCxw, $vf1xw, $I
	vsubai.xw	$ACCxw, $vf31xw, $I
	vsubai.xy	$ACCxy, $vf0xy, $I
	vsubai.xy	$ACCxy, $vf1xy, $I
	vsubai.xy	$ACCxy, $vf31xy, $I
	vsubai.xyw	$ACCxyw, $vf0xyw, $I
	vsubai.xyw	$ACCxyw, $vf1xyw, $I
	vsubai.xyw	$ACCxyw, $vf31xyw, $I
	vsubai.xyz	$ACCxyz, $vf0xyz, $I
	vsubai.xyz	$ACCxyz, $vf1xyz, $I
	vsubai.xyz	$ACCxyz, $vf31xyz, $I
	vsubai.xyzw	$ACCxyzw, $vf0xyzw, $I
	vsubai.xyzw	$ACCxyzw, $vf1xyzw, $I
	vsubai.xyzw	$ACCxyzw, $vf31xyzw, $I
	vsubai.xz	$ACCxz, $vf0xz, $I
	vsubai.xz	$ACCxz, $vf1xz, $I
	vsubai.xz	$ACCxz, $vf31xz, $I
	vsubai.xzw	$ACCxzw, $vf0xzw, $I
	vsubai.xzw	$ACCxzw, $vf1xzw, $I
	vsubai.xzw	$ACCxzw, $vf31xzw, $I
	vsubai.y	$ACCy, $vf0y, $I
	vsubai.y	$ACCy, $vf1y, $I
	vsubai.y	$ACCy, $vf31y, $I
	vsubai.yw	$ACCyw, $vf0yw, $I
	vsubai.yw	$ACCyw, $vf1yw, $I
	vsubai.yw	$ACCyw, $vf31yw, $I
	vsubai.yz	$ACCyz, $vf0yz, $I
	vsubai.yz	$ACCyz, $vf1yz, $I
	vsubai.yz	$ACCyz, $vf31yz, $I
	vsubai.yzw	$ACCyzw, $vf0yzw, $I
	vsubai.yzw	$ACCyzw, $vf1yzw, $I
	vsubai.yzw	$ACCyzw, $vf31yzw, $I
	vsubai.z	$ACCz, $vf0z, $I
	vsubai.z	$ACCz, $vf1z, $I
	vsubai.z	$ACCz, $vf31z, $I
	vsubai.zw	$ACCzw, $vf0zw, $I
	vsubai.zw	$ACCzw, $vf1zw, $I
	vsubai.zw	$ACCzw, $vf31zw, $I
	vsubaq.w	$ACCw, $vf0w, $Q
	vsubaq.w	$ACCw, $vf1w, $Q
	vsubaq.w	$ACCw, $vf31w, $Q
	vsubaq.x	$ACCx, $vf0x, $Q
	vsubaq.x	$ACCx, $vf1x, $Q
	vsubaq.x	$ACCx, $vf31x, $Q
	vsubaq.xw	$ACCxw, $vf0xw, $Q
	vsubaq.xw	$ACCxw, $vf1xw, $Q
	vsubaq.xw	$ACCxw, $vf31xw, $Q
	vsubaq.xy	$ACCxy, $vf0xy, $Q
	vsubaq.xy	$ACCxy, $vf1xy, $Q
	vsubaq.xy	$ACCxy, $vf31xy, $Q
	vsubaq.xyw	$ACCxyw, $vf0xyw, $Q
	vsubaq.xyw	$ACCxyw, $vf1xyw, $Q
	vsubaq.xyw	$ACCxyw, $vf31xyw, $Q
	vsubaq.xyz	$ACCxyz, $vf0xyz, $Q
	vsubaq.xyz	$ACCxyz, $vf1xyz, $Q
	vsubaq.xyz	$ACCxyz, $vf31xyz, $Q
	vsubaq.xyzw	$ACCxyzw, $vf0xyzw, $Q
	vsubaq.xyzw	$ACCxyzw, $vf1xyzw, $Q
	vsubaq.xyzw	$ACCxyzw, $vf31xyzw, $Q
	vsubaq.xz	$ACCxz, $vf0xz, $Q
	vsubaq.xz	$ACCxz, $vf1xz, $Q
	vsubaq.xz	$ACCxz, $vf31xz, $Q
	vsubaq.xzw	$ACCxzw, $vf0xzw, $Q
	vsubaq.xzw	$ACCxzw, $vf1xzw, $Q
	vsubaq.xzw	$ACCxzw, $vf31xzw, $Q
	vsubaq.y	$ACCy, $vf0y, $Q
	vsubaq.y	$ACCy, $vf1y, $Q
	vsubaq.y	$ACCy, $vf31y, $Q
	vsubaq.yw	$ACCyw, $vf0yw, $Q
	vsubaq.yw	$ACCyw, $vf1yw, $Q
	vsubaq.yw	$ACCyw, $vf31yw, $Q
	vsubaq.yz	$ACCyz, $vf0yz, $Q
	vsubaq.yz	$ACCyz, $vf1yz, $Q
	vsubaq.yz	$ACCyz, $vf31yz, $Q
	vsubaq.yzw	$ACCyzw, $vf0yzw, $Q
	vsubaq.yzw	$ACCyzw, $vf1yzw, $Q
	vsubaq.yzw	$ACCyzw, $vf31yzw, $Q
	vsubaq.z	$ACCz, $vf0z, $Q
	vsubaq.z	$ACCz, $vf1z, $Q
	vsubaq.z	$ACCz, $vf31z, $Q
	vsubaq.zw	$ACCzw, $vf0zw, $Q
	vsubaq.zw	$ACCzw, $vf1zw, $Q
	vsubaq.zw	$ACCzw, $vf31zw, $Q
	vsuba.w		$ACCw, $vf0w, $vf0w
	vsuba.w		$ACCw, $vf0w, $vf31w
	vsuba.w		$ACCw, $vf1w, $vf2w
	vsuba.w		$ACCw, $vf31w, $vf0w
	vsuba.w		$ACCw, $vf31w, $vf15w
	vsuba.w		$ACCw, $vf31w, $vf31w
	vsubaw.w	$ACCw, $vf0w, $vf0w
	vsubaw.w	$ACCw, $vf0w, $vf31w
	vsubaw.w	$ACCw, $vf1w, $vf2w
	vsubaw.w	$ACCw, $vf31w, $vf0w
	vsubaw.w	$ACCw, $vf31w, $vf15w
	vsubaw.w	$ACCw, $vf31w, $vf31w
	vsubaw.x	$ACCx, $vf0x, $vf0w
	vsubaw.x	$ACCx, $vf0x, $vf31w
	vsubaw.x	$ACCx, $vf1x, $vf2w
	vsubaw.x	$ACCx, $vf31x, $vf0w
	vsubaw.x	$ACCx, $vf31x, $vf15w
	vsubaw.x	$ACCx, $vf31x, $vf31w
	vsubaw.xw	$ACCxw, $vf0xw, $vf0w
	vsubaw.xw	$ACCxw, $vf0xw, $vf31w
	vsubaw.xw	$ACCxw, $vf1xw, $vf2w
	vsubaw.xw	$ACCxw, $vf31xw, $vf0w
	vsubaw.xw	$ACCxw, $vf31xw, $vf15w
	vsubaw.xw	$ACCxw, $vf31xw, $vf31w
	vsubaw.xy	$ACCxy, $vf0xy, $vf0w
	vsubaw.xy	$ACCxy, $vf0xy, $vf31w
	vsubaw.xy	$ACCxy, $vf1xy, $vf2w
	vsubaw.xy	$ACCxy, $vf31xy, $vf0w
	vsubaw.xy	$ACCxy, $vf31xy, $vf15w
	vsubaw.xy	$ACCxy, $vf31xy, $vf31w
	vsubaw.xyw	$ACCxyw, $vf0xyw, $vf0w
	vsubaw.xyw	$ACCxyw, $vf0xyw, $vf31w
	vsubaw.xyw	$ACCxyw, $vf1xyw, $vf2w
	vsubaw.xyw	$ACCxyw, $vf31xyw, $vf0w
	vsubaw.xyw	$ACCxyw, $vf31xyw, $vf15w
	vsubaw.xyw	$ACCxyw, $vf31xyw, $vf31w
	vsubaw.xyz	$ACCxyz, $vf0xyz, $vf0w
	vsubaw.xyz	$ACCxyz, $vf0xyz, $vf31w
	vsubaw.xyz	$ACCxyz, $vf1xyz, $vf2w
	vsubaw.xyz	$ACCxyz, $vf31xyz, $vf0w
	vsubaw.xyz	$ACCxyz, $vf31xyz, $vf15w
	vsubaw.xyz	$ACCxyz, $vf31xyz, $vf31w
	vsubaw.xyzw	$ACCxyzw, $vf0xyzw, $vf0w
	vsubaw.xyzw	$ACCxyzw, $vf0xyzw, $vf31w
	vsubaw.xyzw	$ACCxyzw, $vf1xyzw, $vf2w
	vsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf0w
	vsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf15w
	vsubaw.xyzw	$ACCxyzw, $vf31xyzw, $vf31w
	vsubaw.xz	$ACCxz, $vf0xz, $vf0w
	vsubaw.xz	$ACCxz, $vf0xz, $vf31w
	vsubaw.xz	$ACCxz, $vf1xz, $vf2w
	vsubaw.xz	$ACCxz, $vf31xz, $vf0w
	vsubaw.xz	$ACCxz, $vf31xz, $vf15w
	vsubaw.xz	$ACCxz, $vf31xz, $vf31w
	vsubaw.xzw	$ACCxzw, $vf0xzw, $vf0w
	vsubaw.xzw	$ACCxzw, $vf0xzw, $vf31w
	vsubaw.xzw	$ACCxzw, $vf1xzw, $vf2w
	vsubaw.xzw	$ACCxzw, $vf31xzw, $vf0w
	vsubaw.xzw	$ACCxzw, $vf31xzw, $vf15w
	vsubaw.xzw	$ACCxzw, $vf31xzw, $vf31w
	vsubaw.y	$ACCy, $vf0y, $vf0w
	vsubaw.y	$ACCy, $vf0y, $vf31w
	vsubaw.y	$ACCy, $vf1y, $vf2w
	vsubaw.y	$ACCy, $vf31y, $vf0w
	vsubaw.y	$ACCy, $vf31y, $vf15w
	vsubaw.y	$ACCy, $vf31y, $vf31w
	vsubaw.yw	$ACCyw, $vf0yw, $vf0w
	vsubaw.yw	$ACCyw, $vf0yw, $vf31w
	vsubaw.yw	$ACCyw, $vf1yw, $vf2w
	vsubaw.yw	$ACCyw, $vf31yw, $vf0w
	vsubaw.yw	$ACCyw, $vf31yw, $vf15w
	vsubaw.yw	$ACCyw, $vf31yw, $vf31w
	vsubaw.yz	$ACCyz, $vf0yz, $vf0w
	vsubaw.yz	$ACCyz, $vf0yz, $vf31w
	vsubaw.yz	$ACCyz, $vf1yz, $vf2w
	vsubaw.yz	$ACCyz, $vf31yz, $vf0w
	vsubaw.yz	$ACCyz, $vf31yz, $vf15w
	vsubaw.yz	$ACCyz, $vf31yz, $vf31w
	vsubaw.yzw	$ACCyzw, $vf0yzw, $vf0w
	vsubaw.yzw	$ACCyzw, $vf0yzw, $vf31w
	vsubaw.yzw	$ACCyzw, $vf1yzw, $vf2w
	vsubaw.yzw	$ACCyzw, $vf31yzw, $vf0w
	vsubaw.yzw	$ACCyzw, $vf31yzw, $vf15w
	vsubaw.yzw	$ACCyzw, $vf31yzw, $vf31w
	vsubaw.z	$ACCz, $vf0z, $vf0w
	vsubaw.z	$ACCz, $vf0z, $vf31w
	vsubaw.z	$ACCz, $vf1z, $vf2w
	vsubaw.z	$ACCz, $vf31z, $vf0w
	vsubaw.z	$ACCz, $vf31z, $vf15w
	vsubaw.z	$ACCz, $vf31z, $vf31w
	vsubaw.zw	$ACCzw, $vf0zw, $vf0w
	vsubaw.zw	$ACCzw, $vf0zw, $vf31w
	vsubaw.zw	$ACCzw, $vf1zw, $vf2w
	vsubaw.zw	$ACCzw, $vf31zw, $vf0w
	vsubaw.zw	$ACCzw, $vf31zw, $vf15w
	vsubaw.zw	$ACCzw, $vf31zw, $vf31w
	vsuba.x		$ACCx, $vf0x, $vf0x
	vsuba.x		$ACCx, $vf0x, $vf31x
	vsuba.x		$ACCx, $vf1x, $vf2x
	vsuba.x		$ACCx, $vf31x, $vf0x
	vsuba.x		$ACCx, $vf31x, $vf15x
	vsuba.x		$ACCx, $vf31x, $vf31x
	vsubax.w	$ACCw, $vf0w, $vf0x
	vsubax.w	$ACCw, $vf0w, $vf31x
	vsubax.w	$ACCw, $vf1w, $vf2x
	vsubax.w	$ACCw, $vf31w, $vf0x
	vsubax.w	$ACCw, $vf31w, $vf15x
	vsubax.w	$ACCw, $vf31w, $vf31x
	vsuba.xw	$ACCxw, $vf0xw, $vf0xw
	vsuba.xw	$ACCxw, $vf0xw, $vf31xw
	vsuba.xw	$ACCxw, $vf1xw, $vf2xw
	vsuba.xw	$ACCxw, $vf31xw, $vf0xw
	vsuba.xw	$ACCxw, $vf31xw, $vf15xw
	vsuba.xw	$ACCxw, $vf31xw, $vf31xw
	vsubax.x	$ACCx, $vf0x, $vf0x
	vsubax.x	$ACCx, $vf0x, $vf31x
	vsubax.x	$ACCx, $vf1x, $vf2x
	vsubax.x	$ACCx, $vf31x, $vf0x
	vsubax.x	$ACCx, $vf31x, $vf15x
	vsubax.x	$ACCx, $vf31x, $vf31x
	vsubax.xw	$ACCxw, $vf0xw, $vf0x
	vsubax.xw	$ACCxw, $vf0xw, $vf31x
	vsubax.xw	$ACCxw, $vf1xw, $vf2x
	vsubax.xw	$ACCxw, $vf31xw, $vf0x
	vsubax.xw	$ACCxw, $vf31xw, $vf15x
	vsubax.xw	$ACCxw, $vf31xw, $vf31x
	vsubax.xy	$ACCxy, $vf0xy, $vf0x
	vsubax.xy	$ACCxy, $vf0xy, $vf31x
	vsubax.xy	$ACCxy, $vf1xy, $vf2x
	vsubax.xy	$ACCxy, $vf31xy, $vf0x
	vsubax.xy	$ACCxy, $vf31xy, $vf15x
	vsubax.xy	$ACCxy, $vf31xy, $vf31x
	vsubax.xyw	$ACCxyw, $vf0xyw, $vf0x
	vsubax.xyw	$ACCxyw, $vf0xyw, $vf31x
	vsubax.xyw	$ACCxyw, $vf1xyw, $vf2x
	vsubax.xyw	$ACCxyw, $vf31xyw, $vf0x
	vsubax.xyw	$ACCxyw, $vf31xyw, $vf15x
	vsubax.xyw	$ACCxyw, $vf31xyw, $vf31x
	vsubax.xyz	$ACCxyz, $vf0xyz, $vf0x
	vsubax.xyz	$ACCxyz, $vf0xyz, $vf31x
	vsubax.xyz	$ACCxyz, $vf1xyz, $vf2x
	vsubax.xyz	$ACCxyz, $vf31xyz, $vf0x
	vsubax.xyz	$ACCxyz, $vf31xyz, $vf15x
	vsubax.xyz	$ACCxyz, $vf31xyz, $vf31x
	vsubax.xyzw	$ACCxyzw, $vf0xyzw, $vf0x
	vsubax.xyzw	$ACCxyzw, $vf0xyzw, $vf31x
	vsubax.xyzw	$ACCxyzw, $vf1xyzw, $vf2x
	vsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf0x
	vsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf15x
	vsubax.xyzw	$ACCxyzw, $vf31xyzw, $vf31x
	vsubax.xz	$ACCxz, $vf0xz, $vf0x
	vsubax.xz	$ACCxz, $vf0xz, $vf31x
	vsubax.xz	$ACCxz, $vf1xz, $vf2x
	vsubax.xz	$ACCxz, $vf31xz, $vf0x
	vsubax.xz	$ACCxz, $vf31xz, $vf15x
	vsubax.xz	$ACCxz, $vf31xz, $vf31x
	vsubax.xzw	$ACCxzw, $vf0xzw, $vf0x
	vsubax.xzw	$ACCxzw, $vf0xzw, $vf31x
	vsubax.xzw	$ACCxzw, $vf1xzw, $vf2x
	vsubax.xzw	$ACCxzw, $vf31xzw, $vf0x
	vsubax.xzw	$ACCxzw, $vf31xzw, $vf15x
	vsubax.xzw	$ACCxzw, $vf31xzw, $vf31x
	vsuba.xy	$ACCxy, $vf0xy, $vf0xy
	vsuba.xy	$ACCxy, $vf0xy, $vf31xy
	vsuba.xy	$ACCxy, $vf1xy, $vf2xy
	vsuba.xy	$ACCxy, $vf31xy, $vf0xy
	vsuba.xy	$ACCxy, $vf31xy, $vf15xy
	vsuba.xy	$ACCxy, $vf31xy, $vf31xy
	vsubax.y	$ACCy, $vf0y, $vf0x
	vsubax.y	$ACCy, $vf0y, $vf31x
	vsubax.y	$ACCy, $vf1y, $vf2x
	vsubax.y	$ACCy, $vf31y, $vf0x
	vsubax.y	$ACCy, $vf31y, $vf15x
	vsubax.y	$ACCy, $vf31y, $vf31x
	vsuba.xyw	$ACCxyw, $vf0xyw, $vf0xyw
	vsuba.xyw	$ACCxyw, $vf0xyw, $vf31xyw
	vsuba.xyw	$ACCxyw, $vf1xyw, $vf2xyw
	vsuba.xyw	$ACCxyw, $vf31xyw, $vf0xyw
	vsuba.xyw	$ACCxyw, $vf31xyw, $vf15xyw
	vsuba.xyw	$ACCxyw, $vf31xyw, $vf31xyw
	vsubax.yw	$ACCyw, $vf0yw, $vf0x
	vsubax.yw	$ACCyw, $vf0yw, $vf31x
	vsubax.yw	$ACCyw, $vf1yw, $vf2x
	vsubax.yw	$ACCyw, $vf31yw, $vf0x
	vsubax.yw	$ACCyw, $vf31yw, $vf15x
	vsubax.yw	$ACCyw, $vf31yw, $vf31x
	vsuba.xyz	$ACCxyz, $vf0xyz, $vf0xyz
	vsuba.xyz	$ACCxyz, $vf0xyz, $vf31xyz
	vsuba.xyz	$ACCxyz, $vf1xyz, $vf2xyz
	vsuba.xyz	$ACCxyz, $vf31xyz, $vf0xyz
	vsuba.xyz	$ACCxyz, $vf31xyz, $vf15xyz
	vsuba.xyz	$ACCxyz, $vf31xyz, $vf31xyz
	vsubax.yz	$ACCyz, $vf0yz, $vf0x
	vsubax.yz	$ACCyz, $vf0yz, $vf31x
	vsubax.yz	$ACCyz, $vf1yz, $vf2x
	vsubax.yz	$ACCyz, $vf31yz, $vf0x
	vsubax.yz	$ACCyz, $vf31yz, $vf15x
	vsubax.yz	$ACCyz, $vf31yz, $vf31x
	vsuba.xyzw	$ACCxyzw, $vf0xyzw, $vf0xyzw
	vsuba.xyzw	$ACCxyzw, $vf0xyzw, $vf31xyzw
	vsuba.xyzw	$ACCxyzw, $vf1xyzw, $vf2xyzw
	vsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf0xyzw
	vsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf15xyzw
	vsuba.xyzw	$ACCxyzw, $vf31xyzw, $vf31xyzw
	vsubax.yzw	$ACCyzw, $vf0yzw, $vf0x
	vsubax.yzw	$ACCyzw, $vf0yzw, $vf31x
	vsubax.yzw	$ACCyzw, $vf1yzw, $vf2x
	vsubax.yzw	$ACCyzw, $vf31yzw, $vf0x
	vsubax.yzw	$ACCyzw, $vf31yzw, $vf15x
	vsubax.yzw	$ACCyzw, $vf31yzw, $vf31x
	vsuba.xz	$ACCxz, $vf0xz, $vf0xz
	vsuba.xz	$ACCxz, $vf0xz, $vf31xz
	vsuba.xz	$ACCxz, $vf1xz, $vf2xz
	vsuba.xz	$ACCxz, $vf31xz, $vf0xz
	vsuba.xz	$ACCxz, $vf31xz, $vf15xz
	vsuba.xz	$ACCxz, $vf31xz, $vf31xz
	vsubax.z	$ACCz, $vf0z, $vf0x
	vsubax.z	$ACCz, $vf0z, $vf31x
	vsubax.z	$ACCz, $vf1z, $vf2x
	vsubax.z	$ACCz, $vf31z, $vf0x
	vsubax.z	$ACCz, $vf31z, $vf15x
	vsubax.z	$ACCz, $vf31z, $vf31x
	vsuba.xzw	$ACCxzw, $vf0xzw, $vf0xzw
	vsuba.xzw	$ACCxzw, $vf0xzw, $vf31xzw
	vsuba.xzw	$ACCxzw, $vf1xzw, $vf2xzw
	vsuba.xzw	$ACCxzw, $vf31xzw, $vf0xzw
	vsuba.xzw	$ACCxzw, $vf31xzw, $vf15xzw
	vsuba.xzw	$ACCxzw, $vf31xzw, $vf31xzw
	vsubax.zw	$ACCzw, $vf0zw, $vf0x
	vsubax.zw	$ACCzw, $vf0zw, $vf31x
	vsubax.zw	$ACCzw, $vf1zw, $vf2x
	vsubax.zw	$ACCzw, $vf31zw, $vf0x
	vsubax.zw	$ACCzw, $vf31zw, $vf15x
	vsubax.zw	$ACCzw, $vf31zw, $vf31x
	vsuba.y		$ACCy, $vf0y, $vf0y
	vsuba.y		$ACCy, $vf0y, $vf31y
	vsuba.y		$ACCy, $vf1y, $vf2y
	vsuba.y		$ACCy, $vf31y, $vf0y
	vsuba.y		$ACCy, $vf31y, $vf15y
	vsuba.y		$ACCy, $vf31y, $vf31y
	vsubay.w	$ACCw, $vf0w, $vf0y
	vsubay.w	$ACCw, $vf0w, $vf31y
	vsubay.w	$ACCw, $vf1w, $vf2y
	vsubay.w	$ACCw, $vf31w, $vf0y
	vsubay.w	$ACCw, $vf31w, $vf15y
	vsubay.w	$ACCw, $vf31w, $vf31y
	vsuba.yw	$ACCyw, $vf0yw, $vf0yw
	vsuba.yw	$ACCyw, $vf0yw, $vf31yw
	vsuba.yw	$ACCyw, $vf1yw, $vf2yw
	vsuba.yw	$ACCyw, $vf31yw, $vf0yw
	vsuba.yw	$ACCyw, $vf31yw, $vf15yw
	vsuba.yw	$ACCyw, $vf31yw, $vf31yw
	vsubay.x	$ACCx, $vf0x, $vf0y
	vsubay.x	$ACCx, $vf0x, $vf31y
	vsubay.x	$ACCx, $vf1x, $vf2y
	vsubay.x	$ACCx, $vf31x, $vf0y
	vsubay.x	$ACCx, $vf31x, $vf15y
	vsubay.x	$ACCx, $vf31x, $vf31y
	vsubay.xw	$ACCxw, $vf0xw, $vf0y
	vsubay.xw	$ACCxw, $vf0xw, $vf31y
	vsubay.xw	$ACCxw, $vf1xw, $vf2y
	vsubay.xw	$ACCxw, $vf31xw, $vf0y
	vsubay.xw	$ACCxw, $vf31xw, $vf15y
	vsubay.xw	$ACCxw, $vf31xw, $vf31y
	vsubay.xy	$ACCxy, $vf0xy, $vf0y
	vsubay.xy	$ACCxy, $vf0xy, $vf31y
	vsubay.xy	$ACCxy, $vf1xy, $vf2y
	vsubay.xy	$ACCxy, $vf31xy, $vf0y
	vsubay.xy	$ACCxy, $vf31xy, $vf15y
	vsubay.xy	$ACCxy, $vf31xy, $vf31y
	vsubay.xyw	$ACCxyw, $vf0xyw, $vf0y
	vsubay.xyw	$ACCxyw, $vf0xyw, $vf31y
	vsubay.xyw	$ACCxyw, $vf1xyw, $vf2y
	vsubay.xyw	$ACCxyw, $vf31xyw, $vf0y
	vsubay.xyw	$ACCxyw, $vf31xyw, $vf15y
	vsubay.xyw	$ACCxyw, $vf31xyw, $vf31y
	vsubay.xyz	$ACCxyz, $vf0xyz, $vf0y
	vsubay.xyz	$ACCxyz, $vf0xyz, $vf31y
	vsubay.xyz	$ACCxyz, $vf1xyz, $vf2y
	vsubay.xyz	$ACCxyz, $vf31xyz, $vf0y
	vsubay.xyz	$ACCxyz, $vf31xyz, $vf15y
	vsubay.xyz	$ACCxyz, $vf31xyz, $vf31y
	vsubay.xyzw	$ACCxyzw, $vf0xyzw, $vf0y
	vsubay.xyzw	$ACCxyzw, $vf0xyzw, $vf31y
	vsubay.xyzw	$ACCxyzw, $vf1xyzw, $vf2y
	vsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf0y
	vsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf15y
	vsubay.xyzw	$ACCxyzw, $vf31xyzw, $vf31y
	vsubay.xz	$ACCxz, $vf0xz, $vf0y
	vsubay.xz	$ACCxz, $vf0xz, $vf31y
	vsubay.xz	$ACCxz, $vf1xz, $vf2y
	vsubay.xz	$ACCxz, $vf31xz, $vf0y
	vsubay.xz	$ACCxz, $vf31xz, $vf15y
	vsubay.xz	$ACCxz, $vf31xz, $vf31y
	vsubay.xzw	$ACCxzw, $vf0xzw, $vf0y
	vsubay.xzw	$ACCxzw, $vf0xzw, $vf31y
	vsubay.xzw	$ACCxzw, $vf1xzw, $vf2y
	vsubay.xzw	$ACCxzw, $vf31xzw, $vf0y
	vsubay.xzw	$ACCxzw, $vf31xzw, $vf15y
	vsubay.xzw	$ACCxzw, $vf31xzw, $vf31y
	vsubay.y	$ACCy, $vf0y, $vf0y
	vsubay.y	$ACCy, $vf0y, $vf31y
	vsubay.y	$ACCy, $vf1y, $vf2y
	vsubay.y	$ACCy, $vf31y, $vf0y
	vsubay.y	$ACCy, $vf31y, $vf15y
	vsubay.y	$ACCy, $vf31y, $vf31y
	vsubay.yw	$ACCyw, $vf0yw, $vf0y
	vsubay.yw	$ACCyw, $vf0yw, $vf31y
	vsubay.yw	$ACCyw, $vf1yw, $vf2y
	vsubay.yw	$ACCyw, $vf31yw, $vf0y
	vsubay.yw	$ACCyw, $vf31yw, $vf15y
	vsubay.yw	$ACCyw, $vf31yw, $vf31y
	vsubay.yz	$ACCyz, $vf0yz, $vf0y
	vsubay.yz	$ACCyz, $vf0yz, $vf31y
	vsubay.yz	$ACCyz, $vf1yz, $vf2y
	vsubay.yz	$ACCyz, $vf31yz, $vf0y
	vsubay.yz	$ACCyz, $vf31yz, $vf15y
	vsubay.yz	$ACCyz, $vf31yz, $vf31y
	vsubay.yzw	$ACCyzw, $vf0yzw, $vf0y
	vsubay.yzw	$ACCyzw, $vf0yzw, $vf31y
	vsubay.yzw	$ACCyzw, $vf1yzw, $vf2y
	vsubay.yzw	$ACCyzw, $vf31yzw, $vf0y
	vsubay.yzw	$ACCyzw, $vf31yzw, $vf15y
	vsubay.yzw	$ACCyzw, $vf31yzw, $vf31y
	vsuba.yz	$ACCyz, $vf0yz, $vf0yz
	vsuba.yz	$ACCyz, $vf0yz, $vf31yz
	vsuba.yz	$ACCyz, $vf1yz, $vf2yz
	vsuba.yz	$ACCyz, $vf31yz, $vf0yz
	vsuba.yz	$ACCyz, $vf31yz, $vf15yz
	vsuba.yz	$ACCyz, $vf31yz, $vf31yz
	vsubay.z	$ACCz, $vf0z, $vf0y
	vsubay.z	$ACCz, $vf0z, $vf31y
	vsubay.z	$ACCz, $vf1z, $vf2y
	vsubay.z	$ACCz, $vf31z, $vf0y
	vsubay.z	$ACCz, $vf31z, $vf15y
	vsubay.z	$ACCz, $vf31z, $vf31y
	vsuba.yzw	$ACCyzw, $vf0yzw, $vf0yzw
	vsuba.yzw	$ACCyzw, $vf0yzw, $vf31yzw
	vsuba.yzw	$ACCyzw, $vf1yzw, $vf2yzw
	vsuba.yzw	$ACCyzw, $vf31yzw, $vf0yzw
	vsuba.yzw	$ACCyzw, $vf31yzw, $vf15yzw
	vsuba.yzw	$ACCyzw, $vf31yzw, $vf31yzw
	vsubay.zw	$ACCzw, $vf0zw, $vf0y
	vsubay.zw	$ACCzw, $vf0zw, $vf31y
	vsubay.zw	$ACCzw, $vf1zw, $vf2y
	vsubay.zw	$ACCzw, $vf31zw, $vf0y
	vsubay.zw	$ACCzw, $vf31zw, $vf15y
	vsubay.zw	$ACCzw, $vf31zw, $vf31y
	vsuba.z		$ACCz, $vf0z, $vf0z
	vsuba.z		$ACCz, $vf0z, $vf31z
	vsuba.z		$ACCz, $vf1z, $vf2z
	vsuba.z		$ACCz, $vf31z, $vf0z
	vsuba.z		$ACCz, $vf31z, $vf15z
	vsuba.z		$ACCz, $vf31z, $vf31z
	vsubaz.w	$ACCw, $vf0w, $vf0z
	vsubaz.w	$ACCw, $vf0w, $vf31z
	vsubaz.w	$ACCw, $vf1w, $vf2z
	vsubaz.w	$ACCw, $vf31w, $vf0z
	vsubaz.w	$ACCw, $vf31w, $vf15z
	vsubaz.w	$ACCw, $vf31w, $vf31z
	vsuba.zw	$ACCzw, $vf0zw, $vf0zw
	vsuba.zw	$ACCzw, $vf0zw, $vf31zw
	vsuba.zw	$ACCzw, $vf1zw, $vf2zw
	vsuba.zw	$ACCzw, $vf31zw, $vf0zw
	vsuba.zw	$ACCzw, $vf31zw, $vf15zw
	vsuba.zw	$ACCzw, $vf31zw, $vf31zw
	vsubaz.x	$ACCx, $vf0x, $vf0z
	vsubaz.x	$ACCx, $vf0x, $vf31z
	vsubaz.x	$ACCx, $vf1x, $vf2z
	vsubaz.x	$ACCx, $vf31x, $vf0z
	vsubaz.x	$ACCx, $vf31x, $vf15z
	vsubaz.x	$ACCx, $vf31x, $vf31z
	vsubaz.xw	$ACCxw, $vf0xw, $vf0z
	vsubaz.xw	$ACCxw, $vf0xw, $vf31z
	vsubaz.xw	$ACCxw, $vf1xw, $vf2z
	vsubaz.xw	$ACCxw, $vf31xw, $vf0z
	vsubaz.xw	$ACCxw, $vf31xw, $vf15z
	vsubaz.xw	$ACCxw, $vf31xw, $vf31z
	vsubaz.xy	$ACCxy, $vf0xy, $vf0z
	vsubaz.xy	$ACCxy, $vf0xy, $vf31z
	vsubaz.xy	$ACCxy, $vf1xy, $vf2z
	vsubaz.xy	$ACCxy, $vf31xy, $vf0z
	vsubaz.xy	$ACCxy, $vf31xy, $vf15z
	vsubaz.xy	$ACCxy, $vf31xy, $vf31z
	vsubaz.xyw	$ACCxyw, $vf0xyw, $vf0z
	vsubaz.xyw	$ACCxyw, $vf0xyw, $vf31z
	vsubaz.xyw	$ACCxyw, $vf1xyw, $vf2z
	vsubaz.xyw	$ACCxyw, $vf31xyw, $vf0z
	vsubaz.xyw	$ACCxyw, $vf31xyw, $vf15z
	vsubaz.xyw	$ACCxyw, $vf31xyw, $vf31z
	vsubaz.xyz	$ACCxyz, $vf0xyz, $vf0z
	vsubaz.xyz	$ACCxyz, $vf0xyz, $vf31z
	vsubaz.xyz	$ACCxyz, $vf1xyz, $vf2z
	vsubaz.xyz	$ACCxyz, $vf31xyz, $vf0z
	vsubaz.xyz	$ACCxyz, $vf31xyz, $vf15z
	vsubaz.xyz	$ACCxyz, $vf31xyz, $vf31z
	vsubaz.xyzw	$ACCxyzw, $vf0xyzw, $vf0z
	vsubaz.xyzw	$ACCxyzw, $vf0xyzw, $vf31z
	vsubaz.xyzw	$ACCxyzw, $vf1xyzw, $vf2z
	vsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf0z
	vsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf15z
	vsubaz.xyzw	$ACCxyzw, $vf31xyzw, $vf31z
	vsubaz.xz	$ACCxz, $vf0xz, $vf0z
	vsubaz.xz	$ACCxz, $vf0xz, $vf31z
	vsubaz.xz	$ACCxz, $vf1xz, $vf2z
	vsubaz.xz	$ACCxz, $vf31xz, $vf0z
	vsubaz.xz	$ACCxz, $vf31xz, $vf15z
	vsubaz.xz	$ACCxz, $vf31xz, $vf31z
	vsubaz.xzw	$ACCxzw, $vf0xzw, $vf0z
	vsubaz.xzw	$ACCxzw, $vf0xzw, $vf31z
	vsubaz.xzw	$ACCxzw, $vf1xzw, $vf2z
	vsubaz.xzw	$ACCxzw, $vf31xzw, $vf0z
	vsubaz.xzw	$ACCxzw, $vf31xzw, $vf15z
	vsubaz.xzw	$ACCxzw, $vf31xzw, $vf31z
	vsubaz.y	$ACCy, $vf0y, $vf0z
	vsubaz.y	$ACCy, $vf0y, $vf31z
	vsubaz.y	$ACCy, $vf1y, $vf2z
	vsubaz.y	$ACCy, $vf31y, $vf0z
	vsubaz.y	$ACCy, $vf31y, $vf15z
	vsubaz.y	$ACCy, $vf31y, $vf31z
	vsubaz.yw	$ACCyw, $vf0yw, $vf0z
	vsubaz.yw	$ACCyw, $vf0yw, $vf31z
	vsubaz.yw	$ACCyw, $vf1yw, $vf2z
	vsubaz.yw	$ACCyw, $vf31yw, $vf0z
	vsubaz.yw	$ACCyw, $vf31yw, $vf15z
	vsubaz.yw	$ACCyw, $vf31yw, $vf31z
	vsubaz.yz	$ACCyz, $vf0yz, $vf0z
	vsubaz.yz	$ACCyz, $vf0yz, $vf31z
	vsubaz.yz	$ACCyz, $vf1yz, $vf2z
	vsubaz.yz	$ACCyz, $vf31yz, $vf0z
	vsubaz.yz	$ACCyz, $vf31yz, $vf15z
	vsubaz.yz	$ACCyz, $vf31yz, $vf31z
	vsubaz.yzw	$ACCyzw, $vf0yzw, $vf0z
	vsubaz.yzw	$ACCyzw, $vf0yzw, $vf31z
	vsubaz.yzw	$ACCyzw, $vf1yzw, $vf2z
	vsubaz.yzw	$ACCyzw, $vf31yzw, $vf0z
	vsubaz.yzw	$ACCyzw, $vf31yzw, $vf15z
	vsubaz.yzw	$ACCyzw, $vf31yzw, $vf31z
	vsubaz.z	$ACCz, $vf0z, $vf0z
	vsubaz.z	$ACCz, $vf0z, $vf31z
	vsubaz.z	$ACCz, $vf1z, $vf2z
	vsubaz.z	$ACCz, $vf31z, $vf0z
	vsubaz.z	$ACCz, $vf31z, $vf15z
	vsubaz.z	$ACCz, $vf31z, $vf31z
	vsubaz.zw	$ACCzw, $vf0zw, $vf0z
	vsubaz.zw	$ACCzw, $vf0zw, $vf31z
	vsubaz.zw	$ACCzw, $vf1zw, $vf2z
	vsubaz.zw	$ACCzw, $vf31zw, $vf0z
	vsubaz.zw	$ACCzw, $vf31zw, $vf15z
	vsubaz.zw	$ACCzw, $vf31zw, $vf31z
	vsubi.w		$vf0w, $vf0w, $I
	vsubi.w		$vf0w, $vf31w, $I
	vsubi.w		$vf1w, $vf2w, $I
	vsubi.w		$vf31w, $vf0w, $I
	vsubi.w		$vf31w, $vf15w, $I
	vsubi.w		$vf31w, $vf31w, $I
	vsubi.x		$vf0x, $vf0x, $I
	vsubi.x		$vf0x, $vf31x, $I
	vsubi.x		$vf1x, $vf2x, $I
	vsubi.x		$vf31x, $vf0x, $I
	vsubi.x		$vf31x, $vf15x, $I
	vsubi.x		$vf31x, $vf31x, $I
	vsubi.xw	$vf0xw, $vf0xw, $I
	vsubi.xw	$vf0xw, $vf31xw, $I
	vsubi.xw	$vf1xw, $vf2xw, $I
	vsubi.xw	$vf31xw, $vf0xw, $I
	vsubi.xw	$vf31xw, $vf15xw, $I
	vsubi.xw	$vf31xw, $vf31xw, $I
	vsubi.xy	$vf0xy, $vf0xy, $I
	vsubi.xy	$vf0xy, $vf31xy, $I
	vsubi.xy	$vf1xy, $vf2xy, $I
	vsubi.xy	$vf31xy, $vf0xy, $I
	vsubi.xy	$vf31xy, $vf15xy, $I
	vsubi.xy	$vf31xy, $vf31xy, $I
	vsubi.xyw	$vf0xyw, $vf0xyw, $I
	vsubi.xyw	$vf0xyw, $vf31xyw, $I
	vsubi.xyw	$vf1xyw, $vf2xyw, $I
	vsubi.xyw	$vf31xyw, $vf0xyw, $I
	vsubi.xyw	$vf31xyw, $vf15xyw, $I
	vsubi.xyw	$vf31xyw, $vf31xyw, $I
	vsubi.xyz	$vf0xyz, $vf0xyz, $I
	vsubi.xyz	$vf0xyz, $vf31xyz, $I
	vsubi.xyz	$vf1xyz, $vf2xyz, $I
	vsubi.xyz	$vf31xyz, $vf0xyz, $I
	vsubi.xyz	$vf31xyz, $vf15xyz, $I
	vsubi.xyz	$vf31xyz, $vf31xyz, $I
	vsubi.xyzw	$vf0xyzw, $vf0xyzw, $I
	vsubi.xyzw	$vf0xyzw, $vf31xyzw, $I
	vsubi.xyzw	$vf1xyzw, $vf2xyzw, $I
	vsubi.xyzw	$vf31xyzw, $vf0xyzw, $I
	vsubi.xyzw	$vf31xyzw, $vf15xyzw, $I
	vsubi.xyzw	$vf31xyzw, $vf31xyzw, $I
	vsubi.xz	$vf0xz, $vf0xz, $I
	vsubi.xz	$vf0xz, $vf31xz, $I
	vsubi.xz	$vf1xz, $vf2xz, $I
	vsubi.xz	$vf31xz, $vf0xz, $I
	vsubi.xz	$vf31xz, $vf15xz, $I
	vsubi.xz	$vf31xz, $vf31xz, $I
	vsubi.xzw	$vf0xzw, $vf0xzw, $I
	vsubi.xzw	$vf0xzw, $vf31xzw, $I
	vsubi.xzw	$vf1xzw, $vf2xzw, $I
	vsubi.xzw	$vf31xzw, $vf0xzw, $I
	vsubi.xzw	$vf31xzw, $vf15xzw, $I
	vsubi.xzw	$vf31xzw, $vf31xzw, $I
	vsubi.y		$vf0y, $vf0y, $I
	vsubi.y		$vf0y, $vf31y, $I
	vsubi.y		$vf1y, $vf2y, $I
	vsubi.y		$vf31y, $vf0y, $I
	vsubi.y		$vf31y, $vf15y, $I
	vsubi.y		$vf31y, $vf31y, $I
	vsubi.yw	$vf0yw, $vf0yw, $I
	vsubi.yw	$vf0yw, $vf31yw, $I
	vsubi.yw	$vf1yw, $vf2yw, $I
	vsubi.yw	$vf31yw, $vf0yw, $I
	vsubi.yw	$vf31yw, $vf15yw, $I
	vsubi.yw	$vf31yw, $vf31yw, $I
	vsubi.yz	$vf0yz, $vf0yz, $I
	vsubi.yz	$vf0yz, $vf31yz, $I
	vsubi.yz	$vf1yz, $vf2yz, $I
	vsubi.yz	$vf31yz, $vf0yz, $I
	vsubi.yz	$vf31yz, $vf15yz, $I
	vsubi.yz	$vf31yz, $vf31yz, $I
	vsubi.yzw	$vf0yzw, $vf0yzw, $I
	vsubi.yzw	$vf0yzw, $vf31yzw, $I
	vsubi.yzw	$vf1yzw, $vf2yzw, $I
	vsubi.yzw	$vf31yzw, $vf0yzw, $I
	vsubi.yzw	$vf31yzw, $vf15yzw, $I
	vsubi.yzw	$vf31yzw, $vf31yzw, $I
	vsubi.z		$vf0z, $vf0z, $I
	vsubi.z		$vf0z, $vf31z, $I
	vsubi.z		$vf1z, $vf2z, $I
	vsubi.z		$vf31z, $vf0z, $I
	vsubi.z		$vf31z, $vf15z, $I
	vsubi.z		$vf31z, $vf31z, $I
	vsubi.zw	$vf0zw, $vf0zw, $I
	vsubi.zw	$vf0zw, $vf31zw, $I
	vsubi.zw	$vf1zw, $vf2zw, $I
	vsubi.zw	$vf31zw, $vf0zw, $I
	vsubi.zw	$vf31zw, $vf15zw, $I
	vsubi.zw	$vf31zw, $vf31zw, $I
	vsubq.w		$vf0w, $vf0w, $Q
	vsubq.w		$vf0w, $vf31w, $Q
	vsubq.w		$vf1w, $vf2w, $Q
	vsubq.w		$vf31w, $vf0w, $Q
	vsubq.w		$vf31w, $vf15w, $Q
	vsubq.w		$vf31w, $vf31w, $Q
	vsubq.x		$vf0x, $vf0x, $Q
	vsubq.x		$vf0x, $vf31x, $Q
	vsubq.x		$vf1x, $vf2x, $Q
	vsubq.x		$vf31x, $vf0x, $Q
	vsubq.x		$vf31x, $vf15x, $Q
	vsubq.x		$vf31x, $vf31x, $Q
	vsubq.xw	$vf0xw, $vf0xw, $Q
	vsubq.xw	$vf0xw, $vf31xw, $Q
	vsubq.xw	$vf1xw, $vf2xw, $Q
	vsubq.xw	$vf31xw, $vf0xw, $Q
	vsubq.xw	$vf31xw, $vf15xw, $Q
	vsubq.xw	$vf31xw, $vf31xw, $Q
	vsubq.xy	$vf0xy, $vf0xy, $Q
	vsubq.xy	$vf0xy, $vf31xy, $Q
	vsubq.xy	$vf1xy, $vf2xy, $Q
	vsubq.xy	$vf31xy, $vf0xy, $Q
	vsubq.xy	$vf31xy, $vf15xy, $Q
	vsubq.xy	$vf31xy, $vf31xy, $Q
	vsubq.xyw	$vf0xyw, $vf0xyw, $Q
	vsubq.xyw	$vf0xyw, $vf31xyw, $Q
	vsubq.xyw	$vf1xyw, $vf2xyw, $Q
	vsubq.xyw	$vf31xyw, $vf0xyw, $Q
	vsubq.xyw	$vf31xyw, $vf15xyw, $Q
	vsubq.xyw	$vf31xyw, $vf31xyw, $Q
	vsubq.xyz	$vf0xyz, $vf0xyz, $Q
	vsubq.xyz	$vf0xyz, $vf31xyz, $Q
	vsubq.xyz	$vf1xyz, $vf2xyz, $Q
	vsubq.xyz	$vf31xyz, $vf0xyz, $Q
	vsubq.xyz	$vf31xyz, $vf15xyz, $Q
	vsubq.xyz	$vf31xyz, $vf31xyz, $Q
	vsubq.xyzw	$vf0xyzw, $vf0xyzw, $Q
	vsubq.xyzw	$vf0xyzw, $vf31xyzw, $Q
	vsubq.xyzw	$vf1xyzw, $vf2xyzw, $Q
	vsubq.xyzw	$vf31xyzw, $vf0xyzw, $Q
	vsubq.xyzw	$vf31xyzw, $vf15xyzw, $Q
	vsubq.xyzw	$vf31xyzw, $vf31xyzw, $Q
	vsubq.xz	$vf0xz, $vf0xz, $Q
	vsubq.xz	$vf0xz, $vf31xz, $Q
	vsubq.xz	$vf1xz, $vf2xz, $Q
	vsubq.xz	$vf31xz, $vf0xz, $Q
	vsubq.xz	$vf31xz, $vf15xz, $Q
	vsubq.xz	$vf31xz, $vf31xz, $Q
	vsubq.xzw	$vf0xzw, $vf0xzw, $Q
	vsubq.xzw	$vf0xzw, $vf31xzw, $Q
	vsubq.xzw	$vf1xzw, $vf2xzw, $Q
	vsubq.xzw	$vf31xzw, $vf0xzw, $Q
	vsubq.xzw	$vf31xzw, $vf15xzw, $Q
	vsubq.xzw	$vf31xzw, $vf31xzw, $Q
	vsubq.y		$vf0y, $vf0y, $Q
	vsubq.y		$vf0y, $vf31y, $Q
	vsubq.y		$vf1y, $vf2y, $Q
	vsubq.y		$vf31y, $vf0y, $Q
	vsubq.y		$vf31y, $vf15y, $Q
	vsubq.y		$vf31y, $vf31y, $Q
	vsubq.yw	$vf0yw, $vf0yw, $Q
	vsubq.yw	$vf0yw, $vf31yw, $Q
	vsubq.yw	$vf1yw, $vf2yw, $Q
	vsubq.yw	$vf31yw, $vf0yw, $Q
	vsubq.yw	$vf31yw, $vf15yw, $Q
	vsubq.yw	$vf31yw, $vf31yw, $Q
	vsubq.yz	$vf0yz, $vf0yz, $Q
	vsubq.yz	$vf0yz, $vf31yz, $Q
	vsubq.yz	$vf1yz, $vf2yz, $Q
	vsubq.yz	$vf31yz, $vf0yz, $Q
	vsubq.yz	$vf31yz, $vf15yz, $Q
	vsubq.yz	$vf31yz, $vf31yz, $Q
	vsubq.yzw	$vf0yzw, $vf0yzw, $Q
	vsubq.yzw	$vf0yzw, $vf31yzw, $Q
	vsubq.yzw	$vf1yzw, $vf2yzw, $Q
	vsubq.yzw	$vf31yzw, $vf0yzw, $Q
	vsubq.yzw	$vf31yzw, $vf15yzw, $Q
	vsubq.yzw	$vf31yzw, $vf31yzw, $Q
	vsubq.z		$vf0z, $vf0z, $Q
	vsubq.z		$vf0z, $vf31z, $Q
	vsubq.z		$vf1z, $vf2z, $Q
	vsubq.z		$vf31z, $vf0z, $Q
	vsubq.z		$vf31z, $vf15z, $Q
	vsubq.z		$vf31z, $vf31z, $Q
	vsubq.zw	$vf0zw, $vf0zw, $Q
	vsubq.zw	$vf0zw, $vf31zw, $Q
	vsubq.zw	$vf1zw, $vf2zw, $Q
	vsubq.zw	$vf31zw, $vf0zw, $Q
	vsubq.zw	$vf31zw, $vf15zw, $Q
	vsubq.zw	$vf31zw, $vf31zw, $Q
	vsub.w		$vf0w, $vf0w, $vf0w
	vsub.w		$vf0w, $vf0w, $vf31w
	vsub.w		$vf0w, $vf31w, $vf0w
	vsub.w		$vf1w, $vf2w, $vf3w
	vsub.w		$vf31w, $vf0w, $vf0w
	vsub.w		$vf31w, $vf15w, $vf7w
	vsub.w		$vf31w, $vf31w, $vf31w
	vsubw.w		$vf0w, $vf0w, $vf0w
	vsubw.w		$vf0w, $vf0w, $vf31w
	vsubw.w		$vf0w, $vf31w, $vf0w
	vsubw.w		$vf1w, $vf2w, $vf3w
	vsubw.w		$vf31w, $vf0w, $vf0w
	vsubw.w		$vf31w, $vf15w, $vf7w
	vsubw.w		$vf31w, $vf31w, $vf31w
	vsubw.x		$vf0x, $vf0x, $vf0w
	vsubw.x		$vf0x, $vf0x, $vf31w
	vsubw.x		$vf0x, $vf31x, $vf0w
	vsubw.x		$vf1x, $vf2x, $vf3w
	vsubw.x		$vf31x, $vf0x, $vf0w
	vsubw.x		$vf31x, $vf15x, $vf7w
	vsubw.x		$vf31x, $vf31x, $vf31w
	vsubw.xw	$vf0xw, $vf0xw, $vf0w
	vsubw.xw	$vf0xw, $vf0xw, $vf31w
	vsubw.xw	$vf0xw, $vf31xw, $vf0w
	vsubw.xw	$vf1xw, $vf2xw, $vf3w
	vsubw.xw	$vf31xw, $vf0xw, $vf0w
	vsubw.xw	$vf31xw, $vf15xw, $vf7w
	vsubw.xw	$vf31xw, $vf31xw, $vf31w
	vsubw.xy	$vf0xy, $vf0xy, $vf0w
	vsubw.xy	$vf0xy, $vf0xy, $vf31w
	vsubw.xy	$vf0xy, $vf31xy, $vf0w
	vsubw.xy	$vf1xy, $vf2xy, $vf3w
	vsubw.xy	$vf31xy, $vf0xy, $vf0w
	vsubw.xy	$vf31xy, $vf15xy, $vf7w
	vsubw.xy	$vf31xy, $vf31xy, $vf31w
	vsubw.xyw	$vf0xyw, $vf0xyw, $vf0w
	vsubw.xyw	$vf0xyw, $vf0xyw, $vf31w
	vsubw.xyw	$vf0xyw, $vf31xyw, $vf0w
	vsubw.xyw	$vf1xyw, $vf2xyw, $vf3w
	vsubw.xyw	$vf31xyw, $vf0xyw, $vf0w
	vsubw.xyw	$vf31xyw, $vf15xyw, $vf7w
	vsubw.xyw	$vf31xyw, $vf31xyw, $vf31w
	vsubw.xyz	$vf0xyz, $vf0xyz, $vf0w
	vsubw.xyz	$vf0xyz, $vf0xyz, $vf31w
	vsubw.xyz	$vf0xyz, $vf31xyz, $vf0w
	vsubw.xyz	$vf1xyz, $vf2xyz, $vf3w
	vsubw.xyz	$vf31xyz, $vf0xyz, $vf0w
	vsubw.xyz	$vf31xyz, $vf15xyz, $vf7w
	vsubw.xyz	$vf31xyz, $vf31xyz, $vf31w
	vsubw.xyzw	$vf0xyzw, $vf0xyzw, $vf0w
	vsubw.xyzw	$vf0xyzw, $vf0xyzw, $vf31w
	vsubw.xyzw	$vf0xyzw, $vf31xyzw, $vf0w
	vsubw.xyzw	$vf1xyzw, $vf2xyzw, $vf3w
	vsubw.xyzw	$vf31xyzw, $vf0xyzw, $vf0w
	vsubw.xyzw	$vf31xyzw, $vf15xyzw, $vf7w
	vsubw.xyzw	$vf31xyzw, $vf31xyzw, $vf31w
	vsubw.xz	$vf0xz, $vf0xz, $vf0w
	vsubw.xz	$vf0xz, $vf0xz, $vf31w
	vsubw.xz	$vf0xz, $vf31xz, $vf0w
	vsubw.xz	$vf1xz, $vf2xz, $vf3w
	vsubw.xz	$vf31xz, $vf0xz, $vf0w
	vsubw.xz	$vf31xz, $vf15xz, $vf7w
	vsubw.xz	$vf31xz, $vf31xz, $vf31w
	vsubw.xzw	$vf0xzw, $vf0xzw, $vf0w
	vsubw.xzw	$vf0xzw, $vf0xzw, $vf31w
	vsubw.xzw	$vf0xzw, $vf31xzw, $vf0w
	vsubw.xzw	$vf1xzw, $vf2xzw, $vf3w
	vsubw.xzw	$vf31xzw, $vf0xzw, $vf0w
	vsubw.xzw	$vf31xzw, $vf15xzw, $vf7w
	vsubw.xzw	$vf31xzw, $vf31xzw, $vf31w
	vsubw.y		$vf0y, $vf0y, $vf0w
	vsubw.y		$vf0y, $vf0y, $vf31w
	vsubw.y		$vf0y, $vf31y, $vf0w
	vsubw.y		$vf1y, $vf2y, $vf3w
	vsubw.y		$vf31y, $vf0y, $vf0w
	vsubw.y		$vf31y, $vf15y, $vf7w
	vsubw.y		$vf31y, $vf31y, $vf31w
	vsubw.yw	$vf0yw, $vf0yw, $vf0w
	vsubw.yw	$vf0yw, $vf0yw, $vf31w
	vsubw.yw	$vf0yw, $vf31yw, $vf0w
	vsubw.yw	$vf1yw, $vf2yw, $vf3w
	vsubw.yw	$vf31yw, $vf0yw, $vf0w
	vsubw.yw	$vf31yw, $vf15yw, $vf7w
	vsubw.yw	$vf31yw, $vf31yw, $vf31w
	vsubw.yz	$vf0yz, $vf0yz, $vf0w
	vsubw.yz	$vf0yz, $vf0yz, $vf31w
	vsubw.yz	$vf0yz, $vf31yz, $vf0w
	vsubw.yz	$vf1yz, $vf2yz, $vf3w
	vsubw.yz	$vf31yz, $vf0yz, $vf0w
	vsubw.yz	$vf31yz, $vf15yz, $vf7w
	vsubw.yz	$vf31yz, $vf31yz, $vf31w
	vsubw.yzw	$vf0yzw, $vf0yzw, $vf0w
	vsubw.yzw	$vf0yzw, $vf0yzw, $vf31w
	vsubw.yzw	$vf0yzw, $vf31yzw, $vf0w
	vsubw.yzw	$vf1yzw, $vf2yzw, $vf3w
	vsubw.yzw	$vf31yzw, $vf0yzw, $vf0w
	vsubw.yzw	$vf31yzw, $vf15yzw, $vf7w
	vsubw.yzw	$vf31yzw, $vf31yzw, $vf31w
	vsubw.z		$vf0z, $vf0z, $vf0w
	vsubw.z		$vf0z, $vf0z, $vf31w
	vsubw.z		$vf0z, $vf31z, $vf0w
	vsubw.z		$vf1z, $vf2z, $vf3w
	vsubw.z		$vf31z, $vf0z, $vf0w
	vsubw.z		$vf31z, $vf15z, $vf7w
	vsubw.z		$vf31z, $vf31z, $vf31w
	vsubw.zw	$vf0zw, $vf0zw, $vf0w
	vsubw.zw	$vf0zw, $vf0zw, $vf31w
	vsubw.zw	$vf0zw, $vf31zw, $vf0w
	vsubw.zw	$vf1zw, $vf2zw, $vf3w
	vsubw.zw	$vf31zw, $vf0zw, $vf0w
	vsubw.zw	$vf31zw, $vf15zw, $vf7w
	vsubw.zw	$vf31zw, $vf31zw, $vf31w
	vsub.x		$vf0x, $vf0x, $vf0x
	vsub.x		$vf0x, $vf0x, $vf31x
	vsub.x		$vf0x, $vf31x, $vf0x
	vsub.x		$vf1x, $vf2x, $vf3x
	vsub.x		$vf31x, $vf0x, $vf0x
	vsub.x		$vf31x, $vf15x, $vf7x
	vsub.x		$vf31x, $vf31x, $vf31x
	vsubx.w		$vf0w, $vf0w, $vf0x
	vsubx.w		$vf0w, $vf0w, $vf31x
	vsubx.w		$vf0w, $vf31w, $vf0x
	vsub.xw		$vf0xw, $vf0xw, $vf0xw
	vsub.xw		$vf0xw, $vf0xw, $vf31xw
	vsub.xw		$vf0xw, $vf31xw, $vf0xw
	vsubx.w		$vf1w, $vf2w, $vf3x
	vsub.xw		$vf1xw, $vf2xw, $vf3xw
	vsubx.w		$vf31w, $vf0w, $vf0x
	vsubx.w		$vf31w, $vf15w, $vf7x
	vsubx.w		$vf31w, $vf31w, $vf31x
	vsub.xw		$vf31xw, $vf0xw, $vf0xw
	vsub.xw		$vf31xw, $vf15xw, $vf7xw
	vsub.xw		$vf31xw, $vf31xw, $vf31xw
	vsubx.x		$vf0x, $vf0x, $vf0x
	vsubx.x		$vf0x, $vf0x, $vf31x
	vsubx.x		$vf0x, $vf31x, $vf0x
	vsubx.x		$vf1x, $vf2x, $vf3x
	vsubx.x		$vf31x, $vf0x, $vf0x
	vsubx.x		$vf31x, $vf15x, $vf7x
	vsubx.x		$vf31x, $vf31x, $vf31x
	vsubx.xw	$vf0xw, $vf0xw, $vf0x
	vsubx.xw	$vf0xw, $vf0xw, $vf31x
	vsubx.xw	$vf0xw, $vf31xw, $vf0x
	vsubx.xw	$vf1xw, $vf2xw, $vf3x
	vsubx.xw	$vf31xw, $vf0xw, $vf0x
	vsubx.xw	$vf31xw, $vf15xw, $vf7x
	vsubx.xw	$vf31xw, $vf31xw, $vf31x
	vsubx.xy	$vf0xy, $vf0xy, $vf0x
	vsubx.xy	$vf0xy, $vf0xy, $vf31x
	vsubx.xy	$vf0xy, $vf31xy, $vf0x
	vsubx.xy	$vf1xy, $vf2xy, $vf3x
	vsubx.xy	$vf31xy, $vf0xy, $vf0x
	vsubx.xy	$vf31xy, $vf15xy, $vf7x
	vsubx.xy	$vf31xy, $vf31xy, $vf31x
	vsubx.xyw	$vf0xyw, $vf0xyw, $vf0x
	vsubx.xyw	$vf0xyw, $vf0xyw, $vf31x
	vsubx.xyw	$vf0xyw, $vf31xyw, $vf0x
	vsubx.xyw	$vf1xyw, $vf2xyw, $vf3x
	vsubx.xyw	$vf31xyw, $vf0xyw, $vf0x
	vsubx.xyw	$vf31xyw, $vf15xyw, $vf7x
	vsubx.xyw	$vf31xyw, $vf31xyw, $vf31x
	vsubx.xyz	$vf0xyz, $vf0xyz, $vf0x
	vsubx.xyz	$vf0xyz, $vf0xyz, $vf31x
	vsubx.xyz	$vf0xyz, $vf31xyz, $vf0x
	vsubx.xyz	$vf1xyz, $vf2xyz, $vf3x
	vsubx.xyz	$vf31xyz, $vf0xyz, $vf0x
	vsubx.xyz	$vf31xyz, $vf15xyz, $vf7x
	vsubx.xyz	$vf31xyz, $vf31xyz, $vf31x
	vsubx.xyzw	$vf0xyzw, $vf0xyzw, $vf0x
	vsubx.xyzw	$vf0xyzw, $vf0xyzw, $vf31x
	vsubx.xyzw	$vf0xyzw, $vf31xyzw, $vf0x
	vsubx.xyzw	$vf1xyzw, $vf2xyzw, $vf3x
	vsubx.xyzw	$vf31xyzw, $vf0xyzw, $vf0x
	vsubx.xyzw	$vf31xyzw, $vf15xyzw, $vf7x
	vsubx.xyzw	$vf31xyzw, $vf31xyzw, $vf31x
	vsubx.xz	$vf0xz, $vf0xz, $vf0x
	vsubx.xz	$vf0xz, $vf0xz, $vf31x
	vsubx.xz	$vf0xz, $vf31xz, $vf0x
	vsubx.xz	$vf1xz, $vf2xz, $vf3x
	vsubx.xz	$vf31xz, $vf0xz, $vf0x
	vsubx.xz	$vf31xz, $vf15xz, $vf7x
	vsubx.xz	$vf31xz, $vf31xz, $vf31x
	vsubx.xzw	$vf0xzw, $vf0xzw, $vf0x
	vsubx.xzw	$vf0xzw, $vf0xzw, $vf31x
	vsubx.xzw	$vf0xzw, $vf31xzw, $vf0x
	vsubx.xzw	$vf1xzw, $vf2xzw, $vf3x
	vsubx.xzw	$vf31xzw, $vf0xzw, $vf0x
	vsubx.xzw	$vf31xzw, $vf15xzw, $vf7x
	vsubx.xzw	$vf31xzw, $vf31xzw, $vf31x
	vsub.xy		$vf0xy, $vf0xy, $vf0xy
	vsub.xy		$vf0xy, $vf0xy, $vf31xy
	vsub.xy		$vf0xy, $vf31xy, $vf0xy
	vsubx.y		$vf0y, $vf0y, $vf0x
	vsubx.y		$vf0y, $vf0y, $vf31x
	vsubx.y		$vf0y, $vf31y, $vf0x
	vsub.xy		$vf1xy, $vf2xy, $vf3xy
	vsubx.y		$vf1y, $vf2y, $vf3x
	vsub.xy		$vf31xy, $vf0xy, $vf0xy
	vsub.xy		$vf31xy, $vf15xy, $vf7xy
	vsub.xy		$vf31xy, $vf31xy, $vf31xy
	vsubx.y		$vf31y, $vf0y, $vf0x
	vsubx.y		$vf31y, $vf15y, $vf7x
	vsubx.y		$vf31y, $vf31y, $vf31x
	vsub.xyw	$vf0xyw, $vf0xyw, $vf0xyw
	vsub.xyw	$vf0xyw, $vf0xyw, $vf31xyw
	vsub.xyw	$vf0xyw, $vf31xyw, $vf0xyw
	vsubx.yw	$vf0yw, $vf0yw, $vf0x
	vsubx.yw	$vf0yw, $vf0yw, $vf31x
	vsubx.yw	$vf0yw, $vf31yw, $vf0x
	vsub.xyw	$vf1xyw, $vf2xyw, $vf3xyw
	vsubx.yw	$vf1yw, $vf2yw, $vf3x
	vsub.xyw	$vf31xyw, $vf0xyw, $vf0xyw
	vsub.xyw	$vf31xyw, $vf15xyw, $vf7xyw
	vsub.xyw	$vf31xyw, $vf31xyw, $vf31xyw
	vsubx.yw	$vf31yw, $vf0yw, $vf0x
	vsubx.yw	$vf31yw, $vf15yw, $vf7x
	vsubx.yw	$vf31yw, $vf31yw, $vf31x
	vsub.xyz	$vf0xyz, $vf0xyz, $vf0xyz
	vsub.xyz	$vf0xyz, $vf0xyz, $vf31xyz
	vsub.xyz	$vf0xyz, $vf31xyz, $vf0xyz
	vsubx.yz	$vf0yz, $vf0yz, $vf0x
	vsubx.yz	$vf0yz, $vf0yz, $vf31x
	vsubx.yz	$vf0yz, $vf31yz, $vf0x
	vsub.xyz	$vf1xyz, $vf2xyz, $vf3xyz
	vsubx.yz	$vf1yz, $vf2yz, $vf3x
	vsub.xyz	$vf31xyz, $vf0xyz, $vf0xyz
	vsub.xyz	$vf31xyz, $vf15xyz, $vf7xyz
	vsub.xyz	$vf31xyz, $vf31xyz, $vf31xyz
	vsubx.yz	$vf31yz, $vf0yz, $vf0x
	vsubx.yz	$vf31yz, $vf15yz, $vf7x
	vsubx.yz	$vf31yz, $vf31yz, $vf31x
	vsub.xyzw	$vf0xyzw, $vf0xyzw, $vf0xyzw
	vsub.xyzw	$vf0xyzw, $vf0xyzw, $vf31xyzw
	vsub.xyzw	$vf0xyzw, $vf31xyzw, $vf0xyzw
	vsubx.yzw	$vf0yzw, $vf0yzw, $vf0x
	vsubx.yzw	$vf0yzw, $vf0yzw, $vf31x
	vsubx.yzw	$vf0yzw, $vf31yzw, $vf0x
	vsub.xyzw	$vf1xyzw, $vf2xyzw, $vf3xyzw
	vsubx.yzw	$vf1yzw, $vf2yzw, $vf3x
	vsub.xyzw	$vf31xyzw, $vf0xyzw, $vf0xyzw
	vsub.xyzw	$vf31xyzw, $vf15xyzw, $vf7xyzw
	vsub.xyzw	$vf31xyzw, $vf31xyzw, $vf31xyzw
	vsubx.yzw	$vf31yzw, $vf0yzw, $vf0x
	vsubx.yzw	$vf31yzw, $vf15yzw, $vf7x
	vsubx.yzw	$vf31yzw, $vf31yzw, $vf31x
	vsub.xz		$vf0xz, $vf0xz, $vf0xz
	vsub.xz		$vf0xz, $vf0xz, $vf31xz
	vsub.xz		$vf0xz, $vf31xz, $vf0xz
	vsubx.z		$vf0z, $vf0z, $vf0x
	vsubx.z		$vf0z, $vf0z, $vf31x
	vsubx.z		$vf0z, $vf31z, $vf0x
	vsub.xz		$vf1xz, $vf2xz, $vf3xz
	vsubx.z		$vf1z, $vf2z, $vf3x
	vsub.xz		$vf31xz, $vf0xz, $vf0xz
	vsub.xz		$vf31xz, $vf15xz, $vf7xz
	vsub.xz		$vf31xz, $vf31xz, $vf31xz
	vsubx.z		$vf31z, $vf0z, $vf0x
	vsubx.z		$vf31z, $vf15z, $vf7x
	vsubx.z		$vf31z, $vf31z, $vf31x
	vsub.xzw	$vf0xzw, $vf0xzw, $vf0xzw
	vsub.xzw	$vf0xzw, $vf0xzw, $vf31xzw
	vsub.xzw	$vf0xzw, $vf31xzw, $vf0xzw
	vsubx.zw	$vf0zw, $vf0zw, $vf0x
	vsubx.zw	$vf0zw, $vf0zw, $vf31x
	vsubx.zw	$vf0zw, $vf31zw, $vf0x
	vsub.xzw	$vf1xzw, $vf2xzw, $vf3xzw
	vsubx.zw	$vf1zw, $vf2zw, $vf3x
	vsub.xzw	$vf31xzw, $vf0xzw, $vf0xzw
	vsub.xzw	$vf31xzw, $vf15xzw, $vf7xzw
	vsub.xzw	$vf31xzw, $vf31xzw, $vf31xzw
	vsubx.zw	$vf31zw, $vf0zw, $vf0x
	vsubx.zw	$vf31zw, $vf15zw, $vf7x
	vsubx.zw	$vf31zw, $vf31zw, $vf31x
	vsub.y		$vf0y, $vf0y, $vf0y
	vsub.y		$vf0y, $vf0y, $vf31y
	vsub.y		$vf0y, $vf31y, $vf0y
	vsub.y		$vf1y, $vf2y, $vf3y
	vsub.y		$vf31y, $vf0y, $vf0y
	vsub.y		$vf31y, $vf15y, $vf7y
	vsub.y		$vf31y, $vf31y, $vf31y
	vsuby.w		$vf0w, $vf0w, $vf0y
	vsuby.w		$vf0w, $vf0w, $vf31y
	vsuby.w		$vf0w, $vf31w, $vf0y
	vsub.yw		$vf0yw, $vf0yw, $vf0yw
	vsub.yw		$vf0yw, $vf0yw, $vf31yw
	vsub.yw		$vf0yw, $vf31yw, $vf0yw
	vsuby.w		$vf1w, $vf2w, $vf3y
	vsub.yw		$vf1yw, $vf2yw, $vf3yw
	vsuby.w		$vf31w, $vf0w, $vf0y
	vsuby.w		$vf31w, $vf15w, $vf7y
	vsuby.w		$vf31w, $vf31w, $vf31y
	vsub.yw		$vf31yw, $vf0yw, $vf0yw
	vsub.yw		$vf31yw, $vf15yw, $vf7yw
	vsub.yw		$vf31yw, $vf31yw, $vf31yw
	vsuby.x		$vf0x, $vf0x, $vf0y
	vsuby.x		$vf0x, $vf0x, $vf31y
	vsuby.x		$vf0x, $vf31x, $vf0y
	vsuby.x		$vf1x, $vf2x, $vf3y
	vsuby.x		$vf31x, $vf0x, $vf0y
	vsuby.x		$vf31x, $vf15x, $vf7y
	vsuby.x		$vf31x, $vf31x, $vf31y
	vsuby.xw	$vf0xw, $vf0xw, $vf0y
	vsuby.xw	$vf0xw, $vf0xw, $vf31y
	vsuby.xw	$vf0xw, $vf31xw, $vf0y
	vsuby.xw	$vf1xw, $vf2xw, $vf3y
	vsuby.xw	$vf31xw, $vf0xw, $vf0y
	vsuby.xw	$vf31xw, $vf15xw, $vf7y
	vsuby.xw	$vf31xw, $vf31xw, $vf31y
	vsuby.xy	$vf0xy, $vf0xy, $vf0y
	vsuby.xy	$vf0xy, $vf0xy, $vf31y
	vsuby.xy	$vf0xy, $vf31xy, $vf0y
	vsuby.xy	$vf1xy, $vf2xy, $vf3y
	vsuby.xy	$vf31xy, $vf0xy, $vf0y
	vsuby.xy	$vf31xy, $vf15xy, $vf7y
	vsuby.xy	$vf31xy, $vf31xy, $vf31y
	vsuby.xyw	$vf0xyw, $vf0xyw, $vf0y
	vsuby.xyw	$vf0xyw, $vf0xyw, $vf31y
	vsuby.xyw	$vf0xyw, $vf31xyw, $vf0y
	vsuby.xyw	$vf1xyw, $vf2xyw, $vf3y
	vsuby.xyw	$vf31xyw, $vf0xyw, $vf0y
	vsuby.xyw	$vf31xyw, $vf15xyw, $vf7y
	vsuby.xyw	$vf31xyw, $vf31xyw, $vf31y
	vsuby.xyz	$vf0xyz, $vf0xyz, $vf0y
	vsuby.xyz	$vf0xyz, $vf0xyz, $vf31y
	vsuby.xyz	$vf0xyz, $vf31xyz, $vf0y
	vsuby.xyz	$vf1xyz, $vf2xyz, $vf3y
	vsuby.xyz	$vf31xyz, $vf0xyz, $vf0y
	vsuby.xyz	$vf31xyz, $vf15xyz, $vf7y
	vsuby.xyz	$vf31xyz, $vf31xyz, $vf31y
	vsuby.xyzw	$vf0xyzw, $vf0xyzw, $vf0y
	vsuby.xyzw	$vf0xyzw, $vf0xyzw, $vf31y
	vsuby.xyzw	$vf0xyzw, $vf31xyzw, $vf0y
	vsuby.xyzw	$vf1xyzw, $vf2xyzw, $vf3y
	vsuby.xyzw	$vf31xyzw, $vf0xyzw, $vf0y
	vsuby.xyzw	$vf31xyzw, $vf15xyzw, $vf7y
	vsuby.xyzw	$vf31xyzw, $vf31xyzw, $vf31y
	vsuby.xz	$vf0xz, $vf0xz, $vf0y
	vsuby.xz	$vf0xz, $vf0xz, $vf31y
	vsuby.xz	$vf0xz, $vf31xz, $vf0y
	vsuby.xz	$vf1xz, $vf2xz, $vf3y
	vsuby.xz	$vf31xz, $vf0xz, $vf0y
	vsuby.xz	$vf31xz, $vf15xz, $vf7y
	vsuby.xz	$vf31xz, $vf31xz, $vf31y
	vsuby.xzw	$vf0xzw, $vf0xzw, $vf0y
	vsuby.xzw	$vf0xzw, $vf0xzw, $vf31y
	vsuby.xzw	$vf0xzw, $vf31xzw, $vf0y
	vsuby.xzw	$vf1xzw, $vf2xzw, $vf3y
	vsuby.xzw	$vf31xzw, $vf0xzw, $vf0y
	vsuby.xzw	$vf31xzw, $vf15xzw, $vf7y
	vsuby.xzw	$vf31xzw, $vf31xzw, $vf31y
	vsuby.y		$vf0y, $vf0y, $vf0y
	vsuby.y		$vf0y, $vf0y, $vf31y
	vsuby.y		$vf0y, $vf31y, $vf0y
	vsuby.y		$vf1y, $vf2y, $vf3y
	vsuby.y		$vf31y, $vf0y, $vf0y
	vsuby.y		$vf31y, $vf15y, $vf7y
	vsuby.y		$vf31y, $vf31y, $vf31y
	vsuby.yw	$vf0yw, $vf0yw, $vf0y
	vsuby.yw	$vf0yw, $vf0yw, $vf31y
	vsuby.yw	$vf0yw, $vf31yw, $vf0y
	vsuby.yw	$vf1yw, $vf2yw, $vf3y
	vsuby.yw	$vf31yw, $vf0yw, $vf0y
	vsuby.yw	$vf31yw, $vf15yw, $vf7y
	vsuby.yw	$vf31yw, $vf31yw, $vf31y
	vsuby.yz	$vf0yz, $vf0yz, $vf0y
	vsuby.yz	$vf0yz, $vf0yz, $vf31y
	vsuby.yz	$vf0yz, $vf31yz, $vf0y
	vsuby.yz	$vf1yz, $vf2yz, $vf3y
	vsuby.yz	$vf31yz, $vf0yz, $vf0y
	vsuby.yz	$vf31yz, $vf15yz, $vf7y
	vsuby.yz	$vf31yz, $vf31yz, $vf31y
	vsuby.yzw	$vf0yzw, $vf0yzw, $vf0y
	vsuby.yzw	$vf0yzw, $vf0yzw, $vf31y
	vsuby.yzw	$vf0yzw, $vf31yzw, $vf0y
	vsuby.yzw	$vf1yzw, $vf2yzw, $vf3y
	vsuby.yzw	$vf31yzw, $vf0yzw, $vf0y
	vsuby.yzw	$vf31yzw, $vf15yzw, $vf7y
	vsuby.yzw	$vf31yzw, $vf31yzw, $vf31y
	vsub.yz		$vf0yz, $vf0yz, $vf0yz
	vsub.yz		$vf0yz, $vf0yz, $vf31yz
	vsub.yz		$vf0yz, $vf31yz, $vf0yz
	vsuby.z		$vf0z, $vf0z, $vf0y
	vsuby.z		$vf0z, $vf0z, $vf31y
	vsuby.z		$vf0z, $vf31z, $vf0y
	vsub.yz		$vf1yz, $vf2yz, $vf3yz
	vsuby.z		$vf1z, $vf2z, $vf3y
	vsub.yz		$vf31yz, $vf0yz, $vf0yz
	vsub.yz		$vf31yz, $vf15yz, $vf7yz
	vsub.yz		$vf31yz, $vf31yz, $vf31yz
	vsuby.z		$vf31z, $vf0z, $vf0y
	vsuby.z		$vf31z, $vf15z, $vf7y
	vsuby.z		$vf31z, $vf31z, $vf31y
	vsub.yzw	$vf0yzw, $vf0yzw, $vf0yzw
	vsub.yzw	$vf0yzw, $vf0yzw, $vf31yzw
	vsub.yzw	$vf0yzw, $vf31yzw, $vf0yzw
	vsuby.zw	$vf0zw, $vf0zw, $vf0y
	vsuby.zw	$vf0zw, $vf0zw, $vf31y
	vsuby.zw	$vf0zw, $vf31zw, $vf0y
	vsub.yzw	$vf1yzw, $vf2yzw, $vf3yzw
	vsuby.zw	$vf1zw, $vf2zw, $vf3y
	vsub.yzw	$vf31yzw, $vf0yzw, $vf0yzw
	vsub.yzw	$vf31yzw, $vf15yzw, $vf7yzw
	vsub.yzw	$vf31yzw, $vf31yzw, $vf31yzw
	vsuby.zw	$vf31zw, $vf0zw, $vf0y
	vsuby.zw	$vf31zw, $vf15zw, $vf7y
	vsuby.zw	$vf31zw, $vf31zw, $vf31y
	vsub.z		$vf0z, $vf0z, $vf0z
	vsub.z		$vf0z, $vf0z, $vf31z
	vsub.z		$vf0z, $vf31z, $vf0z
	vsub.z		$vf1z, $vf2z, $vf3z
	vsub.z		$vf31z, $vf0z, $vf0z
	vsub.z		$vf31z, $vf15z, $vf7z
	vsub.z		$vf31z, $vf31z, $vf31z
	vsubz.w		$vf0w, $vf0w, $vf0z
	vsubz.w		$vf0w, $vf0w, $vf31z
	vsubz.w		$vf0w, $vf31w, $vf0z
	vsub.zw		$vf0zw, $vf0zw, $vf0zw
	vsub.zw		$vf0zw, $vf0zw, $vf31zw
	vsub.zw		$vf0zw, $vf31zw, $vf0zw
	vsubz.w		$vf1w, $vf2w, $vf3z
	vsub.zw		$vf1zw, $vf2zw, $vf3zw
	vsubz.w		$vf31w, $vf0w, $vf0z
	vsubz.w		$vf31w, $vf15w, $vf7z
	vsubz.w		$vf31w, $vf31w, $vf31z
	vsub.zw		$vf31zw, $vf0zw, $vf0zw
	vsub.zw		$vf31zw, $vf15zw, $vf7zw
	vsub.zw		$vf31zw, $vf31zw, $vf31zw
	vsubz.x		$vf0x, $vf0x, $vf0z
	vsubz.x		$vf0x, $vf0x, $vf31z
	vsubz.x		$vf0x, $vf31x, $vf0z
	vsubz.x		$vf1x, $vf2x, $vf3z
	vsubz.x		$vf31x, $vf0x, $vf0z
	vsubz.x		$vf31x, $vf15x, $vf7z
	vsubz.x		$vf31x, $vf31x, $vf31z
	vsubz.xw	$vf0xw, $vf0xw, $vf0z
	vsubz.xw	$vf0xw, $vf0xw, $vf31z
	vsubz.xw	$vf0xw, $vf31xw, $vf0z
	vsubz.xw	$vf1xw, $vf2xw, $vf3z
	vsubz.xw	$vf31xw, $vf0xw, $vf0z
	vsubz.xw	$vf31xw, $vf15xw, $vf7z
	vsubz.xw	$vf31xw, $vf31xw, $vf31z
	vsubz.xy	$vf0xy, $vf0xy, $vf0z
	vsubz.xy	$vf0xy, $vf0xy, $vf31z
	vsubz.xy	$vf0xy, $vf31xy, $vf0z
	vsubz.xy	$vf1xy, $vf2xy, $vf3z
	vsubz.xy	$vf31xy, $vf0xy, $vf0z
	vsubz.xy	$vf31xy, $vf15xy, $vf7z
	vsubz.xy	$vf31xy, $vf31xy, $vf31z
	vsubz.xyw	$vf0xyw, $vf0xyw, $vf0z
	vsubz.xyw	$vf0xyw, $vf0xyw, $vf31z
	vsubz.xyw	$vf0xyw, $vf31xyw, $vf0z
	vsubz.xyw	$vf1xyw, $vf2xyw, $vf3z
	vsubz.xyw	$vf31xyw, $vf0xyw, $vf0z
	vsubz.xyw	$vf31xyw, $vf15xyw, $vf7z
	vsubz.xyw	$vf31xyw, $vf31xyw, $vf31z
	vsubz.xyz	$vf0xyz, $vf0xyz, $vf0z
	vsubz.xyz	$vf0xyz, $vf0xyz, $vf31z
	vsubz.xyz	$vf0xyz, $vf31xyz, $vf0z
	vsubz.xyz	$vf1xyz, $vf2xyz, $vf3z
	vsubz.xyz	$vf31xyz, $vf0xyz, $vf0z
	vsubz.xyz	$vf31xyz, $vf15xyz, $vf7z
	vsubz.xyz	$vf31xyz, $vf31xyz, $vf31z
	vsubz.xyzw	$vf0xyzw, $vf0xyzw, $vf0z
	vsubz.xyzw	$vf0xyzw, $vf0xyzw, $vf31z
	vsubz.xyzw	$vf0xyzw, $vf31xyzw, $vf0z
	vsubz.xyzw	$vf1xyzw, $vf2xyzw, $vf3z
	vsubz.xyzw	$vf31xyzw, $vf0xyzw, $vf0z
	vsubz.xyzw	$vf31xyzw, $vf15xyzw, $vf7z
	vsubz.xyzw	$vf31xyzw, $vf31xyzw, $vf31z
	vsubz.xz	$vf0xz, $vf0xz, $vf0z
	vsubz.xz	$vf0xz, $vf0xz, $vf31z
	vsubz.xz	$vf0xz, $vf31xz, $vf0z
	vsubz.xz	$vf1xz, $vf2xz, $vf3z
	vsubz.xz	$vf31xz, $vf0xz, $vf0z
	vsubz.xz	$vf31xz, $vf15xz, $vf7z
	vsubz.xz	$vf31xz, $vf31xz, $vf31z
	vsubz.xzw	$vf0xzw, $vf0xzw, $vf0z
	vsubz.xzw	$vf0xzw, $vf0xzw, $vf31z
	vsubz.xzw	$vf0xzw, $vf31xzw, $vf0z
	vsubz.xzw	$vf1xzw, $vf2xzw, $vf3z
	vsubz.xzw	$vf31xzw, $vf0xzw, $vf0z
	vsubz.xzw	$vf31xzw, $vf15xzw, $vf7z
	vsubz.xzw	$vf31xzw, $vf31xzw, $vf31z
	vsubz.y		$vf0y, $vf0y, $vf0z
	vsubz.y		$vf0y, $vf0y, $vf31z
	vsubz.y		$vf0y, $vf31y, $vf0z
	vsubz.y		$vf1y, $vf2y, $vf3z
	vsubz.y		$vf31y, $vf0y, $vf0z
	vsubz.y		$vf31y, $vf15y, $vf7z
	vsubz.y		$vf31y, $vf31y, $vf31z
	vsubz.yw	$vf0yw, $vf0yw, $vf0z
	vsubz.yw	$vf0yw, $vf0yw, $vf31z
	vsubz.yw	$vf0yw, $vf31yw, $vf0z
	vsubz.yw	$vf1yw, $vf2yw, $vf3z
	vsubz.yw	$vf31yw, $vf0yw, $vf0z
	vsubz.yw	$vf31yw, $vf15yw, $vf7z
	vsubz.yw	$vf31yw, $vf31yw, $vf31z
	vsubz.yz	$vf0yz, $vf0yz, $vf0z
	vsubz.yz	$vf0yz, $vf0yz, $vf31z
	vsubz.yz	$vf0yz, $vf31yz, $vf0z
	vsubz.yz	$vf1yz, $vf2yz, $vf3z
	vsubz.yz	$vf31yz, $vf0yz, $vf0z
	vsubz.yz	$vf31yz, $vf15yz, $vf7z
	vsubz.yz	$vf31yz, $vf31yz, $vf31z
	vsubz.yzw	$vf0yzw, $vf0yzw, $vf0z
	vsubz.yzw	$vf0yzw, $vf0yzw, $vf31z
	vsubz.yzw	$vf0yzw, $vf31yzw, $vf0z
	vsubz.yzw	$vf1yzw, $vf2yzw, $vf3z
	vsubz.yzw	$vf31yzw, $vf0yzw, $vf0z
	vsubz.yzw	$vf31yzw, $vf15yzw, $vf7z
	vsubz.yzw	$vf31yzw, $vf31yzw, $vf31z
	vsubz.z		$vf0z, $vf0z, $vf0z
	vsubz.z		$vf0z, $vf0z, $vf31z
	vsubz.z		$vf0z, $vf31z, $vf0z
	vsubz.z		$vf1z, $vf2z, $vf3z
	vsubz.z		$vf31z, $vf0z, $vf0z
	vsubz.z		$vf31z, $vf15z, $vf7z
	vsubz.z		$vf31z, $vf31z, $vf31z
	vsubz.zw	$vf0zw, $vf0zw, $vf0z
	vsubz.zw	$vf0zw, $vf0zw, $vf31z
	vsubz.zw	$vf0zw, $vf31zw, $vf0z
	vsubz.zw	$vf1zw, $vf2zw, $vf3z
	vsubz.zw	$vf31zw, $vf0zw, $vf0z
	vsubz.zw	$vf31zw, $vf15zw, $vf7z
	vsubz.zw	$vf31zw, $vf31zw, $vf31z
	vwaitq

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
