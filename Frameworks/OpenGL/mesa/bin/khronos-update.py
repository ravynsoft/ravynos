#!/usr/bin/env python3

import argparse
import base64
import pathlib
import requests
import subprocess
import typing


def error(msg: str) -> None:
    print('\033[31m' + msg + '\033[0m')


class Source:
    def __init__(self, filename: str, url: typing.Optional[str]):
        self.file = pathlib.Path(filename)
        self.url = url

    def sync(self) -> None:
        if self.url is None:
            return

        print('Syncing {}...'.format(self.file), end=' ', flush=True)
        req = requests.get(self.url)

        if not req.ok:
            error('Failed to retrieve file: {} {}'.format(req.status_code, req.reason))
            return

        # Gitiles returns base64-encoded strings.
        # Google has been resisting for years to the idea of allowing plain text: https://github.com/google/gitiles/issues/7
        if 'format=TEXT' in self.url:
            content = base64.b64decode(req.content)
        else:
            content = req.content

        with open(self.file, 'wb') as f:
            f.write(content)

        print('Done')


# a URL of `None` means there is no upstream, because *we* are the upstream
SOURCES = [
    {
        'api': 'khr',
        'inc_folder': 'KHR',
        'sources': [
            Source('include/KHR/khrplatform.h',    'https://github.com/KhronosGroup/EGL-Registry/raw/main/api/KHR/khrplatform.h'),
        ],
    },

    {
        'api': 'egl',
        'inc_folder': 'EGL',
        'sources': [
            Source('src/egl/generate/egl.xml',     'https://github.com/KhronosGroup/EGL-Registry/raw/main/api/egl.xml'),
            Source('include/EGL/egl.h',            'https://github.com/KhronosGroup/EGL-Registry/raw/main/api/EGL/egl.h'),
            Source('include/EGL/eglplatform.h',    'https://github.com/KhronosGroup/EGL-Registry/raw/main/api/EGL/eglplatform.h'),
            Source('include/EGL/eglext.h',         'https://github.com/KhronosGroup/EGL-Registry/raw/main/api/EGL/eglext.h'),
            Source('include/EGL/eglext_angle.h',   'https://chromium.googlesource.com/angle/angle/+/refs/heads/main/include/EGL/eglext_angle.h?format=TEXT'),
            Source('include/EGL/eglmesaext.h',     None),
        ],
    },

    {
        'api': 'gl',
        'inc_folder': 'GL',
        'sources': [
            Source('src/mapi/glapi/registry/gl.xml', 'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/xml/gl.xml'),
            Source('include/GL/glcorearb.h',         'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GL/glcorearb.h'),
            Source('include/GL/glext.h',             'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GL/glext.h'),
            Source('include/GL/glxext.h',            'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GL/glxext.h'),
            Source('include/GL/wglext.h',            'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GL/wglext.h'),
            Source('include/GL/gl.h',                None),  # FIXME: I don't know what the canonical source is
            Source('include/GL/glx.h',               None),  # FIXME: I don't know what the canonical source is
            Source('include/GL/internal/',           None),
            Source('include/GL/mesa_glinterop.h',    None),
            Source('include/GL/osmesa.h',            None),
        ],
    },

    {
        'api': 'gles1',
        'inc_folder': 'GLES',
        'sources': [
            Source('include/GLES/gl.h',         'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES/gl.h'),
            Source('include/GLES/glplatform.h', 'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES/glplatform.h'),
            Source('include/GLES/glext.h',      'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES/glext.h'),
            Source('include/GLES/egl.h',        'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES/egl.h'),
        ],
    },

    {
        'api': 'gles2',
        'inc_folder': 'GLES2',
        'sources': [
            Source('include/GLES2/gl2.h',         'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES2/gl2.h'),
            Source('include/GLES2/gl2platform.h', 'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES2/gl2platform.h'),
            Source('include/GLES2/gl2ext.h',      'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES2/gl2ext.h'),
        ],
    },

    {
        'api': 'gles3',
        'inc_folder': 'GLES3',
        'sources': [
            Source('include/GLES3/gl3.h',         'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES3/gl3.h'),
            Source('include/GLES3/gl31.h',        'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES3/gl31.h'),
            Source('include/GLES3/gl32.h',        'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES3/gl32.h'),
            Source('include/GLES3/gl3platform.h', 'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GLES3/gl3platform.h'),
            Source('include/GLES3/gl3ext.h',      None),  # FIXME: I don't know what the canonical source is
        ],
    },

    {
        'api': 'opencl',
        'inc_folder': 'CL',
        'sources': [
            Source('include/CL/opencl.h',                        'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/opencl.h'),
            Source('include/CL/cl.h',                            'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl.h'),
            Source('include/CL/cl_platform.h',                   'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_platform.h'),
            Source('include/CL/cl_gl.h',                         'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_gl.h'),
            Source('include/CL/cl_gl_ext.h',                     'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_gl_ext.h'),
            Source('include/CL/cl_ext.h',                        'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_ext.h'),
            Source('include/CL/cl_version.h',                    'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_version.h'),
            Source('include/CL/cl_icd.h',                        'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_icd.h'),
            Source('include/CL/cl_egl.h',                        'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_egl.h'),
            Source('include/CL/cl_d3d10.h',                      'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_d3d10.h'),
            Source('include/CL/cl_d3d11.h',                      'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_d3d11.h'),
            Source('include/CL/cl_dx9_media_sharing.h',          'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_dx9_media_sharing.h'),
            Source('include/CL/cl_dx9_media_sharing_intel.h',    'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_dx9_media_sharing_intel.h'),
            Source('include/CL/cl_ext_intel.h',                  'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_ext_intel.h'),
            Source('include/CL/cl_va_api_media_sharing_intel.h', 'https://github.com/KhronosGroup/OpenCL-Headers/raw/main/CL/cl_va_api_media_sharing_intel.h'),

            Source('include/CL/cl.hpp',                          'https://github.com/KhronosGroup/OpenCL-CLHPP/raw/5f3cc41df821a3e5988490232082a3e3b82c0283/include/CL/cl.hpp'),
            Source('include/CL/cl2.hpp',                         'https://github.com/KhronosGroup/OpenCL-CLHPP/raw/main/include/CL/cl2.hpp'),
            Source('include/CL/opencl.hpp',                      'https://github.com/KhronosGroup/OpenCL-CLHPP/raw/main/include/CL/opencl.hpp'),
        ],
    },

    {
        'api': 'spirv',
        'sources': [
            Source('src/compiler/spirv/spirv.h',                    'https://github.com/KhronosGroup/SPIRV-Headers/raw/main/include/spirv/unified1/spirv.h'),
            Source('src/compiler/spirv/spirv.core.grammar.json',    'https://github.com/KhronosGroup/SPIRV-Headers/raw/main/include/spirv/unified1/spirv.core.grammar.json'),
            Source('src/compiler/spirv/OpenCL.std.h',               'https://github.com/KhronosGroup/SPIRV-Headers/raw/main/include/spirv/unified1/OpenCL.std.h'),
            Source('src/compiler/spirv/GLSL.std.450.h',             'https://github.com/KhronosGroup/SPIRV-Headers/raw/main/include/spirv/unified1/GLSL.std.450.h'),
            Source('src/compiler/spirv/GLSL.ext.AMD.h',             'https://github.com/KhronosGroup/glslang/raw/main/SPIRV/GLSL.ext.AMD.h'),  # FIXME: is this the canonical source?
        ],
    },

    {
        'api': 'vulkan',
        'inc_folder': 'vulkan',
        'sources': [
            Source('src/vulkan/registry/vk.xml',                'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/registry/vk.xml'),
            Source('include/vulkan/vulkan.h',                   'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan.h'),
            Source('include/vulkan/vulkan_core.h',              'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_core.h'),
            Source('include/vulkan/vulkan_beta.h',              'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_beta.h'),
            Source('include/vulkan/vk_icd.h',                   'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vk_icd.h'),
            Source('include/vulkan/vk_layer.h',                 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vk_layer.h'),
            Source('include/vulkan/vk_platform.h',              'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vk_platform.h'),
            Source('include/vulkan/vulkan_android.h',           'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_android.h'),
            Source('include/vulkan/vulkan_directfb.h',          'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_directfb.h'),
            Source('include/vulkan/vulkan_fuchsia.h',           'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_fuchsia.h'),
            Source('include/vulkan/vulkan_ggp.h',               'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_ggp.h'),
            Source('include/vulkan/vulkan_ios.h',               'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_ios.h'),
            Source('include/vulkan/vulkan_macos.h',             'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_macos.h'),
            Source('include/vulkan/vulkan_metal.h',             'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_metal.h'),
            Source('include/vulkan/vulkan_screen.h',            'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_screen.h'),
            Source('include/vulkan/vulkan_vi.h',                'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_vi.h'),
            Source('include/vulkan/vulkan_wayland.h',           'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_wayland.h'),
            Source('include/vulkan/vulkan_win32.h',             'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_win32.h'),
            Source('include/vulkan/vulkan_xcb.h',               'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_xcb.h'),
            Source('include/vulkan/vulkan_xlib.h',              'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_xlib.h'),
            Source('include/vulkan/vulkan_xlib_xrandr.h',       'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vulkan/vulkan_xlib_xrandr.h'),
            Source('include/vulkan/vk_android_native_buffer.h', 'https://android.googlesource.com/platform/frameworks/native/+/master/vulkan/include/vulkan/vk_android_native_buffer.h?format=TEXT'),
            Source('include/vk_video/vulkan_video_codec_h264std.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h264std.h'),
            Source('include/vk_video/vulkan_video_codec_h264std_decode.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h264std_decode.h'),
            Source('include/vk_video/vulkan_video_codec_h264std_encode.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h264std_encode.h'),
            Source('include/vk_video/vulkan_video_codec_h265std.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h265std.h'),
            Source('include/vk_video/vulkan_video_codec_h265std_decode.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h265std_decode.h'),
            Source('include/vk_video/vulkan_video_codec_h265std_encode.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codec_h265std_encode.h'),
            Source('include/vk_video/vulkan_video_codecs_common.h', 'https://github.com/KhronosGroup/Vulkan-Headers/raw/main/include/vk_video/vulkan_video_codecs_common.h'),
            Source('include/vulkan/.editorconfig',              None),
        ],
    },
]


if __name__ == '__main__':
    git_toplevel = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                           stderr=subprocess.DEVNULL).decode("ascii").strip()
    if not pathlib.Path(git_toplevel).resolve() == pathlib.Path('.').resolve():
        error('Please run this script from the root folder ({})'.format(git_toplevel))
        exit(1)

    parser = argparse.ArgumentParser()
    parser.add_argument('apis', nargs='*',
                        # the `[[]]` here is a workaround for python bug 9625
                        # where having `choices` breaks `nargs='*'`:
                        # https://bugs.python.org/issue9625
                        choices=[group['api'] for group in SOURCES] + [[]],
                        help='Only update the APIs specified.')
    args = parser.parse_args()

    # These APIs all depend on the KHR header
    depend_on_khr = set(['egl', 'gl', 'gles', 'gles2', 'gles3'])
    if args.apis and 'khr' not in args.apis and depend_on_khr.intersection(set(args.apis)):
        args.apis = ['khr'] + args.apis

    for group in SOURCES:
        if args.apis and group['api'] not in args.apis:
            continue

        for source in group['sources']:
            source.sync()

        # Make sure all the API files are handled by this script
        if 'inc_folder' in group:
            for file in pathlib.Path('include/' + group['inc_folder']).iterdir():
                if file not in [source.file for source in group['sources']]:
                    error('{} is unknown, please add it to SOURCES'.format(file))
