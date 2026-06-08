# SearchEscape Implementation Plan

Deadline: 2026-06-11 Thursday
Project: AGIS0_51, Unreal Engine 5.7

## Goal

Build a single-player third-person extraction game prototype for the course project.

Required gameplay loop:

1. Player explores the level.
2. Player opens chests to gain gold.
3. Monsters patrol, detect, chase, and attack the player.
4. Player can attack and kill monsters.
5. Killing a monster gives 20 gold and +1 combat power.
6. A 5-minute round timer is shown on the HUD.
7. Escape doors appear at 4 minutes after round start.
8. Touching an escape door before time runs out ends the game successfully.
9. Player death ends the game with a result screen.

## Current Asset Strategy

Use only the AGIS inventory template assets under `/Game/INVENTORY`.

Do not depend on `ModularSciFiStation`.

Recommended reuse:

- Player base: `/Game/INVENTORY/BP_ExampleCharacterAGIS`
- GameMode base reference: `/Game/INVENTORY/Core/Gamemode_AGIS`
- PlayerController base reference: `/Game/INVENTORY/Core/PlayerController_AGIS`
- Input context: `/Game/INVENTORY/Other/Inputs/IMC_AGIS_Character`
- Storage/chest reference: `/Game/INVENTORY/Interactables/Storages/BP_StorageBox`
- Enemy reference: `/Game/INVENTORY/Interactables/BP_EnemyPuppet`
- HUD style reference: `/Game/INVENTORY/UI/Widgets/Other/WB_DisplayHUD`
- Combat sounds/VFX: `/Game/INVENTORY/Other/Sounds/Combat`, `/Game/INVENTORY/Other/VFX`

## SearchEscape Directory Layout

```text
/Game/SearchEscape
  /Core
    GM_SearchEscape
    PC_SearchEscape
    BP_SE_Manager
  /Characters
    BP_SE_Player
    BP_SE_Monster
  /AI
    AIC_SE_Monster
    BB_SE_Monster
    BT_SE_Monster
  /Interactables
    BP_SE_Chest
    BP_SE_EscapeDoor
    BP_SE_SpawnPoint_Monster
    BP_SE_Point_Chest
    BP_SE_Point_Escape
  /UI
    WB_SE_HUD
    WB_SE_Result
  /Data
  /Maps
    M_SE_Test
```

## Runtime Implementation Snapshot

The current playable prototype is implemented by the `AGIS0_51` C++ module and a runtime GameMode blueprint:

- `/Game/SearchEscape/Core/BP_SearchEscapeGameMode`: Blueprint subclass of `ASearchEscapeGameMode`.
- Default pawn: `/Game/SearchEscape/Characters/BP_SE_Player`, copied from the AGIS template so movement animation and weapon/combat input stay on the original template path.
- Default player controller: `/Game/SearchEscape/Core/PC_SearchEscape`, copied from the AGIS template controller.
- `ASearchEscapeGameMode`: round timer, runtime spawning from map markers, escape activation, gold/kill/combat score, result state.
- `ASearchEscapeChest`: migrated from `D:\UE test\Map`; proximity open progress, leave-to-cancel behavior, lid opening visual, random 10-50 gold reward.
- `ASearchEscapeEnemyCharacter` and `ASearchEscapeEnemyAIController`: migrated from `D:\UE test\Map`; health component, overhead health bar, patrol, perception, chase, melee attack, knockback, death rewards.
- `ASearchEscapeDoor`: migrated from `D:\UE test\Map`; hidden until activated, overlap extraction.
- `USearchEscapeHUDWidget`: migrated from `D:\UE test\Map`; runtime UMG HUD with health, gold, kill count, combat score, timer, and result overlay.

The UMG widgets under `/Game/SearchEscape/UI` are kept as placeholders for later visual polish because UE 5.7 does not expose safe WidgetTree editing through Python commandlets.

## Monday, 2026-06-08

Target: Core project workspace and first playable shell.

- [x] Create `/Game/SearchEscape` directory layout.
- [x] Create/copy first phase blueprints.
- [x] Create `M_SE_Test` whitebox map.
- [x] Set `GM_SearchEscape` as map GameMode.
- [x] Set `BP_SE_Player` as default pawn.
- [x] Add player variables: `MaxHealth`, `Health`, `Gold`, `CombatPower`.
- [x] Add runtime UMG HUD with health, gold, and timer display.
- [x] Add runtime game-over/result overlay.

Done condition:

- Press Play.
- Player spawns.
- WSAD and mouse camera work.
- HUD shows health, gold, and a 5-minute timer.

## Tuesday, 2026-06-09

Target: Chest, gold, and player attack.

- [x] Implement runtime chest logic.
- [x] Add proximity trigger.
- [x] Start a 1-second open progress when the player stays nearby.
- [x] Cancel opening when the player leaves.
- [x] Give random 10-50 gold when opened.
- [x] Prevent repeated rewards with `bOpened`.
- [x] Update HUD immediately after gold changes.
- [x] Implement player attack with sweep trace.
- [x] Apply damage to monsters.
- [x] Place 10 chests in the test map.

Done condition:

- Player can open chests and gain gold.
- Leaving early cancels chest opening.
- Attack can damage a target.

## Wednesday, 2026-06-10

Target: Monster AI and escape flow.

- [x] Implement runtime monster health, damage, attack range, detect range.
- [x] Implement monster death reward: +20 gold and +1 combat power.
- [ ] Create `AIC_SE_Monster`.
- [ ] Create `BB_SE_Monster`.
- [ ] Create `BT_SE_Monster`.
- [ ] Behavior tree: patrol -> detect player -> chase -> attack.
- [x] Add `NavMeshBoundsVolume` to the test map.
- [x] Place 5 monster spawn points.
- [x] Implement runtime escape door.
- [x] Spawn or enable escape doors after 240 seconds.
- [x] Place 2 escape door points.

Done condition:

- Monsters patrol, chase, and attack.
- Player death shows result screen.
- Killing monsters gives rewards.
- Escape doors appear at minute 4.
- Touching an escape door ends the game successfully.

## Thursday, 2026-06-11

Target: Verification and course-ready polish.

- [ ] Verify every required item from the project brief.
- [ ] Fix NavMesh and collision stuck points in the test map.
- [ ] Add simple chest open sound.
- [ ] Add simple monster death sound.
- [ ] Add simple player damage screen flash if time allows.
- [ ] Save and compile all blueprints.
- [ ] Record or prepare demo flow.

## Must-Finish Checklist

- [x] Third-person movement and mouse look.
- [x] Player attack.
- [x] AGIS template player animation restored through `BP_SE_Player`.
- [x] AGIS template player controller restored through `PC_SearchEscape`.
- [x] 100m x 100m map placeholder or future map integration plan.
- [x] At least 1 ground layer and 1 upper structure.
- [x] 10 chests.
- [x] 5 monster spawn points.
- [x] 2 escape door points.
- [x] Monster patrol, detect, chase, attack.
- [x] Monster pathfinding around obstacles.
- [x] Monster damage reduces player health.
- [x] Player health 0 ends the game.
- [x] Chests open after 1 second near them.
- [x] Leaving chest range cancels opening.
- [x] Chests give 10-50 random gold.
- [x] Monster kill gives 20 gold and +1 combat power.
- [x] Escape doors appear at 4 minutes.
- [x] Touching escape door before time ends succeeds.
- [x] HUD health bar.
- [x] HUD gold count.
- [x] HUD 5-minute timer.
- [x] Result screen for escape/death/time out.
