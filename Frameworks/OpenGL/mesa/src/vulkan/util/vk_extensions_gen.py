COPYRIGHT = """\
/*
 * Copyright 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
"""

import argparse

from mako.template import Template

# Mesa-local imports must be declared in meson variable
# '{file_without_suffix}_depend_files'.
from vk_extensions import get_all_exts_from_xml, init_exts_from_xml

_TEMPLATE_H = Template(COPYRIGHT + """

#ifndef ${driver.upper()}_EXTENSIONS_H
#define ${driver.upper()}_EXTENSIONS_H

#include <stdbool.h>

%if driver == 'vk':

<%def name="extension_table(type, extensions)">
#define VK_${type.upper()}_EXTENSION_COUNT ${len(extensions)}

extern const VkExtensionProperties vk_${type}_extensions[];

struct vk_${type}_extension_table {
   union {
      bool extensions[VK_${type.upper()}_EXTENSION_COUNT];
      struct {
%for ext in extensions:
         bool ${ext.name[3:]};
%endfor
      };

      /* Workaround for "error: too many initializers for vk_${type}_extension_table" */
      struct {
%for ext in extensions:
         bool ${ext.name[3:]};
%endfor
      } table;
   };
};
</%def>

${extension_table('instance', instance_extensions)}
${extension_table('device', device_extensions)}

%else:
#include "vk_extensions.h"
%endif

struct ${driver}_physical_device;

%if driver == 'vk':
#ifdef ANDROID_STRICT
extern const struct vk_instance_extension_table vk_android_allowed_instance_extensions;
extern const struct vk_device_extension_table vk_android_allowed_device_extensions;
#endif
%else:
extern const struct vk_instance_extension_table ${driver}_instance_extensions_supported;

void
${driver}_physical_device_get_supported_extensions(const struct ${driver}_physical_device *device,
                                             struct vk_device_extension_table *extensions);
%endif

#endif /* ${driver.upper()}_EXTENSIONS_H */
""")

_TEMPLATE_C = Template(COPYRIGHT + """
#include "vulkan/vulkan_core.h"
%if driver != 'vk':
#include "${driver}_private.h"
%endif

#include "${driver}_extensions.h"

%if driver == 'vk':
const VkExtensionProperties ${driver}_instance_extensions[${driver.upper()}_INSTANCE_EXTENSION_COUNT] = {
%for ext in instance_extensions:
   {"${ext.name}", ${ext.ext_version}},
%endfor
};

const VkExtensionProperties ${driver}_device_extensions[${driver.upper()}_DEVICE_EXTENSION_COUNT] = {
%for ext in device_extensions:
   {"${ext.name}", ${ext.ext_version}},
%endfor
};

#ifdef ANDROID_STRICT
const struct vk_instance_extension_table vk_android_allowed_instance_extensions = {
%for ext in instance_extensions:
   .${ext.name[3:]} = ${ext.c_android_condition()},
%endfor
};

const struct vk_device_extension_table vk_android_allowed_device_extensions = {
%for ext in device_extensions:
   .${ext.name[3:]} = ${ext.c_android_condition()},
%endfor
};
#endif
%endif

%if driver != 'vk':
#include "vk_util.h"

/* Convert the VK_USE_PLATFORM_* defines to booleans */
%for platform_define in platform_defines:
#ifdef ${platform_define}
#   undef ${platform_define}
#   define ${platform_define} true
#else
#   define ${platform_define} false
#endif
%endfor

/* And ANDROID too */
#ifdef ANDROID
#   undef ANDROID
#   define ANDROID true
#else
#   define ANDROID false
#   define ANDROID_API_LEVEL 0
#endif

#define ${driver.upper()}_HAS_SURFACE (VK_USE_PLATFORM_WIN32_KHR || \\
                                       VK_USE_PLATFORM_WAYLAND_KHR || \\
                                       VK_USE_PLATFORM_XCB_KHR || \\
                                       VK_USE_PLATFORM_XLIB_KHR || \\
                                       VK_USE_PLATFORM_DISPLAY_KHR)

static const uint32_t MAX_API_VERSION = ${MAX_API_VERSION.c_vk_version()};

VKAPI_ATTR VkResult VKAPI_CALL ${driver}_EnumerateInstanceVersion(
    uint32_t*                                   pApiVersion)
{
    *pApiVersion = MAX_API_VERSION;
    return VK_SUCCESS;
}

const struct vk_instance_extension_table ${driver}_instance_extensions_supported = {
%for ext in instance_extensions:
   .${ext.name[3:]} = ${ext.enable},
%endfor
};

uint32_t
${driver}_physical_device_api_version(struct ${driver}_physical_device *device)
{
    uint32_t version = 0;

    uint32_t override = vk_get_version_override();
    if (override)
        return MIN2(override, MAX_API_VERSION);

%for version in API_VERSIONS:
    if (!(${version.enable}))
        return version;
    version = ${version.version.c_vk_version()};

%endfor
    return version;
}

void
${driver}_physical_device_get_supported_extensions(const struct ${driver}_physical_device *device,
                                                   struct vk_device_extension_table *extensions)
{
   *extensions = (struct vk_device_extension_table) {
%for ext in device_extensions:
      .${ext.name[3:]} = ${ext.enable},
%endfor
   };
}
%endif
""")

def gen_extensions(driver, xml_files, api_versions, max_api_version,
                   extensions, out_c, out_h):
    platform_defines = []
    for filename in xml_files:
        init_exts_from_xml(filename, extensions, platform_defines)

    for ext in extensions:
        assert ext.type in {'instance', 'device'}

    template_env = {
        'driver': driver,
        'API_VERSIONS': api_versions,
        'MAX_API_VERSION': max_api_version,
        'instance_extensions': [e for e in extensions if e.type == 'instance'],
        'device_extensions': [e for e in extensions if e.type == 'device'],
        'platform_defines': platform_defines,
    }

    if out_h:
        with open(out_h, 'w', encoding='utf-8') as f:
            f.write(_TEMPLATE_H.render(**template_env))

    if out_c:
        with open(out_c, 'w', encoding='utf-8') as f:
            f.write(_TEMPLATE_C.render(**template_env))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out-c', help='Output C file.')
    parser.add_argument('--out-h', help='Output H file.')
    parser.add_argument('--xml',
                        help='Vulkan API XML file.',
                        required=True,
                        action='append',
                        dest='xml_files')
    args = parser.parse_args()

    extensions = []
    for filename in args.xml_files:
        extensions += get_all_exts_from_xml(filename)

    gen_extensions('vk', args.xml_files, None, None,
                   extensions, args.out_c, args.out_h)

if __name__ == '__main__':
    main()
