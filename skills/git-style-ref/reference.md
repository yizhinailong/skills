# Commit Style Evidence from mcr

This reference was derived from all 116 commits reachable from `mcr` HEAD at `1c1de309f067eace00eaad563c5ddccd9fb6dae2`. Commit hashes below identify the source examples. They document a historical snapshot, not requirements to retain obsolete module names or implementation behavior.

## Observed Conventions

- 115 subjects use `type: summary`, `type(scope): summary`, or the breaking variant with `!`. The initial `Initial commit` is the only exception.
- All subjects are English. Types are lowercase; summaries start with imperative verbs and have no final period.
- 29 conventionally prefixed subjects omit scope. Scopes can name responsibilities, modules, or exact files; they are not restricted to lowercase identifiers.
- 113 commits have a body. Bulleted descriptions are common, alongside short prose explanations. Some bodies contain additional typed headings for related areas.
- Nine subjects exceed 72 characters; the longest is 81. The history does not justify a strict 50- or 72-character rejection rule.
- Breaking changes appear both with and without `!` in the subject. Two subjects use `refactor!`; explanations use `BREAKING CHANGE:` in the body.

| Subject type | Count | Interpretation |
| --- | ---: | --- |
| `feat` | 63 | Features, module exports, CI capability, and examples; also a few inconsistent maintenance labels. |
| `refactor` | 34 | Structural changes, naming, module organization, and code formatting. |
| `docs` | 9 | Documentation and repository guidance. |
| `chore` | 7 | Five release bumps, dependency bookkeeping, and build housekeeping. |
| `fix` | 1 | A concrete session lifetime bug. |
| `style` | 1 | An early formatting/configuration change. |

These are observed types, not a closed vocabulary for every target repository. Types such as `test`, `perf`, `build`, and `ci` are not evidenced as subject types in this snapshot; `tests`, `build`, and `ci` do appear as scopes. Follow explicit or established target-repository conventions when they differ.

## Representative Subjects

The following subjects are copied from the source history.

| Commit | Subject |
| --- | --- |
| `62b68fb` | `feat(session): add native CA store support on Windows` |
| `835e0c4` | `feat(api): add [[nodiscard]] attribute to all request methods` |
| `e3b59d3` | `feat(ci): add automated version tagging and Windows CI workflows` |
| `3dfeb93` | `feat: add JSON POST examples with six calling patterns` |
| `39110bb` | `fix(session): prevent premature node deallocation in list reset` |
| `3000a7a` | `refactor(module): update C++20 module syntax from dot notation to colon` |
| `2e7bc89` | `refactor(api): format function calls with multiline syntax` |
| `26cc71a` | `docs(AGENTS.md): update module organization documentation` |
| `1c1de30` | `chore(release): bump version from 0.2.0 to 0.2.1` |
| `54fc9c6` | `chore(deps): remove mcpp.lock file` |
| `5c14ab0` | `chore(build): add mcpp.lock to .gitignore` |
| `fa1fbed` | `style: format mcpp.toml and update .gitignore` |

Use feature-oriented `feat` for a new capability even when it lives in CI or examples. Use `fix` when correcting a demonstrated failure. Scope identifies where the change belongs; it does not determine the type.

## Complete Body Examples

### Causal Bug Explanation

Commit `39110bb`, verbatim:

```text
fix(session): prevent premature node deallocation in list reset

Appending keeps the current head, so only a fresh first node needs adopting;
resetting unconditionally would free the node it is about to adopt.
```

The body explains the ownership failure and why the correction works. A prose paragraph is appropriate when the causal chain matters more than a list of edits.

### Bulleted Implementation Detail

Commit `2e7bc89`, verbatim:

```text
refactor(api): format function calls with multiline syntax

- Break down long lambda expressions in apply_options function
  across multiple lines for better readability
- Format std::apply call with proper indentation and line breaks
- Reformat request_async function to use multiline syntax
  for improved code clarity
```

This is a direct example of `refactor` being used for formatting. Do not mechanically replace it with `style` when following this history.

### Release Housekeeping

Commit `1c1de30`, verbatim:

```text
chore(release): bump version from 0.2.0 to 0.2.1

- Update mcpp.toml version field from 0.2.0 to 0.2.1
```

Use actual versions and the target project's version file. The source does not establish a release process for other repositories.

### Breaking API Migration

Commit `4b5e75b`, verbatim:

```text
refactor!: remove cprtypes module and migrate to types module

BREAKING CHANGE: Remove the cprtypes module entirely and replace
it with the new types module. This change affects the main module
export structure where mcr.cprtypes has been replaced with mcr.types.
Users importing mcr.cprtypes will need to update their imports to
use mcr.types instead. The functionality has been consolidated into
the new types module.
```

This provides an old-to-new migration, rather than only saying that a change is breaking. Commit `e74697d` similarly explains the migration to `Result<T>` return values while using `feat(api):` without `!` in its subject. The new-message recommendation to place breaking explanations after other body text is a formatting preference, not a claim that all historical messages do so.

## Multiple Typed Sections

Commits such as `835e0c4` and `6d5657d` describe a primary change and then use additional typed headings in the body. For example, `835e0c4` has a `feat(api)` subject and later body headings for `docs`, `refactor(ssl)`, `refactor(options)`, and `refactor(utils)`.

The following is an illustrative template, not a verbatim historical commit:

```text
refactor(api): consolidate request option handling

- Share option application between related request entry points

refactor(tests): align request fixtures with shared option handling

- Update fixtures to exercise the shared path

docs: update request option examples

- Describe the consolidated behavior
```

Use this shape for related consequences of one purpose. It does not require multiple headings for a small change or combining unrelated work into one commit.

## Historical Exceptions to Avoid Copying

- `9f06a26` includes `BREAKING CHANGE: None` for formatting. Omit the marker when no real incompatibility exists.
- `fbc4ecb` marks test import consolidation as breaking, and `6c97f9b` marks an internal destructor call change as breaking. Those descriptions alone do not establish an external compatibility break; examine the actual supported interface before applying the marker to new work.
- `113d1b4` uses `feat` for a version-only bump. Prefer the repeated `chore(release)` pattern for that purpose.
- `22c6c8c` and `122019d` use `feat` for adding `.clang-format`. They do not establish a general rule that configuration maintenance is a feature.
- `26cc71a` contains a secondary `ref(build.mcpp)` heading. The repeated spelling is `refactor`; do not introduce `ref` as a standard type on this evidence.
- Older scopes such as `mr` and `cprtypes` reflect past project names. Use current responsibility names rather than preserving obsolete scopes.

The source `AGENTS.md` also explicitly asks for short, imperative descriptions with prefixes such as `feat:` and `style:`. The recommended rules in `SKILL.md` retain those recurring conventions while avoiding incidental inconsistencies.
