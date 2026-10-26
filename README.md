# 2.20-World-Log-Server-Source

> **Archived (2026-10-26).** The 2.20 servers are being recreated as two binaries, Login and World. This repo is kept as found in [2.20-Reference](https://github.com/Original-Helbreath/2.20-Reference) (`as-found/servers/2.20-World-Log-Server-Source/`); its open issues moved to `2.20-Login-Server-Source`.

Source for the Helbreath World Log server - to be recreated.

## Contents

No original source exists. `original` holds the Ghidra pseudo-C decompile of the official `WorldServer.exe`
as the starting point. The server is to be recreated from the analysis in 2.20-Documentation
(`research/servers/world-log-server.md`, `official-worldserver.md`); that work is on the project board.

## Branches

| Branch | Purpose |
|---|---|
| `original` | The files exactly as found. Read-only by convention: never commit here. (It is not protected yet, because the org is on GitHub Free and its repos are private.) |
| `development` | Integration branch. Work happens in feature branches, which merge into `development`. |
| `stable` | Default branch. `development` is merged here once it is stable. |

## Project

Part of [Original Helbreath](https://github.com/Original-Helbreath): keeping the original 1999-2005 Siementech Helbreath alive.
Repos are named `<version>-<Part>`; other versions follow the same pattern.
Issues are tracked on the [project board](https://github.com/Original-Helbreath/projects/1). The analysis behind everything is in
[2.20-Documentation](https://github.com/Original-Helbreath/2.20-Documentation).
