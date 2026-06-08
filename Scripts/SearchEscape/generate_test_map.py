import math
import unreal


ROOT = "/Game/SearchEscape"
MAP_PATH = f"{ROOT}/Maps/M_SE_Test"


def log(message):
    unreal.log(f"[SearchEscapeMap] {message}")


def warn(message):
    unreal.log_warning(f"[SearchEscapeMap] {message}")


def asset_exists(asset_path):
    return unreal.EditorAssetLibrary.does_asset_exist(asset_path)


def load_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        warn(f"Missing asset: {asset_path}")
    return asset


def load_bp_class(asset_path):
    if hasattr(unreal.EditorAssetLibrary, "load_blueprint_class"):
        loaded = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        if loaded:
            return loaded

    asset_name = asset_path.rsplit("/", 1)[-1]
    return unreal.load_class(None, f"{asset_path}.{asset_name}_C")


def set_prop(obj, prop_name, value):
    try:
        obj.set_editor_property(prop_name, value)
        log(f"Set {obj.get_name()}.{prop_name}")
        return True
    except Exception as exc:
        warn(f"Could not set {obj.get_name()}.{prop_name}: {exc}")
        return False


def ensure_level():
    if asset_exists(MAP_PATH):
        unreal.EditorLevelLibrary.load_level(MAP_PATH)
        log(f"Loaded existing map: {MAP_PATH}")
    else:
        unreal.EditorLevelLibrary.new_level(MAP_PATH)
        log(f"Created map: {MAP_PATH}")


def actor_with_label_exists(label):
    world = unreal.EditorLevelLibrary.get_editor_world()
    for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
        try:
            if actor.get_actor_label() == label:
                return True
        except Exception:
            continue
    return False


def spawn_actor(actor_class, label, location, rotation=None, scale=None):
    if actor_with_label_exists(label):
        log(f"Skipped existing actor: {label}")
        return None

    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
    if not actor:
        warn(f"Failed to spawn actor: {label}")
        return None

    try:
        actor.set_actor_label(label)
    except Exception:
        pass

    if scale:
        actor.set_actor_scale3d(scale)

    return actor


def spawn_static_mesh(label, mesh, location, scale, rotation=None):
    actor = spawn_actor(unreal.StaticMeshActor, label, location, rotation, scale)
    if not actor:
        return None

    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    if component and mesh:
        component.set_static_mesh(mesh)
        component.set_collision_profile_name("BlockAll")

    return actor


def create_whitebox_geometry():
    cube = load_asset("/Engine/BasicShapes/Cube")
    if not cube:
        warn("Basic cube mesh is unavailable. Whitebox geometry was not created.")
        return

    # UE units are centimeters. 100m x 100m = 10000cm x 10000cm.
    spawn_static_mesh(
        "SE_Floor_100m_x_100m",
        cube,
        unreal.Vector(0.0, 0.0, -10.0),
        unreal.Vector(100.0, 100.0, 0.2),
    )

    # Boundary walls.
    spawn_static_mesh(
        "SE_Wall_North",
        cube,
        unreal.Vector(0.0, 5050.0, 150.0),
        unreal.Vector(101.0, 1.0, 3.0),
    )
    spawn_static_mesh(
        "SE_Wall_South",
        cube,
        unreal.Vector(0.0, -5050.0, 150.0),
        unreal.Vector(101.0, 1.0, 3.0),
    )
    spawn_static_mesh(
        "SE_Wall_East",
        cube,
        unreal.Vector(5050.0, 0.0, 150.0),
        unreal.Vector(1.0, 101.0, 3.0),
    )
    spawn_static_mesh(
        "SE_Wall_West",
        cube,
        unreal.Vector(-5050.0, 0.0, 150.0),
        unreal.Vector(1.0, 101.0, 3.0),
    )

    # Upper structure: platform, ramp, and blocky stairs.
    spawn_static_mesh(
        "SE_Upper_Platform",
        cube,
        unreal.Vector(1800.0, 1200.0, 430.0),
        unreal.Vector(22.0, 22.0, 0.6),
    )

    spawn_static_mesh(
        "SE_Ramp_To_Upper_Platform",
        cube,
        unreal.Vector(350.0, 1200.0, 220.0),
        unreal.Vector(28.0, 5.0, 0.35),
        unreal.Rotator(0.0, -16.0, 0.0),
    )

    step_count = 10
    for index in range(step_count):
        x = -2600.0 + index * 130.0
        z = 20.0 + index * 40.0
        label = f"SE_StairStep_{index + 1:02d}"
        spawn_static_mesh(
            label,
            cube,
            unreal.Vector(x, 1200.0, z),
            unreal.Vector(1.3, 5.0, 0.4),
        )

    # Simple obstacles for pathfinding and collision testing.
    obstacle_positions = [
        (-1500.0, -1500.0, 120.0, 8.0, 2.0, 2.4),
        (500.0, -1700.0, 120.0, 2.0, 8.0, 2.4),
        (1800.0, -900.0, 120.0, 6.0, 2.0, 2.4),
        (-900.0, 1800.0, 120.0, 2.0, 6.0, 2.4),
    ]
    for index, (x, y, z, sx, sy, sz) in enumerate(obstacle_positions, start=1):
        spawn_static_mesh(
            f"SE_NavObstacle_{index:02d}",
            cube,
            unreal.Vector(x, y, z),
            unreal.Vector(sx, sy, sz),
        )


def create_lighting_and_player_start():
    spawn_actor(
        unreal.PlayerStart,
        "SE_PlayerStart",
        unreal.Vector(-4200.0, -4200.0, 110.0),
        unreal.Rotator(0.0, 45.0, 0.0),
    )

    sun = spawn_actor(
        unreal.DirectionalLight,
        "SE_DirectionalLight",
        unreal.Vector(0.0, 0.0, 2500.0),
        unreal.Rotator(-45.0, -35.0, 0.0),
    )
    if sun:
        light = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if light:
            light.set_editor_property("intensity", 5.0)

    sky = spawn_actor(
        unreal.SkyLight,
        "SE_SkyLight",
        unreal.Vector(0.0, 0.0, 1200.0),
    )
    if sky:
        component = sky.get_component_by_class(unreal.SkyLightComponent)
        if component:
            component.set_editor_property("intensity", 0.6)


def spawn_search_escape_point_assets():
    chest_cls = load_bp_class(f"{ROOT}/Interactables/BP_SE_Chest")
    monster_point_cls = load_bp_class(f"{ROOT}/Interactables/BP_SE_SpawnPoint_Monster")
    escape_point_cls = load_bp_class(f"{ROOT}/Interactables/BP_SE_Point_Escape")

    chest_locations = [
        (-3500.0, -3200.0, 60.0),
        (-1800.0, -3200.0, 60.0),
        (200.0, -3300.0, 60.0),
        (2200.0, -3000.0, 60.0),
        (3800.0, -1800.0, 60.0),
        (-3600.0, 300.0, 60.0),
        (-1200.0, 700.0, 60.0),
        (2500.0, 700.0, 60.0),
        (1800.0, 1200.0, 500.0),
        (3000.0, 2300.0, 60.0),
    ]
    for index, (x, y, z) in enumerate(chest_locations, start=1):
        if chest_cls:
            spawn_actor(
                chest_cls,
                f"SE_Chest_{index:02d}",
                unreal.Vector(x, y, z),
                unreal.Rotator(0.0, index * 17.0, 0.0),
            )

    monster_locations = [
        (-2600.0, -400.0, 90.0),
        (-1000.0, -2400.0, 90.0),
        (1200.0, -2100.0, 90.0),
        (3200.0, 200.0, 90.0),
        (-2600.0, 2600.0, 90.0),
    ]
    for index, (x, y, z) in enumerate(monster_locations, start=1):
        if monster_point_cls:
            spawn_actor(
                monster_point_cls,
                f"SE_MonsterSpawn_{index:02d}",
                unreal.Vector(x, y, z),
            )

    escape_locations = [
        (4300.0, 4300.0, 80.0),
        (-4300.0, 4300.0, 80.0),
    ]
    for index, (x, y, z) in enumerate(escape_locations, start=1):
        if escape_point_cls:
            spawn_actor(
                escape_point_cls,
                f"SE_EscapePoint_{index:02d}",
                unreal.Vector(x, y, z),
                unreal.Rotator(0.0, 180.0 if index == 1 else 0.0, 0.0),
            )


def create_navmesh_bounds():
    try:
        actor = spawn_actor(
            unreal.NavMeshBoundsVolume,
            "SE_NavMeshBounds_100m",
            unreal.Vector(0.0, 0.0, 500.0),
            scale=unreal.Vector(55.0, 55.0, 8.0),
        )
        if actor:
            log("Created NavMeshBoundsVolume. Press P in editor to preview navmesh.")
    except Exception as exc:
        warn(f"Could not create NavMeshBoundsVolume: {exc}")


def configure_game_mode():
    gm_asset_path = f"{ROOT}/Core/GM_SearchEscape"
    player_asset_path = f"{ROOT}/Characters/BP_SE_Player"
    pc_asset_path = f"{ROOT}/Core/PC_SearchEscape"

    gm_asset = load_asset(gm_asset_path)
    gm_cls = load_bp_class(gm_asset_path)
    player_cls = load_bp_class(player_asset_path)
    pc_cls = load_bp_class(pc_asset_path)

    if gm_asset and gm_cls:
        cdo = unreal.get_default_object(gm_cls)
        if player_cls:
            set_prop(cdo, "default_pawn_class", player_cls)
        if pc_cls:
            set_prop(cdo, "player_controller_class", pc_cls)
        unreal.EditorAssetLibrary.save_loaded_asset(gm_asset)

    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()
    if gm_cls:
        if not set_prop(settings, "default_game_mode", gm_cls):
            set_prop(settings, "game_mode_override", gm_cls)


def save_all():
    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    log("Saved SearchEscape map and assets.")


def main():
    ensure_level()
    configure_game_mode()
    create_whitebox_geometry()
    create_lighting_and_player_start()
    create_navmesh_bounds()
    spawn_search_escape_point_assets()
    save_all()
    log("Test map generation complete.")


main()
