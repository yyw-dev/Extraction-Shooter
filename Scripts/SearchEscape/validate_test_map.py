import unreal


MAP_PATH = "/Game/SearchEscape/Maps/M_SE_Test"


def log(message):
    unreal.log(f"[SearchEscapeValidate] {message}")


def load_level():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    log(f"Loaded map: {MAP_PATH}")


def count_labels(prefix):
    world = unreal.EditorLevelLibrary.get_editor_world()
    count = 0
    for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
        try:
            if actor.get_actor_label().startswith(prefix):
                count += 1
        except Exception:
            pass
    return count


def main():
    load_level()
    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()
    game_mode = settings.get_editor_property("default_game_mode")

    log(f"WorldSettings default_game_mode: {game_mode}")
    log(f"Chest actors: {count_labels('SE_Chest_')}")
    log(f"Monster spawn point actors: {count_labels('SE_MonsterSpawn_')}")
    log(f"Escape point actors: {count_labels('SE_EscapePoint_')}")
    log(f"Stair step actors: {count_labels('SE_StairStep_')}")
    log(f"Nav obstacle actors: {count_labels('SE_NavObstacle_')}")
    log(f"Validation complete.")


main()
