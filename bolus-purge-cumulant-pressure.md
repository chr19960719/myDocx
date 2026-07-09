
//********************************************************
//*******Copyright (C), 2019-2099, comen. Co., Ltd.*******
//*
//*  文件名          :   MenuDropdown.c
//*  文件功能        :   首页下拉快捷菜单实现
//*  编程者          :   CodeBuddy
//*  日期            :   2026-06-30
//*  编号---作者----日期--------注释-----------------------
//* 00     CodeBuddy 2026-06-30   创建
//*  说明: 首页顶部下拉菜单实现下拉显示/上拉关闭动画，
//*        包含8个快捷功能卡片，用于快速访问常用功能
//********************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "stdio.h"
#include "CmLvglWin.h"
#include "CmLvglImgBtn.h"
#include "UiStyle.h"
#include "MenuDropdown.h"
#include "GuiInclude.h"
#include "PageIndicator.h"
#include "MenuPromp.h"          /* NS: 通用提示弹窗（CreatePrompWin） */
#include "DskManager.h"         /* NS: 桌面模式（DskMgrSetMode 进入排气模式） */

/* 排气确认弹窗句柄（同一时刻仅允许一个） */
static lv_obj_t *s_pPurgeHint = NULL;

//========================================================
// Function     : _cbPurgePromp
// Description  : 排气前提示窗口回调（确定→进入排气；取消→仅关闭）
// Input        : e 事件句柄 / u8MsgId 按钮消息ID
// Others       : 对齐 .inch MenuCtrlCentor.c::_cbPurgePromp
//========================================================
static void _cbPurgePromp(lv_event_t * e, UINT8 u8MsgId)
{
    if(EM_ID_PROMP_OK == u8MsgId)
    {
        /* 进入排气模式：主界面快进按钮将在 DskUpdate 定时器回调中切换为“排气” */
        DskMgrSetMode(EM_DESK_MODE_PURGE);
        /* TODO: 业务层就绪后调用 SetRunAreaBolusOrExhaust(EM_RUN_AREA_BTN_EXHAUST)
         *       启动实际排气动作 */
    }
    s_pPurgeHint = NULL;
}

//========================================================
// Function     : _MenuDropdownPurge
// Description  : 下拉菜单"排气"卡片点击逻辑
// Others       : 对齐 .inch MenuCentorPurge(bFlag=FALSE) 进入排气分支
//========================================================
static void _MenuDropdownPurge(void)
{
    /* TODO: 状态检查——仅在 停止/暂停 且已安装耗材 时允许排气
     * (.inch: if(GetSysStatus()->bInstallOk == FALSE) 置灰并返回) */

    /* 弹出"排气前，请断开泵与病人的连接"确认弹窗 */
    if(NULL != s_pPurgeHint && lv_obj_is_valid(s_pPurgeHint))
    {
        lv_obj_del(s_pPurgeHint);
        s_pPurgeHint = NULL;
    }
    s_pPurgeHint = CreatePrompWin(GetStr(IDS_PURGE_TIPS, GetLanguage()), _cbPurgePromp, PrompWin_GetUiRect());
}

/*********************
 *      DEFINES
 *********************/

/* 卡片内部布局常量 */
#define CARD_ICON_CIRCLE_SIZE   72      /* 图标圆形背景直径 */
#define CARD_ICON_CIRCLE_Y      31      /* 图标圆形相对于卡片顶部偏移 */
#define CARD_ICON_SIZE          36      /* 图标占位符尺寸 */
#define CARD_TEXT_Y             119     /* 文字相对于卡片顶部偏移 */
#define CARD_RADIUS             12      /* 卡片圆角 */

/* 下拉菜单有效内容高度（不覆盖状态栏） */
#define DROPDOWN_CONTENT_H      (WIN7_HEIGHT - WIN7_STATUS_H)  /* 240 */

/* 底部拖拽指示条（相对面板顶部偏移，面板内部坐标） */
#define DRAG_BAR_W              320
#define DRAG_BAR_H              8
#define DRAG_BAR_Y              230
#define DRAG_BAR_RADIUS         4

/* 卡片背景色 (半透明浅灰) */
#define CARD_BG_COLOR           0xD8D8D8
#define CARD_BG_OPA             LV_OPA_10
/* 图标圆形背景色 (半透明白) */
#define CARD_ICON_BG_OPA        (LV_OPA_20 + (LV_OPA_10 / 3)) /* ~23% */

/* 注意：GetStr 返回的是 language.c 中 6 个轮转静态缓冲区之一的指针
 * (s_szStrBufPool[6]，每次调用轮回复用)。绝不能把 GetStr 的返回指针跨多次
 * 调用缓存到数组中——后续 GetStr 会覆盖早期缓冲区，导致前面试图项的文本
 * 被后面项覆盖。这里改为自有存储，在 InitTexts 中立即拷贝字符串内容。 */
#define DROPDOWN_TEXT_BUF_SIZE  48
static char s_aszCardTexts[EM_DROPDOWN_MAX][DROPDOWN_TEXT_BUF_SIZE];
static char s_aszCard2Texts[EM_DROPDOWN_PAGE2_MAX][DROPDOWN_TEXT_BUF_SIZE];
static char s_aszCard3Texts[EM_DROPDOWN_PAGE3_MAX][DROPDOWN_TEXT_BUF_SIZE];

// 在某个初始化函数中（例如 MenuDropdown_Init 中调用）
void MenuDropdown_InitTexts(void)
{
    INT32 iLang = GetLanguage();

    /* 第一页卡片文本：GetStr 后必须立即拷贝，否则下一次 GetStr 会覆盖缓冲区 */
    snprintf(s_aszCardTexts[EM_DROPDOWN_ALARM_PAUSE], sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_ALARM_VOICE_PAUSE, iLang)); /* "报警暂停" */
    snprintf(s_aszCardTexts[EM_DROPDOWN_PURGE],       sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_PURGE,            iLang)); /* "排气"     */
    snprintf(s_aszCardTexts[EM_DROPDOWN_STANDBY],     sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_STANDBY,          iLang)); /* "待机"     */
    snprintf(s_aszCardTexts[EM_DROPDOWN_ACCUMULATE],  sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_TOTAL_VOL,        iLang)); /* "累积量"   */
    snprintf(s_aszCardTexts[EM_DROPDOWN_SETTING],     sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_SET,              iLang)); /* "设置"     */
    snprintf(s_aszCardTexts[EM_DROPDOWN_LOCK],        sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_DLG_SCR_LOCK,     iLang)); /* "锁屏"     */
    snprintf(s_aszCardTexts[EM_DROPDOWN_LARGE_FONT],  sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_LARGE_FONT,       iLang)); /* "大字体"   */
    snprintf(s_aszCardTexts[EM_DROPDOWN_CLEAN],       sizeof(s_aszCardTexts[0]), "%s", GetStr(IDS_CLEAN_OPERATION,  iLang)); /* "清洁"     */

    /* 第二页卡片文本 */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_MAIN_DRUG],    sizeof(s_aszCard2Texts[0]), "%s", GetStr(IDS_PIGGYBACK,     iLang)); /* "主副药"   */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_TREND_CHART],  sizeof(s_aszCard2Texts[0]), "%s", GetStr(IDS_CURVE,         iLang)); /* "趋势图"   */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_ALARM_VOLUME], sizeof(s_aszCard2Texts[0]), "%s", GetStr(IDS_DLG_ALARM_VOL, iLang)); /* "报警音量" */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_BRIGHTNESS],   sizeof(s_aszCard2Texts[0]), "%s", GetStr(IDS_DISLCDL,       iLang)); /* "亮度"     */
    /* "注射器视图"、"联机" 无对应 IDS，保留临时字面量 */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_SYRINGE_VIEW], sizeof(s_aszCard2Texts[0]), "%s", "注射器视图"); /* TODO: Missing IDS_SYRINGE_VIEW */
    snprintf(s_aszCard2Texts[EM_DROPDOWN_PAGE2_ONLINE],       sizeof(s_aszCard2Texts[0]), "%s", "联机");        /* TODO: Missing IDS_ONLINE */

    /* 第三页卡片文本（扩展预留） */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT1], sizeof(s_aszCard3Texts[0]), "%s", "关机");   /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT2], sizeof(s_aszCard3Texts[0]), "%s", "常用治疗方案");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT3], sizeof(s_aszCard3Texts[0]), "%s", "扩展3");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT4], sizeof(s_aszCard3Texts[0]), "%s", "扩展4");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT5], sizeof(s_aszCard3Texts[0]), "%s", "扩展5");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT6], sizeof(s_aszCard3Texts[0]), "%s", "扩展6");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT7], sizeof(s_aszCard3Texts[0]), "%s", "扩展7");  /* TODO: Pending feature definition */
    snprintf(s_aszCard3Texts[EM_DROPDOWN_PAGE3_EXT8], sizeof(s_aszCard3Texts[0]), "%s", "扩展8");  /* TODO: Pending feature definition */
}






/* 第二页大卡片默认数值 */
static const char *s_apCard2Values[EM_DROPDOWN_PAGE2_MAX] =
{
    [EM_DROPDOWN_PAGE2_ALARM_VOLUME] = "8",
    [EM_DROPDOWN_PAGE2_BRIGHTNESS]   = "8"
};

/* 第二页大卡片滑动条范围 */
#define PAGE2_SLIDER_MIN        1
#define PAGE2_SLIDER_MAX        10
#define PAGE2_BRIGHTNESS_DEFAULT 9

/*********************
 *      ENUM
 *********************/

/* 下拉菜单动画状态 */
typedef enum
{
    EM_DROPDOWN_STATE_HIDDEN  = 0,   /* 隐藏状态 (y = -280) */
    EM_DROPDOWN_STATE_ANIMATING,     /* 动画进行中 */
    EM_DROPDOWN_STATE_VISIBLE        /* 显示状态 (y = 0) */
} EM_DROPDOWN_STATE;

/* 下拉菜单页面索引 */
typedef enum
{
    EM_DROPDOWN_PAGE_1 = 0,          /* 第一页：快捷功能 */
    EM_DROPDOWN_PAGE_2 = 1,          /* 第二页：系统功能 */
    EM_DROPDOWN_PAGE_3 = 2,          /* 第三页：扩展功能 */
    EM_DROPDOWN_PAGE_MAX
} EM_DROPDOWN_PAGE;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void _cbDropdownEvent(lv_event_t *pEvent);
static void _cbDropdownCardClick(lv_event_t *pEvent);
static void _cbDropdownCard2Click(lv_event_t *pEvent);
static void _cbDropdownCard3Click(lv_event_t *pEvent);
static void _cbPanelPress(lv_event_t *pEvent);
static void _cbPanelRelease(lv_event_t *pEvent);
static void _createDropdownCard(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY);
static void _createDropdownCard2(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY);
static void _createDropdownCard3(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY);
static void _createDragIndicator(void);
static void _updatePageIndicator(UINT8 u8ActivePage);
static void _animShowFinished(lv_anim_t *pAnim);
static void _animHideFinished(lv_anim_t *pAnim);
static lv_obj_t *_getDesktopScreen(void);
static void _switchToPage(UINT8 u8Page);
static void _cbAlarmVolumeChanged(lv_event_t *pEvent);
static void _cbBrightnessChanged(lv_event_t *pEvent);
static const char *_getDropdownCardIcon(UINT8 eCardId);
static const char *_getDropdownCard2Icon(UINT8 eCardId);
static const char *_getDropdownCard3Icon(UINT8 eCardId);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *pDropdownPanel    = NULL;     /* 下拉菜单主面板 */
static lv_obj_t *pDropdownOverlay  = NULL;     /* 遮罩层(点击空白关闭) */
static lv_obj_t *apDropdownCards[EM_DROPDOWN_MAX] = {NULL}; /* 第一页卡片指针数组 */
static lv_obj_t *apDropdownCards2[EM_DROPDOWN_PAGE2_MAX] = {NULL}; /* 第二页卡片指针数组 */
static lv_obj_t *apDropdownCards3[EM_DROPDOWN_PAGE3_MAX] = {NULL}; /* 第三页卡片指针数组 */
static lv_obj_t *pDropdownContent  = NULL;     /* 滑动内容容器 */
static lv_obj_t *pPageContainer1   = NULL;      /* 第一页容器 */
static lv_obj_t *pPageContainer2   = NULL;      /* 第二页容器 */
static lv_obj_t *pPageContainer3   = NULL;      /* 第三页容器 */
static lv_obj_t *pPageIndicator    = NULL;      /* 翻页指示器（PageIndicator 组件） */
static lv_anim_t s_stAnim;                     /* 动画对象 */
static lv_anim_t s_stPageAnim;                  /* 页面切换动画对象 */
static EM_DROPDOWN_STATE s_eState = EM_DROPDOWN_STATE_HIDDEN; /* 当前状态 */
static UINT8 s_u8CurrentPage = EM_DROPDOWN_PAGE_1; /* 当前页面索引 */
static INT16 s_i16SlideStartX = 0;              /* 滑动起始X坐标 */
static bool s_bSlideHandled = true;             /* 滑动是否已处理 */
static lv_obj_t *pAlarmVolumeSlider = NULL;     /* 报警音量滑动条 */
static lv_obj_t *pAlarmVolumeValLabel = NULL;   /* 报警音量数值标签 */
static lv_obj_t *pBrightnessSlider = NULL;      /* 亮度滑动条 */
static lv_obj_t *pBrightnessValLabel = NULL;    /* 亮度数值标签 */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

//========================================================
// Function: CreateDropdownMenu
// Description: 创建下拉菜单面板（仅创建，初始隐藏）
// Input：      无
// Output:      无
// Return:      无
//========================================================
void CreateDropdownMenu(void)
{
    /* 避免重复创建 */
    if (lv_obj_is_valid(pDropdownPanel))
    {
        return;
    }

    /* 初始化多语言文本 */
    MenuDropdown_InitTexts();

    lv_obj_t *pParent = _getDesktopScreen();

    /* ===== 1. 创建遮罩层(半透明黑色背景，点击关闭，不覆盖状态栏) ===== */
    pDropdownOverlay = lv_obj_create(pParent);
    lv_obj_set_size(pDropdownOverlay, WIN7_WIDTH, DROPDOWN_CONTENT_H);
    lv_obj_set_pos(pDropdownOverlay, 0, WIN7_STATUS_H);
    lv_obj_set_style_bg_color(pDropdownOverlay, lv_color_hex(PUMP_BG_BLACK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pDropdownOverlay, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_border_width(pDropdownOverlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pDropdownOverlay, 0, LV_PART_MAIN);
    lv_obj_add_flag(pDropdownOverlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(pDropdownOverlay, _cbDropdownEvent, LV_EVENT_CLICKED, NULL);

    /* ===== 2. 创建下拉面板容器（从状态栏下方开始，不遮挡状态栏） ===== */
    pDropdownPanel = lv_obj_create(pParent);
    lv_obj_set_size(pDropdownPanel, WIN7_WIDTH, DROPDOWN_CONTENT_H);
    /* 初始位置: 隐藏在屏幕上方（状态栏上方） */
    lv_obj_set_pos(pDropdownPanel, 0, -DROPDOWN_CONTENT_H);
    lv_obj_set_style_bg_color(pDropdownPanel, lv_color_hex(PUMP_BAR_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pDropdownPanel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(pDropdownPanel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pDropdownPanel, 0, LV_PART_MAIN);
    lv_obj_add_flag(pDropdownPanel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(pDropdownPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(pDropdownPanel, _cbDropdownEvent, LV_EVENT_CLICKED, NULL);
    /* 添加滑动按下/释放事件 */
    lv_obj_add_event_cb(pDropdownPanel, _cbPanelPress, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(pDropdownPanel, _cbPanelRelease, LV_EVENT_RELEASED, NULL);

    /* ===== 3. 创建滑动内容容器(三页) ===== */
    pDropdownContent = lv_obj_create(pDropdownPanel);
    lv_obj_set_size(pDropdownContent, WIN7_WIDTH * 3, DROPDOWN_CONTENT_H - 20);  /* 三页宽度 */
    lv_obj_set_pos(pDropdownContent, 0, 0);
    lv_obj_set_style_bg_opa(pDropdownContent, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pDropdownContent, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pDropdownContent, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pDropdownContent, LV_OBJ_FLAG_SCROLLABLE);

    /* ===== 4. 创建第一页容器 ===== */
    pPageContainer1 = lv_obj_create(pDropdownContent);
    lv_obj_set_size(pPageContainer1, WIN7_WIDTH, DROPDOWN_CONTENT_H - 20);
    lv_obj_set_pos(pPageContainer1, 0, 0);
    lv_obj_set_style_bg_opa(pPageContainer1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pPageContainer1, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pPageContainer1, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pPageContainer1, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建第一页8张功能卡片 */
    INT16 u16PosX;
    INT16 u16PosY = DROPDOWN_START_Y;
    UINT8 eCardId;

    for (eCardId = 0; eCardId < EM_DROPDOWN_MAX; eCardId++)
    {
        u16PosX = DROPDOWN_START_X + (INT16)((DROPDOWN_CARD_W + DROPDOWN_CARD_GAP) * eCardId);
        _createDropdownCard(eCardId, u16PosX, u16PosY);
    }

    /* ===== 5. 创建第二页容器 ===== */
    pPageContainer2 = lv_obj_create(pDropdownContent);
    lv_obj_set_size(pPageContainer2, WIN7_WIDTH, DROPDOWN_CONTENT_H - 20);
    lv_obj_set_pos(pPageContainer2, WIN7_WIDTH, 0);
    lv_obj_set_style_bg_opa(pPageContainer2, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pPageContainer2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pPageContainer2, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pPageContainer2, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建第二页6张功能卡片（4小 + 2大） */
    u16PosX = DROPDOWN_PAGE2_START_X;
    for (eCardId = 0; eCardId < EM_DROPDOWN_PAGE2_MAX; eCardId++)
    {
        _createDropdownCard2(eCardId, u16PosX, DROPDOWN_PAGE2_START_Y);
        /* 计算下一张卡片X坐标：根据当前卡片宽度递增 */
        if (eCardId < EM_DROPDOWN_PAGE2_ALARM_VOLUME)
        {
            u16PosX += (INT16)(DROPDOWN_PAGE2_CARD_W + DROPDOWN_PAGE2_CARD_GAP);
        }
        else
        {
            u16PosX += (INT16)(DROPDOWN_PAGE2_LARGE_W + DROPDOWN_PAGE2_CARD_GAP);
        }
    }

    /* ===== 5.5 创建第三页容器 ===== */
    pPageContainer3 = lv_obj_create(pDropdownContent);
    lv_obj_set_size(pPageContainer3, WIN7_WIDTH, DROPDOWN_CONTENT_H - 20);
    lv_obj_set_pos(pPageContainer3, WIN7_WIDTH * 2, 0);
    lv_obj_set_style_bg_opa(pPageContainer3, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pPageContainer3, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pPageContainer3, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pPageContainer3, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建第三页8张扩展卡片（参考第一页布局） */
    u16PosY = DROPDOWN_START_Y;
    for (eCardId = 0; eCardId < EM_DROPDOWN_PAGE3_MAX; eCardId++)
    {
        u16PosX = DROPDOWN_START_X + (INT16)((DROPDOWN_CARD_W + DROPDOWN_CARD_GAP) * eCardId);
        _createDropdownCard3(eCardId, u16PosX, u16PosY);
    }

    /* ===== 6. 创建页面指示器（使用 PageIndicator 通用组件） ===== */
    pPageIndicator = PageIndicator_Create(pDropdownPanel, EM_DROPDOWN_PAGE_MAX);
    if (NULL != pPageIndicator)
    {
        lv_obj_set_pos(pPageIndicator, 0, DROPDOWN_INDICATOR_Y);
        /* PageIndicator 宽度由 LV_PCT(100) 自动撑满，内部 flex 居中排列 */
        /* 背景透明：面板颜色已由 pDropdownPanel 提供 */
        lv_obj_set_style_bg_opa(pPageIndicator, LV_OPA_TRANSP, LV_PART_MAIN);
    }

    /* ===== 8. 创建底部拖拽指示条 ===== */
    _createDragIndicator();

    /* 初始隐藏面板和遮罩 */
    lv_obj_add_flag(pDropdownPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(pDropdownOverlay, LV_OBJ_FLAG_HIDDEN);

    /* 重置到第一页（内容 + 状态 + 指示器） */
    s_u8CurrentPage = EM_DROPDOWN_PAGE_1;
    lv_obj_set_pos(pDropdownContent, 0, 0);
    _updatePageIndicator(EM_DROPDOWN_PAGE_1);
}

//========================================================
// Function: ShowDropdownMenu
// Description: 下拉动画显示菜单
// Input：      无
// Output:      无
// Return:      无
//========================================================
void ShowDropdownMenu(void)
{
    if (!lv_obj_is_valid(pDropdownPanel))
    {
        CreateDropdownMenu();
    }

    if (s_eState == EM_DROPDOWN_STATE_VISIBLE
        || s_eState == EM_DROPDOWN_STATE_ANIMATING)
    {
        return;
    }

    s_eState = EM_DROPDOWN_STATE_ANIMATING;

    /* 显示面板并置顶 */
    lv_obj_clear_flag(pDropdownPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(pDropdownOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(pDropdownOverlay);
    lv_obj_move_foreground(pDropdownPanel);

    /* 下拉动画: y 从 -240 滑动到状态栏下方(40) */
    lv_anim_init(&s_stAnim);
    lv_anim_set_var(&s_stAnim, pDropdownPanel);
    lv_anim_set_exec_cb(&s_stAnim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&s_stAnim, -DROPDOWN_CONTENT_H, WIN7_STATUS_H);
    lv_anim_set_time(&s_stAnim, DROPDOWN_ANIM_TIME);
    lv_anim_set_path_cb(&s_stAnim, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&s_stAnim, _animShowFinished);
    lv_anim_start(&s_stAnim);
}

//========================================================
// Function: HideDropdownMenu
// Description: 上拉动画收起菜单
// Input：      无
// Output:      无
// Return:      无
//========================================================
void HideDropdownMenu(void)
{
    if (!lv_obj_is_valid(pDropdownPanel))
    {
        return;
    }

    if (s_eState == EM_DROPDOWN_STATE_HIDDEN
        || s_eState == EM_DROPDOWN_STATE_ANIMATING)
    {
        return;
    }

    s_eState = EM_DROPDOWN_STATE_ANIMATING;

    /* 上拉动画: y 从状态栏下方回退隐藏 */
    lv_anim_init(&s_stAnim);
    lv_anim_set_var(&s_stAnim, pDropdownPanel);
    lv_anim_set_exec_cb(&s_stAnim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&s_stAnim, WIN7_STATUS_H, -DROPDOWN_CONTENT_H);
    lv_anim_set_time(&s_stAnim, DROPDOWN_ANIM_TIME);
    lv_anim_set_path_cb(&s_stAnim, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&s_stAnim, _animHideFinished);
    lv_anim_start(&s_stAnim);
}

//========================================================
// Function: HideDropdownMenuNow
// Description: 立即隐藏下拉菜单（无动画），用于页面跳转同步关闭
// Input：      无
// Output:      无
// Return:      无
//========================================================
void HideDropdownMenuNow(void)
{
    if (!lv_obj_is_valid(pDropdownPanel))
    {
        return;
    }

    /* 停止可能正在进行的动画 */
    lv_anim_del(pDropdownPanel, (lv_anim_exec_xcb_t)lv_obj_set_y);

    /* 立即设置隐藏状态 */
    s_eState = EM_DROPDOWN_STATE_HIDDEN;

    lv_obj_add_flag(pDropdownPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(pDropdownOverlay, LV_OBJ_FLAG_HIDDEN);

    /* 重置到第一页（内容 + 状态 + 指示器） */
    s_u8CurrentPage = EM_DROPDOWN_PAGE_1;
    lv_obj_set_pos(pDropdownContent, 0, 0);
    _updatePageIndicator(EM_DROPDOWN_PAGE_1);
}

//========================================================
// Function: ToggleDropdownMenu
// Description: 切换下拉菜单显示/隐藏状态
// Input：      无
// Output:      无
// Return:      无
//========================================================
void ToggleDropdownMenu(void)
{
    if (s_eState == EM_DROPDOWN_STATE_VISIBLE)
    {
        HideDropdownMenu();
    }
    else if (s_eState == EM_DROPDOWN_STATE_HIDDEN)
    {
        ShowDropdownMenu();
    }
    /* 动画进行中则忽略 */
}

//========================================================
// Function: IsDropdownMenuVisible
// Description: 查询下拉菜单可见状态
// Input：      无
// Output:      无
// Return:      TRUE=可见, FALSE=隐藏
//========================================================
BOOL IsDropdownMenuVisible(void)
{
    return (s_eState == EM_DROPDOWN_STATE_VISIBLE) ? TRUE : FALSE;
}

//========================================================
// Function: DropdownSwitchToPage
// Description: 切换到指定页面的下拉菜单
// Input：      u8Page - 页面索引 (0=第一页, 1=第二页)
// Output:      无
// Return:      无
//========================================================
void DropdownSwitchToPage(UINT8 u8Page)
{
    if (s_eState != EM_DROPDOWN_STATE_VISIBLE)
    {
        /* 如果菜单未显示，先显示菜单并切换到指定页 */
        if (!lv_obj_is_valid(pDropdownPanel))
        {
            CreateDropdownMenu();
        }
        ShowDropdownMenu();
        /* 延迟切换页面，等显示动画完成 */
        /* 简单处理：直接切换到目标页 */
        _switchToPage(u8Page);
    }
    else
    {
        _switchToPage(u8Page);
    }
}

//========================================================
// Function: DropdownGetCurrentPage
// Description: 获取当前页面索引
// Input：      无
// Output:      无
// Return:      当前页面索引
//========================================================
UINT8 DropdownGetCurrentPage(void)
{
    return s_u8CurrentPage;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

//========================================================
// Function: _getDropdownCardIcon
// Description: 通过枚举 ID 获取第一页卡片图标占位符文本
// Input：      eCardId 卡片标识枚举
// Return:      图标占位符字符串
//========================================================
static const char *_getDropdownCardIcon(UINT8 eCardId)
{
    switch (eCardId)
    {
    case EM_DROPDOWN_ALARM_PAUSE:  return HOME_ICON_PULL_MENU_ALARM_MUTE;
    case EM_DROPDOWN_PURGE:        return HOME_ICON_PULL_MENU_BOLUS;
    case EM_DROPDOWN_STANDBY:      return HOME_ICON_PULL_MENU_SHUTDOWN;
    case EM_DROPDOWN_ACCUMULATE:   return HOME_ICON_PULL_MENU_CUMULANT;
    case EM_DROPDOWN_SETTING:      return HOME_ICON_PULL_MENU_SETTING;
    case EM_DROPDOWN_LOCK:         return STATUSBAR_ICON_LOCKSCREEN;
    case EM_DROPDOWN_LARGE_FONT:   return HOME_ICON_PULL_MENU_LARGE_FONT;
    case EM_DROPDOWN_CLEAN:        return HOME_ICON_PULL_MENU_CLEAN;
    default:                       return "?";
    }
}


//========================================================
// Function: _getDropdownCard2Icon
// Description: 通过枚举 ID 获取第二页卡片图标占位符文本
// Input：      eCardId 卡片标识枚举
// Return:      图标占位符字符串
//========================================================
static const char *_getDropdownCard2Icon(UINT8 eCardId)
{
    switch (eCardId)
    {
    case EM_DROPDOWN_PAGE2_SYRINGE_VIEW: return HOME_ICON_PULL_MENU_SYRINGE;
    case EM_DROPDOWN_PAGE2_MAIN_DRUG:    return HOME_ICON_PULL_MENU_PIGGY;
    case EM_DROPDOWN_PAGE2_ONLINE:       return HOME_ICON_PULL_MENU_ONLINE;
    case EM_DROPDOWN_PAGE2_TREND_CHART:  return HOME_ICON_PULL_MENU_TRENDS;
    case EM_DROPDOWN_PAGE2_ALARM_VOLUME: return HOME_ICON_PULL_MENU_SOUND1;
    case EM_DROPDOWN_PAGE2_BRIGHTNESS:   return HOME_ICON_PULL_MENU_BRIGHTNESS1;
    default:                             return HOME_ICON_BOLUS_PROMPT;
    }
}

//========================================================
// Function: _getDropdownCard3Icon
// Description: 通过枚举 ID 获取第三页卡片图标占位符文本
// Input：      eCardId 卡片标识枚举
// Return:      图标占位符字符串
//========================================================
static const char *_getDropdownCard3Icon(UINT8 eCardId)
{
    (void)eCardId;
    /* TODO: 扩展页面图标待分配，暂用设置图标占位 */
    return HOME_ICON_PULL_MENU_SETTING;
}

//========================================================
// Function: _createDropdownCard
// Description: 创建单个下拉菜单功能卡片（第一页）
// Input：      eCardId   卡片标识枚举
//              u16PosX   卡片X坐标
//              u16PosY   卡片Y坐标
// Output:      无
// Return:      无
//========================================================
static void _createDropdownCard(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY)
{
    if (eCardId >= EM_DROPDOWN_MAX)
    {
        return;
    }

    lv_obj_t *pCard = lv_obj_create(pPageContainer1);
    if (!pCard)
    {
        return;
    }

    /* 卡片基础尺寸和位置 */
    lv_obj_set_size(pCard, DROPDOWN_CARD_W, DROPDOWN_CARD_H);
    lv_obj_set_pos(pCard, u16PosX, u16PosY);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pCard, CARD_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(pCard, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pCard, CARD_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pCard, 0, LV_PART_MAIN);
    lv_obj_add_flag(pCard, LV_OBJ_FLAG_CLICKABLE);

    /* 卡片按压效果 */
    lv_obj_set_style_bg_opa(pCard, LV_OPA_20, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_PRESSED);

    /* 绑定卡片点击回调 */
    lv_obj_add_event_cb(pCard, _cbDropdownCardClick, LV_EVENT_CLICKED, NULL);
    /* 绑定滑动按压/释放回调（解决卡片拦截触摸导致面板滑动失效） */
    lv_obj_add_event_cb(pCard, _cbPanelPress, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(pCard, _cbPanelRelease, LV_EVENT_RELEASED, NULL);
    /* 通过 user_data 存储卡片 ID */
    lv_obj_set_user_data(pCard, (void *)(uintptr_t)eCardId);

    /* ===== 图标圆形背景 ===== */
    lv_obj_t *pIconCircle = lv_obj_create(pCard);
    /* 图标圆形相对于卡片居中: (158-72)/2 = 43 */
    lv_obj_set_size(pIconCircle, CARD_ICON_CIRCLE_SIZE, CARD_ICON_CIRCLE_SIZE);
    lv_obj_set_pos(pIconCircle, (DROPDOWN_CARD_W - CARD_ICON_CIRCLE_SIZE) / 2, CARD_ICON_CIRCLE_Y);
    lv_obj_set_style_bg_color(pIconCircle, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pIconCircle, CARD_ICON_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(pIconCircle, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pIconCircle, CARD_ICON_CIRCLE_SIZE / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pIconCircle, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pIconCircle, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* ===== 图标图片 ===== */
    lv_obj_t *pIconImg = lv_img_create(pIconCircle);
    lv_img_set_src(pIconImg, _getDropdownCardIcon(eCardId));
    lv_obj_center(pIconImg);


    /* ===== 卡片底部文字标签 ===== */
    lv_obj_t *pTextLabel = lv_label_create(pCard);
    lv_label_set_text(pTextLabel, s_aszCardTexts[eCardId]);
    lv_obj_set_style_text_color(pTextLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(pTextLabel, GetFont(EM_FONT_TEXT_28), LV_PART_MAIN);
    lv_obj_set_width(pTextLabel, DROPDOWN_CARD_W);
    lv_obj_set_style_text_align(pTextLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(pTextLabel, LV_ALIGN_TOP_MID, 0, CARD_TEXT_Y);

    /* 保存指针 */
    apDropdownCards[eCardId] = pCard;
}

//========================================================
// Function: _createDropdownCard2
// Description: 创建第二页下拉菜单功能卡片（4小卡 + 2大卡）
//              大卡片(报警音量/亮度)含控制条和数值显示
// Input：      eCardId   卡片标识枚举
//              u16PosX   卡片X坐标
//              u16PosY   卡片Y坐标
// Output:      无
// Return:      无
//========================================================
static void _createDropdownCard2(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY)
{
    if (eCardId >= EM_DROPDOWN_PAGE2_MAX)
    {
        return;
    }

    BOOL bIsLarge = (eCardId >= EM_DROPDOWN_PAGE2_ALARM_VOLUME) ? TRUE : FALSE;
    UINT16 u16CardW = bIsLarge ? DROPDOWN_PAGE2_LARGE_W : DROPDOWN_PAGE2_CARD_W;

    lv_obj_t *pCard = lv_obj_create(pPageContainer2);
    if (!pCard)
    {
        return;
    }

    /* 卡片基础尺寸和位置 */
    lv_obj_set_size(pCard, u16CardW, DROPDOWN_PAGE2_CARD_H);
    lv_obj_set_pos(pCard, u16PosX, u16PosY);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pCard, CARD_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(pCard, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pCard, CARD_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pCard, 0, LV_PART_MAIN);
    lv_obj_add_flag(pCard, LV_OBJ_FLAG_CLICKABLE);

    /* 卡片按压效果 */
    lv_obj_set_style_bg_opa(pCard, LV_OPA_20, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_PRESSED);

    /* 绑定卡片点击回调 */
    lv_obj_add_event_cb(pCard, _cbDropdownCard2Click, LV_EVENT_CLICKED, NULL);
    /* 绑定滑动按压/释放回调 */
    lv_obj_add_event_cb(pCard, _cbPanelPress, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(pCard, _cbPanelRelease, LV_EVENT_RELEASED, NULL);
    /* 通过 user_data 存储卡片 ID */
    lv_obj_set_user_data(pCard, (void *)(uintptr_t)eCardId);

    if (bIsLarge)
    {
        /* ===== 大卡片布局 (332x188) ===== */
        /* 标题文字 (左上) */
        lv_obj_t *pTitleLabel = lv_label_create(pCard);
        lv_label_set_text(pTitleLabel, s_aszCard2Texts[eCardId]);
        lv_obj_set_style_text_color(pTitleLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_font(pTitleLabel, GetFont(EM_FONT_TEXT_28), LV_PART_MAIN);
        lv_obj_set_pos(pTitleLabel, DROPDOWN_PAGE2_ICON_X, DROPDOWN_PAGE2_TITLE_Y);

        /* 数值文字 (右上) */
        const char *pVal = s_apCard2Values[eCardId];
        lv_obj_t *pValueLabel = NULL;
        if (pVal != NULL)
        {
            pValueLabel = lv_label_create(pCard);
            lv_label_set_text(pValueLabel, pVal);
            lv_obj_set_style_text_color(pValueLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
            lv_obj_set_style_text_font(pValueLabel, GetFont(EM_FONT_PARAM_30), LV_PART_MAIN);
            lv_obj_set_pos(pValueLabel,
                (INT16)(u16CardW - DROPDOWN_PAGE2_VALUE_RIGHT),
                DROPDOWN_PAGE2_TITLE_Y);
        }

/* 控制条（报警音量和亮度均使用交互式滑动条） */
            if (EM_DROPDOWN_PAGE2_ALARM_VOLUME == eCardId
                || EM_DROPDOWN_PAGE2_BRIGHTNESS == eCardId)
        {
            /* ===== 交互式滑动条 ===== */
            lv_obj_t *pSlider = lv_slider_create(pCard);
            lv_obj_set_size(pSlider, DROPDOWN_PAGE2_BAR_W, DROPDOWN_PAGE2_BAR_H);
            lv_obj_set_pos(pSlider, DROPDOWN_PAGE2_ICON_X, DROPDOWN_PAGE2_BAR_Y);
            /* 滑动条范围 */
            lv_slider_set_range(pSlider, PAGE2_SLIDER_MIN, PAGE2_SLIDER_MAX);
            /* 默认值：报警音量 8，亮度 9 */
            if (EM_DROPDOWN_PAGE2_ALARM_VOLUME == eCardId)
            {
                lv_slider_set_value(pSlider, 8, LV_ANIM_OFF);
            }
            else
            {
                lv_slider_set_value(pSlider, PAGE2_BRIGHTNESS_DEFAULT, LV_ANIM_OFF);
            }
            /* 背景轨道样式：半透明白色，圆角 */
            lv_obj_set_style_bg_color(pSlider, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(pSlider, LV_OPA_10, LV_PART_MAIN);
            lv_obj_set_style_radius(pSlider, 12, LV_PART_MAIN);
            lv_obj_set_style_pad_all(pSlider, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(pSlider, 0, LV_PART_MAIN);  // 移除边框

            /* 指示器（已填充部分）样式：不透明白色 */
            lv_obj_set_style_bg_color(pSlider, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_INDICATOR);
            lv_obj_set_style_bg_opa(pSlider, LV_OPA_COVER, LV_PART_INDICATOR);
            lv_obj_set_style_radius(pSlider, 12, LV_PART_INDICATOR);
            /* 隐藏滑块旋钮（仅保留轨道和指示器，实现纯滑动条效果） */
            lv_obj_set_style_pad_all(pSlider, 0, LV_PART_KNOB);
            lv_obj_set_style_bg_opa(pSlider, LV_OPA_TRANSP, LV_PART_KNOB);
            /* 绑定值变化事件并存储句柄 */
            if (EM_DROPDOWN_PAGE2_ALARM_VOLUME == eCardId)
            {
                lv_obj_add_event_cb(pSlider, _cbAlarmVolumeChanged, LV_EVENT_VALUE_CHANGED, NULL);
                pAlarmVolumeSlider = pSlider;
                if (pValueLabel != NULL)
                {
                    lv_label_set_text(pValueLabel, "8");
                    pAlarmVolumeValLabel = pValueLabel;
                }
            }
            else
            {
                lv_obj_add_event_cb(pSlider, _cbBrightnessChanged, LV_EVENT_VALUE_CHANGED, NULL);
                pBrightnessSlider = pSlider;
                if (pValueLabel != NULL)
                {
                    char szBuf[4];
                    snprintf(szBuf, sizeof(szBuf), "%d", PAGE2_BRIGHTNESS_DEFAULT);
                    lv_label_set_text(pValueLabel, szBuf);
                    pBrightnessValLabel = pValueLabel;
                }
            }
        }

        /* 图标区域 (控制条左侧) */
        lv_obj_t *pIconBg = lv_obj_create(pCard);
        lv_obj_set_size(pIconBg, DROPDOWN_PAGE2_ICON_SIZE, DROPDOWN_PAGE2_ICON_SIZE);
        lv_obj_set_pos(pIconBg, DROPDOWN_PAGE2_ICON_X + 4, DROPDOWN_PAGE2_ICON_Y);
        lv_obj_set_style_bg_color(pIconBg, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(pIconBg, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(pIconBg, 0, LV_PART_MAIN);
        lv_obj_set_style_border_color(pIconBg, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_pad_all(pIconBg, 0, LV_PART_MAIN);
        lv_obj_clear_flag(pIconBg, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);




        lv_obj_t *pIconImg = lv_img_create(pIconBg);
        lv_img_set_src(pIconImg, _getDropdownCard2Icon(eCardId));
        lv_obj_center(pIconImg);
    }
    else
    {
        /* ===== 小卡片布局 (158x188) ===== */
        /* 图标圆形背景 */
        lv_obj_t *pIconCircle = lv_obj_create(pCard);
        lv_obj_set_size(pIconCircle, CARD_ICON_CIRCLE_SIZE, CARD_ICON_CIRCLE_SIZE);
        lv_obj_set_pos(pIconCircle, (DROPDOWN_PAGE2_CARD_W - CARD_ICON_CIRCLE_SIZE) / 2, CARD_ICON_CIRCLE_Y);
        lv_obj_set_style_bg_color(pIconCircle, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(pIconCircle, CARD_ICON_BG_OPA, LV_PART_MAIN);
        lv_obj_set_style_border_width(pIconCircle, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(pIconCircle, CARD_ICON_CIRCLE_SIZE / 2, LV_PART_MAIN);
        lv_obj_set_style_pad_all(pIconCircle, 0, LV_PART_MAIN);
        lv_obj_clear_flag(pIconCircle, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        /* 图标占位标签 */

        lv_obj_t *pIconImg = lv_img_create(pIconCircle);
        lv_img_set_src(pIconImg, _getDropdownCard2Icon(eCardId));
        lv_obj_center(pIconImg);


        /* 卡片底部文字标签 */
        lv_obj_t *pTextLabel = lv_label_create(pCard);
        lv_label_set_text(pTextLabel, s_aszCard2Texts[eCardId]);
        lv_obj_set_style_text_color(pTextLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_font(pTextLabel, GetFont(EM_FONT_TEXT_28), LV_PART_MAIN);
        lv_obj_set_width(pTextLabel, DROPDOWN_PAGE2_CARD_W);
        lv_obj_set_style_text_align(pTextLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(pTextLabel, LV_ALIGN_TOP_MID, 0, CARD_TEXT_Y);
    }

    /* 保存指针 */
    apDropdownCards2[eCardId] = pCard;
}

//========================================================
// Function: _createDropdownCard3
// Description: 创建第三页下拉菜单扩展卡片（参考第一页布局）
// Input：      eCardId   卡片标识枚举
//              u16PosX   卡片X坐标
//              u16PosY   卡片Y坐标
// Output:      无
// Return:      无
//========================================================
static void _createDropdownCard3(UINT8 eCardId, INT16 u16PosX, INT16 u16PosY)
{
    if (eCardId >= EM_DROPDOWN_PAGE3_MAX)
    {
        return;
    }

    lv_obj_t *pCard = lv_obj_create(pPageContainer3);
    if (!pCard)
    {
        return;
    }

    /* 卡片基础尺寸和位置 */
    lv_obj_set_size(pCard, DROPDOWN_CARD_W, DROPDOWN_CARD_H);
    lv_obj_set_pos(pCard, u16PosX, u16PosY);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pCard, CARD_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(pCard, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pCard, CARD_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pCard, 0, LV_PART_MAIN);
    lv_obj_add_flag(pCard, LV_OBJ_FLAG_CLICKABLE);

    /* 卡片按压效果 */
    lv_obj_set_style_bg_opa(pCard, LV_OPA_20, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pCard, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_PRESSED);

    /* 绑定卡片点击回调 */
    lv_obj_add_event_cb(pCard, _cbDropdownCard3Click, LV_EVENT_CLICKED, NULL);
    /* 绑定滑动按压/释放回调（解决卡片拦截触摸导致面板滑动失效） */
    lv_obj_add_event_cb(pCard, _cbPanelPress, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(pCard, _cbPanelRelease, LV_EVENT_RELEASED, NULL);
    /* 通过 user_data 存储卡片 ID */
    lv_obj_set_user_data(pCard, (void *)(uintptr_t)eCardId);

    /* ===== 图标圆形背景 ===== */
    lv_obj_t *pIconCircle = lv_obj_create(pCard);
    /* 图标圆形相对于卡片居中: (158-72)/2 = 43 */
    lv_obj_set_size(pIconCircle, CARD_ICON_CIRCLE_SIZE, CARD_ICON_CIRCLE_SIZE);
    lv_obj_set_pos(pIconCircle, (DROPDOWN_CARD_W - CARD_ICON_CIRCLE_SIZE) / 2, CARD_ICON_CIRCLE_Y);
    lv_obj_set_style_bg_color(pIconCircle, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pIconCircle, CARD_ICON_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(pIconCircle, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pIconCircle, CARD_ICON_CIRCLE_SIZE / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pIconCircle, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pIconCircle, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* ===== 图标图片 ===== */
    lv_obj_t *pIconImg = lv_img_create(pIconCircle);
    lv_img_set_src(pIconImg, _getDropdownCard3Icon(eCardId));
    lv_obj_center(pIconImg);

    /* ===== 卡片底部文字标签 ===== */
    lv_obj_t *pTextLabel = lv_label_create(pCard);
    lv_label_set_text(pTextLabel, s_aszCard3Texts[eCardId]);
    lv_obj_set_style_text_color(pTextLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(pTextLabel, GetFont(EM_FONT_TEXT_28), LV_PART_MAIN);
    lv_obj_set_width(pTextLabel, DROPDOWN_CARD_W);
    lv_obj_set_style_text_align(pTextLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(pTextLabel, LV_ALIGN_TOP_MID, 0, CARD_TEXT_Y);

    /* 保存指针 */
    apDropdownCards3[eCardId] = pCard;
}

//========================================================
// Function: _createDragIndicator
// Description: 创建底部拖拽指示条
// Input：      无
// Output:      无
// Return:      无
//========================================================
static void _createDragIndicator(void)
{
    lv_obj_t *pDragBar = lv_obj_create(pDropdownPanel);
    if (!pDragBar)
    {
        return;
    }

    INT16 u16DragBarX = (WIN7_WIDTH - DRAG_BAR_W) / 2;  /* 水平居中 */

    lv_obj_set_size(pDragBar, DRAG_BAR_W, DRAG_BAR_H);
    lv_obj_set_pos(pDragBar, u16DragBarX, DRAG_BAR_Y);
    lv_obj_set_style_bg_color(pDragBar, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pDragBar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(pDragBar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(pDragBar, DRAG_BAR_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pDragBar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pDragBar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
}

//========================================================
// Function: _updatePageIndicator
// Description: 更新页面指示器状态（委托给 PageIndicator 组件）
// Input：      u8ActivePage  当前激活的页面索引
// Output:      无
// Return:      无
//========================================================
static void _updatePageIndicator(UINT8 u8ActivePage)
{
    PageIndicator_SetActive(pPageIndicator, u8ActivePage);
}

//========================================================
// Function: _switchToPage
// Description: 切换到指定页面（带动画）
// Input：      u8Page  页面索引
// Output:      无
// Return:      无
//========================================================
static void _switchToPage(UINT8 u8Page)
{
    if (u8Page >= EM_DROPDOWN_PAGE_MAX)
    {
        return;
    }

    if (s_u8CurrentPage == u8Page)
    {
        return;
    }

    if (NULL == pDropdownContent)
    {
        return;
    }

    s_u8CurrentPage = u8Page;

    /* 更新指示器 */
    PageIndicator_SetActive(pPageIndicator, u8Page);

    /* 页面滑动动画 */
    lv_anim_init(&s_stPageAnim);
    lv_anim_set_var(&s_stPageAnim, pDropdownContent);
    lv_anim_set_exec_cb(&s_stPageAnim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&s_stPageAnim, lv_obj_get_x(pDropdownContent), -(INT16)(u8Page * WIN7_WIDTH));
    lv_anim_set_time(&s_stPageAnim, 200);
    lv_anim_set_path_cb(&s_stPageAnim, lv_anim_path_ease_out);
    lv_anim_start(&s_stPageAnim);
}

//========================================================
// Function: _cbPanelPress
// Description: 面板按下回调 - 记录按下位置
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbPanelPress(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    /* 仅在下拉菜单可见时记录按下列位置 */
    if (NULL == pDropdownPanel)
    {
        return;
    }

    /* 获取按下时的触摸点位置 */
    lv_indev_t *pIndev = lv_event_get_indev(pEvent);
    if (NULL == pIndev)
    {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(pIndev, &point);
    s_i16SlideStartX = point.x;
}

//========================================================
// Function: _cbPanelRelease
// Description: 面板释放回调 - 检测滑动方向
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbPanelRelease(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    /* 仅在下拉菜单可见时处理滑动 */
    if (NULL == pDropdownPanel)
    {
        return;
    }

    /* 获取释放时的触摸点位置 */
    lv_indev_t *pIndev = lv_event_get_indev(pEvent);
    if (NULL == pIndev)
    {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(pIndev, &point);

    /* 计算滑动距离 */
    INT16 i16Diff = point.x - s_i16SlideStartX;

    /* 滑动阈值判断（至少滑动50像素才触发） */
    INT16 i16Threshold = 50;

    if (i16Diff > i16Threshold)
    {
        /* 向右滑 -> 切换到上一页 */
        s_bSlideHandled = true;
        if (s_u8CurrentPage > EM_DROPDOWN_PAGE_1)
        {
            _switchToPage(s_u8CurrentPage - 1);
        }
    }
    else if (i16Diff < -i16Threshold)
    {
        /* 向左滑 -> 切换到下一页 */
        s_bSlideHandled = true;
        if (s_u8CurrentPage < (EM_DROPDOWN_PAGE_MAX - 1))
        {
            _switchToPage(s_u8CurrentPage + 1);
        }
    }
    else
    {
        /* 没有滑动，设置标志让卡片点击回调处理 */
        s_bSlideHandled = false;
    }
}

//========================================================
// Function: _cbDropdownEvent
// Description: 下拉菜单面板/遮罩统一事件回调
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbDropdownEvent(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_event_code_t code = lv_event_get_code(pEvent);
    lv_obj_t *pTarget    = lv_event_get_target(pEvent);

    /* 点击遮罩层 -> 关闭下拉菜单 */
    if (pDropdownOverlay == pTarget)
    {
        if (LV_EVENT_CLICKED == code)
        {
            HideDropdownMenu();
        }
    }

    /* 面板删除 -> 清理所有指针 */
    if (pDropdownPanel == pTarget)
    {
        if (LV_EVENT_DELETE == code)
        {
            pDropdownPanel   = NULL;
            pDropdownOverlay = NULL;
            memset(apDropdownCards, 0, sizeof(apDropdownCards));
            memset(apDropdownCards2, 0, sizeof(apDropdownCards2));
            memset(apDropdownCards3, 0, sizeof(apDropdownCards3));
            pDropdownContent = NULL;
            pPageContainer1 = NULL;
            pPageContainer2 = NULL;
            pPageContainer3 = NULL;
            pPageIndicator = NULL;
            pAlarmVolumeSlider   = NULL;
            pAlarmVolumeValLabel = NULL;
            pBrightnessSlider    = NULL;
            pBrightnessValLabel  = NULL;
            s_eState = EM_DROPDOWN_STATE_HIDDEN;
            s_u8CurrentPage = EM_DROPDOWN_PAGE_1;
        }
    }
}

//========================================================
// Function: _cbDropdownCardClick
// Description: 下拉菜单卡片点击回调（第一页）
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbDropdownCardClick(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_obj_t *pTarget = lv_event_get_target(pEvent);
    if (NULL == pTarget)
    {
        return;
    }

    /* 通过 user_data 获取卡片 ID */
    uintptr_t uCardId = (uintptr_t)lv_obj_get_user_data(pTarget);

    /* 第一页卡片点击后立即隐藏菜单(无动画)。
     * 注意: 必须用 HideDropdownMenuNow()，因为下拉菜单在 lv_layer_top() 上，
     * 异步动画期间遮罩层会挡住后续创建的页面窗口。
     * 第二页的报警音量/亮度卡片由 _cbDropdownCard2Click 处理，不会进入此回调。 */
    if (!s_bSlideHandled)
    {
        HideDropdownMenu();
    }

    /* 根据卡片类型执行对应操作 */
    switch ((EM_DROPDOWN_CARD)uCardId)
    {
    case EM_DROPDOWN_ALARM_PAUSE:
        /* TODO: 触发报警暂停逻辑 */
        break;

    case EM_DROPDOWN_PURGE:
        /* 触发排气：弹确认弹窗（确定→进入排气） */
        _MenuDropdownPurge();
        break;

    case EM_DROPDOWN_STANDBY:
        /* TODO: 触发待机逻辑 */
        break;

    case EM_DROPDOWN_ACCUMULATE:
        /* TODO: 跳转到累积量页面 */
        break;

    case EM_DROPDOWN_SETTING:
        /* 打开设置页面 */
        CreateMainMenuWin(NULL);
        break;

    case EM_DROPDOWN_LOCK:
        /* TODO: 触发锁屏逻辑 */
        break;

    case EM_DROPDOWN_LARGE_FONT:
        /* TODO: 切换大字体模式 */
        break;

    case EM_DROPDOWN_CLEAN:
        /* TODO: 触发清洁模式 */
        break;

    default:
        break;
    }
}

//========================================================
// Function: _cbAlarmVolumeChanged
// Description: 报警音量滑动条值变化回调
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbAlarmVolumeChanged(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_obj_t *pSlider = lv_event_get_target(pEvent);
    if (NULL == pSlider)
    {
        return;
    }

    INT16 s16Val = (INT16)lv_slider_get_value(pSlider);

    /* 更新数值标签 */
    if (NULL != pAlarmVolumeValLabel)
    {
        char szBuf[4];
        snprintf(szBuf, sizeof(szBuf), "%d", (int)s16Val);
        lv_label_set_text(pAlarmVolumeValLabel, szBuf);
    }

    /*
     * TODO: 以下功能依赖嵌入式平台 API，待移植到目标平台时启用以完整实现
     *      - GetMinAlarmVolume() 获取最小报警音量
     *      - TaskEventPublish(TASK_EVENT_ID_SET_ALARM_VOL) 发布报警音量事件
     */
}

//========================================================
// Function: _cbBrightnessChanged
// Description: 亮度滑动条值变化回调
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbBrightnessChanged(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_obj_t *pSlider = lv_event_get_target(pEvent);
    if (NULL == pSlider)
    {
        return;
    }

    INT16 s16Val = (INT16)lv_slider_get_value(pSlider);

    /* 更新数值标签 */
    if (NULL != pBrightnessValLabel)
    {
        char szBuf[4];
        snprintf(szBuf, sizeof(szBuf), "%d", (int)s16Val);
        lv_label_set_text(pBrightnessValLabel, szBuf);
    }

    /*
     * TODO: 以下功能依赖嵌入式平台 API，待移植到目标平台时启用以完整实现
     *      - GetBackLight() 获取当前亮度值
     *      - TaskEventPublish(TASK_EVENT_ID_SET_BACKLIGHT) 发布屏幕亮度事件
     */
}

//========================================================
// Function: _cbDropdownCard2Click
// Description: 下拉菜单第二页卡片点击回调
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbDropdownCard2Click(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_obj_t *pTarget = lv_event_get_target(pEvent);
    if (NULL == pTarget)
    {
        return;
    }

    /* 通过 user_data 获取卡片 ID */
    uintptr_t uCardId = (uintptr_t)lv_obj_get_user_data(pTarget);

    /* 只有在没有滑动的情况下才关闭菜单 */
    /* 报警音量和亮度卡片内有交互控件，不关闭菜单 */
    if (!s_bSlideHandled
        && EM_DROPDOWN_PAGE2_ALARM_VOLUME != (EM_DROPDOWN_PAGE2_CARD)uCardId
        && EM_DROPDOWN_PAGE2_BRIGHTNESS != (EM_DROPDOWN_PAGE2_CARD)uCardId)
    {
        HideDropdownMenu();
    }

    /* 根据卡片类型执行对应操作 */
    switch ((EM_DROPDOWN_PAGE2_CARD)uCardId)
    {
    case EM_DROPDOWN_PAGE2_SYRINGE_VIEW:
        /* TODO: 切换到注射器视图 */
        break;

    case EM_DROPDOWN_PAGE2_MAIN_DRUG:
        /* TODO: 跳转到主副药设置页面 */
        break;

    case EM_DROPDOWN_PAGE2_ONLINE:
        /* TODO: 跳转到联机页面 */
        break;

    case EM_DROPDOWN_PAGE2_TREND_CHART:
        /* TODO: 跳转到趋势图页面 */
        break;

    case EM_DROPDOWN_PAGE2_ALARM_VOLUME:
        /* TODO: 打开报警音量调节弹窗 */
        break;

    case EM_DROPDOWN_PAGE2_BRIGHTNESS:
        /* TODO: 打开亮度调节弹窗 */
        break;

    default:
        break;
    }
}

//========================================================
// Function: _cbDropdownCard3Click
// Description: 下拉菜单第三页扩展卡片点击回调
// Input：      pEvent  LVGL事件句柄
// Output:      无
// Return:      无
//========================================================
static void _cbDropdownCard3Click(lv_event_t *pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }

    lv_obj_t *pTarget = lv_event_get_target(pEvent);
    if (NULL == pTarget)
    {
        return;
    }

    /* 通过 user_data 获取卡片 ID */
    uintptr_t uCardId = (uintptr_t)lv_obj_get_user_data(pTarget);

    /* 只有在没有滑动的情况下才关闭菜单 */
    if (!s_bSlideHandled)
    {
        HideDropdownMenuNow();
    }

    /* 根据卡片类型执行对应操作 */
    switch ((EM_DROPDOWN_PAGE3_CARD)uCardId)
    {
    case EM_DROPDOWN_PAGE3_EXT1:
        /* 打开关机/待机确认弹窗 */
        CreateMenuShutdownWin();
        break;

    case EM_DROPDOWN_PAGE3_EXT2:
        /* 打开常用治疗方案页面 */
        CreateTreatmentPlanWin();
        break;

    case EM_DROPDOWN_PAGE3_EXT3:
    case EM_DROPDOWN_PAGE3_EXT4:
    case EM_DROPDOWN_PAGE3_EXT5:
    case EM_DROPDOWN_PAGE3_EXT6:
    case EM_DROPDOWN_PAGE3_EXT7:
    case EM_DROPDOWN_PAGE3_EXT8:
        /* TODO: 根据产品需求，为各扩展项绑定具体功能逻辑 */
        break;

    default:
        break;
    }
}

//========================================================
// Function: _animShowFinished
// Description: 下拉动画结束后的回调
// Input：      pAnim  动画对象
// Output:      无
// Return:      无
//========================================================
static void _animShowFinished(lv_anim_t *pAnim)
{
    (void)pAnim;
    s_eState = EM_DROPDOWN_STATE_VISIBLE;
}

//========================================================
// Function: _animHideFinished
// Description: 收起动画结束后的回调
// Input：      pAnim  动画对象
// Output:      无
// Return:      无
//========================================================
static void _animHideFinished(lv_anim_t *pAnim)
{
    (void)pAnim;

    s_eState = EM_DROPDOWN_STATE_HIDDEN;

    if (lv_obj_is_valid(pDropdownPanel))
    {
        lv_obj_add_flag(pDropdownPanel, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_obj_is_valid(pDropdownOverlay))
    {
        lv_obj_add_flag(pDropdownOverlay, LV_OBJ_FLAG_HIDDEN);
    }
}

//========================================================
// Function: _getDesktopScreen
// Description: 获取桌面屏幕对象(安全获取)
// Input：      无
// Output:      无
// Return:      桌面屏幕对象指针
//========================================================
static lv_obj_t *_getDesktopScreen(void)
{
    return lv_layer_top();
}
