// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/BoneTransformDataIn.h"
#include "carla/rpc/String.h"
#include "carla/rpc/Transform.h"

#ifdef LIBCARLA_INCLUDED_FROM_UNREAL_ENGINE
#include <compiler/enable-ue4-macros.h>
#include "Carla/Walker/WalkerBoneControlIn.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UNREAL_ENGINE

#include <vector>

namespace carla {
namespace rpc {

  class WalkerBoneControlIn {
  public:

    WalkerBoneControlIn() = default;

    explicit WalkerBoneControlIn(
        std::vector<rpc::BoneTransformDataIn> bone_transforms)
      : bone_transforms(bone_transforms) {}

#ifdef LIBCARLA_INCLUDED_FROM_UNREAL_ENGINE

    operator FWalkerBoneControlIn() const {
      FWalkerBoneControlIn Control;
      for (auto &bone_transform : bone_transforms) {
        Control.BoneTransforms.Add(ToFString(bone_transform.first), bone_transform.second);
      }
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UNREAL_ENGINE

    std::vector<rpc::BoneTransformDataIn> bone_transforms;

    MSGPACK_DEFINE_ARRAY(bone_transforms);
  };

} // namespace rpc
} // namespace carla
