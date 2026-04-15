# Compaction Policy Migration Plan

## Goal

Move `DynamicToolsState` ownership out of base context/runtime flow and isolate tools-compaction behavior behind a Qwen3-specific policy implementation. Base logic should not depend on Qwen3 or tools-compact internals.

## Scope

- Introduce a runtime compaction-policy abstraction.
- Keep tokenizer-specific mechanics in `TextLlmContext` and `MtmdLlmContext`.
- Move compaction state and decisions into policy implementations.
- Remove `DynamicToolsState` from `LlmContext`.
- Preserve behavior, error text, and public API semantics.

## Ownership Boundary (Mechanism vs Policy)

### Common Mechanism (Base Context / Runtime)

These are generic capabilities and should remain common:

- token/KV memory operations (`removeLastNTokens`, context slide primitives)
- context token counters (`nPast`, `firstMsgTokens`, discard configuration)
- optional generic helpers for safe trimming execution

### Policy-Specific State (Compaction Policy)

These should be owned by tools-compaction policy implementations:

- `nPastBeforeTools` (anchor semantics)
- `conversationOnlyTokens` as currently defined for tools-anchor math
- degenerate-boundary handling decisions
- post-generation trim eligibility based on compact-tools output semantics

Design rule:

- Base code provides *how to trim* primitives.
- Policy computes *when/what/how much to trim*.

## Non-Goals

- No user-facing API changes.
- No cache file format changes.
- No model-loading backend changes.

## Baseline Validation Set

Run after each migration phase:

- `TextLlmContextTest.*`
- `TextLlmContextQwen3Test.*`
- `CacheManagementQwen3Test.*`
- `ModelToolsQwen3Test.*`
- `LlmContextBaseTest.*`

## Migration Steps

### 1) Add Compaction Policy Types (No Behavior Change)

Create:

- `addon/src/runtime/policies/compaction/CompactionPolicy.hpp`
- `addon/src/runtime/policies/compaction/NoopCompactionPolicy.*`
- `addon/src/runtime/policies/compaction/Qwen3ToolsCompactPolicy.*`

Policy responsibilities:

- run-start/reset boundary handling
- conversation-only token accounting hooks
- discard clamping and post-slide adjustment hooks
- post-generation trim eligibility helpers
- debug-boundary exposure for runtime stats

Also wire CMake/test targets for the new files.

Acceptance:

- clean compile
- no runtime behavior change

### 2) Bind Policy via Profile + Config

In model init, select policy:

- `profile.supportsToolsCompact && tools_compact=true` -> `Qwen3ToolsCompactPolicy`
- otherwise -> `NoopCompactionPolicy`

Pass policy through runtime deps/request. Keep current `DynamicToolsState` path active temporarily during migration.

Acceptance:

- model load + run flow unchanged

### 3) Move Runtime Gating to Policy

Replace context-state checks with policy checks in:

- `PromptPolicy` (tools_compact shape validation gate)
- `RunPipeline` (output-capture gate)
- `PostRunPolicy` (trim boundary and eligibility checks)

Acceptance:

- prompt/run/postrun behavior parity for compact and non-compact runs

### 4) Migrate Text Context Hooks

In `TextLlmContext`, replace direct `dynamicToolsState()` calls with policy hooks:

- run-start reset
- conversation-only token recording
- discard clamp and post-slide adjustment
- boundary recording
- degenerate-boundary fallback behavior

Acceptance:

- `TextLlmContext*` and Qwen3 compact tests remain stable

### 5) Migrate Multimodal Context Hooks

Apply equivalent migration in `MtmdLlmContext` while keeping mtmd tokenization specifics local.

Acceptance:

- multimodal behavior parity and stable tests

### 6) Remove `DynamicToolsState` From Base Types

- remove `DynamicToolsState` class from `LlmContext.hpp`
- remove `dynamicToolsState()` accessors and remaining references
- update `LlamaModel::getNPastBeforeTools` and debug stats to read from policy
- replace old state tests with policy-level tests

Acceptance:

- base context/runtime flow no longer has Qwen3 tools-compact state ownership
- base context still exposes generic trimming mechanisms only

### 7) Add Focused Policy Tests

Add unit tests for:

- boundary recording and degenerate cases
- discard clamp + slide adjustment
- post-run trim eligibility
- noop policy identity behavior

Acceptance:

- policy behavior verified in isolation
- existing integration behavior preserved

## Suggested Commit Sequence

1. `runtime: add compaction policy interface and implementations`
2. `runtime: bind compaction policy from profile and config`
3. `runtime: route prompt/run/postrun gating through compaction policy`
4. `context[text]: replace dynamicToolsState with policy hooks`
5. `context[mtmd]: replace dynamicToolsState with policy hooks`
6. `cleanup: remove DynamicToolsState from LlmContext and update debug path`
7. `test: add compaction policy unit coverage`

## Risk Controls

- Keep migration incremental (no big-bang rewrite).
- Preserve existing error text where tests may assert exact messages.
- Re-run Qwen3 compact/cache suites at each phase.
- Avoid mixing behavior changes with structural moves.

