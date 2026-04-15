#include "runtime/policies/compaction/NoopCompactionPolicy.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

bool NoopCompactionPolicy::toolsCompactEnabled() const { return false; }

bool NoopCompactionPolicy::shouldEnforcePromptShape() const { return false; }

bool NoopCompactionPolicy::shouldCaptureGeneratedOutput() const { return false; }

void NoopCompactionPolicy::onRunStart(llama_pos /*nPast*/, bool /*isCacheLoaded*/) {}

void NoopCompactionPolicy::setConversationOnlyTokens(llama_pos /*tokens*/) {}

void NoopCompactionPolicy::clearConversationOnlyTokens() {}

void NoopCompactionPolicy::recordToolBoundary(
    llama_pos /*nPast*/, llama_pos /*totalTokens*/) {}

llama_pos NoopCompactionPolicy::clampDiscard(
    llama_pos nDiscarded, llama_pos /*firstMsgTokens*/) const {
  return nDiscarded;
}

void NoopCompactionPolicy::adjustAfterSlide(
    llama_pos /*discard*/, llama_pos /*firstMsgTokens*/) {}

bool NoopCompactionPolicy::hasDegenerateBoundary(
    llama_pos /*firstMsgTokens*/) const {
  return false;
}

bool NoopCompactionPolicy::hasUsableBoundary(
    llama_pos /*firstMsgTokens*/, llama_pos /*nPast*/) const {
  return false;
}

bool NoopCompactionPolicy::shouldTrimAfterGeneration(
    llama_pos /*firstMsgTokens*/, llama_pos /*nPast*/,
    const std::string& /*output*/) const {
  return false;
}

llama_pos NoopCompactionPolicy::nPastBeforeTools() const { return -1; }

void NoopCompactionPolicy::reset() {}

} // namespace qvac_lib_inference_addon_llama::runtime

