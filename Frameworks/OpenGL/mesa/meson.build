# Copyright © 2017-2020 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(
  'mesa',
  ['c', 'cpp'],
  version : files('VERSION'),
  license : 'MIT',
  meson_version : '>= 0.60',
  default_options : [
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
    'c_std=c11',
    'cpp_std=c++17',
    'rust_std=2021',
    'build.rust_std=2021',
  ],
)

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

null_dep = dependency('', required : false)

if get_option('layout') != 'mirror'
  error('`mirror` is the only build directory layout supported')
endif

# Arguments for the preprocessor, put these in a separate array from the C and
# C++ (cpp in meson terminology) arguments since they need to be added to the
# default arguments for both C and C++.
pre_args = [
  '-D__STDC_CONSTANT_MACROS',
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_LIMIT_MACROS',
  '-DPACKAGE_VERSION="@0@"'.format(meson.project_version()),
  '-DPACKAGE_BUGREPORT="https://gitlab.freedesktop.org/mesa/mesa/-/issues"',
]
# Arguments for c or cpp compiler, can be compiler options
c_cpp_args = []

c_args = []
cpp_args = []

with_moltenvk_dir = get_option('moltenvk-dir')
with_vulkan_icd_dir = get_option('vulkan-icd-dir')
with_tests = get_option('build-tests')
with_glcpp_tests = get_option('enable-glcpp-tests')
with_aco_tests = get_option('build-aco-tests')
with_glx_read_only_text = get_option('glx-read-only-text')
with_glx_direct = get_option('glx-direct')
with_osmesa = get_option('osmesa')
with_vulkan_overlay_layer = get_option('vulkan-layers').contains('overlay')
with_vulkan_device_select_layer = get_option('vulkan-layers').contains('device-select')
with_tools = get_option('tools')
if with_tools.contains('all')
  with_tools = [
    'drm-shim',
    'dlclose-skip',
    'etnaviv',
    'freedreno',
    'glsl',
    'intel',
    'intel-ui',
    'lima',
    'nir',
    'nouveau',
    'asahi',
    'imagination',
  ]
endif

with_any_vulkan_layers = get_option('vulkan-layers').length() != 0
with_intel_tools = with_tools.contains('intel') or with_tools.contains('intel-ui')
with_imgui = with_intel_tools or with_vulkan_overlay_layer

dri_drivers_path = get_option('dri-drivers-path')
if dri_drivers_path == ''
  dri_drivers_path = join_paths(get_option('prefix'), get_option('libdir'), 'dri')
endif
dri_search_path = get_option('dri-search-path')
if dri_search_path == ''
  dri_search_path = dri_drivers_path
endif

gbm_backends_path = get_option('gbm-backends-path')
if gbm_backends_path == ''
  gbm_backends_path = join_paths(get_option('prefix'), get_option('libdir'), 'gbm')
endif

# Default shared glapi disabled for windows, enabled elsewhere.
with_shared_glapi = get_option('shared-glapi') \
  .disable_auto_if(host_machine.system() == 'windows') \
  .allowed()

with_opengl = get_option('opengl')

with_gles1 = get_option('gles1') \
  .require(with_shared_glapi, error_message : 'OpengGL ES 1.x requires shared-glapi') \
  .allowed()

with_gles2 = get_option('gles2') \
  .require(with_shared_glapi, error_message : 'OpengGL ES 2.x requires shared-glapi') \
  .allowed()

pre_args += '-DHAVE_OPENGL=@0@'.format(with_opengl ? '1' : '0')
pre_args += '-DHAVE_OPENGL_ES_1=@0@'.format(with_gles1 ? '1' : '0')
pre_args += '-DHAVE_OPENGL_ES_2=@0@'.format(with_gles2 ? '1' : '0')

with_any_opengl = with_opengl or with_gles1 or with_gles2
# Only build shared_glapi if at least one OpenGL API is enabled
with_shared_glapi = with_shared_glapi and with_any_opengl

system_has_kms_drm = ['openbsd', 'netbsd', 'freebsd', 'gnu/kfreebsd', 'dragonfly', 'linux', 'sunos', 'android', 'managarm'].contains(host_machine.system())

gallium_drivers = get_option('gallium-drivers')
if gallium_drivers.contains('auto')
  if system_has_kms_drm
    # TODO: PPC, Sparc
    if ['x86', 'x86_64'].contains(host_machine.cpu_family())
      gallium_drivers = [
        'r300', 'r600', 'radeonsi', 'nouveau', 'virgl', 'svga', 'swrast',
        'iris', 'crocus', 'i915'
      ]
    elif ['arm', 'aarch64'].contains(host_machine.cpu_family())
      gallium_drivers = [
        'vc4', 'etnaviv', 'nouveau', 'svga',
        'tegra', 'virgl', 'lima', 'panfrost', 'swrast', 'iris'
      ]
    elif ['mips', 'mips64', 'riscv32', 'riscv64'].contains(host_machine.cpu_family())
      gallium_drivers = [
        'r300', 'r600', 'radeonsi', 'nouveau', 'virgl', 'swrast'
      ]
    elif ['loongarch64'].contains(host_machine.cpu_family())
      gallium_drivers = [
        'r300', 'r600', 'radeonsi', 'nouveau', 'virgl', 'etnaviv', 'swrast'
      ]
    else
      error('Unknown architecture @0@. Please pass -Dgallium-drivers to set driver options. Patches gladly accepted to fix this.'.format(
            host_machine.cpu_family()))
    endif
  elif ['darwin', 'windows', 'cygwin', 'haiku'].contains(host_machine.system())
    gallium_drivers = ['swrast']
  else
    error('Unknown OS @0@. Please pass -Dgallium-drivers to set driver options. Patches gladly accepted to fix this.'.format(
          host_machine.system()))
  endif
elif gallium_drivers.contains('all')
   # Build-test everything except for i915, which depends on libdrm-intel which
   # is not available on non-Intel distros.
   gallium_drivers = [
     'r300', 'r600', 'radeonsi', 'crocus', 'v3d', 'vc4', 'freedreno', 'etnaviv',
     'nouveau', 'svga', 'tegra', 'virgl', 'lima', 'panfrost', 'swrast', 'iris',
     'zink', 'd3d12', 'asahi'
   ]
endif

with_gallium_radeonsi = gallium_drivers.contains('radeonsi')
with_gallium_r300 = gallium_drivers.contains('r300')
with_gallium_r600 = gallium_drivers.contains('r600')
with_gallium_nouveau = gallium_drivers.contains('nouveau')
with_gallium_freedreno = gallium_drivers.contains('freedreno')
with_gallium_softpipe = gallium_drivers.contains('swrast')
with_gallium_vc4 = gallium_drivers.contains('vc4')
with_gallium_v3d = gallium_drivers.contains('v3d')
with_gallium_panfrost = gallium_drivers.contains('panfrost')
with_gallium_etnaviv = gallium_drivers.contains('etnaviv')
with_gallium_tegra = gallium_drivers.contains('tegra')
with_gallium_crocus = gallium_drivers.contains('crocus')
with_gallium_iris = gallium_drivers.contains('iris')
with_gallium_i915 = gallium_drivers.contains('i915')
with_gallium_svga = gallium_drivers.contains('svga')
with_gallium_virgl = gallium_drivers.contains('virgl')
with_gallium_lima = gallium_drivers.contains('lima')
with_gallium_zink = gallium_drivers.contains('zink')
with_gallium_d3d12 = gallium_drivers.contains('d3d12')
with_gallium_asahi = gallium_drivers.contains('asahi')
foreach gallium_driver : gallium_drivers
  pre_args += '-DHAVE_@0@'.format(gallium_driver.to_upper())
endforeach

with_gallium = gallium_drivers.length() != 0
with_gallium_kmsro = system_has_kms_drm and [
  with_gallium_asahi,
  with_gallium_etnaviv,
  with_gallium_freedreno,
  with_gallium_lima,
  with_gallium_panfrost,
  with_gallium_v3d,
  with_gallium_vc4,
].contains(true)

_vulkan_drivers = get_option('vulkan-drivers')
if _vulkan_drivers.contains('auto')
  if system_has_kms_drm
    if host_machine.cpu_family().startswith('x86')
      _vulkan_drivers = ['amd', 'intel', 'intel_hasvk', 'swrast']
    elif ['arm', 'aarch64'].contains(host_machine.cpu_family())
      _vulkan_drivers = ['swrast', 'intel']
    elif ['mips', 'mips64', 'riscv32', 'riscv64'].contains(host_machine.cpu_family())
      _vulkan_drivers = ['amd', 'swrast']
    elif ['loongarch64'].contains(host_machine.cpu_family())
      _vulkan_drivers = ['amd', 'swrast']
    else
      error('Unknown architecture @0@. Please pass -Dvulkan-drivers to set driver options. Patches gladly accepted to fix this.'.format(
            host_machine.cpu_family()))
    endif
  elif ['darwin', 'windows', 'cygwin', 'haiku'].contains(host_machine.system())
    # No vulkan driver supports windows or macOS currently
    _vulkan_drivers = []
  else
    error('Unknown OS @0@. Please pass -Dvulkan-drivers to set driver options. Patches gladly accepted to fix this.'.format(
          host_machine.system()))
  endif
elif _vulkan_drivers.contains('all')
   # Build every vulkan driver regardless of architecture.
   _vulkan_drivers = ['amd', 'intel', 'intel_hasvk', 'swrast',
                      'freedreno', 'panfrost', 'virtio', 'broadcom',
                      'imagination-experimental', 'microsoft-experimental',
                      'nouveau-experimental']
endif

with_intel_vk = _vulkan_drivers.contains('intel')
with_intel_hasvk = _vulkan_drivers.contains('intel_hasvk')
with_amd_vk = _vulkan_drivers.contains('amd')
with_freedreno_vk = _vulkan_drivers.contains('freedreno')
with_panfrost_vk = _vulkan_drivers.contains('panfrost')
with_swrast_vk = _vulkan_drivers.contains('swrast')
with_virtio_vk = _vulkan_drivers.contains('virtio')
with_broadcom_vk = _vulkan_drivers.contains('broadcom')
with_imagination_vk = _vulkan_drivers.contains('imagination-experimental')
with_imagination_srv = get_option('imagination-srv')
with_microsoft_vk = _vulkan_drivers.contains('microsoft-experimental')
with_nouveau_vk = _vulkan_drivers.contains('nouveau-experimental')
with_any_vk = _vulkan_drivers.length() != 0

with_vk_compiler = [
  with_intel_vk,
  with_intel_hasvk,
  with_amd_vk,
  with_freedreno_vk,
  with_panfrost_vk,
  with_swrast_vk,
  with_broadcom_vk,
  with_imagination_vk,
  with_microsoft_vk,
  with_nouveau_vk,
].contains(true)
pre_args += '-DUSE_VK_COMPILER=@0@'.format(with_vk_compiler ? '1' : '0')

freedreno_kmds = get_option('freedreno-kmds')
if freedreno_kmds.length() != 0 and freedreno_kmds != [ 'msm' ] and with_freedreno_vk
  if freedreno_kmds.contains('msm')
      warning('Turnip with the DRM KMD will require libdrm to always be present at runtime which may not always be the case on platforms such as Android.')
  elif with_gallium_kmsro
      warning('As a side-effect, Turnip is forced to link with libdrm when built alongside Gallium DRM drivers which platforms such as Android may not have available at runtime.')
  elif _vulkan_drivers != [ 'freedreno' ]
      warning('Turnip is forced to link with libdrm when built alongside other Vulkan drivers which platforms such as Android may not have available at runtime.')
  else
    # If DRM support isn't needed, we can get rid of it since linking
    # to libdrm can be a potential compatibility hazard.
    system_has_kms_drm = false
  endif
endif

with_dri = false
if with_gallium and system_has_kms_drm
  _glx = get_option('glx')
  _egl = get_option('egl')
  if _glx == 'dri' or _egl.enabled() or (_glx == 'disabled' and _egl.allowed())
    with_dri = true
  endif
endif

with_any_broadcom = [
  with_gallium_vc4,
  with_gallium_v3d,
  with_broadcom_vk,
].contains(true)

if ['x86_64'].contains(host_machine.cpu_family())
  with_intel_clc = get_option('intel-clc') == 'enabled'
  with_intel_vk_rt = with_intel_vk and get_option('intel-clc') != 'disabled'
else
  with_intel_clc = false
  with_intel_vk_rt = false
endif

with_any_intel = [
  with_gallium_crocus,
  with_gallium_i915,
  with_gallium_iris,
  with_intel_clc,
  with_intel_hasvk,
  with_intel_tools,
  with_intel_vk,
].contains(true)
with_any_nouveau = with_gallium_nouveau or with_nouveau_vk

if with_swrast_vk and not with_gallium_softpipe
  error('swrast vulkan requires gallium swrast')
endif
if with_gallium_tegra and not with_gallium_nouveau
  error('tegra driver requires nouveau driver')
endif
if with_aco_tests and not with_amd_vk
  error('ACO tests require Radv')
endif

with_microsoft_clc = get_option('microsoft-clc').enabled()
with_clc = with_microsoft_clc or with_intel_clc or with_gallium_asahi
with_spirv_to_dxil = get_option('spirv-to-dxil')

if host_machine.system() == 'darwin'
  with_dri_platform = 'apple'
  pre_args += '-DBUILDING_MESA'
elif ['windows', 'cygwin'].contains(host_machine.system())
  with_dri_platform = 'windows'
elif system_has_kms_drm
  with_dri_platform = 'drm'
else
  # FIXME: haiku doesn't use dri, and xlib doesn't use dri, probably should
  # assert here that one of those cases has been met.
  # FIXME: illumos ends up here as well
  with_dri_platform = 'none'
endif

with_vulkan_beta = get_option('vulkan-beta')
if host_machine.system() == 'darwin'
  #macOS seems to need beta extensions to build for now:
  with_vulkan_beta = true
endif
if with_vulkan_beta
  pre_args += '-DVK_ENABLE_BETA_EXTENSIONS'
endif

_codecs = get_option('video-codecs')
patent_codecs = ['vc1dec', 'h264dec', 'h264enc', 'h265dec', 'h265enc']
free_codecs = ['av1dec', 'av1enc', 'vp9dec']
all_codecs = patent_codecs + free_codecs

if _codecs.contains('all')
  _codecs = all_codecs
elif _codecs.contains('all_free')
  selected_codecs = _codecs
  _codecs = free_codecs
  foreach c : patent_codecs
    if selected_codecs.contains(c)
      _codecs += c
    endif
  endforeach
endif
foreach c : all_codecs
   pre_args += '-DVIDEO_CODEC_@0@=@1@'.format(c.to_upper(), _codecs.contains(c).to_int())
endforeach

_platforms = get_option('platforms')
if _platforms.contains('auto')
  if system_has_kms_drm
    _platforms = ['x11', 'wayland']
  elif ['darwin', 'cygwin'].contains(host_machine.system())
    _platforms = ['x11']
  elif ['haiku'].contains(host_machine.system())
    _platforms = ['haiku']
  elif host_machine.system() == 'windows'
    _platforms = ['windows']
  else
    error('Unknown OS @0@. Please pass -Dplatforms to set platforms. Patches gladly accepted to fix this.'.format(
          host_machine.system()))
  endif
endif

with_platform_android = _platforms.contains('android')
with_platform_x11 = _platforms.contains('x11')
with_platform_wayland = _platforms.contains('wayland')
with_platform_haiku = _platforms.contains('haiku')
with_platform_windows = _platforms.contains('windows')

with_glx = get_option('glx')
if with_glx == 'auto'
  if not with_opengl
    with_glx = 'disabled'
  elif with_platform_android
    with_glx = 'disabled'
  elif with_dri
    with_glx = 'dri'
  elif with_platform_haiku
    with_glx = 'disabled'
  elif host_machine.system() == 'windows'
    with_glx = 'disabled'
  elif with_gallium
    # Even when building just gallium drivers the user probably wants dri
    with_glx = 'dri'
  elif with_platform_x11 and with_any_opengl and not with_any_vk
    # The automatic behavior should not be to turn on xlib based glx when
    # building only vulkan drivers
    with_glx = 'xlib'
  else
    with_glx = 'disabled'
  endif
endif
if with_glx == 'dri'
   if with_gallium
      with_dri = true
   endif
endif

if not with_opengl and with_glx != 'disabled'
  error('Building GLX without OpenGL is not supported.') \
endif

if not (with_dri or with_gallium or with_glx != 'disabled')
  with_gles1 = false
  with_gles2 = false
  with_opengl = false
  with_any_opengl = false
  with_shared_glapi = false
endif

with_gbm = get_option('gbm') \
  .require(system_has_kms_drm, error_message : 'GBM only supports DRM/KMS platforms') \
  .disable_auto_if(not with_dri) \
  .allowed()

with_xlib_lease = get_option('xlib-lease') \
  .require(with_platform_x11 and system_has_kms_drm, error_message : 'xlib-lease requires X11 and KMS/DRM support') \
  .allowed()

with_egl = get_option('egl') \
  .require(host_machine.system() != 'darwin', error_message : 'EGL not supported on MacOS') \
  .require(with_platform_windows or with_platform_haiku or with_dri or with_platform_android, error_message : 'EGL requires DRI, Haiku, Windows or Android') \
  .require(with_shared_glapi, error_message : 'EGL requires shared-glapi') \
  .require(with_glx != 'xlib', error_message :'EGL requires DRI, but GLX is being built with xlib support') \
  .disable_auto_if(with_platform_haiku) \
  .allowed()

if with_egl
  _platforms += 'surfaceless'
  if with_gbm and not with_platform_android
    _platforms += 'drm'
  endif

  egl_native_platform = get_option('egl-native-platform')
  if egl_native_platform.contains('auto')
    egl_native_platform = _platforms[0]
  endif
endif

if with_egl and not _platforms.contains(egl_native_platform)
  error('-Degl-native-platform does not specify an enabled platform')
endif

if 'x11' in _platforms
  _platforms += 'xcb'
endif

foreach platform : _platforms
  pre_args += '-DHAVE_@0@_PLATFORM'.format(platform.to_upper())
endforeach

if with_platform_android and get_option('platform-sdk-version') >= 29
  # By default the NDK compiler, at least, emits emutls references instead of
  # ELF TLS, even when building targeting newer API levels.  Make it actually do
  # ELF TLS instead.
  c_cpp_args += '-fno-emulated-tls'
endif

# -mtls-dialect=gnu2 speeds up non-initial-exec TLS significantly but requires
# full toolchain (including libc) support.
have_mtls_dialect = false
foreach c_arg : get_option('c_args')
  if c_arg.startswith('-mtls-dialect=')
    have_mtls_dialect = true
    break
  endif
endforeach
if not have_mtls_dialect
  # need .run to check libc support. meson aborts when calling .run when
  # cross-compiling, but because this is just an optimization we can skip it
  if meson.is_cross_build() and not meson.can_run_host_binaries()
    warning('cannot auto-detect -mtls-dialect when cross-compiling, using compiler default')
  else
    # -fpic to force dynamic tls, otherwise TLS relaxation defeats check
    gnu2_test = cc.run('int __thread x; int main() { return x; }',
                       args: ['-mtls-dialect=gnu2', '-fpic'],
                       name: '-mtls-dialect=gnu2')
    if gnu2_test.returncode() == 0 and (
          # check for lld 13 bug: https://gitlab.freedesktop.org/mesa/mesa/-/issues/5665
          host_machine.cpu_family() != 'x86_64' or
          # get_linker_id misses LDFLAGS=-fuse-ld=lld: https://github.com/mesonbuild/meson/issues/6377
          #cc.get_linker_id() != 'ld.lld' or
          cc.links('''int __thread x; int y; int main() { __asm__(
                "leaq x@TLSDESC(%rip), %rax\n"
                "movq y@GOTPCREL(%rip), %rdx\n"
                "call *x@TLSCALL(%rax)\n"); }''', name: 'split TLSDESC')
          )
      c_cpp_args += '-mtls-dialect=gnu2'
    endif
  endif
endif

if with_glx != 'disabled'
  if not (with_platform_x11 and with_any_opengl)
    error('Cannot build GLX support without X11 platform support and at least one OpenGL API')
  elif with_glx == 'xlib'
    if not with_gallium
      error('xlib based GLX requires at least one gallium driver')
    elif not with_gallium_softpipe
      error('xlib based GLX requires softpipe or llvmpipe.')
    elif with_dri
      error('xlib conflicts with any dri driver')
    endif
  elif with_glx == 'dri'
    if not with_shared_glapi
      error('dri based GLX requires shared-glapi')
    endif
  endif
endif

with_glvnd = get_option('glvnd')
glvnd_vendor_name = get_option('glvnd-vendor-name')
if with_glvnd
  if with_platform_windows
    error('glvnd cannot be used on Windows')
  elif with_glx == 'xlib'
    error('Cannot build glvnd support for GLX that is not DRI based.')
  elif with_glx == 'disabled' and not with_egl
    error('glvnd requires DRI based GLX and/or EGL')
  endif
  if get_option('egl-lib-suffix') != ''
    error('''EGL lib suffix can't be used with libglvnd''')
  endif
endif

if with_vulkan_icd_dir == ''
  with_vulkan_icd_dir = join_paths(get_option('datadir'), 'vulkan/icd.d')
endif

# GNU/Hurd includes egl_dri2, without drm.
with_dri2 = (with_dri or with_any_vk) and (with_dri_platform == 'drm' or
  host_machine.system() == 'gnu')
with_dri3 = get_option('dri3').disable_auto_if(not (system_has_kms_drm and with_dri2)).allowed()

if with_any_vk and (with_platform_x11 and not with_dri3)
  error('Vulkan drivers require dri3 for X11 support')
endif
if with_dri
  if with_glx == 'disabled' and not with_egl and not with_gbm
    error('building dri drivers require at least one windowing system')
  endif
endif

if with_gallium_kmsro and (with_platform_x11 and not with_dri3)
  error('kmsro requires dri3 for X11 support')
endif

dep_dxheaders = null_dep
if with_gallium_d3d12 or with_microsoft_clc or with_microsoft_vk
  dep_dxheaders = dependency('directx-headers', required : false)
  if not dep_dxheaders.found()
    dep_dxheaders = dependency('DirectX-Headers',
      version : '>= 1.611.0',
      fallback : ['DirectX-Headers', 'dep_dxheaders'],
      required : with_gallium_d3d12 or with_microsoft_vk
    )
  endif
endif

_with_gallium_d3d12_video = get_option('gallium-d3d12-video')
with_gallium_d3d12_video = false
if with_gallium_d3d12 and not _with_gallium_d3d12_video.disabled()
  with_gallium_d3d12_video = true
  pre_args += '-DHAVE_GALLIUM_D3D12_VIDEO'
endif

_vdpau_drivers = [
  with_gallium_d3d12_video,
  with_gallium_nouveau,
  with_gallium_r600,
  with_gallium_radeonsi,
  with_gallium_virgl,
]

vdpau = get_option('gallium-vdpau') \
  .require(system_has_kms_drm, error_message : 'VDPAU state tracker can only be build on unix-like OSes.') \
  .require(with_platform_x11, error_message : 'VDPAU state tracker requires X11 support.') \
  .require(_vdpau_drivers.contains(true), error_message : 'VDPAU state tracker requires at least one of the following gallium drivers: r600, radeonsi, nouveau, d3d12 (with option gallium-d3d12-video, virgl).') 

dep_vdpau = dependency('vdpau', version : '>= 1.4', required : vdpau)
if dep_vdpau.found()
  dep_vdpau = dep_vdpau.partial_dependency(compile_args : true)
  pre_args += '-DHAVE_ST_VDPAU'
endif
with_gallium_vdpau = dep_vdpau.found()

vdpau_drivers_path = get_option('vdpau-libs-path')
if vdpau_drivers_path == ''
  vdpau_drivers_path = join_paths(get_option('libdir'), 'vdpau')
endif

prog_glslang = find_program('glslangValidator', native : true, required : with_vulkan_overlay_layer or with_aco_tests or with_amd_vk or with_intel_vk)
if prog_glslang.found()
  if run_command(prog_glslang, [ '--quiet', '--version' ], check : false).returncode() == 0
    glslang_quiet = ['--quiet']
  else
    glslang_quiet = []
  endif
endif

dep_xv = null_dep
_omx = get_option('gallium-omx')
if not system_has_kms_drm
  if ['auto', 'disabled'].contains(_omx)
    _omx = 'disabled'
  else
    error('OMX state tracker can only be built on unix-like OSes.')
  endif
elif not (with_gallium_r600 or with_gallium_radeonsi or with_gallium_nouveau)
  if ['auto', 'disabled'].contains(_omx)
    _omx = 'disabled'
  else
    error('OMX state tracker requires at least one of the following gallium drivers: r600, radeonsi, nouveau.')
  endif
endif
with_gallium_omx = _omx
dep_omx = null_dep
dep_omx_other = []
if ['auto', 'bellagio'].contains(_omx)
  dep_omx = dependency(
    'libomxil-bellagio', required : _omx == 'bellagio'
  )
  if dep_omx.found()
    with_gallium_omx = 'bellagio'
  endif
endif
if ['auto', 'tizonia'].contains(_omx)
  if with_dri and with_egl
    dep_omx = dependency(
      'libtizonia', version : '>= 0.10.0',
      required : _omx == 'tizonia',
    )
    dep_omx_other = [
      dependency('libtizplatform', required : _omx == 'tizonia'),
      dependency('tizilheaders', required : _omx == 'tizonia'),
    ]
    if dep_omx.found() and dep_omx_other[0].found() and dep_omx_other[1].found()
      with_gallium_omx = 'tizonia'
    endif
  elif _omx == 'tizonia'
    error('OMX-Tizonia state tracker requires dri and egl')
  endif
endif
if _omx == 'auto'
  with_gallium_omx = 'disabled'
else
  with_gallium_omx = _omx
endif

pre_args += [
  '-DENABLE_ST_OMX_BELLAGIO=' + (with_gallium_omx == 'bellagio' ? '1' : '0'),
  '-DENABLE_ST_OMX_TIZONIA=' + (with_gallium_omx == 'tizonia' ? '1' : '0'),
]


omx_drivers_path = get_option('omx-libs-path')

if with_gallium_omx != 'disabled'
  # Figure out where to put the omx driver.
  # FIXME: this could all be vastly simplified by adding a 'defined_variable'
  # argument to meson's get_variable method.
  if omx_drivers_path == ''
    _omx_libdir = dep_omx.get_variable(pkgconfig : 'libdir')
    _omx_drivers_dir = dep_omx.get_variable(pkgconfig : 'pluginsdir')
    if _omx_libdir == get_option('libdir')
      omx_drivers_path = _omx_drivers_dir
    else
      _omx_base_dir = []
      # This will fail on windows. Does OMX run on windows?
      _omx_libdir = _omx_libdir.split('/')
      _omx_drivers_dir = _omx_drivers_dir.split('/')
      foreach o : _omx_drivers_dir
        if not _omx_libdir.contains(o)
          _omx_base_dir += o
        endif
      endforeach
      omx_drivers_path = join_paths(get_option('libdir'), _omx_base_dir)
    endif
  endif
endif

_va_drivers = [
  with_gallium_d3d12_video,
  with_gallium_nouveau,
  with_gallium_r600,
  with_gallium_radeonsi,
  with_gallium_virgl,
]

_va = get_option('gallium-va') \
  .require(_va_drivers.contains(true),
           error_message : 'VA state tracker requires at least one of the following gallium drivers: r600, radeonsi, nouveau, d3d12 (with option gallium-d3d12-video), virgl.')
_dep_va_name = host_machine.system() == 'windows' ? 'libva-win32' : 'libva'
dep_va = dependency(_dep_va_name, version : '>= 1.8.0', required : _va)
if dep_va.found()
  dep_va_headers = dep_va.partial_dependency(compile_args : true)
  if cc.has_header_symbol('va/va.h', 'VASurfaceAttribDRMFormatModifiers',
                          dependencies: dep_va_headers)
    pre_args += '-DHAVE_VA_SURFACE_ATTRIB_DRM_FORMAT_MODIFIERS'
  endif
endif
with_gallium_va = dep_va.found()

va_drivers_path = get_option('va-libs-path')
if va_drivers_path == ''
  va_drivers_path = join_paths(get_option('libdir'), 'dri')
endif

with_gallium_xa = get_option('gallium-xa') \
  .require(system_has_kms_drm, error_message : 'XA state tracker can only be built on unix-like OSes.') \
  .require(with_gallium_nouveau or with_gallium_freedreno or with_gallium_i915 or with_gallium_svga,
           error_message : 'XA state tracker requires at least one of the following gallium drivers: nouveau, freedreno, i915, svga.') \
  .allowed()

d3d_drivers_path = get_option('d3d-drivers-path')
if d3d_drivers_path == ''
  d3d_drivers_path = join_paths(get_option('prefix'), get_option('libdir'), 'd3d')
endif

with_gallium_st_nine =  get_option('gallium-nine')
if with_gallium_st_nine
  if not with_gallium_softpipe
    error('The nine state tracker requires gallium softpipe/llvmpipe.')
  elif not [
             with_gallium_crocus,
             with_gallium_freedreno,
             with_gallium_i915,
             with_gallium_iris,
             with_gallium_nouveau,
             with_gallium_panfrost,
             with_gallium_r300,
             with_gallium_r600,
             with_gallium_radeonsi,
             with_gallium_svga,
             with_gallium_zink,
           ].contains(true)
    error('The nine state tracker requires at least one non-swrast gallium driver.')
  endif
  if not with_dri3
    error('Using nine with wine requires dri3')
  endif
endif
with_gallium_st_d3d10umd =  get_option('gallium-d3d10umd')
if with_gallium_st_d3d10umd
  if not with_gallium_softpipe
    error('The d3d10umd state tracker requires gallium softpipe/llvmpipe.')
  endif
endif
_power8 = get_option('power8')
if _power8.allowed()
  if host_machine.cpu_family() == 'ppc64' and host_machine.endian() == 'little'
    if cc.get_id() == 'gcc' and cc.version().version_compare('< 4.8')
      error('Altivec is not supported with gcc version < 4.8.')
    endif
    if cc.compiles('''
        #include <altivec.h>
        int main() {
          vector unsigned char r;
          vector unsigned int v = vec_splat_u32 (1);
          r = __builtin_vec_vgbbd ((vector unsigned char) v);
          return 0;
        }''',
        args : '-mpower8-vector',
        name : 'POWER8 intrinsics')
      pre_args += ['-D_ARCH_PWR8']
      c_cpp_args += '-mpower8-vector'
    elif _power8.enabled()
      error('POWER8 intrinsic support required but not found.')
    endif
  endif
endif

if get_option('vmware-mks-stats')
  if not with_gallium_svga
    error('vmware-mks-stats requires gallium VMware/svga driver.')
  endif
  pre_args += '-DVMX86_STATS=1'
endif

_opencl = get_option('gallium-opencl')
_rtti = get_option('cpp_rtti')
if _opencl != 'disabled'
  if not with_gallium
    error('OpenCL Clover implementation requires at least one gallium driver.')
  endif
  if not _rtti
    error('The Clover OpenCL state tracker requires rtti')
  endif

  with_gallium_opencl = true
  with_opencl_icd = _opencl == 'icd'
else
  with_gallium_opencl = false
  with_opencl_icd = false
endif

with_gallium_rusticl = get_option('gallium-rusticl')
if with_gallium_rusticl
  if not with_gallium
    error('rusticl requires at least one gallium driver.')
  endif

  if meson.version().version_compare('< 1.3.1')
    error('rusticl requires meson 1.3.1 or newer')
  endif

  add_languages('rust', required: true)
  rustc = meson.get_compiler('rust')

  with_clc = true
endif

dep_clc = null_dep
if with_gallium_opencl or with_clc
  dep_clc = dependency('libclc')
endif

gl_pkgconfig_c_flags = []
with_glx_indirect_rendering = false
if with_platform_x11
  if with_glx == 'xlib'
    pre_args += '-DUSE_XSHM'
  else
    with_glx_indirect_rendering = true
    pre_args += '-DGLX_INDIRECT_RENDERING'
    if with_glx_direct
      pre_args += '-DGLX_DIRECT_RENDERING'
    endif
    if with_dri_platform == 'drm'
      pre_args += '-DGLX_USE_DRM'
    elif with_dri_platform == 'apple'
      pre_args += '-DGLX_USE_APPLEGL'
    elif with_dri_platform == 'windows'
      pre_args += '-DGLX_USE_WINDOWSGL'
    endif
  endif
endif

with_glapi_export_proto_entry_points = false
if with_shared_glapi and not with_glx_indirect_rendering
  # Imply !defined(GLX_INDIRECT_RENDERING)
  with_glapi_export_proto_entry_points = true
endif
pre_args += '-DGLAPI_EXPORT_PROTO_ENTRY_POINTS=@0@'.format(with_glapi_export_proto_entry_points ? '1' : '0')

with_android_stub = get_option('android-stub')
if with_android_stub and not with_platform_android
  error('`-D android-stub=true` makes no sense without `-D platforms=android`')
endif

with_libbacktrace = get_option('android-libbacktrace') \
  .require(with_platform_android, error_message : '`-D android-libbacktrace=enabled` makes no sense without `-D platforms=android`') \
  .disable_auto_if(not with_platform_android) \
  .allowed()

if with_libbacktrace
  cpp_args += '-DWITH_LIBBACKTRACE'
endif

if with_platform_android
  dep_android_mapper4 = null_dep
  if not with_android_stub
    dep_android = [
      dependency('cutils'),
      dependency('hardware'),
      dependency('sync'),
    ]
    if with_libbacktrace
      dep_android += dependency('backtrace')
    endif
    if get_option('platform-sdk-version') >= 26
      dep_android += dependency('nativewindow')
    endif
    if get_option('platform-sdk-version') >= 30
      dep_android_mapper4 = dependency('android.hardware.graphics.mapper', version : '>= 4.0', required : false)
    endif
  endif
  pre_args += [
    '-DANDROID',
    '-DANDROID_API_LEVEL=' + get_option('platform-sdk-version').to_string()
  ]
  if get_option('android-strict')
    pre_args += '-DANDROID_STRICT'
  endif
endif

# On Android, seccomp kills the process on kernels without
# CONFIG_KCMP/CONFIG_CHECKPOINT_RESTORE if it attemps to use KCMP.
# Since we can't detect that, err on the side of caution and disable
# KCMP by default on Android.
if get_option('allow-kcmp') \
    .disable_auto_if(with_platform_android) \
    .allowed()
  pre_args += '-DALLOW_KCMP'
endif

prog_python = import('python').find_installation('python3')
has_mako = run_command(
  prog_python, '-c',
  '''
try:
  from packaging.version import Version
except:
  from distutils.version import StrictVersion as Version
import mako
assert Version(mako.__version__) >= Version("0.8.0")
  ''', check: false)
if has_mako.returncode() != 0
  error('Python (3.x) mako module >= 0.8.0 required to build mesa.')
endif

if cc.get_id() == 'gcc' and cc.version().version_compare('< 4.4.6')
  error('When using GCC, version 4.4.6 or later is required.')
endif

# Support systems without ETIME (e.g. FreeBSD)
if cc.get_define('ETIME', prefix : '#include <errno.h>') == ''
  pre_args += '-DETIME=ETIMEDOUT'
endif

# Define DEBUG for debug builds only (debugoptimized is not included on this one)
if get_option('buildtype') == 'debug'
  pre_args += '-DDEBUG'
endif

with_shader_cache = get_option('shader-cache') \
  .require(host_machine.system() != 'windows', error_message : 'Shader Cache does not currently work on Windows') \
  .allowed() 

if with_shader_cache
  pre_args += '-DENABLE_SHADER_CACHE'
  if not get_option('shader-cache-default')
    pre_args += '-DSHADER_CACHE_DISABLE_BY_DEFAULT'
  endif

  shader_cache_max_size = get_option('shader-cache-max-size')
  if shader_cache_max_size != ''
    pre_args += '-DMESA_SHADER_CACHE_MAX_SIZE="@0@"'.format(shader_cache_max_size)
  endif
endif

# Check for GCC style builtins
foreach b : ['bswap32', 'bswap64', 'clz', 'clzll', 'ctz', 'expect', 'ffs',
             'ffsll', 'popcount', 'popcountll', 'unreachable', 'types_compatible_p']
  if cc.has_function(b)
    pre_args += '-DHAVE___BUILTIN_@0@'.format(b.to_upper())
  endif
endforeach

# check for GCC __attribute__
_attributes = [
  'const', 'flatten', 'malloc', 'pure', 'unused', 'warn_unused_result',
  'weak', 'format', 'packed', 'returns_nonnull', 'alias', 'noreturn',
]
foreach a : cc.get_supported_function_attributes(_attributes)
  pre_args += '-DHAVE_FUNC_ATTRIBUTE_@0@'.format(a.to_upper())
endforeach
if cc.has_function_attribute('visibility:hidden')
  pre_args += '-DHAVE_FUNC_ATTRIBUTE_VISIBILITY'
endif
if cc.compiles('__uint128_t foo(void) { return 0; }',
               name : '__uint128_t')
  pre_args += '-DHAVE_UINT128'
endif

if cc.has_function('reallocarray')
   pre_args += '-DHAVE_REALLOCARRAY'
endif
if cc.has_function('fmemopen')
   pre_args += '-DHAVE_FMEMOPEN'
endif

# TODO: this is very incomplete
if ['linux', 'cygwin', 'gnu', 'freebsd', 'gnu/kfreebsd', 'haiku', 'android', 'managarm'].contains(host_machine.system())
  pre_args += '-D_GNU_SOURCE'
elif host_machine.system() == 'sunos'
  pre_args += '-D__EXTENSIONS__'
elif host_machine.system() == 'windows'
  pre_args += [
    '-D_WINDOWS', '-D_WIN32_WINNT=0x0A00', '-DWINVER=0x0A00',
    '-DPIPE_SUBSYSTEM_WINDOWS_USER',
    '-D_USE_MATH_DEFINES',  # XXX: scons didn't use this for mingw
  ]
  if cc.get_argument_syntax() == 'msvc'
    pre_args += [
      '-DVC_EXTRALEAN',
      '-D_CRT_SECURE_NO_WARNINGS',
      '-D_CRT_SECURE_NO_DEPRECATE',
      '-D_SCL_SECURE_NO_WARNINGS',
      '-D_SCL_SECURE_NO_DEPRECATE',
      '-D_ALLOW_KEYWORD_MACROS',
      '-D_HAS_EXCEPTIONS=0', # Tell C++ STL to not use exceptions
      '-DNOMINMAX',
    ]
  else
    # When the target is not mingw/ucrt
    # NOTE: clang's stddef.h are conflict with mingw/ucrt's stddef.h
    # So do not include headers that defined in clang for detecting
    # _UCRT
    if cc.compiles('''
      #include <string.h>
      #if defined(__MINGW32__) && defined(_UCRT)
      #error
      #endif
      int main(void) { return 0; }''')
      pre_args += ['-D__MSVCRT_VERSION__=0x0700']
    endif
  endif
elif host_machine.system() == 'openbsd'
  pre_args += '-D_ISOC11_SOURCE'
endif

# Check for generic C arguments
c_msvc_compat_args = []
no_override_init_args = []
cpp_msvc_compat_args = []
ld_args_gc_sections = []
if cc.get_argument_syntax() == 'msvc'
  _trial = [
    '/wd4018',  # signed/unsigned mismatch
    '/wd4056',  # overflow in floating-point constant arithmetic
    '/wd4244',  # conversion from 'type1' to 'type2', possible loss of data
    '/wd4267',  # 'var' : conversion from 'size_t' to 'type', possible loss of data
    '/wd4305',  # truncation from 'type1' to 'type2'
    '/wd4351',  # new behavior: elements of array 'array' will be default initialized
    '/wd4756',  # overflow in constant arithmetic
    '/wd4800',  # forcing value to bool 'true' or 'false' (performance warning)
    '/wd4996',  # disabled deprecated POSIX name warnings
    '/wd4291',  # no matching operator delete found
    '/wd4146',  # unary minus operator applied to unsigned type, result still unsigned
    '/wd4200',  # nonstandard extension used: zero-sized array in struct/union
    '/wd4624',  # destructor was implicitly defined as deleted [from LLVM]
    '/wd4309',  # 'initializing': truncation of constant value
    '/wd4838',  # conversion from 'int' to 'const char' requires a narrowing conversion
    '/wd5105',  # macro expansion producing 'defined' has undefined behavior (winbase.h, need Windows SDK upgrade)
    '/we4020',  # Error when passing the wrong number of parameters
    '/we4024',  # Error when passing different type of parameter
    '/we4189',  # 'identifier' : local variable is initialized but not referenced
    '/Zc:__cplusplus', #Set __cplusplus macro to match the /std:c++<version> on the command line
  ]
  c_args += cc.get_supported_arguments(_trial)
  cpp_args += cpp.get_supported_arguments(_trial)
else
  _trial_c = [
    '-Werror=implicit-function-declaration',
    '-Werror=missing-prototypes',
    '-Werror=return-type',
    '-Werror=empty-body',
    '-Werror=incompatible-pointer-types',
    '-Werror=int-conversion',
    '-Wimplicit-fallthrough',
    '-Wmisleading-indentation',
    '-Wno-missing-field-initializers',
    '-Wno-format-truncation',
    '-Wno-nonnull-compare',
    '-fno-math-errno',
    '-fno-trapping-math',
    '-Qunused-arguments',
    '-fno-common',
    '-Wno-unknown-pragmas',
    # Clang
    '-Wno-microsoft-enum-value',
    '-Wno-unused-function',
  ]
  _trial_cpp = [
    '-Werror=return-type',
    '-Werror=empty-body',
    '-Wmisleading-indentation',
    '-Wno-non-virtual-dtor',
    '-Wno-missing-field-initializers',
    '-Wno-format-truncation',
    '-fno-math-errno',
    '-fno-trapping-math',
    '-Qunused-arguments',
    # Some classes use custom new operator which zeroes memory, however
    # gcc does aggressive dead-store elimination which threats all writes
    # to the memory before the constructor as "dead stores".
    # For now we disable this optimization.
    '-flifetime-dse=1',
    '-Wno-unknown-pragmas',
    # Clang
    '-Wno-microsoft-enum-value',
  ]

  # MinGW chokes on format specifiers and I can't get it all working
  if not (cc.get_argument_syntax() == 'gcc' and host_machine.system() == 'windows')
    _trial_c += ['-Wformat-security']
    _trial_cpp += ['-Wformat-security']
  endif

  # FreeBSD annotated <pthread.h> but Mesa isn't ready
  if not (cc.get_id() == 'clang' and host_machine.system() == 'freebsd')
    _trial_c += ['-Werror=thread-safety']
  endif

  # If the compiler supports it, put function and data symbols in their
  # own sections and GC the sections after linking.  This lets drivers
  # drop shared code unused by that specific driver (particularly
  # relevant for Vulkan drivers).
  if cc.links('static char unused() { return 5; } int main() { return 0; }',
              args : '-Wl,--gc-sections', name : 'gc-sections')
    ld_args_gc_sections += '-Wl,--gc-sections'
    _trial_c += ['-ffunction-sections', '-fdata-sections']
    _trial_cpp += ['-ffunction-sections', '-fdata-sections']
  endif

  # Variables that are only used for assertions are considered unused when assertions
  # are disabled. Don't treat this as an error, since we build with -Werror even if
  # assertions are disabled.
  if get_option('b_ndebug') == 'true' or (get_option('buildtype') == 'release' and get_option('b_ndebug') == 'if-release')
    _trial_c += ['-Wno-unused-variable', '-Wno-unused-but-set-variable', '/wd4189']
    _trial_cpp += ['-Wno-unused-variable', '-Wno-unused-but-set-variable', '/wd4189']
  endif

  c_args += cc.get_supported_arguments(_trial_c)
  cpp_args += cpp.get_supported_arguments(_trial_cpp)

  no_override_init_args += cc.get_supported_arguments(
    ['-Wno-override-init', '-Wno-initializer-overrides']
  )

  # Check for C and C++ arguments for MSVC compatibility. These are only used
  # in parts of the mesa code base that need to compile with MSVC, mainly
  # common code
  _trial_msvc = ['-Werror=pointer-arith', '-Werror=vla', '-Werror=gnu-empty-initializer']
  c_msvc_compat_args += cc.get_supported_arguments(_trial_msvc)
  cpp_msvc_compat_args += cpp.get_supported_arguments(_trial_msvc)
endif

# set linker arguments
if host_machine.system() == 'windows'
  if cc.get_argument_syntax() == 'msvc'
    add_project_link_arguments(
      '/fixed:no',
      '/dynamicbase',
      '/nxcompat',
      language : ['c', 'cpp'],
    )
    if get_option('buildtype') != 'debug'
      add_project_link_arguments(
        '/incremental:no',
        language : ['c', 'cpp'],
      )
    endif
  else
    add_project_link_arguments(
      cc.get_supported_link_arguments(
        '-Wl,--nxcompat',
        '-Wl,--dynamicbase',
        '-static-libgcc',
        '-static-libstdc++',
      ),
      language : ['c'],
    )
    add_project_link_arguments(
      cpp.get_supported_link_arguments(
        '-Wl,--nxcompat',
        '-Wl,--dynamicbase',
        '-static-libgcc',
        '-static-libstdc++',
      ),
      language : ['cpp'],
    )
  endif
endif

sse2_arg = []
sse2_args = []
sse41_args = []
with_sse41 = false
if host_machine.cpu_family().startswith('x86')
  pre_args += '-DUSE_SSE41'
  with_sse41 = true

  if cc.get_id() != 'msvc'
    sse41_args = ['-msse4.1']

    if host_machine.cpu_family() == 'x86'
      # x86_64 have sse2 by default, so sse2 args only for x86
      sse2_arg = ['-msse2', '-mfpmath=sse']
      sse2_args = [sse2_arg, '-mstackrealign']
      if get_option('sse2')
        # These settings make generated GCC code match MSVC and follow
        # GCC advice on https://gcc.gnu.org/wiki/FloatingPointMath#x86note
        #
        # NOTE: We need to ensure stack is realigned given that we
        # produce shared objects, and have no control over the stack
        # alignment policy of the application. Therefore we need
        # -mstackrealign or -mincoming-stack-boundary=2.
        #
        # XXX: We could have SSE without -mstackrealign if we always used
        # __attribute__((force_align_arg_pointer)), but that's not
        # always the case.
        c_cpp_args += sse2_args
        # sse2_args are adopted into c_cpp_args to avoid duplicated sse2 command line args
        sse2_arg = []
        sse2_args = []
      else
        # GCC on x86 (not x86_64) with -msse* assumes a 16 byte aligned stack, but
        # that's not guaranteed
        sse41_args += '-mstackrealign'
      endif
    endif
  endif
endif

# Detect __builtin_ia32_clflushopt support
if cc.has_function('__builtin_ia32_clflushopt', args : '-mclflushopt')
  pre_args += '-DHAVE___BUILTIN_IA32_CLFLUSHOPT'
  clflushopt_args = ['-mclflushopt']
  with_clflushopt = true
else
  clflushopt_args = []
  with_clflushopt = false
endif

# Check for GCC style atomics
dep_atomic = null_dep

if cc.compiles('''#include <stdint.h>
                  int main() {
                    struct {
                      uint64_t *v;
                    } x;
                    return (int)__atomic_load_n(x.v, __ATOMIC_ACQUIRE) &
                           (int)__atomic_add_fetch(x.v, (uint64_t)1, __ATOMIC_ACQ_REL);

                  }''',
               name : 'GCC atomic builtins')
  pre_args += '-DUSE_GCC_ATOMIC_BUILTINS'

  # Not all atomic calls can be turned into lock-free instructions, in which
  # GCC will make calls into the libatomic library. Check whether we need to
  # link with -latomic.
  #
  # This can happen for 64-bit atomic operations on 32-bit architectures such
  # as ARM.
  if not cc.links('''#include <stdint.h>
                     int main() {
                       struct {
                         uint64_t *v;
                       } x;
                       return (int)__atomic_load_n(x.v, __ATOMIC_ACQUIRE) &
                              (int)__atomic_add_fetch(x.v, (uint64_t)1, __ATOMIC_ACQ_REL);
                     }''',
                  name : 'GCC atomic builtins required -latomic')
    dep_atomic = cc.find_library('atomic')
  endif
endif
if not cc.links('''#include <stdint.h>
                   uint64_t v;
                   int main() {
                     return __sync_add_and_fetch(&v, (uint64_t)1);
                   }''',
                dependencies : dep_atomic,
                name : 'GCC 64bit atomics')
  pre_args += '-DMISSING_64BIT_ATOMICS'
endif

dep_ws2_32 = cc.find_library('ws2_32', required : with_platform_windows)

# TODO: shared/static? Is this even worth doing?

with_asm_arch = ''
if host_machine.cpu_family() == 'x86'
  if system_has_kms_drm or host_machine.system() == 'gnu'
    with_asm_arch = 'x86'
    pre_args += ['-DUSE_X86_ASM']

    if with_glx_read_only_text
      pre_args += ['-DGLX_X86_READONLY_TEXT']
    endif
  endif
elif host_machine.cpu_family() == 'x86_64'
  if system_has_kms_drm
    with_asm_arch = 'x86_64'
    pre_args += ['-DUSE_X86_64_ASM']
  endif
elif host_machine.cpu_family() == 'arm'
  if system_has_kms_drm
    with_asm_arch = 'arm'
    pre_args += ['-DUSE_ARM_ASM']
  endif
elif host_machine.cpu_family() == 'aarch64'
  if system_has_kms_drm
    with_asm_arch = 'aarch64'
    pre_args += ['-DUSE_AARCH64_ASM']
  endif
elif host_machine.cpu_family() == 'sparc64'
  if system_has_kms_drm
    with_asm_arch = 'sparc'
    pre_args += ['-DUSE_SPARC_ASM']
  endif
elif host_machine.cpu_family() == 'ppc64' and host_machine.endian() == 'little'
  if system_has_kms_drm
    with_asm_arch = 'ppc64le'
    pre_args += ['-DUSE_PPC64LE_ASM']
  endif
elif host_machine.cpu_family() == 'mips64' and host_machine.endian() == 'little'
  if system_has_kms_drm
    with_asm_arch = 'mips64el'
    pre_args += ['-DUSE_MIPS64EL_ASM']
  endif
elif host_machine.cpu_family() == 'loongarch64'
  if system_has_kms_drm
    with_asm_arch = 'loongarch64'
    pre_args += ['-DUSE_LOONGARCH64_ASM']
  endif
endif

# Check for standard headers and functions
if (cc.has_header_symbol('sys/sysmacros.h', 'major') and
  cc.has_header_symbol('sys/sysmacros.h', 'minor') and
  cc.has_header_symbol('sys/sysmacros.h', 'makedev'))
  pre_args += '-DMAJOR_IN_SYSMACROS'
endif
if (cc.has_header_symbol('sys/mkdev.h', 'major') and
  cc.has_header_symbol('sys/mkdev.h', 'minor') and
  cc.has_header_symbol('sys/mkdev.h', 'makedev'))
  pre_args += '-DMAJOR_IN_MKDEV'
endif

if cc.check_header('sched.h')
  pre_args += '-DHAS_SCHED_H'
  if cc.has_function('sched_getaffinity')
    pre_args += '-DHAS_SCHED_GETAFFINITY'
  endif
endif

if not ['linux'].contains(host_machine.system())
  # Deprecated on Linux and requires <sys/types.h> on FreeBSD and OpenBSD
  if cc.check_header('sys/sysctl.h', prefix : '#include <sys/types.h>')
    pre_args += '-DHAVE_SYS_SYSCTL_H'
  endif
endif

foreach h : ['xlocale.h', 'linux/futex.h', 'endian.h', 'dlfcn.h', 'sys/shm.h',
             'cet.h', 'pthread_np.h', 'renderdoc_app.h', 'sys/inotify.h']
  if cc.check_header(h)
    pre_args += '-DHAVE_@0@'.format(h.to_upper().underscorify())
  endif
endforeach

functions_to_detect = {
  'strtof': '',
  'mkostemp': '',
  'memfd_create': '',
  'random_r': '',
  'flock': '',
  'strtok_r': '',
  'getrandom': '',
  'qsort_s': '',
  'posix_fallocate': '',
  'secure_getenv': '',
}

foreach f, prefix: functions_to_detect
  if cc.has_function(f, prefix: prefix)
    pre_args += '-DHAVE_@0@'.format(f.to_upper())
  endif
endforeach

if cpp.links('''
    #define _GNU_SOURCE
    #include <stdlib.h>

    static int dcomp(const void *l, const void *r, void *t) { return 0; }

    int main(int ac, char **av) {
      int arr[] = { 1 };
      void *t = NULL;
      qsort_r((void*)&arr[0], 1, 1, dcomp, t);
      return (0);
    }''',
    args : pre_args,
    name : 'GNU qsort_r')
  pre_args += '-DHAVE_GNU_QSORT_R'
elif cpp.links('''
    #include <stdlib.h>

    static int dcomp(void *t, const void *l, const void *r) { return 0; }

    int main(int ac, char **av) {
      int arr[] = { 1 };
      void *t = NULL;
      qsort_r((void*)&arr[0], 1, 1, t, dcomp);
      return (0);
    }''',
    args : pre_args,
    name : 'BSD qsort_r')
  pre_args += '-DHAVE_BSD_QSORT_R'
endif

if cc.has_header_symbol('time.h', 'struct timespec')
   pre_args += '-DHAVE_STRUCT_TIMESPEC'
endif

with_c11_threads = false
if cc.has_function('thrd_create', prefix: '#include <threads.h>')
  if with_platform_android
    # Current only Android's c11 <threads.h> are verified
    pre_args += '-DHAVE_THRD_CREATE'
    with_c11_threads = true
  endif
endif

if cc.has_header_symbol('errno.h', 'program_invocation_name',
                        args : '-D_GNU_SOURCE')
   pre_args += '-DHAVE_PROGRAM_INVOCATION_NAME'
elif with_tools.contains('intel')
  error('Intel tools require the program_invocation_name variable')
endif

if cc.has_header_symbol('math.h', 'issignaling',
                        args : '-D_GNU_SOURCE')
   pre_args += '-DHAVE_ISSIGNALING'
endif

# MinGW provides a __builtin_posix_memalign function, but not a posix_memalign.
# This means that this check will succeed, but then compilation will later
# fail. MSVC doesn't have this function at all, so only check for it on
# non-windows platforms.
if host_machine.system() != 'windows'
  if cc.has_function('posix_memalign')
    pre_args += '-DHAVE_POSIX_MEMALIGN'
  endif
endif

if cc.has_member('struct dirent', 'd_type', prefix: '''#include <sys/types.h>
   #include <dirent.h>''')
   pre_args += '-DHAVE_DIRENT_D_TYPE'
endif

# strtod locale support
if cc.links('''
    #define _GNU_SOURCE
    #include <stdlib.h>
    #include <locale.h>
    #ifdef HAVE_XLOCALE_H
    #include <xlocale.h>
    #endif
    int main() {
      locale_t loc = newlocale(LC_CTYPE_MASK, "C", NULL);
      const char *s = "1.0";
      char *end;
      double d = strtod_l(s, &end, loc);
      float f = strtof_l(s, &end, loc);
      freelocale(loc);
      return 0;
    }''',
    args : pre_args,
    name : 'strtod has locale support')
  pre_args += '-DHAVE_STRTOD_L'
endif

# Check for some linker flags
ld_args_bsymbolic = []
if cc.links('int main() { return 0; }', args : '-Wl,-Bsymbolic', name : 'Bsymbolic')
  ld_args_bsymbolic += '-Wl,-Bsymbolic'
endif
with_ld_version_script = false
if cc.links('int main() { return 0; }',
            args : '-Wl,--version-script=@0@'.format(
              join_paths(meson.current_source_dir(), 'build-support/conftest.map')),
            name : 'version-script')
  with_ld_version_script = true
endif
with_ld_dynamic_list = false
if cc.links('int main() { return 0; }',
            args : '-Wl,--dynamic-list=@0@'.format(
              join_paths(meson.current_source_dir(), 'build-support/conftest.dyn')),
            name : 'dynamic-list')
  with_ld_dynamic_list = true
endif

ld_args_build_id = cc.get_supported_link_arguments('-Wl,--build-id=sha1')

# check for dl support
dep_dl = null_dep
if host_machine.system() != 'windows'
  if not cc.has_function('dlopen')
    dep_dl = cc.find_library('dl', required : true)
  endif
  if cc.has_function('dladdr', dependencies : dep_dl)
    # This is really only required for util/disk_cache.h
    pre_args += '-DHAVE_DLADDR'
  endif
endif

if cc.has_function('dl_iterate_phdr')
  pre_args += '-DHAVE_DL_ITERATE_PHDR'
elif with_intel_vk or with_intel_hasvk
  error('Intel "Anvil" Vulkan driver requires the dl_iterate_phdr function')
endif

if with_any_intel and ['x86', 'x86_64'].contains(host_machine.cpu_family())
  pre_args += '-DSUPPORT_INTEL_INTEGRATED_GPUS'
endif

if get_option('intel-xe-kmd').enabled()
  pre_args += '-DINTEL_XE_KMD_SUPPORTED'
endif


if with_gallium_i915 and host_machine.cpu_family().startswith('x86') == false
  error('Intel "i915" Gallium driver requires x86 or x86_64 CPU family')
endif

# Determine whether or not the rt library is needed for time functions
if host_machine.system() == 'windows' or cc.has_function('clock_gettime')
  dep_clock = null_dep
else
  dep_clock = cc.find_library('rt')
endif

dep_zlib = dependency('zlib', version : '>= 1.2.3',
                      allow_fallback: true,
                      required : get_option('zlib'))
#if dep_zlib.found()
  pre_args += '-DHAVE_ZLIB'
#endif

dep_zstd = [] #dependency('libzstd', required : get_option('zstd'))
#if dep_zstd.found()
#  pre_args += '-DHAVE_ZSTD'
#endif

with_compression = true #dep_zlib.found() or dep_zstd.found()
if with_compression
  pre_args += '-DHAVE_COMPRESSION'
elif with_shader_cache
  error('Shader Cache requires compression')
endif

if host_machine.system() == 'windows'
  # For MSVC and MinGW we aren't using pthreads, and dependency('threads') will add linkage
  # to pthread for MinGW, so leave the dependency null_dep for Windows. For Windows linking to
  # kernel32 is enough for c11/threads.h and it's already linked by meson by default
  dep_thread = null_dep
else
  dep_thread = dependency('threads')
endif
if dep_thread.found()
  pre_args += '-DHAVE_PTHREAD'
  if host_machine.system() != 'netbsd' and cc.has_function(
      'pthread_setaffinity_np',
      dependencies : dep_thread,
      prefix : '#include <pthread.h>',
      args : '-D_GNU_SOURCE')
    pre_args += '-DHAVE_PTHREAD_SETAFFINITY'
  endif
endif

with_expat = get_option('expat') \
  .disable_auto_if(with_platform_android or with_platform_windows)

if host_machine.system() == 'darwin'
  dep_expat = meson.get_compiler('c').find_library('expat', required : with_expat)
else
  dep_expat = dependency('expat', allow_fallback: true,
                         required : with_expat)
endif

# TODO: with Meson 1.1.0 this can be replaced with with_expat.enable_if(with_intel_tools)
if with_intel_tools and not dep_expat.found()
  error('Intel tools require expat')
endif

# We don't require expat on Android or Windows
use_xmlconfig = get_option('xmlconfig') \
  .require(not (with_platform_android or with_platform_windows),
           error_message : 'xmlconfig not available on Android or Windows') \
  .require(dep_expat.found(),
           error_message : 'requires expat') \
  .allowed()
  
# Predefined macros for windows
if host_machine.system() == 'windows'
  pre_args += '-DWIN32_LEAN_AND_MEAN' # http://msdn2.microsoft.com/en-us/library/6dwk3a1z.aspx
endif
# this only exists on linux so either this is linux and it will be found, or
# it's not linux and wont
dep_m = cc.find_library('m', required : false)

if host_machine.system() == 'windows'
  dep_regex = meson.get_compiler('c').find_library('regex', required : false)
  if not dep_regex.found()
    dep_regex = declare_dependency(compile_args : ['-DNO_REGEX'])
  endif
else
  dep_regex = null_dep
endif

if with_platform_haiku
  dep_network = cc.find_library('network')
endif

dep_futex = null_dep
if host_machine.system() == 'windows'
  if (get_option('min-windows-version') < 8)
    pre_args += '-DWINDOWS_NO_FUTEX'
  else
    dep_futex = cc.find_library('synchronization', required : true)
  endif
endif

# Check for libdrm. Various drivers have different libdrm version requirements,
# but we always want to use the same version for all libdrm modules. That means
# even if driver foo requires 2.4.0 and driver bar requires 2.4.3, if foo and
# bar are both on use 2.4.3 for both of them
dep_libdrm_amdgpu = null_dep
dep_libdrm_radeon = null_dep
dep_libdrm_nouveau = null_dep
dep_libdrm_intel = null_dep

_drm_amdgpu_ver = '2.4.119'
_drm_radeon_ver = '2.4.71'
_drm_nouveau_ver = '2.4.102'
_drm_intel_ver = '2.4.75'
_drm_ver = '2.4.109'

_libdrm_checks = [
  ['intel', with_gallium_i915],
  ['amdgpu', (with_amd_vk and not with_platform_windows) or with_gallium_radeonsi],
  ['radeon', (with_gallium_radeonsi or with_gallium_r300 or with_gallium_r600)],
  ['nouveau', with_any_nouveau],
]

# Loop over the enables versions and get the highest libdrm requirement for all
# active drivers.
_drm_blame = ''
foreach d : _libdrm_checks
  ver = get_variable('_drm_@0@_ver'.format(d[0]))
  if d[1] and ver.version_compare('>' + _drm_ver)
    _drm_ver = ver
    _drm_blame = d[0]
  endif
endforeach
if _drm_blame != ''
  message('libdrm @0@ needed because @1@ has the highest requirement'.format(_drm_ver, _drm_blame))
endif

# Then get each libdrm module
#foreach d : _libdrm_checks
#  if d[1]
#    set_variable(
#      'dep_libdrm_' + d[0],
#      dependency('libdrm_' + d[0], version : '>=' + _drm_ver)
#    )
#  endif
#endforeach

add_project_arguments(pre_args, language : ['c', 'cpp'])
add_project_arguments(c_cpp_args, language : ['c', 'cpp'])

add_project_arguments(c_args,   language : ['c'])
add_project_arguments(cpp_args, language : ['cpp'])


with_gallium_drisw_kms = false
if system_has_kms_drm
  dep_libdrm = declare_dependency(
    link_with: static_library('libdrm.a'),
    include_directories: include_directories('.'),
    # GNU/Hurd includes egl_dri2, without drm.
    #required: (with_dri2 and host_machine.system() != 'gnu') or with_dri3
  )
else
  # We should prevent libdrm from being available when the target doesn't have it to avoid transitive
  # dependencies (such as vk-runtime) linking to it
  dep_libdrm = null_dep
endif
#if dep_libdrm.found()
  pre_args += '-DHAVE_LIBDRM'
  if with_dri_platform == 'drm' and with_dri
    with_gallium_drisw_kms = true
  endif
#endif

dep_libudev = dependency('libudev', required : false)
if dep_libudev.found()
  pre_args += '-DHAVE_LIBUDEV'
endif

llvm_modules = ['bitwriter', 'engine', 'mcdisassembler', 'mcjit', 'core', 'executionengine', 'scalaropts', 'transformutils', 'instcombine']
llvm_optional_modules = ['coroutines']
if with_amd_vk or with_gallium_radeonsi or with_gallium_r600
  llvm_modules += ['amdgpu', 'bitreader', 'ipo']
  if with_gallium_r600
    llvm_modules += 'asmparser'
  endif
endif
if with_gallium_opencl
  llvm_modules += [
    'linker', 'coverage', 'instrumentation', 'ipo', 'irreader',
    'lto', 'option', 'objcarcopts', 'profiledata'
  ]
  # all-targets is needed to support static linking LLVM build with multiple targets
  # windowsdriver is needded with LLVM>=15, but we don't know what LLVM verrsion we are using yet
  llvm_optional_modules += ['all-targets', 'frontendopenmp', 'windowsdriver']
endif
if with_clc
  llvm_modules += ['coverage', 'target', 'linker', 'irreader', 'option', 'libdriver', 'lto']
  # all-targets is needed to support static linking LLVM build with multiple targets.
  # windowsdriver is needded with LLVM>=15 and frontendhlsl is needed with LLVM>=16,
  # but we don't know what LLVM version we are using yet
  llvm_optional_modules += ['all-targets', 'windowsdriver', 'frontendhlsl']
endif
draw_with_llvm = get_option('draw-use-llvm')
if draw_with_llvm
  llvm_modules += 'native'
  # lto is needded with LLVM>=15, but we don't know what LLVM verrsion we are using yet
  llvm_optional_modules += ['lto']
endif

if with_amd_vk or with_gallium_radeonsi
  _llvm_version = '>= 15.0.0'
elif with_intel_clc
  _llvm_version = '>= 13.0.0'
elif with_gallium_opencl
  _llvm_version = '>= 11.0.0'
elif with_clc
  _llvm_version = '>= 10.0.0'
else
  _llvm_version = '>= 5.0.0'
endif

_shared_llvm = get_option('shared-llvm') \
  .disable_auto_if(host_machine.system() == 'windows') \
  .allowed()

_llvm = get_option('llvm')
dep_llvm = null_dep
with_llvm = false
if _llvm.allowed()
  dep_llvm = dependency(
    'llvm',
    method : host_machine.system() == 'windows' ? 'auto' : 'config-tool',
    version : _llvm_version,
    modules : llvm_modules,
    optional_modules : llvm_optional_modules,
    required : (
      with_amd_vk or with_gallium_radeonsi or with_gallium_opencl or with_clc
      or _llvm.enabled()
    ),
    static : not _shared_llvm,
    fallback : ['llvm', 'dep_llvm'],
    include_type : 'system',
  )
  with_llvm = dep_llvm.found()
endif
if with_llvm
  pre_args += '-DMESA_LLVM_VERSION_STRING="@0@"'.format(dep_llvm.version())
  pre_args += '-DLLVM_IS_SHARED=@0@'.format(_shared_llvm.to_int())

  if with_swrast_vk and not draw_with_llvm
    error('Lavapipe requires LLVM draw support.')
  endif

  if host_machine.system() != 'windows'
    # LLVM can be built without rtti, turning off rtti changes the ABI of C++
    # programs, so we need to build all C++ code in mesa without rtti as well to
    # ensure that linking works. Note that Win32 compilers does handle mismatching RTTI
    # without issues, so only apply this for other compilers.
    if dep_llvm.type_name() == 'internal'
      _llvm_rtti = subproject('llvm').get_variable('has_rtti', true)
    else
      # The CMake finder will return 'ON', the llvm-config will return 'YES'
      _llvm_rtti = ['ON', 'YES'].contains(dep_llvm.get_variable(cmake : 'LLVM_ENABLE_RTTI', configtool: 'has-rtti'))
    endif
    if _rtti != _llvm_rtti
      if _llvm_rtti
        error('LLVM was built with RTTI, cannot build Mesa with RTTI disabled. Remove cpp_rtti disable switch or use LLVM built without LLVM_ENABLE_RTTI.')
      else
        error('LLVM was built without RTTI, so Mesa must also disable RTTI. Use an LLVM built with LLVM_ENABLE_RTTI or add cpp_rtti=false.')
      endif
    endif
  endif

  if cc.get_argument_syntax() == 'msvc'
    # Suppress "/DELAYLOAD:ole32.dll/shell32.dll ignored" warnings that LLVM adds
    add_project_link_arguments(
      '/ignore:4199',
      language : ['c', 'cpp'],
    )
  endif
elif with_amd_vk and with_aco_tests
  error('ACO tests require LLVM, but LLVM is disabled.')
elif with_swrast_vk
  error('The following drivers require LLVM: Lavapipe. One of these is enabled, but LLVM is disabled.')
elif with_gallium_opencl
  error('The OpenCL "Clover" state tracker requires LLVM, but LLVM is disabled.')
elif with_clc
  error('The CLC compiler requires LLVM, but LLVM is disabled.')
else
  draw_with_llvm = false
endif
pre_args += '-DLLVM_AVAILABLE=' + (with_llvm ? '1' : '0')
pre_args += '-DDRAW_LLVM_AVAILABLE=' + (with_llvm and draw_with_llvm ? '1' : '0')

with_opencl_spirv = (_opencl != 'disabled' and get_option('opencl-spirv')) or with_clc
if with_opencl_spirv
  chosen_llvm_version_array = dep_llvm.version().split('.')
  chosen_llvm_version_major = chosen_llvm_version_array[0].to_int()
  chosen_llvm_version_minor = chosen_llvm_version_array[1].to_int()

  # Require an SPIRV-LLVM-Translator version compatible with the chosen LLVM
  # one.

  # This first version check is still needed as maybe LLVM 8.0 was picked but
  # we do not want to accept SPIRV-LLVM-Translator 8.0.0.1 as that version
  # does not have the required API and those are only available starting from
  # 8.0.1.3.
  _llvmspirvlib_min_version = '>= 8.0.1.3'
  if with_intel_clc
    _llvmspirvlib_min_version = '>= 13.0.0.0'
  endif

  _llvmspirvlib_version = [
    _llvmspirvlib_min_version,
    '>= @0@.@1@'.format(chosen_llvm_version_major, chosen_llvm_version_minor),
    '< @0@.@1@'.format(chosen_llvm_version_major, chosen_llvm_version_minor + 1) ]

  dep_spirv_tools = dependency('SPIRV-Tools', required : true, version : '>= 2018.0')
  # LLVMSPIRVLib is available at https://github.com/KhronosGroup/SPIRV-LLVM-Translator
  dep_llvmspirvlib = dependency('LLVMSPIRVLib', required : true, version : _llvmspirvlib_version)
else
  dep_spirv_tools = null_dep
  dep_llvmspirvlib = null_dep
endif

dep_clang = null_dep
if with_clc
  llvm_libdir = dep_llvm.get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir')

  dep_clang = cpp.find_library('clang-cpp', dirs : llvm_libdir, required : false)

  if not dep_clang.found() or not _shared_llvm
    clang_modules = [
      'clangBasic', 'clangAST', 'clangCodeGen', 'clangLex',
      'clangDriver', 'clangFrontend', 'clangFrontendTool',
      'clangHandleCXX', 'clangHandleLLVM', 'clangSerialization',
      'clangSema', 'clangParse', 'clangEdit', 'clangAnalysis'
    ]
    if dep_llvm.version().version_compare('>= 15.0')
      clang_modules += 'clangSupport'
    endif
    if dep_llvm.version().version_compare('>= 16.0')
      clang_modules += 'clangASTMatchers'
    endif

    dep_clang = []
    foreach m : clang_modules
      dep_clang += cpp.find_library(m, dirs : llvm_libdir, required : true)
    endforeach
  endif
endif

# Be explicit about only using this lib on Windows, to avoid picking
# up random libs with the generic name 'libversion'
dep_version = null_dep
if host_machine.system() == 'windows'
  dep_version = cpp.find_library('version')
endif

dep_elf = dependency('libelf', required : false)
if not with_platform_windows and not dep_elf.found()
  dep_elf = cc.find_library('elf', required : false)
endif
if dep_elf.found()
  pre_args += '-DUSE_LIBELF'
endif

dep_glvnd = null_dep
if with_glvnd
  dep_glvnd = dependency('libglvnd', version : '>= 1.3.2')
  pre_args += '-DUSE_LIBGLVND=1'
endif

dep_valgrind = dependency('valgrind', required : get_option('valgrind'))
if dep_valgrind.found()
  pre_args += '-DHAVE_VALGRIND'
endif

# AddressSanitizer's leak reports need all the symbols to be present at exit to
# decode well, which runs afoul of our dlopen()/dlclose()ing of the DRI drivers.
# Set a flag so we can skip the dlclose for asan builds.
if ['address', 'address,undefined'].contains(get_option('b_sanitize'))
  asan_c_args = ['-DBUILT_WITH_ASAN=1']
else
  asan_c_args = ['-DBUILT_WITH_ASAN=0']
endif

yacc_is_bison = true
needs_flex_bison = with_any_opengl or with_freedreno_vk or with_intel_tools or with_gallium

if build_machine.system() == 'windows'
  # Prefer the winflexbison versions, they're much easier to install and have
  # better windows support.

  prog_flex = find_program('win_flex', required : false)
  if prog_flex.found()
    # windows compatibility (uses <io.h> instead of <unistd.h> and _isatty,
    # _fileno functions)
    prog_flex = [prog_flex, '--wincompat']
  else
    prog_flex = [find_program('flex', 'lex', required : needs_flex_bison, disabler : true)]
  endif
  # Force flex to use const keyword in prototypes, as relies on __cplusplus or
  # __STDC__ macro to determine whether it's safe to use const keyword
  prog_flex += '-DYY_USE_CONST='

  prog_flex_cpp = prog_flex
  # Convince win_flex to use <inttypes.h> for C++ files
  # Note that we are using a C99 version here rather than C11,
  # because using a C11 version can cause the MSVC CRT headers to define
  # static_assert to _Static_assert, which breaks other parts of the CRT
  prog_flex_cpp += '-D__STDC_VERSION__=199901'

  prog_bison = find_program('win_bison', required : false)
  if not prog_bison.found()
    prog_bison = find_program('bison', 'yacc', required : needs_flex_bison, disabler : true)
  endif
else
  prog_bison = find_program('bison', required : false)

  if not prog_bison.found()
    prog_bison = find_program('byacc', required : needs_flex_bison, disabler : true)
    yacc_is_bison = false
  endif

  # Disable deprecated keyword warnings, since we have to use them for
  # old-bison compat.  See discussion in
  # https://gitlab.freedesktop.org/mesa/mesa/merge_requests/2161
  if find_program('bison', required : false, version : '> 2.3').found()
    prog_bison = [prog_bison, '-Wno-deprecated']
  endif

  prog_flex = find_program('flex', required : needs_flex_bison, disabler : true)
  prog_flex_cpp = prog_flex
endif

dep_selinux = null_dep
if get_option('selinux')
  dep_selinux = dependency('libselinux')
  pre_args += '-DMESA_SELINUX'
endif

_libunwind = get_option('libunwind') \
  .require(not with_platform_android, error_message : 'Android requires the use of the backtrace library, not libunwind')
if host_machine.system() == 'darwin'
  dep_unwind = meson.get_compiler('c').find_library('System', required : _libunwind)
else
  dep_unwind = dependency('libunwind', required : _libunwind)
endif
if dep_unwind.found()
  pre_args += '-DHAVE_LIBUNWIND'
endif

if with_osmesa
  if not with_gallium_softpipe
    error('OSMesa gallium requires gallium softpipe or llvmpipe.')
  endif
  if host_machine.system() == 'windows'
    osmesa_lib_name = 'osmesa'
  else
    osmesa_lib_name = 'OSMesa'
  endif
endif

# TODO: symbol mangling

if with_platform_wayland
#  dep_wl_scanner = dependency('wayland-scanner', native: true)
#  prog_wl_scanner = find_program(dep_wl_scanner.get_variable(pkgconfig : 'wayland_scanner'))
  prog_wl_scanner = find_program('wayland-scanner')
#  if dep_wl_scanner.version().version_compare('>= 1.15')
    wl_scanner_arg = 'private-code'
#  else
#    wl_scanner_arg = 'code'
#  endif
  dep_wl_protocols = [] #dependency('wayland-protocols', version : '>= 1.30')
  dep_wayland_client = [] #dependency('wayland-client', version : '>=1.18')
  dep_wayland_server = [] #dependency('wayland-server', version : '>=1.18')
  if with_egl
    dep_wayland_egl = [] #dependency('wayland-egl-backend', version : '>= 3')
    dep_wayland_egl_headers = [] #dep_wayland_egl.partial_dependency(compile_args : true)
  endif
  pre_args += '-DWL_HIDE_DEPRECATED'
endif

dep_x11 = null_dep
dep_xext = null_dep
dep_xfixes = null_dep
dep_x11_xcb = null_dep
dep_xcb = null_dep
dep_xcb_keysyms = null_dep
dep_xcb_glx = null_dep
dep_xcb_dri2 = null_dep
dep_xcb_dri3 = null_dep
dep_dri2proto = null_dep
dep_glproto = null_dep
dep_xxf86vm = null_dep
dep_xcb_dri3 = null_dep
dep_xcb_present = null_dep
dep_xcb_sync = null_dep
dep_xcb_xfixes = null_dep
dep_xshmfence = null_dep
dep_xcb_xrandr = null_dep
dep_xcb_shm = null_dep
dep_xlib_xrandr = null_dep
dep_openmp = null_dep

# Even if we find OpenMP, Gitlab CI fails to link with gcc/i386 and clang/anyarch.
if host_machine.cpu_family() == 'x86_64' and cc.get_id() == 'gcc'
  dep_openmp = dependency('openmp', required : false)
  if dep_openmp.found()
    pre_args += ['-DHAVE_OPENMP']
  endif
endif

with_dri3_modifiers = false
with_xcb_keysyms = false
if with_platform_x11
  if with_glx == 'xlib'
    dep_x11 = dependency('x11')
    dep_xext = dependency('xext')
    dep_xcb = dependency('xcb')
    dep_xcb_xrandr = dependency('xcb-randr')
  elif with_glx == 'dri'
    dep_x11 = dependency('x11')
    dep_xext = dependency('xext')
    dep_xfixes = dependency('xfixes', version : '>= 2.0')
    dep_xcb_glx = dependency('xcb-glx', version : '>= 1.8.1')
    dep_xcb_shm = dependency('xcb-shm')
  elif with_gallium_rusticl
    # needed for GL sharing extension
    dep_x11 = dependency('x11')
  endif
  if (with_any_vk or with_glx == 'dri' or with_egl or
       (with_gallium_vdpau or with_gallium_va or
        with_gallium_omx != 'disabled'))
    dep_xcb = dependency('xcb')
    dep_xcb_keysyms = dependency('xcb-keysyms', required : false)
    with_xcb_keysyms = dep_xcb_keysyms.found()
    if with_xcb_keysyms
      pre_args += '-DXCB_KEYSYMS_AVAILABLE'
    endif
    dep_x11_xcb = dependency('x11-xcb')
    if with_dri_platform == 'drm' and not dep_libdrm.found()
      error('libdrm required for gallium video statetrackers when using x11')
    endif
  endif
  if with_any_vk or with_egl or (with_glx == 'dri' and with_dri_platform == 'drm')
    dep_xcb_dri2 = dependency('xcb-dri2', version : '>= 1.8')

    if with_dri3
      dep_xcb_dri3 = dependency('xcb-dri3')
      dep_xcb_present = dependency('xcb-present')
      # until xcb-dri3 has been around long enough to make a hard-dependency:
      if (dep_xcb_dri3.version().version_compare('>= 1.13') and
          dep_xcb_present.version().version_compare('>= 1.13'))
        with_dri3_modifiers = true
      endif
      dep_xcb_shm = dependency('xcb-shm')
      dep_xcb_sync = dependency('xcb-sync')
      dep_xshmfence = dependency('xshmfence', version : '>= 1.1')
    endif
  endif
  if with_glx == 'dri' or with_glx == 'xlib'
    dep_glproto = dependency('glproto', version : '>= 1.4.14')
  endif
  if with_glx == 'dri'
    if with_dri_platform == 'drm'
      dep_dri2proto = dependency('dri2proto', version : '>= 2.8')
      if with_glx_direct
        dep_xxf86vm = dependency('xxf86vm')
      endif
    endif
  endif
  if (with_egl or
      with_dri3 or (
      with_gallium_vdpau or with_gallium_xa or
      with_gallium_omx != 'disabled'))
    dep_xcb_xfixes = dependency('xcb-xfixes')
  endif
  if with_xlib_lease or with_any_vk
    dep_xcb_xrandr = dependency('xcb-randr')
  endif
  if with_xlib_lease
    dep_xlib_xrandr = dependency('xrandr', version : '>= 1.3')
  endif
endif

if with_dri
  pre_args += '-DHAVE_DRI'
endif
if with_dri2
  pre_args += '-DHAVE_DRI2'
endif
if with_dri3
  pre_args += '-DHAVE_DRI3'
endif
if with_dri3_modifiers
  pre_args += '-DHAVE_DRI3_MODIFIERS'
endif
if with_gallium_drisw_kms
  pre_args += '-DHAVE_DRISW_KMS'
endif

if get_option('gallium-extra-hud')
  pre_args += '-DHAVE_GALLIUM_EXTRA_HUD=1'
endif

dep_lmsensors = cc.find_library('sensors', required : get_option('lmsensors'))
if dep_lmsensors.found()
  pre_args += '-DHAVE_LIBSENSORS=1'
endif

_shader_replacement = get_option('custom-shader-replacement')
if _shader_replacement == ''
else
  pre_args += '-DCUSTOM_SHADER_REPLACEMENT'
endif

with_perfetto = get_option('perfetto')
with_datasources = get_option('datasources')
with_any_datasource = with_datasources.length() != 0
if with_perfetto
  dep_perfetto = dependency('perfetto', fallback: ['perfetto', 'dep_perfetto'])
  pre_args += '-DHAVE_PERFETTO'
endif

with_gpuvis = get_option('gpuvis')
if with_gpuvis
  pre_args += '-DHAVE_GPUVIS'
endif

gl_priv_reqs = []

if with_glx == 'xlib'
  gl_priv_reqs += ['x11', 'xext', 'xcb']
elif with_glx == 'dri'
  gl_priv_reqs += [
    'x11', 'xext', 'xfixes', 'x11-xcb', 'xcb',
    'xcb-glx >= 1.8.1']
  if with_dri_platform == 'drm'
    gl_priv_reqs += 'xcb-dri2 >= 1.8'
    if with_glx_direct
      gl_priv_reqs += 'xxf86vm'
    endif
  endif
endif
if dep_libdrm.found()
  gl_priv_reqs += 'libdrm >= 2.4.75'
endif

gl_priv_libs = []
if dep_thread.found()
  gl_priv_libs += ['-lpthread', '-pthread']
endif
if dep_m.found()
  gl_priv_libs += '-lm'
endif
if dep_dl.found()
  gl_priv_libs += '-ldl'
endif

# FIXME: autotools lists this as incomplete
gbm_priv_libs = []
if dep_dl.found()
  gbm_priv_libs += '-ldl'
endif

pkg = import('pkgconfig')

if host_machine.system() == 'windows'
  prog_dumpbin = find_program('dumpbin', required : false)
  with_symbols_check = prog_dumpbin.found() and with_tests
  if with_symbols_check
    symbols_check_args = ['--dumpbin', prog_dumpbin.full_path()]
  endif
else
  prog_nm = find_program('nm')
  with_symbols_check = with_tests
  symbols_check_args = ['--nm', prog_nm.full_path()]
endif

# This quirk needs to be applied to sources with functions defined in assembly
# as GCC LTO drops them. See: https://bugs.freedesktop.org/show_bug.cgi?id=109391
gcc_lto_quirk = (cc.get_id() == 'gcc') ? ['-fno-lto'] : []

devenv = environment()

dir_compiler_nir = join_paths(meson.current_source_dir(), 'src/compiler/nir/')
dir_source_root = meson.project_source_root()


subdir('include')
subdir('bin')
subdir('src')

meson.add_devenv(devenv)

summary(
  {
    'prefix': get_option('prefix'),
    'libdir': get_option('libdir'),
    'includedir': get_option('includedir'),
  },
  section: 'Directories'
)

summary(
  {
    'c_cpp_args': c_cpp_args,
  },
  section: 'Common C and C++ arguments'
)

summary(
  {
    'OpenGL': with_opengl,
    'ES1': with_gles1,
    'ES2': with_gles2,
    'Shared glapi': with_shared_glapi,
    'GLVND': with_glvnd,
  },
  section: 'OpenGL', bool_yn: true
)

summary(
  {
    'Platform': with_dri_platform,
    'Driver dir': dri_drivers_path,
  },
  section: 'DRI', bool_yn: true, list_sep: ' '
)

summary(
  {
    'Enabled': with_glx != 'disabled',
    'Provider': with_glx == 'disabled' ? 'None' : with_glx
  },
  section: 'GLX', bool_yn: true, list_sep: ' '
)

egl_summary = {'Enabled': with_egl}
if with_egl
  egl_drivers = []
  if with_dri
    egl_drivers += 'builtin:egl_dri2'
  endif
  if with_dri3
    egl_drivers += 'builtin:egl_dri3'
  endif
  if with_platform_windows
    egl_drivers += 'builtin:wgl'
  endif
  egl_summary += {'Drivers': egl_drivers}
  egl_summary += {'Platforms': _platforms}
endif
summary(egl_summary, section: 'EGL', bool_yn: true, list_sep: ' ')

gbm_summary = {'Enabled': with_gbm}
if with_gbm
  gbm_summary += {'Backends path': gbm_backends_path}
endif
summary(gbm_summary, section: 'GBM', bool_yn: true, list_sep: ' ')

vulkan_summary = {'Drivers': _vulkan_drivers.length() != 0 ? _vulkan_drivers : false }
if with_any_vk
  vulkan_summary += {'Platforms': _platforms}
  vulkan_summary += {'ICD dir': with_vulkan_icd_dir}
  if with_any_vulkan_layers
    vulkan_summary += {'Layers': get_option('vulkan-layers')}
  endif
  vulkan_summary += {'Intel Ray tracing': with_intel_vk_rt}
endif
summary(vulkan_summary, section: 'Vulkan', bool_yn: true, list_sep: ' ')

video_summary = {'Codecs': _codecs.length() != 0 ? _codecs : false}
video_apis = []
if with_gallium_vdpau
  video_apis += 'vdpau'
endif
if with_gallium_va
  video_apis += 'va'
endif
if with_any_vk
  video_apis += 'vulkan'
endif
if with_gallium_xa
  video_apis += 'xa'
endif
if with_gallium_omx != 'disabled'
  video_apis += 'omx'
endif
video_summary += {'APIs': video_apis.length() != 0 ? video_apis : false}
summary(video_summary, section: 'Video', bool_yn: true, list_sep: ' ')

llvm_summary = {'Enabled': with_llvm}
if with_llvm
  llvm_summary += {'Version': dep_llvm.version()}
endif
summary(llvm_summary, section: 'LLVM', bool_yn: true, list_sep: ' ')

gallium_summary = {'Enabled': with_gallium}
if with_gallium
  gallium_summary += {'Drivers': gallium_drivers}
  gallium_summary += {'Platforms': _platforms}

  gallium_frontends = ['mesa']
  if with_gallium_xa
    gallium_frontends += 'xa'
  endif
  if with_gallium_vdpau
    gallium_frontends += 'vdpau'
  endif
  if with_gallium_omx != 'disabled'
    gallium_frontends += 'omx' + with_gallium_omx
  endif
  if with_gallium_va
    gallium_frontends += 'va'
  endif
  if with_gallium_st_nine
    gallium_frontends += 'nine'
  endif
  if with_gallium_opencl
    gallium_frontends += 'clover'
  endif
  if with_gallium_rusticl
    gallium_frontends += 'rusticl'
  endif
  gallium_summary += {'Frontends': gallium_frontends}
  gallium_summary += {'Off-screen rendering (OSMesa)': with_osmesa ? 'lib' + osmesa_lib_name : false}
  gallium_summary += {'HUD lm-sensors': dep_lmsensors.found()}
endif
summary(gallium_summary, section: 'Gallium', bool_yn: true, list_sep: ' ')

perfetto_summary = {'Enabled': with_perfetto}
if with_perfetto and with_any_datasource
  perfetto_summary += {'Data source': with_datasources}
endif
summary(perfetto_summary, section: 'Perfetto', bool_yn: true, list_sep: ' ')
