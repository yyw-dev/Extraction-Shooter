import unreal


path = "/Game/SearchEscape/UI/WB_SE_HUD"
asset = unreal.EditorAssetLibrary.load_asset(path)
unreal.log(f"[SearchEscapeWidgetInspect] asset: {asset}")

for prop in ["widget_tree", "WidgetTree", "generated_class", "simple_construction_script"]:
    try:
        value = asset.get_editor_property(prop)
        unreal.log(f"[SearchEscapeWidgetInspect] {prop}: {value}")
        if value:
            unreal.log(
                f"[SearchEscapeWidgetInspect] {prop} dir: {', '.join([x for x in dir(value) if not x.startswith('_')])}"
            )
    except Exception as exc:
        unreal.log(f"[SearchEscapeWidgetInspect] {prop}: {exc}")

for name in [
    "CanvasPanel",
    "TextBlock",
    "ProgressBar",
    "VerticalBox",
    "HorizontalBox",
    "WidgetTree",
    "WidgetBlueprintLibrary",
]:
    obj = getattr(unreal, name, None)
    unreal.log(f"[SearchEscapeWidgetInspect] unreal.{name}: {obj}")
    if obj:
        unreal.log(
            f"[SearchEscapeWidgetInspect] unreal.{name} dir: {', '.join([x for x in dir(obj) if not x.startswith('_')][:100])}"
        )

