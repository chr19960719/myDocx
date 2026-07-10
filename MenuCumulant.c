#include "GuiInclude.h"
#include "MenuCumulant.h"
#include "WidgetCommonNS.h"
#include "WidgetGridPagedNS.h"
#include "WidgetStdWinNS.h"
#include "PageIndicator.h"
#include "MenuPromp.h"
#include "Font.h"

#define CUMULANT_WARNING_RED 0xEE3232
#ifndef TABLESIZE
#define TABLESIZE(table) (sizeof(table) / sizeof((table)[0]))
#endif

#define CUMULANT_MAIN_STRIP_HEIGHT   240
#define CUMULANT_CARD_TOP_OFFSET     8
#define CUMULANT_CARD_HEIGHT         204
#define CUMULANT_INDICATOR_HEIGHT    12
#define CUMULANT_INDICATOR_OFFSET_Y  -4
#define CUMULANT_PAGED_WRAP_HEIGHT   CUMULANT_MAIN_STRIP_HEIGHT
#define CUMULANT_PAGE_PAD_BOTTOM     (CUMULANT_MAIN_STRIP_HEIGHT - CUMULANT_CARD_TOP_OFFSET - CUMULANT_CARD_HEIGHT - CUMULANT_INDICATOR_HEIGHT)
#define CUMULANT_GRID_SCROLL_HEIGHT  (CUMULANT_PAGED_WRAP_HEIGHT - CUMULANT_INDICATOR_HEIGHT)
#define CUMULANT_ROW_GAP             16
#define CUMULANT_PAGED_CARD_COUNT    4
#define CUMULANT_PAGED_WRAP_WIDTH    ((DEF_MENU_BTN_TYPE1_WIDTH * CUMULANT_PAGED_CARD_COUNT) + (CUMULANT_ROW_GAP * (CUMULANT_PAGED_CARD_COUNT - 1)))
#define CUMULANT_MAIN_ROW_WIDTH      (CUMULANT_PAGED_WRAP_WIDTH + CUMULANT_ROW_GAP + DEF_MENU_BTN_TYPE1_WIDTH)

enum
{
    EM_ID_CUMULANT_TOTAL = 0,
    EM_ID_CUMULANT_24HOUR,
    EM_ID_CUMULANT_RECENT,
    EM_ID_CUMULANT_CUSTOM,
    EM_ID_CUMULANT_TIMING,
    EM_ID_CUMULANT_SPEED_LOG,
    EM_ID_CUMULANT_MAX
};

typedef struct
{
    UINT8 u8Hour;
    UINT8 u8Minute;
} ST_DEMO_TIME;

typedef struct
{
    FLOAT32 fRate;
    char szTime[16];
} ST_DEMO_SPEED_LOG;

typedef struct
{
    FLOAT32 fTotal;
    FLOAT32 f24Hour;
    FLOAT32 fRecent;
    UINT8 u8RecentHours;
    FLOAT32 fCustom;
    ST_DEMO_TIME stCustomStart;
    ST_DEMO_TIME stCustomEnd;
    UINT8 u8TimingInterval;
    FLOAT32 afTimingBucket[6];
    ST_DEMO_SPEED_LOG astHistory[10];
    UINT8 u8HistoryCount;
} ST_CUMULANT_DEMO_DATA;

static lv_obj_t *s_pGridPaged = NULL;
static lv_obj_t *s_apMainCards[EM_ID_CUMULANT_MAX] = {0};
static lv_obj_t *s_pRecentValueLabel = NULL;
static lv_obj_t *s_pRecentMinusBtn = NULL;
static lv_obj_t *s_pRecentPlusBtn = NULL;
static lv_obj_t *s_pCustomStartCard = NULL;
static lv_obj_t *s_pCustomEndCard = NULL;
static lv_obj_t *s_pTimingIntervalCard = NULL;
static lv_obj_t *s_pClearBtn = NULL;
static lv_obj_t *s_pMainIndicator = NULL;

static ST_CUMULANT_DEMO_DATA s_stDemoData =
{
    182.35f,
    56.80f,
    12.60f,
    6,
    24.30f,
    {8, 0},
    {20, 0},
    4,
    {4.50f, 3.20f, 5.80f, 2.70f, 4.00f, 4.10f},
    {
        {120.0f, "08:10"},
        {135.0f, "08:42"},
        {110.0f, "09:05"},
        {98.0f,  "09:40"},
        {125.0f, "10:12"},
        {140.0f, "10:45"},
        {132.0f, "11:18"},
        {118.0f, "11:56"},
        {105.0f, "12:22"},
        {96.0f,  "12:48"}
    },
    10
};

static const UINT32 s_au32MainTitleIds[EM_ID_CUMULANT_MAX] =
{
    IDS_SUM_TOTAL,
    IDS_HOUR_TOTAL,
    IDS_RECENT_TOTAL,
    IDS_CUSTOM_CUMULANT,
    IDS_TIMING_INTERVAL_VOL,
    IDS_HISTORY_RECORD
};

static const UINT8 s_au8TimingOptions[] = {1, 2, 4, 8, 12, 24};

static void _cbMainWinFunc(lv_event_t *pEvent);
static void _cbMainPagerScrollEnd(lv_event_t *pEvent);
static void _cbClearBtn(lv_event_t *pEvent);
static void _cbRecentWinFunc(lv_event_t *pEvent);
static void _cbCustomWinFunc(lv_event_t *pEvent);
static void _cbTimingWinFunc(lv_event_t *pEvent);
static void _cbClearTotal(lv_event_t *pEvent, UINT8 u8MsgId);
static void _styleParamCard(lv_obj_t *pBtn);
static void _UpdateParamCardData(lv_obj_t *pBtn, const char *pText);
static void _UpdateParamCardUnit(lv_obj_t *pBtn, const char *pText);
static void _StyleClearCard(lv_obj_t *pBtn);
static lv_obj_t *_GetParamCardDataLabel(lv_obj_t *pBtn);
static lv_obj_t *_GetParamCardUnitLabel(lv_obj_t *pBtn);
static void _LayoutMainPager(lv_obj_t *pGridPaged);
static void _RefreshMainCards(void);
static void _RefreshRecentPage(void);
static void _RefreshCustomPage(void);
static void _RefreshTimingPage(void);
static void _CreateRecentCumulantWin(void);
static void _CreateCustomCumulantWin(void);
static void _CreateTimingCumulantWin(void);
static void _CreateSpeedLogWin(void);

static void _FormatVolumeValue(char *pszBuf, UINT32 u32Size, FLOAT32 fValue)
{
    if ((NULL == pszBuf) || (0u == u32Size))
    {
        return;
    }

    snprintf(pszBuf, u32Size, "%.2f", (double)fValue);
}

static void _FormatTime(char *pszBuf, UINT32 u32Size, const ST_DEMO_TIME *pstTime)
{
    if ((NULL == pszBuf) || (0u == u32Size) || (NULL == pstTime))
    {
        return;
    }

    snprintf(pszBuf, u32Size, "%02u:%02u", pstTime->u8Hour, pstTime->u8Minute);
}

static void _FormatRecentTitle(char *pszBuf, UINT32 u32Size)
{
    if ((NULL == pszBuf) || (0u == u32Size))
    {
        return;
    }

    snprintf(pszBuf, u32Size, "%s/%u%s",
        GetStr(IDS_RECENT_TOTAL, GetLanguage()),
        s_stDemoData.u8RecentHours,
        GetStr(IDS_HOUR, GetLanguage()));
}

static void _FormatCustomTitle(char *pszBuf, UINT32 u32Size)
{
    char szStart[16] = {0};
    char szEnd[16] = {0};

    if ((NULL == pszBuf) || (0u == u32Size))
    {
        return;
    }

    _FormatTime(szStart, sizeof(szStart), &s_stDemoData.stCustomStart);
    _FormatTime(szEnd, sizeof(szEnd), &s_stDemoData.stCustomEnd);
    snprintf(pszBuf, u32Size, "%s%s%s",
        szStart,
        GetStr(IDS_TO, GetLanguage()),
        szEnd);
}

static void _FormatTimingData(char *pszBuf, UINT32 u32Size)
{
    UINT32 u32BucketCount = 0;

    if ((NULL == pszBuf) || (0u == u32Size))
    {
        return;
    }

    if (0u != s_stDemoData.u8TimingInterval)
    {
        u32BucketCount = 24u / s_stDemoData.u8TimingInterval;
    }

    snprintf(pszBuf, u32Size, "%u%s x %u",
        s_stDemoData.u8TimingInterval,
        GetStr(IDS_HOUR, GetLanguage()),
        u32BucketCount);
}

static void _ResetDemoData(void)
{
    UINT8 u8Idx = 0;

    s_stDemoData.fTotal = 0.0f;
    s_stDemoData.f24Hour = 0.0f;
    s_stDemoData.fRecent = 0.0f;
    s_stDemoData.fCustom = 0.0f;

    for (u8Idx = 0; u8Idx < TABLESIZE(s_stDemoData.afTimingBucket); u8Idx++)
    {
        s_stDemoData.afTimingBucket[u8Idx] = 0.0f;
    }
}

static lv_obj_t *_CreateSectionLabel(lv_obj_t *pParent, const char *pszText)
{
    lv_obj_t *pLabel = lv_label_create(pParent);

    lv_label_set_text(pLabel, pszText);
    lv_obj_set_style_text_color(pLabel, lv_color_hex(PUMP_TEXT_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(pLabel, GetFont(EM_FONT_TEXT_24), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(pLabel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(pLabel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return pLabel;
}

static lv_obj_t *_CreateActionButton(lv_obj_t *pParent, const char *pszText, lv_event_cb_t cbEvent)
{
    lv_obj_t *pBtn = lv_btn_create(pParent);
    lv_obj_t *pLabel = lv_label_create(pBtn);

    lv_obj_set_size(pBtn, 160, 56);
    lv_obj_set_style_radius(pBtn, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(pBtn, lv_color_hex(PUMP_BAR_BG_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(pBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(pBtn, cbEvent, LV_EVENT_CLICKED, NULL);

    lv_label_set_text(pLabel, pszText);
    lv_obj_set_style_text_font(pLabel, GetFont(PUMP_BTN_TEXT_FONT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(pLabel, lv_color_hex(PUMP_TEXT_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(pLabel);

    return pBtn;
}

static void _styleParamCard(lv_obj_t *pBtn)
{
    lv_obj_t *pUnitLabel = NULL;
    lv_obj_t *pDataLabel = NULL;
    lv_obj_t *pArrow = NULL;
    UINT32 u32ChildCnt = 0;

    if ((NULL == pBtn) || (lv_obj_get_child_cnt(pBtn) < 2))
    {
        return;
    }

    u32ChildCnt = lv_obj_get_child_cnt(pBtn);
    pDataLabel = lv_obj_get_child(pBtn, u32ChildCnt - 1u);

    if (NULL != pDataLabel)
    {
        lv_obj_align(pDataLabel, LV_ALIGN_CENTER, -16, 0);
        lv_obj_set_style_text_align(pDataLabel, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    }

    if (u32ChildCnt >= 3u)
    {
        pUnitLabel = lv_obj_get_child(pBtn, 1u);
        if ((NULL != pUnitLabel) && (NULL != pDataLabel))
        {
            lv_obj_set_style_text_font(pUnitLabel, GetFont(EM_FONT_TEXT_24), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(pUnitLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(pUnitLabel, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align_to(pUnitLabel, pDataLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -4);
        }
    }

    pArrow = lv_label_create(pBtn);
    lv_label_set_text(pArrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(pArrow, GetFont(EM_FONT_TEXT_24), LV_PART_MAIN);
    lv_obj_set_style_text_color(pArrow, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_opa(pArrow, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(pArrow, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
}

static lv_obj_t *_GetParamCardDataLabel(lv_obj_t *pBtn)
{
    UINT32 u32ChildCnt = 0;

    if (NULL == pBtn)
    {
        return NULL;
    }

    u32ChildCnt = lv_obj_get_child_cnt(pBtn);
    if (u32ChildCnt < 2u)
    {
        return NULL;
    }

    if (u32ChildCnt >= 4u)
    {
        return lv_obj_get_child(pBtn, u32ChildCnt - 2u);
    }

    return lv_obj_get_child(pBtn, u32ChildCnt - 1u);
}

static lv_obj_t *_GetParamCardUnitLabel(lv_obj_t *pBtn)
{
    if ((NULL == pBtn) || (lv_obj_get_child_cnt(pBtn) < 3u))
    {
        return NULL;
    }

    return lv_obj_get_child(pBtn, 1u);
}

static void _UpdateParamCardData(lv_obj_t *pBtn, const char *pText)
{
    lv_obj_t *pDataLabel = NULL;
    lv_obj_t *pUnitLabel = NULL;

    if (NULL == pBtn)
    {
        return;
    }

    pDataLabel = _GetParamCardDataLabel(pBtn);

    if (NULL != pDataLabel)
    {
        lv_label_set_text(pDataLabel, pText);
        lv_obj_align(pDataLabel, LV_ALIGN_CENTER, -16, 0);
    }

    pUnitLabel = _GetParamCardUnitLabel(pBtn);
    if ((NULL != pUnitLabel) && (NULL != pDataLabel))
    {
        lv_obj_align_to(pUnitLabel, pDataLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -4);
    }
}

static void _UpdateParamCardUnit(lv_obj_t *pBtn, const char *pText)
{
    lv_obj_t *pUnitLabel = NULL;
    lv_obj_t *pDataLabel = NULL;

    if (NULL == pBtn)
    {
        return;
    }

    pUnitLabel = _GetParamCardUnitLabel(pBtn);
    if (NULL != pUnitLabel)
    {
        lv_label_set_text(pUnitLabel, pText);
        lv_obj_set_style_text_font(pUnitLabel, GetFont(EM_FONT_TEXT_24), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(pUnitLabel, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(pUnitLabel, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);

        pDataLabel = _GetParamCardDataLabel(pBtn);
        if (NULL != pDataLabel)
        {
            lv_obj_align_to(pUnitLabel, pDataLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -4);
        }
    }
}

static void _StyleClearCard(lv_obj_t *pBtn)
{
    UINT32 u32ChildCnt = 0;
    lv_obj_t *pTitleLabel = NULL;
    lv_obj_t *pDataLabel = NULL;

    if ((NULL == pBtn) || (lv_obj_get_child_cnt(pBtn) < 2u))
    {
        return;
    }

    u32ChildCnt = lv_obj_get_child_cnt(pBtn);
    pTitleLabel = lv_obj_get_child(pBtn, 0);
    pDataLabel = lv_obj_get_child(pBtn, u32ChildCnt - 1u);

    lv_obj_set_size(pBtn, DEF_MENU_BTN_TYPE1_WIDTH, DEF_MENU_BTN_TYPE1_HEIGHT);

    if (NULL != pTitleLabel)
    {
        lv_label_set_text(pTitleLabel, "");
        lv_obj_set_style_text_opa(pTitleLabel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if (NULL != pDataLabel)
    {
        lv_obj_set_width(pDataLabel, DEF_MENU_BTN_TYPE1_WIDTH - 24);
        lv_obj_align(pDataLabel, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(pDataLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(pDataLabel, GetFont(EM_FONT_TEXT_48), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(pDataLabel, lv_color_hex(PUMP_SKY_BLUE_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void _RefreshMainCards(void)
{
    char szBuf[64] = {0};

    if (NULL == s_apMainCards[EM_ID_CUMULANT_TOTAL])
    {
        return;
    }

    _FormatVolumeValue(szBuf, sizeof(szBuf), s_stDemoData.fTotal);
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_TOTAL], szBuf);
    _UpdateParamCardUnit(s_apMainCards[EM_ID_CUMULANT_TOTAL], GetStr(IDS_ML, GetLanguage()));

    snprintf(szBuf, sizeof(szBuf), "24%s", GetStr(IDS_HOUR_TOTAL, GetLanguage()));
    CmLv_NoImgBtnUpdateText(s_apMainCards[EM_ID_CUMULANT_24HOUR], szBuf);
    _FormatVolumeValue(szBuf, sizeof(szBuf), s_stDemoData.f24Hour);
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_24HOUR], szBuf);
    _UpdateParamCardUnit(s_apMainCards[EM_ID_CUMULANT_24HOUR], GetStr(IDS_ML, GetLanguage()));

    _FormatRecentTitle(szBuf, sizeof(szBuf));
    CmLv_NoImgBtnUpdateText(s_apMainCards[EM_ID_CUMULANT_RECENT], szBuf);
    _FormatVolumeValue(szBuf, sizeof(szBuf), s_stDemoData.fRecent);
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_RECENT], szBuf);
    _UpdateParamCardUnit(s_apMainCards[EM_ID_CUMULANT_RECENT], GetStr(IDS_ML, GetLanguage()));

    _FormatCustomTitle(szBuf, sizeof(szBuf));
    CmLv_NoImgBtnUpdateText(s_apMainCards[EM_ID_CUMULANT_CUSTOM], szBuf);
    _FormatVolumeValue(szBuf, sizeof(szBuf), s_stDemoData.fCustom);
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_CUSTOM], szBuf);
    _UpdateParamCardUnit(s_apMainCards[EM_ID_CUMULANT_CUSTOM], GetStr(IDS_ML, GetLanguage()));

    _FormatTimingData(szBuf, sizeof(szBuf));
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_TIMING], szBuf);

    snprintf(szBuf, sizeof(szBuf), "%u", s_stDemoData.u8HistoryCount);
    _UpdateParamCardData(s_apMainCards[EM_ID_CUMULANT_SPEED_LOG], szBuf);
}

static void _RefreshRecentPage(void)
{
    char szBuf[32] = {0};

    if (NULL == s_pRecentValueLabel)
    {
        return;
    }

    snprintf(szBuf, sizeof(szBuf), "%u%s", s_stDemoData.u8RecentHours, GetStr(IDS_HOUR, GetLanguage()));
    lv_label_set_text(s_pRecentValueLabel, szBuf);
}

static void _RefreshCustomPage(void)
{
    char szBuf[32] = {0};

    if (NULL != s_pCustomStartCard)
    {
        _FormatTime(szBuf, sizeof(szBuf), &s_stDemoData.stCustomStart);
        _UpdateParamCardData(s_pCustomStartCard, szBuf);
    }

    if (NULL != s_pCustomEndCard)
    {
        _FormatTime(szBuf, sizeof(szBuf), &s_stDemoData.stCustomEnd);
        _UpdateParamCardData(s_pCustomEndCard, szBuf);
    }
}

static void _RefreshTimingPage(void)
{
    char szBuf[32] = {0};

    if (NULL == s_pTimingIntervalCard)
    {
        return;
    }

    snprintf(szBuf, sizeof(szBuf), "%u%s", s_stDemoData.u8TimingInterval, GetStr(IDS_HOUR, GetLanguage()));
    _UpdateParamCardData(s_pTimingIntervalCard, szBuf);
}

static void _cbClearTotal(lv_event_t *pEvent, UINT8 u8MsgId)
{
    LV_UNUSED(pEvent);

    if (EM_ID_PROMP_OK != u8MsgId)
    {
        return;
    }

    _ResetDemoData();
    _RefreshMainCards();
    CreatePrompWin(GetStr(IDS_TOTAL_VOLUME_CLEARED, GetLanguage()), NULL, PrompWin_GetUiRect());
}

static void _CreateRecentCumulantWin(void)
{
    lv_obj_t *pWin = NULL;
    lv_obj_t *pContent = NULL;
    lv_obj_t *pRow = NULL;
    lv_obj_t *pBtn = NULL;

    pWin = CreateStdWinWidgetNS(GetStr(IDS_RECENT_TOTAL, GetLanguage()), _cbRecentWinFunc, TRUE, NULL, 0);
    if (NULL == pWin)
    {
        return;
    }

    pContent = lv_win_get_content(pWin);
    lv_obj_set_flex_flow(pContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(pContent, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    _CreateSectionLabel(pContent, GetStr(IDS_CUMULANT_TIME, GetLanguage()));

    pRow = lv_obj_create(pContent);
    lv_obj_set_width(pRow, lv_pct(100));
    lv_obj_set_height(pRow, 72);
    lv_obj_set_style_bg_opa(pRow, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(pRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(pRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(pRow, 18, LV_PART_MAIN | LV_STATE_DEFAULT);

    pBtn = _CreateActionButton(pRow, "-", _cbRecentWinFunc);
    s_pRecentMinusBtn = pBtn;

    s_pRecentValueLabel = lv_label_create(pRow);
    lv_obj_set_style_text_font(s_pRecentValueLabel, GetFont(EM_FONT_TEXT_48), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_pRecentValueLabel, lv_color_hex(PUMP_TEXT_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);

    pBtn = _CreateActionButton(pRow, "+", _cbRecentWinFunc);
    s_pRecentPlusBtn = pBtn;

    _RefreshRecentPage();
}

static void _CreateCustomCumulantWin(void)
{
    lv_obj_t *pWin = NULL;
    lv_obj_t *pContent = NULL;

    pWin = CreateStdWinWidgetNS(GetStr(IDS_CUSTOM_CUMULANT, GetLanguage()), _cbCustomWinFunc, TRUE, NULL, 0);
    if (NULL == pWin)
    {
        return;
    }

    pContent = lv_win_get_content(pWin);
    lv_obj_set_flex_flow(pContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(pContent, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    _CreateSectionLabel(pContent, GetStr(IDS_TIME, GetLanguage()));

    s_pCustomStartCard = CreateParamBtnWidgetNS(
        pContent,
        GetStr(IDS_START_TIME, GetLanguage()),
        NULL,
        "--",
        _cbCustomWinFunc);
    _styleParamCard(s_pCustomStartCard);

    s_pCustomEndCard = CreateParamBtnWidgetNS(
        pContent,
        GetStr(IDS_END_TIME, GetLanguage()),
        NULL,
        "--",
        _cbCustomWinFunc);
    _styleParamCard(s_pCustomEndCard);

    _RefreshCustomPage();
}

static void _CreateTimingBucketList(lv_obj_t *pParent)
{
    UINT8 u8Idx = 0;
    char szTitle[32] = {0};
    char szValue[32] = {0};

    for (u8Idx = 0; u8Idx < TABLESIZE(s_stDemoData.afTimingBucket); u8Idx++)
    {
        lv_obj_t *pCard = NULL;

        snprintf(szTitle, sizeof(szTitle), "%u%s-%u%s",
            u8Idx * s_stDemoData.u8TimingInterval,
            GetStr(IDS_HOUR, GetLanguage()),
            (u8Idx + 1u) * s_stDemoData.u8TimingInterval,
            GetStr(IDS_HOUR, GetLanguage()));
        _FormatVolumeValue(szValue, sizeof(szValue), s_stDemoData.afTimingBucket[u8Idx]);

        pCard = CreateParamBtnWidgetNS(pParent, szTitle, NULL, szValue, NULL);
        _styleParamCard(pCard);
        lv_obj_set_style_bg_color(pCard, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void _CreateTimingCumulantWin(void)
{
    lv_obj_t *pWin = NULL;
    lv_obj_t *pContent = NULL;

    pWin = CreateStdWinWidgetNS(GetStr(IDS_TIMING_INTERVAL_VOL, GetLanguage()), _cbTimingWinFunc, TRUE, NULL, 0);
    if (NULL == pWin)
    {
        return;
    }

    pContent = lv_win_get_content(pWin);
    lv_obj_set_flex_flow(pContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(pContent, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    s_pTimingIntervalCard = CreateParamBtnWidgetNS(
        pContent,
        GetStr(IDS_CUMULANT_TIME, GetLanguage()),
        NULL,
        "--",
        _cbTimingWinFunc);
    _styleParamCard(s_pTimingIntervalCard);

    _CreateSectionLabel(pContent, GetStr(IDS_TOTAL_VOL, GetLanguage()));
    _CreateTimingBucketList(pContent);
    _RefreshTimingPage();
}

static void _CreateSpeedLogWin(void)
{
    lv_obj_t *pWin = NULL;
    lv_obj_t *pContent = NULL;
    UINT8 u8Idx = 0;
    char szRate[32] = {0};

    pWin = CreateStdWinWidgetNS(GetStr(IDS_HISTORY_RECORD, GetLanguage()), NULL, TRUE, NULL, 0);
    if (NULL == pWin)
    {
        return;
    }

    pContent = lv_win_get_content(pWin);
    lv_obj_set_flex_flow(pContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(pContent, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    for (u8Idx = 0; u8Idx < s_stDemoData.u8HistoryCount; u8Idx++)
    {
        lv_obj_t *pCard = NULL;

        snprintf(szRate, sizeof(szRate), "%.0f", (double)s_stDemoData.astHistory[u8Idx].fRate);
        pCard = CreateParamBtnWidgetNS(
            pContent,
            s_stDemoData.astHistory[u8Idx].szTime,
            GetStr(IDS_ML_H, GetLanguage()),
            szRate,
            NULL);
        _styleParamCard(pCard);
        lv_obj_set_style_bg_color(pCard, lv_color_hex(PUMP_WIN_TX_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void _cbMainWinFunc(lv_event_t *pEvent)
{
    lv_event_code_t eCode = lv_event_get_code(pEvent);
    lv_obj_t *pTarget = lv_event_get_target(pEvent);

    if (LV_EVENT_CLICKED == eCode)
    {
        if (pTarget == s_apMainCards[EM_ID_CUMULANT_RECENT])
        {
            _CreateRecentCumulantWin();
        }
        else if (pTarget == s_apMainCards[EM_ID_CUMULANT_CUSTOM])
        {
            _CreateCustomCumulantWin();
        }
        else if (pTarget == s_apMainCards[EM_ID_CUMULANT_TIMING])
        {
            _CreateTimingCumulantWin();
        }
        else if (pTarget == s_apMainCards[EM_ID_CUMULANT_SPEED_LOG])
        {
            _CreateSpeedLogWin();
        }
    }
    else if (LV_EVENT_DELETE == eCode)
    {
        memset(s_apMainCards, 0, sizeof(s_apMainCards));
        s_pGridPaged = NULL;
        s_pMainIndicator = NULL;
    }
}

static void _cbMainPagerScrollEnd(lv_event_t *pEvent)
{
    LV_UNUSED(pEvent);

    if ((NULL == s_pGridPaged) || (NULL == s_pMainIndicator))
    {
        return;
    }

    PageIndicator_SetActive(s_pMainIndicator, WidgetGridPagedNS_GetCurrentPage(s_pGridPaged));
}

static void _cbClearBtn(lv_event_t *pEvent)
{
    lv_event_code_t eCode = lv_event_get_code(pEvent);

    if (LV_EVENT_CLICKED == eCode)
    {
        CreatePrompWin(GetStr(IDS_CLEAR_ACC, GetLanguage()), _cbClearTotal, PrompWin_GetUiRect());
    }
    else if (LV_EVENT_DELETE == eCode)
    {
        s_pClearBtn = NULL;
    }
}

static void _cbRecentWinFunc(lv_event_t *pEvent)
{
    lv_event_code_t eCode = lv_event_get_code(pEvent);
    lv_obj_t *pTarget = lv_event_get_target(pEvent);

    if (LV_EVENT_CLICKED != eCode)
    {
        if (LV_EVENT_DELETE == eCode)
        {
            s_pRecentValueLabel = NULL;
            s_pRecentMinusBtn = NULL;
            s_pRecentPlusBtn = NULL;
        }
        return;
    }

    if (pTarget == s_pRecentMinusBtn)
    {
        if (s_stDemoData.u8RecentHours > 1u)
        {
            s_stDemoData.u8RecentHours--;
            s_stDemoData.fRecent = (FLOAT32)s_stDemoData.u8RecentHours * 2.1f;
        }
    }
    else if (pTarget == s_pRecentPlusBtn)
    {
        if (s_stDemoData.u8RecentHours < 24u)
        {
            s_stDemoData.u8RecentHours++;
            s_stDemoData.fRecent = (FLOAT32)s_stDemoData.u8RecentHours * 2.1f;
        }
    }

    _RefreshRecentPage();
    _RefreshMainCards();
}

static void _cbCustomWinFunc(lv_event_t *pEvent)
{
    lv_event_code_t eCode = lv_event_get_code(pEvent);
    lv_obj_t *pTarget = lv_event_get_target(pEvent);

    if (LV_EVENT_CLICKED != eCode)
    {
        if (LV_EVENT_DELETE == eCode)
        {
            s_pCustomStartCard = NULL;
            s_pCustomEndCard = NULL;
        }
        return;
    }

    if (pTarget == s_pCustomStartCard)
    {
        s_stDemoData.stCustomStart.u8Hour = (s_stDemoData.stCustomStart.u8Hour + 1u) % 24u;
    }
    else if (pTarget == s_pCustomEndCard)
    {
        s_stDemoData.stCustomEnd.u8Hour = (s_stDemoData.stCustomEnd.u8Hour + 1u) % 24u;
    }

    s_stDemoData.fCustom += 1.25f;
    if (s_stDemoData.fCustom > s_stDemoData.fTotal)
    {
        s_stDemoData.fCustom = s_stDemoData.fTotal;
    }

    _RefreshCustomPage();
    _RefreshMainCards();
}

static void _cbTimingWinFunc(lv_event_t *pEvent)
{
    lv_event_code_t eCode = lv_event_get_code(pEvent);
    lv_obj_t *pTarget = lv_event_get_target(pEvent);
    UINT8 u8Idx = 0;

    if (LV_EVENT_CLICKED != eCode)
    {
        if (LV_EVENT_DELETE == eCode)
        {
            s_pTimingIntervalCard = NULL;
        }
        return;
    }

    if (pTarget != s_pTimingIntervalCard)
    {
        return;
    }

    for (u8Idx = 0; u8Idx < TABLESIZE(s_au8TimingOptions); u8Idx++)
    {
        if (s_au8TimingOptions[u8Idx] == s_stDemoData.u8TimingInterval)
        {
            s_stDemoData.u8TimingInterval = s_au8TimingOptions[(u8Idx + 1u) % TABLESIZE(s_au8TimingOptions)];
            break;
        }
    }

    _RefreshTimingPage();
    _RefreshMainCards();
}

void DskBuildCreateCumulantWin(void *pParam)
{
    lv_obj_t *pWin = NULL;
    lv_obj_t *pContent = NULL;
    lv_obj_t *pBodyRow = NULL;
    lv_obj_t *pPagedWrap = NULL;
    lv_obj_t *pClearBtn = NULL;
    ST_GRID_PAGED_INIT_PARAM stGridParam = {0};
    UINT8 u8Idx = 0;
    char szTitle[64] = {0};
    char szData[64] = {0};

    LV_UNUSED(pParam);

    pWin = CreateStdWinWidgetNS(GetStr(IDS_TOTAL_VOL, GetLanguage()), _cbMainWinFunc, TRUE, NULL, 0);
    if (NULL == pWin)
    {
        return;
    }

    pContent = CmLv_GetContentArea(pWin);
    if (NULL == pContent)
    {
        return;
    }

    lv_obj_clear_flag(pContent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(pContent, LV_DIR_NONE);
    lv_obj_set_flex_flow(pContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pContent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_bottom(pContent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(pContent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    pBodyRow = lv_obj_create(pContent);
    if (NULL == pBodyRow)
    {
        return;
    }

    lv_obj_set_width(pBodyRow, CUMULANT_MAIN_ROW_WIDTH);
    lv_obj_set_height(pBodyRow, CUMULANT_MAIN_STRIP_HEIGHT);
    lv_obj_clear_flag(pBodyRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(pBodyRow, LV_DIR_NONE);
    lv_obj_set_flex_flow(pBodyRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pBodyRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(pBodyRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(pBodyRow, CUMULANT_ROW_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(pBodyRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(pBodyRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(pBodyRow, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);

    pPagedWrap = lv_obj_create(pBodyRow);
    if (NULL == pPagedWrap)
    {
        return;
    }

    lv_obj_set_width(pPagedWrap, CUMULANT_PAGED_WRAP_WIDTH);
    lv_obj_set_height(pPagedWrap, CUMULANT_PAGED_WRAP_HEIGHT);
    lv_obj_clear_flag(pPagedWrap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(pPagedWrap, LV_DIR_NONE);
    lv_obj_set_layout(pPagedWrap, 0);
    lv_obj_set_style_pad_all(pPagedWrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(pPagedWrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(pPagedWrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(pPagedWrap, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);

    stGridParam.u8ColCount = CUMULANT_PAGED_CARD_COUNT;
    stGridParam.u8RowCount = 1;
    stGridParam.wHeight = CUMULANT_PAGED_WRAP_HEIGHT;
    stGridParam.wColWidth = DEF_MENU_BTN_TYPE1_WIDTH;
    stGridParam.wColGap = CUMULANT_ROW_GAP;
    stGridParam.wRowGap = 0;
    stGridParam.wPagePadTop = CUMULANT_CARD_TOP_OFFSET;
    stGridParam.wPagePadBottom = CUMULANT_PAGE_PAD_BOTTOM;
    stGridParam.bShowIndicator = FALSE;
    stGridParam.bgColor = lv_color_hex(PUMP_WIN_BG_COLOR);

    s_pGridPaged = WidgetGridPagedNS_Create(pPagedWrap, &stGridParam);
    if (NULL == s_pGridPaged)
    {
        return;
    }

    lv_obj_clear_flag(s_pGridPaged, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(s_pGridPaged, LV_DIR_NONE);
    lv_obj_set_size(s_pGridPaged, CUMULANT_PAGED_WRAP_WIDTH, CUMULANT_PAGED_WRAP_HEIGHT);
    lv_obj_align(s_pGridPaged, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(s_pGridPaged, LV_OPA_TRANSP, LV_PART_MAIN);

    {
        lv_obj_t *pScrollArea = lv_obj_get_child(s_pGridPaged, 0);
        if (NULL != pScrollArea)
        {
            UINT32 u32PageCount = lv_obj_get_child_cnt(pScrollArea);
            UINT32 u32PageIdx = 0;

            lv_obj_set_height(pScrollArea, CUMULANT_GRID_SCROLL_HEIGHT);
            lv_obj_set_flex_grow(pScrollArea, 0);
            lv_obj_set_style_bg_opa(pScrollArea, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_add_event_cb(pScrollArea, _cbMainPagerScrollEnd, LV_EVENT_SCROLL_END, NULL);

            for (u32PageIdx = 0; u32PageIdx < u32PageCount; u32PageIdx++)
            {
                lv_obj_t *pPage = lv_obj_get_child(pScrollArea, u32PageIdx);
                if (NULL != pPage)
                {
                    lv_obj_set_style_bg_opa(pPage, LV_OPA_TRANSP, LV_PART_MAIN);
                }
            }
        }
    }

    for (u8Idx = 0; u8Idx < EM_ID_CUMULANT_MAX; u8Idx++)
    {
        lv_obj_t *pBtn = NULL;

        if (EM_ID_CUMULANT_24HOUR == u8Idx)
        {
            snprintf(szTitle, sizeof(szTitle), "24%s", GetStr(s_au32MainTitleIds[u8Idx], GetLanguage()));
        }
        else
        {
            snprintf(szTitle, sizeof(szTitle), "%s", GetStr(s_au32MainTitleIds[u8Idx], GetLanguage()));
        }

        snprintf(szData, sizeof(szData), "--");
        pBtn = CreateParamBtnWidgetNS(
            s_pGridPaged,
            szTitle,
            (u8Idx <= EM_ID_CUMULANT_CUSTOM) ? GetStr(IDS_ML, GetLanguage()) : NULL,
            szData,
            _cbMainWinFunc);
        _styleParamCard(pBtn);
        WidgetGridPagedNS_AddItem(s_pGridPaged, pBtn);
        s_apMainCards[u8Idx] = pBtn;
    }

    {
        lv_obj_t *pScrollArea = lv_obj_get_child(s_pGridPaged, 0);
        if (NULL != pScrollArea)
        {
            UINT32 u32PageCount = lv_obj_get_child_cnt(pScrollArea);
            UINT32 u32PageIdx = 0;

            for (u32PageIdx = 0; u32PageIdx < u32PageCount; u32PageIdx++)
            {
                lv_obj_t *pPage = lv_obj_get_child(pScrollArea, u32PageIdx);
                if (NULL != pPage)
                {
                    lv_obj_set_style_bg_opa(pPage, LV_OPA_TRANSP, LV_PART_MAIN);
                }
            }
        }
    }

    s_pMainIndicator = PageIndicator_Create(pPagedWrap, WidgetGridPagedNS_GetPageCount(s_pGridPaged));
    if (NULL != s_pMainIndicator)
    {
        lv_obj_move_foreground(s_pMainIndicator);
        lv_obj_set_size(s_pMainIndicator, 52, CUMULANT_INDICATOR_HEIGHT);
        lv_obj_set_style_bg_opa(s_pMainIndicator, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_align(s_pMainIndicator, LV_ALIGN_BOTTOM_MID, 0, CUMULANT_INDICATOR_OFFSET_Y);
        PageIndicator_SetActive(s_pMainIndicator, 0);
    }

    pClearBtn = CreateParamBtnWidgetNS(
        pBodyRow,
        "",
        NULL,
        GetStr(IDS_CLEAR_TOTAL_VOL, GetLanguage()),
        _cbClearBtn);
    s_pClearBtn = pClearBtn;
    lv_obj_set_style_translate_y(pClearBtn, CUMULANT_CARD_TOP_OFFSET, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(pClearBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(pClearBtn, LV_DIR_NONE);
    lv_obj_add_event_cb(pClearBtn, _cbClearBtn, LV_EVENT_DELETE, NULL);
    _StyleClearCard(pClearBtn);

    _RefreshMainCards();
    lv_scr_load_anim(GetMenuScreen(), LV_SCR_LOAD_ANIM_NONE, 0, 0, FALSE);
}
