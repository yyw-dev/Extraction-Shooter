import unreal


ROOT = "/Game/SearchEscape"
GM_BP_PATH = f"{ROOT}/Core/BP_SearchEscapeGameMode"
GM_BP_NAME = "BP_SearchEscapeGameMode"
GM_FOLDER = f"{ROOT}/Core"

SEARCH_ESCAPE_GM_CLASS = "/Script/AGIS0_51.SearchEscapeGameMode"
PLAYER_BP = f"{ROOT}/Characters/BP_SE_Player"
PC_BP = f"{ROOT}/Core/PC_SearchEscape"


def log(message):
    unreal.log(f"[SearchEscapeRuntimeGM] {message}")


def warn(message):
    unreal.log_warning(f"[SearchEscapeRuntimeGM] {message}")


def load_bp_class(asset_path):
    if hasattr(unreal.EditorAssetLibrary, "load_blueprint_class"):
        loaded = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        if loaded:
            return loaded

    asset_name = asset_path.rsplit("/", 1)[-1]
    return unreal.load_class(None, f"{asset_path}.{asset_name}_C")


def ensure_game_mode_blueprint():
    if unreal.EditorAssetLibrary.does_asset_exist(GM_BP_PATH):
        asset = unreal.EditorAssetLibrary.load_asset(GM_BP_PATH)
        log(f"Loaded existing game mode blueprint: {GM_BP_PATH}")
        return asset

    parent_class = unreal.load_class(None, SEARCH_ESCAPE_GM_CLASS)
    if not parent_class:
        raise RuntimeError(f"Could not load parent class: {SEARCH_ESCAPE_GM_CLASS}")

    factory = unreal.BlueprintFactory()
    try:
        factory.set_editor_property("parent_class", parent_class)
    except Exception:
        factory.set_editor_property("ParentClass", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(GM_BP_NAME, GM_FOLDER, unreal.Blueprint, factory)
    if not asset:
        raise RuntimeError(f"Could not create {GM_BP_PATH}")

    log(f"Created game mode blueprint: {GM_BP_PATH}")
    return asset


def set_default_property(cdo, name, value):
    try:
        cdo.set_editor_property(name, value)
        log(f"Set {name} -> {value}")
        return True
    except Exception as exc:
        warn(f"Could not set {name}: {exc}")
        return False


def main():
    asset = ensure_game_mode_blueprint()
    gm_class = load_bp_class(GM_BP_PATH)
    player_class = load_bp_class(PLAYER_BP)
    pc_class = load_bp_class(PC_BP)

    if not gm_class:
        raise RuntimeError(f"Could not load generated game mode class: {GM_BP_PATH}")
    if not player_class:
        raise RuntimeError(f"Could not load player class: {PLAYER_BP}")
    if not pc_class:
        raise RuntimeError(f"Could not load player controller class: {PC_BP}")

    cdo = unreal.get_default_object(gm_class)
    set_default_property(cdo, "default_pawn_class", player_class)
    set_default_property(cdo, "player_controller_class", pc_class)
    set_default_property(cdo, "bShowStartScreen", False)
    set_default_property(cdo, "bRequireAllEnemiesDeadToEscape", False)

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    log("Runtime game mode blueprint configuration complete.")


main()
