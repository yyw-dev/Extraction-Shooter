import unreal


ROOT = "/Game/SearchEscape"

FOLDERS = [
    ROOT,
    f"{ROOT}/Core",
    f"{ROOT}/Characters",
    f"{ROOT}/AI",
    f"{ROOT}/Interactables",
    f"{ROOT}/UI",
    f"{ROOT}/Data",
    f"{ROOT}/Maps",
]

COPY_ASSETS = [
    ("/Game/INVENTORY/BP_ExampleCharacterAGIS", f"{ROOT}/Characters/BP_SE_Player"),
    ("/Game/INVENTORY/Core/Gamemode_AGIS", f"{ROOT}/Core/GM_SearchEscape"),
    ("/Game/INVENTORY/Core/PlayerController_AGIS", f"{ROOT}/Core/PC_SearchEscape"),
]

BLUEPRINT_ASSETS = [
    (f"{ROOT}/Core", "BP_SE_Manager", unreal.Actor),
    (f"{ROOT}/Characters", "BP_SE_Monster", unreal.Character),
    (f"{ROOT}/AI", "AIC_SE_Monster", unreal.AIController),
    (f"{ROOT}/Interactables", "BP_SE_Chest", unreal.Actor),
    (f"{ROOT}/Interactables", "BP_SE_EscapeDoor", unreal.Actor),
    (f"{ROOT}/Interactables", "BP_SE_SpawnPoint_Monster", unreal.Actor),
    (f"{ROOT}/Interactables", "BP_SE_Point_Chest", unreal.Actor),
    (f"{ROOT}/Interactables", "BP_SE_Point_Escape", unreal.Actor),
]


def log(message):
    unreal.log(f"[SearchEscape] {message}")


def asset_exists(asset_path):
    return unreal.EditorAssetLibrary.does_asset_exist(asset_path)


def make_folders():
    for folder in FOLDERS:
        unreal.EditorAssetLibrary.make_directory(folder)
        log(f"Ensured folder: {folder}")


def duplicate_asset(source_path, destination_path):
    if asset_exists(destination_path):
        log(f"Skipped existing asset: {destination_path}")
        return

    if not asset_exists(source_path):
        unreal.log_warning(f"[SearchEscape] Source asset missing: {source_path}")
        return

    ok = unreal.EditorAssetLibrary.duplicate_asset(source_path, destination_path)
    if ok:
        log(f"Duplicated {source_path} -> {destination_path}")
    else:
        unreal.log_warning(f"[SearchEscape] Failed to duplicate: {source_path}")


def create_blueprint(folder_path, asset_name, parent_class):
    asset_path = f"{folder_path}/{asset_name}"
    if asset_exists(asset_path):
        log(f"Skipped existing asset: {asset_path}")
        return

    factory = unreal.BlueprintFactory()
    try:
        factory.set_editor_property("parent_class", parent_class)
    except Exception:
        factory.set_editor_property("ParentClass", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(asset_name, folder_path, unreal.Blueprint, factory)
    if asset:
        log(f"Created blueprint: {asset_path}")
    else:
        unreal.log_warning(f"[SearchEscape] Failed to create blueprint: {asset_path}")


def create_widget_blueprint(asset_name):
    folder_path = f"{ROOT}/UI"
    asset_path = f"{folder_path}/{asset_name}"
    if asset_exists(asset_path):
        log(f"Skipped existing asset: {asset_path}")
        return

    if not hasattr(unreal, "WidgetBlueprintFactory"):
        unreal.log_warning(f"[SearchEscape] WidgetBlueprintFactory unavailable. Create manually: {asset_path}")
        return

    factory = unreal.WidgetBlueprintFactory()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(asset_name, folder_path, None, factory)
    if asset:
        log(f"Created widget blueprint: {asset_path}")
    else:
        unreal.log_warning(f"[SearchEscape] Failed to create widget blueprint: {asset_path}")


def create_ai_assets():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    blackboard_path = f"{ROOT}/AI/BB_SE_Monster"
    if not asset_exists(blackboard_path) and hasattr(unreal, "BlackboardDataFactory"):
        factory = unreal.BlackboardDataFactory()
        asset_tools.create_asset("BB_SE_Monster", f"{ROOT}/AI", unreal.BlackboardData, factory)
        log(f"Created blackboard: {blackboard_path}")

    behavior_tree_path = f"{ROOT}/AI/BT_SE_Monster"
    if not asset_exists(behavior_tree_path) and hasattr(unreal, "BehaviorTreeFactory"):
        factory = unreal.BehaviorTreeFactory()
        asset_tools.create_asset("BT_SE_Monster", f"{ROOT}/AI", unreal.BehaviorTree, factory)
        log(f"Created behavior tree: {behavior_tree_path}")


def main():
    make_folders()

    for source_path, destination_path in COPY_ASSETS:
        duplicate_asset(source_path, destination_path)

    for folder_path, asset_name, parent_class in BLUEPRINT_ASSETS:
        create_blueprint(folder_path, asset_name, parent_class)

    create_widget_blueprint("WB_SE_HUD")
    create_widget_blueprint("WB_SE_Result")
    create_ai_assets()

    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    log("Phase 1 asset scaffold complete.")


main()
