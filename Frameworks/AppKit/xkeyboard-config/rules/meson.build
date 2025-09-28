# And some files that are a straight install
install_data('README', 'xkb.dtd', 'xfree98',
             install_dir: dir_xkb_rules)

# the actual rules files are generated from a list of parts in a very
# specific order
parts = [
    '0000-hdr.part',
    '0001-lists.part',
    '0002-@0@.lists.part',
    '0004-@0@.m_k.part',
    '0005-l1_k.part',
    '0006-l_k.part',
    '0007-o_k.part',
    '0008-ml_g.part',
    '0009-m_g.part',
    '0011-mlv_s.part',
    '0013-ml_s.part',
    '0015-ml1_s.part',
    '0018-ml2_s.part',
    '0020-ml3_s.part',
    '0022-ml4_s.part',
    '0026-@0@.m_s.part',
    '0027-@0@.ml_s1.part',
    '0033-ml_c.part',
    '0034-ml1_c.part',
    '0035-m_t.part',
    '0036-lo_s.part',
    '0037-l1o_s.part',
    '0038-l2o_s.part',
    '0039-l3o_s.part',
    '0040-l4o_s.part',
    '0042-o_s.part',
    '0043-o_c.part',
    '0044-o_t.part',
]

# generated compat parts
rules_compat_generated = []

if get_option('compat-rules')
  # non-generated compat parts
  parts += [
      'compat/0028-lv_c.part',
      'compat/0029-l1v1_c.part',
      'compat/0030-l2v2_c.part',
      'compat/0031-l3v3_c.part',
      'compat/0032-l4v4_c.part',
      'compat/0041-o_s.part',
  ]

  layout_mappings = files('compat/layoutsMapping.lst')
  variant_mappings = files('compat/variantsMapping.lst')
  map_variants_py = find_program('compat/map-variants.py')

  # two sets of files are generated: ml_s.part and mlv_s.part
  # each with the level name in the filename
  lvl_ml_s = {
    '0': '0012-ml_s.part',
    '1': '0014-ml1_s.part',
    '2': '0017-ml2_s.part',
    '3': '0019-ml3_s.part',
    '4': '0021-ml4_s.part',
  }

  lvl_mlv_s = {
    '0': '0010-mlv_s.part',
    '1': '0016-ml1v1_s.part',
    '2': '0023-ml2v2_s.part',
    '3': '0024-ml3v3_s.part',
    '4': '0025-ml4v4_s.part',
  }

  foreach lvl: [0, 1, 2, 3, 4]
      ml_s_file = lvl_ml_s['@0@'.format(lvl)]
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

      mlv_s_file = lvl_mlv_s['@0@'.format(lvl)]
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
endif # compat-rules

merge_py = find_program('merge.py')
xml2lst = find_program('xml2lst.pl')
foreach ruleset: ['base', 'evdev']
    # generate the "evdev" and "base" rules files
    #
    # First: copy all the parts over to the build directory, replacing
    # RULESET with the rulename (evdev or base) and prefix it with the
    # ruleset name. So 0000-hdr.part becomes 0000-hdr.part.evdev and
    # 0000-hdr.part.base
    rules_parts = []
    foreach part: parts
      infile = part.format(ruleset)
      p = configure_file(output: '@PLAINNAME@.@0@'.format(ruleset),
                         input: infile,
                         copy: true,
                         install: false)
      rules_parts += p
    endforeach

    # Second: merge those parts together into the actual rules file
    custom_target('rules-@0@'.format(ruleset),
                  build_by_default: true,
                  command: [
                      merge_py,
                      rules_parts + rules_compat_generated,
                  ],
                  depends: rules_compat_generated,
                  output: ruleset,
                  capture: true,
                  install: true,
                  install_dir: dir_xkb_rules)

    # Third: the xml files, simply copied from the base*.xml files
    ruleset_xml = configure_file(output: '@0@.xml'.format(ruleset),
                       input: 'base.xml',
                       copy: true,
                       install: true,
                       install_dir: dir_xkb_rules)

    # This is used by the man page's meson.build
    if ruleset == 'evdev'
      evdev_ruleset = ruleset_xml
    endif

    configure_file(output: '@0@.extras.xml'.format(ruleset),
                   input: 'base.extras.xml',
                   copy: true,
                   install: true,
                   install_dir: dir_xkb_rules)

    # Fourth: generate the evdev.lst and base.lst files
    lst_file = '@0@.lst'.format(ruleset)
    custom_target(lst_file,
                  build_by_default: true,
                  command: [xml2lst, ruleset_xml],
                  capture: true,
                  output: lst_file,
                  install: true,
                  install_dir: dir_xkb_rules)

endforeach

if get_option('xorg-rules-symlinks')
	foreach suffix: ['', '.lst', '.xml']
		meson.add_install_script('sh', '-c',
			'ln -s base@0@ $DESTDIR@1@/xorg@0@'.format(suffix, dir_xkb_rules))
	endforeach
endif
# Copy the DTD to the build directory, the man page generation expects it in
# the same directory as the input XML file.
configure_file(output: 'xkb.dtd',
               input: 'xkb.dtd',
               copy: true,
               install: false)
