# 村庄地图 + SE 系统 迁移与配置文档

> 日期: 2026-06-09 | 项目: AdvancedGridInventorySyst 5.7

---

## 一、村庄地图迁移

从 `extraction_project` 复制 `Fantastic_Village_Pack` 到目标项目的 Content 目录：

```
源: extraction_project\Content\Fantastic_Village_Pack
目标: AdvancedGridInventorySyst 5.7\Content\Fantastic_Village_Pack
```

---

## 二、C++ 代码修改清单

### 2.1 敌人伤害系统 — 三层回退

**文件**: `SearchEscapeEnemyCharacter.cpp` — `ResolveAttackImpact()`

```cpp
// 伤害优先级:
// 1. USearchEscapeHealthComponent (SE 敌人专用)
// 2. ASEPlayerCharacter::SE_TakeDamage (SE 玩家)
// 3. USearchEscapePlayerComponent::SE_TakeDamage (组件方式)
// 4. AActor::TakeDamage (通用 UE 伤害管线, 兼容任何角色)
```

### 2.2 远程弓箭手敌人

**新增文件**:
- `SearchEscapeRangedEnemyCharacter.h/.cpp` — 继承自 `SearchEscapeEnemyCharacter`
- `SearchEscapeArrowProjectile.h/.cpp` — 箭矢投射物

**特性**:
- 不移动, 站桩输出 (速度 = 0)
- 3000 视野 / 2500 射程
- 180° 视角
- 每 1.8 秒射一箭, 伤害 20
- 箭矢有可见锥体模型
- 对 Pawn 使用 Overlap 检测, 对 World 使用 Hit 检测

**使用**: 右键搜索 `SearchEscapeRangedEnemyCharacter` 放置

### 2.3 AI 控制器修复

**文件**: `SearchEscapeEnemyAIController.h/.cpp`

- `bWasSensed` 标记 — 感知只在状态变化时触发, 防止刷屏
- 攻击迟滞 — 进入攻击距离 1.0x, 退出需要 1.3x, 防止边界震荡
- `PerformAttack` → `virtual` — 允许子类重写

### 2.4 玩家组件

**新增文件**: `SearchEscapePlayerComponent.h/.cpp`

通用 SE 玩家组件, 可挂到任何角色上:

| 系统 | 函数 |
|------|------|
| 血量 | `SE_TakeDamage(Amount)`, `GetHealthPercent()` |
| 金币 | `SE_AddGold(Amount)`, `SE_SpendGold(Amount)`, `SE_GetGold()` |
| 战力 | `SE_AddCombatPower(Amount)` |
| 弹药 | `ReloadWeapon(Count)`, `CanReload(Count)`, `AddReserveAmmo(Amount)` |
| 交互 | `Interact()`, `OnInteractFound`, `OnInteractMiss` |
| 事件 | `OnGoldChanged(NewAmount)`, `OnDeath` |

**使用**: 在角色蓝图上 Add Component → `SearchEscapePlayerComponent`

### 2.5 商人

**新增文件**: `SEVendor.h/.cpp`

C++ 商人, 使用 SE_Gold:

| 功能 | 函数 |
|------|------|
| 购买 | `BuyItem(Index, Player)` → 自动扣钱 |
| 查询 | `CanPlayerAfford(Player, Index)`, `GetItem(Index)` |
| 事件 | `OnShopOpened`, `OnItemPurchased`, `OnPurchaseFailed` |
| 范围 | `OnPlayerEnterRange`, `OnPlayerLeaveRange` |

**使用**: 右键搜索 `SEVendor` 放置, 在 Details 配 `Shop Items`

### 2.6 HUD 改为读组件

**文件**: `SEHUD.cpp`

`ASEHUD` 现在从 `USearchEscapePlayerComponent` 读数据, 不再依赖 `ASEPlayerCharacter`。

---

## 三、地图配置

### 3.1 村庄地图设置

```
World Settings → GameMode Override → Gamemode_AGIS
```

### 3.2 必要的地图元素

| 元素 | 说明 |
|------|------|
| Nav Mesh Bounds Volume | 覆盖可玩区域 (按 P 确认绿色) |
| Player Start | 玩家出生点 |
| SearchEscapeRangedEnemyCharacter | 塔楼弓箭手 |
| SearchEscapeEnemyCharacter | 地面近战敌人 |
| BP_Vendor | **从 TestRoom 复制粘贴**, 不能直接新建 |

### 3.3 BP_Vendor 关键发现

**必须从 TestRoom 复制**, 因为 TestRoom 中的 BP_Vendor 带有完整的库存引用链 (`Inventory__Main`, `Inventory_Player`, `Inventory_Crafting`), 直接新建的商人缺少这些引用, 导致买卖不扣钱。

---

## 四、玩家角色配置

`BP_ExampleCharacterAGIS` + `SearchEscapePlayerComponent`:

1. 打开 `BP_ExampleCharacterAGIS`
2. Add Component → `SearchEscapePlayerComponent`
3. 设置 `SE_StartingGold = 500`
4. 设置 `SE_MaxHealth = 100`

---

## 五、验证清单

- [ ] 村庄地图 GameMode = Gamemode_AGIS
- [ ] NavMesh 绿色覆盖
- [ ] BP_Vendor 从 TestRoom 复制
- [ ] BP_ExampleCharacterAGIS 有 SearchEscapePlayerComponent
- [ ] 近战敌人巡逻追击正常
- [ ] 弓箭手站立射箭正常
- [ ] 商人买卖扣钱正常
- [ ] HUD 显示血量金币正常
