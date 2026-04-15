#include "runtime/policies/postrun/PostRunPolicy.hpp"

#include "common/common.h"
#include "model-interface/CacheManager.hpp"
#include "context/LlmContext.hpp"
#include "utils/LoggingMacros.hpp"

using namespace qvac_lib_inference_addon_cpp::logger;
using namespace qvac_lib_inference_addon_llama::logging;

namespace qvac_lib_inference_addon_llama::runtime {

PostRunResult PostRunPolicy::finalize(const PostRunRequest& request) const {
  PostRunResult result;

  auto& compactionPolicy = request.compactionPolicy;
  result.nPastBeforeTools = compactionPolicy.nPastBeforeTools();
  const llama_pos firstMsgTokens = request.context.getFirstMsgTokens();

  if (compactionPolicy.hasDegenerateBoundary(firstMsgTokens)) {
    QLOG_IF(
        Priority::WARNING,
        string_format(
            "[LlamaModel] tools_compact degenerate boundary at first message "
            "(nPastBeforeTools=%d, firstMsgTokens=%d); skipping "
            "post-generation tools trim\n",
            compactionPolicy.nPastBeforeTools(),
            firstMsgTokens));
    compactionPolicy.reset();
  }

  const std::string& outputToCheck =
      request.outputCaptured ? request.capturedOutput : request.output;
  if (compactionPolicy.shouldTrimAfterGeneration(
          firstMsgTokens, request.context.getNPast(), outputToCheck)) {
    result.toolsTrimmed = true;
    request.context.removeLastNTokens(
        request.context.getNPast() - compactionPolicy.nPastBeforeTools());
    compactionPolicy.reset();
    if (request.context.getFirstMsgTokens() > request.context.getNPast()) {
      request.context.setFirstMsgTokens(request.context.getNPast());
    }
  }

  if (request.saveCacheToDisk && request.cacheManager != nullptr &&
      request.cacheManager->hasActiveCache()) {
    request.cacheManager->saveCache();
  }

  if (request.shouldResetAfterInference && request.resetState) {
    request.resetState(false);
  }

  return result;
}

} // namespace qvac_lib_inference_addon_llama::runtime

