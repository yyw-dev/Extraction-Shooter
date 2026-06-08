import unreal


MAP_PATH = "/Game/SearchEscape/Maps/M_SE_Test"
GAME_MODE_CLASS = "/Game/SearchEscape/Core/BP_SearchEscapeGameMode.BP_SearchEscapeGameMode_C"


def log(message):
    unreal.log(f"[SearchEscapePhase2Validate] {message}")


def count_tag(world, tag):
    actors = unreal.GameplayStatics.get_all_actors_with_tag(world, unreal.Name(tag))
    return len(actors)


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()
    game_mode = settings.get_editor_property("default_game_mode")

    expected_gm = unreal.load_class(None, GAME_MODE_CLASS)
    log(f"WorldSettings default_game_mode: {game_mode}")
    log(f"Expected game mode loaded: {expected_gm}")
    if expected_gm:
        cdo = unreal.get_default_object(expected_gm)
        log(f"GameMode default_pawn_class: {cdo.get_editor_property('default_pawn_class')}")
        log(f"GameMode player_controller_class: {cdo.get_editor_property('player_controller_class')}")
    log(f"Chest marker tags: {count_tag(world, 'SE.ChestMarker')}")
    log(f"Monster spawn tags: {count_tag(world, 'SE.MonsterSpawn')}")
    log(f"Escape point tags: {count_tag(world, 'SE.EscapePoint')}")
    log("Phase 2 validation complete.")


main()
