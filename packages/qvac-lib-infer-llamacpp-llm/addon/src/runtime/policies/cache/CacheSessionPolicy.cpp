#include "runtime/policies/cache/CacheSessionPolicy.hpp"

#include <utility>

#include "model-interface/CacheManager.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

CacheSessionResolution CacheSessionPolicy::resolveSession(
    const std::string& inputPrompt, const std::string& cacheKey,
    const RuntimeDeps& deps) const {
  CacheSessionResolution resolved;
  if (deps.cacheManager != nullptr) {
    resolved.isCacheLoaded = deps.cacheManager->handleCache(
        resolved.chatMsgs,
        resolved.tools,
        inputPrompt,
        deps.formatPrompt,
        cacheKey);
    resolved.shouldResetAfterInference =
        deps.cacheManager->isCacheDisabled() ||
        !deps.cacheManager->wasCacheUsedInLastPrompt();
    return resolved;
  }

  auto formatted = deps.formatPrompt(inputPrompt);
  resolved.chatMsgs = std::move(formatted.first);
  resolved.tools = std::move(formatted.second);
  resolved.shouldResetAfterInference = true;
  return resolved;
}

} // namespace qvac_lib_inference_addon_llama::runtime

