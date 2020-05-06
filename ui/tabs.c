#include <stdint.h>
#include <stdbool.h>
#include <xdc/runtime/System.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "constants.h"
#include "tabs.h"
#include "ui/user_interface.h"
#include "tabs/stats.h"
#include "tabs/home.h"
#include "tabs/settings.h"


tCanvasWidget panels[];
volatile PANEL selected_panel = STATS;


Canvas(statsPanel, panels + 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_stats);
Canvas(homePanel, panels + 1, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_home);
Canvas(settingsPanel, panels + 2, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_settings);

tCanvasWidget panels[] =
{
    CanvasStruct(0, 0, &statsPanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &homePanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &settingsPanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
};


void select_panel(PANEL panel) {
    if (selected_panel == panel) {
        // we're already looking at this panel
        return;
    }
    WidgetRemove((tWidget *)(panels + selected_panel));
    selected_panel = panel;
    WidgetPaint((tWidget *)(panels + selected_panel));
    WidgetAdd(WIDGET_ROOT, (tWidget *)(panels + selected_panel));
    redraw_tab_buttons();
}

void select_stats_panel(tWidget *psWidget) {
    select_panel(STATS);
}

void select_home_panel(tWidget *psWidget) {
    select_panel(HOME);
}

void select_settings_panel(tWidget *psWidget) {
    System_printf("Here!~");
    System_flush();
}

/* ========= TAB BUTTONS =========== */
RectangularButton(btnStats, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE, SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Stats", 0, 0, 0, 0, select_stats_panel);

RectangularButton(btnHome, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE + TAB_WIDTH, SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Home", 0, 0, 0, 0, select_home_panel);

RectangularButton(btnSettings, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE + (TAB_WIDTH * 2), SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Settings", 0, 0, 0, 0, select_settings_panel);

void redraw_tab_buttons() {
    // background color onto the selected one
    PushButtonFillColorSet(&btnStats, selected_panel == STATS ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnStats, selected_panel == STATS ? ClrGray : ClrAqua);

    PushButtonFillColorSet(&btnHome, selected_panel == HOME ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnHome, selected_panel == HOME ? ClrGray : ClrAqua);

    PushButtonFillColorSet(&btnSettings, selected_panel == SETTINGS ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnSettings, selected_panel == SETTINGS ? ClrGray : ClrAqua);

    WidgetPaint((tWidget *)&btnStats);
    WidgetPaint((tWidget *)&btnHome);
    WidgetPaint((tWidget *)&btnSettings);
}


void setup_tabs() {
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnStats);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnHome);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnSettings);
    redraw_tab_buttons();

    // Add the first panel - home panel
    WidgetAdd(WIDGET_ROOT, (tWidget *)(panels + selected_panel));
}
