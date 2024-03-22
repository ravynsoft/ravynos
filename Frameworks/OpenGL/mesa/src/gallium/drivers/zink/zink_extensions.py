# Copyright © 2020 Hoe Hao Cheng
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# 

import re
from xml.etree import ElementTree
from typing import List,Tuple

class Version:
    device_version = (1,0,0)
    struct_version = (1,0)

    def __init__(self, version, struct=()):
        self.device_version = version

        if not struct:
            self.struct_version = (version[0], version[1])
        else:
            self.struct_version = struct

    # e.g. "VK_MAKE_VERSION(1,2,0)"
    def version(self):
        return ("VK_MAKE_VERSION("
               + str(self.device_version[0])
               + ","
               + str(self.device_version[1])
               + ","
               + str(self.device_version[2])
               + ")")

    # e.g. "10"
    def struct(self):
        return (str(self.struct_version[0])+str(self.struct_version[1]))

    # the sType of the extension's struct
    # e.g. VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT
    # for VK_EXT_transform_feedback and struct="FEATURES"
    def stype(self, struct: str):
        return ("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_"
                + str(self.struct_version[0]) + "_" + str(self.struct_version[1])
                + '_' + struct)

class Extension:
    name           = None
    alias          = None
    is_required    = False
    is_nonstandard = False
    enable_conds   = None
    core_since     = None

    # these are specific to zink_device_info.py:
    has_properties      = False
    has_features        = False
    guard               = False
    features_promoted   = False
    properties_promoted = False
    

    # these are specific to zink_instance.py:
    platform_guard = None

    def __init__(self, name, alias="", required=False, nonstandard=False,
                 properties=False, features=False, conditions=None, guard=False):
        self.name = name
        self.alias = alias
        self.is_required = required
        self.is_nonstandard = nonstandard
        self.has_properties = properties
        self.has_features = features
        self.enable_conds = conditions
        self.guard = guard

        if alias == "" and (properties == True or features == True):
            raise RuntimeError("alias must be available when properties and/or features are used")

    # e.g.: "VK_EXT_robustness2" -> "robustness2"
    def pure_name(self):
        return '_'.join(self.name.split('_')[2:])
    
    # e.g.: "VK_EXT_robustness2" -> "EXT_robustness2"
    def name_with_vendor(self):
        return self.name[3:]
    
    # e.g.: "VK_EXT_robustness2" -> "Robustness2"
    def name_in_camel_case(self):
        return "".join([x.title() for x in self.name.split('_')[2:]])

    # e.g.: "VK_EXT_robustness2" -> "VK_EXT_ROBUSTNESS_2"
    def name_in_snake_uppercase(self):
        def replace(original):
            # we do not split the types into two, e.g. INT_32
            match_types = re.match(".*(int|float)(8|16|32|64)$", original)

            # do not match win32
            match_os = re.match(".*win32$", original)

            # try to match extensions with alphanumeric names, like robustness2
            match_alphanumeric = re.match(r"([a-z]+)(\d+)", original)

            if match_types is not None or match_os is not None:
                return original.upper()

            if match_alphanumeric is not None:
                return (match_alphanumeric[1].upper()
                        + '_' 
                        + match_alphanumeric[2])

            return original.upper()

        replaced = list(map(replace, self.name.split('_')))
        return '_'.join(replaced)

    # e.g.: "VK_EXT_robustness2" -> "ROBUSTNESS_2"
    def pure_name_in_snake_uppercase(self):
        return '_'.join(self.name_in_snake_uppercase().split('_')[2:])

    # e.g.: "VK_EXT_robustness2" -> "VK_EXT_ROBUSTNESS_2_EXTENSION_NAME"
    def extension_name(self):
        return self.name_in_snake_uppercase() + "_EXTENSION_NAME"

    # generate a C string literal for the extension
    def extension_name_literal(self):
        return '"' + self.name + '"'

    # get the field in zink_device_info that refers to the extension's
    # feature/properties struct
    # e.g. rb2_<suffix> for VK_EXT_robustness2
    def field(self, suffix: str):
        return self.alias + '_' + suffix

    def physical_device_struct(self, struct: str):
        if self.name_in_camel_case().endswith(struct):
            struct = ""

        return ("VkPhysicalDevice"
                + self.name_in_camel_case()
                + struct
                + self.vendor())

    # the sType of the extension's struct
    # e.g. VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT
    # for VK_EXT_transform_feedback and struct="FEATURES"
    def stype(self, struct: str):
        return ("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_" 
                + self.pure_name_in_snake_uppercase()
                + '_' + struct + '_' 
                + self.vendor())

    # e.g. EXT in VK_EXT_robustness2
    def vendor(self):
        return self.name.split('_')[1]

# Type aliases
Layer = Extension

class ExtensionRegistryEntry:
    # type of extension - right now it's either "instance" or "device"
    ext_type          = ""
    # the version in which the extension is promoted to core VK
    promoted_in       = None
    # functions added by the extension are referred to as "commands" in the registry
    device_commands   = None
    pdevice_commands  = None
    instance_commands = None
    constants         = None
    features_struct   = None
    features_fields   = None
    features_promoted = False
    properties_struct = None
    properties_fields = None
    properties_promoted = False
    # some instance extensions are locked behind certain platforms
    platform_guard    = ""

class ExtensionRegistry:
    # key = extension name, value = registry entry
    registry = dict()

    def __init__(self, vkxml_path: str):
        vkxml = ElementTree.parse(vkxml_path)

        commands_type = dict()
        command_aliases = dict()
        platform_guards = dict()
        struct_aliases = dict()

        for cmd in vkxml.findall("commands/command"):
            name = cmd.find("./proto/name")

            if name is not None and name.text:
                commands_type[name.text] = cmd.find("./param/type").text
            elif cmd.get("name") is not None:
                command_aliases[cmd.get("name")] = cmd.get("alias")

        for typ in vkxml.findall("types/type"):
            if typ.get("category") != "struct":
                continue

            name = typ.get("name")
            alias = typ.get("alias")

            if name and alias:
                struct_aliases[name] = alias

        for (cmd, alias) in command_aliases.items():
            commands_type[cmd] = commands_type[alias]

        for platform in vkxml.findall("platforms/platform"):
            name = platform.get("name")
            guard = platform.get("protect")
            platform_guards[name] = guard

        for ext in vkxml.findall("extensions/extension"):
            # Reserved extensions are marked with `supported="disabled"`
            if ext.get("supported") == "disabled":
                continue

            name = ext.attrib["name"]

            entry = ExtensionRegistryEntry()
            entry.ext_type = ext.attrib["type"]
            entry.promoted_in = self.parse_promotedto(ext.get("promotedto"))

            entry.device_commands = []
            entry.pdevice_commands = []
            entry.instance_commands = []
            entry.features_fields = []
            entry.properties_fields = []

            for cmd in ext.findall("require/command"):
                cmd_name = cmd.get("name")
                if cmd_name:
                    if commands_type[cmd_name] in ("VkDevice", "VkCommandBuffer", "VkQueue"):
                        entry.device_commands.append(cmd_name)
                    elif commands_type[cmd_name] in ("VkPhysicalDevice"):
                        entry.pdevice_commands.append(cmd_name)
                    else:
                        entry.instance_commands.append(cmd_name)

            entry.constants = []
            for enum in ext.findall("require/enum"):
                enum_name = enum.get("name")
                enum_extends = enum.get("extends")
                # we are only interested in VK_*_EXTENSION_NAME, which does not
                # have an "extends" attribute
                if not enum_extends:
                    entry.constants.append(enum_name)

            for ty in ext.findall("require/type"):
                ty_name = ty.get("name")
                if (self.is_features_struct(ty_name) and
                    entry.features_struct is None):
                    entry.features_struct = ty_name
                    
                elif (self.is_properties_struct(ty_name) and
                      entry.properties_struct is None):
                    entry.properties_struct = ty_name

            if entry.features_struct:
                struct_name = entry.features_struct
                if entry.features_struct in struct_aliases:
                    struct_name = struct_aliases[entry.features_struct]
                    entry.features_promoted = True

                elif entry.promoted_in is not None:
                    # if the extension is promoted but a core-Vulkan alias is not
                    # available for the features, then consider the features struct
                    # non-core-promoted
                    entry.features_promoted = False

                for field in vkxml.findall("./types/type[@name='{}']/member".format(struct_name)):
                    field_name = field.find("name").text
                    
                    # we ignore sType and pNext since they are irrelevant
                    if field_name not in ["sType", "pNext"]:
                        entry.features_fields.append(field_name)

            if entry.properties_struct:
                struct_name = entry.properties_struct
                if entry.properties_struct in struct_aliases:
                    struct_name = struct_aliases[entry.properties_struct]
                    entry.properties_promoted = True

                elif entry.promoted_in is not None:
                    # if the extension is promoted but a core-Vulkan alias is not
                    # available for the properties, then it is not promoted to core
                    entry.properties_promoted = False
                
                for field in vkxml.findall("./types/type[@name='{}']/member".format(struct_name)):
                    field_name = field.find("name").text

                    # we ignore sType and pNext since they are irrelevant
                    if field_name not in ["sType", "pNext"]:
                        entry.properties_fields.append(field_name)

            if ext.get("platform") is not None:
                entry.platform_guard = platform_guards[ext.get("platform")]

            self.registry[name] = entry

    def in_registry(self, ext_name: str):
        return ext_name in self.registry

    def get_registry_entry(self, ext_name: str):
        if self.in_registry(ext_name):
            return self.registry[ext_name]

    # Parses e.g. "VK_VERSION_x_y" to integer tuple (x, y)
    # For any erroneous inputs, None is returned
    def parse_promotedto(self, promotedto: str):
        result = None

        if promotedto and promotedto.startswith("VK_VERSION_"):
            (major, minor) = promotedto.split('_')[-2:]
            result = (int(major), int(minor))

        return result

    def is_features_struct(self, struct: str):
        return re.match(r"VkPhysicalDevice.*Features.*", struct) is not None

    def is_properties_struct(self, struct: str):
        return re.match(r"VkPhysicalDevice.*Properties.*", struct) is not None
