# Vorssaint Linux / Hyprland port

This branch is a clean Linux rewrite of the useful cross-platform parts of Vorssaint.

It intentionally does **not** try to emulate macOS-only functionality such as the
notch/Dynamic Island, Dock/Finder integration, Apple fan/SMC APIs, MacBook battery
features, or macOS Accessibility hooks.

## Architecture

- **Quickshell / Qt Quick** owns the per-monitor bar widget and panel UI.
- **vorssaintctl** is a C++20 + Qt Core backend for XDG persistence, metrics,
  systemd operations, URL cleaning, Hyprland actions, and fuzzy ranking.
- Configuration lives in `$XDG_CONFIG_HOME/vorssaint/settings.json`.
- Mutable data/cache/runtime paths follow the corresponding XDG base directories.

The panel currently provides System, Windows, Services, Utilities, and Settings
pages. It is deliberately light on distro-specific integration so a local agent
can add hardware/audio/package-manager integrations later.

## Security model

No feature invokes `sh -c`, `bash -c`, or constructs a shell command string.
Every subprocess is launched as an executable plus an argv vector.

Untrusted strings such as window titles, service descriptions, search text, URLs,
and settings values are never interpreted as commands. systemd actions accept only
a fixed verb allow-list and validated `.service` unit names. Hyprland workspace
IDs are parsed as integers; monitor names are validated before being passed as an
argument. Rootful systemd operations use `pkexec systemctl ...` directly.

## Build

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the shell from the checkout with:

```sh
qs -p ./quickshell/vorssaint
```

or install it and point Quickshell at the installed configuration.

Runtime tools expected by the current feature set:

- Quickshell 0.3.x
- Hyprland / `hyprctl`
- systemd / `systemctl`
- polkit / `pkexec` for system-scope service mutations

`vorssaintctl doctor` reports which helpers are visible.
