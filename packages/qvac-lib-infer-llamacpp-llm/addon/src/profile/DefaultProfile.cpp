#include "DefaultProfile.hpp"

namespace qvac_lib_inference_addon_llama::profile {

const ModelCapabilities& DefaultProfile::capabilities() const {
  static const ModelCapabilities kCapabilities{
      .isQwen3 = false, .supportsToolsCompact = false};
  return kCapabilities;
}

std::string DefaultProfile::selectChatTemplate(
    const std::string& manualOverride, bool toolsCompact) const {
  (void)toolsCompact;
  return manualOverride;
}

} // namespace qvac_lib_inference_addon_llama::profile
