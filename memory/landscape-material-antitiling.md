---
name: landscape-material-antitiling
description: "Design map landscape uses its own anti-tiling copies (MI/M/MF_Landscape_*_Village); grid lines on flat z=0 ground in editor captures are the editor viewport grid, not the material"
metadata:
  node_type: memory
  type: project
  originSessionId: dfd2a010-cda8-4cda-a68a-d11a0aa33a7d
  modified: 2026-09-26T06:31:44.807Z
---

On 2026-09-26 the Design map landscape (`Landscape_Village`) was switched to `/Game/Map/Environment/Materials/MI_Landscape_Village`. The pack's `M_Landscape` / `MF_Landscape_Layer_Base` / `MI_Landscape` were not modified.

- **Copies:** `M_Landscape_Village` and `MF_Landscape_Layer_Village` are copies of the pack originals.
- **Anti-tiling changes:**
  1. Larger tile sizes (MI scalars `L# UV Near/Distance Tilling`, "repeats per metre").
  2. Macro variation in the master. World-space `LowResBlurredNoise` at 37 m and 131 m tints the final BaseColor. Params: `Macro Scale A/B`, `Macro Contrast`, `Macro Dark/Bright Tint`.
  3. Hex tiling in the layer function. It uses BaseMaterial `MF_ResolveCoordinateFrame_HexTile_DitherUV` on TexCoord0, which is the landscape layer coords, with explicit-derivative sampling. It is toggled per layer by static switches `L1…L6/Slope Hex Tiling`; on for L1, L2, L4.
- **Static-bool input needs a default:** a StaticBool function input needs a StaticBool node on its Preview pin. Otherwise the material fails with "Missing Preview connection" while any call site is unconnected.

**Why:** Thin 1 m grid lines appeared on the flat village plaza (z = 0) in editor captures. It took long to trace. They were the level editor's perspective grid (`ShowFlag.Grid`), drawn on the z = 0 plane. They are editor-only and not a shader bug.

**How to apply:**
- If grid lines show on ground at exactly z = 0 in the editor, check `ShowFlag.Grid 0` first.
- Tune the look through `MI_Landscape_Village` params.
- For PCG / landscape layout see [[unreal-editor-python-console]].
