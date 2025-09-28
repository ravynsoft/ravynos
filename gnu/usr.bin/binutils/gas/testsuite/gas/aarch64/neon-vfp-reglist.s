
	# ARMv8 tests to test neon register 
	# lists syntax.
	.macro ldnstn_reg_list type inst index rep
	\inst\()1\rep {v0.\type}\index, [x0]
	.ifb \index 
	.ifb \rep
	\inst\()1 {v0.\type, v1.\type}\index, [x0]
	\inst\()1 {v0.\type, v1.\type, v2.\type}\index, [x0]
	\inst\()1 {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0]
	.endif
	.endif

	\inst\()2\rep {v0.\type, v1.\type}\index, [x0]
	
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0]

	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0]

	.endm

	.text
	.arch armv8-a

	ldnstn_reg_list type="8B", inst="ld" index="" rep=""
	ldnstn_reg_list type="8B", inst="st" index="" rep=""

	ldnstn_reg_list type="16B", inst="ld" index="" rep=""
	ldnstn_reg_list type="16B", inst="st" index="" rep=""

	ldnstn_reg_list type="4H", inst="ld" index="" rep=""
	ldnstn_reg_list type="4H", inst="st" index="" rep=""

	ldnstn_reg_list type="8H", inst="ld" index="" rep=""
	ldnstn_reg_list type="8H", inst="st" index="" rep=""

	ldnstn_reg_list type="2S", inst="ld" index="" rep=""
	ldnstn_reg_list type="2S", inst="st" index="" rep=""

	ldnstn_reg_list type="4S", inst="ld" index="" rep=""
	ldnstn_reg_list type="4S", inst="st" index="" rep=""

	ldnstn_reg_list type="2D", inst="ld" index="" rep=""
	ldnstn_reg_list type="2D", inst="st" index="" rep=""

	# vector-element form
	ldnstn_reg_list type="B", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="B", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="B", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="B", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="H", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="H", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="H", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="H", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="S", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="S", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="S", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="S", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="D", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="D", inst="st" index="[1]" rep=""

	# replicate form
	ldnstn_reg_list type="8B", inst="ld" index="" rep="r"

	ldnstn_reg_list type="16B", inst="ld" index="" rep="r"

	ldnstn_reg_list type="4H", inst="ld" index="" rep="r"

	ldnstn_reg_list type="8H", inst="ld" index="" rep="r"

	ldnstn_reg_list type="2S", inst="ld" index="" rep="r"

	ldnstn_reg_list type="4S", inst="ld" index="" rep="r"

	ldnstn_reg_list type="1D", inst="ld" index="" rep="r"

	ldnstn_reg_list type="2D", inst="ld" index="" rep="r"
