// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.0
// LVGL version: 8.3.6
// Project name: PubRemote

#ifndef _PUBREMOTE_UI_H
#define _PUBREMOTE_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"

// SCREEN: ui_SplashScreen
void ui_SplashScreen_screen_init(void);
void ui_event_SplashScreen(lv_event_t * e);
extern lv_obj_t * ui_SplashScreen;
extern lv_obj_t * ui_SplashScreenLogo;
// SCREEN: ui_StatsScreen
void ui_StatsScreen_screen_init(void);
extern lv_obj_t * ui_StatsScreen;
extern lv_obj_t * ui_DutyCycle;
extern lv_obj_t * ui_LeftSensor;
extern lv_obj_t * ui_RightSensor;
extern lv_obj_t * ui_StatsScreenContent;
extern lv_obj_t * ui_StatsScreenHeader;
extern lv_obj_t * ui_Logo;
extern lv_obj_t * ui_Time;
extern lv_obj_t * ui_StatsScreenBody;
extern lv_obj_t * ui_PrimaryStat;
extern lv_obj_t * ui_PrimaryStatUnit;
extern lv_obj_t * ui_SecondaryStats;
extern lv_obj_t * ui_ConnectionState;
extern lv_obj_t * ui_DistanceStats;
extern lv_obj_t * ui_TempStats;
extern lv_obj_t * ui_StatsScreenFooter;
void ui_event_BatteryDisplay(lv_event_t * e);
extern lv_obj_t * ui_BatteryDisplay;
// SCREEN: ui_PairingScreen
void ui_PairingScreen_screen_init(void);
extern lv_obj_t * ui_PairingScreen;
extern lv_obj_t * ui_PairingScreenContent;
extern lv_obj_t * ui_PairingScreenHeader;
extern lv_obj_t * ui_PairingScreenBody;
extern lv_obj_t * ui_Label2;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_PairingScreenFooter;
void ui_event_PairingScreenMainActionButton(lv_event_t * e);
extern lv_obj_t * ui_PairingScreenMainActionButton;
extern lv_obj_t * ui_PairingScreenMainActionButtonLabel;
// SCREEN: ui_CalibrationScreen
void ui_CalibrationScreen_screen_init(void);
extern lv_obj_t * ui_CalibrationScreen;
extern lv_obj_t * ui_CalibrationScreenContent;
extern lv_obj_t * ui_CalibrationScreenheader;
extern lv_obj_t * ui_CalibrationScreenBody;
extern lv_obj_t * ui_Label4;
extern lv_obj_t * ui_CalibrationScreenFooter;
void ui_event_CalibrationScreenMainActionButton(lv_event_t * e);
extern lv_obj_t * ui_CalibrationScreenMainActionButton;
extern lv_obj_t * ui_CalibrationScreenMainActionButtonLabel;
// SCREEN: ui_SettingsScreen
void ui_SettingsScreen_screen_init(void);
extern lv_obj_t * ui_SettingsScreen;
extern lv_obj_t * ui_SettingsScreenContent;
extern lv_obj_t * ui_SettingsScreenHeader;
extern lv_obj_t * ui_SettingsScreenBody;
extern lv_obj_t * ui_BrightnessLabel;
extern lv_obj_t * ui_BrightnessSlider;
extern lv_obj_t * ui_SettingsScreenFooter;
void ui_event_SettingsScreenMainActionButton(lv_event_t * e);
extern lv_obj_t * ui_SettingsScreenMainActionButton;
extern lv_obj_t * ui_SettingsScreenMainActionButtonLabel;
extern lv_obj_t * ui____initial_actions0;


LV_IMG_DECLARE(ui_img_icon_png);    // assets/icon.png



LV_FONT_DECLARE(ui_font_Open_Sans_14);
LV_FONT_DECLARE(ui_font_Open_Sans_Bold_14);
LV_FONT_DECLARE(ui_font_Open_Sans_Bold_48);



void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
