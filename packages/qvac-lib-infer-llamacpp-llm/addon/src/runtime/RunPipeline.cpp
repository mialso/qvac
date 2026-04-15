#include "runtime/RunPipeline.hpp"

#include <sstream>
#include <stdexcept>

#include <qvac-lib-inference-addon-cpp/Errors.hpp>

#include "addon/LlmErrors.hpp"
#include "context/LlmContext.hpp"
#include "utils/LoggingMacros.hpp"

using namespace qvac_lib_inference_addon_llama::errors;
using namespace qvac_lib_inference_addon_cpp::logger;
using namespace qvac_lib_inference_addon_llama::logging;

namespace qvac_lib_inference_addon_llama::runtime {

RunResult RunPipeline::run(const RunRequest& request) const {
  if (request.deps.context == nullptr) {
    throw std::runtime_error("RunPipeline request is missing runtime context");
  }
  if (request.cacheSessionPolicy == nullptr) {
    throw std::runtime_error("RunPipeline request is missing cache policy");
  }
  if (request.generationParamsPolicy == nullptr) {
    throw std::runtime_error("RunPipeline request is missing generation policy");
  }
  if (request.deps.compactionPolicy == nullptr) {
    throw std::runtime_error("RunPipeline request is missing compaction policy");
  }
  if (request.postRunPolicy == nullptr) {
    throw std::runtime_error("RunPipeline request is missing post-run policy");
  }
  if (!request.resetState) {
    throw std::runtime_error("RunPipeline request is missing reset callback");
  }
  if (!request.loadMedia) {
    throw std::runtime_error("RunPipeline request is missing media loader");
  }

  RunResult result;
  LlmContext& context = *request.deps.context;

  // Stage orchestrator: keep run flow explicit (cache -> eval -> generate ->
  // finalize) and keep context/model classes focused on primitives.
  // Reset per-inference slide counter so it does not leak across runs.
  context.resetNSlides();

  for (const auto& media : request.media) {
    request.loadMedia(media);
  }

  std::string out;
  auto resolved = request.cacheSessionPolicy->resolveSession(
      request.input, request.cacheKey, request.deps);

  if (resolved.shouldResetAfterInference && context.getNPast() > 0) {
    request.resetState(true);
  }

  if (resolved.chatMsgs.empty() && resolved.tools.empty()) {
    QLOG_IF(Priority::INFO, "No messages to process - returning early\n");
    result.output = out;
    return result;
  }

  auto paramsGuard = request.generationParamsPolicy->applyOverrides(
      context, request.generationParams);

  bool evalOk =
      resolved.tools.empty()
          ? context.evalMessage(
                resolved.chatMsgs, resolved.isCacheLoaded, request.prefill)
          : context.evalMessageWithTools(
                resolved.chatMsgs,
                resolved.tools,
                resolved.isCacheLoaded,
                request.prefill);

  if (!evalOk) {
    QLOG_IF(
        Priority::DEBUG,
        "Inference was interrupted during prompt evaluation\n");
    result.output = out;
    return result;
  }

  if (request.prefill) {
    result.output = out;
    return result;
  }

  std::ostringstream oss;
  bool needsOutputCapture =
      request.deps.compactionPolicy->shouldCaptureGeneratedOutput();
  auto callback = request.outputCallback;
  if (!request.outputCallback) {
    callback = [&](const std::string& token) { oss << token; };
  } else if (needsOutputCapture) {
    callback = [&](const std::string& token) {
      oss << token;
      request.outputCallback(token);
    };
  }

  if (!context.generateResponse(callback)) {
    request.resetState(true);
    std::string errorMsg = string_format("%s: context overflow\n", __func__);
    throw qvac_errors::StatusError(
        ADDON_ID, toString(ContextOverflow), errorMsg);
  }

  if (!request.outputCallback) {
    out = oss.str();
  }

  std::string capturedOutput = needsOutputCapture ? oss.str() : std::string();
  auto postRun = request.postRunPolicy->finalize(
      {.context = context,
       .compactionPolicy = *request.deps.compactionPolicy,
       .cacheManager = request.deps.cacheManager,
       .saveCacheToDisk = request.saveCacheToDisk,
       .shouldResetAfterInference = resolved.shouldResetAfterInference,
       .outputCaptured = needsOutputCapture,
       .output = out,
       .capturedOutput = capturedOutput,
       .resetState = request.resetState});

  if (request.setDebugBoundaryStats) {
    request.setDebugBoundaryStats(postRun.nPastBeforeTools, postRun.toolsTrimmed);
  }

  result.output = out;
  return result;
}

} // namespace qvac_lib_inference_addon_llama::runtime
