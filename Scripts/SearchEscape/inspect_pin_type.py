import unreal


pin = unreal.EdGraphPinType()
unreal.log(f"[SearchEscapePinType] dir: {', '.join([x for x in dir(pin) if not x.startswith('_')])}")

try:
    unreal.log(f"[SearchEscapePinType] editor props: {pin.get_editor_property_names()}")
except Exception as exc:
    unreal.log(f"[SearchEscapePinType] get_editor_property_names unavailable: {exc}")

for name in [
    "PinCategory",
    "pin_category",
    "pinCategory",
    "PinSubCategory",
    "pin_sub_category",
    "PinSubCategoryObject",
    "pin_sub_category_object",
    "ContainerType",
    "container_type",
    "bIsReference",
    "is_reference",
]:
    try:
        value = pin.get_editor_property(name)
        unreal.log(f"[SearchEscapePinType] {name} = {value}")
    except Exception as exc:
        unreal.log(f"[SearchEscapePinType] {name}: {exc}")

