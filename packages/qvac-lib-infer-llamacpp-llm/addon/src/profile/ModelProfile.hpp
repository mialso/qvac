#pragma once

#include <memory>
#include <string>

struct llama_model;
class ModelMetaData;

namespace qvac_lib_inference_addon_llama::profile {

struct ModelCapabilities {
  bool isQwen3 = false;
  bool supportsToolsCompact = false;
};

class ModelProfile {
public:
  virtual ~ModelProfile() = default;

  [[nodiscard]] virtual const ModelCapabilities& capabilities() const = 0;

  [[nodiscard]] virtual std::string selectChatTemplate(
      const std::string& manualOverride, bool toolsCompact) const = 0;
};

std::unique_ptr<ModelProfile> createProfileFromModel(const ::llama_model* model);

std::unique_ptr<ModelProfile>
createProfileFromMetadata(const ModelMetaData& metadata);

} // namespace qvac_lib_inference_addon_llama::profile
