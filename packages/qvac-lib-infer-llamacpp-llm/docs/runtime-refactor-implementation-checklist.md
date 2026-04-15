# Runtime Refactor Implementation Checklist (Commit-by-Commit)

This checklist breaks the `runtime` refactor into small, mergeable commits.  
You can group these commits into multiple PRs or squash into one larger PR later.

## Conventions for This Refactor

- Keep behavior stable unless a commit explicitly states behavior change.
- Prefer extraction + delegation first, logic movement second.
- Keep each commit buildable and testable on its own.
- Avoid large file moves until late phase.

## Progress Tracking

Update this section after each completed step/commit.

### Commit Status Board

| Commit | Title | Status | Branch/Commit | Date | Notes |
| --- | --- | --- | --- | --- | --- |
| 0 | Baseline Safety Net | done | local baseline (no code changes) | 2026-04-15 | Sharded tests fail when shard models are absent; baseline captured before refactor |
| 1 | Runtime Scaffolding | done | local working tree | 2026-04-15 | Added runtime RunRequest/RunPipeline scaffold and delegated processPrompt through pipeline with no logic migration |
| 2 | Runtime State Facade | done | local working tree | 2026-04-15 | Added RuntimeDeps/RunResult facade and threaded through RunPipeline/LlamaModel without moving runtime logic |
| 3 | Prompt Policy | done | local working tree | 2026-04-15 | Added runtime PromptPolicy and delegated LlamaModel prompt parsing/validation to it while preserving legacy prompt errors/constraints |
| 4 | Cache Session Policy | done | local working tree | 2026-04-15 | Added runtime cache session policy and delegated LlamaModel cache session resolution through it with no cache behavior changes |
| 5 | Generation Params Policy | planned | - | - | - |
| 6 | Post-Run Policy | planned | - | - | - |
| 7 | Model Profiles | planned | - | - | - |
| 8 | Pipeline as Primary Orchestrator | planned | - | - | - |
| 9 | Context Folder Normalization (optional) | planned | - | - | - |
| 10 | Cleanup + Dead Code Removal | planned | - | - | - |

Status values:

- `planned`
- `in_progress`
- `done`
- `blocked`
- `skipped`

### Step Update Template

Use this template immediately after each step (or commit):

```md
#### Progress Update - Commit <N>: <Title>

- Status: done | blocked
- Scope completed:
  - <bullet 1>
  - <bullet 2>
- Tests run:
  - <test command/result>
- Notes/Risks:
  - <optional>
- Next:
  - Commit <N+1> - <Title>
```

### Step Update Log

Append new entries at the top (most recent first).

<!--
#### Progress Update - Commit X: Title
- Status:
- Scope completed:
- Tests run:
- Notes/Risks:
- Next:
-->

#### Progress Update - Commit 4: Cache Session Policy

- Status: done
- Scope completed:
  - Added `addon/src/runtime/policies/cache/CacheSessionPolicy.hpp` and `addon/src/runtime/policies/cache/CacheSessionPolicy.cpp` to own cache session resolution decisions for no key/same key/switch key paths via `CacheManager`.
  - Updated `LlamaModel::processPromptImpl` to delegate cache session preparation through `CacheSessionPolicy::resolveSession(...)` and removed cache decision logic from `LlamaModel::resolveChatAndTools`.
  - Wired build/test targets to compile the new policy source in `CMakeLists.txt` and `test/unit/CMakeLists.txt`.
- Tests run:
  - `bare-make build --target addon-test` -> passed
  - `./build/test/unit/addon-test --gtest_filter=CacheManagementTest.*:CacheManagementQwen3Test.*` -> failed (known baseline `CacheManagementQwen3Test` tools_compact cases from Commit 0), with non-Qwen cache suite still passing
  - `./build/test/unit/addon-test --gtest_filter=CacheManagementTest.*` -> all 21 tests passed; process exits non-zero due existing Vulkan LeakSanitizer leak report in this environment
- Notes/Risks:
  - Qwen3 tools_compact cache-path failures remain aligned with baseline risk areas documented earlier and were not introduced by this extraction.
  - LeakSanitizer Vulkan backend leak remains environment baseline noise for local unit runs.
- Next:
  - Commit 5 - Generation Params Policy

#### Progress Update - Commit 3: Prompt Policy

- Status: done
- Scope completed:
  - Added `addon/src/runtime/policies/prompt/PromptPolicy.hpp` and `addon/src/runtime/policies/prompt/PromptPolicy.cpp` and moved prompt parse/validation logic out of `LlamaModel::formatPrompt` into the policy.
  - Updated `LlamaModel::formatPrompt` to delegate to `PromptPolicy::resolvePrompt(...)` while preserving existing tools/tools_compact/media validation semantics and error text.
  - Wired build/test targets to compile the new policy source in `CMakeLists.txt` and `test/unit/CMakeLists.txt`.
- Tests run:
  - `bare-make generate -D BUILD_TESTING=ON` -> passed
  - `bare-make build --target addon-test` -> passed
  - `./build/test/unit/addon-test --gtest_filter=LlamaModelTest.FormatPromptMediaInTextOnlyModel:LlamaModelTest.FormatPromptMediaWithoutUserMessage:LlamaModelTest.FormatPromptMediaWithoutRequest:LlamaModelTest.InvalidJSONInput:LlamaModelTest.MalformedChatMessageFormat:ModelToolsQwen3Test.ToolsCompactRejectsPromptWithoutTools:ModelToolsQwen3Test.ToolsCompactRejectsToolsWithoutUserOrToolMessage:ModelToolsQwen3Test.ToolsCompactRejectsSplitOrDetachedToolBlocks:ModelToolsQwen3Test.ToolsCompactAllowsToolsAfterToolMessage` -> passed (9 tests)
- Notes/Risks:
  - Prompt parsing now depends on the new policy boundary; follow-up commits should continue migrating runtime orchestration through policy types.
- Next:
  - Commit 4 - Cache Session Policy

#### Progress Update - Commit 2: Runtime State Facade

- Status: done
- Scope completed:
  - Added `addon/src/runtime/RuntimeStateFacade.hpp` with facade types `RuntimeDeps`, `RunResult`, and `PromptFormatter`.
  - Updated `RunRequest`/`RunPipeline` to accept explicit runtime dependencies and return `RunResult`.
  - Updated `LlamaModel::processPrompt`/`processPromptImpl` to pass runtime dependencies through the pipeline while keeping legacy inference logic in place.
- Tests run:
  - `npm run test:cpp` -> failed on known baseline/environment-dependent tests (same categories as Commit 0/1 baseline), with runtime facade changes compiling and linking successfully.
- Notes/Risks:
  - Known failing tests remain concentrated in sharded-model + tools-compact/cache Qwen3 paths and match previously documented baseline risk areas.
  - No new compile-time regressions observed in runtime scaffold/facade wiring.
- Next:
  - Commit 3 - Prompt Policy

#### Progress Update - Commit 1: Runtime Scaffolding

- Status: done
- Scope completed:
  - Added `addon/src/runtime/RunRequest.hpp`, `addon/src/runtime/RunPipeline.hpp`, and `addon/src/runtime/RunPipeline.cpp`.
  - Wired `LlamaModel::processPrompt` to delegate through `RunPipeline` while preserving the existing `processPromptImpl` execution path.
  - Updated build/test source lists to compile the new runtime scaffold (`CMakeLists.txt`, `test/unit/CMakeLists.txt`).
- Tests run:
  - `npm run test:cpp` -> failed on known baseline/environment-dependent tests, but runtime scaffolding compiled/linked and full suite executed.
- Notes/Risks:
  - Observed failures align with known baseline categories from Commit 0 (tools-compact/cache/sharded-model related), plus `LlamaModelTest.ReloadThrowsForStreamedShardedModel` in this environment.
  - No new compile or link regressions after wiring `RunPipeline`.
- Next:
  - Commit 2 - Runtime State Facade

#### Progress Update - Commit 0: Baseline Safety Net

- Status: done
- Scope completed:
  - Captured pre-refactor baseline without runtime logic changes.
  - Confirmed and documented sharded-model test failures caused by missing local shard files.
  - Recorded additional known baseline failures to prevent refactor blame confusion.
- Tests run:
  - `npm run test:cpp` -> failed (baseline failures present before refactor work)
  - `npm run test:integration` -> blocked/stalled during model download (`AfriqueGemma-4B.Q4_K_M.gguf`)
- Notes/Risks:
  - Sharded tests requiring `Qwen3-0.6B-UD-IQ1_S-00001-of-00003.gguf` fail when shard assets are not present locally.
  - Additional current baseline failures include several `CacheManagementQwen3Test` cases, `TextLlmContextQwen3Test.DoubleTokenizeBoundaryAccuracy`, and `ModelToolsQwen3Test.CacheEnabledWithToolMessage`.
- Next:
  - Commit 1 - Runtime Scaffolding

## Commit 0 - Baseline Safety Net

### Goal

Capture a clean baseline before structural changes.

### Changes

- No runtime logic changes.
- Add/refresh documentation pointers if needed:
  - `docs/runtime-refactor-plan.md`
  - this checklist file

### Validation

- Ensure current unit/integration tests are green before starting.
- Record baseline failures (if any) to avoid refactor blame confusion.

---

## Commit 1 - Add Runtime Scaffolding (No Behavior Change)

### Goal

Introduce `runtime` skeleton with zero logic migration.

### Changes

- Add:
  - `addon/src/runtime/RunRequest.hpp`
  - `addon/src/runtime/RunPipeline.hpp`
  - `addon/src/runtime/RunPipeline.cpp`
- `RunPipeline` initially wraps existing `LlamaModel::processPrompt` path.
- Keep `LlamaModel` as owner; pipeline is a thin delegate.

### Acceptance Criteria

- No observable behavior change.
- `LlamaModel` still passes all existing tests.

---

## Commit 2 - Introduce Runtime State Facade

### Goal

Define explicit inputs/outputs between orchestrator and model internals.

### Changes

- Add minimal internal facade types (names can vary):
  - `RuntimeDeps` (context, cache manager access, formatting callback)
  - `RunResult` (generated string, flags for post-run)
- Keep actual logic in old methods; just formalize data passing.

### Acceptance Criteria

- Existing tests unchanged.
- No net behavior differences.

---

## Commit 3 - Extract Prompt Policy (Read-Only Move)

### Goal

Move prompt parsing/validation from `LlamaModel::formatPrompt` into policy class.

### Changes

- Add:
  - `addon/src/runtime/policies/prompt/PromptPolicy.hpp`
  - `addon/src/runtime/policies/prompt/PromptPolicy.cpp`
- Move logic for:
  - chat/tool extraction
  - media placeholder rules
  - tools_compact shape constraints
- Keep same errors/messages where possible.
- `LlamaModel` now delegates prompt resolution to policy.

### Acceptance Criteria

- Prompt-related unit tests still pass:
  - tools/tool-compact tests
  - text/multimodal prompt parsing tests

---

## Commit 4 - Extract Cache Session Policy

### Goal

Move session decision logic out of `LlamaModel::resolveChatAndTools`.

### Changes

- Add:
  - `addon/src/runtime/policies/cache/CacheSessionPolicy.hpp`
  - `addon/src/runtime/policies/cache/CacheSessionPolicy.cpp`
- Policy decides:
  - no cache key path
  - same key reuse path
  - switch key path
  - whether reset-after-inference should happen
- Keep `CacheManager` for file load/save/invalidate operations.

### Acceptance Criteria

- Cache behavior parity:
  - no-cache single-shot behavior
  - same-key continuation
  - key switching save/load behavior
  - save-to-disk semantics

---

## Commit 5 - Extract Generation Params Policy

### Goal

Isolate per-run generation override/restore orchestration.

### Changes

- Add:
  - `addon/src/runtime/policies/generation/GenerationParamsPolicy.hpp`
  - `addon/src/runtime/policies/generation/GenerationParamsPolicy.cpp`
- Wrap `applyGenerationParams(...)`/restore lifecycle in policy.
- Keep context-specific implementation in `TextLlmContext` and `MtmdLlmContext`.

### Acceptance Criteria

- Generation-params integration tests remain green.
- No regression in default sampling behavior when overrides are absent.

---

## Commit 6 - Extract Post-Run Policy

### Goal

Move end-of-run cleanup/persistence/trim logic into one place.

### Changes

- Add:
  - `addon/src/runtime/policies/postrun/PostRunPolicy.hpp`
  - `addon/src/runtime/policies/postrun/PostRunPolicy.cpp`
- Migrate logic for:
  - tools_compact post-generation trim checks
  - optional cache save
  - reset/no-reset decisions
  - debug stats boundary bookkeeping

### Acceptance Criteria

- tools_compact chain behavior unchanged.
- post-run cache persistence unchanged.

---

## Commit 7 - Introduce Model Profiles (Default + Qwen3)

### Goal

Separate model-family-specific behavior from generic runtime flow.

### Changes

- Add:
  - `addon/src/profile/ModelProfile.hpp`
  - `addon/src/profile/DefaultProfile.*`
  - `addon/src/profile/Qwen3Profile.*`
- Start by moving minimal responsibilities:
  - qwen3 detection/capability exposure
  - chat template selection hooks
- Keep reasoning token behavior where it currently lives for now.

### Acceptance Criteria

- Non-qwen models unaffected.
- Qwen3 template behavior unchanged.

---

## Commit 8 - Wire RunPipeline as Primary Orchestrator

### Goal

Switch `LlamaModel::processPromptImpl` to explicit pipeline stage calls.

### Changes

- `RunPipeline` becomes authoritative flow:
  1. cache policy prepare
  2. prompt policy resolve
  3. generation param policy apply
  4. eval
  5. generate/prefill branch
  6. post-run policy finalize
- Keep `LlamaModel` as lifecycle owner and adapter boundary.

### Acceptance Criteria

- Full test suite parity.
- Reduced complexity in `processPromptImpl`.

---

## Commit 9 - Context Folder Normalization (Optional)

### Goal

Perform file/folder moves only after stable behavior.

### Changes

- If desired, normalize paths:
  - keep `context/*` for context implementations
  - keep `runtime/*` for orchestration/policies
  - keep `profile/*` for model-specific behavior
- Update includes and CMake lists only.

### Acceptance Criteria

- No behavior change.
- Clean compile and test.

---

## Commit 10 - Cleanup + Dead Code Removal

### Goal

Remove obsolete wrappers and duplicated code paths.

### Changes

- Delete old helper methods in `LlamaModel` that are now policy-owned.
- Keep method names only where public/internal interfaces still require them.
- Add short architecture comments near main orchestration entrypoints.

### Acceptance Criteria

- No duplicate logic remains in old and new paths.
- Final behavior and tests still match baseline.

---

## Recommended Test Focus per Commit

- **Prompt policy commits:** tools, tools_compact, multimodal prompt parsing, malformed input errors.
- **Cache policy commits:** cache state machine tests, cache switching, save/load persistence, no-cache resets.
- **Post-run commits:** tools_compact trimming and boundary stats.
- **Pipeline wiring commits:** api behavior tests, single-job behavior, generation params, sliding context.

## Optional Stretch Commits (After Main Refactor)

- Move overflow/discard algorithm into dedicated overflow policy object shared by text + multimodal contexts.
- Move qwen3 reasoning EOS replacement behind profile hook interface.
- Add focused unit tests for each policy class independent of full model setup.

## Done Definition

- `LlamaModel` is no longer the monolithic runtime decision hub.
- Run-time branching is explicit and stage-based.
- Policy objects are testable in isolation.
- Existing public API and behavior remain compatible.

---

## Execution Tracks

This checklist supports two execution styles.

## Track A (Full) - 10 Commits

Use when:

- you want low-risk, highly reviewable slices
- you want clean bisectability
- multiple reviewers may inspect different parts

Sequence:

1. Commit 0 - Baseline Safety Net
2. Commit 1 - Runtime Scaffolding
3. Commit 2 - Runtime State Facade
4. Commit 3 - Prompt Policy
5. Commit 4 - Cache Session Policy
6. Commit 5 - Generation Params Policy
7. Commit 6 - Post-Run Policy
8. Commit 7 - Model Profiles
9. Commit 8 - Pipeline as Primary Orchestrator
10. Commit 10 - Cleanup + Dead Code Removal

Optional between 9 and 10:

- Commit 9 - Context Folder Normalization

Note: this is the default recommended path for safety and traceability.

## Track B (Compact) - 5 Commits

Use when:

- you prefer fewer rebases/cherry-picks
- a single owner is driving the refactor end-to-end
- you still want logical checkpoints without maximum granularity

### B1 - Foundation

Combines:

- Commit 0
- Commit 1
- Commit 2

Output:

- runtime skeleton + request/facade types in place, no behavior change

### B2 - Prompt + Cache Policies

Combines:

- Commit 3
- Commit 4

Output:

- prompt parsing/validation extracted
- cache session decisions extracted

### B3 - Generation + Post-Run Policies

Combines:

- Commit 5
- Commit 6

Output:

- generation override orchestration extracted
- finalize/trim/save/reset logic extracted

### B4 - Profiles + Pipeline Switch

Combines:

- Commit 7
- Commit 8

Output:

- model profile split introduced
- `RunPipeline` becomes primary orchestrator

### B5 - Cleanup (and Optional Moves)

Combines:

- Commit 10
- optional Commit 9

Output:

- dead code removed
- optional folder normalization completed

## Recommendation

- Prefer **Track A (10 commits)** for maintainability and safer rollback.
- Use **Track B (5 commits)** if you need faster delivery with fewer integration points.


