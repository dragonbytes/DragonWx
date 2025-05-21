#pragma once

#include "Globals.h"

#ifndef _DEBUG
#include "olcPGEX_SplashScreen.h"
#endif

class DragonWx : public olc::PixelGameEngine
{
public:
	DragonWx();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	bool OnUserDestroy() override;
	void OnTextEntryComplete(const std::string&) override;

	bool mouseWithinArea(olc::vf2d, olc::vf2d) const;
	bool mouseClickedInputBox(inputBoxStruct*) const;
	void ActivateInputBox(inputBoxStruct*);
	bool PasteTextFromClipboard();
	bool isWhiteSpaceOnly(const std::string&);
	olc::vi2d StringPixelSize(olc::Font&, std::u32string&);
	olc::vi2d GetCenteredStartPosition(olc::vi2d, olc::vi2d);
	std::u32string ConvertedString32(std::string);
	olc::vi2d TextCenteredOffsetY(std::string, olc::Font*);
	void RenderString(std::string, olc::Font*, olc::Pixel, textObject*, olc::vi2d);
	void RenderString32(std::u32string, olc::Font*, olc::Pixel, textObject*, olc::vi2d, bool);
	void RenderString32(std::u32string, olc::Font*, olc::Pixel, textObject*, olc::vi2d);
	olc::vi2d RenderStringSegment(std::string, olc::Font*, olc::Pixel, textObject*, olc::vi2d, int spaceWidth);
	void RenderStringCentered(std::string, olc::Font*, olc::Pixel, textObject*, olc::vi2d);
	void RenderStringRightJustified(std::string, olc::Font*, olc::Pixel, textObject*, olc::vi2d);
	void RenderHighOrLowValue(std::string, double, textObject*, olc::vf2d, bool);
	void DrawWindDirectionArrow(double, olc::Pixel);
	void DrawCircleArc(olc::vf2d, int, double, double, olc::Pixel);
	//void DrawWindDirPreviousArc(olc::vf2d startPos, int radius, olc::Pixel pixelColor, uint8_t mask);
	void DrawBoxTitle(titleBox* titleBoxPtr, olc::Renderable*);
	void DrawRainGaugeOutlines(olc::vi2d, olc::vi2d, int, olc::Pixel);
	void RenderRainGaugeText();
	void DrawUVindexGraph(olc::vf2d, std::u32string strValue);
	void RenderButton(buttonStruct*);
	void RenderButton(buttonStruct*, std::string);
	void RenderButtonWithEnabler(buttonStruct*);
	void RenderInputBox(inputBoxStruct*, olc::Font* fontToUse, olc::Pixel textColor);
	void ShowCenteredDialogBox(dialogBox*);
	void RenderCenteredWxCondition(olc::vf2d, olc::Decal*, textObject*);
	void RenderCenteredWxForecast(olc::vf2d, olc::Decal*, wxWebEntry*, textObject*);
	void UpdateAreaBordersSprite();
	olc::Decal* UpdateTrendData(std::deque<float>* sourceDeque, float dataValue, olc::Decal*, std::string) const;
	std::string GetWindDirectionName(double);
	bool NextWindDirAnimationPoint(float& currentValue, float targetValue, float& currentVelocity, float& timeSinceLastUpdate, float fElapsedTimePGE);
	float NormalizedAngle(float angle);
	float shortest_angle_diff(float from, float to);
	float update_weather_vane_spring(float current, float target, float& velocity, float stiffness, float damping, float dt);
	float update_weather_vane(float current, float target, float& velocity, float max_speed, float acceleration, float dt);
	void MidnightDailyReset();
	void ResetAllStatistics();
	bool LoadWebWxAssets(wxWebEntry*, olc::Decal*);
	//bool SaveConfigFile();

	#ifndef _DEBUG
	olc::SplashScreen splash;
	#endif

	enum inputBoxType
	{
		BOOL, INT, FLOAT, STRING
	};

	inputBoxStruct inputBoxOutdoorID, inputBoxIndoorID, inputBoxLatitude, inputBoxLongitude, inputBoxSdrExecPath, inputBoxSdrParams, inputBoxSdrGain;
	inputBoxStruct inputBoxOutdoorCalTemp, inputBoxIndoorCalTemp, inputBoxOutdoorCalHumidity, inputBoxIndoorCalHumidity;
	inputBoxStruct* setupActiveInputBoxPtr;

	buttonStruct buttonUnitsToggle, buttonForecastOnOff, buttonResetStats, buttonAboutApp, buttonStartStopRTL433, buttonOk, buttonCancel;

	dialogBox dialogBoxFailedExecCmd, dialogBoxNoValidConfig, dialogBoxInvalidValue, dialogBoxRestartRequired;
	dialogBox* dialogBoxForegroundPtr = nullptr;

	float restartPendingElapsed = 0.0f;
	int minuteTimeCounter = 0;		// This allows me to use single 15 second interval to also handle events once per minute
	float cursorBlinkElapsedTime = 0.0f;
	int setupActiveInputBoxID = -1;

	olc::vi2d positionMouseCursor;
	bool setupUseMetricUnits, setupWebWxEnabled;
	bool pendingRestartRTL433 = false, mouseWaitingButtonRelease, textCursorBlinkState, useFeelsLikeLabel, useLuxValue, useNumericWindDirection;
	olc::Key keyPrevious;

	std::string strClipboardContents;
	std::u32string strFeelsLikeLabel;
	temperatureUnitsStruct degreeUnits;
	windSpeedUnitsStruct windSpeedUnits;
	rainfallUnitsStruct rainfallUnits;

	titleBox titleBoxOutdoorPanel, titleBoxIndoorPanel, titleBoxSensorPanel, titleBoxRainPanel, titleBoxForecastPanel, titleBoxAboutApp;
	titleBox titleBoxSetupOutdoor, titleBoxSetupIndoor;

	olc::Pixel colorLabelText;
	olc::Pixel rainGaugeBorderColor;
	olc::Pixel areasBorderColor;

	olc::Sprite* previousDrawTarget;

	olc::Renderable renderableCloseIcon, renderableSettingsIcon, renderableInfoIcon, renderableDragonLogo;
	olc::Renderable renderableBackgroundImage, renderableAreaBorders, renderableSetupScreen, renderableInfoScreen;
	olc::Renderable renderableThermometerIconF, renderableThermometerIconC, renderableWaterDropIcon, renderableRainfallIcon;
	olc::Renderable renderableWindDir, renderableRainGaugeOutline, renderableRainGaugeWater, renderableSignalStrength[5];
	olc::Renderable renderableTrendArrowUp, renderableTrendArrowSteady, renderableTrendArrowDown;
	olc::Renderable renderableWebConditionsImage, renderableWebForecastImages[3];

	olc::vi2d centerPointWindDir;

	olc::Decal* decalTrendOutdoorTemp;
	olc::Decal* decalTrendOutdoorHumidity;

	olc::Renderable renderableLowLabel18, renderableHighLabel18, renderableLowLabel24, renderableHighLabel24;

	textObject textObjectDateText, textObjectTimeText, textObjectIndoorLabel, textObjectOutdoorLabel, textObjectAppNameLabel, textObjectSetupWinLabel;
	textObject textObjectInfoPageTitle;

	textObject textObjectIndoorLowValue, textObjectIndoorHighValue;

	textObject textObjectLabelTempUnits, textObjectDewPointUnits, textObjectLabelDewpoint;
	textObject textObjectTempValue, textObjectTempDecimalValue, textObjectOutdoorTempLowValue, textObjectOutdoorTempHighValue;
	textObject textObjectIndoorTempUnits, textObjectIndoorTempValue, textObjectIndoorTempLowValue, textObjectIndoorTempHighValue;
	textObject textObjectFeelsLikeLabel, textObjectFeelsLikeValue, textObjectFeelsLikeUnits;
	textObject textObjectUVindexLabel, textObjectUVindexValue, textObjectLightLevelLabel, textObjectLightLevelValue, textObjectLightLeveilUnits;

	textObject textObjectLabelHumidity, textObjectHumidityUnits, textObjectHumidityValue, textObjectHumidityLowValue, textObjectHumidityHighValue, textObjectDewpointValue;
	textObject textObjectIndoorHumidityUnits, textObjectIndoorHumidityValue;

	textObject windSpeedUnitsText, textObjectLabelWindSpeedAvg, windSpeedText, textObjectWindSpeedAvg, textObjectWindSpeedPeakLabel, textObjectWindSpeedPeakValue;
	textObject textObjectWindSpeedUnits, textObjectWindDirName;

	textObject textObjectRainfallLabel, textObjectRainfallTodayLabel, textObjectRainfallRateLabel, textObjectRainTodayUnitsLabel, textObjectRainfallRateUnits;
	textObject textObjectRainfallValue, textObjectRainfallRateValue, textObjectRainGaugeUnits[rainGaugeMarksTotal + 1];
	textObject textObjRainStartLabel, textObjRainStopLabel, textObjRainStartValue, textObjRainStopValue;

	textObject textObjectSensorInfoLabel, textObjectSignalLabel, textObjectSensorOutdoorLabel, textObjectSensorIndoorLabel;
	textObject textObjectSignalOutdoorLabel, textObjectSignalIndoorLabel;
	textObject textObjectBatteryOutdoorLabel, textObjectBatteryIndoorLabel, textObjectBatteryOutdoorValue, textObjectBatteryIndoorValue;
	textObject textObjectChannelOutdoorLabel, textObjectChannelOutdoorValue, textObjectChannelIndoorLabel, textObjectChannelIndoorValue;

	textObject textObjectForecastNowLabel, textObjectForecastToday, textObjectForecastTomorrow, textObjectConditionsDesc;
	textObject textObjectForecastDays[3], textObjForecastToday[3], textObjForecastTomorrow[3];

	textObject textObjectAuthorText[3], textObjectIntroText[5], textObjectThanksTitle, textObjectThanksText[5];
	textObject textObjAboutAppTitle, textObjAboutAppText[4];
	textObject textObjectAttribsTitle, textObjectAttribText[5], textObjectAttribURL[5], textObjectSymbols[5];

	textObject textObjectSetupUnitsButton, textObjectSetupUnitsLabel, textObjectSetupUnitsValue, textObjectSetupsResetStats;
	textObject textObjectSetupWebWxLabel, textObjectSetupWebWxValue, textObjectSetupWebWxButton;
	textObject textObjSetupOutdoorCalLabel, textObjSetupIndoorCalLabel;
	textObject textObjSettingsLabel, textObjInputBuffer, textObjDialogBoxMsg;

	olc::vi2d positionTemp;
	olc::vf2d positionSensorAreaStart, positionSensorAreaSize, posSensorAreaDividerStart, posSensorAreaDividerEnd;
	olc::vf2d positionSignalOutdoorCenter, positionSignalIndoorCenter, positionSignalMeterOutdoor, positionSignalMeterIndoor;
	olc::vf2d positionBatteryOutdoorLabel, positionBatteryIndoorLabel, positionChannelOutdoorLabel, positionChannelIndoorLabel;

	olc::vf2d positionWindowCenter, positionOutdoorAreaStart, positionOutdoorAreaSize;
	olc::vf2d positionIndoorAreaStart, positionIndoorAreaSize;
	olc::vf2d positionOutdoorTempValue, positionTempLabelF, positionOutdoorTempHighValue, positionOutdoorTempLowValue, positionOutdoorTrendOffset;
	olc::vf2d positionOutdoorHumidityValue, positionHumidityLowValue, positionHumidityHighValue;
	olc::vf2d positionDewPointLabel, positionDewPointValue, positionFeelsLikeLabel, positionFeelsLikeLabelSize, positionFeelsLikeValue;
	olc::vf2d positionLeftSideDivider;
	olc::vf2d positionUVindexLabel, positionUVindexValue, positionLightInfoNext, positionLightLevelLabel, positionLightLevelValue, positionUVindexGraph, positionUVindexArrow;

	// Wind-related position definitions
	olc::vf2d positionWindAvgOffset;
	olc::vf2d windCircleCenterPoint, positionWindSpeedAvgLabel, positionWindSpeedPeakLabel;
	olc::vi2d posWindDirectionText, windDirectionTextStart, windDirectionTextSize;

	olc::vi2d positionRainAreaStart, positionRainAreaSize, positionRainGauge, positionRainfallValue;
	olc::vi2d positionRainfallTodayLabel, positionRainfallTodayValue, positionRainfallRateLabel, positionRainfallRateValue;
	olc::vi2d rainGaugeTotalSize, rainGaugeFilledStart, rainGaugeFilledSize;

	olc::vf2d positionForecastAreaStart, positionForecastAreaSize, positionAboutAppBoxStart, positionAboutAppBoxSize;
	olc::vf2d positionSystemDate, positionSystemTime, positionCloseIcon, positionSettingsIcon;

	olc::vi2d setupWindowSize;
	int setupWindowLeftColumnX, setupWindowMiddleColumnX, setupWindowRightColumnX, setupWindowMiddleRowY, setupWindowBottomRowY;
	int setupSensorBoxLeftOffsetX, setupSensorBoxRightOffsetX, setupSensorBoxTopOffsetY, setupSensorBoxBottomOffsetY;
	int setupUnitsLabelY, setupWebWxLabelY, setupSdrExecPathLabelY;

	olc::vi2d positionSetupTextCursor, positionSetupOutdoorCalLabel, positionSetupIndoorCalLabel, positionSetupInfoButton;

	int spacerFontSize18, spacerFontSize24, spacerFontSize32;
	float sensorAreaDividerX;
	int infoAreaDividerX, infoAreaDividerStartY, infoAreaDividerEndY;

	olc::Font fontSize16, fontSize18, fontSize20, fontSize22, fontSize24, fontSize26, fontSize28, fontSize30, fontSize32, fontSize40, fontSize56, fontSize72, fontSize96;

	olc::vf2d positionDashCorrection18 = { 2, 7 };
	olc::vf2d positionDashCorrection24 = { 2, 9 };
	olc::vf2d positionDashCorrection32 = { 0, 11 };
	olc::vf2d positionDashCorrection40 = { 0, 13 };
	olc::vf2d positionDashCorrection56 = { 0, 16 };
	olc::vf2d positionDashCorrection96 = { 0, 12 };

	float windCircleRadius;
	float screenPaddingOffsetY;
	float outdoorAreaStartScaleX, outdoorAreaStartScaleY;
	float outdoorAreaSizeScaleX, outdoorAreaSizeScaleY;
	float outdoorMainValueScaleX, outdoorTempValueScaleY, outdoorHumidityValueScaleY;
	float outdoorHighLowOffsetY, outdoorTrendOffsetX, outdoorTrendOffsetY;
	float leftDividerOffset;

	std::time_t systemTimeNow, systemTimePrevious;
	std::string dayOfTheWeekAbbr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	float windDirNewPosition;
	float windDirCurVelocity = 0.0f;
	float windDirStiffness = 1.0f;  // Higher = faster response
	float windDirDamping = 0.5f;     // Higher = less oscillation
	//float dt = 1.0f / 60.0f;  // 60 FPS

	float rainGaugeFilledPercentage;
	int rainGaugeFilledHeight, rainGaugeTotalWidth = 60, rainGaugeTotalHeight = 280;
	rainGaugeTickMark rainGaugeTickMarks[rainGaugeMarksTotal + 1];

	// UV Index-related variables and definitions
	olc::Pixel uvPixelColorCurrent;
	const olc::Pixel uvPixelColors[5] = { { 0x00, 0xFF, 0x00 }, { 0xFF, 0xFF, 0x00 }, { 0xFF, 0xA5, 0x00 }, { 0xFF, 0x00, 0x00 }, { 0x80, 0x00, 0x80 } };
	const float uvSegmentRatios[4] = { 0.25f, 0.25f, 0.20f, 0.30f };
	float uvGraphTotalHeight, uvSegmentLength, uvGraphTotalWidth, uvSegmentSizes[4], uvSegmentOffsets[4];

	std::u32string strLightLevelValue;

	std::string strTitleAuthorText[3] = { "DragonWx  v1.0", "(Beta)", "Written by Todd Wallace" };

	std::u32string strIntroText[3] = {  U"DragonWx is a graphical frontend for displaying live weather data transmitted by many popular wireless home weather", 
										U"stations. It was written in C++ and leverages the olcPixelGameEngine library for rendering. It requires a functional install",
										U"of the open-source SDR tuner/decoder RTL_433 in order to work. Refer to the included README file for more information." };

	std::u32string strAboutApp[4] = {	U"The idea for this project came out of my frustration with the stock proprietary display units that ship with alot of the home",
										U"personal weather stations. In my experiences, they proved to either be unreliable, hard to view at certain angles, or would",
										U"just fail prematurely. I have a background in coding and knew what SDRs could do from being a Ham Radio operator, so I",
										U"decided to put them all together and make my OWN weather station display!" };

	std::u32string strThanksText[2] = { U"Special thanks to Kirstin Stich for her invaluable insight/input on the look and feel of the app, and to everyone on the ",
										U"OneLoneCoder Discord for all your help, advice, and patience with my coding questions! Thank you very much!" };
};