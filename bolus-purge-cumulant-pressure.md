
# 快进排气累积量与压力预测 — 开发计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **需求文档:** `lv_port_pc_visual_studio/ai_docs/快进排气累积量与压力预测需求说明书.docx`

**Goal:** 逐步为 NS 系列输液泵构建四大业务页面——快进排气设置、累积量统计、限制量设置、压力预测——复用现有控件库和业务层 API，与已有的下拉菜单入口集成。

**Architecture:** 每个功能模块新增一个 Menu 页面文件（`App/View/Menu/MenuXxx.h/.c`），通过 `CreateStdWinWidgetNS` 构建标准窗口，使用现有控件库（`CmLvglNoImgBtn`、`CmLvglNumKeypadWidget`、`CmLvglSwitch`、`CmLvglLabelSelect` 等）组装页面内容，通过项目文件（`.vcxproj`/`.filters`）和 `GuiInclude.h` 注册。压力预测逻辑扩展 `CmLvglPressureWidget` 在控件层实现。

**Tech Stack:** C, LVGL 8.x, Visual Studio project

**前置可复用基础:**

| 类型 | 组件 | 位置 |
|------|------|------|
| 标准窗口 | `CreateStdWinWidgetNS` | [WidgetStdWinNS.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/WidgetStdWinNS.h) |
| 参数项按钮 | `CreateParamBtnWidgetNS` | [WidgetCommonNS.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/WidgetCommonNS.h) |
| 分页网格容器 | `WidgetGridPagedNS`（`wColWidth`: 0=FR平分默认/兼容, >0=固定像素列宽靠左） | [WidgetGridPagedNS.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/WidgetGridPagedNS.h) |
| 数值键盘 | `CmLv_CreateNumKeypadWidget` | [CmLvglNumKeypadWidget.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglNumKeypadWidget.h) |
| 开关控件 | `CmLv_CreateSwitch` | [CmLvglSwitch.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglSwitch.h) |
| 下拉选择 | `CmLv_CreatDropDownWidget` | [CmLvglDropDownWidget.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglDropDownWidget.h) |
| 标签切换 | `CmLvglLabelSelect` | [CmLvglLabelSelect.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglLabelSelect.h) |
| 报警弹窗 | `CmLv_CreateAlarmMsgWinWidget` | [CmLvglAlarmMsgWinWidget.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglAlarmMsgWinWidget.h) |
| 压力控件 | `CmLv_CreatePressWidget` | [CmLvglPressureWidget.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglPressureWidget.h) |
| 纯文本按钮 | `CmLv_CreatButtonWidget` | [CmLvglBtnWidget.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglBtnWidget.h) |
| 快进/排气业务 API | `SetBolusRate/Volume`、`SetPurgeRate/Volume` | [BolusPurge.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmInfusionLib/ModeParam/BolusPurge.h) |
| 参数限制 API | `EM_LIMIT_ERR_TYPE`（50+ 限制类型） | [ModeParamLimit.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmInfusionLib/ModeParam/ModeParamLimit.h) |
| 下拉菜单（已有入口） | `EM_DROPDOWN_PURGE`、`EM_DROPDOWN_ACCUMULATE` | [MenuDropdown.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuDropdown.h) |
| 样式常量 | `PUMP_WIN_BG_COLOR`、字体宏等 | [UiStyle.h](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/UiStyle.h) |

---

### Phase 1: 快进排气功能 (P0)

---

> **导航路径:** 下拉菜单(MenuDropdown)第5按钮"设置" → `CreateMainMenuWin`(MenuSetting) → 参数设置按钮 → 快进排气设置
>
> **已存在的链:** `MenuDropdown.c:1165` 中 `EM_DROPDOWN_SETTING` → `CreateMainMenuWin(NULL)` 已就绪。
> MenuSetting 的 `EM_ID_MENU_PARA_SET` 按钮回调已接入 `CreateParamSetWin(NULL)`（见 [MenuSetting.c:154](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuSetting.c:154)）。

#### Task 1: 参数设置中间页面（MenuParamSet）✅ 已完成

**Files:**
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuParamSet.h`
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuParamSet.c`

**说明:** 实际实现为 13 项参数列表页（对齐 `.inch/CM_Infusion/App/View/Menu/MenuParamSet.c` 枚举顺序），使用 `WidgetGridPagedNS`（5列1行，每页5项）分页 + 底部 PageIndicator。参数项：快进排气/堵塞压力/气泡大小/无操作时间/接近完成/接近排空/排空剩余量/延长管脱落/KVO/常用模式/累积气泡/空瓶检测/大字模式。卡片固定 208px 宽、16px 间距靠左排列（通过 `WidgetGridPagedNS.wColWidth=208` 实现）。业务 API 在模拟器 0 定义，数据用 "--" 占位、子页面跳转留 TODO 桩。

- [x] **Step 1: 定义接口**

```c
// MenuParamSet.h
void CreateParamSetWin(void *pParam);   /* 实际函数名，非 DskBuild 前缀 */
```

- [x] **Step 2: 实现参数设置窗口**

实际实现要点（对齐 .inch）：
- `CreateStdWinWidgetNS` 创建标准窗口，标题 `GetStr(IDS_SETPARA)`
- `pContent` 改 `LV_FLEX_FLOW_COLUMN` + `pad_bottom=4`（覆盖 CmLvglWin 默认 28，使指示器贴底）
- `WidgetGridPagedNS_Create` 创建分页容器，`u8ColCount=5/u8RowCount=1`，`wColWidth=208`，`wColGap=16`
- 循环 `WidgetGridPagedNS_AddItem` 添加 13 项 `CreateParamBtnWidgetNS` 卡片（自动分页 + 分配 grid 单元格）
- 点击分发框架对齐 .inch 子页面跳转（业务 API 0 定义，留 TODO 桩）

- [x] **Step 3: MenuSetting.c 中接入参数设置按钮**

已接入 [MenuSetting.c:154](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuSetting.c:154)：

```c
if(szMenuBtn[EM_ID_MENU_PARA_SET] == lv_event_get_target(pEvent))
{
    if(LV_EVENT_CLICKED == lv_event_get_code(pEvent))
    {
        /* 进入参数设置中间页（设置→参数设置） */
        CreateParamSetWin(NULL);
    }
}
```

#### Task 2: 快进排气设置页面 UI ✅ 已完成

**Files:**
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuBolusExhaustSet.h`（注：实际文件名对齐 `.inch`，非原计划的 `MenuBolusPurgeSet`）
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuBolusExhaustSet.c`

**需求来源:** 需求文档 §2.4.1 快进排气设置路径（下拉菜单→设置→参数设置→快进排气设置）

**实现说明:** 复用 `.inch/CM_Infusion/App/View/Menu/MenuBolusExhaustSet.c` 的枚举顺序与业务回调结构。布局 100% 复用 MenuParamSet 框架（`CreateStdWinWidgetNS` + `WidgetGridPagedNS` 5列1行 + 底部指示器贴底，`wColWidth=208`、`pad_bottom=4`），5 项一页。5 项统一用 `CreateParamBtnWidgetNS` 卡片（含开关项，保持布局一致）。业务 API（`GetSysBolusSpeed` 等）在模拟器 0 定义，4 参数数据用 "--" 占位，键盘弹窗与开关写入留 TODO 桩（注明 `.inch` 源函数）。

**页面内容（对齐 .inch 枚举顺序）:**
- 【快进速度】参数卡片 → 点击 TODO 弹键盘（`.inch: _cbBolusExhaSetBolusRate`）
- 【排气速度】参数卡片 → 点击 TODO 弹键盘（`.inch: _cbBolusExhaSetExhaustRate`）
- 【快进待入量】参数卡片 → 点击 TODO 弹键盘（`.inch: _cbBolusExhaSetBolusVtbi`）
- 【排气待入量】参数卡片 → 点击 TODO 弹键盘（`.inch: _cbBolusExhaSetExhaustVtbi`）
- 【半自动排气】参数卡片（数据区显示"开/关"）→ 点击翻转（业务写入 `GetCommConfig()->bHalfAutoExha` 留 TODO，对齐 `.inch: HalfAutoExhaSwCb`）

- [x] **Step 1: 定义枚举和数据结构**

实际枚举（顺序对齐 `.inch`，非原计划顺序）：

```c
// MenuBolusExhaustSet.c（枚举为文件内匿名 enum，非 .h 导出）
enum
{
    EM_ID_BP_SET_BOLUS_RATE = 0,    //快进速度    (.inch: EM_ID_BOLUEXHA_SET_BOLUS_RATE)
    EM_ID_BP_SET_EXHAUST_RATE,      //排气速度    (.inch: EM_ID_BOLUEXHA_SET_EXHAUST_RATE)
    EM_ID_BP_SET_BOLUS_VTBI,        //快进待入量  (.inch: EM_ID_BOLUEXHA_SET_BOLUS_VTBI)
    EM_ID_BP_SET_EXHAUST_VTBI,      //排气待入量  (.inch: EM_ID_BOLUEXHA_SET_EXHAUST_VTBI)
    EM_ID_BP_SET_HALF_AUTO,         //半自动排气开关 (.inch: CmLv_CreateSwitch + HalfAutoExhaSwCb)
    EM_ID_BP_SET_MAX
};

// .h 仅导出创建函数（函数名对齐 .inch，非 DskBuild 前缀）
void CreateBoluExhaSetWin(void *pParam);
```

数据结构：`ST_BP_ITEM { const char *pData; }`（标题改用独立 `static const UINT32 s_au32TitleIds[]` 数组存 `IDS_xxx`，循环内 `GetStr` 实现多语言——规避 MSVC C2166 静态数组字段运行时赋值问题）。

- [x] **Step 2: 实现快进排气设置窗口**

实际实现要点（对齐 .inch + 复用 MenuParamSet 框架）：
- `CreateStdWinWidgetNS` 创建标准窗口，标题 = `GetStr(IDS_BOLUS) + GetStr(IDS_PURGE)`（"快进排气"）
- `pContent` 改 `LV_FLEX_FLOW_COLUMN` + `pad_bottom=4`（覆盖 CmLvglWin 默认 28，使指示器贴底）
- `WidgetGridPagedNS_Create` 创建分页容器，`u8ColCount=5/u8RowCount=1`，`wColWidth=208`，`wColGap=16`
- 循环 `WidgetGridPagedNS_AddItem` 添加 5 项 `CreateParamBtnWidgetNS` 卡片（标题用 `GetStr(s_au32TitleIds[u8Idx], GetLanguage())`）
- `_styleParamCard` 数据居中 + 右下角 ">" 箭头（复用 MenuParamSet 样式函数）
- `_cbItemClick` 按枚举分发：4 参数项 TODO 弹键盘、开关项翻转"开/关"文本

- [x] **Step 3: 实现键盘范围+步进逻辑**（留 TODO 桩，待业务层就绪）

键盘范围+步进规则（需求文档 §2.4.1，业务 API 就绪后按此对接）：

| 泵类型 | 参数 | 范围 | 默认值 | 步进规则 |
|--------|------|------|--------|---------|
| 输液泵 | 快进待入量 | 0.10~限制量 mL | 5 mL | 0.01 |
| 输液泵 | 快进流速 | 0.10~2000 mL/h | 1000 mL/h | 0.01/0.1/1 三档 |
| 输液泵 | 排气待入量 | 0.10~限制量 mL | 5 mL | 0.01 |
| 输液泵 | 排气流速 | 0.10~2000 mL/h | 1000 mL/h | 0.01/0.1/1 三档 |
| 注射泵 | 快进待入量 | 0.01~限制量 mL | 3 mL | 0.01 |
| 注射泵 | 快进流速 | 按注射器规格 | 按规格 | 0.01/0.1/1 三档 |
| 注射泵 | 排气待入量 | 0.01~限制量 mL | 3 mL | 0.01 |
| 注射泵 | 排气流速 | 按注射器规格 | 按规格 | 0.01/0.1/1 三档 |

> 注射器规格对照表：2/3mL→0.01~150 (def 150)、5/6mL→0.01~300 (def 300)、10/12mL→0.10~800 (def 800)、20mL→0.10~1200 (def 1200)、30/35mL→0.10~1500 (def 1500)、50/60mL→0.10~2300 (def 1500)
>
> 当前模拟器业务 API 0 定义，`_cbItemClick` 内 4 参数项弹键盘逻辑留 TODO 桩（注明 `.inch` 源函数 `_cbBolusExhaSetBolusRate/_cbBolusExhaSetExhaustRate/_cbBolusExhaSetBolusVtbi/_cbBolusExhaSetExhaustVtbi`）。

- [x] **Step 4: 调用业务层 API**（留 TODO 桩，待业务层就绪）

业务 API 对接清单（模拟器就绪后按此调用，对齐 `.inch`）：

```c
/* 快进速度/待入量 (.inch: GetSysBolusSpeed/SetSysBolusSpeed, GetSysBolusVtbi/SetSysBolusVtbi) */
GetSysBolusSpeed(spec);   SetSysBolusSpeed(spec, fRate);
GetSysBolusVtbi(spec);    SetSysBolusVtbi(spec, fVolume);

/* 排气速度/待入量 (.inch: GetSysExhaustSpeed/SetSysExhaustSpeed, GetSysExhaustVtbi/SetSysExhaustVtbi) */
GetSysExhaustSpeed(spec); SetSysExhaustSpeed(spec, fRate);
GetSysExhaustVtbi(spec);  SetSysExhaustVtbi(spec, fVolume);

/* 半自动排气开关 (.inch: GetCommConfig()->bHalfAutoExha) */
GetCommConfig()->bHalfAutoExha;   /* 读 */
GetCommConfig()->bHalfAutoExha = bNew;  /* 写（HalfAutoExhaSwCb） */

/* 范围检查使用 */
// LIMIT_ERR_EX_BOLUS_RATE_MAX/MIN, LIMIT_ERR_EX_BOLUS_VTBI_MAX/MIN
// LIMIT_ERR_EX_EXHAUST_RATE_MAX/MIN, LIMIT_ERR_EX_EXHAUST_VTBI_MAX/MIN
```

> **触发跳转:** MenuParamSet 点击"快进排气设置"卡片 → `CreateBoluExhaSetWin(NULL)`（见 [MenuParamSet.c](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuParamSet.c) `EM_ID_PARAM_SET_BOLUS_EXHAUST` case）。

#### 进度补记 (2026-07-09)：Task 2 键盘确认回调 bug 修复

实际使用中发现 Task 2 键盘确认回调存在崩溃，已修复（仅 `MenuBolusExhaustSet.c`）：

1. **键盘确认显示崩溃（已修复）**
   - 现象：输入数值点「确定」后控制台报 `lv_img_get_src: Asserted ... Incompatible object type`（`lv_img.c line #313`）。
   - 根因：4 个键盘确认回调（`_cbKeypadBolusRate`/`ExhaustRate`/`BolusVtbi`/`ExhaustVtbi`）及 `HALF_AUTO` 开关调用了 `CmLv_NoImgBtnUpdateData()`，该函数假定第二子对象为 `lv_img` 箭头并调用 `lv_img_get_src`；但本页卡片经 `_styleParamCard` 追加了 `>` 箭头 label，真实结构为 `[title, data, arrow]`，数据标签是 `child[cnt-2]`，对 data label 调 `lv_img_get_src` 触发断言。**数字键盘控件本身无问题。**
   - 修复：新增 `_UpdateParamCardData(pBtn, pText)` 帮助函数，直接取 `child[cnt-2]`（数据标签）调 `lv_label_set_text` 更新文本，替换上述 5 处调用。键盘确认现在能正确刷新显示值。

2. **C2371 编译错误（已修复）**
   - 现象：新增 `_UpdateParamCardData` 后报 `C2371 “_UpdateParamCardData”: 重定义；不同的基类型`（行 428）。
   - 根因：函数定义在文件末尾（约行 428），但首次被 `_cbItemClick`（约行 361）调用时未在前向声明段（STATIC PROTOTYPES）声明，编译器隐式推断为 `int` 返回类型，遇到 `void` 定义报重定义类型冲突。
   - 修复：在 `STATIC PROTOTYPES` 段补 `static void _UpdateParamCardData(lv_obj_t *pBtn, const char *pText);` 前向声明。

> **当前状态（Task 2）**：UI、键盘弹窗、确认回调显示刷新均已可用；业务层 `SetSys*Bolus/Exhaust*`、范围检查（`LIMIT_ERR_EX_*`）、`GetCommConfig()->bHalfAutoExha` 写入仍为 TODO 桩（依赖模拟器业务层就绪，见 Step 3/4）。

#### Task 3: 排气确认弹窗 ✅ 已完成

**Files:**
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/MenuPromp.h`
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Widgets/MenuPromp.c`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuDropdown.c`（排气卡片点击 → 确认弹窗）
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/GuiInclude.h`（启用 `MenuPromp.h`）
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj` / `.filters`（注册 MenuPromp.h/.c 到 `App\View\Widgets`）

**需求来源:** §2.4.1 排气功能入口——"排气前，请断开泵与病人的连接"确认弹窗

**实现说明:** 按用户要求，参照 `.inch/CM_Infusion/App/View/Menu/MenuCtrlCentor.c:1257-1278` 的排气点击逻辑落地，并新建通用提示界面 `MenuPromp`（后续所有提示界面都在此添加）。

- [x] **Step 1: 新建通用提示界面 MenuPromp（仿 .inch）**

对齐 `.inch/CM_Infusion/App/View/Widgets/MenuPromp.c` 的结构，但只抽取通用、无业务依赖的部分，作为 NS 模拟器所有提示弹窗的统一定义位置：

  - `MenuPromp.h`：`PrompWinCallBack` 回调类型、`EM_ID_PROMP_CANCEL/OK` 按钮消息 ID 枚举、`CreatePrompWin` / `CreateSucessFailPrompWin` 声明。
  - `MenuPromp.c`：
    - `CreatePrompWin(pText, call_back)`：用 `CmLv_CreateAlarmMsgWinWidget` 创建含"取消(IDS_CANCEL)/确定(IDS_OK)"双按钮的确认弹窗，提示文本用 `pText`，提示图片 `IMG_PROMP_WARNING`，窗口铺满状态栏以下区域（`PUMP_STATUS_HEIGTH`）。按钮颜色用 `PUMP_SKY_BLUE_COLOR`（原 `PUMP_PROMP_BTN_COLOR` 在当前项目 `UiStyle.h` 中被注释，二者同值 0x007DFF）。
    - `_cbPrompTipOK` / `_cbPrompTipCancel`：在 `LV_EVENT_RELEASED` 时回调 `user_data` 传入的 `PrompWinCallBack`，分别带 `EM_ID_PROMP_OK` / `EM_ID_PROMP_CANCEL`。
    - `CreateSucessFailPrompWin(pText, call_back, IMG, bCancel, bOk)`：成功/失败/等待类提示（可单独禁用某按钮），供后续导出/校准等提示界面复用。

- [x] **Step 2: 在 MenuDropdown 中实现排气卡片点击 → 确认弹窗**

逻辑对齐 `.inch MenuCentorPurge(bFlag=FALSE)` 进入排气分支：

```c
/* MenuDropdown.c 排气卡片点击 */
case EM_DROPDOWN_PURGE:
    _MenuDropdownPurge();      // 弹"排气前，请断开泵与病人的连接"确认弹窗
    break;

/* 静态句柄 s_pPurgeHint + _MenuDropdownPurge()：
 *   弹出 CreatePrompWin(GetStr(IDS_PURGE_TIPS, GetLanguage()), _cbPurgePromp)
 * _cbPurgePromp(e, u8MsgId)：
 *   确定(EM_ID_PROMP_OK) → TODO: 进入排气模式（主界面快进按钮切换为排气按钮）
 *   无论确定/取消，回调末将 s_pPurgeHint 置 NULL
 */
```

**当前状态（Task 3）：** 下拉菜单"排气"卡片点击已能弹出"排气前，请断开泵与病人的连接"确认弹窗；确定/取消回调框架已就位。

**留 TODO 桩（待业务层/Task 4 就绪对接）：**
1. `_MenuDropdownPurge` 中的状态检查：仅在 停止/暂停 且已安装耗材 时允许排气（`.inch: GetSysStatus()->bInstallOk`）；
2. `_cbPurgePromp` 确定分支的"进入排气模式"动作：`.inch: SetRunAreaBolusOrExhaust(EM_RUN_AREA_BTN_EXHAUST)` + 主界面快进按钮切换为排气按钮（对应 Task 4 的 `DskMgrSetMode(EM_DESK_MODE_PURGE)`）。

#### Task 4: 主界面快进按钮与排气按钮状态切换

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Desktop/DskBuild.c`（快进按钮创建逻辑）
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Desktop/DskManager.h`（状态管理枚举）
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Desktop/DskUpdate.c`（运行状态更新）

**需求来源:** §2.4.1 主界面快进按钮行为；排气模式下快进按钮切换为排气按钮

- [x] **Step 1: 扩展 DskManager 增加排气模式状态**

```c
/* 桌面模式状态 */
typedef enum {
    EM_DESK_MODE_NORMAL = 0,   /* 正常模式（快进按钮）  */
    EM_DESK_MODE_PURGE,        /* 排气模式（排气按钮）  */
} EM_DESK_MODE;

/* 获取/设置当前桌面模式 */
EM_DESK_MODE DskMgrGetMode(void);
void DskMgrSetMode(EM_DESK_MODE eMode);
```

- [x] **Step 2: 桌面更新回调中切换按钮行为**

```c
/* DskUpdate.c 中更新快进/排气按钮 */
if (DskMgrGetMode() == EM_DESK_MODE_PURGE)
{
    /* 按钮文本显示"排气"，长按=手动排气，短按=自动排气 */
    CmLv_NoImgBtnUpdateText(pBolusBtn, GetStr(IDS_PURGE, GetLanguage()));
}
else
{
    /* 按钮文本显示"快进"，长按=手动快进，短按=自动/半自动快进 */
    CmLv_NoImgBtnUpdateText(pBolusBtn, GetStr(IDS_BOLUS, GetLanguage()));
}
```

- [ ] **Step 3: 实现退出排气的各种条件**（需求 §2.4.1 第10点）

退出排气条件矩阵：
| 条件 | 动作 |
|------|------|
| 自动排气量打完 | 自动退出、恢复停止/暂停状态 |
| 手动排气量=排气待入量 | 自动退出 |
| 自动排气中暂停 | 自动退出 |
| 手动排气中松键 | 自动退出 |
| 启动输注 | 自动退出、切回快进按钮 |
| 再次点击控制中心【排气】 | 手动退出 |
| 去除耗材 | 异常退出、控制中心排气置灰 |
| 高级报警 | 中断退出（系统故障/阻塞/注射器脱落/爪夹异常/上下阻塞/输液器脱落/管路异常/门开/门异常） |

#### 进度补记 (2026-07-09)：Task 4 实现情况

**已完成（Step 1 + Step 2 + Step 3 部分）**
- `DskManager.h/.c`：`EM_DESK_MODE{NORMAL,PURGE}` + `DskMgrGetMode/SetMode/ExitPurgeMode`（静态 `s_eDeskMode`）。
- `DskUpdate.c`：300ms 定时器 `DskLayoutCbDesktopTimer300ms` → `_DskUpdateBolusBtn()`，按模式把 `PAW_BOLUS_BTN`(ImgBtn) label 在 `IDS_BOLUS`/`IDS_PURGE` 间切换（模式未变则跳过）。`s_szEventDAC[]` 注册 `PAW_BOLUS_BTN → _cbBolusBtnCallback`（排气模式再次点击=手动退出）。
- `MenuDropdown.c`：`_cbPurgePromp` 确定分支调用 `DskMgrSetMode(EM_DESK_MODE_PURGE)`，闭合 Task3→Task4 入口。

**关键实现点**：ImgBtn 无官方文本更新 API（`CmLvglImgBtn.h` 只有 `CmLv_ImgBtnUpdateImg`），改用 `lv_obj_check_type(child,&lv_label_class)` 查找子对象 `lv_label_set_text` 刷新；`CmLv_UpdateBtn` 是纯文本按钮 API 不适配 ImgBtn。

**Step 3 其余退出条件（TODO 扩展点）**：自动排气量打完/暂停/启动输注/去除耗材/高级报警等业务事件统一调用 `DskMgrExitPurgeMode()`；当前模拟器业务层为桩，未接实际事件，留 TODO。

**当前状态（Task 4）**：排气入口与"手动再次点击退出"已可演示；按钮文本随模式切换；其余退出条件待业务层就绪后接入 `DskMgrExitPurgeMode()`。

#### Task 5: 注册 Phase 1 文件到项目

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj.filters`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/GuiInclude.h`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuSetting.c`（接入参数设置按钮）

> **进度:** MenuParamSet.h/.c 与 MenuBolusExhaustSet.h/.c 均已注册到 vcxproj/.filters/GuiInclude.h，MenuSetting.c 已接入参数设置跳转，MenuParamSet.c 已接入快进排气设置跳转。Phase 1 文件注册全部完成。

- [x] **Step 1: 添加文件到 vcxproj**（MenuParamSet + MenuBolusExhaustSet 均已添加）

```xml
<!-- vcxproj -->
<ClInclude Include="App\View\Menu\MenuParamSet.h" />            <!-- ✅ 已添加 -->
<ClCompile Include="App\View\Menu\MenuParamSet.c" />            <!-- ✅ 已添加 -->
<ClInclude Include="App\View\Menu\MenuBolusExhaustSet.h" />     <!-- ✅ 已添加 -->
<ClCompile Include="App\View\Menu\MenuBolusExhaustSet.c" />     <!-- ✅ 已添加 -->
```

- [x] **Step 2: 添加文件到 vcxproj.filters**（MenuParamSet + MenuBolusExhaustSet 均已添加）

```xml
<!-- vcxproj.filters -->
<ClInclude Include="App\View\Menu\MenuParamSet.h">              <!-- ✅ -->
  <Filter>App\View\Menu</Filter>
</ClInclude>
<ClCompile Include="App\View\Menu\MenuParamSet.c">              <!-- ✅ -->
  <Filter>App\View\Menu</Filter>
</ClCompile>
<ClInclude Include="App\View\Menu\MenuBolusExhaustSet.h">       <!-- ✅ -->
  <Filter>App\View\Menu</Filter>
</ClInclude>
<ClCompile Include="App\View\Menu\MenuBolusExhaustSet.c">       <!-- ✅ -->
  <Filter>App\View\Menu</Filter>
</ClCompile>
```

- [x] **Step 3: GuiInclude.h 注册**（MenuParamSet + MenuBolusExhaustSet 均已注册）

```c
// GuiInclude.h
#include "MenuParamSet.h"          /* NS: 参数设置（✅ 已注册） */
#include "MenuBolusExhaustSet.h"   /* NS: 快进排气设置（✅ 已注册） */
```

- [x] **Step 4: MenuSetting.c 中接入参数设置按钮跳转**

已完成（见 Task 1 Step 3，[MenuSetting.c:154](/d:/project/ns-lvgl/lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuSetting.c:154) 调用 `CreateParamSetWin(NULL)`）。

---

### Phase 2: 累积量统计 (P1)

#### Task 6: 累积量统计页面 UI

**Files:**
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuCumulant.h`
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuCumulant.c`

**需求来源:** §2.4.3 累积量统计——5 种累积量类型 + 历史流速 + 清空

**入口:** ①主界面点击【累积量】 ②下拉菜单(MenuDropdown)→累积量（已有 `EM_DROPDOWN_ACCUMULATE`）

- [ ] **Step 1: 定义枚举和数据结构**

```c
// MenuCumulant.h
typedef enum {
    EM_CUMULANT_TOTAL = 0,           /* 总累积量（掉电保存）        */
    EM_CUMULANT_24H,                 /* 24小时累积量               */
    EM_CUMULANT_RECENT,              /* 最近累积量（N小时, N=1~24）*/
    EM_CUMULANT_CUSTOM,              /* 自定义时间段累积量          */
    EM_CUMULANT_TIMED,               /* 定时累积量（按间隔分桶）    */
    EM_CUMULANT_TYPE_MAX
} EM_CUMULANT_TYPE;

typedef enum {
    EM_CUMULANT_HISTORY_FLOW = EM_CUMULANT_TYPE_MAX,  /* 历史流速（第6个视图） */
} EM_CUMULANT_EXTRA;

void DskBuildCreateCumulantWin(void *pParam);
```

- [ ] **Step 2: 实现累积量统计窗口**

```c
void DskBuildCreateCumulantWin(void *pParam)
{
    /* 1. 创建 NS 标准窗口 (CreateStdWinWidgetNS, 标题=GetStr(IDS_TOTAL_VOL)) */
    /* 2. 顶部: CmLvglLabelSelect 标签切换 5种累积量类型 + "历史流速"           */
    /* 3. 内容区: 根据当前选中类型动态构建                                    */
    /*    - 总累积量/24h/定时累积量: 一个大型数据展示标签 (48px字体)           */
    /*    - 最近累积量: 数据标签 + N小时设置（数字键盘或滚轮）                 */
    /*    - 自定义: 起始时间+结束时间选择 + 数据标签                           */
    /*    - 历史流速: 可滚动列表（每行: 时间 + 流速值），最大50条              */
    /* 4. 底部: CmLvglBtnWidget 灰色置灰【清空累积量】按钮                     */
    /*         点击 → 二次确认弹窗 → 统一清空所有累积量                        */
}
```

- [ ] **Step 3: 累积量数据模型（桩实现）**

```c
/* 累积量数据暂存（后续与业务层对接） */
typedef struct {
    FLOAT32 fTotalCumulant;               /* 总累积量          */
    FLOAT32 f24hCumulant;                 /* 24小时累积量      */
    FLOAT32 fRecentCumulant;              /* 最近累积量        */
    UINT8   u8RecentHours;                /* 最近 N 小时       */
    FLOAT32 fCustomCumulant;              /* 自定义累积量      */
    UINT32  u32CustomStartTime;           /* 起始时间 (epoch)  */
    UINT32  u32CustomEndTime;             /* 结束时间 (epoch)  */
    FLOAT32 afTimedBuckets[24];           /* 定时分桶（24个1h桶，合并规则按需求）*/
    UINT8   u8TimedInterval;              /* 定时间隔(1/2/4/8/12/24h) */
    
    /* 历史流速 */
    FLOAT32 afHistoryFlow[50];            /* 最多50条流速记录 */
    UINT8   u8HistoryCount;               /* 当前记录数        */
} ST_CUMULANT_DATA;
```

- [ ] **Step 4: 实现标签页切换逻辑**

```c
/* CmLvglLabelSelect 回调 */
static void _cbCumulantTabChanged(lv_event_t *e)
{
    UINT8 u8Idx = /* 从事件中获取选中索引 */;
    /* 切换内容区显示 */
    /* 最近累积量/自定义累积量需要额外显示设置输入控件 */
}
```

#### Task 7: 注册 Phase 2 文件到项目

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj.filters`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/GuiInclude.h`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuDropdown.c`（累积量卡片点击跳转）

- [ ] **Step 1: 添加文件到 vcxproj 和 .filters**

```xml
<ClInclude Include="App\View\Menu\MenuCumulant.h" />
<ClCompile Include="App\View\Menu\MenuCumulant.c" />
```

- [ ] **Step 2: 取消 GuiInclude.h 中 MenuCumulant 注释并替换**

```c
// GuiInclude.h
#include "MenuCumulant.h"           /* NS: 累积量统计 */
```

- [ ] **Step 3: MenuDropdown 累积量卡片点击跳转**

```c
case EM_DROPDOWN_ACCUMULATE:
    HideDropdownMenu();
    DskBuildCreateCumulantWin(NULL);
    break;
```

---

### Phase 3: 限制量设置 (P2)

#### Task 8: 限制量设置页面 UI

**Files:**
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuLimitSet.h`
- Create: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuLimitSet.c`

**需求来源:** §2.4.2 限制量设置——快进限制量、排气限制量、最大流速、最大待入量

**入口:** 下拉菜单(MenuDropdown)→设置(MenuSetting)→维护(`EM_ID_MENU_USR_MAINTE`)→输入用户维护密码→限制量设置

- [ ] **Step 1: 定义枚举**

```c
// MenuLimitSet.h
typedef enum {
    EM_LIMIT_SET_BOLUS_VOL = 0,      /* 快进限制量  */
    EM_LIMIT_SET_PURGE_VOL,          /* 排气限制量  */
    EM_LIMIT_SET_MAX_RATE,           /* 最大流速    */
    EM_LIMIT_SET_MAX_VTBI,           /* 最大待入量  */
    EM_LIMIT_SET_MAX
} EM_LIMIT_SET_ID;

void DskBuildCreateLimitSetWin(void *pParam);
```

- [ ] **Step 2: 实现限制量设置窗口**

```c
void DskBuildCreateLimitSetWin(void *pParam)
{
    /* 1. 创建 NS 标准窗口（CreateStdWinWidgetNS, 标题="限制量设置"）  */
    /* 2. 4项参数 + 1x4 网格布局（使用 WidgetGridPagedNS, 每页1x4）   */
    /*    注意: 此页面非分页，但复用 GridPagedNS 布局一致性             */
    /* 3. 输液泵和注射泵参数范围不同，根据 PUMP_TYPE 宏区分:            */
    /*    - 输液泵:                                                      */
    /*      快进限制量: 0.10~99.99 mL, 默认 50 mL                       */
    /*      排气限制量: 0.10~200.00 mL, 默认 10.00 mL                   */
    /*      最大流速: 0.10~2000 mL/h, 默认 2000 mL/h                     */
    /*      最大待入量: 0.10~9999.99 mL, 默认 9999.99 mL                 */
    /*    - 注射泵:                                                      */
    /*      快进限制量: 0.10~60.00 mL, 默认 10 mL                       */
    /*      排气限制量: 0.10~5.00 mL, 默认 5 mL                         */
    /*      最大流速: 按注射器规格, 默认按规格                            */
    /*      最大待入量: 0.10~9999.99 mL, 默认 9999.99 mL                 */
    /* 4. 所有参数项超范围提示：选择确定 → 清空值、需重新设置            */
}
```

#### Task 9: 注册 Phase 3 文件到项目

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.vcxproj.filters`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/GuiInclude.h`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Menu/MenuSetting.c`（维护按钮回调接入限制量设置）

```xml
<ClInclude Include="App\View\Menu\MenuLimitSet.h" />
<ClCompile Include="App\View\Menu\MenuLimitSet.c" />
```

```c
// GuiInclude.h
#include "MenuLimitSet.h"           /* NS: 限制量设置 */
```

---

### Phase 4: 压力预测 (P2)

#### Task 10: 扩展 CmLvglPressureWidget 支持三级压力状态图标

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglPressureWidget.h`
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/Middle/CmLib/CmLvglLib/CmLvglPressureWidget.c`

**需求来源:** §2.4.4 压力预测——a) 正常压力(白色)、b) 接近阻塞报警(橙色)、c) 达到阻塞报警(红色)

- [ ] **Step 1: 增加压力状态枚举**

```c
// CmLvglPressureWidget.h
typedef enum {
    EM_PRESSURE_NORMAL = 0,         /* 正常压力（白色/本色）   */
    EM_PRESSURE_NEAR_ALARM,         /* 接近阻塞报警（橙色）     */
    EM_PRESSURE_ALARM,              /* 达到阻塞报警（红色）     */
} EM_PRESSURE_STATE;
```

- [ ] **Step 2: 扩展创建参数结构**

```c
typedef struct {
    ST_BASICS_NATURE stBasics;
    FLOAT32 fCurrentPressure;               /* 当前实时压力值      */
    FLOAT32 fMaxPressure;                   /* 最大允许压力阈值    */
    FLOAT32 fAlarmThreshold;                /* 阻塞报警压力阈值    */
    EM_PRESSURE_STATE ePressureState;       /* 当前压力预测状态    */
} ST_PRESSURE_WIDGET_PARAM;  /* 扩展现有结构 */
```

- [ ] **Step 3: 新增 `CmLv_PressUpdateState` API**

```c
/**
 * 更新压力预测状态图标
 * @param pWidget    压力控件对象
 * @param eState     新的压力状态
 *   EM_PRESSURE_NORMAL    → 白色图标（本色）
 *   EM_PRESSURE_NEAR_ALARM → 橙色图标 #FFA500
 *   EM_PRESSURE_ALARM     → 红色图标 #EE3232 (PUMP_WARNING_RED)
 */
void CmLv_PressUpdateState(lv_obj_t *pWidget, EM_PRESSURE_STATE eState);
```

- [ ] **Step 4: 实现压力上升趋势检测（桩）**

```c
/* 在 DskUpdate 定时回调中（如 300ms 或 1s 定时器）检测压力趋势 */
static void _PressurePredictionCheck(void)
{
    FLOAT32 fCurrentPressure = /* 从压力传感器获取 */;
    FLOAT32 fAlarmThreshold = /* 获取阻塞报警阈值 */;
    
    if (fCurrentPressure >= fAlarmThreshold)
    {
        CmLv_PressUpdateState(pPressWidget, EM_PRESSURE_ALARM);
    }
    else if (fCurrentPressure >= fAlarmThreshold * 0.8f  /* 80% 阈值 */)
    {
        /* 检测上升趋势: 如果过去5秒内压力持续上升 */
        CmLv_PressUpdateState(pPressWidget, EM_PRESSURE_NEAR_ALARM);
    }
    else
    {
        CmLv_PressUpdateState(pPressWidget, EM_PRESSURE_NORMAL);
    }
}
```

#### Task 11: 注册 Phase 4 的 DskUpdate 集成

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/App/View/Desktop/DskUpdate.c`

- [ ] **Step 1: 在桌面定时回调中插入压力预测检测**

在 `DskUpdate.c` 现有的 300ms 或 1s 定时器回调中插入 `_PressurePredictionCheck()` 调用。压力控件的指针已通过 DskManager 的 `PAW_PRESSURE_WIDGET` 索引管理。

---

### Phase 5: 集成验证 (P2)

#### Task 12: Simulator 入口集成 + 编译验证

**Files:**
- Modify: `lv_port_pc_visual_studio/LVGL.Simulator/LVGL.Simulator.cpp`

- [ ] **Step 1: 添加功能开关宏 + 集成入口调用**

```c
// Simulator.cpp
#include "MenuBolusExhaustSet.h"
#include "MenuCumulant.h"
#include "MenuLimitSet.h"

#define ENABLE_BOLUS_PURGE_CUMULANT_PRESSURE_DEMO 0

int main()
{
    // ... existing code ...
    
#if ENABLE_BOLUS_PURGE_CUMULANT_PRESSURE_DEMO
    /* 演示: 依次打开各页面 */
    // CreateBoluExhaSetWin(NULL);              /* 快进排气设置 */
    // DskBuildCreateCumulantWin(NULL);         /* 累积量统计   */
    // DskBuildCreateLimitSetWin(NULL);         /* 限制量设置   */
#else
    /* 正式集成: 通过 MenuDropdown 点击触发各页面 */
    DskMgrBuildDesktop();
    CreateDropdownMenu();
    // ... 下拉菜单点击回调已集成 ENTROPDOWN_PURGE/ACCUMULATE ...
#endif
}
```

- [ ] **Step 2: 编译验证**

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' 'D:\project\ns-lvgl\lv_port_pc_visual_studio\LVGL.Simulator\LVGL.Simulator.vcxproj' /p:Configuration=Debug /p:Platform=x64 /m /nologo
```

期望: Build succeeds with `0 Error(s)`.

- [ ] **Step 3: 功能验证清单**

| 模块 | 验证项 |
|------|--------|
| 快进排气设置 | ① 下拉菜单→设置→参数设置→快进排气设置进入<br>② 参数项点击弹出数字键盘<br>③ 流速步进 0.01/0.1/1 切换正确<br>④ 开关控件切换半自动快进<br>⑤ 超范围值弹出提示 |
| 排气确认弹窗 | ① 下拉菜单(MenuDropdown)点击排气弹出确认弹窗<br>② 确定→主界面快进按钮切换为排气<br>③ 取消→弹窗关闭、状态不变<br>④ 运行中/未装耗材时排气图标置灰 |
| 累积量统计 | ① 主界面累积量点击 / 下拉菜单累积量进入<br>② 5种累积量标签切换<br>③ 最近累积量 N小时设置<br>④ 自定义时间段起止时间选择<br>⑤ 历史流速列表滚动<br>⑥ 清空累积量二次确认 |
| 限制量设置 | ① 下拉菜单→设置→维护→密码→限制量设置进入<br>② 4项参数按泵类型显示正确范围<br>③ 超范围值提示后清空 |
| 压力预测 | ① 桌面压力控件显示白色正常状态<br>② 接近阈值时切换橙色<br>③ 达到阈值时切换红色 |

---

### 修改文件汇总

> 状态：✅ 已完成 / ⬜ 待办

| Phase | 操作 | 文件 | 状态 |
|-------|------|------|------|
| P1 | Create | `App/View/Menu/MenuParamSet.h` | ✅ |
| P1 | Create | `App/View/Menu/MenuParamSet.c` | ✅ |
| P1 | Modify | `App/View/Widgets/WidgetGridPagedNS.h`（新增 `wColWidth` 字段） | ✅ |
| P1 | Modify | `App/View/Widgets/WidgetGridPagedNS.c`（colDsc 按 `wColWidth` 选 FR/固定像素） | ✅ |
| P1 | Modify | `App/View/Menu/MenuSetting.c`（参数设置按钮回调） | ✅ |
| P1 | Modify | `App/View/GuiInclude.h`（注册 MenuParamSet.h + MenuBolusExhaustSet.h） | ✅ |
| P1 | Modify | `LVGL.Simulator.vcxproj`（注册 MenuParamSet + MenuBolusExhaustSet） | ✅ |
| P1 | Modify | `LVGL.Simulator.vcxproj.filters`（注册 MenuParamSet + MenuBolusExhaustSet） | ✅ |
| P1 | Create | `App/View/Menu/MenuBolusExhaustSet.h` | ✅ |
| P1 | Create | `App/View/Menu/MenuBolusExhaustSet.c` | ✅ |
| P1 | Modify | `App/View/Menu/MenuDropdown.c`（排气卡片点击逻辑） | ✅ |
| P1 | Create | `App/View/Widgets/MenuPromp.h` | ✅ |
| P1 | Create | `App/View/Widgets/MenuPromp.c` | ✅ |
| P1 | Modify | `App/View/GuiInclude.h`（启用 MenuPromp.h） | ✅ |
| P1 | Modify | `LVGL.Simulator.vcxproj` / `.filters`（注册 MenuPromp） | ✅ |
| P1 | Modify | `App/View/Desktop/DskBuild.c` | ⬜ |
| P1 | Modify | `App/View/Desktop/DskManager.h` | ⬜ |
| P1 | Modify | `App/View/Desktop/DskManager.c` | ⬜ |
| P1 | Modify | `App/View/Desktop/DskUpdate.c` | ⬜ |
| P2 | Create | `App/View/Menu/MenuCumulant.h` | ⬜ |
| P2 | Create | `App/View/Menu/MenuCumulant.c` | ⬜ |
| P3 | Create | `App/View/Menu/MenuLimitSet.h` | ⬜ |
| P3 | Create | `App/View/Menu/MenuLimitSet.c` | ⬜ |
| P3 | Modify | `App/View/Menu/MenuSetting.c`（维护按钮回调） | ⬜ |
| P4 | Modify | `Middle/CmLib/CmLvglLib/CmLvglPressureWidget.h` | ⬜ |
| P4 | Modify | `Middle/CmLib/CmLvglLib/CmLvglPressureWidget.c` | ⬜ |
| P5 | Modify | `LVGL.Simulator.cpp` | ⬜ |

**总计:** 4 New ✅ + 6 Modify ✅（MenuParamSet + MenuBolusExhaustSet + WidgetGridPagedNS 扩展 + 注册接入）+ 2 New ⬜ + 11 Modify ⬜ = 23 个文件
