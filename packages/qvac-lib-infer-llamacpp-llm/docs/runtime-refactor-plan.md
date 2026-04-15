# Runtime Refactor Plan (Pipeline + Policies)

## Goal

Reduce monolithic branching in LLM execution by introducing a `runtime` orchestration layer with composable policy objects. Keep behavior compatible while making flow, ownership, and testability clearer.

## Current Problem

The current implementation correctly handles many scenarios, but feature axes are intertwined:

- cache on/off and cache session switching
- tools on/off
- tools compact mode
- qwen3-specific logic
- overflow sliding behavior
- text vs multimodal

These paths are currently spread across `LlamaModel`, `TextLlmContext`, `MtmdLlmContext`, and helper utilities, which makes runtime reasoning difficult.

## Runtime Perspective (Target Mental Model)

### Load Step

Input:

- model GGUF path
- optional mmproj path
- config map

Decisions resolved at load time:

- model profile (`qwen3` vs default) from GGUF metadata
- inference mode (`text` vs multimodal) from mmproj presence
- backend/device and parsed llama params
- chat template strategy and tools compact enablement rules
- context instantiation

Result:

- runtime is initialized with context + selected profile + policy wiring
- model is ready to accept `run` requests

### Run Step

Input:

- prompt/history messages
- run options (`prefill`, `generationParams`, `cacheKey`, `saveCacheToDisk`)

Flow:

1. resolve cache session (none/same/switch/new)
2. load cache from disk when required
3. parse and validate prompt (+ tools contract)
4. evaluate prompt tokens (prefill path included)
5. generate output (unless prefill-only)
6. post-run adjustments (tools compact trim, optional cache save, context reset decisions)

Result:

- streamed/generated output to caller
- updated in-memory context/cache
- optional updated cache file on disk

## Proposed Structure

Use `runtime` as orchestration layer (not `context`).

```txt
addon/src/
├── runtime/
│   ├── RunPipeline.*
│   ├── RunRequest.*
│   ├── policies/
│   │   ├── cache/
│   │   ├── prompt/
│   │   ├── generation/
│   │   └── postrun/
├── context/
│   ├── TextLlmContext.*
│   └── MtmdLlmContext.*
├── profile/
│   ├── ModelProfile.*
│   ├── Qwen3Profile.*
│   └── DefaultProfile.*
```

Notes:

- `context` remains low-level inference state and token/KV operations.
- `runtime` performs orchestration and composes policies.
- `profile` centralizes model-family-specific behavior (for now: qwen3/default).

## Why `runtime` Should Be Separate from `context`

`context` should stay focused on:

- tokenization/eval/generation primitives
- KV memory operations
- media handling for multimodal
- sampler and model-local state

`runtime` should own:

- cache session lifecycle decisions
- prompt contract validation and shaping
- when to call eval vs generate
- post-run persistence/trim/reset orchestration

Keeping orchestration outside `context` prevents reintroducing a monolith under a different filename.

## Pipeline and Policy Boundaries

### RunPipeline

Single entrypoint for one run request:

1. `CachePolicy::prepareSession(...)`
2. `PromptPolicy::resolvePrompt(...)`
3. `GenerationPolicy::applyOverrides(...)`
4. `Context::eval...(...)`
5. `Context::generate...(...)`
6. `PostRunPolicy::finalize(...)`

### Cache Policy

Owns session semantics:

- no `cacheKey`
- same `cacheKey`
- switching to different `cacheKey`
- loading from disk
- conditional save on completion

### Prompt Policy

Owns:

- message parsing and validation
- tools block extraction and validation
- tools compact prompt shape constraints
- preparation of context-ready chat/tools structures

### Generation Policy

Owns:

- per-run generation parameter override/restore
- generation mode branching (prefill-only vs full generation)

### Post-Run Policy

Owns:

- tools compact trimming decisions
- optional cache persistence
- reset/no-reset decisions for non-cached paths

### Model Profile

Owns model-family-specific concerns:

- template selection quirks
- qwen3-specific reasoning behavior hooks
- model capability flags used by policies

## Migration Plan (Incremental)

### Phase 1: Introduce Runtime Skeleton

- add `runtime/RunPipeline` and minimal `RunRequest`
- keep existing behavior and call existing methods internally
- no logic rewrite yet, only orchestration extraction

### Phase 2: Extract Prompt Policy

- move prompt parse/validation logic from `LlamaModel::formatPrompt`
- keep same errors/messages as much as possible
- preserve current tools/tools_compact constraints

### Phase 3: Extract Cache Policy

- move session handling decisions around `CacheManager`
- preserve load/save/switch behavior
- keep `CacheManager` as low-level persistence engine initially

### Phase 4: Extract Post-Run Policy

- move tools compact post-generation trim logic
- move finalize/reset/save flow out of `LlamaModel::processPromptImpl`

### Phase 5: Introduce Model Profiles

- add `DefaultProfile` and `Qwen3Profile`
- migrate qwen3-specific checks/template/reasoning hooks behind profile

### Phase 6: Optional Folder Reorganization

- physically move files only after behavior stabilizes and tests pass
- avoid large path churn early in refactor

## Testing Strategy

Run existing tests after each phase and add targeted policy-level unit tests:

- cache state transitions (none/same/switch/save)
- tools compact prompt contract acceptance/rejection
- tools compact trim behavior end-of-chain
- qwen3 vs non-qwen3 behavior gating
- overflow sliding behavior under constrained context
- text vs multimodal run parity

## Compatibility Constraints

- preserve public JS API shape (`load`, `run`, run options)
- preserve current default semantics
- preserve current error behavior where practical
- keep migration incremental to reduce regression surface

## Non-Goals (Initial Refactor)

- changing user-facing API contracts
- changing cache file format
- changing model loading backend logic
- introducing new feature flags

## Success Criteria

- major run-time branches become explicit in pipeline stages
- model-family-specific behavior no longer leaks across generic flow
- cache/prompt/postrun rules are testable in isolation
- core `LlamaModel` orchestration complexity is substantially reduced

