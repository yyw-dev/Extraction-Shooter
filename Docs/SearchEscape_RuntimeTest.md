# SearchEscape Runtime Test

Current map: `/Game/SearchEscape/Maps/M_SE_Test`

## Open And Play

1. Open `AGIS0_51.uproject` in Unreal Engine 5.7.
2. Open `/Game/SearchEscape/Maps/M_SE_Test` if it is not already open.
3. Press Play.

Expected startup:

- The default GameMode is `/Game/SearchEscape/Core/BP_SearchEscapeGameMode.BP_SearchEscapeGameMode_C`.
- The default pawn is `/Game/SearchEscape/Characters/BP_SE_Player.BP_SE_Player_C`.
- The default player controller is `/Game/SearchEscape/Core/PC_SearchEscape.PC_SearchEscape_C`.
- The player spawns at `SE_PlayerStart`.
- HUD shows health, gold, power, objective text, and a 5-minute timer.
- Runtime logic spawns 10 D-map-style chests, 5 D-map-style enemies, and 2 hidden escape doors from map marker tags.

## Controls

- `WASD`: move.
- Mouse: camera look.
- `Space`: jump.
- `Left Mouse Button`: AGIS template weapon/fire input.

## Gameplay Checks

Chest test:

- Walk next to a chest and stay close for 1 second.
- Gold should increase by a random value from 10 to 50.
- Leave before 1 second to confirm open progress cancels.
- Re-entering an opened chest should not give gold again.

Monster test:

- Approach a cone-shaped monster.
- It should patrol, detect, chase, and attack.
- Health should decrease when it attacks.
- Shoot or damage it until it dies. The migrated enemy handles UE damage through `TakeDamage`.
- Killing it should give `+20` gold and `+1` power.

Escape test:

- Play until the timer reaches `01:00`.
- Two escape doors become visible and active.
- Touch an active door to end with `Escape Successful`.

Failure tests:

- Let monsters reduce health to 0 to get `You Died`.
- Let the timer reach 0 without extracting to get `Time Out`.

## Command-Line Verification Already Run

```text
Build: succeeded for AGIS0_51Editor Win64 Development.
Map GameMode: /Game/SearchEscape/Core/BP_SearchEscapeGameMode.BP_SearchEscapeGameMode_C
Default pawn: /Game/SearchEscape/Characters/BP_SE_Player.BP_SE_Player_C
Default player controller: /Game/SearchEscape/Core/PC_SearchEscape.PC_SearchEscape_C
Chest marker tags: 10
Monster spawn tags: 5
Escape point tags: 2
```
