# name: csky - all
#as: -mcpu=ck810f -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*f4000100\s*fcmpzhss\s*fr0
\s*[0-9a-f]*:\s*f4010120\s*fcmpzlss\s*fr1
\s*[0-9a-f]*:\s*f4050140\s*fcmpznes\s*fr5
\s*[0-9a-f]*:\s*f4080160\s*fcmpzuos\s*fr8
\s*[0-9a-f]*:\s*f40a0900\s*fcmpzhsd\s*fr10
\s*[0-9a-f]*:\s*f40d0920\s*fcmpzlsd\s*fr13
\s*[0-9a-f]*:\s*f40e0940\s*fcmpzned\s*fr14
\s*[0-9a-f]*:\s*f40f0960\s*fcmpzuod\s*fr15
\s*[0-9a-f]*:\s*f409008c\s*fmovs\s*fr12,\s*fr9
\s*[0-9a-f]*:\s*f40000c1\s*fabss\s*fr1,\s*fr0
\s*[0-9a-f]*:\s*f40100e2\s*fnegs\s*fr2,\s*fr1
\s*[0-9a-f]*:\s*f4470180\s*fcmphss\s*fr7,\s*fr2
\s*[0-9a-f]*:\s*f46601a0\s*fcmplts\s*fr6,\s*fr3
\s*[0-9a-f]*:\s*f48201c0\s*fcmpnes\s*fr2,\s*fr4
\s*[0-9a-f]*:\s*f4ac01e0\s*fcmpuos\s*fr12,\s*fr5
\s*[0-9a-f]*:\s*f406032b\s*frecips\s*fr11,\s*fr6
\s*[0-9a-f]*:\s*f407034a\s*fsqrts\s*fr10,\s*fr7
\s*[0-9a-f]*:\s*f4080889\s*fmovd\s*fr9,\s*fr8
\s*[0-9a-f]*:\s*f40908cd\s*fabsd\s*fr13,\s*fr9
\s*[0-9a-f]*:\s*f40a08ee\s*fnegd\s*fr14,\s*fr10
\s*[0-9a-f]*:\s*f56f0980\s*fcmphsd\s*fr15,\s*fr11
\s*[0-9a-f]*:\s*f58009a0\s*fcmpltd\s*fr0,\s*fr12
\s*[0-9a-f]*:\s*f5a309c0\s*fcmpned\s*fr3,\s*fr13
\s*[0-9a-f]*:\s*f5c409e0\s*fcmpuod\s*fr4,\s*fr14
\s*[0-9a-f]*:\s*f40f0b25\s*frecipd\s*fr5,\s*fr15
\s*[0-9a-f]*:\s*f4080b48\s*fsqrtd\s*fr8,\s*fr8
\s*[0-9a-f]*:\s*f4031081\s*fmovm\s*fr1,\s*fr3
\s*[0-9a-f]*:\s*f40d10c3\s*fabsm\s*fr3,\s*fr13
\s*[0-9a-f]*:\s*f40210ef\s*fnegm\s*fr15,\s*fr2
\s*[0-9a-f]*:\s*f402180f\s*fstosi.rn\s*fr15,\s*fr2
\s*[0-9a-f]*:\s*f402182e\s*fstosi.rz\s*fr14,\s*fr2
\s*[0-9a-f]*:\s*f40f184d\s*fstosi.rpi\s*fr13,\s*fr15
\s*[0-9a-f]*:\s*f40e186c\s*fstosi.rni\s*fr12,\s*fr14
\s*[0-9a-f]*:\s*f40d188b\s*fstoui.rn\s*fr11,\s*fr13
\s*[0-9a-f]*:\s*f40c18aa\s*fstoui.rz\s*fr10,\s*fr12
\s*[0-9a-f]*:\s*f40b18c9\s*fstoui.rpi\s*fr9,\s*fr11
\s*[0-9a-f]*:\s*f40a18e8\s*fstoui.rni\s*fr8,\s*fr10
\s*[0-9a-f]*:\s*f4091907\s*fdtosi.rn\s*fr7,\s*fr9
\s*[0-9a-f]*:\s*f4081926\s*fdtosi.rz\s*fr6,\s*fr8
\s*[0-9a-f]*:\s*f4071945\s*fdtosi.rpi\s*fr5,\s*fr7
\s*[0-9a-f]*:\s*f4061964\s*fdtosi.rni\s*fr4,\s*fr6
\s*[0-9a-f]*:\s*f4051983\s*fdtoui.rn\s*fr3,\s*fr5
\s*[0-9a-f]*:\s*f40419a2\s*fdtoui.rz\s*fr2,\s*fr4
\s*[0-9a-f]*:\s*f40319c1\s*fdtoui.rpi\s*fr1,\s*fr3
\s*[0-9a-f]*:\s*f40219e0\s*fdtoui.rni\s*fr0,\s*fr2
\s*[0-9a-f]*:\s*f4011a0e\s*fsitos\s*fr14,\s*fr1
\s*[0-9a-f]*:\s*f4001a2c\s*fuitos\s*fr12,\s*fr0
\s*[0-9a-f]*:\s*f4021a8d\s*fsitod\s*fr13,\s*fr2
\s*[0-9a-f]*:\s*f4041aab\s*fuitod\s*fr11,\s*fr4
\s*[0-9a-f]*:\s*f4081ac2\s*fdtos\s*fr2,\s*fr8
\s*[0-9a-f]*:\s*f40b1ae5\s*fstod\s*fr5,\s*fr11
#...
