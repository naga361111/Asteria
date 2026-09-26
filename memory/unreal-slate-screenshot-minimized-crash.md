---
name: unreal-slate-screenshot-minimized-crash
description: SlateInspector Screenshot / CaptureEditorImage on a minimized editor window crashes the editor once the window is restored; never restore the window yourself
metadata:
  node_type: memory
  type: feedback
  originSessionId: dfd2a010-cda8-4cda-a68a-d11a0aa33a7d
  modified: 2026-09-26T02:14:14.193Z
---

When the Unreal editor window is minimized, SlateInspector `Screenshot` and EditorAppToolset `CaptureEditorImage` return empty data, but they leave a pending window readback. When the window is restored, that readback runs with a bad size and the editor crashes (EXCEPTION_ACCESS_VIOLATION in `FD3D12DynamicRHI::RHIReadSurfaceData` from `FSlateRHIRenderer::DrawWindowViewport_RenderThread`). This happened on 2026-09-26. I restored the window with a Python ctypes `ShowWindow` call, and the user rejected that action.

**Why:** It caused an editor crash. Moving or restoring the user's editor window is also their call, not mine.

**How to apply:**
- If `SlateInspector.Windows` returns `[]`, or a capture fails with "Failed to capture any editor windows", the editor is probably minimized.
  - Stop all UI screenshots.
  - Ask the user to bring the window back. Never un-minimize or move the window from a script.
- Prefer `SlateInspector.Snapshot` (text tree) for reading UI. Use `CaptureViewport` for the 3D view. Keep window screenshots rare, and only take them when the window is visible.
- Related: [[unreal-mcp-set-transform-resets]], [[unreal-editor-python-console]].
