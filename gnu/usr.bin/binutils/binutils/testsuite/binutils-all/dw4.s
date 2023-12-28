	.file	"main.c"
	.text
.Ltext0:
	.text
	.globl	main
	.type	main, %function
main:
.LFB0:
	.file 1 "main.c"
	.loc 1 6 1 view -0
	.word 1234
	.word 5678
.LFE0:
	.size	main, . - main
	.globl	g_my_private_global
	.data
	.align 4
	.type	g_my_private_global, %object
	.size	g_my_private_global, 4
g_my_private_global:
	.zero	4
	.globl	g_my_externd_global
	.data
	.align 4
	.type	g_my_externd_global, %object
	.size	g_my_externd_global, 4
g_my_externd_global:
	.zero	4
	.text
.Letext0:
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.long	.Linfo_end - .Linfo_start
.Linfo_start:
	.short	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF345
	.byte	0xc
	.long	.LASF346
	.long	.LASF347
	.long	.Ldebug_ranges0 + 0
	.quad	0
	.long	.Ldebug_line0
	.long	.Ldebug_macro0
.Lvar_decl:
	.uleb128 0x2
	.long	.LASF343
	.byte	0x1
	.byte	0x2
	.byte	0xc
	.long	0x39
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x4
	.long	.Lvar_decl - .Ldebug_info0
	.byte	0x3
	.byte	0x5
	.uleb128 .Lblock1_end - .Lblock1_start
.Lblock1_start:
	.byte	0x3
	.dc.a	g_my_externd_global
.Lblock1_end:
	.uleb128 0x5
	.long	.LASF344
	.byte	0x1
	.byte	0x4
	.byte	0x5
	.long	0x39
	.uleb128 .Lblock2_end - .Lblock2_start
.Lblock2_start:
	.byte	0x3
	.dc.a	g_my_private_global
.Lblock2_end:
	.uleb128 0x6
	.long	.LASF348
	.byte	0x1
	.byte	0x5
	.byte	0x5
	.long	0x39
	.quad	0
	.quad	0x10
	.uleb128 0x1
	.byte	0x9c
	.byte	0
.Linfo_end:
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.uleb128 0x2119
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x34
	.byte	0
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",%progbits
	.long	0x2c
	.short	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.short	0
	.short	0
	.dc.a	.LFB0
	.quad	.LFE0 - .LFB0
	.quad	0
	.quad	0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.dc.a	.LFB0
	.dc.a	.LFE0
	.quad	0
	.quad	0
	.section	.debug_macro,"",%progbits
.Ldebug_macro0:
	.short	0x4
	.byte	0x2
	.long	.Ldebug_line0
	.byte	0x3
	.uleb128 0
	.uleb128 0x1
	.byte	0x5
	.uleb128 0x1
	.long	.LASF0
	.byte	0x5
	.uleb128 0x2
	.long	.LASF1
	.byte	0x5
	.uleb128 0x3
	.long	.LASF2
	.byte	0x5
	.uleb128 0x4
	.long	.LASF3
	.byte	0x5
	.uleb128 0x5
	.long	.LASF4
	.byte	0x5
	.uleb128 0x6
	.long	.LASF5
	.byte	0x5
	.uleb128 0x7
	.long	.LASF6
	.byte	0x5
	.uleb128 0x8
	.long	.LASF7
	.byte	0x5
	.uleb128 0x9
	.long	.LASF8
	.byte	0x5
	.uleb128 0xa
	.long	.LASF9
	.byte	0x5
	.uleb128 0xb
	.long	.LASF10
	.byte	0x5
	.uleb128 0xc
	.long	.LASF11
	.byte	0x5
	.uleb128 0xd
	.long	.LASF12
	.byte	0x5
	.uleb128 0xe
	.long	.LASF13
	.byte	0x5
	.uleb128 0xf
	.long	.LASF14
	.byte	0x5
	.uleb128 0x10
	.long	.LASF15
	.byte	0x5
	.uleb128 0x11
	.long	.LASF16
	.byte	0x5
	.uleb128 0x12
	.long	.LASF17
	.byte	0x5
	.uleb128 0x13
	.long	.LASF18
	.byte	0x5
	.uleb128 0x14
	.long	.LASF19
	.byte	0x5
	.uleb128 0x15
	.long	.LASF20
	.byte	0x5
	.uleb128 0x16
	.long	.LASF21
	.byte	0x5
	.uleb128 0x17
	.long	.LASF22
	.byte	0x5
	.uleb128 0x18
	.long	.LASF23
	.byte	0x5
	.uleb128 0x19
	.long	.LASF24
	.byte	0x5
	.uleb128 0x1a
	.long	.LASF25
	.byte	0x5
	.uleb128 0x1b
	.long	.LASF26
	.byte	0x5
	.uleb128 0x1c
	.long	.LASF27
	.byte	0x5
	.uleb128 0x1d
	.long	.LASF28
	.byte	0x5
	.uleb128 0x1e
	.long	.LASF29
	.byte	0x5
	.uleb128 0x1f
	.long	.LASF30
	.byte	0x5
	.uleb128 0x20
	.long	.LASF31
	.byte	0x5
	.uleb128 0x21
	.long	.LASF32
	.byte	0x5
	.uleb128 0x22
	.long	.LASF33
	.byte	0x5
	.uleb128 0x23
	.long	.LASF34
	.byte	0x5
	.uleb128 0x24
	.long	.LASF35
	.byte	0x5
	.uleb128 0x25
	.long	.LASF36
	.byte	0x5
	.uleb128 0x26
	.long	.LASF37
	.byte	0x5
	.uleb128 0x27
	.long	.LASF38
	.byte	0x5
	.uleb128 0x28
	.long	.LASF39
	.byte	0x5
	.uleb128 0x29
	.long	.LASF40
	.byte	0x5
	.uleb128 0x2a
	.long	.LASF41
	.byte	0x5
	.uleb128 0x2b
	.long	.LASF42
	.byte	0x5
	.uleb128 0x2c
	.long	.LASF43
	.byte	0x5
	.uleb128 0x2d
	.long	.LASF44
	.byte	0x5
	.uleb128 0x2e
	.long	.LASF45
	.byte	0x5
	.uleb128 0x2f
	.long	.LASF46
	.byte	0x5
	.uleb128 0x30
	.long	.LASF47
	.byte	0x5
	.uleb128 0x31
	.long	.LASF48
	.byte	0x5
	.uleb128 0x32
	.long	.LASF49
	.byte	0x5
	.uleb128 0x33
	.long	.LASF50
	.byte	0x5
	.uleb128 0x34
	.long	.LASF51
	.byte	0x5
	.uleb128 0x35
	.long	.LASF52
	.byte	0x5
	.uleb128 0x36
	.long	.LASF53
	.byte	0x5
	.uleb128 0x37
	.long	.LASF54
	.byte	0x5
	.uleb128 0x38
	.long	.LASF55
	.byte	0x5
	.uleb128 0x39
	.long	.LASF56
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF57
	.byte	0x5
	.uleb128 0x3b
	.long	.LASF58
	.byte	0x5
	.uleb128 0x3c
	.long	.LASF59
	.byte	0x5
	.uleb128 0x3d
	.long	.LASF60
	.byte	0x5
	.uleb128 0x3e
	.long	.LASF61
	.byte	0x5
	.uleb128 0x3f
	.long	.LASF62
	.byte	0x5
	.uleb128 0x40
	.long	.LASF63
	.byte	0x5
	.uleb128 0x41
	.long	.LASF64
	.byte	0x5
	.uleb128 0x42
	.long	.LASF65
	.byte	0x5
	.uleb128 0x43
	.long	.LASF66
	.byte	0x5
	.uleb128 0x44
	.long	.LASF67
	.byte	0x5
	.uleb128 0x45
	.long	.LASF68
	.byte	0x5
	.uleb128 0x46
	.long	.LASF69
	.byte	0x5
	.uleb128 0x47
	.long	.LASF70
	.byte	0x5
	.uleb128 0x48
	.long	.LASF71
	.byte	0x5
	.uleb128 0x49
	.long	.LASF72
	.byte	0x5
	.uleb128 0x4a
	.long	.LASF73
	.byte	0x5
	.uleb128 0x4b
	.long	.LASF74
	.byte	0x5
	.uleb128 0x4c
	.long	.LASF75
	.byte	0x5
	.uleb128 0x4d
	.long	.LASF76
	.byte	0x5
	.uleb128 0x4e
	.long	.LASF77
	.byte	0x5
	.uleb128 0x4f
	.long	.LASF78
	.byte	0x5
	.uleb128 0x50
	.long	.LASF79
	.byte	0x5
	.uleb128 0x51
	.long	.LASF80
	.byte	0x5
	.uleb128 0x52
	.long	.LASF81
	.byte	0x5
	.uleb128 0x53
	.long	.LASF82
	.byte	0x5
	.uleb128 0x54
	.long	.LASF83
	.byte	0x5
	.uleb128 0x55
	.long	.LASF84
	.byte	0x5
	.uleb128 0x56
	.long	.LASF85
	.byte	0x5
	.uleb128 0x57
	.long	.LASF86
	.byte	0x5
	.uleb128 0x58
	.long	.LASF87
	.byte	0x5
	.uleb128 0x59
	.long	.LASF88
	.byte	0x5
	.uleb128 0x5a
	.long	.LASF89
	.byte	0x5
	.uleb128 0x5b
	.long	.LASF90
	.byte	0x5
	.uleb128 0x5c
	.long	.LASF91
	.byte	0x5
	.uleb128 0x5d
	.long	.LASF92
	.byte	0x5
	.uleb128 0x5e
	.long	.LASF93
	.byte	0x5
	.uleb128 0x5f
	.long	.LASF94
	.byte	0x5
	.uleb128 0x60
	.long	.LASF95
	.byte	0x5
	.uleb128 0x61
	.long	.LASF96
	.byte	0x5
	.uleb128 0x62
	.long	.LASF97
	.byte	0x5
	.uleb128 0x63
	.long	.LASF98
	.byte	0x5
	.uleb128 0x64
	.long	.LASF99
	.byte	0x5
	.uleb128 0x65
	.long	.LASF100
	.byte	0x5
	.uleb128 0x66
	.long	.LASF101
	.byte	0x5
	.uleb128 0x67
	.long	.LASF102
	.byte	0x5
	.uleb128 0x68
	.long	.LASF103
	.byte	0x5
	.uleb128 0x69
	.long	.LASF104
	.byte	0x5
	.uleb128 0x6a
	.long	.LASF105
	.byte	0x5
	.uleb128 0x6b
	.long	.LASF106
	.byte	0x5
	.uleb128 0x6c
	.long	.LASF107
	.byte	0x5
	.uleb128 0x6d
	.long	.LASF108
	.byte	0x5
	.uleb128 0x6e
	.long	.LASF109
	.byte	0x5
	.uleb128 0x6f
	.long	.LASF110
	.byte	0x5
	.uleb128 0x70
	.long	.LASF111
	.byte	0x5
	.uleb128 0x71
	.long	.LASF112
	.byte	0x5
	.uleb128 0x72
	.long	.LASF113
	.byte	0x5
	.uleb128 0x73
	.long	.LASF114
	.byte	0x5
	.uleb128 0x74
	.long	.LASF115
	.byte	0x5
	.uleb128 0x75
	.long	.LASF116
	.byte	0x5
	.uleb128 0x76
	.long	.LASF117
	.byte	0x5
	.uleb128 0x77
	.long	.LASF118
	.byte	0x5
	.uleb128 0x78
	.long	.LASF119
	.byte	0x5
	.uleb128 0x79
	.long	.LASF120
	.byte	0x5
	.uleb128 0x7a
	.long	.LASF121
	.byte	0x5
	.uleb128 0x7b
	.long	.LASF122
	.byte	0x5
	.uleb128 0x7c
	.long	.LASF123
	.byte	0x5
	.uleb128 0x7d
	.long	.LASF124
	.byte	0x5
	.uleb128 0x7e
	.long	.LASF125
	.byte	0x5
	.uleb128 0x7f
	.long	.LASF126
	.byte	0x5
	.uleb128 0x80
	.long	.LASF127
	.byte	0x5
	.uleb128 0x81
	.long	.LASF128
	.byte	0x5
	.uleb128 0x82
	.long	.LASF129
	.byte	0x5
	.uleb128 0x83
	.long	.LASF130
	.byte	0x5
	.uleb128 0x84
	.long	.LASF131
	.byte	0x5
	.uleb128 0x85
	.long	.LASF132
	.byte	0x5
	.uleb128 0x86
	.long	.LASF133
	.byte	0x5
	.uleb128 0x87
	.long	.LASF134
	.byte	0x5
	.uleb128 0x88
	.long	.LASF135
	.byte	0x5
	.uleb128 0x89
	.long	.LASF136
	.byte	0x5
	.uleb128 0x8a
	.long	.LASF137
	.byte	0x5
	.uleb128 0x8b
	.long	.LASF138
	.byte	0x5
	.uleb128 0x8c
	.long	.LASF139
	.byte	0x5
	.uleb128 0x8d
	.long	.LASF140
	.byte	0x5
	.uleb128 0x8e
	.long	.LASF141
	.byte	0x5
	.uleb128 0x8f
	.long	.LASF142
	.byte	0x5
	.uleb128 0x90
	.long	.LASF143
	.byte	0x5
	.uleb128 0x91
	.long	.LASF144
	.byte	0x5
	.uleb128 0x92
	.long	.LASF145
	.byte	0x5
	.uleb128 0x93
	.long	.LASF146
	.byte	0x5
	.uleb128 0x94
	.long	.LASF147
	.byte	0x5
	.uleb128 0x95
	.long	.LASF148
	.byte	0x5
	.uleb128 0x96
	.long	.LASF149
	.byte	0x5
	.uleb128 0x97
	.long	.LASF150
	.byte	0x5
	.uleb128 0x98
	.long	.LASF151
	.byte	0x5
	.uleb128 0x99
	.long	.LASF152
	.byte	0x5
	.uleb128 0x9a
	.long	.LASF153
	.byte	0x5
	.uleb128 0x9b
	.long	.LASF154
	.byte	0x5
	.uleb128 0x9c
	.long	.LASF155
	.byte	0x5
	.uleb128 0x9d
	.long	.LASF156
	.byte	0x5
	.uleb128 0x9e
	.long	.LASF157
	.byte	0x5
	.uleb128 0x9f
	.long	.LASF158
	.byte	0x5
	.uleb128 0xa0
	.long	.LASF159
	.byte	0x5
	.uleb128 0xa1
	.long	.LASF160
	.byte	0x5
	.uleb128 0xa2
	.long	.LASF161
	.byte	0x5
	.uleb128 0xa3
	.long	.LASF162
	.byte	0x5
	.uleb128 0xa4
	.long	.LASF163
	.byte	0x5
	.uleb128 0xa5
	.long	.LASF164
	.byte	0x5
	.uleb128 0xa6
	.long	.LASF165
	.byte	0x5
	.uleb128 0xa7
	.long	.LASF166
	.byte	0x5
	.uleb128 0xa8
	.long	.LASF167
	.byte	0x5
	.uleb128 0xa9
	.long	.LASF168
	.byte	0x5
	.uleb128 0xaa
	.long	.LASF169
	.byte	0x5
	.uleb128 0xab
	.long	.LASF170
	.byte	0x5
	.uleb128 0xac
	.long	.LASF171
	.byte	0x5
	.uleb128 0xad
	.long	.LASF172
	.byte	0x5
	.uleb128 0xae
	.long	.LASF173
	.byte	0x5
	.uleb128 0xaf
	.long	.LASF174
	.byte	0x5
	.uleb128 0xb0
	.long	.LASF175
	.byte	0x5
	.uleb128 0xb1
	.long	.LASF176
	.byte	0x5
	.uleb128 0xb2
	.long	.LASF177
	.byte	0x5
	.uleb128 0xb3
	.long	.LASF178
	.byte	0x5
	.uleb128 0xb4
	.long	.LASF179
	.byte	0x5
	.uleb128 0xb5
	.long	.LASF180
	.byte	0x5
	.uleb128 0xb6
	.long	.LASF181
	.byte	0x5
	.uleb128 0xb7
	.long	.LASF182
	.byte	0x5
	.uleb128 0xb8
	.long	.LASF183
	.byte	0x5
	.uleb128 0xb9
	.long	.LASF184
	.byte	0x5
	.uleb128 0xba
	.long	.LASF185
	.byte	0x5
	.uleb128 0xbb
	.long	.LASF186
	.byte	0x5
	.uleb128 0xbc
	.long	.LASF187
	.byte	0x5
	.uleb128 0xbd
	.long	.LASF188
	.byte	0x5
	.uleb128 0xbe
	.long	.LASF189
	.byte	0x5
	.uleb128 0xbf
	.long	.LASF190
	.byte	0x5
	.uleb128 0xc0
	.long	.LASF191
	.byte	0x5
	.uleb128 0xc1
	.long	.LASF192
	.byte	0x5
	.uleb128 0xc2
	.long	.LASF193
	.byte	0x5
	.uleb128 0xc3
	.long	.LASF194
	.byte	0x5
	.uleb128 0xc4
	.long	.LASF195
	.byte	0x5
	.uleb128 0xc5
	.long	.LASF196
	.byte	0x5
	.uleb128 0xc6
	.long	.LASF197
	.byte	0x5
	.uleb128 0xc7
	.long	.LASF198
	.byte	0x5
	.uleb128 0xc8
	.long	.LASF199
	.byte	0x5
	.uleb128 0xc9
	.long	.LASF200
	.byte	0x5
	.uleb128 0xca
	.long	.LASF201
	.byte	0x5
	.uleb128 0xcb
	.long	.LASF202
	.byte	0x5
	.uleb128 0xcc
	.long	.LASF203
	.byte	0x5
	.uleb128 0xcd
	.long	.LASF204
	.byte	0x5
	.uleb128 0xce
	.long	.LASF205
	.byte	0x5
	.uleb128 0xcf
	.long	.LASF206
	.byte	0x5
	.uleb128 0xd0
	.long	.LASF207
	.byte	0x5
	.uleb128 0xd1
	.long	.LASF208
	.byte	0x5
	.uleb128 0xd2
	.long	.LASF209
	.byte	0x5
	.uleb128 0xd3
	.long	.LASF210
	.byte	0x5
	.uleb128 0xd4
	.long	.LASF211
	.byte	0x5
	.uleb128 0xd5
	.long	.LASF212
	.byte	0x5
	.uleb128 0xd6
	.long	.LASF213
	.byte	0x5
	.uleb128 0xd7
	.long	.LASF214
	.byte	0x5
	.uleb128 0xd8
	.long	.LASF215
	.byte	0x5
	.uleb128 0xd9
	.long	.LASF216
	.byte	0x5
	.uleb128 0xda
	.long	.LASF217
	.byte	0x5
	.uleb128 0xdb
	.long	.LASF218
	.byte	0x5
	.uleb128 0xdc
	.long	.LASF219
	.byte	0x5
	.uleb128 0xdd
	.long	.LASF220
	.byte	0x5
	.uleb128 0xde
	.long	.LASF221
	.byte	0x5
	.uleb128 0xdf
	.long	.LASF222
	.byte	0x5
	.uleb128 0xe0
	.long	.LASF223
	.byte	0x5
	.uleb128 0xe1
	.long	.LASF224
	.byte	0x5
	.uleb128 0xe2
	.long	.LASF225
	.byte	0x5
	.uleb128 0xe3
	.long	.LASF226
	.byte	0x5
	.uleb128 0xe4
	.long	.LASF227
	.byte	0x5
	.uleb128 0xe5
	.long	.LASF228
	.byte	0x5
	.uleb128 0xe6
	.long	.LASF229
	.byte	0x5
	.uleb128 0xe7
	.long	.LASF230
	.byte	0x5
	.uleb128 0xe8
	.long	.LASF231
	.byte	0x5
	.uleb128 0xe9
	.long	.LASF232
	.byte	0x5
	.uleb128 0xea
	.long	.LASF233
	.byte	0x5
	.uleb128 0xeb
	.long	.LASF234
	.byte	0x5
	.uleb128 0xec
	.long	.LASF235
	.byte	0x5
	.uleb128 0xed
	.long	.LASF236
	.byte	0x5
	.uleb128 0xee
	.long	.LASF237
	.byte	0x5
	.uleb128 0xef
	.long	.LASF238
	.byte	0x5
	.uleb128 0xf0
	.long	.LASF239
	.byte	0x5
	.uleb128 0xf1
	.long	.LASF240
	.byte	0x5
	.uleb128 0xf2
	.long	.LASF241
	.byte	0x5
	.uleb128 0xf3
	.long	.LASF242
	.byte	0x5
	.uleb128 0xf4
	.long	.LASF243
	.byte	0x5
	.uleb128 0xf5
	.long	.LASF244
	.byte	0x5
	.uleb128 0xf6
	.long	.LASF245
	.byte	0x5
	.uleb128 0xf7
	.long	.LASF246
	.byte	0x5
	.uleb128 0xf8
	.long	.LASF247
	.byte	0x5
	.uleb128 0xf9
	.long	.LASF248
	.byte	0x5
	.uleb128 0xfa
	.long	.LASF249
	.byte	0x5
	.uleb128 0xfb
	.long	.LASF250
	.byte	0x5
	.uleb128 0xfc
	.long	.LASF251
	.byte	0x5
	.uleb128 0xfd
	.long	.LASF252
	.byte	0x5
	.uleb128 0xfe
	.long	.LASF253
	.byte	0x5
	.uleb128 0xff
	.long	.LASF254
	.byte	0x5
	.uleb128 0x100
	.long	.LASF255
	.byte	0x5
	.uleb128 0x101
	.long	.LASF256
	.byte	0x5
	.uleb128 0x102
	.long	.LASF257
	.byte	0x5
	.uleb128 0x103
	.long	.LASF258
	.byte	0x5
	.uleb128 0x104
	.long	.LASF259
	.byte	0x5
	.uleb128 0x105
	.long	.LASF260
	.byte	0x5
	.uleb128 0x106
	.long	.LASF261
	.byte	0x5
	.uleb128 0x107
	.long	.LASF262
	.byte	0x5
	.uleb128 0x108
	.long	.LASF263
	.byte	0x5
	.uleb128 0x109
	.long	.LASF264
	.byte	0x5
	.uleb128 0x10a
	.long	.LASF265
	.byte	0x5
	.uleb128 0x10b
	.long	.LASF266
	.byte	0x5
	.uleb128 0x10c
	.long	.LASF267
	.byte	0x5
	.uleb128 0x10d
	.long	.LASF268
	.byte	0x5
	.uleb128 0x10e
	.long	.LASF269
	.byte	0x5
	.uleb128 0x10f
	.long	.LASF270
	.byte	0x5
	.uleb128 0x110
	.long	.LASF271
	.byte	0x5
	.uleb128 0x111
	.long	.LASF272
	.byte	0x5
	.uleb128 0x112
	.long	.LASF273
	.byte	0x5
	.uleb128 0x113
	.long	.LASF274
	.byte	0x5
	.uleb128 0x114
	.long	.LASF275
	.byte	0x5
	.uleb128 0x115
	.long	.LASF276
	.byte	0x5
	.uleb128 0x116
	.long	.LASF277
	.byte	0x5
	.uleb128 0x117
	.long	.LASF278
	.byte	0x5
	.uleb128 0x118
	.long	.LASF279
	.byte	0x5
	.uleb128 0x119
	.long	.LASF280
	.byte	0x5
	.uleb128 0x11a
	.long	.LASF281
	.byte	0x5
	.uleb128 0x11b
	.long	.LASF282
	.byte	0x5
	.uleb128 0x11c
	.long	.LASF283
	.byte	0x5
	.uleb128 0x11d
	.long	.LASF284
	.byte	0x5
	.uleb128 0x11e
	.long	.LASF285
	.byte	0x5
	.uleb128 0x11f
	.long	.LASF286
	.byte	0x5
	.uleb128 0x120
	.long	.LASF287
	.byte	0x5
	.uleb128 0x121
	.long	.LASF288
	.byte	0x5
	.uleb128 0x122
	.long	.LASF289
	.byte	0x5
	.uleb128 0x123
	.long	.LASF290
	.byte	0x5
	.uleb128 0x124
	.long	.LASF291
	.byte	0x5
	.uleb128 0x125
	.long	.LASF292
	.byte	0x5
	.uleb128 0x126
	.long	.LASF293
	.byte	0x5
	.uleb128 0x127
	.long	.LASF294
	.byte	0x5
	.uleb128 0x128
	.long	.LASF295
	.byte	0x5
	.uleb128 0x129
	.long	.LASF296
	.byte	0x5
	.uleb128 0x12a
	.long	.LASF297
	.byte	0x5
	.uleb128 0x12b
	.long	.LASF298
	.byte	0x5
	.uleb128 0x12c
	.long	.LASF299
	.byte	0x5
	.uleb128 0x12d
	.long	.LASF300
	.byte	0x5
	.uleb128 0x12e
	.long	.LASF301
	.byte	0x5
	.uleb128 0x12f
	.long	.LASF302
	.byte	0x5
	.uleb128 0x130
	.long	.LASF303
	.byte	0x5
	.uleb128 0x131
	.long	.LASF304
	.byte	0x5
	.uleb128 0x132
	.long	.LASF305
	.byte	0x5
	.uleb128 0x133
	.long	.LASF306
	.byte	0x5
	.uleb128 0x134
	.long	.LASF307
	.byte	0x5
	.uleb128 0x135
	.long	.LASF308
	.byte	0x5
	.uleb128 0x136
	.long	.LASF309
	.byte	0x5
	.uleb128 0x137
	.long	.LASF310
	.byte	0x5
	.uleb128 0x138
	.long	.LASF311
	.byte	0x5
	.uleb128 0x139
	.long	.LASF312
	.byte	0x5
	.uleb128 0x13a
	.long	.LASF313
	.byte	0x5
	.uleb128 0x13b
	.long	.LASF314
	.byte	0x5
	.uleb128 0x13c
	.long	.LASF315
	.byte	0x5
	.uleb128 0x13d
	.long	.LASF316
	.byte	0x5
	.uleb128 0x13e
	.long	.LASF317
	.byte	0x5
	.uleb128 0x13f
	.long	.LASF318
	.byte	0x5
	.uleb128 0x140
	.long	.LASF319
	.byte	0x5
	.uleb128 0x141
	.long	.LASF320
	.byte	0x5
	.uleb128 0x142
	.long	.LASF321
	.byte	0x5
	.uleb128 0x143
	.long	.LASF322
	.byte	0x5
	.uleb128 0x144
	.long	.LASF323
	.byte	0x5
	.uleb128 0x145
	.long	.LASF324
	.byte	0x5
	.uleb128 0x146
	.long	.LASF325
	.byte	0x5
	.uleb128 0x147
	.long	.LASF326
	.byte	0x5
	.uleb128 0x148
	.long	.LASF327
	.byte	0x5
	.uleb128 0x149
	.long	.LASF328
	.byte	0x5
	.uleb128 0x14a
	.long	.LASF329
	.byte	0x5
	.uleb128 0x14b
	.long	.LASF330
	.byte	0x5
	.uleb128 0x14c
	.long	.LASF331
	.byte	0x5
	.uleb128 0x14d
	.long	.LASF332
	.byte	0x5
	.uleb128 0x14e
	.long	.LASF333
	.byte	0x5
	.uleb128 0x14f
	.long	.LASF334
	.byte	0x5
	.uleb128 0x150
	.long	.LASF335
	.byte	0x5
	.uleb128 0x151
	.long	.LASF336
	.byte	0x5
	.uleb128 0x152
	.long	.LASF337
	.byte	0x5
	.uleb128 0x153
	.long	.LASF338
	.file 2 "/usr/include/stdc-predef.h"
	.byte	0x3
	.uleb128 0x1f
	.uleb128 0x2
	.byte	0x7
	.long	.Ldebug_macro2
	.byte	0x4
	.byte	0x4
	.byte	0
	.section	.debug_macro,"G",%progbits,wm4.stdcpredef.h.19.8dc41bed5d9037ff9622e015fb5f0ce3,comdat
.Ldebug_macro2:
	.short	0x4
	.byte	0
	.byte	0x5
	.uleb128 0x13
	.long	.LASF339
	.byte	0x5
	.uleb128 0x26
	.long	.LASF340
	.byte	0x5
	.uleb128 0x2e
	.long	.LASF341
	.byte	0x5
	.uleb128 0x3a
	.long	.LASF342
	.byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF122:
	.string	"__UINT_LEAST8_MAX__ 0xff"
.LASF219:
	.string	"__FLT64_HAS_DENORM__ 1"
.LASF250:
	.string	"__FLT64X_MANT_DIG__ 64"
.LASF2:
	.string	"__STDC_UTF_16__ 1"
.LASF252:
	.string	"__FLT64X_MIN_EXP__ (-16381)"
.LASF126:
	.string	"__UINT_LEAST32_MAX__ 0xffffffffU"
.LASF160:
	.string	"__FLT_EPSILON__ 1.19209289550781250000000000000000000e-7F"
.LASF8:
	.string	"__VERSION__ \"9.2.1 20190827 (Red Hat 9.2.1-1)\""
.LASF247:
	.string	"__FLT32X_HAS_DENORM__ 1"
.LASF334:
	.string	"__unix 1"
.LASF70:
	.string	"__UINTPTR_TYPE__ long unsigned int"
.LASF35:
	.string	"__SIZEOF_POINTER__ 8"
.LASF90:
	.string	"__WCHAR_WIDTH__ 32"
.LASF279:
	.string	"__DEC128_MIN_EXP__ (-6142)"
.LASF271:
	.string	"__DEC64_MANT_DIG__ 16"
.LASF272:
	.string	"__DEC64_MIN_EXP__ (-382)"
.LASF173:
	.string	"__DBL_MIN__ ((double)2.22507385850720138309023271733240406e-308L)"
.LASF141:
	.string	"__UINT_FAST64_MAX__ 0xffffffffffffffffUL"
.LASF335:
	.string	"__unix__ 1"
.LASF240:
	.string	"__FLT32X_MAX_EXP__ 1024"
.LASF184:
	.string	"__LDBL_MAX_10_EXP__ 4932"
.LASF267:
	.string	"__DEC32_MIN__ 1E-95DF"
.LASF215:
	.string	"__FLT64_MAX__ 1.79769313486231570814527423731704357e+308F64"
.LASF249:
	.string	"__FLT32X_HAS_QUIET_NAN__ 1"
.LASF265:
	.string	"__DEC32_MIN_EXP__ (-94)"
.LASF234:
	.string	"__FLT128_HAS_INFINITY__ 1"
.LASF170:
	.string	"__DBL_MAX_10_EXP__ 308"
.LASF216:
	.string	"__FLT64_MIN__ 2.22507385850720138309023271733240406e-308F64"
.LASF310:
	.string	"__amd64 1"
.LASF188:
	.string	"__LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L"
.LASF118:
	.string	"__INT_LEAST32_WIDTH__ 32"
.LASF144:
	.string	"__UINTPTR_MAX__ 0xffffffffffffffffUL"
.LASF19:
	.string	"__LP64__ 1"
.LASF129:
	.string	"__UINT64_C(c) c ## UL"
.LASF177:
	.string	"__DBL_HAS_INFINITY__ 1"
.LASF1:
	.string	"__STDC_VERSION__ 201710L"
.LASF321:
	.string	"__code_model_small__ 1"
.LASF239:
	.string	"__FLT32X_MIN_10_EXP__ (-307)"
.LASF117:
	.string	"__INT32_C(c) c"
.LASF245:
	.string	"__FLT32X_EPSILON__ 2.22044604925031308084726333618164062e-16F32x"
.LASF7:
	.string	"__GNUC_PATCHLEVEL__ 1"
.LASF261:
	.string	"__FLT64X_HAS_DENORM__ 1"
.LASF143:
	.string	"__INTPTR_WIDTH__ 64"
.LASF4:
	.string	"__STDC_HOSTED__ 1"
.LASF82:
	.string	"__WINT_MIN__ 0U"
.LASF148:
	.string	"__FLT_EVAL_METHOD_TS_18661_3__ 0"
.LASF190:
	.string	"__LDBL_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951L"
.LASF174:
	.string	"__DBL_EPSILON__ ((double)2.22044604925031308084726333618164062e-16L)"
.LASF253:
	.string	"__FLT64X_MIN_10_EXP__ (-4931)"
.LASF296:
	.string	"__GCC_ATOMIC_WCHAR_T_LOCK_FREE 2"
.LASF51:
	.string	"__UINT32_TYPE__ unsigned int"
.LASF325:
	.string	"__FXSR__ 1"
.LASF221:
	.string	"__FLT64_HAS_QUIET_NAN__ 1"
.LASF277:
	.string	"__DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD"
.LASF85:
	.string	"__SCHAR_WIDTH__ 8"
.LASF189:
	.string	"__LDBL_EPSILON__ 1.08420217248550443400745280086994171e-19L"
.LASF165:
	.string	"__DBL_MANT_DIG__ 53"
.LASF203:
	.string	"__FLT32_EPSILON__ 1.19209289550781250000000000000000000e-7F32"
.LASF204:
	.string	"__FLT32_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F32"
.LASF130:
	.string	"__INT_FAST8_MAX__ 0x7f"
.LASF178:
	.string	"__DBL_HAS_QUIET_NAN__ 1"
.LASF217:
	.string	"__FLT64_EPSILON__ 2.22044604925031308084726333618164062e-16F64"
.LASF86:
	.string	"__SHRT_WIDTH__ 16"
.LASF266:
	.string	"__DEC32_MAX_EXP__ 97"
.LASF47:
	.string	"__INT32_TYPE__ int"
.LASF44:
	.string	"__SIG_ATOMIC_TYPE__ int"
.LASF273:
	.string	"__DEC64_MAX_EXP__ 385"
.LASF167:
	.string	"__DBL_MIN_EXP__ (-1021)"
.LASF18:
	.string	"_LP64 1"
.LASF115:
	.string	"__INT_LEAST16_WIDTH__ 16"
.LASF192:
	.string	"__LDBL_HAS_INFINITY__ 1"
.LASF64:
	.string	"__INT_FAST64_TYPE__ long int"
.LASF162:
	.string	"__FLT_HAS_DENORM__ 1"
.LASF224:
	.string	"__FLT128_MIN_EXP__ (-16381)"
.LASF231:
	.string	"__FLT128_EPSILON__ 1.92592994438723585305597794258492732e-34F128"
.LASF107:
	.string	"__UINT16_MAX__ 0xffff"
.LASF263:
	.string	"__FLT64X_HAS_QUIET_NAN__ 1"
.LASF68:
	.string	"__UINT_FAST64_TYPE__ long unsigned int"
.LASF33:
	.string	"__BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__"
.LASF309:
	.string	"__SIZEOF_PTRDIFF_T__ 8"
.LASF238:
	.string	"__FLT32X_MIN_EXP__ (-1021)"
.LASF34:
	.string	"__FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__"
.LASF124:
	.string	"__UINT_LEAST16_MAX__ 0xffff"
.LASF345:
	.string	"GNU C17 9.2.1 20190827 (Red Hat 9.2.1-1) -mtune=generic -march=x86-64 -g3 -gdwarf-4 -O1 -ffunction-sections -fdata-sections -fno-common"
.LASF305:
	.string	"__PRAGMA_REDEFINE_EXTNAME 1"
.LASF54:
	.string	"__INT_LEAST16_TYPE__ short int"
.LASF207:
	.string	"__FLT32_HAS_QUIET_NAN__ 1"
.LASF185:
	.string	"__DECIMAL_DIG__ 21"
.LASF72:
	.string	"__has_include_next(STR) __has_include_next__(STR)"
.LASF142:
	.string	"__INTPTR_MAX__ 0x7fffffffffffffffL"
.LASF330:
	.string	"__gnu_linux__ 1"
.LASF241:
	.string	"__FLT32X_MAX_10_EXP__ 308"
.LASF161:
	.string	"__FLT_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F"
.LASF193:
	.string	"__LDBL_HAS_QUIET_NAN__ 1"
.LASF25:
	.string	"__SIZEOF_DOUBLE__ 8"
.LASF106:
	.string	"__UINT8_MAX__ 0xff"
.LASF111:
	.string	"__INT8_C(c) c"
.LASF52:
	.string	"__UINT64_TYPE__ long unsigned int"
.LASF55:
	.string	"__INT_LEAST32_TYPE__ int"
.LASF198:
	.string	"__FLT32_MAX_EXP__ 128"
.LASF317:
	.string	"__ATOMIC_HLE_RELEASE 131072"
.LASF212:
	.string	"__FLT64_MAX_EXP__ 1024"
.LASF324:
	.string	"__SSE2__ 1"
.LASF16:
	.string	"__OPTIMIZE__ 1"
.LASF95:
	.string	"__INTMAX_C(c) c ## L"
.LASF295:
	.string	"__GCC_ATOMIC_CHAR32_T_LOCK_FREE 2"
.LASF306:
	.string	"__SIZEOF_INT128__ 16"
.LASF37:
	.string	"__PTRDIFF_TYPE__ long int"
.LASF254:
	.string	"__FLT64X_MAX_EXP__ 16384"
.LASF181:
	.string	"__LDBL_MIN_EXP__ (-16381)"
.LASF22:
	.string	"__SIZEOF_LONG_LONG__ 8"
.LASF152:
	.string	"__FLT_DIG__ 6"
.LASF175:
	.string	"__DBL_DENORM_MIN__ ((double)4.94065645841246544176568792868221372e-324L)"
.LASF120:
	.string	"__INT64_C(c) c ## L"
.LASF139:
	.string	"__UINT_FAST16_MAX__ 0xffffffffffffffffUL"
.LASF156:
	.string	"__FLT_MAX_10_EXP__ 38"
.LASF60:
	.string	"__UINT_LEAST64_TYPE__ long unsigned int"
.LASF26:
	.string	"__SIZEOF_LONG_DOUBLE__ 16"
.LASF342:
	.string	"__STDC_ISO_10646__ 201706L"
.LASF92:
	.string	"__PTRDIFF_WIDTH__ 64"
.LASF74:
	.string	"__SCHAR_MAX__ 0x7f"
.LASF202:
	.string	"__FLT32_MIN__ 1.17549435082228750796873653722224568e-38F32"
.LASF6:
	.string	"__GNUC_MINOR__ 2"
.LASF206:
	.string	"__FLT32_HAS_INFINITY__ 1"
.LASF258:
	.string	"__FLT64X_MIN__ 3.36210314311209350626267781732175260e-4932F64x"
.LASF149:
	.string	"__DEC_EVAL_METHOD__ 2"
.LASF268:
	.string	"__DEC32_MAX__ 9.999999E96DF"
.LASF78:
	.string	"__LONG_LONG_MAX__ 0x7fffffffffffffffLL"
.LASF292:
	.string	"__GCC_ATOMIC_BOOL_LOCK_FREE 2"
.LASF36:
	.string	"__SIZE_TYPE__ long unsigned int"
.LASF308:
	.string	"__SIZEOF_WINT_T__ 4"
.LASF194:
	.string	"__FLT32_MANT_DIG__ 24"
.LASF146:
	.string	"__GCC_IEC_559_COMPLEX 2"
.LASF164:
	.string	"__FLT_HAS_QUIET_NAN__ 1"
.LASF153:
	.string	"__FLT_MIN_EXP__ (-125)"
.LASF332:
	.string	"__linux__ 1"
.LASF244:
	.string	"__FLT32X_MIN__ 2.22507385850720138309023271733240406e-308F32x"
.LASF84:
	.string	"__SIZE_MAX__ 0xffffffffffffffffUL"
.LASF128:
	.string	"__UINT_LEAST64_MAX__ 0xffffffffffffffffUL"
.LASF136:
	.string	"__INT_FAST64_MAX__ 0x7fffffffffffffffL"
.LASF343:
	.string	"g_my_externd_global"
.LASF31:
	.string	"__ORDER_BIG_ENDIAN__ 4321"
.LASF69:
	.string	"__INTPTR_TYPE__ long int"
.LASF182:
	.string	"__LDBL_MIN_10_EXP__ (-4931)"
.LASF135:
	.string	"__INT_FAST32_WIDTH__ 64"
.LASF260:
	.string	"__FLT64X_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951F64x"
.LASF199:
	.string	"__FLT32_MAX_10_EXP__ 38"
.LASF187:
	.string	"__LDBL_MAX__ 1.18973149535723176502126385303097021e+4932L"
.LASF23:
	.string	"__SIZEOF_SHORT__ 2"
.LASF159:
	.string	"__FLT_MIN__ 1.17549435082228750796873653722224568e-38F"
.LASF298:
	.string	"__GCC_ATOMIC_INT_LOCK_FREE 2"
.LASF220:
	.string	"__FLT64_HAS_INFINITY__ 1"
.LASF114:
	.string	"__INT16_C(c) c"
.LASF262:
	.string	"__FLT64X_HAS_INFINITY__ 1"
.LASF336:
	.string	"unix 1"
.LASF155:
	.string	"__FLT_MAX_EXP__ 128"
.LASF275:
	.string	"__DEC64_MAX__ 9.999999999999999E384DD"
.LASF225:
	.string	"__FLT128_MIN_10_EXP__ (-4931)"
.LASF76:
	.string	"__INT_MAX__ 0x7fffffff"
.LASF233:
	.string	"__FLT128_HAS_DENORM__ 1"
.LASF300:
	.string	"__GCC_ATOMIC_LLONG_LOCK_FREE 2"
.LASF230:
	.string	"__FLT128_MIN__ 3.36210314311209350626267781732175260e-4932F128"
.LASF119:
	.string	"__INT_LEAST64_MAX__ 0x7fffffffffffffffL"
.LASF13:
	.string	"__ATOMIC_RELEASE 3"
.LASF197:
	.string	"__FLT32_MIN_10_EXP__ (-37)"
.LASF56:
	.string	"__INT_LEAST64_TYPE__ long int"
.LASF166:
	.string	"__DBL_DIG__ 15"
.LASF157:
	.string	"__FLT_DECIMAL_DIG__ 9"
.LASF11:
	.string	"__ATOMIC_SEQ_CST 5"
.LASF281:
	.string	"__DEC128_MIN__ 1E-6143DL"
.LASF29:
	.string	"__BIGGEST_ALIGNMENT__ 16"
.LASF20:
	.string	"__SIZEOF_INT__ 4"
.LASF63:
	.string	"__INT_FAST32_TYPE__ long int"
.LASF186:
	.string	"__LDBL_DECIMAL_DIG__ 21"
.LASF280:
	.string	"__DEC128_MAX_EXP__ 6145"
.LASF103:
	.string	"__INT16_MAX__ 0x7fff"
.LASF290:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1"
.LASF318:
	.string	"__GCC_ASM_FLAG_OUTPUTS__ 1"
.LASF226:
	.string	"__FLT128_MAX_EXP__ 16384"
.LASF42:
	.string	"__CHAR16_TYPE__ short unsigned int"
.LASF45:
	.string	"__INT8_TYPE__ signed char"
.LASF87:
	.string	"__INT_WIDTH__ 32"
.LASF49:
	.string	"__UINT8_TYPE__ unsigned char"
.LASF98:
	.string	"__INTMAX_WIDTH__ 64"
.LASF41:
	.string	"__UINTMAX_TYPE__ long unsigned int"
.LASF5:
	.string	"__GNUC__ 9"
.LASF341:
	.string	"__STDC_IEC_559_COMPLEX__ 1"
.LASF46:
	.string	"__INT16_TYPE__ short int"
.LASF297:
	.string	"__GCC_ATOMIC_SHORT_LOCK_FREE 2"
.LASF9:
	.string	"__GNUC_RH_RELEASE__ 1"
.LASF195:
	.string	"__FLT32_DIG__ 6"
.LASF282:
	.string	"__DEC128_MAX__ 9.999999999999999999999999999999999E6144DL"
.LASF347:
	.string	"/home/nickc/work/builds/binutils/current/x86_64-pc-linux-gnu/tests"
.LASF112:
	.string	"__INT_LEAST8_WIDTH__ 8"
.LASF58:
	.string	"__UINT_LEAST16_TYPE__ short unsigned int"
.LASF61:
	.string	"__INT_FAST8_TYPE__ signed char"
.LASF169:
	.string	"__DBL_MAX_EXP__ 1024"
.LASF236:
	.string	"__FLT32X_MANT_DIG__ 53"
.LASF147:
	.string	"__FLT_EVAL_METHOD__ 0"
.LASF94:
	.string	"__INTMAX_MAX__ 0x7fffffffffffffffL"
.LASF12:
	.string	"__ATOMIC_ACQUIRE 2"
.LASF67:
	.string	"__UINT_FAST32_TYPE__ long unsigned int"
.LASF200:
	.string	"__FLT32_DECIMAL_DIG__ 9"
.LASF116:
	.string	"__INT_LEAST32_MAX__ 0x7fffffff"
.LASF293:
	.string	"__GCC_ATOMIC_CHAR_LOCK_FREE 2"
.LASF283:
	.string	"__DEC128_EPSILON__ 1E-33DL"
.LASF158:
	.string	"__FLT_MAX__ 3.40282346638528859811704183484516925e+38F"
.LASF307:
	.string	"__SIZEOF_WCHAR_T__ 4"
.LASF242:
	.string	"__FLT32X_DECIMAL_DIG__ 17"
.LASF62:
	.string	"__INT_FAST16_TYPE__ long int"
.LASF65:
	.string	"__UINT_FAST8_TYPE__ unsigned char"
.LASF104:
	.string	"__INT32_MAX__ 0x7fffffff"
.LASF315:
	.string	"__SIZEOF_FLOAT128__ 16"
.LASF337:
	.string	"__ELF__ 1"
.LASF211:
	.string	"__FLT64_MIN_10_EXP__ (-307)"
.LASF180:
	.string	"__LDBL_DIG__ 18"
.LASF340:
	.string	"__STDC_IEC_559__ 1"
.LASF284:
	.string	"__DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL"
.LASF316:
	.string	"__ATOMIC_HLE_ACQUIRE 65536"
.LASF326:
	.string	"__SSE_MATH__ 1"
.LASF132:
	.string	"__INT_FAST16_MAX__ 0x7fffffffffffffffL"
.LASF113:
	.string	"__INT_LEAST16_MAX__ 0x7fff"
.LASF208:
	.string	"__FLT64_MANT_DIG__ 53"
.LASF278:
	.string	"__DEC128_MANT_DIG__ 34"
.LASF88:
	.string	"__LONG_WIDTH__ 64"
.LASF93:
	.string	"__SIZE_WIDTH__ 64"
.LASF91:
	.string	"__WINT_WIDTH__ 32"
.LASF228:
	.string	"__FLT128_DECIMAL_DIG__ 36"
.LASF80:
	.string	"__WCHAR_MIN__ (-__WCHAR_MAX__ - 1)"
.LASF289:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1"
.LASF183:
	.string	"__LDBL_MAX_EXP__ 16384"
.LASF251:
	.string	"__FLT64X_DIG__ 18"
.LASF237:
	.string	"__FLT32X_DIG__ 15"
.LASF320:
	.string	"__k8__ 1"
.LASF43:
	.string	"__CHAR32_TYPE__ unsigned int"
.LASF96:
	.string	"__UINTMAX_MAX__ 0xffffffffffffffffUL"
.LASF201:
	.string	"__FLT32_MAX__ 3.40282346638528859811704183484516925e+38F32"
.LASF77:
	.string	"__LONG_MAX__ 0x7fffffffffffffffL"
.LASF339:
	.string	"_STDC_PREDEF_H 1"
.LASF53:
	.string	"__INT_LEAST8_TYPE__ signed char"
.LASF125:
	.string	"__UINT16_C(c) c"
.LASF75:
	.string	"__SHRT_MAX__ 0x7fff"
.LASF311:
	.string	"__amd64__ 1"
.LASF24:
	.string	"__SIZEOF_FLOAT__ 4"
.LASF291:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1"
.LASF79:
	.string	"__WCHAR_MAX__ 0x7fffffff"
.LASF232:
	.string	"__FLT128_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966F128"
.LASF0:
	.string	"__STDC__ 1"
.LASF32:
	.string	"__ORDER_PDP_ENDIAN__ 3412"
.LASF302:
	.string	"__GCC_ATOMIC_POINTER_LOCK_FREE 2"
.LASF30:
	.string	"__ORDER_LITTLE_ENDIAN__ 1234"
.LASF39:
	.string	"__WINT_TYPE__ unsigned int"
.LASF333:
	.string	"linux 1"
.LASF10:
	.string	"__ATOMIC_RELAXED 0"
.LASF17:
	.string	"__FINITE_MATH_ONLY__ 0"
.LASF81:
	.string	"__WINT_MAX__ 0xffffffffU"
.LASF346:
	.string	"main.c"
.LASF264:
	.string	"__DEC32_MANT_DIG__ 7"
.LASF276:
	.string	"__DEC64_EPSILON__ 1E-15DD"
.LASF327:
	.string	"__SSE2_MATH__ 1"
.LASF145:
	.string	"__GCC_IEC_559 2"
.LASF27:
	.string	"__SIZEOF_SIZE_T__ 8"
.LASF246:
	.string	"__FLT32X_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F32x"
.LASF322:
	.string	"__MMX__ 1"
.LASF105:
	.string	"__INT64_MAX__ 0x7fffffffffffffffL"
.LASF83:
	.string	"__PTRDIFF_MAX__ 0x7fffffffffffffffL"
.LASF329:
	.string	"__SEG_GS 1"
.LASF99:
	.string	"__SIG_ATOMIC_MAX__ 0x7fffffff"
.LASF312:
	.string	"__x86_64 1"
.LASF323:
	.string	"__SSE__ 1"
.LASF66:
	.string	"__UINT_FAST16_TYPE__ long unsigned int"
.LASF248:
	.string	"__FLT32X_HAS_INFINITY__ 1"
.LASF133:
	.string	"__INT_FAST16_WIDTH__ 64"
.LASF205:
	.string	"__FLT32_HAS_DENORM__ 1"
.LASF150:
	.string	"__FLT_RADIX__ 2"
.LASF140:
	.string	"__UINT_FAST32_MAX__ 0xffffffffffffffffUL"
.LASF163:
	.string	"__FLT_HAS_INFINITY__ 1"
.LASF101:
	.string	"__SIG_ATOMIC_WIDTH__ 32"
.LASF134:
	.string	"__INT_FAST32_MAX__ 0x7fffffffffffffffL"
.LASF214:
	.string	"__FLT64_DECIMAL_DIG__ 17"
.LASF89:
	.string	"__LONG_LONG_WIDTH__ 64"
.LASF218:
	.string	"__FLT64_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F64"
.LASF243:
	.string	"__FLT32X_MAX__ 1.79769313486231570814527423731704357e+308F32x"
.LASF97:
	.string	"__UINTMAX_C(c) c ## UL"
.LASF171:
	.string	"__DBL_DECIMAL_DIG__ 17"
.LASF331:
	.string	"__linux 1"
.LASF257:
	.string	"__FLT64X_MAX__ 1.18973149535723176502126385303097021e+4932F64x"
.LASF21:
	.string	"__SIZEOF_LONG__ 8"
.LASF40:
	.string	"__INTMAX_TYPE__ long int"
.LASF191:
	.string	"__LDBL_HAS_DENORM__ 1"
.LASF210:
	.string	"__FLT64_MIN_EXP__ (-1021)"
.LASF71:
	.string	"__has_include(STR) __has_include__(STR)"
.LASF294:
	.string	"__GCC_ATOMIC_CHAR16_T_LOCK_FREE 2"
.LASF314:
	.string	"__SIZEOF_FLOAT80__ 16"
.LASF285:
	.string	"__REGISTER_PREFIX__ "
.LASF15:
	.string	"__ATOMIC_CONSUME 1"
.LASF109:
	.string	"__UINT64_MAX__ 0xffffffffffffffffUL"
.LASF3:
	.string	"__STDC_UTF_32__ 1"
.LASF50:
	.string	"__UINT16_TYPE__ short unsigned int"
.LASF255:
	.string	"__FLT64X_MAX_10_EXP__ 4932"
.LASF127:
	.string	"__UINT32_C(c) c ## U"
.LASF319:
	.string	"__k8 1"
.LASF303:
	.string	"__HAVE_SPECULATION_SAFE_VALUE 1"
.LASF344:
	.string	"g_my_private_global"
.LASF270:
	.string	"__DEC32_SUBNORMAL_MIN__ 0.000001E-95DF"
.LASF229:
	.string	"__FLT128_MAX__ 1.18973149535723176508575932662800702e+4932F128"
.LASF73:
	.string	"__GXX_ABI_VERSION 1013"
.LASF28:
	.string	"__CHAR_BIT__ 8"
.LASF100:
	.string	"__SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)"
.LASF137:
	.string	"__INT_FAST64_WIDTH__ 64"
.LASF269:
	.string	"__DEC32_EPSILON__ 1E-6DF"
.LASF176:
	.string	"__DBL_HAS_DENORM__ 1"
.LASF235:
	.string	"__FLT128_HAS_QUIET_NAN__ 1"
.LASF222:
	.string	"__FLT128_MANT_DIG__ 113"
.LASF209:
	.string	"__FLT64_DIG__ 15"
.LASF179:
	.string	"__LDBL_MANT_DIG__ 64"
.LASF299:
	.string	"__GCC_ATOMIC_LONG_LOCK_FREE 2"
.LASF172:
	.string	"__DBL_MAX__ ((double)1.79769313486231570814527423731704357e+308L)"
.LASF102:
	.string	"__INT8_MAX__ 0x7f"
.LASF138:
	.string	"__UINT_FAST8_MAX__ 0xff"
.LASF259:
	.string	"__FLT64X_EPSILON__ 1.08420217248550443400745280086994171e-19F64x"
.LASF154:
	.string	"__FLT_MIN_10_EXP__ (-37)"
.LASF301:
	.string	"__GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1"
.LASF256:
	.string	"__FLT64X_DECIMAL_DIG__ 21"
.LASF14:
	.string	"__ATOMIC_ACQ_REL 4"
.LASF38:
	.string	"__WCHAR_TYPE__ int"
.LASF131:
	.string	"__INT_FAST8_WIDTH__ 8"
.LASF227:
	.string	"__FLT128_MAX_10_EXP__ 4932"
.LASF286:
	.string	"__USER_LABEL_PREFIX__ "
.LASF196:
	.string	"__FLT32_MIN_EXP__ (-125)"
.LASF59:
	.string	"__UINT_LEAST32_TYPE__ unsigned int"
.LASF123:
	.string	"__UINT8_C(c) c"
.LASF223:
	.string	"__FLT128_DIG__ 33"
.LASF57:
	.string	"__UINT_LEAST8_TYPE__ unsigned char"
.LASF48:
	.string	"__INT64_TYPE__ long int"
.LASF121:
	.string	"__INT_LEAST64_WIDTH__ 64"
.LASF328:
	.string	"__SEG_FS 1"
.LASF288:
	.string	"__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1"
.LASF348:
	.string	"main"
.LASF108:
	.string	"__UINT32_MAX__ 0xffffffffU"
.LASF304:
	.string	"__GCC_HAVE_DWARF2_CFI_ASM 1"
.LASF313:
	.string	"__x86_64__ 1"
.LASF110:
	.string	"__INT_LEAST8_MAX__ 0x7f"
.LASF274:
	.string	"__DEC64_MIN__ 1E-383DD"
.LASF168:
	.string	"__DBL_MIN_10_EXP__ (-307)"
.LASF338:
	.string	"__DECIMAL_BID_FORMAT__ 1"
.LASF287:
	.string	"__GNUC_STDC_INLINE__ 1"
.LASF213:
	.string	"__FLT64_MAX_10_EXP__ 308"
.LASF151:
	.string	"__FLT_MANT_DIG__ 24"


