#include "Qwen3Profile.hpp"

#include "utils/Qwen3ToolsDynamicTemplate.hpp"
#include "utils/QwenTemplate.hpp"

namespace qvac_lib_inference_addon_llama::profile {

const ModelCapabilities& Qwen3Profile::capabilities() const {
  static const ModelCapabilities kCapabilities{
      .isQwen3 = true, .supportsToolsCompact = true};
  return kCapabilities;
}

std::string Qwen3Profile::selectChatTemplate(
    const std::string& manualOverride, bool toolsCompact) const {
  if (!manualOverride.empty()) {
    return manualOverride;
  }

  return toolsCompact ? qvac_lib_inference_addon_llama::utils::
                            getToolsDynamicQwen3Template()
                      : qvac_lib_inference_addon_llama::utils::
                            getFixedQwen3Template();
}

} // namespace qvac_lib_inference_addon_llama::profile
