import unreal


def log(message):
    unreal.log(f"[SearchEscapeUMGAPI] {message}")


keywords = ("widget", "umg", "blueprint")
names = [name for name in dir(unreal) if all(k in name.lower() for k in ("widget",))]
for name in sorted(names):
    if any(k in name.lower() for k in ("blueprint", "tree", "editor", "panel", "slot", "umg")):
        obj = getattr(unreal, name, None)
        log(f"class {name}: {obj}")
        public = [item for item in dir(obj) if not item.startswith("_")] if obj else []
        interesting = [item for item in public if any(k in item.lower() for k in ("widget", "tree", "blueprint", "editor", "add", "create", "root", "compile"))]
        if interesting:
            log(f"  methods: {', '.join(interesting[:80])}")

for name in sorted(dir(unreal)):
    lowered = name.lower()
    if "widgetblueprint" in lowered or "widget_blueprint" in lowered:
        obj = getattr(unreal, name, None)
        log(f"direct {name}: {obj}")
        public = [item for item in dir(obj) if not item.startswith("_")] if obj else []
        log(f"  public: {', '.join(public[:120])}")
