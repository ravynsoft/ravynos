# name: csky - v2-float
#as: -mcpu=ck810ef -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*f4243403\s*fstms\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4243003\s*fldms\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4243603\s*fstmm\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4243203\s*fldmm\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4243503\s*fstmd\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4243103\s*fldmd\s*fr3-fr4,\s*\(r4\)
\s*[0-9a-f]*:\s*f4022600\s*fstm\s*fr0,\s*\(r2,\s*0x0\)
\s*[0-9a-f]*:\s*f4022200\s*fldm\s*fr0,\s*\(r2,\s*0x0\)
\s*[0-9a-f]*:\s*f4022610\s*fstm\s*fr0,\s*\(r2,\s*0x8\)
\s*[0-9a-f]*:\s*f4022210\s*fldm\s*fr0,\s*\(r2,\s*0x8\)
\s*[0-9a-f]*:\s*f4022510\s*fstd\s*fr0,\s*\(r2,\s*0x4\)
\s*[0-9a-f]*:\s*f4022110\s*fldd\s*fr0,\s*\(r2,\s*0x4\)
\s*[0-9a-f]*:\s*f4022410\s*fsts\s*fr0,\s*\(r2,\s*0x4\)
\s*[0-9a-f]*:\s*f4022010\s*flds\s*fr0,\s*\(r2,\s*0x4\)
\s*[0-9a-f]*:\s*f4822e02\s*fstrm\s*fr2,\s*\(r2,\s*r4\s*<<\s*0\)
\s*[0-9a-f]*:\s*f4822e42\s*fstrm\s*fr2,\s*\(r2,\s*r4\s*<<\s*2\)
\s*[0-9a-f]*:\s*f4822d02\s*fstrd\s*fr2,\s*\(r2,\s*r4\s*<<\s*0\)
\s*[0-9a-f]*:\s*f4822d42\s*fstrd\s*fr2,\s*\(r2,\s*r4\s*<<\s*2\)
\s*[0-9a-f]*:\s*f4822c02\s*fstrs\s*fr2,\s*\(r2,\s*r4\s*<<\s*0\)
\s*[0-9a-f]*:\s*f4822c42\s*fstrs\s*fr2,\s*\(r2,\s*r4\s*<<\s*2\)
\s*[0-9a-f]*:\s*f4831222\s*fnmulm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48312e2\s*fnmscm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48312c2\s*fnmacm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48312a2\s*fmscm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4831282\s*fmacm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4831202\s*fmulm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4831022\s*fsubm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4831002\s*faddm\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830a22\s*fnmuld\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830ae2\s*fnmscd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830ac2\s*fnmacd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830aa2\s*fmscd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830a82\s*fmacd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830b02\s*fdivd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830a02\s*fmuld\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830822\s*fsubd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830802\s*faddd\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830222\s*fnmuls\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48302e2\s*fnmscs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48302c2\s*fnmacs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f48302a2\s*fmscs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830282\s*fmacs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830302\s*fdivs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830202\s*fmuls\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830022\s*fsubs\s*fr2,\s*fr3,\s*fr4
\s*[0-9a-f]*:\s*f4830002\s*fadds\s*fr2,\s*fr3,\s*fr4
#...
