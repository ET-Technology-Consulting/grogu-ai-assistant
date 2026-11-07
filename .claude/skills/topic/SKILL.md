---
name: topic
description: Mandatory workflow for any new subject in the Grogu AI assistant repo (a feature, an improvement or a bug in the firmware, the hardware docs, the server or the AI pipeline) - GitHub issue in the « Grogu AI Assistant » Project, plan, worktree, build, compile checks, safety review, bench test on the real board by the human, then PR and Project status. Use when asked to add, change or fix something in this repo, to create its issues or tasks, for "take #12", or /topic. Not for a plain question, nor to continue a subject already in progress.
---

# Topic: a new subject on Grogu

One subject, from a GitHub issue to the pull request, with three human gates: the **issue definition**, the **plan** and the **bench test** on the real board. No code before the issue is defined, and nothing is pushed until the human has said the bench test is OK.

It is the `topic-github` workflow, adapted to a hardware project: the end-to-end test is a test on the ESP32 bench (or inside the plush), done by hand, and safety is about motors, power and the microphone as much as about code.

## Language and attribution

- Talk to the user in the language they use.
- **Everything written is in English**: code, comments, docs, issues and their comments, branch names, commits, PR titles and bodies. The repo and the video series are public and in English.
- Commits and PR titles follow **Conventional Commits**, scoped by area: `feat(esp32): …`, `fix(server): …`, `docs(readme): …`.
- **No AI attribution, anywhere**: no `Co-Authored-By` naming Claude, no "Generated with Claude Code", no mention of an AI assistant in commits, PRs, issues or comments.
- Commit as the ETTC identity (`etripoli@et-technology-consulting.com`); check `git config user.email` before the first commit.
- The user often dictates: part names (INMP441, MAX98357A, DRV8833, XIAO ESP32S3) can come out garbled. Check them against `esp32/README.md` and `pins.h` before writing them anywhere.
- A bare `@name` notifies that GitHub user: write one only for a real login you mean to notify.

## How work is organised on GitHub

- **Repo**: `ET-Technology-Consulting/grogu-ai-assistant`, **public**. Base branch and PR target: `main`.
- **Project**: « Grogu AI Assistant », number **12**, owned by `ET-Technology-Consulting`, private, linked to the repo (`https://github.com/orgs/ET-Technology-Consulting/projects/12`). It tracks the **build only**: electronics, firmware, server, AI pipeline, control interface. Filming and editing are tracked elsewhere, never in this repo or Project.
- **Issue types**: `Feature` (a parent, one capability you can show, usually one roadmap line of the README), `Task` and `Bug` (its sub-issues, each one reviewable change). A lone change with no parent is a `Task` or a `Bug`.
- **Dependencies** are `blocked by` relations, not sentences.
- **Labels**: one area label per issue: `area:hardware` (wiring, parts, power, enclosure), `area:firmware` (ESP32 and Arduino sketches), `area:tools` (scripts such as `grogu_serial.py`), `area:server` (event hub, WebSocket, APIs), `area:ai` (STT, LLM, TTS, prompts, tools), `area:interface` (control interface), `area:docs`. Create a missing one (`gh label create area:<x> -R ET-Technology-Consulting/grogu-ai-assistant`) rather than skipping it.
- **Status** (field `PVTSSF_lADOB7rGYc4Bqo2Uzho_FfQ`, project id `PVT_kwDOB7rGYc4Bqo2U`):

  | Status | Option id | Meaning |
  |---|---|---|
  | Backlog | `60f92ed9` | Not ready to start |
  | Todo | `295c355f` | Defined and ready |
  | In Progress | `e8707e8f` | Being built |
  | In Test | `601018b1` | PR open, waiting for merge |
  | Done | `e8b0bdaa` | Merged and checked on the board |

  If an id stops working, read them again with `gh project field-list 12 --owner ET-Technology-Consulting --format json`.

### Commands

Create an issue, typed, labelled, in the Project, optionally under a Feature:

```bash
gh issue create -R ET-Technology-Consulting/grogu-ai-assistant --type Task --title "<title>" \
  --body-file <file> --label area:<area> --project "Grogu AI Assistant" [--parent <feature number>]
```

Set the Status of an issue:

```bash
item=$(gh project item-list 12 --owner ET-Technology-Consulting --format json --limit 500 \
  --jq '.items[] | select(.content.number==<n>) | .id')
gh project item-edit --project-id PVT_kwDOB7rGYc4Bqo2U --id "$item" \
  --field-id PVTSSF_lADOB7rGYc4Bqo2Uzho_FfQ --single-select-option-id <option id>
```

A just-created issue can take a few seconds to show in `item-list`: retry rather than conclude it is missing.

Record a dependency (node ids from `gh issue view <n> --json id`):

```bash
gh api graphql -f query='mutation($a:ID!,$b:ID!){addBlockedBy(input:{issueId:$a,blockingIssueId:$b}){clientMutationId}}' -f a=<blocked> -f b=<blocking>
```

Read a Feature and its sub-issues:

```bash
gh api graphql -f query='query($n:Int!){repository(owner:"ET-Technology-Consulting",name:"grogu-ai-assistant"){issue(number:$n){title body subIssues(first:50){nodes{number title state issueType{name}}}}}}' -F n=<number>
```

## Scope of a run

A run delivers one **batch**: one issue, a few related issues, or a whole Feature in **goal mode** (its open sub-issues, one after the other, in sub-issue order, `blocked by` respected). One batch = one branch and one PR. If the user slips in an unrelated subject, note it and handle it afterwards, in its own batch.

## Progress

The steps carry these short names: Issues, Analysis, Plan, Worktree, Build, Checks, Safety, Bench, PR, Report.

At the start of each step, mark a chapter with `mcp__ccd_session__mark_chapter`, titled exactly `N/10 Step` (for example `8/10 Bench`), nothing else. If a step is blocked, do not change chapter: say in one sentence what blocks, with the output. If you go back (the bench test fails, you fix it in Build), mark the chapter of the step you return to. Without that tool, write `N/10 Step` as the first line of the message.

## Model per type of task

Values are the `model` and `effort` of the Agent tool.

| Type of task | Model |
|---|---|
| Read the code and docs, search existing issues | Haiku, low |
| Docs, README tables, wiring notes, copy | Haiku, low |
| Everyday firmware, tools and server code | Sonnet, medium |
| Anything that drives the motor or the amplifier, power and timing, audio streaming and buffers, the WebSocket protocol, the hub's decision rules, prompts and tool calling, secrets | Opus, high |
| A bug whose cause is unknown (resets, noise, no audio) | Opus, high |
| Safety review of the diff (step 7) | Opus, high |

The issue definition, the plan, the bench test, the commit and the PR stay in the main session. A sub-agent does not see the conversation (its brief gives the goal, files, what is done and excluded, what to return), asks the user nothing, writes nothing to GitHub, and its result is a hint: reread what it cites.

## 1. Issues (gate 1)

If `gh` is not authenticated, say so and stop.

**Numbers or a Feature given**: read them (`gh issue view <n> --json number,title,body,state,issueType,labels,comments,projectItems`, and the sub-issues query for a Feature). Their acceptance criteria are the contract.

**Free description**: search first, open and closed, with two sets of keywords (`gh issue list -R ET-Technology-Consulting/grogu-ai-assistant --state all --search "<words>"`), and look at the open Features of the Project. An open match: summarize it and check its criteria with the user. A closed match: never reopen it; a new issue cites it. Nothing matches: define the issue with the user **before any code**. Ask what you cannot know: expected behaviour, which board (bench DevKitC-1 or the XIAO inside the plush), what the motor, speaker and mic must do, what happens when the server is offline, what is out of scope. Draft in the format below, show it, wait for the OK, then create it: type, parent, area label, Project, `Status` `Todo`, `blocked by` relations. Give each number and link.

### Issue format

```
Title: <Area>: <the outcome, in one sentence>

As a <who>, I want <what>, so that <why>.

<Context: what happens today, how it fits with other issues (#n), and the technical anchor:
the file, function, pin or module it builds on, in backticks.>

**Acceptance criteria**

- [ ] <one observable behaviour, checkable on the bench, by a command or by reading a file>
- [ ] <the bench test that proves it works on the real board>
- [ ] <the doc to update: README, esp32/README.md, wiring.svg, a spec>

Out of scope: <one line>.
Shipped: `pending`
```

- The "who" is the maker (the user), someone reproducing the build from the repo, or a viewer of the series.
- A **bug** starts with the observed problem: `When <action> (<board, firmware, power>), <what happens> instead of <what should happen>.`, then the steps to reproduce and the serial output.
- A **Feature** keeps the capability as a whole; the detail lives in its sub-issues.
- **Definition of Ready**: user story (or observed problem), verifiable criteria, technical anchor checked in the real files, type, area label, Project and Status, out-of-scope line. An issue too big for one PR becomes a Feature with sub-issues.
- **Public repo**: no client names, no personal data, no real emails or calendar entries in an issue. Use demo data.

## 2. Analysis

1. Run `git status --short`. If the tree is not clean, show it and ask; never stash, discard or commit someone else's changes.
2. `git fetch origin --prune`.
3. Read the files the issues name and the docs of their area: `README.md`, `esp32/README.md`, `esp32/bringup/pins.h`, `arduino/motor/README.md`, `server/event-hub.md`. Follow the code path end to end.
4. **Bug**: reproduce it before planning (a serial log, a compile error, a measurement). If you cannot, say what you tried.
5. Note what the issue did not anticipate (a pin conflict, a power budget, a missing part) and ask instead of guessing.

## 3. Plan (gate 2)

A short plan: the approach in two or three sentences; the files to create or change; the wiring or parts change, if any; how each acceptance criterion will be checked (compile, script, bench step); the risks (motor stall, brown-out, speaker damage, mic privacy, latency); the model per block of work; in goal mode, the order of the sub-issues.

Post it as a comment on each issue (on the Feature in goal mode), show it, and **stop until the user says OK**. Then assign the issues (`--add-assignee @me`) and set them to `In Progress`.

## 4. Worktree

Always a worktree, without asking for the name. Branch `<type>/<short-kebab-slug>` in English, no issue number (`feat/websocket-audio-stream`, `fix/touch-false-trigger`):

```bash
git worktree add --no-track -b <type>/<slug> ../.worktrees/grogu-<slug> origin/main
```

Say the path and branch in one line. If the session already runs in an app-made worktree, use it. Check `git branch --show-current` before each commit.

## 5. Build

- **Bug**: reproduce first (a failing check or a serial log), then fix.
- Stay inside the plan; note anything else for the hand-off.
- **The pin map lives in `esp32/bringup/pins.h`** and is the same on both boards. A pin change updates `pins.h`, the wiring table of `esp32/README.md` and `esp32/wiring.svg` in the same PR.
- Firmware keeps working on both the DevKitC-1 N16R8 and the XIAO ESP32S3; avoid GPIO 35-37 (octal PSRAM on the DevKitC-1).
- Settings are named constants at the top of the sketch (`MOTOR_AUTO_STOP_MS`, `TOUCH_RATIO`, `MIC_SHIFT`), with a comment giving the unit.
- Secrets (Wi-Fi password, hub token) live in a git-ignored file (`secrets.h`, `.env`) with a committed `.example` next to it. Never in code.
- A change of behaviour updates the matching README section and command table in the same PR.

## 6. Checks

From the worktree, for what the batch touches:

```bash
# ESP32 firmware, both boards
arduino-cli compile -b "esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB" --build-path <scratchpad>/build-devkit esp32/bringup
arduino-cli compile -b esp32:esp32:XIAO_ESP32S3 --build-path <scratchpad>/build-xiao esp32/bringup
# Arduino motor test
arduino-cli compile -b arduino:avr:mega --build-path <scratchpad>/build-mega arduino/motor
# Python tools: syntax, without writing __pycache__ into the repo
python3 -c "import ast,sys; [ast.parse(open(f).read(), f) for f in sys.argv[1:]]" esp32/tools/*.py
```

Use `--build-path` in the scratchpad so no build output lands in the repo. A new sketch folder gets its own compile line here and in the README. A server, once it exists, brings its own lint and test commands: take them from its README or manifest, never from memory, and add them to this step.

No new compiler warning. On a failure, fix the cause and rerun; **after three failed attempts at the same failure, stop**, report what you tried and ask. A failure that `main` already has is not the batch's: propose a `Bug` issue.

Then walk the acceptance criteria one by one and say for each whether it is met and how you know. A criterion that only the bench can prove is checked at step 8.

## 7. Safety review of the diff

Review `git diff origin/main...HEAD` and check at least:

- **Motor**: every way to start it keeps the auto-stop; no blocking call (audio, recording, network) can outlast `MOTOR_AUTO_STOP_MS` with the motor running; it is off at boot and on any error.
- **Audio**: the volume starts low; nothing can drive the original 0.25 W speaker at full power.
- **Power**: everything stays on 5 V USB; no mains, no 12 V inside the plush; motor and amplifier on 5 V, mic on 3V3; bulk and ceramic capacitors kept in the wiring docs.
- **Microphone** (outside the bring-up firmware): it listens only after a head touch, and audio leaves the board only towards the local server.
- **Network**: the server binds `127.0.0.1` or the LAN, never the internet; the hub checks its shared token; transcripts and logs stay local.
- **Secrets and privacy**: no password, token, Wi-Fi name or personal data in code, logs, fixtures or docs; demo data only.
- **Actions**: tools are read-only by default; any action asks for explicit confirmation; Grogu never acts on an event.

Run the `security-review` skill as well when the diff touches the server or the AI pipeline. Fix what you find, then run step 6 again.

## 8. Bench test by the human (gate 3)

1. Check the board is connected (`arduino-cli board list`). **Ask before uploading**: an upload moves the motor and plays sound on real hardware. With the OK, upload from the worktree, for the board the user names:

   ```bash
   arduino-cli upload -b "<fqbn>" -p <port> --build-path <scratchpad>/build-<board> esp32/bringup
   ```

   Without a board, say so and give the user the command to run once it is plugged in.
2. Give the user a numbered checklist, one block per acceptance criterion: what to plug or watch, the command to send (`esp32/tools/.venv/bin/python esp32/tools/grogu_serial.py <keys>`, or the serial monitor at 115200 baud), and what they should see, hear or measure. Include at least one negative case: a stalled motor, no touch, server offline, a wrong command.
3. Run the scripted part yourself with `grogu_serial.py` when the board is on this Mac, and quote the output.
4. **Stop and wait for the user's verdict.** Read their terminal too (`read_terminal`, or ask them to paste it): resets (`rst:`, `Brownout`), `Guru Meditation`, `failed`, `no data`. A clean log does not say what they saw or heard: wait for their answer. Never treat silence as an OK. Not OK: get exactly what they saw, go back to step 5.

## 9. Commit, push, pull request

1. Show `git status`, stage only the batch's files: never `.env`, `secrets.h`, build output, `.venv/`, `__pycache__/`, `.omc/` or recordings.
2. Commit:
   ```
   <type>(<scope>): <imperative subject, lower case, no final period, under 72 chars>

   <why, not how>

   Refs: #12, #13
   ```
   `Refs:` lists every issue. **Never `Closes`, `Fixes` or `Resolves`**: closing happens at step 10, after the merge. No `Co-Authored-By`. Get it right the first time: rewriting a pushed message needs a force-push only the user may run.
3. `git push -u origin HEAD`. Never force-push, never push to `main`.
4. `gh pr create --base main --title "<same format>" --body-file <file>`. Body: **Context** (the problem, then `Refs:` with every issue), **Changes**, **Checks** (commands and results), **Bench test** (the checklist, marked OK, with the board used), **Safety** (outcome of step 7), **To review closely**. No AI attribution. Add the PR to the Project: `gh project item-add 12 --owner ET-Technology-Consulting --url <pr url>`.
5. On each issue: a hand-off comment (PR link, each criterion with its verdict and how it was checked, anything left out); `Status` to `In Test`; the `pending` of **Shipped** replaced by the PR link; the met criteria ticked. Never `Done` and never close before the merge.

## 10. Report and tidy up

Tell the user: the issues, the branch, the PR, the checks and their results, what is left. Propose a `Backlog` issue for anything noticed outside the scope.

Once the PR is merged (the user says so, or `gh pr view` shows it): `Status` to `Done` on each issue and the PR item, close each issue (`gh issue close <n> --reason completed --comment "Merged in <pr url>."`), and close a Feature whose last sub-issue just shipped. Tick the matching line of the README roadmap in the same or the next PR.

Then remove the worktree and its branch, after checking from the main folder: `git fetch --prune`; the work is in `origin/main` (`git branch -r --merged origin/main`, or an empty `git diff origin/main <branch>` after a squash); `git -C <worktree> status --short` is empty. Then `git worktree remove <path>` and `git branch -d <branch>` (`-D` after a squash, once the diff is empty). If a check fails, remove nothing: show the output and ask.

## Never, without the user's explicit approval in this conversation

- Write code before the issue is defined and the plan approved.
- Create, edit or close an issue, label, field or Project item the user has not seen and agreed to.
- Upload firmware to a board, or run a command that moves the motor or plays sound.
- Push to `main`, force-push, merge the PR, or close an issue before its PR is merged.
- Commit a secret, a recording, a real transcript or someone's personal data.
- Add filming or editing work to this repo or Project.
- Skip one of the three gates.
