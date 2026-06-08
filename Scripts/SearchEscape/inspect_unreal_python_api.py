import unreal


def dump(name):
    obj = getattr(unreal, name, None)
    if obj is None:
        unreal.log(f"[SearchEscapeInspect] {name}: MISSING")
        return

    members = [member for member in dir(obj) if not member.startswith("_")]
    interesting = [
        member
        for member in members
        if any(
            token.lower() in member.lower()
            for token in [
                "blueprint",
                "variable",
                "function",
                "graph",
                "node",
                "component",
                "widget",
                "property",
                "compile",
                "save",
            ]
        )
    ]
    unreal.log(f"[SearchEscapeInspect] {name}: {', '.join(interesting[:120])}")


for class_name in [
    "BlueprintEditorLibrary",
    "KismetEditorUtilities",
    "KismetSystemLibrary",
    "EditorAssetLibrary",
    "SubobjectDataSubsystem",
    "SubobjectDataBlueprintFunctionLibrary",
    "WidgetBlueprint",
    "WidgetBlueprintFactory",
    "BlueprintFactory",
    "AssetToolsHelpers",
]:
    dump(class_name)

