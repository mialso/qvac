#pragma once

#include "ModelProfile.hpp"

namespace qvac_lib_inference_addon_llama::profile {

class Qwen3Profile : public ModelProfile {
public:
  [[nodiscard]] const ModelCapabilities& capabilities() const override;

  [[nodiscard]] std::string selectChatTemplate(
      const std::string& manualOverride, bool toolsCompact) const override;
};

} // namespace qvac_lib_inference_addon_llama::profile
