import unreal


MAP_PATH = "/Game/SearchEscape/Maps/M_SE_Test"
GAME_MODE_CLASS = "/Game/SearchEscape/Core/BP_SearchEscapeGameMode.BP_SearchEscapeGameMode_C"

MARKER_TAGS = (
    ("SE_Chest_", "SE.ChestMarker"),
    ("SE_MonsterSpawn_", "SE.MonsterSpawn"),
    ("SE_EscapePoint_", "SE.EscapePoint"),
)


def log(message):
    unreal.log(f"[SearchEscapePhase2] {message}")


def warn(message):
    unreal.log_warning(f"[SearchEscapePhase2] {message}")


def has_label(actor, prefix):
    try:
        return actor.get_actor_label().startswith(prefix)
    except Exception:
        return False


def add_tag(actor, tag_text):
    tags = list(actor.get_editor_property("tags"))
    existing = {str(tag) for tag in tags}
    if tag_text not in existing:
        try:
            tags.append(unreal.Name(tag_text))
        except Exception:
            tags.append(tag_text)
        actor.set_editor_property("tags", tags)
        return True
    return False


def tag_existing_markers():
    world = unreal.EditorLevelLibrary.get_editor_world()
    counts = {tag: 0 for _, tag in MARKER_TAGS}

    for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
        for prefix, tag in MARKER_TAGS:
            if has_label(actor, prefix):
                add_tag(actor, tag)
                counts[tag] += 1

    for tag, count in counts.items():
        log(f"Tagged {count} actors with {tag}")


def configure_game_mode():
    gm_class = unreal.load_class(None, GAME_MODE_CLASS)
    if not gm_class:
        warn(f"Could not load game mode class: {GAME_MODE_CLASS}")
        return False

    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()

    try:
        settings.set_editor_property("default_game_mode", gm_class)
        log(f"WorldSettings default_game_mode set to {GAME_MODE_CLASS}")
        return True
    except Exception as exc:
        warn(f"Could not set default_game_mode: {exc}")
        try:
            settings.set_editor_property("game_mode_override", gm_class)
            log(f"WorldSettings game_mode_override set to {GAME_MODE_CLASS}")
            return True
        except Exception as override_exc:
            warn(f"Could not set game_mode_override: {override_exc}")
            return False


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    tag_existing_markers()
    configure_game_mode()
    unreal.EditorLevelLibrary.save_current_level()
    log("Phase 2 runtime map configuration complete.")


main()
