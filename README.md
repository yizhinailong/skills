# Skills

Zig programming skills for AI coding assistants (Claude Code, OpenCode, etc.).

## Available Skills

| Skill | Version | Description |
|-------|---------|-------------|
| [zig-0.15](skills/zig-0.15/) | 0.15.2 | Zig 0.15.x API guidance — ArrayList, std.Io.Writer/Reader, HTTP client, JSON, and more. Prevents outdated 0.11–0.14 patterns. |
| [zig-0.16](skills/zig-0.16/) | 0.16-dev | Porting notes for Zig 0.16-dev (std.Io era). Use when migrating 0.15 code to 0.16 master/dev. |
| [zig-memory](skills/zig-memory/) | 1.0.0 | Zig memory management — defer/errdefer patterns, allocators, leak detection. Targets Zig 0.15.x. |

## Installation

Using the [skills CLI](https://www.npmjs.com/package/skills) (supports Claude Code, OpenCode, Cursor, and 40+ agents):

```bash
# Install all skills from this repo
npx skills add yizhinailong/skills

# Install a specific skill
npx skills add yizhinailong/skills --skill zig-0.15

# Install to a specific agent
npx skills add yizhinailong/skills -a claude-code -a opencode

# Install globally (available across all projects)
npx skills add yizhinailong/skills -g
```

Or manually clone and copy the skill directories to your agent's skills folder (e.g., `.claude/skills/` for Claude Code).

## License

[MIT](LICENSE)
