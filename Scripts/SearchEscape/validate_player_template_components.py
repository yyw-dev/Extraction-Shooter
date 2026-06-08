import unreal


PLAYER_BP = "/Game/SearchEscape/Characters/BP_SE_Player"


def log(message):
    unreal.log(f"[SearchEscapePlayerTemplate] {message}")


def load_bp_class(asset_path):
    if hasattr(unreal.EditorAssetLibrary, "load_blueprint_class"):
        loaded = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        if loaded:
            return loaded

    asset_name = asset_path.rsplit("/", 1)[-1]
    return unreal.load_class(None, f"{asset_path}.{asset_name}_C")


def main():
    player_class = load_bp_class(PLAYER_BP)
    log(f"Player class: {player_class}")
    cdo = unreal.get_default_object(player_class)
    log(f"Player CDO: {cdo}")

    components = []
    try:
        components = cdo.get_components_by_class(unreal.ActorComponent)
    except Exception as exc:
        log(f"Could not enumerate components: {exc}")

    for component in components:
        name = component.get_name()
        class_name = component.get_class().get_name()
        log(f"Component: {name} class={class_name}")

    mesh = None
    try:
        mesh = cdo.get_editor_property("mesh")
    except Exception as exc:
        log(f"Could not read mesh property: {exc}")

    if mesh:
        try:
            log(f"Mesh skeletal mesh: {mesh.get_editor_property('skeletal_mesh')}")
        except Exception:
            pass
        try:
            log(f"Mesh anim class: {mesh.get_editor_property('anim_class')}")
        except Exception:
            pass


main()
