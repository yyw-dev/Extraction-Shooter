import unreal


ROOT = "/Game/SearchEscape"


def log(message):
    unreal.log(f"[SearchEscapeVars] {message}")


def warn(message):
    unreal.log_warning(f"[SearchEscapeVars] {message}")


def load_bp(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        warn(f"Missing blueprint asset: {asset_path}")
    return asset


def pin_type(category, sub_category=""):
    pin = unreal.EdGraphPinType()
    if sub_category:
        text = f'(PinCategory="{category}",PinSubCategory="{sub_category}")'
    else:
        text = f'(PinCategory="{category}")'
    pin.import_text(text)
    return pin


FLOAT_TYPE = pin_type("real", "double")
INT_TYPE = pin_type("int")
BOOL_TYPE = pin_type("bool")
STRING_TYPE = pin_type("string")


def get_variable_names(blueprint):
    names = set()
    for prop_name in ["new_variables", "member_variables"]:
        try:
            for variable in blueprint.get_editor_property(prop_name):
                try:
                    names.add(str(variable.var_name))
                except Exception:
                    names.add(str(variable.get_editor_property("var_name")))
        except Exception:
            pass
    return names


def set_default(cdo, name, value):
    try:
        cdo.set_editor_property(name, value)
        log(f"Default {cdo.get_name()}.{name} = {value}")
    except Exception as exc:
        warn(f"Could not set default {cdo.get_name()}.{name}: {exc}")


def add_variable(blueprint, name, variable_type, default_value=None, instance_editable=True):
    existing = get_variable_names(blueprint)
    if name in existing:
        log(f"Variable already exists: {blueprint.get_name()}.{name}")
    else:
        added = unreal.BlueprintEditorLibrary.add_member_variable(blueprint, name, variable_type)
        if added:
            log(f"Added variable: {blueprint.get_name()}.{name}")
        else:
            warn(f"Failed to add variable: {blueprint.get_name()}.{name}")

    try:
        unreal.BlueprintEditorLibrary.set_blueprint_variable_instance_editable(
            blueprint, name, instance_editable
        )
    except Exception:
        pass

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

    if default_value is not None:
        generated_class = blueprint.generated_class()
        cdo = unreal.get_default_object(generated_class)
        set_default(cdo, name, default_value)


def compile_and_save(asset_path, blueprint):
    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    except Exception as exc:
        warn(f"Compile failed for {asset_path}: {exc}")
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    log(f"Saved: {asset_path}")


def configure_player():
    path = f"{ROOT}/Characters/BP_SE_Player"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_MaxHealth", FLOAT_TYPE, 100.0),
        ("SE_Health", FLOAT_TYPE, 100.0),
        ("SE_Gold", INT_TYPE, 0),
        ("SE_CombatPower", INT_TYPE, 0),
        ("SE_AttackDamage", FLOAT_TYPE, 25.0),
        ("SE_AttackRange", FLOAT_TYPE, 2500.0),
        ("SE_IsDead", BOOL_TYPE, False),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    for function_name in ["SE_TakeDamage", "SE_AddGold", "SE_AddCombatPower", "SE_Die"]:
        try:
            graph = unreal.BlueprintEditorLibrary.find_graph(bp, function_name)
        except Exception:
            graph = None
        if graph:
            log(f"Function graph already exists: {path}.{function_name}")
        else:
            try:
                unreal.BlueprintEditorLibrary.add_function_graph(bp, function_name)
                log(f"Added function graph: {path}.{function_name}")
            except Exception as exc:
                warn(f"Could not add function graph {function_name}: {exc}")

    compile_and_save(path, bp)


def configure_monster():
    path = f"{ROOT}/Characters/BP_SE_Monster"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_MaxHealth", FLOAT_TYPE, 60.0),
        ("SE_Health", FLOAT_TYPE, 60.0),
        ("SE_Damage", FLOAT_TYPE, 10.0),
        ("SE_AttackRange", FLOAT_TYPE, 150.0),
        ("SE_DetectRange", FLOAT_TYPE, 1200.0),
        ("SE_PatrolRadius", FLOAT_TYPE, 600.0),
        ("SE_ChaseLimit", FLOAT_TYPE, 2000.0),
        ("SE_RewardGold", INT_TYPE, 20),
        ("SE_RewardCombatPower", INT_TYPE, 1),
        ("SE_IsDead", BOOL_TYPE, False),
        ("SE_IsElite", BOOL_TYPE, False),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    for function_name in ["SE_TakeDamage", "SE_AttackPlayer", "SE_Die"]:
        try:
            graph = unreal.BlueprintEditorLibrary.find_graph(bp, function_name)
        except Exception:
            graph = None
        if graph:
            log(f"Function graph already exists: {path}.{function_name}")
        else:
            try:
                unreal.BlueprintEditorLibrary.add_function_graph(bp, function_name)
                log(f"Added function graph: {path}.{function_name}")
            except Exception as exc:
                warn(f"Could not add function graph {function_name}: {exc}")

    compile_and_save(path, bp)


def configure_game_mode():
    path = f"{ROOT}/Core/GM_SearchEscape"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_RoundDuration", FLOAT_TYPE, 300.0),
        ("SE_EscapeDoorSpawnTime", FLOAT_TYPE, 240.0),
        ("SE_TimeRemaining", FLOAT_TYPE, 300.0),
        ("SE_TotalGold", INT_TYPE, 0),
        ("SE_GameEnded", BOOL_TYPE, False),
        ("SE_ResultText", STRING_TYPE, "Playing"),
        ("SE_HUDWidget", pin_type("object"), None),
        ("SE_ResultWidget", pin_type("object"), None),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    for function_name in [
        "SE_StartRound",
        "SE_UpdateTimer",
        "SE_AddGold",
        "SE_EndGame_Escape",
        "SE_EndGame_Dead",
        "SE_EndGame_TimeOut",
        "SE_ShowResult",
    ]:
        try:
            graph = unreal.BlueprintEditorLibrary.find_graph(bp, function_name)
        except Exception:
            graph = None
        if graph:
            log(f"Function graph already exists: {path}.{function_name}")
        else:
            try:
                unreal.BlueprintEditorLibrary.add_function_graph(bp, function_name)
                log(f"Added function graph: {path}.{function_name}")
            except Exception as exc:
                warn(f"Could not add function graph {function_name}: {exc}")

    compile_and_save(path, bp)


def configure_manager():
    path = f"{ROOT}/Core/BP_SE_Manager"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_RoundDuration", FLOAT_TYPE, 300.0),
        ("SE_EscapeDoorSpawnTime", FLOAT_TYPE, 240.0),
        ("SE_TimeRemaining", FLOAT_TYPE, 300.0),
        ("SE_GameEnded", BOOL_TYPE, False),
        ("SE_ResultText", STRING_TYPE, "Playing"),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    compile_and_save(path, bp)


def configure_chest():
    path = f"{ROOT}/Interactables/BP_SE_Chest"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_OpenDuration", FLOAT_TYPE, 1.0),
        ("SE_OpenProgress", FLOAT_TYPE, 0.0),
        ("SE_MinGold", INT_TYPE, 10),
        ("SE_MaxGold", INT_TYPE, 50),
        ("SE_IsOpening", BOOL_TYPE, False),
        ("SE_IsOpened", BOOL_TYPE, False),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    compile_and_save(path, bp)


def configure_escape_door():
    path = f"{ROOT}/Interactables/BP_SE_EscapeDoor"
    bp = load_bp(path)
    if not bp:
        return

    variables = [
        ("SE_IsActive", BOOL_TYPE, False),
        ("SE_ResultText", STRING_TYPE, "Escaped"),
    ]
    for name, var_type, default in variables:
        add_variable(bp, name, var_type, default)

    compile_and_save(path, bp)


def main():
    configure_player()
    configure_monster()
    configure_game_mode()
    configure_manager()
    configure_chest()
    configure_escape_door()
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    log("Phase 2 variables complete.")


main()
