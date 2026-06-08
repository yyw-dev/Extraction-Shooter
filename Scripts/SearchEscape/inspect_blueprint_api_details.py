import inspect
import unreal


def show_callable(owner_name, function_name):
    owner = getattr(unreal, owner_name, None)
    function = getattr(owner, function_name, None) if owner else None
    if not function:
        unreal.log(f"[SearchEscapeApiDetails] {owner_name}.{function_name}: missing")
        return

    unreal.log(f"[SearchEscapeApiDetails] {owner_name}.{function_name}: {function}")
    try:
        unreal.log(f"[SearchEscapeApiDetails] signature: {inspect.signature(function)}")
    except Exception as exc:
        unreal.log(f"[SearchEscapeApiDetails] signature unavailable: {exc}")
    try:
        unreal.log(f"[SearchEscapeApiDetails] doc: {function.__doc__}")
    except Exception as exc:
        unreal.log(f"[SearchEscapeApiDetails] doc unavailable: {exc}")


for name in [
    "add_member_variable",
    "compile_blueprint",
    "add_function_graph",
    "find_event_graph",
]:
    show_callable("BlueprintEditorLibrary", name)

