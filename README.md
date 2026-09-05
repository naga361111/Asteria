# Asteria

> **Co-op Adventurer Guild Hall Simulation** built with Unreal Engine 5 & C++.

Asteria is a cooperative simulation game set in a living fantasy world. Players run and manage an adventurer guild hall, cooperating with other players while autonomous adventurer NPCs accept quests from the guild board, embark on expeditions, and progress through ranks.

---

## 🌟 Key Features

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

---

## 🛠️ Tech Stack & Requirements

- **Engine**: Unreal Engine 5.8
- **Language**: C++20
- **Key Modules**: `AIModule`, `UMG`, `CoreUObject`, `Engine`
- **IDE**: Visual Studio 2022 / JetBrains Rider

---

## 📁 Project Structure

```
Asteria/
├── Source/Asteria/
│   ├── BTNodes/        # Behavior Tree tasks (quest evaluation & execution)
│   ├── Common/         # Common definitions (ERank, RequiredRankUpData)
│   ├── Debugger/       # Custom Gameplay Debugger categories
│   ├── GameMode/       # Server authority and session rules
│   ├── GameState/      # Replicated world and quest state
│   ├── Interaction/    # Interaction framework for actors
│   ├── NPC/            # Adventurer NPCs, AI controllers, and progression
│   ├── Player/         # Player characters and controller logic
│   ├── Quest/          # Quest board, quest objects, and state management
│   └── UI/             # UMG Slate widgets and TileView quest board UI
├── Docs/               # World setting and game design documentation
└── Asteria.uproject    # Unreal Engine project descriptor
```

---

## 🚀 Getting Started

1. **Clone the repository**:
   ```bash
   git clone https://github.com/naga361111/Asteria.git
   cd Asteria
   ```

2. **Generate Project Files**:
   - Right-click `Asteria.uproject` and select **Generate Visual Studio project files**.

3. **Build & Run**:
   - Open `Asteria.sln` in Visual Studio 2022 (or Rider).
   - Set build configuration to `Development Editor` and `Win64`.
   - Build and launch the project in the Unreal Editor.

---

## 📜 License

Private project. All rights reserved.
