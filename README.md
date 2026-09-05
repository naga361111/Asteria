# Asteria

> **Co-op Adventurer Guild Hall Simulation** built with Unreal Engine 5 & C++.

Asteria is a cooperative simulation game set in a living fantasy world. Players run and manage an adventurer guild hall, cooperating with other players while autonomous adventurer NPCs accept quests from the guild board, embark on expeditions, and progress through ranks.

---

## Key Features

- **Co-op Guild Simulation**: Collaborate with players to manage guild quests, adventurer contracts, and hall operations.
- **Server-Authoritative Architecture**: Multiplayer-first design where the host/server owns authoritative gameplay state and replicates data to clients with strict input validation.
- **Dynamic Quest & Rank System**:
  - 7-tier ranking structure from `F` to `S`.
  - Quest acceptance validation based on NPC rank requirements.
  - Rank-up progression tracked through quest completion count data assets.
- **Autonomous NPC AI**:
  - Behavior Tree and utility-driven NPC logic (`BTTask_GetQuest`, `BTTask_DoQuest`).
  - NPCs autonomously inspect the quest board, accept matching contracts, and carry out quests.
- **Custom Gameplay Debugger**: Real-time in-game debugging category (`GameplayDebuggerCategoryQuest`) for inspecting quest states, NPC assignments, and rank data.
