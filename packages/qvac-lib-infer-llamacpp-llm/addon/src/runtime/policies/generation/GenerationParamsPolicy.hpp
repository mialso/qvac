#pragma once

#include <functional>

#include "model-interface/LlmContext.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

class GenerationParamsGuard {
public:
  GenerationParamsGuard() = default;
  explicit GenerationParamsGuard(std::function<void()>&& restoreFn);
  ~GenerationParamsGuard();

  GenerationParamsGuard(GenerationParamsGuard&& other) noexcept;
  GenerationParamsGuard& operator=(GenerationParamsGuard&& other) noexcept;

  GenerationParamsGuard(const GenerationParamsGuard&) = delete;
  GenerationParamsGuard& operator=(const GenerationParamsGuard&) = delete;

  void restore();
  void dismiss();

private:
  std::function<void()> restoreFn_;
};

class GenerationParamsPolicy {
public:
  GenerationParamsGuard applyOverrides(
      LlmContext& context, const GenerationParams& params) const;
};

} // namespace qvac_lib_inference_addon_llama::runtime

