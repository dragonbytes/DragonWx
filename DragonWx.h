#pragma once

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TTF
#include "./olcPGEX_TTF-main/olcPGEX_TTF.h"

#define OLC_PGEX_SPLASHSCREEN
#include "olcPGEX_SplashScreen.h"

#include "Main.h"
#include "WxCodeDefs.h"

class DragonWx : public olc::PixelGameEngine
{
public:
	DragonWx();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	bool OnUserDestroy() override;

	bool mouseWithinArea(olc::vf2d, olc::vf2d, olc::vf2d);
	olc::vf2d GetCenteredStartPosition(olc::vf2d totalAreaSize, olc::vf2d objectAreaSize);
	std::u32string ConvertedString32(std::string inputString);
	olc::vf2d GetTextOffsetPosition(olc::vf2d startPos, olc::Font* fontToUse, std::string inputString);
	olc::vf2d GetTextOffsetPosition32(olc::vf2d startPos, olc::Font* fontToUse, std::u32string inputString32);
	void RenderString(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos);
	void RenderString32(std::u32string inputString32, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos);
	void RenderStringCentered(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d centerPos);
	void RenderStringRightJustified(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d rightPos);
	void RenderHighOrLowValue(std::string labelText, double highLowValue, olc::Renderable* renderableValue, olc::vf2d valuePos, bool useSmallerText);
	void DrawWindDirectionArrow(double degreesBearing, olc::Pixel arrowColor);
	void DrawCircleArc(olc::vf2d startPos, int radius, double startAngle, double endAngle, olc::Pixel pixelColor);
	//void DrawWindDirPreviousArc(olc::vf2d startPos, int radius, olc::Pixel pixelColor, uint8_t mask);
	olc::vf2d DrawBoxTitle(std::u32string strSectionTitle, olc::Font* fontToUse, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor);
	void DrawRainGauge(double gaugeFullValue, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor);
	void DrawUVindexGraph(olc::vf2d startPos, std::u32string strValue);
	void RenderCenteredWxCondition(olc::vf2d centeredPos, olc::Decal* decalToDraw, olc::Renderable* renderable);
	void RenderCenteredWxForecast(olc::vf2d centeredPos, olc::Decal* decalToDraw, wxWebEntry* wxWebEntryPtr, olc::Renderable* renderableText);
	olc::Decal* UpdateTrendData(std::deque<double>* sourceDeque, double dataValue, olc::Decal* decalTarget, std::string debugTextLabel);
	std::string GetWindDirectionName(double windDirDegrees);
	void MidnightDailyReset();
	bool LoadWebWxAssets(wxWebEntry* wxDataEntry, olc::Decal* decalTarget);
	bool SaveConfigFile();

	//olc::SplashScreen splash;

	bool mouseWaitingButtonRelease, useFeelsLikeLabel, useLuxValue;
	olc::Key keyPrevious;
	std::string strDewpointLabel = "Dewpoint";

	std::u32string strPanelNameOutdoor32 = U"Outdoor";
	std::u32string strPanelNameIndoor32 = U"Indoor";
	std::u32string strPanelNameSensors32 = U"Sensors";
	std::u32string strPanelNameRainfall32 = U"Rainfall";
	std::u32string strPanelNameConditions32 = U"Forecast";

	int spriteLayers[3];

	olc::Pixel colorLabelText;
	olc::Pixel rainGaugeBorderColor;
	olc::Pixel areasBorderColor;

	olc::Sprite* backgroundSprite;
	olc::Decal* backgroundDecal;
	olc::Sprite* backgroundAlphaSprite;
	olc::Decal* backgroundAlphaDecal;
	olc::Sprite* spriteSettingsScreen;
	olc::Decal* decalSettingsScreen;
	olc::Sprite* spriteSettingsIcon;
	olc::Decal* decalSettingsIcon;

	olc::Sprite* spriteThermometer;
	olc::Decal* decalThermometer;
	olc::Sprite* spriteWaterDrop;
	olc::Decal* decalWaterDrop;
	olc::Sprite* spriteRainfallIcon;
	olc::Decal* decalRainfallIcon;

	olc::Sprite* spriteWindDir;
	olc::Decal* decalWindDir;
	olc::vf2d centerPointWindDir;

	olc::Sprite* spriteWindCircle;
	olc::Decal* decalWindCircle;

	olc::Sprite* spriteRainGaugeBorder;
	olc::Decal* decalRainGaugeBorder;
	olc::Sprite* spriteRainGauge;
	olc::Decal* decalRainGauge;
	olc::Sprite* spriteRainGaugeClear;

	olc::Sprite* spriteSignalStrength[5];
	olc::Decal* decalSignalStrength[5];

	olc::Sprite* spriteTrendArrowUp;
	olc::Decal* decalTrendArrowUp;
	olc::Sprite* spriteTrendArrowSteady;
	olc::Decal* decalTrendArrowSteady;
	olc::Sprite* spriteTrendArrowDown;
	olc::Decal* decalTrendArrowDown;

	olc::Decal* decalTrendOutdoorTemp;
	olc::Decal* decalTrendOutdoorHumidity;

	olc::Sprite* spriteWebConditionsImage;
	olc::Decal* decalWebConditionsImage;
	olc::Sprite* spriteWebForecastImages[3];
	olc::Decal* decalWebForecastImages[3];

	olc::Renderable renderableDateText, renderableTimeText, renderableIndoorLabel, renderableOutdoorLabel, renderableAppNameLabel, renderableSettingsLabel;

	olc::Renderable renderableLowLabel18, renderableHighLabel18, renderableLowLabel24, renderableHighLabel24;
	olc::Renderable renderableIndoorLowValue, renderableIndoorHighValue;

	olc::Renderable renderableLabelTempF, renderableDewPointUnits, renderableLabelDewpoint;
	olc::Renderable renderableTempValue, renderableTempDecimalValue, renderableOutdoorTempLowValue, renderableOutdoorTempHighValue;
	olc::Renderable renderableIndoorTempUnits, renderableIndoorTempValue, renderableIndoorTempLowValue, renderableIndoorTempHighValue;
	olc::Renderable renderableFeelsLikeLabel, renderableFeelsLikeValue, renderableFeelsLikeUnits;
	olc::Renderable renderableUVindexLabel, renderableUVindexValue, renderableLightLevelLabel, renderableLightLevelValue, renderableLightLeveilUnits;

	//olc::Renderable renderableLabelHumidityLow, renderableLabelHumidityHigh;
	olc::Renderable renderableLabelHumidity, renderableHumidityUnits, renderableHumidityValue, renderableHumidityLowValue, renderableHumidityHighValue, renderableDewpointValue;
	olc::Renderable renderableIndoorHumidityUnits, renderableIndoorHumidityValue;

	olc::Renderable windSpeedUnitsText, renderableLabelWindSpeedAvg, windSpeedText, renderableWindSpeedAvg, renderableWindSpeedHighLabel, renderableWindSpeedHighValue;
	olc::Renderable renderableWindSpeedUnits, renderableWindDirName;

	olc::Renderable renderableRainfallLabel, renderbleRainfallTodayLabel, renderableRainfallRateLabel, renderableRainTodayUnitsLabel, renderableRainfallRateUnits;
	olc::Renderable renderableRainfallValue, renderableRainfallRateValue, renderableRainGaugeUnits[rainGaugeMarksTotal + 1];

	olc::Renderable renderableSensorInfoLabel, renderableSignalLabel, renderableSensorOutdoorLabel, renderableSensorIndoorLabel;
	olc::Renderable renderableSignalOutdoorLabel, renderableSignalIndoorLabel;
	olc::Renderable renderableBatteryOutdoorLabel, renderableBatteryIndoorLabel, renderableBatteryOutdoorValue, renderableBatteryIndoorValue;
	olc::Renderable renderableChannelOutdoorLabel, renderableChannelOutdoorValue, renderableChannelIndoorLabel, renderableChannelIndoorValue;

	olc::Renderable renderableForecastNowLabel, renderableForecastToday, renderableForecastTomorrow, renderableConditionsDesc;
	olc::Renderable renderableForecastDays[3], renderableForecastText[3];

	olc::vf2d positionTemp, positionSensorInfoTitle, positionSensorAreaStart, positionSensorAreaSize;
	olc::vf2d positionSignalOutdoorCenter, positionSignalIndoorCenter, positionSignalMeterOutdoor, positionSignalMeterIndoor;
	olc::vf2d positionBatteryOutdoorLabel, positionBatteryIndoorLabel, positionChannelOutdoorLabel, positionChannelIndoorLabel;

	olc::vf2d positionWindowCenter, positionOutdoorAreaStart, positionOutdoorAreaSize, positionOutdoorTitle;
	olc::vf2d positionIndoorAreaStart, positionIndoorAreaSize, positionIndoorTitle;
	olc::vf2d positionOutdoorTempValueF, positionTempLabelF, positionOutdoorTempHighValue, positionOutdoorTempLowValue, positionOutdoorTrendOffset;
	olc::vf2d positionOutdoorHumidityValue, positionHumidityLowValue, positionHumidityHighValue;
	olc::vf2d positionDewPointLabel, positionDewPointValue, positionFeelsLikeLabel, positionFeelsLikeLabelSize, positionFeelsLikeValue;
	olc::vf2d positionLeftSideDivider;
	olc::vf2d positionUVindexLabel, positionUVindexValue, positionLightInfoNext, positionLightLevelLabel, positionLightLevelValue, positionUVindexGraph, positionUVindexArrow;
	olc::vf2d windCircleCenterPoint, positionWindSpeedAvgLabel, positionWindSpeedHighLabel;	// Wind-related position definitions

	olc::vf2d positionRainAreaStart, positionRainAreaSize, positionRainAreaTitle;
	olc::vf2d rainGaugeFilledStart, rainGaugeFilledSize, rainGaugeTotalSize, rainGaugeFilledStartPrev, rainGaugeFilledSizePrev;

	olc::vf2d positionInfoAreaStart, positionInfoAreaSize, positionInfoAreaTitle, positionInfoAreaGearIcon;

	olc::vf2d positionAnimationSheetOffset = { 0, 0 };

	int spacerFontSize18, spacerFontSize24, spacerFontSize32;
	int sensorAreaDividerX;
	int infoAreaDividerX, infoAreaDividerStartY, infoAreaDividerEndY;

	olc::Font fontSize18, fontSize22, fontSize24, fontSize32, fontSize40, fontSize56, fontSize72, fontSize96;

	olc::vf2d positionDashCorrection18 = { 2, 7 };
	olc::vf2d positionDashCorrection24 = { 2, 9 };
	olc::vf2d positionDashCorrection32 = { 0, 11 };
	olc::vf2d positionDashCorrection40 = { 0, 13 };
	olc::vf2d positionDashCorrection56 = { 0, 16 };
	olc::vf2d positionDashCorrection96 = { 0, 12 };

	std::u32string strFeelsLikeLabel;

	float windCircleRadius;

	float screenPaddingOffsetY;
	float outdoorAreaStartScaleX, outdoorAreaStartScaleY;
	float outdoorAreaSizeScaleX, outdoorAreaSizeScaleY;

	float outdoorMainValueScaleX, outdoorTempValueScaleY, outdoorHumidityValueScaleY;

	float outdoorHighLowOffsetY, outdoorTrendOffsetX, outdoorTrendOffsetY;

	float leftDividerOffset;

	std::time_t systemTimeNow, systemTimePrevious;
	std::string dayOfTheWeekAbbr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	float animationElapsedTime = 0.0f;

	int wxIconAnimatedCounter;

	float rainGaugeFilledPercentage;
	float rainGaugeTotalWidth = 60.0f;
	//float rainGaugeTotalHeight = 352.0f;
	float rainGaugeTotalHeight = 300.0f;
	float rainGaugeFilledHeight;

	// UV Index-related variables and definitions
	const olc::Pixel uvPixelColors[5] = { { 0x00, 0xFF, 0x00 }, { 0xFF, 0xFF, 0x00 }, { 0xFF, 0xA5, 0x00 }, { 0xFF, 0x00, 0x00 }, { 0x80, 0x00, 0x80 } };
	const float uvSegmentRatios[4] = { 0.25f, 0.25f, 0.20f, 0.30f };
	float uvGraphTotalHeight, uvSegmentLength, uvGraphTotalWidth, uvSegmentSizes[4], uvSegmentOffsets[4];

	std::u32string strLightLevelValue;
};