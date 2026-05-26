# ExtractionGame — UE 5.7 搜打撤游戏

## 项目概述

在 UE 5.7 中从零构建单机"搜刮-战斗-撤离"游戏。玩家在小地图中出生 → 搜刮武器/物资 → 与 AI 敌人交战 → 到达撤离点获胜。类似塔科夫的单机简化版。

- **引擎**: Unreal Engine 5.7
- **开发方式**: 蓝图为主，少量 C++ (Struct/Enum/性能敏感逻辑)
- **输入系统**: Enhanced Input (增强输入)，无旧版 Action Mappings
- **模块名**: ExtractionGame (Runtime)

## 目录结构

```
Source/ExtractionGame/
├── ExtractionGame.h/cpp       — 模块入口
├── ExtractionTypes.h/cpp      — 枚举 + 结构体定义 (ItemCategory, WeaponType, InventorySlot, WeaponData, ContainerLootEntry)
├── ExtractionGame.Build.cs    — 模块依赖: Core, CoreUObject, Engine, InputCore

Content/
├── BP_PickupBase              — 拾取物基类蓝图
├── Characters/                — 角色相关
├── Input/                     — Enhanced Input 资产 (IA_*, IMC_*)
├── LevelPrototyping/          — 关卡原型
├── Maps/                      — 关卡
├── ThirdPerson/               — 第三人称模板内容
└── Collections/               — 集合
```

## 已有的 C++ 类型

### 枚举 (ExtractionTypes.h)
- **EWeaponType**: Pistol, Rifle, Shotgun
- **EItemCategory**: Weapon, Ammo, Health, Valuables, Misc
- **EContainerType**: Crate, DuffleBag, WeaponCase

### 结构体 (ExtractionTypes.h)
- **FInventorySlot**: ItemID, ItemName, Category, Quantity, Icon
- **FWeaponData** (DataTable): WeaponName, WeaponType, BaseDamage, FireRate, MagazineSize, Range
- **FContainerLootEntry** (DataTable): ItemID, ItemName, Category, MinQuantity, MaxQuantity, SpawnChance

## 蓝图变量/函数命名

蓝图中的结构体类型去掉 F 前缀搜索：
- `FInventorySlot` → 搜索 `InventorySlot`
- `FWeaponData` → 搜索 `WeaponData`
- `FContainerLootEntry` → 搜索 `ContainerLootEntry`
- `Make FInventorySlot` → 搜索 `Make InventorySlot`
- `Break FInventorySlot` → 搜索 `Break InventorySlot`

## 技术约定

- **输入**: 所有按键通过 Input Action + Input Mapping Context 绑定，不使用旧版 Action Mappings
- **碰撞**: 拾取/交互使用 Sphere/Box Collision + OverlapAllDynamic
- **UI**: Widget Blueprint + Canvas Panel 布局
- **AI**: 推荐初学者使用 Tick AI（行为树备选）
- **伤害**: Actor Component (AC_HealthComponent) 复用
- **数据**: DataTable 驱动掉落/武器属性

## 当前进度

参照 搜打撤游戏_7天实施计划.md：
- ✅ Day 1: 项目创建 + C++ 类型定义
- 🔨 Day 2: 拾取系统 + 背包系统（BP_PickupBase 已创建）
- ⬜ Day 3-7: 待开发

## UnrealClaude MCP 工具

已安装 UnrealClaude v1.5.0 插件。当 UE 编辑器运行时，可通过 MCP 工具直接操作编辑器。
工具前缀为 `unreal_`，例如: `unreal_asset_search`, `unreal_blueprint_query`, `unreal_spawn_actor`, `unreal_get_level_actors`

**使用规则:**
- 优先使用 MCP 工具而非文件系统工具操作 UE 内容
- 涉及蓝图/动画/角色的操作通过 `unreal_ue` router 转发
- 并行工具调用上限: 4 个 (读操作可并行, 写操作不要同时改同一对象)
- UE 编辑器必须运行中且端口 3000 可访问

## 工作方式

- **C++ 代码**: 直接读写 `.h` / `.cpp` 文件
- **蓝图 (MCP)**: UE 编辑器运行时，用 `unreal_blueprint_query` 直接读取蓝图结构
- **蓝图 (手动)**: 在 UE 编辑器中 Ctrl+C 复制蓝图节点，粘贴到对话
- **UE 操作**: 用 MCP 工具搜索资产、生成 Actor、操作关卡等
