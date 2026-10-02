# Vorssaint — Linux / Hyprland rewrite

A C++20 + Qt / Quickshell rewrite of the useful desktop-utility parts of Vorssaint
for Linux systems running Hyprland.

This branch deliberately excludes macOS- and MacBook-specific behavior. The focus is
a small, hackable backend and a per-monitor Quickshell panel that can be extended
locally without dragging in distro-specific policy.

## Included

- Per-monitor Quickshell bar widget and popup panel.
- System overview using Linux `/proc` and filesystem information.
- Hyprland window list with focus/close controls.
- Numeric workspace creation/focus on the monitor whose widget was opened.
- Move the active Hyprland window to a numeric workspace.
- systemd service manager for user and system services.
  - fuzzy ranked search
  - start/stop/restart
  - enable/disable
  - system mutations authenticate with polkit via `pkexec`
- Settings GUI.
- XDG base-directory storage.
- URL tracking-parameter cleaner.
- `vorssaintctl doctor` for lightweight dependency checks.

## Security

Subprocesses are never constructed as shell strings. The C++ backend uses
`QProcess::setProgram()` + `setArguments()`, while Quickshell uses argv arrays.
There is no `sh -c`, `bash -c`, or `sudo`.

Window titles and application IDs are display-only. Service units/actions, monitor
names, and workspace IDs are validated at the backend boundary. Tests include
shell-metacharacter payloads to catch accidental regressions.

See [LINUX-PORT.md](LINUX-PORT.md) for architecture and integration notes.

## Build

Requirements:

- CMake 3.24+
- C++20 compiler
- Qt 6 Core development files
- Quickshell 0.3.x
- Hyprland
- systemd
- polkit (`pkexec`) for rootful service operations

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Run from the checkout:

```sh
PATH="$PWD/build:$PATH" qs -p ./quickshell/vorssaint
```

## Configuration

Settings are stored at:

```text
$XDG_CONFIG_HOME/vorssaint/settings.json
```

with the normal XDG fallbacks when variables are unset. Data, cache, and runtime
directories use the corresponding XDG base directories as well.

## Scope

This first Linux rewrite intentionally avoids deep integration with PipeWire,
package managers, hardware fan/brightness interfaces, portals, screen capture,
clipboard-history databases, and distro-specific helpers. Those can be added behind
the existing C++/Quickshell boundary without redesigning the shell UI.

The project remains GPL-3.0-or-later; see [LICENSE](LICENSE).
