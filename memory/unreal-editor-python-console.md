---
name: unreal-editor-python-console
description: "The MCP toolsets can't create landscapes or run arbitrary Python, but typing `py \"<script>\"` into the editor console (SlateInspector Type) runs full editor Python; Landscape panel settings live at /Engine/Transient.UISettings"
metadata:
  node_type: memory
  type: reference
  originSessionId: dfd2a010-cda8-4cda-a68a-d11a0aa33a7d
  modified: 2026-09-26T02:14:22.471Z
---

- The unreal-mcp `ProgrammaticToolset` sandbox only allows tool calls and a few stdlib modules. There is no `unreal` module and no console-exec tool.
- Workaround for full editor Python:
  1. Take a `SlateInspector.Snapshot` of the main window.
  2. Find the Output Log console textbox (label "Cmd"). The status-bar console box also works.
  3. Call `SlateInspector.Type` with `py "C:/abs/path/script.py"` and `submit: true`.
  4. The script can write JSON results to a file. It also works while the window is minimized (keystrokes are injected).
- Python has no API to create a Landscape. The existing-landscape functions (`landscape_import_heightmap_from_render_target` / `landscape_import_weightmap_from_render_target`) do exist.
- Creating a landscape therefore needs Landscape Mode → Manage → New. The form's backing object is `/Engine/Transient.UISettings` (class `LandscapeEditorObject`). `ObjectTools.set_properties` works on it:
  - `newLandscape_Material`
  - `newLandscape_*`
  - `importLandscape_HeightmapFilename`
  - `newLandscape_Layers` / `importLandscape_Layers`
- Related: [[unreal-slate-screenshot-minimized-crash]].
