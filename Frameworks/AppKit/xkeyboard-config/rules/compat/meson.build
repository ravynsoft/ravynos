# generated files
rules_compat_generated = []

layout_mappings = files('layoutsMapping.lst')
variant_mappings = files('variantsMapping.lst')
map_variants_py = find_program('map-variants.py')

ml_s_file = 'base.ml_s.part'
ml_s = custom_target(ml_s_file,
                     build_by_default: true,
                     command: [
                         map_variants_py,
                         '--want=mls',
                         '@OUTPUT@',
                         layout_mappings,
                         variant_mappings,
                     ],
                     output: ml_s_file,
                     install: false)
rules_compat_generated += [ml_s]

# base.mlv_s.part and friends
mlv_s_file = 'base.mlv_s.part'
mlv_s = custom_target(mlv_s_file,
                     build_by_default: true,
                     command: [
                         map_variants_py,
                         '--want=mlvs',
                         '@OUTPUT@',
                         variant_mappings,
                     ],
                     output: mlv_s_file,
                     install: false)
rules_compat_generated += [mlv_s]

foreach lvl: [1, 2, 3, 4]
    # base.ml1_s.part and friends
    ml_s_file = 'base.ml@0@_s.part'.format(lvl)
    ml_s = custom_target(ml_s_file,
                         build_by_default: true,
                         command: [
                             map_variants_py,
                             '--want=mls',
                             '--number=@0@'.format(lvl),
                             '@OUTPUT@',
                             layout_mappings,
                             variant_mappings,
                         ],
                         output: ml_s_file,
                         install: false)
    rules_compat_generated += [ml_s]

    # base.ml1v1_s.part and friends
    mlv_s_file = 'base.ml@0@v@0@_s.part'.format(lvl)
    mlv_s = custom_target(mlv_s_file,
                         build_by_default: true,
                         command: [
                             map_variants_py,
                             '--want=mlvs',
                             '--number=@0@'.format(lvl),
                             '@OUTPUT@',
                             variant_mappings,
                         ],
                         output: mlv_s_file,
                         install: false)
    rules_compat_generated += [mlv_s]
endforeach

# Non-generated files
others = [
    'base.lists.part',
    'base.lv_c.part',
    'base.l1v1_c.part',
    'base.l2v2_c.part',
    'base.l3v3_c.part',
    'base.l4v4_c.part',
    'base.o_s.part',
]
foreach o: others
    configure_file(output: o,
                   input: o,
                   copy: true,
                   install: false)
endforeach
