---
name: unreal-edit-copy-python-crash
description: "Running `EDIT COPY` via Python execute_console_command(None, ...) crashes the editor (UWorld::GetLevel null world) — never do it"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 9dac531a-4778-435d-b32d-3976a305f273
  modified: 2026-09-30T05:17:24.113Z
---

Calling `unreal.SystemLibrary.execute_console_command(None, "EDIT COPY")` from editor Python crashed UE 5.8 with an access violation in `CanCopySelectedActorsToClipboard` → `UWorld::GetLevel`. The world context was null. The level's unsaved changes were lost. (2026-09-30)

**Why:** Exec_Edit needs a valid world. Passing None means there isn't one, so it dereferences null.

**How to apply:** Don't read actor state as T3D through the clipboard. Read properties directly: compare `get_editor_property` against the CDO/archetype, or use ObjectTools.get_properties. Before running any risky editor script, ask the user to save first. Related: [[unreal-editor-python-console]].
