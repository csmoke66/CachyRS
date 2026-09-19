# UI Sandbox

Standalone RmlUi host for iterating on CachyRS overlay UI without the game.

## Build

From the CachyRS build tree:

```bash
cmake --build build --target ui-sandbox -j$(nproc)
```

## Run

```bash
./build/ui-sandbox/ui-sandbox
```

Optional:

```bash
./build/ui-sandbox/ui-sandbox --config /path/to/parent/of/rmlui/
```

By default the binary looks for `rmlui/` next to itself (build posts a symlink to `config/rmlui`).

## What it simulates

- Sample DOM tree (world / players / npcs / widgets) with live-updating values
- Periodic `LOG(...)` traffic including duplicate PLUGIN lines for collapse bubbles
- Widget-pick buttons log instead of arming the in-game overlay

Esc or window close quits.
