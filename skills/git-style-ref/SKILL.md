---
name: git-style-ref
description: "Git commit message style reference derived from mcr history. Use when drafting, reviewing, or creating commit messages in this style, including type and scope selection, English imperative subjects, explanatory bodies, and breaking changes. Preserve explicit user instructions and established repository conventions."
---

# git-style-ref

Write commit messages in the style established by `mcr`: an English Conventional Commit subject followed, when useful, by an explanatory body. Apply this reference when requested or when the target repository has no conflicting convention. This skill covers commit messages; it does not prescribe branch names, merge strategies, or release automation.

Read [reference.md](reference.md) when choosing between overlapping types, handling a multi-area change, or checking historical examples and exceptions. The reference is self-contained; using this skill does not require a checkout of `mcr`.

## Draft from the Selected Changes

Base the message on the changes being described. For a staged commit, inspect `git diff --cached`; use `git diff` for unstaged changes and inspect untracked files when they are part of the requested commit. Check nearby commit messages for the target area's vocabulary and scope names.

Identify the main purpose and any actual caller incompatibility before choosing a prefix. Describe the resulting change rather than listing files. When asked only for a message, return ready-to-use message text; when asked only for a subject, omit the body.

## Subject

```text
type(scope): imperative summary
```

- Use an English, lowercase type and an English imperative verb: `add`, `fix`, `prevent`, `remove`, `update`, `move`, `rename`, `consolidate`, `format`, or `bump`.
- Scope is optional: `feat: add JSON POST examples with six calling patterns` is a valid shape.
- Start the summary with a lowercase verb, preserve exact identifier casing such as `Result<T>`, `CurlContainer`, or `AGENTS.md`, and omit a final period.
- Keep the subject concise. The history establishes no strict character limit; move secondary details into the body rather than enforcing an invented cutoff.
- Use `!` only for a genuine breaking change, immediately before the colon: `refactor(api)!: ...` or `refactor!: ...`. A breaking-change footer without `!` is also established practice.

## Type and Scope

| Type | Use in this style |
| --- | --- |
| `feat` | Add a capability, API, module export, workflow, or feature example. |
| `fix` | Correct existing incorrect behavior; state the failure being prevented or corrected. |
| `refactor` | Reorganize, rename, move, or restyle existing code without adding a feature or correcting a bug. An API reorganization can still break compatibility. |
| `docs` | Change documentation, usage guidance, or explanatory comments. |
| `chore` | Maintain repository configuration, dependency bookkeeping, build artifacts, or release versions. |
| `style` | Make a dedicated formatting change; this type appears much less often than `refactor` for formatting. |

Choose the type from the actual purpose. In `mcr`, adding CI capability uses `feat(ci)`, and most code-formatting changes use `refactor`. Match relevant local history when these categories overlap. For new version-only commits, prefer `chore(release): bump version from X to Y`; historical version bumps labeled `feat` are exceptions rather than the default.

Use a scope that helps locate the responsibility: `api`, `session`, `coro`, `utils`, `options`, `tests`, `module`, `ci`, `build`, `deps`, or `release`. Exact filenames such as `AGENTS.md` and `build.mcpp` are also established scopes; preserve their spelling. Use the target project's own names rather than copying `mcr`-specific names. Omit scope when a change has no useful single area.

## Body

Separate the subject and body with a blank line. Use `- ` bullets for several concrete changes, or a short paragraph for a causal explanation. Small, self-explanatory changes can use only a subject, although bodies are common in the source history.

Explain meaningful behavior, motivation, implementation consequences, or migration steps that the subject cannot convey. Preserve technical names and indent wrapped bullet continuations. Include test or issue details only when supported by the actual work.

For a cohesive change spanning several areas, the history also uses secondary `type(scope): summary` headings inside the body, separated by blank lines. Use these only when they make the message easier to read; a normal bullet list is sufficient for most commits. If asked to organize commits, consider independent changes separately; a request for message text alone does not require changing the commit structure.

## Breaking Changes

Use `BREAKING CHANGE: ...` for a real caller incompatibility, explaining what previously worked, what changed, and how callers should adapt. For new messages, prefer placing this explanation after the descriptive body, separated by a blank line. The history also contains explanations at the start of the body.

```text
refactor!: remove cprtypes module and migrate to types module

BREAKING CHANGE: Remove the cprtypes module entirely and replace it
with the types module. Replace imports of mcr.cprtypes with mcr.types.
```

This shortened example follows a historical migration; use the names and impact of the actual change. Both `!` plus an explanation and an explanation without `!` occur in `mcr`.

Do not emit `BREAKING CHANGE: None`. Formatting, private implementation changes, or updates to internal tests alone do not establish a breaking public change. Determine compatibility from the actual API or supported behavior rather than copying a marker from a superficially similar commit.
