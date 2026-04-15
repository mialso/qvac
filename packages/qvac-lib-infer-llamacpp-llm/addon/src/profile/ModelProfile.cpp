#include "ModelProfile.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <utility>

#include <llama.h>

#include "DefaultProfile.hpp"
#include "Qwen3Profile.hpp"
#include "model-interface/ModelMetadata.hpp"

namespace qvac_lib_inference_addon_llama::profile {
namespace {

std::string toLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return value;
}

bool containsQwen3(const std::string& value) {
  const std::string lower = toLower(value);
  return lower.find("qwen3") != std::string::npos ||
         lower.find("qwen-3") != std::string::npos;
}

std::optional<std::string>
readModelMetaString(const ::llama_model* model, const char* key, size_t maxLen) {
  if (model == nullptr || key == nullptr || maxLen == 0) {
    return std::nullopt;
  }

  std::string value(maxLen, '\0');
  const int32_t len =
      llama_model_meta_val_str(model, key, value.data(), value.size());
  if (len <= 0 || static_cast<size_t>(len) >= value.size()) {
    return std::nullopt;
  }

  value.resize(static_cast<size_t>(len));
  return value;
}

bool isQwen3FromModel(const ::llama_model* model) {
  if (model == nullptr) {
    return false;
  }

  constexpr size_t kModelNameMaxLen = 256;
  if (const auto modelName =
          readModelMetaString(model, "general.name", kModelNameMaxLen);
      modelName.has_value() && containsQwen3(*modelName)) {
    return true;
  }

  constexpr size_t kArchMaxLen = 64;
  if (const auto arch = readModelMetaString(
          model, "general.architecture", kArchMaxLen);
      arch.has_value() && containsQwen3(*arch)) {
    return true;
  }

  return false;
}

bool isQwen3FromMetadata(const ModelMetaData& metadata) {
  if (const auto modelName = metadata.tryGetString("general.name");
      modelName.has_value() && containsQwen3(*modelName)) {
    return true;
  }

  if (const auto arch = metadata.tryGetString("general.architecture");
      arch.has_value() && containsQwen3(*arch)) {
    return true;
  }

  return false;
}

std::unique_ptr<ModelProfile> createProfileForQwen3(bool isQwen3) {
  if (isQwen3) {
    return std::make_unique<Qwen3Profile>();
  }
  return std::make_unique<DefaultProfile>();
}

} // namespace

std::unique_ptr<ModelProfile> createProfileFromModel(const ::llama_model* model) {
  return createProfileForQwen3(isQwen3FromModel(model));
}

std::unique_ptr<ModelProfile>
createProfileFromMetadata(const ModelMetaData& metadata) {
  return createProfileForQwen3(isQwen3FromMetadata(metadata));
}

} // namespace qvac_lib_inference_addon_llama::profile
