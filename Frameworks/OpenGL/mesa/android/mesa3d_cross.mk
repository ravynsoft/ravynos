# Mesa 3-D graphics library
#
# Copyright (C) 2021 GlobalLogic Ukraine
# Copyright (C) 2021 Roman Stratiienko (r.stratiienko@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

MY_PATH := $(call my-dir)

AOSP_ABSOLUTE_PATH := $(realpath .)
define relative-to-absolute
$(if $(patsubst /%,,$1),$(AOSP_ABSOLUTE_PATH)/$1,$1)
endef

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE := meson.dummy.$(LOCAL_MULTILIB)

m_dummy := $(local-generated-sources-dir)/dummy.c
$(m_dummy):
	mkdir -p $(dir $@)
	touch $@

LOCAL_GENERATED_SOURCES := $(m_dummy)
LOCAL_VENDOR_MODULE := true

# Prepare intermediate variables by AOSP make/core internals
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(MY_PATH)

link_deps := \
	$(built_static_libraries) \
	$(built_shared_libraries) \
	$(built_whole_libraries) \
	$(strip $(all_objects)) \
	$(my_target_libatomic) \
	$(my_target_libcrt_builtins) \
	$(my_target_crtbegin_so_o) \
	$(my_target_crtend_so_o)

# Build mesa3d using intermediate variables provided by AOSP make/core internals
M_TARGET_PREFIX := $(my_2nd_arch_prefix)

MESA3D_LIB_DIR := lib$(subst 32,,$(LOCAL_MULTILIB))

MESON_OUT_DIR                            := $($(M_TARGET_PREFIX)TARGET_OUT_INTERMEDIATES)/MESON_MESA3D
MESON_GEN_DIR                            := $(MESON_OUT_DIR)_GEN
MESON_GEN_FILES_TARGET                   := $(MESON_GEN_DIR)/.timestamp

MESA3D_GALLIUM_DRI_DIR                   := $(MESON_OUT_DIR)/install/usr/local/lib/dri
$(M_TARGET_PREFIX)MESA3D_GALLIUM_DRI_BIN := $(MESON_OUT_DIR)/install/usr/local/lib/libgallium_dri.so
$(M_TARGET_PREFIX)MESA3D_LIBEGL_BIN      := $(MESON_OUT_DIR)/install/usr/local/lib/libEGL.so.1.0.0
$(M_TARGET_PREFIX)MESA3D_LIBGLESV1_BIN   := $(MESON_OUT_DIR)/install/usr/local/lib/libGLESv1_CM.so.1.1.0
$(M_TARGET_PREFIX)MESA3D_LIBGLESV2_BIN   := $(MESON_OUT_DIR)/install/usr/local/lib/libGLESv2.so.2.0.0
$(M_TARGET_PREFIX)MESA3D_LIBGLAPI_BIN    := $(MESON_OUT_DIR)/install/usr/local/lib/libglapi.so.0.0.0
$(M_TARGET_PREFIX)MESA3D_LIBGBM_BIN      := $(MESON_OUT_DIR)/install/usr/local/lib/$(MESA_LIBGBM_NAME).so.1.0.0


MESA3D_GLES_BINS := \
    $($(M_TARGET_PREFIX)MESA3D_LIBEGL_BIN)    \
    $($(M_TARGET_PREFIX)MESA3D_LIBGLESV1_BIN) \
    $($(M_TARGET_PREFIX)MESA3D_LIBGLESV2_BIN) \
    $($(M_TARGET_PREFIX)MESA3D_LIBGLAPI_BIN)  \

MESON_GEN_NINJA := \
	cd $(MESON_OUT_DIR) && PATH=/usr/bin:/usr/local/bin:$$PATH meson ./build     \
	--cross-file $(call relative-to-absolute,$(MESON_GEN_DIR))/aosp_cross        \
	--buildtype=release                                                          \
	-Ddri-search-path=/vendor/$(MESA3D_LIB_DIR)/dri                              \
	-Dplatforms=android                                                          \
	-Dplatform-sdk-version=$(PLATFORM_SDK_VERSION)                               \
	-Dgallium-drivers=$(subst $(space),$(comma),$(BOARD_MESA3D_GALLIUM_DRIVERS)) \
	-Dvulkan-drivers=$(subst $(space),$(comma),$(subst radeon,amd,$(BOARD_MESA3D_VULKAN_DRIVERS)))   \
	-Dgbm=enabled                                                                \
	-Degl=$(if $(BOARD_MESA3D_GALLIUM_DRIVERS),enabled,disabled)                 \
	-Dllvm=$(if $(MESON_GEN_LLVM_STUB),enabled,disabled)                         \
	-Dcpp_rtti=false                                                             \
	-Dlmsensors=disabled                                                         \
	-Dandroid-libbacktrace=disabled                                              \
	$(BOARD_MESA3D_MESON_ARGS)                                                   \

MESON_BUILD := PATH=/usr/bin:/bin:/sbin:$$PATH ninja -C $(MESON_OUT_DIR)/build

$(MESON_GEN_FILES_TARGET): MESON_CPU_FAMILY := $(subst arm64,aarch64,$(TARGET_$(M_TARGET_PREFIX)ARCH))

define create-pkgconfig
echo -e "Name: $2" \
	"\nDescription: $2" \
	"\nVersion: $3" > $1/$2.pc

endef

# Taken from build/make/core/binary.mk. We need this
# to use definitions from build/make/core/definitions.mk
$(MESON_GEN_FILES_TARGET): PRIVATE_GLOBAL_C_INCLUDES := $(my_target_global_c_includes)
$(MESON_GEN_FILES_TARGET): PRIVATE_GLOBAL_C_SYSTEM_INCLUDES := $(my_target_global_c_system_includes)

$(MESON_GEN_FILES_TARGET): PRIVATE_2ND_ARCH_VAR_PREFIX := $(M_TARGET_PREFIX)
$(MESON_GEN_FILES_TARGET): PRIVATE_CC := $(my_cc)
$(MESON_GEN_FILES_TARGET): PRIVATE_LINKER := $(my_linker)
$(MESON_GEN_FILES_TARGET): PRIVATE_CXX := $(my_cxx)
$(MESON_GEN_FILES_TARGET): PRIVATE_CXX_LINK := $(my_cxx_link)
$(MESON_GEN_FILES_TARGET): PRIVATE_YACCFLAGS := $(LOCAL_YACCFLAGS)
$(MESON_GEN_FILES_TARGET): PRIVATE_ASFLAGS := $(my_asflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_CONLYFLAGS := $(my_conlyflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_CFLAGS := $(my_cflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_CPPFLAGS := $(my_cppflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_CFLAGS_NO_OVERRIDE := $(my_cflags_no_override)
$(MESON_GEN_FILES_TARGET): PRIVATE_CPPFLAGS_NO_OVERRIDE := $(my_cppflags_no_override)
$(MESON_GEN_FILES_TARGET): PRIVATE_RTTI_FLAG := $(LOCAL_RTTI_FLAG)
$(MESON_GEN_FILES_TARGET): PRIVATE_DEBUG_CFLAGS := $(debug_cflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_C_INCLUDES := $(my_c_includes)
$(MESON_GEN_FILES_TARGET): PRIVATE_IMPORTED_INCLUDES := $(imported_includes)
$(MESON_GEN_FILES_TARGET): PRIVATE_LDFLAGS := $(my_ldflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_LDLIBS := $(my_ldlibs)
$(MESON_GEN_FILES_TARGET): PRIVATE_TIDY_CHECKS := $(my_tidy_checks)
$(MESON_GEN_FILES_TARGET): PRIVATE_TIDY_FLAGS := $(my_tidy_flags)
$(MESON_GEN_FILES_TARGET): PRIVATE_ARFLAGS := $(my_arflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_ALL_SHARED_LIBRARIES := $(built_shared_libraries)
$(MESON_GEN_FILES_TARGET): PRIVATE_ALL_STATIC_LIBRARIES := $(built_static_libraries)
$(MESON_GEN_FILES_TARGET): PRIVATE_ALL_WHOLE_STATIC_LIBRARIES := $(built_whole_libraries)
$(MESON_GEN_FILES_TARGET): PRIVATE_ALL_OBJECTS := $(strip $(all_objects))

$(MESON_GEN_FILES_TARGET): PRIVATE_ARM_CFLAGS := $(normal_objects_cflags)

$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_GLOBAL_CFLAGS := $(my_target_global_cflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_GLOBAL_CONLYFLAGS := $(my_target_global_conlyflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_GLOBAL_CPPFLAGS := $(my_target_global_cppflags)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_GLOBAL_LDFLAGS := $(my_target_global_ldflags)

$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_LIBCRT_BUILTINS := $(my_target_libcrt_builtins)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_LIBATOMIC := $(my_target_libatomic)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_CRTBEGIN_SO_O := $(my_target_crtbegin_so_o)
$(MESON_GEN_FILES_TARGET): PRIVATE_TARGET_CRTEND_SO_O := $(my_target_crtend_so_o)
##

define m-lld-flags
  -Wl,-e,main \
  -nostdlib -Wl,--gc-sections \
  $(PRIVATE_TARGET_CRTBEGIN_SO_O) \
  $(PRIVATE_ALL_OBJECTS) \
  -Wl,--whole-archive \
  $(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES) \
  -Wl,--no-whole-archive \
  $(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
  $(PRIVATE_ALL_STATIC_LIBRARIES) \
  $(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
  $(if $(filter true,$(NATIVE_COVERAGE)),$(PRIVATE_TARGET_COVERAGE_LIB)) \
  $(PRIVATE_TARGET_LIBCRT_BUILTINS) \
  $(PRIVATE_TARGET_LIBATOMIC) \
  $(PRIVATE_TARGET_GLOBAL_LDFLAGS) \
  $(PRIVATE_LDFLAGS) \
  $(PRIVATE_ALL_SHARED_LIBRARIES) \
  $(PRIVATE_TARGET_CRTEND_SO_O) \
  $(PRIVATE_LDLIBS)
endef

define m-lld-flags-cleaned
  $(patsubst -Wl$(comma)--build-id=%,, \
  $(subst prebuilts/,$(AOSP_ABSOLUTE_PATH)/prebuilts/, \
  $(subst $(OUT_DIR)/,$(call relative-to-absolute,$(OUT_DIR))/, \
  $(subst -Wl$(comma)--fatal-warnings,,                \
  $(subst -Wl$(comma)--no-undefined-version,,          \
  $(subst -Wl$(comma)--gc-sections,,                   \
  $(patsubst %dummy.o,,                                \
    $(m-lld-flags))))))))
endef

define m-cpp-flags
  $(PRIVATE_TARGET_GLOBAL_CFLAGS) \
  $(PRIVATE_TARGET_GLOBAL_CPPFLAGS) \
  $(PRIVATE_ARM_CFLAGS) \
  $(PRIVATE_RTTI_FLAG) \
  $(PRIVATE_CFLAGS) \
  $(PRIVATE_CPPFLAGS) \
  $(PRIVATE_DEBUG_CFLAGS) \
  $(PRIVATE_CFLAGS_NO_OVERRIDE) \
  $(PRIVATE_CPPFLAGS_NO_OVERRIDE)
endef

define m-c-flags
  $(PRIVATE_TARGET_GLOBAL_CFLAGS) \
  $(PRIVATE_TARGET_GLOBAL_CONLYFLAGS) \
  $(PRIVATE_ARM_CFLAGS) \
  $(PRIVATE_CFLAGS) \
  $(PRIVATE_CONLYFLAGS) \
  $(PRIVATE_DEBUG_CFLAGS) \
  $(PRIVATE_CFLAGS_NO_OVERRIDE)
endef

define filter-c-flags
  $(filter-out -std=gnu++17 -std=gnu++14 -std=gnu99 -fno-rtti \
    -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang \
    -ftrivial-auto-var-init=zero,
    $(patsubst  -W%,, $1))
endef

define nospace-includes
  $(subst $(space)-isystem$(space),$(space)-isystem, \
  $(subst $(space)-I$(space),$(space)-I, \
  $(strip $(c-includes))))
endef

# Ensure include paths are always absolute
# When OUT_DIR_COMMON_BASE env variable is set the AOSP/KATI will use absolute paths
# for headers in intermediate output directories, but relative for all others.
define abs-include
$(strip \
  $(if $(patsubst -I%,,$1),\
    $(if $(patsubst -isystem/%,,$1),\
      $(subst -isystem,-isystem$(AOSP_ABSOLUTE_PATH)/,$1),\
      $1\
    ),\
    $(if $(patsubst -I/%,,$1),\
      $(subst -I,-I$(AOSP_ABSOLUTE_PATH)/,$1),\
      $1\
    )\
  )
)
endef

$(MESON_GEN_FILES_TARGET): PREPROCESS_MESON_CONFIGS:=$(PREPROCESS_MESON_CONFIGS)
$(MESON_GEN_FILES_TARGET): MESON_GEN_DIR:=$(MESON_GEN_DIR)
$(MESON_GEN_FILES_TARGET): $(sort $(shell find -L $(MESA3D_TOP) -not -path '*/\.*'))
	mkdir -p $(dir $@)
	echo -e "[properties]\n"                                                                                                  \
		"c_args = [$(foreach flag,$(call filter-c-flags,$(m-c-flags)),'$(flag)', ) \
                           $(foreach inc,$(nospace-includes),'$(call abs-include,$(inc))', )'']\n" \
		"cpp_args = [$(foreach flag,$(call filter-c-flags,$(m-cpp-flags)),'$(flag)', ) \
                             $(foreach inc,$(nospace-includes),'$(call abs-include,$(inc))', )'']\n" \
		"c_link_args = [$(foreach flag, $(m-lld-flags-cleaned),'$(flag)',)'']\n"                                          \
		"cpp_link_args = [$(foreach flag, $(m-lld-flags-cleaned),'$(flag)',)'']\n"                                        \
		"needs_exe_wrapper = true\n"                                                                                      \
		"[binaries]\n"                                                                                                    \
		"ar = '$(AOSP_ABSOLUTE_PATH)/$($($(M_TARGET_PREFIX))TARGET_AR)'\n"                                                \
		"c = [$(foreach arg,$(PRIVATE_CC),'$(subst prebuilts/,$(AOSP_ABSOLUTE_PATH)/prebuilts/,$(arg))',)'']\n"           \
		"cpp = [$(foreach arg,$(PRIVATE_CXX),'$(subst prebuilts/,$(AOSP_ABSOLUTE_PATH)/prebuilts/,$(arg))',)'']\n"        \
		"c_ld = 'lld'\n"                                                                                                  \
		"cpp_ld = 'lld'\n\n"                                                                                              \
		"pkgconfig = ['env', 'PKG_CONFIG_LIBDIR=' + '$(call relative-to-absolute,$(MESON_GEN_DIR))', '/usr/bin/pkg-config']\n\n" \
		"llvm-config = '/dev/null'\n"                                                                                     \
		"[host_machine]\n"                                                                                                \
		"system = 'linux'\n"                                                                                              \
		"cpu_family = '$(MESON_CPU_FAMILY)'\n"                                                                            \
		"cpu = '$(MESON_CPU_FAMILY)'\n"                                                                                   \
		"endian = 'little'" > $(dir $@)/aosp_cross

	#
	$(foreach pkg, $(MESON_GEN_PKGCONFIGS), $(call create-pkgconfig,$(dir $@),$(word 1, $(subst :, ,$(pkg))),$(word 2, $(subst :, ,$(pkg)))))
	touch $@

$(MESON_OUT_DIR)/.build.timestamp: MESON_GEN_NINJA:=$(MESON_GEN_NINJA)
$(MESON_OUT_DIR)/.build.timestamp: MESON_BUILD:=$(MESON_BUILD)
$(MESON_OUT_DIR)/.build.timestamp: $(MESON_GEN_FILES_TARGET) $(link_deps)
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	mkdir -p $(dir $@)/build
	# Meson will update timestamps in sources directory, continuously retriggering the build
	# even if nothing changed. Copy sources into intermediate dir to avoid this effect.
	cp -r $(MESA3D_TOP)/* $(dir $@)
ifneq ($(MESON_GEN_LLVM_STUB),)
	mkdir -p $(dir $@)/subprojects/llvm/
	echo -e "project('llvm', 'cpp', version : '$(MESON_LLVM_VERSION)')\n" \
		"dep_llvm = declare_dependency()\n"                           \
		"has_rtti = false\n" > $(dir $@)/subprojects/llvm/meson.build
endif
	$(MESON_GEN_NINJA)
	$(MESON_BUILD)
	touch $@

MESON_COPY_LIBGALLIUM := \
	cp `ls -1 $(MESA3D_GALLIUM_DRI_DIR)/* | head -1` $($(M_TARGET_PREFIX)MESA3D_GALLIUM_DRI_BIN)

$(MESON_OUT_DIR)/install/.install.timestamp: MESON_COPY_LIBGALLIUM:=$(MESON_COPY_LIBGALLIUM)
$(MESON_OUT_DIR)/install/.install.timestamp: MESON_BUILD:=$(MESON_BUILD)
$(MESON_OUT_DIR)/install/.install.timestamp: $(MESON_OUT_DIR)/.build.timestamp
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	DESTDIR=$(call relative-to-absolute,$(dir $@)) $(MESON_BUILD) install
	$(if $(BOARD_MESA3D_GALLIUM_DRIVERS),$(MESON_COPY_LIBGALLIUM))
	touch $@

$($(M_TARGET_PREFIX)MESA3D_LIBGBM_BIN) $(MESA3D_GLES_BINS): $(MESON_OUT_DIR)/install/.install.timestamp
	echo "Build $@"
	touch $@

define vulkan_target
$(M_TARGET_PREFIX)MESA3D_VULKAN_$1_BIN := $(MESON_OUT_DIR)/install/usr/local/lib/libvulkan_$(MESA_VK_LIB_SUFFIX_$1).so
$(MESON_OUT_DIR)/install/usr/local/lib/libvulkan_$(MESA_VK_LIB_SUFFIX_$1).so: $(MESON_OUT_DIR)/install/.install.timestamp
	touch $(MESON_OUT_DIR)/install/usr/local/lib/libvulkan_$(MESA_VK_LIB_SUFFIX_$1).so

endef

$(foreach driver,$(BOARD_MESA3D_VULKAN_DRIVERS), $(eval $(call vulkan_target,$(driver))))

$($(M_TARGET_PREFIX)TARGET_OUT_VENDOR_SHARED_LIBRARIES)/dri/.symlinks.timestamp: MESA3D_GALLIUM_DRI_DIR:=$(MESA3D_GALLIUM_DRI_DIR)
$($(M_TARGET_PREFIX)TARGET_OUT_VENDOR_SHARED_LIBRARIES)/dri/.symlinks.timestamp: $(MESON_OUT_DIR)/install/.install.timestamp
	# Create Symlinks
	mkdir -p $(dir $@)
	ls -1 $(MESA3D_GALLIUM_DRI_DIR)/ | PATH=/usr/bin:$$PATH xargs -I{} ln -s -f libgallium_dri.so $(dir $@)/{}
	touch $@

$($(M_TARGET_PREFIX)MESA3D_GALLIUM_DRI_BIN): $(TARGET_OUT_VENDOR)/$(MESA3D_LIB_DIR)/dri/.symlinks.timestamp
	echo "Build $@"
	touch $@
