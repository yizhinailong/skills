# Skills

Reusable skills for AI coding assistants (Claude Code, OpenCode, Cursor, and other agents supported by the skills CLI).

## Available Skills

| Skill | Description |
|-------|-------------|
| [cpp-style-ref](skills/cpp-style-ref/) | C++ coding style reference for xmake C++20 modules and CMake + vcpkg header/source projects. Covers naming, module organization, class layout, initialization, error handling, ownership, and modern C++ practice rules. |

## Installation

Using the [skills CLI](https://www.npmjs.com/package/skills) (supports Claude Code, OpenCode, Cursor, and 40+ agents):

```bash
# Install all skills from this repo
npx skills add yizhinailong/skills

# Install a specific skill
npx skills add yizhinailong/skills --skill cpp-style-ref

# Install to specific agents
npx skills add yizhinailong/skills -a claude-code -a opencode

# Install globally (available across all projects)
npx skills add yizhinailong/skills -g
```

Or manually clone and copy the skill directories to your agent's skills folder (for example, `.claude/skills/` for Claude Code).

## License

[MIT](LICENSE)
