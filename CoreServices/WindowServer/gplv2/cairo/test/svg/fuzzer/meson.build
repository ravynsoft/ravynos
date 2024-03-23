fuzz_targets = [
  'svg-render-fuzzer'
]

fuzz_args = ['-fsanitize=fuzzer,address']

foreach target_name : fuzz_targets
  exe = executable(target_name, [target_name + '.c'],
  include_directories: [incbase, incsrc],
  c_args: [fuzz_args],
  link_with: [libcairo],
  link_args: [fuzz_args, extra_link_args],
  dependencies: [deps, test_deps])
endforeach
