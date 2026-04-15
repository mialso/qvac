#include "runtime/policies/generation/GenerationParamsPolicy.hpp"

#include <utility>

namespace qvac_lib_inference_addon_llama::runtime {

GenerationParamsGuard::GenerationParamsGuard(std::function<void()>&& restoreFn)
    : restoreFn_(std::move(restoreFn)) {}

GenerationParamsGuard::~GenerationParamsGuard() {
  restore();
}

GenerationParamsGuard::GenerationParamsGuard(GenerationParamsGuard&& other) noexcept
    : restoreFn_(std::move(other.restoreFn_)) {}

GenerationParamsGuard&
GenerationParamsGuard::operator=(GenerationParamsGuard&& other) noexcept {
  if (this != &other) {
    restore();
    restoreFn_ = std::move(other.restoreFn_);
  }
  return *this;
}

void GenerationParamsGuard::restore() {
  if (!restoreFn_) {
    return;
  }
  auto restoreFn = std::move(restoreFn_);
  restoreFn_ = nullptr;
  restoreFn();
}

void GenerationParamsGuard::dismiss() {
  restoreFn_ = nullptr;
}

GenerationParamsGuard GenerationParamsPolicy::applyOverrides(
    LlmContext& context, const GenerationParams& params) const {
  return GenerationParamsGuard(context.applyGenerationParams(params));
}

} // namespace qvac_lib_inference_addon_llama::runtime

