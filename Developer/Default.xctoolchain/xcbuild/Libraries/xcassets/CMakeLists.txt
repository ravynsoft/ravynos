#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(xcassets
            Sources/BrandAssetRole.cpp
            Sources/Compression.cpp
            Sources/CubeFace.cpp
            Sources/Insets.cpp
            Sources/Resizing.cpp
            Sources/ContentReference.cpp
            Sources/FullyQualifiedName.cpp
            Sources/MatchingStyle.cpp
            Sources/MipmapLevel.cpp
            Sources/MipmapLevelMode.cpp
            Sources/TemplateRenderingIntent.cpp
            Sources/StickerDurationType.cpp
            Sources/StickerGridSize.cpp
            Sources/TextureInterpretation.cpp
            Sources/TextureOrigin.cpp
            Sources/TexturePixelFormat.cpp
            Sources/WatchComplicationRole.cpp
            Sources/Slot/ColorSpace.cpp
            Sources/Slot/DeviceSubtype.cpp
            Sources/Slot/GraphicsFeatureSet.cpp
            Sources/Slot/Idiom.cpp
            Sources/Slot/ImageSize.cpp
            Sources/Slot/Orientation.cpp
            Sources/Slot/LaunchImageExtent.cpp
            Sources/Slot/MemoryRequirement.cpp
            Sources/Slot/Scale.cpp
            Sources/Slot/SizeClass.cpp
            Sources/Slot/SystemVersion.cpp
            Sources/Slot/WatchIconRole.cpp
            Sources/Slot/WatchSubtype.cpp
            Sources/Asset/Asset.cpp
            Sources/Asset/AppIconSet.cpp
            Sources/Asset/BrandAssets.cpp
            Sources/Asset/Catalog.cpp
            Sources/Asset/ComplicationSet.cpp
            Sources/Asset/CubeTextureSet.cpp
            Sources/Asset/DataSet.cpp
            Sources/Asset/GCDashboardImage.cpp
            Sources/Asset/GCLeaderboardSet.cpp
            Sources/Asset/GCLeaderboard.cpp
            Sources/Asset/Group.cpp
            Sources/Asset/IconSet.cpp
            Sources/Asset/ImageSet.cpp
            Sources/Asset/ImageStack.cpp
            Sources/Asset/ImageStackLayer.cpp
            Sources/Asset/LaunchImage.cpp
            Sources/Asset/MipmapSet.cpp
            Sources/Asset/SpriteAtlas.cpp
            Sources/Asset/Sticker.cpp
            Sources/Asset/StickerPack.cpp
            Sources/Asset/StickerSequence.cpp
            Sources/Asset/Stickers.cpp
            Sources/Asset/StickersIconSet.cpp
            Sources/Asset/TextureSet.cpp
            )

target_link_libraries(xcassets PUBLIC util_xcbuild plist ext)
target_include_directories(xcassets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcassets DESTINATION usr/lib)

add_executable(dump_xcassets Tools/dump_xcassets.cpp)
target_link_libraries(dump_xcassets PRIVATE xcassets)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(xcassets FullyQualifiedName Tests/test_FullyQualifiedName.cpp)
  ADD_UNIT_GTEST(xcassets IconSet Tests/test_IconSet.cpp)
  ADD_UNIT_GTEST(xcassets Scale Tests/test_Scale.cpp)
  ADD_UNIT_GTEST(xcassets ImageSize Tests/test_ImageSize.cpp)
  ADD_UNIT_GTEST(xcassets SystemVersion Tests/test_SystemVersion.cpp)
  ADD_UNIT_GTEST(xcassets Group Tests/test_Group.cpp)
endif ()
