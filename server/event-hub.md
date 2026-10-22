# Event hub

Every tool that needs my attention (Claude Code, local agents, scheduled tasks) sends its events to one hub running on the Mac mini. The hub decides what deserves to be said, and Grogu is the single output.

Everything runs locally. Nothing is exposed to the internet.

## Inputs

The hub has two entry points that feed the same pipeline.

| Entry point | For | Transport |
| --- | --- | --- |
| `POST /events` | Scripts, hooks, cron jobs | HTTP on `127.0.0.1` |
| MCP server | AI agents (Claude Code, Claude Desktop, local agents) | Streamable HTTP on `127.0.0.1` |

The MCP server runs inside the hub process rather than as a per-client stdio process, so every agent talks to the same queue and state.

Both entry points require a shared token, stored in each client's local config.

## Event format

```json
{
  "source": "mail-agent",
  "type": "mail.important",
  "priority": "normal",
  "title": "Important email from a client",
  "summary": "Reply drafted, waiting for review",
  "action": "Open the mail agent to review the draft",
  "link": "optional local URL or deep link",
  "dedupe_key": "optional, merges repeated events"
}
```

| Field | Required | Notes |
| --- | --- | --- |
| `source` | yes | Short id of the sender |
| `type` | yes | Dotted name, used by the rules |
| `priority` | yes | `low`, `normal`, `high`, `critical` |
| `title` | yes | One line, what Grogu says first |
| `summary` | no | Said only when I ask for details |
| `action` | no | What I should do next |
| `link` | no | Shown in the control interface |
| `dedupe_key` | no | Events with the same key within a window are merged |

## MCP tools

| Tool | Purpose |
| --- | --- |
| `notify(event)` | Send an event, same format as `POST /events` |
| `get_status()` | Grogu online or not, focus session running, quiet hours |
| `list_events(since, status)` | Recent events and what the hub did with them |

There is no direct `say` tool for agents: every message goes through the decision step, so no agent can make Grogu talk during a focus session.

## Decision

1. **Rules first**, deterministic and free. Examples: `critical` is always announced; `low` goes to the evening digest; nothing but `critical` during quiet hours.
2. **LLM triage** for everything the rules do not settle. A small, fast model picks one outcome:
   - `announce`: say it now;
   - `hold`: wait for the end of the current focus session;
   - `digest`: add it to the evening summary;
   - `drop`: ignore it.
3. Every decision is logged with its reason, so the rules can be tuned.

## Output

- Grogu moves, then says the title in one sentence.
- Touching the head right after an announcement reads the summary and the suggested action.
- Grogu announces and points to the right tool. He never acts on an event.

## Privacy

- Events, decisions and logs stay on the Mac mini.
- Emails are announced by sender category and subject, never read in full.
- Demo data only when filming.

## Milestones

1. Hub with `POST /events`, rules only, Grogu announces.
2. First real source: Claude Code hooks (task finished, waiting for approval).
3. MCP server with `notify`, `get_status`, `list_events`.
4. LLM triage and focus-session hold (with the pomodoro timer).
5. Remaining sources one by one: mail agent, scheduled tasks, production alerts.
