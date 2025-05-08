
#include "olcPixelGameEngine.h"
#include "./olcPGEX_TTF-main/olcPGEX_TTF.h"
#include "DragonWx.h"

// Override base class with your custom functionality
DragonWx::DragonWx()
{
	sAppName = "DragonWeather";
}

bool DragonWx::OnUserCreate()
{
	mouseWaitingButtonRelease = false;
	useFeelsLikeLabel = true;
	useLuxValue = false;
	useNumericWindDirection = false;

	setupWindowSize = GetWindowSize();

	outdoorSensor.telemetryStarted = false;
	outdoorSensor.recentlyUpdated = false;
	outdoorSensor.packetCounter = 0;
	outdoorSensor.batteryStatus = undefinedFloatValue;

	indoorSensor.telemetryStarted = false;
	indoorSensor.recentlyUpdated = false;
	indoorSensor.packetCounter = 0;
	indoorSensor.batteryStatus = undefinedFloatValue;

	if (!useRealPipe)
		populateTestData();
	else
		dequeWindDirections.push_front(0.0f);		// Set initial wind arrow direction to North

	colorLabelText = { 48, 139, 151 };
	rainGaugeBorderColor = { 53, 157, 242 };
	areasBorderColor = { 100, 100, 100 };

	positionWindowCenter = { float(GetWindowSize().x / 2.0f), float(GetWindowSize().y / 2.0f) };
	PRINT_DEBUG("Window Center: x = %f, y = %f\n", positionWindowCenter.x, positionWindowCenter.y);
	// Called once at the start, so create things here
	olc::Font::init();

	//fontMap.emplace(16, olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 16));

	fontSize16 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 16);
	fontSize18 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 18);
	fontSize22 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 22);
	fontSize24 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 24);
	fontSize26 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 26);
	fontSize28 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 28);
	fontSize30 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 30);
	fontSize32 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 32);
	fontSize40 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 40);
	fontSize56 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 56);
	fontSize72 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 72);
	fontSize96 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 96);

	spacerFontSize18 = fontSize18.GetStringBounds(U"-").size.x;
	spacerFontSize24 = fontSize24.GetStringBounds(U"-").size.x;
	spacerFontSize32 = fontSize32.GetStringBounds(U"-").size.x;

	renderableBackgroundImage.Load("./Images/Background_3840x2160.png");
	renderableAreaBorders.Create(setupWindowSize.x, setupWindowSize.y);
	renderableRainGaugeOutline.Create(setupWindowSize.x, setupWindowSize.y);
	renderableSetupScreen.Create(setupWindowSize.x, setupWindowSize.y);
	renderableInfoScreen.Create(setupWindowSize.x, setupWindowSize.y);

	renderableThermometerIconC.Load("./Images/ThermometerC_44px.png");
	renderableThermometerIconF.Load("./Images/ThermometerF_44px.png");

	renderableWaterDropIcon.Load("./Images/Humidity_40px.png");
	renderableRainfallIcon.Load("./Images/Raincloud_32px.png");


	renderableWindDir.Create(46, 46);
	centerPointWindDir = renderableWindDir.Sprite()->Size() / olc::vi2d(2, 2);
	//spriteWindDir = new olc::Sprite(46, 46);

	//spriteWindCircle = new olc::Sprite(258, 258);
	//SetDrawTarget(spriteWindCircle);
	//Clear(olc::BLANK);
	//DrawCircle({ spriteWindCircle->Size().x / 2, spriteWindCircle->Size().y / 2 }, 128, rainGaugeBorderColor);
	//decalWindCircle = new olc::Decal(spriteWindCircle);

	renderableRainGaugeWater.Load("./Images/RainGaugeWater.png");

	std::string filePathSignalIcon;
	for (int i = 0; i < 5; i++)
	{
		filePathSignalIcon = "./Images/Signal_" + std::to_string(i) + ".png";
		if (renderableSignalStrength[i].Load(filePathSignalIcon) != olc::rcode::OK)
			PRINT_DEBUG("Signal Bar icon file missing\m");
	}

	// Load and setup the sprites/decals for the trending arrows (up, down, and steady)
	renderableTrendArrowUp.Load("./Images/TrendUp.png");
	renderableTrendArrowSteady.Load("./Images/TrendSteady.png");
	renderableTrendArrowDown.Load("./Images/TrendDown.png");

	renderableSettingsIcon.Load("./Images/Gear_24px.png");
	renderableCloseIcon.Load("./Images/Close_24px.png");
	renderableInfoIcon.Load("./Images/Info_24px.png");
	renderableDragonLogo.Load("./Images/TekTodd_Logo_160px.png");

	renderableWebConditionsImage.Load("./Images/Conditions_Icons/not-available.png");

	for (int i = 0; i < 3; i++)
		renderableWebForecastImages[i].Load("./Images/Conditions_Icons/not-available.png");

	strFeelsLikeLabel = U"Feels Like";

	screenPaddingOffsetY = GetWindowSize().y * 0.04f;
	outdoorAreaStartScaleX = 0.03125f;
	outdoorAreaStartScaleY = 0.07f;
	outdoorAreaSizeScaleX = 0.25f;
	outdoorAreaSizeScaleY = 0.48f;
	float sensorAreaStartScaleY = 0.75f;

	positionOutdoorAreaStart = { GetWindowSize().x * outdoorAreaStartScaleX, screenPaddingOffsetY };
	positionOutdoorAreaSize = { GetWindowSize().x * outdoorAreaSizeScaleX, GetWindowSize().y * outdoorAreaSizeScaleY };

	positionSensorAreaStart = GetWindowSize() * olc::vf2d(0.72656f, 0.73611f);
	positionSensorAreaSize = { 310, 170 };

	positionSignalOutdoorCenter = positionSensorAreaStart + olc::vf2d(positionSensorAreaSize.x * 0.25f, 35);
	positionSignalIndoorCenter = { positionSensorAreaStart.x + positionSensorAreaSize.x - (positionSensorAreaSize.x * 0.25f), positionSensorAreaStart.y + 35 };
	positionSignalMeterOutdoor = positionSignalOutdoorCenter + olc::vf2d(9, 50 - (fontSize18.GetStringBounds(U"Signal:").size.y / 2));
	positionSignalMeterIndoor = positionSignalIndoorCenter + olc::vf2d(9, 50 - (fontSize18.GetStringBounds(U"Signal:").size.y / 2));

	positionBatteryOutdoorLabel = positionSignalOutdoorCenter + olc::vf2d(-5, 75);
	positionBatteryIndoorLabel = positionSignalIndoorCenter + olc::vf2d(-5, 75);
	positionChannelOutdoorLabel = positionSignalOutdoorCenter + olc::vf2d(-5, 100);
	positionChannelIndoorLabel = positionSignalIndoorCenter + olc::vf2d(-5, 100);

	positionIndoorAreaStart = GetWindowSize() * olc::vf2d(0.03125f, 0.84f);
	positionIndoorAreaSize = { GetWindowSize().x * outdoorAreaSizeScaleX, (positionSensorAreaStart.y + positionSensorAreaSize.y) - positionIndoorAreaStart.y };

	positionRainAreaStart = olc::vi2d(positionSensorAreaStart.x, positionOutdoorAreaStart.y);
	positionRainAreaSize = olc::vi2d(positionSensorAreaSize.x, GetWindowSize().y * 0.61f);
	positionRainGauge = { positionRainAreaStart.x + (positionRainAreaSize.x / 2), positionRainAreaStart.y + 40 };
	positionRainfallValue = { positionRainGauge.x + (rainGaugeTotalWidth / 2), positionRainGauge.y + rainGaugeTotalHeight + 20 };
	positionRainfallTodayLabel = positionRainAreaStart + olc::vf2d(25, positionRainAreaSize.y - 90);
	positionRainfallTodayValue = { positionRainGauge.x, positionRainfallTodayLabel.y };
	positionRainfallRateLabel = positionRainfallTodayLabel + olc::vf2d(0, 45);
	positionRainfallRateValue = { positionRainGauge.x, positionRainfallRateLabel.y };

	olc::vf2d positionOutdoorTempOffset = positionOutdoorAreaSize * olc::vf2d(0.62f, 0.14468f);
	olc::vf2d positionOutdoorHumidityOffset = positionOutdoorAreaSize * olc::vf2d(0.62f, 0.60764f);
	olc::vf2d positionOutdoorLowLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.35f, 0.260412f);	// Target distance of 110 pixels between High/Low
	olc::vf2d positionOutdoorHighLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.02f, 0.260412f);

	//positionOutdoorTrendOffset = positionOutdoorAreaSize * olc::vf2d(0.18f, 0.115f);
	positionOutdoorTrendOffset = { positionOutdoorAreaSize.x * 0.18f, TextCenteredOffsetY("0", &fontSize96).y - ((renderableTrendArrowSteady.Sprite()->height * 0.29f) / 2.0f) };

	positionOutdoorTempValue = positionOutdoorAreaStart + positionOutdoorTempOffset;
	positionOutdoorTempLowValue = positionOutdoorTempValue + positionOutdoorLowLabelOffset;
	positionOutdoorTempHighValue = positionOutdoorTempValue + positionOutdoorHighLabelOffset;

	positionOutdoorHumidityValue = positionOutdoorAreaStart + positionOutdoorHumidityOffset;
	positionHumidityLowValue = positionOutdoorHumidityValue + positionOutdoorLowLabelOffset;
	positionHumidityHighValue = positionOutdoorHumidityValue + positionOutdoorHighLabelOffset;

	positionDewPointLabel = positionOutdoorAreaStart + olc::vf2d(0, 365);
	positionDewPointValue = positionDewPointLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);

	positionFeelsLikeLabel = positionDewPointLabel + olc::vf2d(0, 45);
	positionFeelsLikeValue = positionFeelsLikeLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);

	positionLeftSideDivider = positionFeelsLikeLabel + olc::vf2d(0, 50);

	//positionLightInfoNext = positionLeftSideDivider + olc::vf2d(0, 20);
	//positionLightLevelValue = positionLightInfo + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);

	//uvGraphTotalHeight = (fontSize32.GetStringBounds(U"11").size.y / 2.0f);
	//positionUVindexLabel = positionLightInfo + olc::vf2d(0, 35);
	//positionUVindexValue = positionUVindexLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);
	//positionUVindexGraph = positionUVindexValue + olc::vf2d(fontSize32.GetStringBounds(U"11").size.x + 5.0f, uvGraphTotalHeight - (uvGraphTotalHeight / 2.0f));


	positionForecastAreaSize = { GetWindowSize().x * 0.3f, GetWindowSize().y * 0.2f };
	positionForecastAreaStart = { positionWindowCenter.x - (positionForecastAreaSize.x / 2), positionIndoorAreaStart.y + positionIndoorAreaSize.y - positionForecastAreaSize.y };
	//positionInfoAreaGearIcon = positionInfoAreaStart + (positionInfoAreaSize * olc::vf2d(0.90f, 0.50f)) - (spriteSettingsIcon->Size() / olc::vi2d(2, 2));

	infoAreaDividerX = positionForecastAreaStart.x + (positionForecastAreaSize.x * 0.3f);
	infoAreaDividerStartY = positionForecastAreaStart.y + (positionForecastAreaSize.y * 0.23f);
	infoAreaDividerEndY = positionForecastAreaStart.y + positionForecastAreaSize.y - (positionForecastAreaSize.y * 0.2f);

	windCircleRadius = 128.0f;
	PRINT_DEBUG("Debug: Wind Circle Radius = %.1f\n", windCircleRadius);
	//olc::vf2d positionWindAvgOffset = { windCircleRadius * -1.2f, windCircleRadius * 1.1875f };
	//olc::vf2d positionWindHighOffset = { windCircleRadius * 0.7f, windCircleRadius * 1.1875f };

	rainGaugeTotalSize = { rainGaugeTotalWidth, rainGaugeTotalHeight };

	positionSystemDate = { positionWindowCenter.x, positionOutdoorAreaStart.y };
	positionSystemTime = { positionWindowCenter.x, positionOutdoorAreaStart.y + 50.0f };

	/*
	// Configure dimensions and position info of the UV Index graph decals
	uvGraphTotalWidth = (positionOutdoorAreaStart.x + positionOutdoorAreaSize.x - positionUVindexGraph.x);
	//positionUVindexGraph += olc::vf2d((positionOutdoorAreaSize.x * 0.10f) / 2.0f, 0);	// Adjust the position of graph to center it within the Outdoor Area Width
	uvSegmentLength = uvGraphTotalWidth / 11.0f;
	float segmentOffsetCounter = 0.0f;
	for (int i = 0; i < 4; i++)
	{
		uvSegmentSizes[i] = uvSegmentRatios[i] * uvGraphTotalWidth;
		uvSegmentOffsets[i] = segmentOffsetCounter;
		segmentOffsetCounter += uvSegmentSizes[i];
	}
	*/

	// DEBUG: Report calculated Position coordinates
	PRINT_DEBUG("Debug: Outdoor Area Start Coords = %.1f, %.1f\n", positionOutdoorAreaStart.x, positionOutdoorAreaStart.y);
	PRINT_DEBUG("Debug: Outdoor Area Size = %.1f, %.1f\n", positionOutdoorAreaSize.x, positionOutdoorAreaSize.y);
	PRINT_DEBUG("Debug: Outdoor High/Low Offset = %.1f, %.1f\n", positionOutdoorHighLabelOffset.x, positionOutdoorHighLabelOffset.y);
	PRINT_DEBUG("Debug: Outdoor Temp Value Coords = %.1f, %.1f\n", positionOutdoorTempValue.x, positionOutdoorTempValue.y);
	PRINT_DEBUG("Debug: Outdoor Temp Low Coords = %.1f, %.1f\n", positionOutdoorTempLowValue.x, positionOutdoorTempLowValue.y);
	PRINT_DEBUG("Debug: Outdoor Temp High Coords = %.1f, %.1f\n", positionOutdoorTempHighValue.x, positionOutdoorTempHighValue.y);
	PRINT_DEBUG("Debug: Outdoor Humidity Value Coords = %.1f, %.1f\n", positionOutdoorHumidityValue.x, positionOutdoorHumidityValue.y);
	PRINT_DEBUG("Debug: Outdoor Humidity Low Coords = %.1f, %.1f\n", positionHumidityLowValue.x, positionHumidityLowValue.y);
	PRINT_DEBUG("Debug: Outdoor Humidity High Coords = %.1f, %.1f\n", positionHumidityHighValue.x, positionHumidityHighValue.y);
	PRINT_DEBUG("Debug: Outdoor Trend Arrow Offset = %.1f, %.1f\n", positionOutdoorTrendOffset.x, positionOutdoorTrendOffset.y);

	renderableLowLabel18 = fontSize18.RenderStringToRenderable(U"Low ", colorLabelText);
	renderableHighLabel18 = fontSize18.RenderStringToRenderable(U"High ", colorLabelText);
	renderableLowLabel24 = fontSize24.RenderStringToRenderable(U"Low ", colorLabelText);
	renderableHighLabel24 = fontSize24.RenderStringToRenderable(U"High ", colorLabelText);

	// Configure the various screen sprite layers and pre-render the corresponding objects inside them
	SetDrawTarget(renderableWindDir.Sprite());
	Clear(olc::BLANK);
	FillTriangle({ 0, 0 }, { renderableWindDir.Sprite()->Size().x, 0 }, { renderableWindDir.Sprite()->Size().x / 2, renderableWindDir.Sprite()->Size().y }, olc::WHITE);
	renderableWindDir.Decal()->Update();

	DrawRainGaugeOutlines(positionRainGauge - olc::vf2d(1, 1), rainGaugeTotalSize + olc::vf2d(1, 1), 10, rainGaugeBorderColor);

	olc::vi2d setupSensorBoxSize = { 350, 300 };
	int setupOutdoorTitleBoxX = setupWindowSize.x * 0.04f;
	int setupIndoorTitleBoxX = setupWindowSize.x * 0.37f;
	int setupSensorTitleBoxesY = setupWindowSize.y * 0.13f;
	int inputBoxHeight = 40;

	//titleBoxSetup = { U"Settings", &fontSize32, areasBorderColor, positionSetupWindow, setupWindowSize, 20, 45 };
	titleBoxSetupOutdoor = { U"Outdoor Sensor", &fontSize30, areasBorderColor, { setupOutdoorTitleBoxX, setupSensorTitleBoxesY }, setupSensorBoxSize, 20, 45};
	titleBoxSetupIndoor = { U"Indoor Sensor", &fontSize30, areasBorderColor, { setupIndoorTitleBoxX, setupSensorTitleBoxesY }, setupSensorBoxSize, 20, 45 };

	SetDrawTarget(renderableSetupScreen.Sprite());
	Clear(olc::BLANK);
	DrawBoxTitle(&titleBoxSetupOutdoor, &renderableSetupScreen);
	DrawBoxTitle(&titleBoxSetupIndoor, &renderableSetupScreen);
	renderableSetupScreen.Decal()->Update();

	//setupWinBoxLeftColumnX = titleBoxSetupOutdoor.posStart.x + (titleBoxSetupOutdoor.size.x * 0.15f);
	//setupWinBoxMiddleColumnX = positionSetupWinBox.x + (setupWinBoxSize.x * 0.42f);
	//setupWinBoxRightColumnX = titleBoxSetup.posStart.x + (setupWinBoxSize.x * 0.68f);
	//setupWindowTopRowY = setupWindowSize.y * 0.12f;
	setupWindowMiddleRowY = setupWindowSize.y * 0.3f;
	setupWindowBottomRowY = setupWindowSize.y * 0.59f;

	setupSensorBoxLeftOffsetX = setupSensorBoxSize.x * 0.11f;
	setupSensorBoxRightOffsetX = setupSensorBoxSize.x * 0.48f;
	setupSensorBoxTopOffsetY = setupSensorBoxSize.y * 0.2f;
	setupSensorBoxBottomOffsetY = setupSensorBoxSize.y * 0.45f;

	setupWindowLeftColumnX = setupOutdoorTitleBoxX + setupSensorBoxLeftOffsetX;
	//setupWindowMiddleColumnX = setupWindowSize.x * 0.42f;
	setupWindowMiddleColumnX = setupIndoorTitleBoxX + setupSensorBoxLeftOffsetX;
	setupWindowRightColumnX = setupWindowSize.x * 0.71f;

	setupUnitsLabelY = setupSensorTitleBoxesY;
	setupWebWxLabelY = setupWindowMiddleRowY;

	//setupUnitsButtonSize = { 150, inputBoxHeight };
	//setupWebWxButtonSize = { 120, inputBoxHeight };
	//positionSetupUnitsButton = { setupWindowRightColumnX, setupUnitsLabelY + 40 };
	//positionSetupWebWxButton = { setupWindowRightColumnX, setupWebWxLabelY + 40 };

	positionSetupOutdoorCalLabel = titleBoxSetupOutdoor.posStart + olc::vi2d(setupSensorBoxLeftOffsetX, setupSensorBoxBottomOffsetY);
	positionSetupIndoorCalLabel = titleBoxSetupIndoor.posStart + olc::vi2d(setupSensorBoxLeftOffsetX, setupSensorBoxBottomOffsetY);
	setupSdrExecPathLabelY = setupWindowSize.y * 0.74f;

	inputBoxOutdoorID.isEnabled = true;
	inputBoxOutdoorID.label = { U"Sensor ID", titleBoxSetupOutdoor.posStart + olc::vi2d(setupSensorBoxLeftOffsetX, setupSensorBoxTopOffsetY) };
	int tempInt = (inputBoxHeight - fontSize30.GetStringBounds(inputBoxOutdoorID.label.text32).size.y) / 2;
	olc::vi2d tempPos = { inputBoxOutdoorID.label.pos.x + fontSize30.GetStringBounds(inputBoxOutdoorID.label.text32).size.x + 20, inputBoxOutdoorID.label.pos.y - tempInt };
	inputBoxOutdoorID.value = { tempPos, { 140, inputBoxHeight }, inputBoxType::STRING };

	inputBoxIndoorID.isEnabled = true;
	inputBoxIndoorID.label = { U"Sensor ID", titleBoxSetupIndoor.posStart + olc::vi2d(setupSensorBoxLeftOffsetX, setupSensorBoxTopOffsetY) };
	tempInt = (inputBoxHeight - fontSize30.GetStringBounds(inputBoxIndoorID.label.text32).size.y) / 2;
	tempPos = { inputBoxIndoorID.label.pos.x + fontSize30.GetStringBounds(inputBoxIndoorID.label.text32).size.x + 18, inputBoxIndoorID.label.pos.y - tempInt };
	inputBoxIndoorID.value = { tempPos, { 140, inputBoxHeight }, inputBoxType::STRING };

	inputBoxIndoorCalTemp.isEnabled = true;
	inputBoxIndoorCalTemp.label = { U"Temperature", positionSetupIndoorCalLabel + olc::vi2d(0, 40) };
	inputBoxIndoorCalTemp.value = { inputBoxIndoorCalTemp.label.pos + olc::vi2d(0, 30), { 100, inputBoxHeight }, inputBoxType::FLOAT };

	inputBoxIndoorCalHumidity.isEnabled = true;
	inputBoxIndoorCalHumidity.label = { U"Humidity", inputBoxIndoorCalTemp.label.pos + olc::vi2d(setupSensorBoxRightOffsetX, 0) };
	inputBoxIndoorCalHumidity.value = { inputBoxIndoorCalHumidity.label.pos + olc::vi2d(0, 30), { 100, inputBoxHeight }, inputBoxType::INT };

	inputBoxOutdoorCalTemp.isEnabled = true;
	inputBoxOutdoorCalTemp.label = { U"Temperature", positionSetupOutdoorCalLabel + olc::vi2d(0, 40) };
	inputBoxOutdoorCalTemp.value = { inputBoxOutdoorCalTemp.label.pos + olc::vi2d(0, 30), { 100, inputBoxHeight }, inputBoxType::FLOAT };

	inputBoxOutdoorCalHumidity.isEnabled = true;
	inputBoxOutdoorCalHumidity.label = { U"Humidity", inputBoxOutdoorCalTemp.label.pos + olc::vi2d(setupSensorBoxRightOffsetX, 0) };
	inputBoxOutdoorCalHumidity.value = { inputBoxOutdoorCalHumidity.label.pos + olc::vi2d(0, 30), { 100, inputBoxHeight }, inputBoxType::INT };

	buttonUnitsToggle = { true, &fontSize24, olc::WHITE, { setupWindowRightColumnX, setupUnitsLabelY + 40 }, { 150, inputBoxHeight } };
	buttonForecastOnOff = { true, &fontSize24, olc::WHITE, { setupWindowRightColumnX, setupWebWxLabelY + 40 }, { 120, inputBoxHeight } };

	inputBoxSdrGain.isEnabled = true;
	inputBoxSdrGain.label = { U"SDR Gain", { setupWindowMiddleColumnX, setupWindowBottomRowY } };
	inputBoxSdrGain.value = { inputBoxSdrGain.label.pos + olc::vi2d(0, 35), { 100, inputBoxHeight }, inputBoxType::STRING };

	inputBoxSdrParams.isEnabled = true;
	inputBoxSdrParams.label = { U"RTL433 Params", { setupWindowLeftColumnX, setupWindowBottomRowY } };
	int setupSdrParamsBoxLength = (inputBoxOutdoorCalHumidity.value.pos.x + inputBoxOutdoorCalHumidity.value.size.x) - inputBoxOutdoorCalTemp.value.pos.x;
	inputBoxSdrParams.value = { inputBoxSdrParams.label.pos + olc::vi2d(0, 35), { setupSdrParamsBoxLength, inputBoxHeight }, inputBoxType::STRING };
	//inputBoxSdrParams.value = { &sdrExtraArguments, inputBoxSdrParams.label.pos + olc::vi2d(0, 35), { 300, inputBoxHeight } };

	inputBoxLatitude.isEnabled = webWxEnabled;
	inputBoxLatitude.label = { U"Latitude", { setupWindowRightColumnX, setupWebWxLabelY + 95 } };
	inputBoxLatitude.value = { inputBoxLatitude.label.pos + olc::vi2d(0, 30), { 140, inputBoxHeight }, inputBoxType::FLOAT };

	inputBoxLongitude.isEnabled = webWxEnabled;
	inputBoxLongitude.label = { U"Longitude", inputBoxLatitude.label.pos + olc::vi2d(160, 0) };
	inputBoxLongitude.value = { inputBoxLongitude.label.pos + olc::vi2d(0, 30), { 140, inputBoxHeight }, inputBoxType::FLOAT };

	inputBoxSdrExecPath.isEnabled = true;
	inputBoxSdrExecPath.label = { U"RTL433 Exec Path", { setupWindowLeftColumnX, setupSdrExecPathLabelY } };
	//int setupSdrExecPathBoxLength = (inputBoxIndoorCalHumidity.value.pos.x + inputBoxIndoorCalHumidity.value.size.x) - inputBoxOutdoorCalTemp.value.pos.x;
	int setupSdrExecPathBoxLength = (inputBoxLongitude.value.pos.x + inputBoxLongitude.value.size.x) - inputBoxOutdoorCalTemp.value.pos.x;
	inputBoxSdrExecPath.value = { inputBoxSdrExecPath.label.pos + olc::vi2d(0, 35), { setupSdrExecPathBoxLength, inputBoxHeight }, inputBoxType::STRING };

	int setupOkCancelButtonWidth = 120, setupButtonPaddingX = 20;
	int setupOkCancelButtonsX = (inputBoxLongitude.value.pos.x + inputBoxLongitude.value.size.x) - (setupOkCancelButtonWidth * 2) - setupButtonPaddingX;
	int setupButtonsBottomY = setupWindowSize.y - 75;
	//buttonStartStopRTL433 = { true, &fontSize24, olc::WHITE, { buttonResetStats.pos.x + buttonResetStats.size.x + setupButtonPaddingX, setupButtonsBottomY }, { 190, inputBoxHeight } };
	buttonStartStopRTL433 = { true, &fontSize24, olc::WHITE, { setupWindowLeftColumnX, setupButtonsBottomY }, { 190, inputBoxHeight } };
	buttonResetStats = { true, &fontSize24, olc::WHITE, { buttonStartStopRTL433.pos.x + buttonStartStopRTL433.size.x + setupButtonPaddingX, setupButtonsBottomY }, { 190, inputBoxHeight }, U"Reset Statistics" };
	buttonOk = { true, &fontSize24, olc::WHITE, { setupOkCancelButtonsX, setupButtonsBottomY }, { setupOkCancelButtonWidth, inputBoxHeight }, U"Ok" };
	buttonCancel = { true, &fontSize24, olc::WHITE, { setupOkCancelButtonsX + buttonOk.size.x + setupButtonPaddingX, setupButtonsBottomY }, { setupOkCancelButtonWidth, inputBoxHeight }, U"Cancel" };
	
	//buttonAboutApp = { true, &fontSize24, olc::WHITE, { (buttonOk.pos.x - setupButtonPaddingX - 90), setupButtonsBottomY }, { 90, inputBoxHeight }, U"About" };
	buttonAboutApp = { true, &fontSize24, olc::WHITE, { (buttonResetStats.pos.x + buttonResetStats.size.x + setupButtonPaddingX), setupButtonsBottomY }, { 190, inputBoxHeight }, U"About DragonWx" };
	positionSetupInfoButton = { buttonOk.pos.x - setupButtonPaddingX - 24, setupButtonsBottomY + 8 };

	titleBoxOutdoorPanel = { U"Outdoor", &fontSize40, areasBorderColor, positionOutdoorAreaStart, positionOutdoorAreaSize, 20, 45 };
	titleBoxIndoorPanel = { U"Indoor", &fontSize32, areasBorderColor, positionIndoorAreaStart, positionIndoorAreaSize, 20, 45 };
	titleBoxRainPanel = { U"Rainfall", &fontSize32, areasBorderColor, positionRainAreaStart, positionRainAreaSize, 20, 44 };
	titleBoxSensorPanel = { U"Sensor", &fontSize32, areasBorderColor, positionSensorAreaStart, positionSensorAreaSize, 20, 44 };
	titleBoxForecastPanel = { U"Forecast", &fontSize32, areasBorderColor, positionForecastAreaStart, positionForecastAreaSize, 20, 50 };

	olc::vi2d boxSize = { 1100, 640 };
	titleBoxAboutApp = { U"About DragonWx", &fontSize32, areasBorderColor, GetCenteredStartPosition(GetWindowSize(), boxSize), boxSize, 20, 50 };

	SetDrawTarget(renderableInfoScreen.Sprite());
	Clear(olc::BLANK);
	DrawBoxTitle(&titleBoxAboutApp, &renderableInfoScreen);
	renderableInfoScreen.Decal()->Update();

	UpdateAreaBordersSprite();

	dialogBoxNoValidConfig = { { "No valid config file could be loaded." }, { 500, 150 } };
	dialogBoxFailedExecCmd = { { "Could not launch RTL433 executable. Make", "sure the \"Exec Path\" in your Settings", "points to a RTL433.exe file." }, { 550, 180 } };
	dialogBoxInvalidValue = { { "Invalid value." }, { 300, 100 } };
	dialogBoxRestartRequired = { { "RTL433 must be restarted for SDR", "changes to take effect."}, { 450, 150 } };

	sensorAreaDividerX = positionSensorAreaStart.x + (positionSensorAreaSize.x / 2);
	posSensorAreaDividerStart = { sensorAreaDividerX, positionSensorAreaStart.y + (positionSensorAreaSize.y * 0.18f) };
	posSensorAreaDividerEnd = { sensorAreaDividerX, positionSensorAreaStart.y + positionSensorAreaSize.y - (positionSensorAreaSize.y * 0.18f) };

	//DrawCircle(windCircleCenterPoint, windCircleRadius, rainGaugeBorderColor);

	//DrawWindDirPreviousArc(positionWindowCenter, 120, olc::RED, 0xFF);
	//DrawWindDirPreviousArc(positionWindowCenter, 119, olc::RED, 0xFF);
	//DrawWindDirPreviousArc(positionWindowCenter, 118, olc::RED, 0xFF);
	//DrawCircle(positionWindowCenter, 117, olc::RED, 0x80);

	for (int x = 0; x < trendSampleSize; x++)
	{
		xSum += x;
		xSumSquare += (x * x);
	}
	sumsBottomEquation = (trendSampleSize * xSumSquare) - (xSum * xSum);

	decalTrendOutdoorTemp = renderableTrendArrowSteady.Decal();
	decalTrendOutdoorHumidity = renderableTrendArrowSteady.Decal();

	systemTimePrevious = std::time(nullptr);	// Set initial value for previous timestamp
	std::srand(std::time(nullptr));				// Seed based on current time

	if (webWxEnabled)
	{
		//strLocationURL = "https://api.open-meteo.com/v1/forecast?latitude=41.8907&longitude=-71.3923&current=temperature_2m,is_day,weather_code,surface_pressure&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_probability_max&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch&timezone=auto&forecast_days=3&timeformat=unixtime";
		strLocationURL = "https://api.open-meteo.com/v1/forecast?latitude=" + webWxLocationLat + "&longitude=" + webWxLocationLon;
		strLocationURL += "&current=temperature_2m,is_day,weather_code,surface_pressure&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_probability_max&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch&timezone=auto&forecast_days=3&timeformat=unixtime";
		webWxRequested = true;		// Trigger the initial Web Weather request
	}

	return true;
}

bool DragonWx::OnUserUpdate(float fElapsedTime)
{
	if (GetMouse(olc::Mouse::LEFT).bPressed)
	{
		positionMouseCursor = GetMousePos();
		if (dialogBoxInForeground)
			dialogBoxInForeground = false;
		else if (showNoValidConfigMsg)
			showNoValidConfigMsg = false;
		else if (failedExecRTL433)
			failedExecRTL433 = false;
		else if (restartMsgInForeground)
			restartMsgInForeground = false;
		else if (infoPageIsForeground)
		{
			SetDrawTarget(previousDrawTarget);
			infoPageIsForeground = false;
			settingsPageIsForeground = true;
		}
		else if (settingsPageIsForeground)
		{
			// If we are in Text Entry mode (when Input Box is active) AND the next left-click occurs OUTSIDE that active Input Box area, treat as user confirmation of value
			if (IsTextEntryEnabled() && !mouseWithinArea(setupActiveInputBoxPtr->value.pos, setupActiveInputBoxPtr->value.size))
				OnTextEntryComplete(TextEntryGetString());

			if (mouseWithinArea(buttonUnitsToggle.pos, buttonUnitsToggle.size))
				setupUseMetricUnits = !setupUseMetricUnits;
			else if (mouseWithinArea(buttonForecastOnOff.pos, buttonForecastOnOff.size))
			{
				//webWxEnabled = !webWxEnabled;
				setupWebWxEnabled = !setupWebWxEnabled;
				inputBoxLatitude.isEnabled = setupWebWxEnabled;
				inputBoxLongitude.isEnabled = setupWebWxEnabled;
			}
			else if (mouseClickedInputBox(&inputBoxOutdoorID))
				ActivateInputBox(&inputBoxOutdoorID);
			else if (mouseClickedInputBox(&inputBoxOutdoorCalTemp))
				ActivateInputBox(&inputBoxOutdoorCalTemp);
			else if (mouseClickedInputBox(&inputBoxOutdoorCalHumidity))
				ActivateInputBox(&inputBoxOutdoorCalHumidity);
			else if (mouseClickedInputBox(&inputBoxIndoorID))
				ActivateInputBox(&inputBoxIndoorID);
			else if (mouseClickedInputBox(&inputBoxIndoorCalTemp))
				ActivateInputBox(&inputBoxIndoorCalTemp);
			else if (mouseClickedInputBox(&inputBoxIndoorCalHumidity))
				ActivateInputBox(&inputBoxIndoorCalHumidity);
			else if (mouseClickedInputBox(&inputBoxLatitude) && inputBoxLatitude.isEnabled)
				ActivateInputBox(&inputBoxLatitude);
			else if (mouseClickedInputBox(&inputBoxLongitude) && inputBoxLongitude.isEnabled)
				ActivateInputBox(&inputBoxLongitude);
			else if (mouseClickedInputBox(&inputBoxSdrGain))
				ActivateInputBox(&inputBoxSdrGain);
			else if (mouseClickedInputBox(&inputBoxSdrParams))
				ActivateInputBox(&inputBoxSdrParams);
			else if (mouseClickedInputBox(&inputBoxSdrExecPath))
				ActivateInputBox(&inputBoxSdrExecPath);
			else if (mouseWithinArea(buttonStartStopRTL433.pos, buttonStartStopRTL433.size) && buttonStartStopRTL433.isEnabled)
			{
				if (rtl433_pipeIsRunning)
				{
					if (ClosePipeRTL433() && StopThreadRTL433())
						pendingRestartRTL433 = true;
				}
				else
					StartPipeRTL433();

			}
			else if (mouseWithinArea(buttonAboutApp.pos, buttonAboutApp.size))
			{
				infoPageIsForeground = true;
				settingsPageIsForeground = false;
			}
			else if (mouseWithinArea(buttonOk.pos, buttonOk.size))
			{
				currentUnits = setupUseMetricUnits;
				if (setupWebWxEnabled != webWxEnabled)
				{
					bool webWxPreviousState = webWxEnabled;
					if (setupWebWxEnabled && !isWhiteSpaceOnly(inputBoxLatitude.value.text) && !isWhiteSpaceOnly(inputBoxLongitude.value.text))
						webWxEnabled = true;
					else
						webWxEnabled = false;
					
					if (webWxEnabled != webWxPreviousState)
						UpdateAreaBordersSprite();
				}
				outdoorSensor.ID = inputBoxOutdoorID.value.text;
				indoorSensor.ID = inputBoxIndoorID.value.text;
				if (isWhiteSpaceOnly(inputBoxOutdoorCalTemp.value.text))
					outdoorSensor.temperature.offset.SetZero();
				else
					outdoorSensor.temperature.offset.SetValue(std::stod(inputBoxOutdoorCalTemp.value.text), currentUnits);
				if (isWhiteSpaceOnly(inputBoxOutdoorCalHumidity.value.text))
					outdoorSensor.humidity.offset = 0;
				else
					outdoorSensor.humidity.offset = std::stoi(inputBoxOutdoorCalHumidity.value.text);
				if (isWhiteSpaceOnly(inputBoxIndoorCalTemp.value.text))
					indoorSensor.temperature.offset.SetZero();
				else
					indoorSensor.temperature.offset.SetValue(std::stod(inputBoxIndoorCalTemp.value.text), currentUnits);
				if (isWhiteSpaceOnly(inputBoxIndoorCalHumidity.value.text))
					indoorSensor.humidity.offset = 0;
				else
					indoorSensor.humidity.offset = std::stoi(inputBoxIndoorCalHumidity.value.text);
				webWxLocationLat = inputBoxLatitude.value.text;
				webWxLocationLon = inputBoxLongitude.value.text;
				restartMsgInForeground = ((sdrGainSetting != inputBoxSdrGain.value.text) || (sdrExtraArguments != inputBoxSdrParams.value.text) || (pathToExec != inputBoxSdrExecPath.value.text));
				sdrGainSetting = inputBoxSdrGain.value.text;
				sdrExtraArguments = inputBoxSdrParams.value.text;
				pathToExec = inputBoxSdrExecPath.value.text;
				setupActiveInputBoxPtr = nullptr;
				settingsPageIsForeground = restartMsgInForeground;		// Keep the settings window on screen if the restart msg Dialog Box is being shown
			}
			else if (mouseWithinArea(buttonCancel.pos, buttonCancel.size))
			{
				setupActiveInputBoxPtr = nullptr;
				settingsPageIsForeground = false;
			}
		}
		else if (positionMouseCursor.y < screenPaddingOffsetY)
		{
			fullscreenToggle = !fullscreenToggle;
			return false;
		}
		else if (mouseWithinArea(positionSettingsIcon, renderableSettingsIcon.Sprite()->Size()))
		{
			setupUseMetricUnits = currentUnits;
			setupWebWxEnabled = webWxEnabled;
			inputBoxOutdoorID.value.text = outdoorSensor.ID;
			inputBoxIndoorID.value.text = indoorSensor.ID;
			inputBoxOutdoorCalTemp.value.text = std::format("{:.1f}", outdoorSensor.temperature.offset.GetValue(setupUseMetricUnits));
			inputBoxOutdoorCalHumidity.value.text = std::to_string(outdoorSensor.humidity.offset);
			inputBoxIndoorCalTemp.value.text = std::format("{:.1f}", indoorSensor.temperature.offset.GetValue(setupUseMetricUnits));
			inputBoxIndoorCalHumidity.value.text = std::to_string(indoorSensor.humidity.offset);
			inputBoxLatitude.isEnabled = setupWebWxEnabled;
			inputBoxLongitude.isEnabled = setupWebWxEnabled;
			inputBoxLatitude.value.text = webWxLocationLat;
			inputBoxLongitude.value.text = webWxLocationLon;
			inputBoxSdrGain.value.text = sdrGainSetting;
			inputBoxSdrParams.value.text = sdrExtraArguments;
			inputBoxSdrExecPath.value.text = pathToExec;
			settingsPageIsForeground = true;
		}
		else if (mouseWithinArea(positionCloseIcon, renderableCloseIcon.Sprite()->Size()))
		{
			SaveConfigFile();
			appExitRequested = true;
			return false;
			//previousDrawTarget = GetDrawTarget();
			//infoPageIsForeground = true;
		}
		else if (mouseWithinArea(positionFeelsLikeLabel, fontSize32.GetStringBounds(strFeelsLikeLabel).size))
			useFeelsLikeLabel = !useFeelsLikeLabel;
		else if (lightLevelLux.current != undefinedIntValue)
		{
			if (mouseWithinArea(positionLightLevelValue, fontSize24.GetStringBounds(strLightLevelValue).size) ||
				mouseWithinArea(positionLightLevelLabel, fontSize24.GetStringBounds(U"Light").size))
				useLuxValue = !useLuxValue;
		}
		else if ((textObjectWindDirName.width > 0) && mouseWithinArea(windDirectionTextStart, windDirectionTextSize))
		{
			useNumericWindDirection = !useNumericWindDirection;
			windDirectionTextStart = posWindDirectionText - textObjectWindDirName.posOffset;
			windDirectionTextSize = { textObjectWindDirName.width, textObjectWindDirName.height };
		}
	}
	else if (GetMouse(olc::Mouse::RIGHT).bPressed)
	{
		if (IsTextEntryEnabled())
		{
			TextEntryEnable(false);
			tempString = TextEntryGetString().substr(0, TextEntryGetCursor());		// If cursor is in mid-string, extract the first segment 
			if (PasteTextFromClipboard())
				TextEntryEnable(true, tempString + strClipboardContents + TextEntryGetString().substr(TextEntryGetCursor()));
			else
				TextEntryEnable(true, TextEntryGetString());
		}
	}

	if (!IsTextEntryEnabled())		// Only check for app keyboard-shortcuts if we are not in text entry mode (like while we are in setup menu entering info)
	{
		if (GetKey(olc::Key::Q).bHeld)
		{
			SaveConfigFile();
			appExitRequested = true;
			return false;
		}
		else if (GetKey(olc::Key::ESCAPE).bHeld)
		{
			if (settingsPageIsForeground)
			{
				settingsPageIsForeground = false;
				return true;
			}
		}

		if (GetKey(olc::Key::R).bPressed && !prevKeyPressed)
		{
			//PRINT_DEBUG("Sending Web Wx Request to wttr.in... %s\n", strLocationURL.c_str());
			//webWxRequested = true;
			dialogBoxInForeground = true;
			prevKeyPressed = true;
		}
		else if (!GetKey(olc::Key::R).bPressed)
			prevKeyPressed = false;
	}

	elapsedTimeCounter += fElapsedTime;
	animationElapsedTime += fElapsedTime;

	if (pendingRestartRTL433)
		restartPendingElapsed += fElapsedTime;

	positionAnimationSheetOffset = { wxIconAnimatedCounter * 64.0f, 0.0f };
	wxIconAnimatedCounter++;
	if (wxIconAnimatedCounter >= 30)
		wxIconAnimatedCounter = 0;

	if (elapsedTimeCounter >= 15.0f)
	{
		if (!useRealPipe)
		{
			if (dequeWindDirections.size() >= 3)
				dequeWindDirections.pop_back();
			float newRandomWindDirection = std::rand() % 360;
			dequeWindDirections.push_front(newRandomWindDirection);
			PRINT_DEBUG("%s Debug: New random wind value = %.0f\n", GetTimestamp().c_str(), newRandomWindDirection);
			if (dequeWindDirections.size() > 1)
			{
				windDirAnimatedPosition = dequeWindDirections.at(1);
				windDirDistanceLeft = dequeWindDirections.at(0) - windDirAnimatedPosition;
				if (std::abs(windDirDistanceLeft) >= 180)
				{
					if (windDirDistanceLeft > 0)
					{
						windAnimationDirPositive = false;
						windDirDistanceLeft = 360.0f - windDirDistanceLeft;
					}
					else
					{
						windAnimationDirPositive = true;
						windDirDistanceLeft += 360.0f;
					}
				}
				else
				{
					if (windDirDistanceLeft > 0)
						windAnimationDirPositive = true;
					else
					{
						windAnimationDirPositive = false;
						windDirDistanceLeft = std::abs(windDirDistanceLeft);
					}
				}
				//	windAnimationDirPositive = !(windDirectionDelta > 0);
				//else
				//	windAnimationDirPositive = (windDirectionDelta > 0);
				windDirHalfDistance = windDirDistanceLeft / 2.0f;
				windDirAnimatedSpeed = 0.01f;
				windAnimationIsMoving = true;
				PRINT_DEBUG("%s Debug: Moving from %.0f to %.0f\n", GetTimestamp().c_str(), windDirAnimatedPosition, newRandomWindDirection);
			}
		}

		if (outdoorSensor.recentlyUpdated)
		{
			if (outdoorSensor.packetCounter < 4)
				outdoorSensor.packetCounter++;

			outdoorSensor.recentlyUpdated = false;
		}
		else if (outdoorSensor.packetCounter > 0)
			outdoorSensor.packetCounter--;

		// Handle Indoor sensor telemetry timer/packet count
		if (indoorSensor.recentlyUpdated)
		{
			if (indoorSensor.packetCounter < 4)
				indoorSensor.packetCounter++;
			indoorSensor.recentlyUpdated = false;
		}
		else if (indoorSensor.packetCounter > 0)
			indoorSensor.packetCounter--;

		minuteTimeCounter++;
		if (minuteTimeCounter >= 4)			// Effectively manages a 1 minute timer out of the 15 second one
		{
			minuteTimeCounter = 0;
			if (outdoorSensor.temperature.current.IsDefined())
				decalTrendOutdoorTemp = UpdateTrendData(&dequeOutdoorTemps, outdoorSensor.temperature.current.imperial, decalTrendOutdoorTemp, "Outdoor Temp");
			if (outdoorSensor.humidity.current != undefinedIntValue)
				decalTrendOutdoorHumidity = UpdateTrendData(&dequeOutdoorHumidity, outdoorSensor.humidity.current, decalTrendOutdoorHumidity, "Outdoor Humidity");

			if (activeRainfallEvent)
			{
				if ((std::time(nullptr) - rainEventLastUpdateTime) > rainEventClearedInterval)
				{
					activeRainfallEvent = false;
					rainEventStopTime = rainEventLastUpdateTime;
					strRainEventStopTime = GetFormattedLocalTime("%I:%M %p", &rainEventStopTime);
					if (strRainEventStopTime.at(0) == '0')
						strRainEventStopTime.erase(0, 1);		// Strip off any leading zeros on the hours value
				}
			}
		}

		// If Web Weather forecast is enabled, check if we transitioned from day into night or vice-versa and update the assets accordingly
		if (webWxEnabled)
		{
			if (!webWxCurrentConditions.useDaytime && (systemTimeNow >= webWxDailyForecasts[0].sunrise) && (systemTimeNow < webWxDailyForecasts[0].sunset))
			{
				webWxCurrentConditions.useDaytime = true;
				LoadWebWxAssets(&webWxCurrentConditions, renderableWebConditionsImage.Decal());
			}
			else if (webWxCurrentConditions.useDaytime && (systemTimeNow >= webWxDailyForecasts[0].sunset) && (systemTimeNow < webWxDailyForecasts[1].sunrise))
			{
				webWxCurrentConditions.useDaytime = false;
				LoadWebWxAssets(&webWxCurrentConditions, renderableWebConditionsImage.Decal());
			}
		}

		/*
		secondsCounter30++;
		if (secondsCounter30 > 2)
		{
			secondsCounter30 = 0;
			if (dequeRainRateSamples.size() > 0)
				dequeRainRateSamples.pop_front();	// Remove oldest sample since we missed a packet
		}
		*/
		elapsedTimeCounter = 0;
		PRINT_DEBUG("%s Outdoor Sensor: Consecutive packets = %u\n", GetTimestamp().c_str(), outdoorSensor.packetCounter);
		PRINT_DEBUG("%s Indoor Sensor: Consecutive packets = %u\n", GetTimestamp().c_str(), indoorSensor.packetCounter);
	}

	if (restartPendingElapsed >= 5.0f)
	{
		StartPipeRTL433();
		pendingRestartRTL433 = false;
		restartPendingElapsed = 0.0f;
	}


	// Render blank background image
	DrawDecal({ 0.0f, 0.0f }, renderableBackgroundImage.Decal(), {0.333f, 0.333f});

	if (failedExecRTL433)
	{
		ShowCenteredDialogBox(&dialogBoxFailedExecCmd);
		return true;
	}

	if (settingsPageIsForeground)
	{
		DrawDecal({ 0.0f, 0.0f }, renderableSetupScreen.Decal());

		std::u32string strSettingsLabel32 = U"Settings";
		olc::vf2d positionSetupLabel = GetWindowSize() * olc::vf2d(0.5f, 0.03f);
		float setupLineEdgeLeft = GetWindowSize().x * 0.15f;
		float setupLineEdgeRight = GetWindowSize().x - setupLineEdgeLeft;
		float setupLinePadding = 30.0f;
		float setupLabelCenterY = positionSetupLabel.y + (fontSize32.GetStringBounds(strSettingsLabel32).size.y / 2.0f);
		RenderStringCentered("Settings", &fontSize32, olc::WHITE, &textObjSettingsLabel, positionSetupLabel);
		DrawLineDecal({ setupLineEdgeLeft, setupLabelCenterY }, { textObjSettingsLabel.posOffset.x - setupLinePadding, setupLabelCenterY }, areasBorderColor);
		DrawLineDecal({ textObjSettingsLabel.posOffset.x + fontSize32.GetStringBounds(strSettingsLabel32).size.x + setupLinePadding, setupLabelCenterY }, { setupLineEdgeRight, setupLabelCenterY }, areasBorderColor);

		//RenderString32(titleBoxSetup.text32, titleBoxSetup.fontPtr, olc::GREY, &titleBoxSetup.textObj, titleBoxSetup.posTitle);
		RenderString32(titleBoxSetupOutdoor.text32, titleBoxSetupOutdoor.fontPtr, olc::GREY, &titleBoxSetupOutdoor.textObj, titleBoxSetupOutdoor.posTitle);
		RenderString32(titleBoxSetupIndoor.text32, titleBoxSetupIndoor.fontPtr, olc::GREY, &titleBoxSetupIndoor.textObj, titleBoxSetupIndoor.posTitle);

		// First render the Outdoor Sensor labels and text boxes
		RenderString32(inputBoxOutdoorID.label.text32, &fontSize30, olc::GREY, &inputBoxOutdoorID.label.textObj, inputBoxOutdoorID.label.pos);
		RenderInputBox(&inputBoxOutdoorID, &fontSize30, olc::BLACK);
		RenderString32(U"Calibration Offset  ( + / - )", &fontSize28, olc::GREY, &textObjSetupOutdoorCalLabel, positionSetupOutdoorCalLabel);
		RenderString32(inputBoxOutdoorCalTemp.label.text32, &fontSize26, colorLabelText, &inputBoxOutdoorCalTemp.label.textObj, inputBoxOutdoorCalTemp.label.pos);
		RenderInputBox(&inputBoxOutdoorCalTemp, &fontSize30, olc::BLACK);
		RenderString32(inputBoxOutdoorCalHumidity.label.text32, &fontSize26, colorLabelText, &inputBoxOutdoorCalHumidity.label.textObj, inputBoxOutdoorCalHumidity.label.pos);
		RenderInputBox(&inputBoxOutdoorCalHumidity, &fontSize30, olc::BLACK);

		// Next render the Indoor Sensor labels and text boxes
		RenderString32(inputBoxIndoorID.label.text32, &fontSize30, olc::GREY, &inputBoxIndoorID.label.textObj, inputBoxIndoorID.label.pos);
		RenderInputBox(&inputBoxIndoorID, &fontSize30, olc::BLACK);
		RenderString32(U"Calibration Offset  ( + / - )", &fontSize28, olc::GREY, &textObjSetupIndoorCalLabel, positionSetupIndoorCalLabel);
		RenderString32(inputBoxIndoorCalTemp.label.text32, &fontSize26, colorLabelText, &inputBoxIndoorCalTemp.label.textObj, inputBoxIndoorCalTemp.label.pos);
		RenderInputBox(&inputBoxIndoorCalTemp, &fontSize30, olc::BLACK);
		RenderString32(inputBoxIndoorCalHumidity.label.text32, &fontSize26, colorLabelText, &inputBoxIndoorCalHumidity.label.textObj, inputBoxIndoorCalHumidity.label.pos);
		RenderInputBox(&inputBoxIndoorCalHumidity, &fontSize30, olc::BLACK);

		RenderString32(inputBoxSdrParams.label.text32, &fontSize30, olc::GREY, &inputBoxSdrParams.label.textObj, inputBoxSdrParams.label.pos);
		RenderInputBox(&inputBoxSdrParams, &fontSize30, olc::BLACK);
		RenderString32(inputBoxSdrGain.label.text32, &fontSize30, olc::GREY, &inputBoxSdrGain.label.textObj, inputBoxSdrGain.label.pos);
		RenderInputBox(&inputBoxSdrGain, &fontSize30, olc::BLACK);
		RenderString32(inputBoxSdrExecPath.label.text32, &fontSize30, olc::GREY, &inputBoxSdrExecPath.label.textObj, inputBoxSdrExecPath.label.pos);
		RenderInputBox(&inputBoxSdrExecPath, &fontSize30, olc::BLACK);

		// Now do the Units selection status and toggle button
		olc::vi2d positionSetupUnits = { setupWindowRightColumnX, setupUnitsLabelY };
		positionSetupUnits = RenderStringSegment("Units: ", &fontSize30, olc::GREY, &textObjectSetupUnitsLabel, positionSetupUnits, 15);
		RenderString(setupUseMetricUnits ? "Metric" : "Imperial", &fontSize30, olc::WHITE, &textObjectSetupUnitsValue, positionSetupUnits);
		RenderButton(&buttonUnitsToggle, "Set " + std::string(setupUseMetricUnits ? "Imperial" : "Metric"));

		// Next render the Web Forecast enable/disable button and lat/lon input boxes (when enabled)
		olc::vi2d positionSetupWebWx = { setupWindowRightColumnX, setupWebWxLabelY };
		positionSetupWebWx = RenderStringSegment("Web Forecasts: ", &fontSize30, olc::GREY, &textObjectSetupWebWxLabel, positionSetupWebWx, 15);
		RenderString(setupWebWxEnabled ? "Enabled" : "Disabled", &fontSize30, olc::WHITE, &textObjectSetupWebWxValue, positionSetupWebWx);
		RenderButton(&buttonForecastOnOff, std::string(setupWebWxEnabled ? "Disable" : "Enable"));
		RenderString32(inputBoxLatitude.label.text32, &fontSize26, colorLabelText, &inputBoxLatitude.label.textObj, inputBoxLatitude.label.pos);
		RenderString32(inputBoxLongitude.label.text32, &fontSize26, colorLabelText, &inputBoxLongitude.label.textObj, inputBoxLongitude.label.pos);
		RenderInputBox(&inputBoxLatitude, &fontSize30, olc::BLACK);
		RenderInputBox(&inputBoxLongitude, &fontSize30, olc::BLACK);

		// Show the Reset Statistics button and Ok/Cancel buttons
		RenderButton(&buttonResetStats);
		RenderButton(&buttonAboutApp);
		RenderButton(&buttonOk);
		RenderButton(&buttonCancel);

		if (rtl433_threadRunning)
		{
			buttonStartStopRTL433.text32 = U"Restart RTL433";
			buttonStartStopRTL433.isEnabled = true;
		}
		else if (pendingRestartRTL433)
		{
			buttonStartStopRTL433.text32 = U"Restarting...";
			buttonStartStopRTL433.isEnabled = false;
		}
		else
		{
			buttonStartStopRTL433.text32 = U"Start RTL433";
			buttonStartStopRTL433.isEnabled = true;
		}
		RenderButtonWithEnabler(&buttonStartStopRTL433);
		
		// If one of the input boxes is in Text Entry mode, handle the state of blinking of the cursor
		if (IsTextEntryEnabled())
		{
			cursorBlinkElapsedTime += fElapsedTime;
			if (cursorBlinkElapsedTime >= 0.25f)
			{
				textCursorBlinkState = !textCursorBlinkState;
				cursorBlinkElapsedTime = 0.0f;
			}
		}

		if (dialogBoxInForeground)
			ShowCenteredDialogBox(&dialogBoxInvalidValue);
		else if (restartMsgInForeground)
			ShowCenteredDialogBox(&dialogBoxRestartRequired);

		return true;
	}
	else if (infoPageIsForeground)
	{
		DrawDecal({ 0, 0 }, renderableInfoScreen.Decal());
		RenderString32(titleBoxAboutApp.text32, titleBoxAboutApp.fontPtr, olc::GREY, &titleBoxAboutApp.textObj, titleBoxAboutApp.posTitle);
		//RenderStringCentered("About DragonWx", &fontSize40, olc::WHITE, &textObjectInfoPageTitle, olc::vi2d(positionWindowCenter.x, 70));

		DrawDecal(titleBoxAboutApp.posStart + olc::vi2d(40, 40), renderableDragonLogo.Decal());

		RenderString32(strTitleAuthorText[0], &fontSize32, olc::WHITE, &textObjectAuthorText[0], titleBoxAboutApp.posStart + olc::vi2d(210, 80));
		RenderString32(strTitleAuthorText[1], &fontSize32, olc::WHITE, &textObjectAuthorText[1], titleBoxAboutApp.posStart + olc::vi2d(210, 120));

		RenderString32(strIntroText[0], &fontSize22, olc::WHITE, &textObjectIntroText[0], { titleBoxAboutApp.posStart.x + 60, 260 });
		RenderString32(strIntroText[1], &fontSize22, olc::WHITE, &textObjectIntroText[1], { titleBoxAboutApp.posStart.x + 60, 285 });
		RenderString32(strIntroText[2], &fontSize22, olc::WHITE, &textObjectIntroText[2], { titleBoxAboutApp.posStart.x + 60, 310 });

		RenderStringCentered("Credits / Attributions", &fontSize30, olc::WHITE, &textObjectAttribsTitle, olc::vi2d(positionWindowCenter.x, 375));


		RenderString32(strThanksText[0], &fontSize22, olc::WHITE, &textObjectThanksText[0], { titleBoxAboutApp.posStart.x + 60, 450 });
		RenderString32(strThanksText[1], &fontSize22, olc::WHITE, &textObjectThanksText[1], { titleBoxAboutApp.posStart.x + 60, 475 });

		olc::vi2d positionAttribTextCurrent = { 180, 525 };
		int attribLineOffset = 50, attribURLoffset = 25;
		olc::vi2d positionAttribOffsetX = { 500, 0 };
		olc::vi2d positionAttribURLOffset = { 0, 25 };
		RenderString32(strAttribText[0], &fontSize18, olc::WHITE, &textObjectAttribText[0], positionAttribTextCurrent);
		RenderString32(strAttribURL[0], &fontSize18, olc::Pixel(0x00, 0x70, 0xFF), &textObjectAttribURL[0], positionAttribTextCurrent + olc::vi2d(0, 20));
		RenderString32(strAttribText[2], &fontSize18, olc::WHITE, &textObjectAttribText[2], positionAttribTextCurrent + olc::vi2d(520, 0));
		RenderString32(strAttribURL[2], &fontSize18, olc::Pixel(0x00, 0x70, 0xFF), &textObjectAttribURL[2], positionAttribTextCurrent + olc::vi2d(520, 20));
		positionAttribTextCurrent.y += attribLineOffset;
		RenderString32(strAttribText[1], &fontSize18, olc::WHITE, &textObjectAttribText[1], positionAttribTextCurrent);
		RenderString32(strAttribURL[1], &fontSize18, olc::Pixel(0x00, 0x70, 0xFF), &textObjectAttribURL[1], positionAttribTextCurrent + olc::vi2d(0, 20));
		RenderString32(strAttribText[3], &fontSize18, olc::WHITE, &textObjectAttribText[3], positionAttribTextCurrent + olc::vi2d(520, 0));
		RenderString32(strAttribURL[3], &fontSize18, olc::Pixel(0x00, 0x70, 0xFF), &textObjectAttribURL[3], positionAttribTextCurrent + olc::vi2d(520, 20));
		return true;
	}

	//DrawDecal({ 0, 0 }, decalAreaBorders);
	DrawDecal({ 0, 0 }, renderableAreaBorders.Decal());
	RenderString32(titleBoxOutdoorPanel.text32, titleBoxOutdoorPanel.fontPtr, olc::GREY, &titleBoxOutdoorPanel.textObj, titleBoxOutdoorPanel.posTitle);
	RenderString32(titleBoxIndoorPanel.text32, titleBoxIndoorPanel.fontPtr, olc::GREY, &titleBoxIndoorPanel.textObj, titleBoxIndoorPanel.posTitle);
	RenderString32(titleBoxSensorPanel.text32, titleBoxSensorPanel.fontPtr, olc::GREY, &titleBoxSensorPanel.textObj, titleBoxSensorPanel.posTitle);
	RenderString32(titleBoxRainPanel.text32, titleBoxRainPanel.fontPtr, olc::GREY, &titleBoxRainPanel.textObj, titleBoxRainPanel.posTitle);
	if (webWxEnabled)
		RenderString32(titleBoxForecastPanel.text32, titleBoxForecastPanel.fontPtr, olc::GREY, &titleBoxForecastPanel.textObj, titleBoxForecastPanel.posTitle);

	//SetDecalMode(olc::DecalMode::WIREFRAME);
	//DrawPolygonDecal(nullptr, { {10,10},{100,10},{100,50},{10,50} },
	//	{ {0,0},{0,0},{0,0},{0,0} }, olc::GREEN);
	//SetDecalMode(olc::DecalMode::NORMAL);

	//DrawDecal({ 900, 300 }, decalTest);


	// Check the current system time and if it has changed since last check, render/display it
	systemTimeNow = std::time(nullptr);
	if (systemTimeNow != systemTimePrevious)
	{
		std::tm systemTimeLocalNow = *std::localtime(&systemTimeNow);
		std::tm systemTimeLocalPrevious = *std::localtime(&systemTimePrevious);

		//ConvertTimeToLocal(&systemTimeLocalNow, systemTimeNow);
		//ConvertTimeToLocal(&systemTimeLocalPrevious, systemTimePrevious);

		// Check for Midnight rollover to reset statistics, etc
		if ((systemTimeLocalNow.tm_hour == 0) && (systemTimeLocalPrevious.tm_hour != 0))
		{
			MidnightDailyReset();
			PRINT_DEBUG("%s Time: Midnight rollover\n", GetTimestamp().c_str());
		}

		// Check if it's time to update Web Wx forecast (if enabled), and if so, set web weather request flag (6 hour intervals)
		if (webWxEnabled && (systemTimeLocalNow.tm_min == 0) && (systemTimeLocalNow.tm_sec == 0) && ((systemTimeLocalNow.tm_hour % 6) == 0))
			webWxRequested = true;

		// Now format the date/time and display it on the screen
		strFullyFormattedDate = GetFormattedLocalTime("%A, %B %d %Y", &systemTimeNow);
		strFullyFormattedTime = GetFormattedLocalTime("%I:%M %p", &systemTimeNow);
		if (strFullyFormattedTime.at(0) == '0')
			strFullyFormattedTime.erase(0, 1);		// Strip off any leading zeros on the hours value
		/*
		int dateResult = std::strftime(strDateWeekMonthDay, sizeof(strDateWeekMonthDay), "%A, %B %d ", &systemTimeLocalNow);
		int timeResult = std::strftime(strFormattedTime, sizeof(strFormattedTime), "%I:%M %p", &systemTimeLocalNow);
		if (dateResult && timeResult)
		{
			strFullyFormattedDate = strDateWeekMonthDay + std::to_string(systemTimeLocalNow.tm_year + 1900);
			strFullyFormattedTime = strFormattedTime;
			if (strFullyFormattedTime.at(0) == '0')
				strFullyFormattedTime.erase(0, 1);		// Strip off any leading zeros on the hours value
		}
		*/
		systemTimePrevious = systemTimeNow;
	}
	RenderStringCentered(strFullyFormattedDate, &fontSize40, olc::WHITE, &textObjectDateText, positionSystemDate);
	RenderStringCentered(strFullyFormattedTime, &fontSize72, olc::WHITE, &textObjectTimeText, positionSystemTime);
	
	// Display the Thermometer and Water Drop icons in Outdoor area
	if (currentUnits == metricUnits)
		DrawDecal({ positionOutdoorAreaStart.x + 15, positionOutdoorTempValue.y + 2 }, renderableThermometerIconC.Decal());
	else
		DrawDecal({ positionOutdoorAreaStart.x + 15, positionOutdoorTempValue.y + 2 }, renderableThermometerIconF.Decal());
	DrawDecal({ positionOutdoorAreaStart.x + 18, positionOutdoorHumidityValue.y + 5 }, renderableWaterDropIcon.Decal());

	// Display the temperature
	if (outdoorSensor.temperature.current.IsDefined())
	{
		//RenderStringRightJustified(std::format("{:.1f}", outdoorTempValueF.current), &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF);
		tempFloat = outdoorSensor.temperature.current.GetValue(currentUnits);
		tempString = std::to_string(int(tempFloat));
		RenderStringRightJustified(tempString, &fontSize96, olc::WHITE, &textObjectTempValue, positionOutdoorTempValue);
		tempString32 = ConvertedString32(std::format("{:.1f}", tempFloat - int(tempFloat)));
		tempString32.erase(tempString32.begin());
		positionTemp = { 9, fontSize96.GetStringBounds(ConvertedString32(tempString)).size.y - fontSize40.GetStringBounds(tempString32).size.y };
		RenderString32(tempString32, &fontSize40, olc::WHITE, &textObjectTempDecimalValue, positionOutdoorTempValue + positionTemp);
	}
	else
		RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &textObjectTempValue, positionOutdoorTempValue + positionDashCorrection96 - olc::vf2d(15, 0));
	RenderString32(degreeUnits.Label(currentUnits), &fontSize32, colorLabelText, &textObjectLabelTempUnits, positionOutdoorTempValue + olc::vf2d(9, 0));

	// Display the Outdoor Temperature's current 24-hour low/high
	RenderHighOrLowValue("Low", outdoorSensor.temperature.low.GetValue(currentUnits), &textObjectOutdoorTempLowValue, positionOutdoorTempLowValue, false);
	RenderHighOrLowValue("High", outdoorSensor.temperature.high.GetValue(currentUnits), &textObjectOutdoorTempHighValue, positionOutdoorTempHighValue, false);

	// Display the current Outdoor temperature trend arrow
	if (outdoorSensor.temperature.current.IsDefined())
		//DrawDecal(positionOutdoorTempValueF + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));
		DrawDecal(positionOutdoorTempValue + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));

	// Display the Outdoor Humidity
	if (outdoorSensor.humidity.current != undefinedIntValue)
		RenderStringRightJustified(std::format("{:d}", outdoorSensor.humidity.current), &fontSize96, olc::WHITE, &textObjectHumidityValue, positionOutdoorHumidityValue);
	else
		RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &textObjectHumidityValue, positionOutdoorHumidityValue + positionDashCorrection96 - olc::vf2d(15, 0));
	RenderString("%", &fontSize32, colorLabelText, &textObjectHumidityUnits, positionOutdoorHumidityValue + olc::vf2d(9, 0));

	// Display the Outdoor Humidity's 24-hour low/high
	RenderHighOrLowValue("Low", outdoorSensor.humidity.low, &textObjectHumidityLowValue, positionHumidityLowValue, false);
	RenderHighOrLowValue("High", outdoorSensor.humidity.high, &textObjectHumidityHighValue, positionHumidityHighValue, false);

	// Display the Outdoor Humidity's current trend arrow
	if (outdoorSensor.humidity.current != undefinedFloatValue)
		DrawDecal(positionOutdoorHumidityValue + positionOutdoorTrendOffset, decalTrendOutdoorHumidity, { 0.33f, 0.33f }, olc::Pixel(0, 77, 230));

	// Display the calculated dewpoint
	RenderString("Dewpoint", &fontSize32, olc::GREY, &textObjectLabelDewpoint, positionDewPointLabel);
	if (dewpointValue.IsDefined())
		positionTemp = RenderStringSegment(std::format("{:.1f}", dewpointValue.GetValue(currentUnits)), &fontSize32, olc::WHITE, &textObjectDewpointValue, positionDewPointValue, spacerFontSize24);
	else
		positionTemp = RenderStringSegment("-  -", &fontSize32, olc::GREY, &textObjectDewpointValue, positionDewPointValue + positionDashCorrection32, spacerFontSize24 * 2);
	RenderString32(degreeUnits.Label(currentUnits), &fontSize24, colorLabelText, &textObjectDewPointUnits, positionTemp);

	// Display the "Feels Like" temperature (or the name of the actual method being used if toggled on by user)
	float feelsLikeTemp;
	strFeelsLikeLabel = U"Feels Like";
	if ((outdoorSensor.temperature.current.imperial < 50.0f) && (windSpeedValue.current.mph >= 3.0f))
	{
		if (!useFeelsLikeLabel)
			strFeelsLikeLabel = U"Wind Chill";
		feelsLikeTemp = calculatedWindChill;
	}
	else if ((outdoorSensor.temperature.current.imperial >= 80.0f) && (outdoorSensor.humidity.current >= 40))
	{
		if (!useFeelsLikeLabel)
			strFeelsLikeLabel = U"Heat Index";
		feelsLikeTemp = calculatedHeatIndex.GetValue(currentUnits);
	}
	else
	{
		if ((outdoorSensor.temperature.current.imperial >= 50.0f) && (outdoorSensor.temperature.current.imperial < 80.0f))
		{
			if (!useFeelsLikeLabel)
				strFeelsLikeLabel = U"Apparent";
			feelsLikeTemp = calculatedApparentTemp.GetValue(currentUnits);
		}
		else
		{
			if (!useFeelsLikeLabel)
				strFeelsLikeLabel = U"Actual";
			feelsLikeTemp = outdoorSensor.temperature.current.GetValue(currentUnits);
		}
	}
	RenderString32(strFeelsLikeLabel, &fontSize32, olc::GREY, &textObjectFeelsLikeLabel, positionFeelsLikeLabel);
	if (feelsLikeTemp != undefinedFloatValue)
		positionTemp = RenderStringSegment(std::format("{:.1f}", feelsLikeTemp), &fontSize32, olc::WHITE, &textObjectFeelsLikeValue, positionFeelsLikeValue, spacerFontSize24);
	else
		positionTemp = RenderStringSegment("-  -", &fontSize32, olc::GREY, &textObjectFeelsLikeValue, positionFeelsLikeValue + positionDashCorrection32, spacerFontSize24 * 2);
	RenderString32(degreeUnits.Label(currentUnits), &fontSize24, colorLabelText, &textObjectFeelsLikeUnits, positionTemp);

	// Display Light Intensity information and UV Index (if applicable to the weather sensor being used)
	if ((uvIndex.current != undefinedIntValue) || (lightLevelLux.current != undefinedIntValue))
	{
		// First draw a dividing line between the above temperature-related info and the light info to come below
		olc::vf2d positionLeftDividerStart = { positionLeftSideDivider.x + (positionOutdoorAreaSize.x * 0.00f), positionLeftSideDivider.y };
		olc::vf2d positionLeftDividerEnd = { positionLeftSideDivider.x + positionOutdoorAreaSize.x - (positionOutdoorAreaSize.x * 0.00f), positionLeftSideDivider.y };
		DrawLineDecal(positionLeftDividerStart, positionLeftDividerEnd, areasBorderColor);

		positionLightInfoNext = positionLeftSideDivider + olc::vf2d(0, 20);		// Set initial position for first light-measurement-related info

		// First show the Light Intensity (if applicable)
		if (lightLevelLux.current != undefinedIntValue)
		{
			positionLightLevelLabel = positionLightInfoNext;
			positionLightLevelValue = positionLightLevelLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);
			RenderString32(U"Light", &fontSize24, olc::GREY, &textObjectLightLevelLabel, positionLightLevelLabel);
			if (!useLuxValue)
			{
				if (lightLevelLux.current < 10)
					strLightLevelValue = U"Dark / Night";
				else if ((lightLevelLux.current >= 10) && (lightLevelLux.current < 500))
					strLightLevelValue = U"Twilight";
				else if ((lightLevelLux.current >= 500) && (lightLevelLux.current < 5300))
					strLightLevelValue = U"Low Light";
				else if ((lightLevelLux.current >= 5300) && (lightLevelLux.current < 20000))
					strLightLevelValue = U"Overcast / Shade";
				else if ((lightLevelLux.current >= 20000) && (lightLevelLux.current < 45000))
					strLightLevelValue = U"Daylight";
				else if (lightLevelLux.current >= 45000)
					strLightLevelValue = U"Direct Sunlight";
				else
					strLightLevelValue = U"";
				RenderString32(strLightLevelValue, &fontSize24, olc::WHITE, &textObjectLightLevelValue, positionLightLevelValue);
			}
			else
			{
				strLightLevelValue = ConvertedString32(std::to_string(lightLevelLux.current));
				RenderString32(strLightLevelValue, &fontSize24, olc::WHITE, &textObjectLightLevelValue, positionLightLevelValue);
				RenderString32(U"lux", &fontSize18, colorLabelText, &textObjectLightLeveilUnits, positionLightLevelValue + olc::vf2d(fontSize24.GetStringBounds(strLightLevelValue + U"0").size.x, 0));
			}
			positionLightInfoNext += olc::vf2d(0, 40);
		}

		// Then display UV Index information and graph (if applicable)
		if (uvIndex.current != undefinedIntValue)
		{
			std::u32string strUVindex = ConvertedString32(std::to_string(uvIndex.current));
			positionUVindexValue = positionLightInfoNext + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);
			DrawUVindexGraph(positionUVindexValue, strUVindex);
			RenderString32(U"UV Index", &fontSize24, olc::GREY, &textObjectUVindexLabel, positionLightInfoNext);
			RenderString32(strUVindex, &fontSize24, olc::WHITE, &textObjectUVindexValue, positionUVindexValue);
		}
	}

	olc::Pixel windArrowColors[3] = { { 170, 0, 0 }, { 93, 147, 201 }, { 23, 77, 131 } };
	olc::Pixel tempPixel = { 3, 57, 111 };
	//olc::Pixel tempPixel = { 100, 100, 100 };
	olc::Pixel tempPixelAdjustment1 = { 90, 90, 90 };
	olc::Pixel tempPixelAdjustment2 = { 20, 20, 20 };
	// Display the Wind Direction compass and render the current and previous directional arrows in the right places

	if (dequeWindDirections.size() > 1)
		for (int i = dequeWindDirections.size() - 1; i >= 1 ; i--)
			DrawWindDirectionArrow(dequeWindDirections.at(i), windArrowColors[i]);

	DrawWindDirectionArrow(dequeWindDirections.at(0), windArrowColors[0]);
	std::string strWindDirectionText;
	if (useNumericWindDirection)
		strWindDirectionText = std::format("{:.0f}", dequeWindDirections.at(0)) + "\u00B0";
	else
		strWindDirectionText = GetWindDirectionName(dequeWindDirections.at(0));
	RenderStringCentered(strWindDirectionText, &fontSize40, olc::GREY, &textObjectWindDirName, posWindDirectionText);
	windDirectionTextStart = posWindDirectionText - textObjectWindDirName.posOffset;
	windDirectionTextSize = { textObjectWindDirName.width, textObjectWindDirName.height };

	/*
	if (dequeWindDirections.size() > 1)
	{
		windDirAnimatedPosition = update_weather_vane(dequeWindDirections.at(1), dequeWindDirections.at(0), windDirCurVelocity, 10.0f, 10.0f, fElapsedTime);
		//if (windDirCurVelocity > 0.0f)
		//	printf("Vel = %.3f\n", windDirCurVelocity);
		DrawWindDirectionArrow(windDirAnimatedPosition, windArrowColors[0]);
		RenderStringCentered(GetWindDirectionName(windDirAnimatedPosition), &fontSize40, olc::GREY, &textObjectWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	}
	*/

	/*
	if (windAnimationIsMoving)
	{
		if (animationElapsedTime >= (1.0f / 60.0f))
		{
			//float tempNum = (windDirHalfDistance * 2.0f) - windDirDistanceLeft;

			//if (windAnimationIsMoving)
			//	PRINT_DEBUG("Distance left = %.2f, Current speed = %.2f (Halfway = %.2f)\n", windDirDistanceLeft, windDirAnimatedSpeed, windDirHalfDistance);

			//windDirAnimatedSpeed = std::sinf((std::numbers::pi * tempNum) / (windDirHalfDistance * 2.0f));
			//PRINT_DEBUG("Speed calc = %.3f (%.3f)\n", windDirAnimatedSpeed, (std::numbers::pi* tempNum) / (windDirHalfDistance * 2.0f));

			windDirAnimatedSpeed = 0.5f;

			if (windAnimationDirPositive)
				windDirAnimatedPosition += windDirAnimatedSpeed;
			else
				windDirAnimatedPosition -= windDirAnimatedSpeed;

			// After incrementing or decrementing or animated position, check for and handle any rollover passed 360 degrees
			if (windDirAnimatedPosition >= 360.0f)
				windDirAnimatedPosition = 0.0f;
			else if (windDirAnimatedPosition < 0.0f)
				windDirAnimatedPosition = 359.0f;

			windDirDistanceLeft -= windDirAnimatedSpeed;

			if (windDirDistanceLeft < 1.0f)
				windAnimationIsMoving = false;
			//else if ((windDirDistanceLeft <= windDirHalfDistance) && (windDirAnimatedSpeed >= 0.02f))
			//	windDirAnimatedSpeed -= 0.01f;
			//else if (windDirAnimatedSpeed < 1.0f)
			//	windDirAnimatedSpeed += 0.01f;


			animationElapsedTime -= (1.0f / 60.0f);
		}

		DrawWindDirectionArrow(windDirAnimatedPosition, windArrowColors[0]);
		RenderStringCentered(GetWindDirectionName(windDirAnimatedPosition), &fontSize40, olc::GREY, &textObjectWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	}
	else
	{
		DrawWindDirectionArrow(dequeWindDirections.at(0), windArrowColors[0]);
		RenderStringCentered(GetWindDirectionName(dequeWindDirections.at(0)), &fontSize40, olc::GREY, &textObjectWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	}
	*/






	//olc::vf2d windArrowOffset = olc::vf2d(windCircleRadius * std::sin(degreesToRadians(arrowCircleCounter)), (windCircleRadius * std::cos(degreesToRadians(arrowCircleCounter))) * -1.0f);
	//DrawRotatedDecal(windCircleCenterPoint + windArrowOffset, decalWindDir, degreesToRadians(arrowCircleCounter), centerPointWindDir, { 1.0f, 1.0f }, windArrowColors[0]);
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(33, 77, 125));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(192, 192, 192, 120));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(70, 104, 140));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(47, 88, 131));

	// Display Wind Speed and speed units label
	if (windSpeedValue.current.GetValue(currentUnits) != undefinedFloatValue)
		RenderStringCentered(std::format("{:.0f}", windSpeedValue.current.GetValue(currentUnits)), &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 30));
	else
		RenderStringCentered("- -", &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 2));
	RenderStringCentered(windSpeedUnits.Label(currentUnits), &fontSize32, colorLabelText, &textObjectWindSpeedUnits, windCircleCenterPoint + olc::vf2d(0, 70));

	// Display Wind Speed current average and high
	positionTemp = RenderStringSegment("Avg", &fontSize24, colorLabelText, &textObjectLabelWindSpeedAvg, positionWindSpeedAvgLabel, 9);
	if (windSpeedValue.average.IsDefined())
		RenderString(std::format("{:.0f}", windSpeedValue.average.GetValue(currentUnits)), &fontSize24, olc::WHITE, &textObjectWindSpeedAvg, positionTemp);
	else
		RenderString32(U"-  -", &fontSize24, olc::WHITE, &textObjectWindSpeedAvg, positionTemp + positionDashCorrection24);
	positionTemp = RenderStringSegment("Peak", &fontSize24, colorLabelText, &textObjectWindSpeedPeakLabel, positionWindSpeedPeakLabel, 9);
	if (windSpeedValue.peak.IsDefined())
		RenderString(std::format("{:.0f}", windSpeedValue.peak.GetValue(currentUnits)), &fontSize24, olc::WHITE, &textObjectWindSpeedPeakValue, positionTemp);
	else
		RenderString32(U"-  -", &fontSize24, olc::WHITE, &textObjectWindSpeedPeakValue, positionTemp + positionDashCorrection24);

	// Display Rainfall panel and it's elements
	DrawDecal(positionRainAreaStart + olc::vf2d(25, 40), renderableRainfallIcon.Decal());	// Display the rainfall area icon

	// First we render the raingauge gfx
	if (currentUnits)
		rainGaugeFilledPercentage = rainfallTotalToday.millimeters / rainGaugeCapacity.millimeters;
	else
		rainGaugeFilledPercentage = rainfallTotalToday.inches / rainGaugeCapacity.inches;
	rainGaugeFilledPercentage = rainfallTotalToday.GetValue(currentUnits) / rainGaugeCapacity.GetValue(currentUnits);

	rainGaugeFilledHeight = (rainGaugeTotalHeight * rainGaugeFilledPercentage);
	rainGaugeFilledStart = { 0, rainGaugeTotalHeight - rainGaugeFilledHeight };
	rainGaugeFilledSize = { rainGaugeTotalWidth, rainGaugeFilledHeight };
	olc::vi2d waterSpriteSourceStart = { 0, renderableRainGaugeWater.Sprite()->Size().y - rainGaugeFilledHeight};

	//DrawPartialSprite(positionRainGauge + rainGaugeFilledStart, spriteRainGaugeWater, waterSpriteSourceStart, rainGaugeFilledSize);
	DrawPartialDecal(positionRainGauge + rainGaugeFilledStart, rainGaugeFilledSize, renderableRainGaugeWater.Decal(), waterSpriteSourceStart, rainGaugeFilledSize);
	DrawDecal({ 0, 0 }, renderableRainGaugeOutline.Decal());
	RenderRainGaugeText();

	if (currentUnits == metricUnits)
		tempString32 = ConvertedString32(std::format("{:04.1f}", rainfallTotalToday.millimeters));
	else
		tempString32 = ConvertedString32(std::format("{:.2f}", rainfallTotalToday.inches));
	RenderString32(U"Today", &fontSize32, olc::GREY, &textObjectRainfallTodayLabel, positionRainfallTodayLabel);
	RenderString32(tempString32, &fontSize32, olc::WHITE, &textObjectRainfallValue, positionRainfallTodayValue);
	olc::vi2d unitsLabelOffset = { fontSize32.GetStringBounds(tempString32).size.x + 15, fontSize32.GetStringBounds(tempString32).size.y - fontSize30.GetStringBounds(rainfallUnits.amount.Label(currentUnits)).size.y };
	RenderString32(rainfallUnits.amount.Label(currentUnits), &fontSize30, colorLabelText, &textObjectRainTodayUnitsLabel, positionRainfallTodayValue + unitsLabelOffset);
	RenderString32(U"Rate", &fontSize32, olc::GREY, &textObjectRainfallRateLabel, positionRainfallRateLabel);
	if (currentUnits == metricUnits)
		tempString32 = ConvertedString32(std::format("{:04.1f}", rainfallRatePerHour.millimeters));
	else
		tempString32 = ConvertedString32(std::format("{:.2f}", rainfallRatePerHour.inches));
	unitsLabelOffset = { fontSize32.GetStringBounds(tempString32).size.x + 15, fontSize32.GetStringBounds(tempString32).size.y - fontSize30.GetStringBounds(rainfallUnits.rate.Label(currentUnits)).size.y };
	RenderString32(tempString32, &fontSize32, olc::WHITE, &textObjectRainfallRateValue, positionRainfallRateValue);
	RenderString32(rainfallUnits.rate.Label(currentUnits), &fontSize30, colorLabelText, &textObjectRainfallRateUnits, positionRainfallRateValue + unitsLabelOffset);
	//RenderStringCentered(std::format("{:.2f} in", rainfallTotalTodayValue), &fontSize32, olc::WHITE, &renderableRainfallValue, positionRainfallValue);

	if (rainEventStartTime != 0)
	{
		olc::vi2d gfxTextPos = RenderStringSegment("Rain started at", &fontSize18, olc::GREY, &textObjRainStartLabel, olc::vi2d(positionRainfallTodayLabel.x, positionRainAreaStart.y + positionRainAreaSize.y + 8), 8);
		gfxTextPos = RenderStringSegment(strRainEventStartTime, &fontSize18, olc::WHITE, &textObjRainStartValue, gfxTextPos, 0);
		if (rainEventStopTime != 0)
		{
			gfxTextPos = RenderStringSegment("; stopped", &fontSize18, olc::GREY, &textObjRainStopLabel, gfxTextPos, 8);
			RenderString(strRainEventStopTime, &fontSize18, olc::WHITE, &textObjRainStopValue, gfxTextPos);
		}
	}

	DrawLineDecal(posSensorAreaDividerStart, posSensorAreaDividerEnd, areasBorderColor);	// Show divider line between Outdoor/Indoor Sensor information

	// Display the signal reliability meter and render the corresponding icon
	RenderStringCentered("Outdoor", &fontSize24, colorLabelText, &textObjectSensorOutdoorLabel, positionSignalOutdoorCenter);
	RenderStringCentered("Indoor", &fontSize24, colorLabelText, &textObjectSensorIndoorLabel, positionSignalIndoorCenter);

	// Display signal meters for both Outdoor and Indoor sensors representing how stable the telemetry is
	olc::Pixel sensorSignalColors[5] = { olc::Pixel(255, 62, 46), olc::WHITE, olc::WHITE, olc::WHITE, olc::GREEN };
	RenderStringRightJustified("Signal:", &fontSize18, olc::GREY, &textObjectSignalOutdoorLabel, positionSignalOutdoorCenter + olc::vf2d(-5, 50));
	RenderStringRightJustified("Signal:", &fontSize18, olc::GREY, &textObjectSignalIndoorLabel, positionSignalIndoorCenter + olc::vf2d(-5, 50));
	DrawDecal(positionSignalMeterOutdoor, renderableSignalStrength[0].Decal(), { 0.0625f, 0.0625f });
	DrawDecal(positionSignalMeterIndoor, renderableSignalStrength[0].Decal(), { 0.0625f, 0.0625f });
	DrawDecal(positionSignalMeterOutdoor, renderableSignalStrength[outdoorSensor.packetCounter].Decal(), { 0.0625f, 0.0625f }, sensorSignalColors[outdoorSensor.packetCounter]);
	DrawDecal(positionSignalMeterIndoor, renderableSignalStrength[indoorSensor.packetCounter].Decal(), { 0.0625f, 0.0625f }, sensorSignalColors[indoorSensor.packetCounter]);

	// Next display each sensor's battery condition
	RenderStringRightJustified("Battery:", &fontSize18, olc::GREY, &textObjectBatteryOutdoorLabel, positionBatteryOutdoorLabel);
	RenderStringRightJustified("Battery:", &fontSize18, olc::GREY, &textObjectBatteryIndoorLabel, positionBatteryIndoorLabel);
	RenderStringRightJustified("Channel: ", &fontSize18, olc::GREY, &textObjectChannelOutdoorLabel, positionChannelOutdoorLabel);
	RenderStringRightJustified("Channel: ", &fontSize18, olc::GREY, &textObjectChannelIndoorLabel, positionChannelIndoorLabel);
	if (outdoorSensor.batteryStatus == undefinedFloatValue)
		RenderString32(U"-  -", &fontSize18, olc::GREY, &textObjectBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	else if (outdoorSensor.batteryStatus == batteryStatusNormal)
		RenderString32(U"Normal", &fontSize18, olc::GREEN, &textObjectBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0));
	else
		RenderString32(U"Low", &fontSize18, olc::Pixel(255, 62, 46), &textObjectBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0));
	if (indoorSensor.batteryStatus == undefinedFloatValue)
		RenderString32(U"-  -", &fontSize18, olc::GREY, &textObjectBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	else if (indoorSensor.batteryStatus == batteryStatusNormal)
		RenderString32(U"Normal", &fontSize18, olc::GREEN, &textObjectBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0));
	else
		RenderString32(U"Low", &fontSize18, olc::Pixel(255, 62, 46), &textObjectBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0));

	// Finally display each sensor's current channel value
	if (outdoorSensor.channel != "")
		RenderString(outdoorSensor.channel, &fontSize18, olc::WHITE, &textObjectChannelOutdoorValue, positionChannelOutdoorLabel + olc::vf2d(14, 0));
	else
		RenderString("-  -", &fontSize18, olc::GREY, &textObjectChannelOutdoorValue, positionChannelOutdoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	if (indoorSensor.channel != "")
		RenderString(indoorSensor.channel, &fontSize18, olc::WHITE, &textObjectChannelIndoorValue, positionChannelIndoorLabel + olc::vf2d(14, 0));
	else
		RenderString("-  -", &fontSize18, olc::GREY, &textObjectChannelIndoorValue, positionChannelIndoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);

	// Display Indoor Temperature and Humidity
	olc::vf2d positionIndoorTempValue = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.39f), (positionIndoorAreaSize.y * 0.32f) + positionIndoorAreaStart.y };
	olc::vf2d positionIndoorHumidityValue = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.80f), (positionIndoorAreaSize.y * 0.30f) + positionIndoorAreaStart.y };
	if (indoorSensor.temperature.current.IsDefined())
		RenderStringRightJustified(std::format("{:.1f}", indoorSensor.temperature.current.GetValue(currentUnits)), &fontSize56, olc::WHITE, &textObjectIndoorTempValue, positionIndoorTempValue);
	else
		RenderStringRightJustified("- -", &fontSize56, olc::GREY, &textObjectIndoorTempValue, positionIndoorTempValue + olc::vf2d(-10, positionDashCorrection56.y));
	RenderString32(degreeUnits.Label(currentUnits), &fontSize24, colorLabelText, &textObjectIndoorTempUnits, positionIndoorTempValue + olc::vf2d(8, 0));

	//DrawDecal(positionTemp + olc::vf2d(0, fontSize56.GetStringBounds(tempString32).size.y + 10), renderableLabelTempLow.Decal());
	//RenderHighOrLowValue("Low", highLowIndoorTempF.low, &renderableIndoorTempLowValue, positionTemp + olc::vf2d(10, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
	//RenderHighOrLowValue("High", highLowIndoorTempF.high, &renderableIndoorTempHighValue, positionTemp + olc::vf2d(85, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
	//RenderString32(U"Low  69    High  72", &fontSize18, colorLabelText, &renderableIndoorHighLabel, positionTemp + olc::vf2d(-10, fontSize56.GetStringBounds(tempString32).size.y + 10));
	//positionTemp += olc::vf2d(fontSize56.GetStringBounds(tempString32).size.x + spacerFontSize18, 0);
	//RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableIndoorTempUnits, positionTemp);
	if (indoorSensor.humidity.current != undefinedIntValue)
		RenderStringRightJustified(std::format("{:d}", indoorSensor.humidity.current), &fontSize56, olc::WHITE, &textObjectIndoorHumidityValue, positionIndoorHumidityValue);
	else
		RenderStringRightJustified("- -", &fontSize56, olc::GREY, &textObjectIndoorHumidityValue, positionIndoorHumidityValue + olc::vf2d(-10, positionDashCorrection56.y));
	RenderString32(U"%", &fontSize24, colorLabelText, &textObjectIndoorHumidityUnits, positionIndoorHumidityValue + olc::vf2d(8, 0));

	// Display the Settings/Info Area assets
	//DrawDecal(positionInfoAreaGearIcon, decalSettingsIcon);


	//DrawDecal(positionInfoAreaStart + positionConditionsNowOffset, decalCurConditionsImage);
	//RenderStringCentered(webWxCurConditionsDesc, &fontSize18, olc::WHITE, &renderableConditionsDesc, positionInfoAreaStart + positionConditionsNowOffset + olc::vf2d(spriteCurConditionsImage->width / 2, spriteCurConditionsImage->height));

	float infoAreaThirdsWidth = positionForecastAreaSize.x * 0.33333f;
	float infoAreaThirdsOffset = infoAreaThirdsWidth / 2.0f;
	float infoAreaIconStartY = positionForecastAreaStart.y + 54;
	olc::vf2d positionInfoNowCenter = positionForecastAreaStart + olc::vf2d(infoAreaThirdsWidth / 2.0f, 30);
	olc::vf2d positionInfoTodayCenter = positionForecastAreaStart + olc::vf2d(infoAreaThirdsWidth + infoAreaThirdsOffset, 30);
	olc::vf2d positionInfoTomorrowCenter = positionForecastAreaStart + olc::vf2d(infoAreaThirdsWidth + infoAreaThirdsWidth + infoAreaThirdsOffset, 30);
	olc::vf2d positionInfoDividerTop = positionForecastAreaStart + olc::vf2d(infoAreaThirdsWidth, positionForecastAreaSize.y * 0.23f);
	olc::vf2d positionInfoDividerBottom = { positionInfoDividerTop.x, positionForecastAreaStart.y + positionForecastAreaSize.y - (positionForecastAreaSize.y * 0.2f) };
	//olc::vf2d positionInfo3dayCenter = { infoAreaDividerX + ((positionInfoAreaStart.x + positionInfoAreaSize.x - infoAreaDividerX) / 2.0f), positionInfoAreaStart.y + 30 };
	if (webWxEnabled)
	{
		RenderStringCentered("Now", &fontSize22, colorLabelText, &textObjectForecastNowLabel, positionInfoNowCenter);
		RenderStringCentered(dayOfTheWeekAbbr[webWxDailyForecasts[0].dateTime.tm_wday], &fontSize22, colorLabelText, &textObjectForecastToday, positionInfoTodayCenter);
		RenderStringCentered(dayOfTheWeekAbbr[webWxDailyForecasts[1].dateTime.tm_wday], &fontSize22, colorLabelText, &textObjectForecastTomorrow, positionInfoTomorrowCenter);
		// Draw divider line betweeen "Current" conditions and the "Today/Tomorrow" forecasts
		DrawLineDecal(positionInfoDividerTop, positionInfoDividerBottom, areasBorderColor);

		if (webWxNewDataReady)
		{
			LoadWebWxAssets(&webWxCurrentConditions, renderableWebConditionsImage.Decal());
			for (int i = 0; i < 2; i++)
				LoadWebWxAssets(&webWxDailyForecasts[i], renderableWebForecastImages[i].Decal());
			curlResponseBuffer.clear();
			webWxNewDataReady = false;
		}

		RenderCenteredWxCondition({ positionInfoNowCenter.x, infoAreaIconStartY }, renderableWebConditionsImage.Decal(), &textObjectConditionsDesc);
		RenderCenteredWxForecast({ positionInfoTodayCenter.x, infoAreaIconStartY }, renderableWebForecastImages[0].Decal(), &webWxDailyForecasts[0], textObjForecastToday);
		RenderCenteredWxForecast({ positionInfoTomorrowCenter.x, infoAreaIconStartY }, renderableWebForecastImages[1].Decal(), &webWxDailyForecasts[1], textObjForecastTomorrow);
	}

	float outdoorAreaRightEdgeX = (positionOutdoorAreaStart.x + positionOutdoorAreaSize.x);
	float iconsCenterOffsetX = positionWindowCenter.x - ((((positionWindowCenter.x - windCircleRadius) - outdoorAreaRightEdgeX) / 2) + outdoorAreaRightEdgeX);
	float iconsHalfWidth = renderableSettingsIcon.Sprite()->width / 2;
	int settingsIconY = positionSystemTime.y + fontSize72.GetStringBounds(ConvertedString32(strFullyFormattedTime)).size.y - renderableSettingsIcon.Sprite()->height;
	float infoIconX = positionWindowCenter.x - (fontSize40.GetStringBounds(ConvertedString32(strFullyFormattedDate)).size.x / 2.0f);
	positionSettingsIcon = olc::vi2d(positionWindowCenter.x + iconsCenterOffsetX - iconsHalfWidth, settingsIconY);
	positionCloseIcon = olc::vi2d(positionWindowCenter.x - iconsCenterOffsetX - iconsHalfWidth, settingsIconY);
	DrawDecal(positionSettingsIcon, renderableSettingsIcon.Decal());
	DrawDecal(positionCloseIcon, renderableCloseIcon.Decal());
	
	// If there was no valid config loaded, show an error message
	if (showNoValidConfigMsg)
		ShowCenteredDialogBox(&dialogBoxNoValidConfig);

	return true;
}

bool DragonWx::OnUserDestroy()
{
	appShouldExit = appExitRequested;
	return true;
}

void DragonWx::OnTextEntryComplete(const std::string& textResult)
{
	bool inputValueValid = true;
	if (setupActiveInputBoxPtr->value.type == inputBoxType::INT)
	{
		if (!isWhiteSpaceOnly(textResult))
		{
			try { setupActiveInputBoxPtr->value.text = std::to_string(std::stoi(textResult)); }
			catch (...) { inputValueValid = false; }
		}
		else
			setupActiveInputBoxPtr->value.text = "0";
	}
	else if (setupActiveInputBoxPtr->value.type == inputBoxType::FLOAT)
	{
		try
		{
			// If the Input Box is one of the GPS coordinates, use fixed 4 decimal places, otherwise 1 decimal place for temperature offsets
			if ((setupActiveInputBoxPtr == &inputBoxOutdoorCalTemp) || (setupActiveInputBoxPtr == &inputBoxIndoorCalTemp))
			{
				if (!isWhiteSpaceOnly(textResult))
					setupActiveInputBoxPtr->value.text = std::format("{:.1f}", std::stod(textResult));
				else
					setupActiveInputBoxPtr->value.text = "0.0";
			}
			else
				setupActiveInputBoxPtr->value.text = std::format("{:.4f}", std::stod(textResult));
		}
		catch (...) { inputValueValid = false; }
	}
	else
		setupActiveInputBoxPtr->value.text = textResult;

	if (inputValueValid)
	{
		TextEntryEnable(false);
		setupActiveInputBoxPtr = nullptr;
	}
	else
		dialogBoxInForeground = true;
}

bool DragonWx::mouseWithinArea(olc::vf2d areaStart, olc::vf2d areaSize) const
{ return ((positionMouseCursor.x > areaStart.x) && (positionMouseCursor.x < (areaStart.x + areaSize.x)) && (positionMouseCursor.y > areaStart.y) && (positionMouseCursor.y < (areaStart.y + areaSize.y))); }

bool DragonWx::mouseClickedInputBox(inputBoxStruct* inputBoxPtr) const
{ return mouseWithinArea(inputBoxPtr->value.pos, inputBoxPtr->value.size) && (setupActiveInputBoxPtr != inputBoxPtr); }

void DragonWx::ActivateInputBox(inputBoxStruct* inputBoxPtr)
{
	setupActiveInputBoxPtr = inputBoxPtr;
	TextEntryEnable(true, inputBoxPtr->value.text);
}

bool DragonWx::PasteTextFromClipboard()
{
	#ifdef _WIN32
	if (!OpenClipboard(nullptr))
	{
		PRINT_DEBUG("Could not open Windows clipboard.\n");
		return false;
	}

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData != nullptr)
		{
			char* clipboardTextPtr = static_cast<char*>(GlobalLock(hData));
			if (clipboardTextPtr != nullptr)
			{
				strClipboardContents = clipboardTextPtr;
				GlobalUnlock(hData);
				CloseClipboard();
				return true;
			}
			GlobalUnlock(hData);
		}
	}
	CloseClipboard();
	#endif
	return false;
}

bool DragonWx::isWhiteSpaceOnly(const std::string& inputString)
{ return std::all_of(inputString.begin(), inputString.end(), [](unsigned char c) { return std::isspace(c); }); }

olc::vi2d DragonWx::StringPixelSize(olc::Font& sourceFont, std::u32string& targetString32)
{ return sourceFont.GetStringBounds(targetString32).size; }

olc::vi2d DragonWx::GetCenteredStartPosition(olc::vi2d totalAreaSize, olc::vi2d objectAreaSize)
{ return olc::vi2d((totalAreaSize.x / 2) - (objectAreaSize.x / 2), (totalAreaSize.y / 2) - (objectAreaSize.y / 2)); }

std::u32string DragonWx::ConvertedString32(std::string inputString)
{ return std::u32string(inputString.begin(), inputString.end()); }

olc::vi2d DragonWx::TextCenteredOffsetY(std::string inputString, olc::Font* fontPtr)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	return { 0, fontPtr->GetStringBounds(tempString32).size.y / 2 };
}

/*
std::u32string DragonWx::BoxResultVariableAsString32(inputBoxStruct* inputBoxPtr)
{
	std::string tempString;
	switch (inputBoxPtr->value.resultPtr.index())
	{
	case 0:
		tempString = *std::get<std::string*>(inputBoxPtr->value.resultPtr);
		break;
	case 1:
		tempString = std::format("{:.1f}", *std::get<float*>(inputBoxPtr->value.resultPtr));
		break;
	case 2:
		tempString = std::to_string(*std::get<int*>(inputBoxPtr->value.resultPtr));
		break;
	}
	return std::u32string(tempString.begin(), tempString.end());
}
*/

void DragonWx::RenderString32(std::u32string inputString32, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d leftPos, bool forceRender)
{
	if ((inputString32 != textObj->string32) || forceRender)
	{
		textObj->renderable = fontToUse->RenderStringToRenderable(inputString32 + U" ", textColor);
		textObj->posOffset = { 0, 0 };
		textObj->width = fontToUse->GetStringBounds(inputString32).size.x;
		textObj->height = fontToUse->GetStringBounds(inputString32).size.y;
		textObj->string32 = inputString32;
	}
	DrawDecal(leftPos, textObj->renderable.Decal());
}

void DragonWx::RenderString32(std::u32string inputString32, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d leftPos)
{  RenderString32(inputString32, fontToUse, textColor, textObj, leftPos, false);  }

void DragonWx::RenderString(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d leftPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	if (tempString32 != textObj->string32)
	{
		textObj->renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		textObj->posStart = leftPos;
		textObj->posOffset = { 0, 0 };
		textObj->width = fontToUse->GetStringBounds(tempString32).size.x;
		textObj->height = fontToUse->GetStringBounds(tempString32).size.y;
		textObj->string32 = tempString32;
	}
	DrawDecal(leftPos, textObj->renderable.Decal());
}

olc::vi2d DragonWx::RenderStringSegment(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d leftPos, int spaceWidth)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	if (tempString32 != textObj->string32)
	{
		textObj->renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		textObj->posOffset = { 0, 0 };
		textObj->width = fontToUse->GetStringBounds(tempString32).size.x;
		textObj->height = fontToUse->GetStringBounds(tempString32).size.y;
		textObj->string32 = tempString32;
	}
	DrawDecal(leftPos, textObj->renderable.Decal());
	return olc::vi2d(leftPos.x + fontToUse->GetStringBounds(tempString32).size.x + spaceWidth, leftPos.y);
}

void DragonWx::RenderStringCentered(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d centerPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	if ((tempString32 != textObj->string32) || (centerPos != textObj->posStart))
	{
		textObj->renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		textObj->posStart = centerPos;
		textObj->posOffset = centerPos - olc::vi2d(fontToUse->GetStringBounds(tempString32).size.x / 2, 0);
		textObj->width = fontToUse->GetStringBounds(tempString32).size.x;
		textObj->height = fontToUse->GetStringBounds(tempString32).size.y;
		textObj->string32 = tempString32;
	}
	DrawDecal(textObj->posOffset, textObj->renderable.Decal());
}

void DragonWx::RenderStringRightJustified(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, textObject* textObj, olc::vi2d rightPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	if (tempString32 != textObj->string32)
	{
		textObj->renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		textObj->posOffset = { fontToUse->GetStringBounds(tempString32).size.x, 0 };
		textObj->width = fontToUse->GetStringBounds(tempString32).size.x;
		textObj->height = fontToUse->GetStringBounds(tempString32).size.y;
		textObj->string32 = tempString32;
	}
	DrawDecal(rightPos - textObj->posOffset, textObj->renderable.Decal());
}

void DragonWx::RenderHighOrLowValue(std::string labelText, double highLowValue, textObject* textValue, olc::vf2d valuePos, bool useSmallerText)
{
	if (useSmallerText)
	{
		if (labelText == "Low")
			DrawDecal(valuePos - olc::vf2d(fontSize18.GetStringBounds(U"Low ").size.x, 0), renderableLowLabel18.Decal());
		else if (labelText == "High")
			DrawDecal(valuePos - olc::vf2d(fontSize18.GetStringBounds(U"High ").size.x, 0), renderableHighLabel18.Decal());
		if (highLowValue != undefinedFloatValue)
			RenderString(std::format("{:.0f}", highLowValue), &fontSize18, olc::WHITE, textValue, valuePos + olc::vf2d(8, 0));
		else
			RenderString32(U"-  -", &fontSize18, olc::WHITE, textValue, valuePos + positionDashCorrection18 + olc::vf2d(8, 0));
	}
	else
	{
		if (labelText == "Low")
			DrawDecal(valuePos - olc::vf2d(fontSize24.GetStringBounds(U"Low ").size.x, 0), renderableLowLabel24.Decal());
		else if (labelText == "High")
			DrawDecal(valuePos - olc::vf2d(fontSize24.GetStringBounds(U"High ").size.x, 0), renderableHighLabel24.Decal());

		if (highLowValue != undefinedFloatValue)
			RenderString(std::format("{:.0f}", highLowValue), &fontSize24, olc::WHITE, textValue, valuePos + olc::vf2d(9, 0));
		else
			RenderString32(U"-  -", &fontSize24, olc::WHITE, textValue, valuePos + positionDashCorrection24 + olc::vf2d(9, 0));
	}
}

void DragonWx::DrawWindDirectionArrow(double degreesBearing, olc::Pixel arrowColor)
{
	olc::vf2d windArrowOffset = olc::vf2d(windCircleRadius * std::sin(degreesToRadians(degreesBearing)), (windCircleRadius * std::cos(degreesToRadians(degreesBearing))) * -1.0f);
	DrawRotatedDecal(windCircleCenterPoint + windArrowOffset, renderableWindDir.Decal(), degreesToRadians(degreesBearing), centerPointWindDir, {1.0f, 1.0f}, arrowColor);
}

void DragonWx::DrawCircleArc(olc::vf2d startPos, int radius, double startAngle, double endAngle, olc::Pixel pixelColor)
{
	for (double angle = startAngle; angle <= endAngle; angle += 0.1)	// Increment in degrees
	{
		double rad = angle * (std::numbers::pi / 180.0);				// Convert to radians
		int x = startPos.x + radius * cos(rad);
		int y = startPos.y + radius * sin(rad);
		Draw({ x, y }, olc::RED);
	}
}
/*
void DragonWx::DrawWindDirPreviousArc(olc::vf2d startPos, int radius, olc::Pixel pixelColor, uint8_t mask)
{
	if (radius < 0 || startPos.x < -radius || startPos.y < -radius || startPos.x - GetDrawTargetWidth() > radius || startPos.y - GetDrawTargetHeight() > radius)
		return;

	if (radius > 0)
	{
		int x0 = 0;
		int y0 = radius;
		int d = 3 - 2 * radius;

		while ((y0 - 50) >= x0) // only formulate 1/8 of circle
		{
			// Draw even octants
			if (mask & 0x01) Draw(startPos.x + x0, startPos.y - y0, pixelColor);		// Q6 - upper right right
			if (mask & 0x04) Draw(startPos.x + y0, startPos.y + x0, pixelColor);		// Q4 - lower lower right
			if (mask & 0x10) Draw(startPos.x - x0, startPos.y + y0, pixelColor);		// Q2 - lower left left
			if (mask & 0x40) Draw(startPos.x - y0, startPos.y - x0, pixelColor);		// Q0 - upper upper left
			if (x0 != 0 && x0 != y0)
			{
				if (mask & 0x02) Draw(startPos.x + y0, startPos.y - x0, pixelColor);	// Q7 - upper upper right
				if (mask & 0x08) Draw(startPos.x + x0, startPos.y + y0, pixelColor);	// Q5 - lower right right
				if (mask & 0x20) Draw(startPos.x - y0, startPos.y + x0, pixelColor);	// Q3 - lower lower left
				if (mask & 0x80) Draw(startPos.x - x0, startPos.y - y0, pixelColor);	// Q1 - upper left left
			}

			if (d < 0)
				d += 4 * x0++ + 6;
			else
				d += 4 * (x0++ - y0--) + 10;
		}
	}
	else
		Draw(startPos.x, startPos.y, pixelColor);

}
*/

void DragonWx::DrawBoxTitle(titleBox* titleBoxPtr, olc::Renderable* targetRenderablePtr)
{
	olc::Sprite* prevDrawTarget = GetDrawTarget();
	SetDrawTarget(targetRenderablePtr->Sprite());

	int xRightEdge = titleBoxPtr->posStart.x + titleBoxPtr->size.x;
	int yBottomEdge = titleBoxPtr->posStart.y + titleBoxPtr->size.y;
	int textOffset = (titleBoxPtr->size.x - titleBoxPtr->fontPtr->GetStringBounds(titleBoxPtr->text32).size.x) / 2;
	int topSegLength = titleBoxPtr->size.x - (titleBoxPtr->fontPtr->GetStringBounds(titleBoxPtr->text32).size.x + titleBoxPtr->titlePadding + titleBoxPtr->radius);
	int topSegRightStart = xRightEdge - topSegLength;
	int distSquared, radiusSquared;

	DrawLine(olc::vi2d(topSegRightStart, titleBoxPtr->posStart.y), olc::vi2d(xRightEdge - titleBoxPtr->radius, titleBoxPtr->posStart.y), titleBoxPtr->color);

	DrawLine(olc::vi2d(titleBoxPtr->posStart.x + titleBoxPtr->radius, yBottomEdge), olc::vi2d(xRightEdge - titleBoxPtr->radius, yBottomEdge), titleBoxPtr->color);
	DrawLine(olc::vi2d(titleBoxPtr->posStart.x, titleBoxPtr->posStart.y + titleBoxPtr->radius), olc::vi2d(titleBoxPtr->posStart.x, yBottomEdge - titleBoxPtr->radius), titleBoxPtr->color);
	DrawLine(olc::vi2d(xRightEdge, titleBoxPtr->posStart.y + titleBoxPtr->radius), olc::vi2d(xRightEdge, yBottomEdge - titleBoxPtr->radius), titleBoxPtr->color);

	for (int dx = 0; dx <= titleBoxPtr->radius; dx++)
		for (int dy = 0; dy <= titleBoxPtr->radius; dy++)
		{
			distSquared = (dx * dx) + (dy * dy);
			radiusSquared = titleBoxPtr->radius * titleBoxPtr->radius;
			if ((distSquared >= (radiusSquared - titleBoxPtr->radius)) && (distSquared <= (radiusSquared + titleBoxPtr->radius)))
			{
				olc::vi2d pixelPosTopLeft = { titleBoxPtr->posStart.x + titleBoxPtr->radius - dx, titleBoxPtr->posStart.y + titleBoxPtr->radius - dy };
				olc::vi2d pixelPosTopRight = { xRightEdge - titleBoxPtr->radius + dx, titleBoxPtr->posStart.y + titleBoxPtr->radius - dy };
				olc::vi2d pixelPosBottomLeft = { titleBoxPtr->posStart.x + titleBoxPtr->radius - dx, yBottomEdge - titleBoxPtr->radius + dy };
				olc::vi2d pixelPosBottomRight = { xRightEdge - titleBoxPtr->radius + dx, yBottomEdge - titleBoxPtr->radius + dy };
				Draw(olc::vi2d(titleBoxPtr->posStart.x + titleBoxPtr->radius - dx, titleBoxPtr->posStart.y + titleBoxPtr->radius - dy), titleBoxPtr->color);
				Draw(olc::vi2d(xRightEdge - titleBoxPtr->radius + dx, titleBoxPtr->posStart.y + titleBoxPtr->radius - dy), titleBoxPtr->color);
				Draw(olc::vi2d(titleBoxPtr->posStart.x + titleBoxPtr->radius - dx, yBottomEdge - titleBoxPtr->radius + dy), titleBoxPtr->color);
				Draw(olc::vi2d(xRightEdge - titleBoxPtr->radius + dx, yBottomEdge - titleBoxPtr->radius + dy), titleBoxPtr->color);
			}
		}

	titleBoxPtr->posTitle = olc::vi2d(titleBoxPtr->posStart.x + titleBoxPtr->radius + (titleBoxPtr->titlePadding / 2), (titleBoxPtr->posStart.y - titleBoxPtr->fontPtr->GetStringBounds(titleBoxPtr->text32).size.y / 2));
	//targetRenderablePtr->Decal()->Update();
	SetDrawTarget(prevDrawTarget);
}

void DragonWx::DrawRainGaugeOutlines(olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor)
{
	olc::Sprite* prevDrawTarget = GetDrawTarget();
	SetDrawTarget(renderableRainGaugeOutline.Sprite());
	Clear(olc::BLANK);

	int xRightEdge = posUpperLeft.x + rectSize.x;
	int yBottomEdge = posUpperLeft.y + rectSize.y;
	int distSquared, radiusSquared, positionY;
	
	int gaugeSingleTickPixels = rectSize.y / rainGaugeMarksTotal;

	// Draw the line marking "ticks" and calculate the text positions for the unit labels to render later
	rainGaugeTickMarks[0].pos = { posUpperLeft.x - 60, yBottomEdge - fontSize18.GetStringBounds(U"0.00 in").size.y };
	rainGaugeTickMarks[rainGaugeMarksTotal].pos = { posUpperLeft.x - 60, posUpperLeft.y };
	for (int i = 1; i < rainGaugeMarksTotal; i++)
	{
		positionY = yBottomEdge - (i * gaugeSingleTickPixels);
		DrawLine({ posUpperLeft.x, positionY }, { posUpperLeft.x + 10, positionY }, olc::GREY);
		rainGaugeTickMarks[i].pos = { posUpperLeft.x - 60, positionY - ((fontSize18.GetStringBounds(U"1").size.y / 2) + 2) };
	}

	DrawLine({ posUpperLeft.x + radius, posUpperLeft.y }, { xRightEdge - radius, posUpperLeft.y }, pixelColor);
	DrawLine({ posUpperLeft.x + radius, yBottomEdge }, { xRightEdge - radius, yBottomEdge }, pixelColor);
	DrawLine({ posUpperLeft.x, posUpperLeft.y + radius }, { posUpperLeft.x, yBottomEdge - radius }, pixelColor);
	DrawLine({ xRightEdge, posUpperLeft.y + radius }, { xRightEdge, yBottomEdge - radius }, pixelColor);

	for (int dx = 0; dx <= radius; dx++)
		for (int dy = 0; dy <= radius; dy++)
		{
			distSquared = (dx * dx) + (dy * dy);
			radiusSquared = radius * radius;
			if ((distSquared >= (radiusSquared - radius)) && (distSquared <= (radiusSquared + radius)))
			{
				olc::vi2d pixelPosTopLeft = { posUpperLeft.x + radius - dx, posUpperLeft.y + radius - dy };
				olc::vi2d pixelPosTopRight = { xRightEdge - radius + dx, posUpperLeft.y + radius - dy };
				olc::vi2d pixelPosBottomLeft = { posUpperLeft.x + radius - dx, yBottomEdge - radius + dy };
				olc::vi2d pixelPosBottomRight = { xRightEdge - radius + dx, yBottomEdge - radius + dy };
				Draw({ posUpperLeft.x + radius - dx, posUpperLeft.y + radius - dy }, pixelColor);
				Draw({ xRightEdge - radius + dx, posUpperLeft.y + radius - dy }, pixelColor);
				Draw({ posUpperLeft.x + radius - dx, yBottomEdge - radius + dy }, pixelColor);
				Draw({ xRightEdge - radius + dx, yBottomEdge - radius + dy }, pixelColor);
			}
		}
	SetDrawTarget(prevDrawTarget);
	renderableRainGaugeOutline.Decal()->Update();
}

void DragonWx::RenderRainGaugeText()
{
	float gaugeSingleTickValue = rainGaugeCapacity.GetValue(currentUnits) / rainGaugeMarksTotal;

	// First draw the bottom and top unit labels
	if (currentUnits == metricUnits)
	{
		olc::vi2d extraOffset = olc::vi2d(10, 0);
		RenderString32(U"00.0 mm", &fontSize18, olc::GREY, &rainGaugeTickMarks[0].textObj, rainGaugeTickMarks[0].pos - extraOffset);
		RenderString(std::format("{:04.1f} mm", rainGaugeCapacity.GetValue(currentUnits)), &fontSize18, olc::GREY, &rainGaugeTickMarks[rainGaugeMarksTotal].textObj, rainGaugeTickMarks[rainGaugeMarksTotal].pos - extraOffset);
		RenderString(std::format("{:04.1f} mm", 2 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[2].textObj, rainGaugeTickMarks[2].pos - extraOffset);
		RenderString(std::format("{:04.1f} mm", 4 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[4].textObj, rainGaugeTickMarks[4].pos - extraOffset);
		RenderString(std::format("{:04.1f} mm", 6 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[6].textObj, rainGaugeTickMarks[6].pos - extraOffset);
	}
	else
	{
		RenderString32(U"0.00 in", &fontSize18, olc::GREY, &rainGaugeTickMarks[0].textObj, rainGaugeTickMarks[0].pos);
		RenderString(std::format("{:.2f} in", rainGaugeCapacity.GetValue(currentUnits)), &fontSize18, olc::GREY, &rainGaugeTickMarks[rainGaugeMarksTotal].textObj, rainGaugeTickMarks[rainGaugeMarksTotal].pos);
		RenderString(std::format("{:.2f} in", 2 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[2].textObj, rainGaugeTickMarks[2].pos);
		RenderString(std::format("{:.2f} in", 4 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[4].textObj, rainGaugeTickMarks[4].pos);
		RenderString(std::format("{:.2f} in", 6 * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[6].textObj, rainGaugeTickMarks[6].pos);
	}
	/*
	// Now draw the other unit labels next to their corresponding line markings
	for (int i = 1; i < rainGaugeMarksTotal; i++)
	{
		if ((i % 2) == 0)
			RenderString(std::format(tempString, i * gaugeSingleTickValue), &fontSize18, olc::GREY, &rainGaugeTickMarks[i].textObj, rainGaugeTickMarks[i].pos);
	}
	*/
}

void DragonWx::DrawUVindexGraph(olc::vf2d startPos, std::u32string strValue32)
{
	// Configure dimensions and position info for the graph decals
	uvGraphTotalHeight = fontSize24.GetStringBounds(strValue32).size.y * 0.65f;
	//positionUVindexLabel = positionLightLevelLabel + olc::vf2d(0, 35);
	positionUVindexGraph = startPos + olc::vf2d(fontSize24.GetStringBounds(strValue32).size.x + 20.0f, (fontSize24.GetStringBounds(strValue32).size.y - uvGraphTotalHeight) / 2.0f);
	uvGraphTotalWidth = (positionOutdoorAreaStart.x + positionOutdoorAreaSize.x - positionUVindexGraph.x);
	uvSegmentLength = uvGraphTotalWidth / 11.0f;

	// Render the gradient graph
	float segmentOffsetCounter = 0.0f;
	for (int i = 0; i < 4; i++)
	{
		uvSegmentSizes[i] = uvSegmentRatios[i] * uvGraphTotalWidth;
		uvSegmentOffsets[i] = segmentOffsetCounter;
		segmentOffsetCounter += uvSegmentSizes[i];
		GradientFillRectDecal(positionUVindexGraph + olc::vf2d(uvSegmentOffsets[i], 0), { uvSegmentSizes[i], uvGraphTotalHeight }, uvPixelColors[i], uvPixelColors[i], uvPixelColors[i + 1], uvPixelColors[i + 1]);
	}
	DrawRectDecal(positionUVindexGraph, { uvGraphTotalWidth, uvGraphTotalHeight }, areasBorderColor);	// Add a border around it

	// Finally, render the arrow pointing to our current value on the graph
	if ((uvIndex.current >= 0) && (uvIndex.current <= 2))
		uvPixelColorCurrent = PixelLerp(uvPixelColors[0], uvPixelColors[1], uvIndex.current / 3.0f);
	else if ((uvIndex.current >= 3) && (uvIndex.current <= 5))
		uvPixelColorCurrent = PixelLerp(uvPixelColors[1], uvPixelColors[2], (uvIndex.current - 3) / 3.0f);
	else if ((uvIndex.current >= 6) && (uvIndex.current <= 7))
		uvPixelColorCurrent = PixelLerp(uvPixelColors[2], uvPixelColors[3], (uvIndex.current - 6) / 2.0f);
	else if ((uvIndex.current >= 8) && (uvIndex.current <= 10))
		uvPixelColorCurrent = PixelLerp(uvPixelColors[3], uvPixelColors[4], (uvIndex.current - 8) / 3.0f);
	else if (uvIndex.current >= 11)
		uvPixelColorCurrent = uvPixelColors[4];

	olc::vf2d positionUVarrow = { uvIndex.current * uvSegmentLength, uvGraphTotalHeight };
	FillTriangleDecal(positionUVindexGraph + positionUVarrow, positionUVindexGraph + positionUVarrow + olc::vf2d(5.0f, 13.0f), positionUVindexGraph + positionUVarrow + olc::vf2d(-5.0f, 13.0f), uvPixelColorCurrent);

	//for (int i = 0; i < 4; i++)
	//	GradientFillRectDecal(positionUVindexGraph + olc::vf2d(uvSegmentOffsets[i], 0), { uvSegmentSizes[i], uvGraphTotalHeight }, uvPixelColors[i], uvPixelColors[i], uvPixelColors[i + 1], uvPixelColors[i + 1]);
}

void DragonWx::RenderButton(buttonStruct* buttonPtr)
{
	//std::u32string tempString32 = std::u32string(buttonText.begin(), buttonText.end()) + U" ";
	if (buttonPtr->text32 != buttonPtr->textObj.string32)
	{
		buttonPtr->textObj.renderable = buttonPtr->fontPtr->RenderStringToRenderable(buttonPtr->text32, buttonPtr->textColor);
		int fontUnderOffset = buttonPtr->fontPtr->GetStringBounds(buttonPtr->text32).size.y + buttonPtr->fontPtr->GetStringBounds(buttonPtr->text32).offset.y;
		buttonPtr->textObj.posOffset = { buttonPtr->fontPtr->GetStringBounds(buttonPtr->text32).size.x / 2, (buttonPtr->fontPtr->GetStringBounds(buttonPtr->text32).size.y - fontUnderOffset) / 2 };
		buttonPtr->textObj.string32 = buttonPtr->text32;
	}
	DrawRectDecal(buttonPtr->pos, buttonPtr->size, olc::GREY);
	olc::vi2d posButtonCenter = buttonPtr->pos + (buttonPtr->size / olc::vi2d(2, 2));
	DrawDecal(posButtonCenter - buttonPtr->textObj.posOffset, buttonPtr->textObj.renderable.Decal());
}

void DragonWx::RenderButton(buttonStruct* buttonPtr, std::string newButtonText)
{
	buttonPtr->text32 = std::u32string(newButtonText.begin(), newButtonText.end()) + U" ";
	RenderButton(buttonPtr);
}

void DragonWx::RenderButtonWithEnabler(buttonStruct* buttonPtr)
{
	if (buttonPtr->isEnabled)
		buttonPtr->textObj.renderable = buttonPtr->fontPtr->RenderStringToRenderable(buttonPtr->text32, buttonPtr->textColor);
	else
		buttonPtr->textObj.renderable = buttonPtr->fontPtr->RenderStringToRenderable(buttonPtr->text32, olc::GREY);
	RenderButton(buttonPtr);
}

void DragonWx::RenderInputBox(inputBoxStruct* inputBoxPtr, olc::Font* fontToUse, olc::Pixel textColor)
{
	std::u32string startText32 = ConvertedString32(inputBoxPtr->value.text);
	olc::vi2d posTextOffset = { 8, (inputBoxPtr->value.size.y - fontToUse->GetStringBounds(U"d").size.y) / 2};

	if (inputBoxPtr->isEnabled)
	{
		FillRectDecal(inputBoxPtr->value.pos, inputBoxPtr->value.size, olc::WHITE);
		// Check if this Input Box is currently active for Text Entry
		if (IsTextEntryEnabled() && (inputBoxPtr == setupActiveInputBoxPtr))
		{
			olc::vi2d posTextOffset = { 8, (inputBoxPtr->value.size.y - fontToUse->GetStringBounds(U"d").size.y) / 2 };
			int textCursorOffsetX = 0;
			tempString32 = ConvertedString32(TextEntryGetString().substr(0, TextEntryGetCursor()));
			if (!tempString32.empty())
				textCursorOffsetX = fontToUse->GetStringBounds(tempString32).size.x;
			if (textCursorBlinkState)
				FillRectDecal(inputBoxPtr->value.pos + posTextOffset + olc::vi2d(textCursorOffsetX, 0), { 2, 25 }, olc::RED);
			RenderString(TextEntryGetString(), fontToUse, textColor, &textObjInputBuffer, inputBoxPtr->value.pos + posTextOffset);
		}
		else if (!startText32.empty())
			RenderString32(startText32, fontToUse, textColor, &inputBoxPtr->value.textObj, inputBoxPtr->value.pos + posTextOffset, true);
	}
	else
	{
		FillRectDecal(inputBoxPtr->value.pos, inputBoxPtr->value.size, olc::GREY);
		if (!startText32.empty())
			RenderString32(startText32, fontToUse, olc::DARK_GREY, &inputBoxPtr->value.textObj, inputBoxPtr->value.pos + posTextOffset, true);
	}

	DrawRectDecal(inputBoxPtr->value.pos, inputBoxPtr->value.size, olc::BLACK);			// Draw black border around the Input Box
}

void DragonWx::ShowCenteredDialogBox(dialogBox* dialogBoxPtr)
{
	positionTemp = GetCenteredStartPosition(GetScreenSize(), dialogBoxPtr->size);

	// Measure the total height of all defined lines of text and then add padding to it to calculate initial position of the first centered line of text
	int totalTextHeight = 0;
	for (int i = 0; i < 3; i++)
		if (!dialogBoxPtr->textArray[i].empty())
			totalTextHeight += fontSize28.GetStringBounds(ConvertedString32(dialogBoxPtr->textArray[i])).size.y + 5;

	olc::vi2d positionBoxCenter = (GetScreenSize() / olc::vi2d(2, 2)) - olc::vi2d(0, totalTextHeight / 2);
	FillRectDecal(positionTemp, dialogBoxPtr->size, olc::GREY);
	DrawRectDecal(positionTemp, dialogBoxPtr->size, olc::BLACK);
	DrawRectDecal(positionTemp + olc::vi2d(6, 6), dialogBoxPtr->size - olc::vi2d(12, 12), olc::BLACK);
	DrawRectDecal(positionTemp + olc::vi2d(7, 7), dialogBoxPtr->size - olc::vi2d(14, 14), olc::BLACK);
	RenderStringCentered(dialogBoxPtr->textArray[0], &fontSize28, olc::BLACK, &dialogBoxPtr->textObjArray[0], positionBoxCenter);
	if (!dialogBoxPtr->textArray[1].empty())
		RenderString(dialogBoxPtr->textArray[1], &fontSize28, olc::BLACK, &dialogBoxPtr->textObjArray[1], dialogBoxPtr->textObjArray[0].posOffset + olc::vi2d(0, 35));
	if (!dialogBoxPtr->textArray[2].empty())
		RenderString(dialogBoxPtr->textArray[2], &fontSize28, olc::BLACK, &dialogBoxPtr->textObjArray[2], dialogBoxPtr->textObjArray[1].posStart + olc::vi2d(0, 35));
}

void DragonWx::RenderCenteredWxCondition(olc::vf2d centeredPos, olc::Decal* decalToDraw, textObject* textObj)
{
	DrawDecal(centeredPos - olc::vf2d(decalToDraw->sprite->width / 2, 0), decalToDraw);
	if (webWxCurrentConditions.code != -1)
		RenderStringCentered(webWxCurrentConditions.description, &fontSize18, olc::WHITE, textObj, centeredPos + olc::vf2d(0, decalToDraw->sprite->height + 6));
}

void DragonWx::RenderCenteredWxForecast(olc::vf2d centeredPos, olc::Decal* decalToDraw, wxWebEntry* wxWebEntryPtr, textObject* textObj)
{
	DrawDecal(centeredPos - olc::vf2d(decalToDraw->sprite->width / 2, 0), decalToDraw);
	if (wxWebEntryPtr->code != -1)
	{
		int forecastHigh = std::round(wxWebEntryPtr->tempMax);
		int forecastLow = std::round(wxWebEntryPtr->tempMin);
		std::string strForecastPrecip = std::to_string(wxWebEntryPtr->precipPercent) + "%";
		std::string strForecastSegment = std::to_string(forecastHigh) + " / " + std::to_string(forecastLow) + " (";
		std::string strForecastSuffix = ")";
		positionTemp = { 0 - (fontSize18.GetStringBounds(ConvertedString32(strForecastSegment + strForecastPrecip + strForecastSuffix)).size.x / 2), decalToDraw->sprite->height + 6 };
		positionTemp = RenderStringSegment(strForecastSegment, &fontSize18, olc::WHITE, &textObj[0], centeredPos + positionTemp, 2);
		positionTemp = RenderStringSegment(strForecastPrecip, &fontSize18, rainGaugeBorderColor, &textObj[1], positionTemp, 2);
		RenderString(strForecastSuffix, &fontSize18, olc::WHITE, &textObj[2], positionTemp);
		//RenderStringCentered(strForecastInfo, &fontSize16, olc::WHITE, textObj, centeredPos + olc::vf2d(0, decalToDraw->sprite->height + 6));
	}
}

void DragonWx::UpdateAreaBordersSprite()
{
	olc::Sprite* prevDrawTarget = GetDrawTarget();
	SetDrawTarget(renderableAreaBorders.Sprite());
	Clear(olc::BLANK);
	DrawBoxTitle(&titleBoxOutdoorPanel, &renderableAreaBorders);
	DrawBoxTitle(&titleBoxIndoorPanel, &renderableAreaBorders);
	DrawBoxTitle(&titleBoxRainPanel, &renderableAreaBorders);
	DrawBoxTitle(&titleBoxSensorPanel, &renderableAreaBorders);
	// If Web Forecast panel is active, we need to shift the wind compass (and other linked assets) up higher, otherwise use actual center of screen
	if (webWxEnabled)
	{
		DrawBoxTitle(&titleBoxForecastPanel, &renderableAreaBorders);
		windCircleCenterPoint = { positionWindowCenter.x, positionWindowCenter.y - (GetWindowSize().y * 0.06f) };
	}
	else
		windCircleCenterPoint = positionWindowCenter;
	posWindDirectionText = windCircleCenterPoint - olc::vi2d(0, 92);
	positionWindSpeedAvgLabel = windCircleCenterPoint + olc::vf2d(windCircleRadius * -1.2f, windCircleRadius * 1.1875f);
	positionWindSpeedPeakLabel = windCircleCenterPoint + olc::vf2d(windCircleRadius * 0.7f, windCircleRadius * 1.1875f);
	DrawCircle(windCircleCenterPoint, windCircleRadius, rainGaugeBorderColor);
	renderableAreaBorders.Decal()->Update();
	SetDrawTarget(prevDrawTarget);
}

olc::Decal* DragonWx::UpdateTrendData(std::deque<float>* sourceDeque, float dataValue, olc::Decal* decalTarget, std::string debugTextLabel) const
{
	if (sourceDeque->size() >= trendSampleSize)
	{
		sourceDeque->pop_front();
		sourceDeque->push_back(dataValue);

		float trendSlope = CalculateTrendSlope(sourceDeque);
		// DEBUG: Show all the samples
		PRINT_DEBUG("%s \x1b[1;34m%s Sample Points: (0,%.1f)", GetTimestamp().c_str(), debugTextLabel.c_str(), sourceDeque->at(0));
		for (int i = 1; i < trendSampleSize; i++)
			PRINT_DEBUG(", (%u,%.1f)", i, sourceDeque->at(i));
		//PRINT_DEBUG("\n\x1b[0m");
		PRINT_DEBUG("%s \n\x1b[1;32m%s Trend Slope: %.3f\n\x1b[0m", GetTimestamp().c_str(), debugTextLabel.c_str(), trendSlope);

		if (trendSlope >= 0.02)
			return renderableTrendArrowUp.Decal();
		else if (trendSlope <= -0.02)
			return renderableTrendArrowDown.Decal();
		else
			return renderableTrendArrowSteady.Decal();
	}
	else
	{
		sourceDeque->push_back(dataValue);
		PRINT_DEBUG("%s \x1b[1;31m%s Trend Sample Size: %lu\n\x1b[0m", GetTimestamp().c_str(), debugTextLabel.c_str(), sourceDeque->size());
	}

	// Return the original pointer, unchanged
	return decalTarget;
}

std::string DragonWx::GetWindDirectionName(double windDirDegrees)
{
	if ((windDirDegrees >= 348.75) || (windDirDegrees < 11.25))
		return "N";
	else if ((windDirDegrees >= 11.25) && (windDirDegrees < 33.75))
		return "NNE";
	else if ((windDirDegrees >= 33.75) && (windDirDegrees < 56.25))
		return "NE";
	else if ((windDirDegrees >= 56.25) && (windDirDegrees < 78.75))
		return "ENE";
	else if ((windDirDegrees >= 78.75) && (windDirDegrees < 101.25))
		return "E";
	else if ((windDirDegrees >= 101.25) && (windDirDegrees < 123.75))
		return "ESE";
	else if ((windDirDegrees >= 123.75) && (windDirDegrees < 146.25))
		return "SE";
	else if ((windDirDegrees >= 146.25) && (windDirDegrees < 168.75))
		return "SSE";
	else if ((windDirDegrees >= 168.75) && (windDirDegrees < 191.25))
		return "S";
	else if ((windDirDegrees >= 191.25) && (windDirDegrees < 213.75))
		return "SSW";
	else if ((windDirDegrees >= 213.75) && (windDirDegrees < 236.25))
		return "SW";
	else if ((windDirDegrees >= 236.25) && (windDirDegrees < 258.75))
		return "WSW";
	else if ((windDirDegrees >= 258.75) && (windDirDegrees < 281.25))
		return "W";
	else if ((windDirDegrees >= 281.25) && (windDirDegrees < 303.75))
		return "WNW";
	else if ((windDirDegrees >= 303.75) && (windDirDegrees < 326.25))
		return "NW";
	else if ((windDirDegrees >= 326.25) && (windDirDegrees < 348.75))
		return "NNW";

	return "";
}

bool DragonWx::NextWindDirAnimationPoint(float& currentValue, float targetValue, float& currentVelocity, float& timeSinceLastUpdate, float fElapsedTimePGE)
{
	float maxVelocity = 3.0f;
	
	timeSinceLastUpdate += fElapsedTimePGE;
	if (timeSinceLastUpdate >= (1.0f / 60.0f))
	{
		if (currentVelocity < maxVelocity)
			currentVelocity = std::min(currentVelocity + 0.1f, maxVelocity);

		float signedCurrentValue = (currentValue >= 180.0f) ? (currentValue - 360.0f) : currentValue;
		currentValue = NormalizedAngle(signedCurrentValue + currentVelocity);

		timeSinceLastUpdate = 0.0f;
	}
	return false;
}

// Normalize angle between 0 and 360 degrees
float DragonWx::NormalizedAngle(float angle)
{
	if (angle < 0.0f)
		angle += 360.0f;
	else if	(angle >= 360.0f)
		angle -= 360.0f;
	return angle;
}

// Find the shortest signed difference between two angles
float DragonWx::shortest_angle_diff(float from, float to)
{
	float diff = NormalizedAngle(to) - NormalizedAngle(from);
	if (diff > 180.0f)
		diff -= 360.0f;
	if (diff < -180.0f)
		diff += 360.0f;
	return diff;
}

// Simulate a spring-damper to smoothly animate the weather vane
float DragonWx::update_weather_vane_spring(float current, float target, float& velocity, float stiffness, float damping, float dt) {
	float diff = shortest_angle_diff(current, target);

	// Spring force (wants to close the distance)
	float spring_force = stiffness * diff;

	// Damping force (opposes velocity)
	float damping_force = -damping * velocity;

	// Net torque applied to vane
	float torque = spring_force + damping_force;

	// Update velocity
	velocity += torque * dt;

	// Update position
	current = NormalizedAngle(current + velocity * dt);

	return current;
}

// Main function to update the angle
float DragonWx::update_weather_vane(float current, float target, float& velocity, float max_speed, float acceleration, float dt) {
	// Calculate shortest path to target
	float diff = shortest_angle_diff(current, target);

	// Determine if we are in overshoot phase
	static bool overshooting = false;
	static float overshoot_target = 0.0f;

	if (!overshooting && std::fabs(diff) < 5.0f) { // Close enough, prepare for overshoot
		overshooting = true;
		overshoot_target = NormalizedAngle(target + (diff > 0 ? 10.0f : -10.0f)); // 10 degrees overshoot
	}

	float desired_target = overshooting ? overshoot_target : target;
	float desired_diff = shortest_angle_diff(current, desired_target);

	// Decide on acceleration/deceleration
	float desired_speed = max_speed;

	// Start decelerating when close to target
	if (std::fabs(desired_diff) < 45.0f) {
		desired_speed = max_speed * (std::fabs(desired_diff) / 45.0f); // linear deceleration
	}

	// Accelerate or decelerate toward desired speed
	if (velocity < desired_speed)
		velocity = std::min(velocity + acceleration * dt, desired_speed);
	else if (velocity > desired_speed)
		velocity = std::max(velocity - acceleration * dt, desired_speed);

	// Move in the correct direction
	float move = velocity * dt * (desired_diff > 0 ? 1.0f : -1.0f);

	// If we would overshoot the desired target in this frame, clamp to target
	if (std::fabs(move) > std::fabs(desired_diff))
		move = desired_diff;

	current = NormalizedAngle(current + move);

	// If we are overshooting and reach the overshoot target, switch back to real target
	if (overshooting && std::fabs(shortest_angle_diff(current, overshoot_target)) < 1.0f) {
		overshooting = false;
	}

	return current;
}

void DragonWx::MidnightDailyReset()
{
	outdoorSensor.temperature.Reset();
	outdoorSensor.humidity.Reset();

	indoorSensor.temperature.Reset();
	indoorSensor.humidity.Reset();

	rainfallTotalToday.SetZero();
	if (!activeRainfallEvent)
	{
		rainEventStartTime = 0;
		rainEventStopTime = 0;
	}
}

bool DragonWx::LoadWebWxAssets(wxWebEntry* wxDataEntry, olc::Decal* decalTarget)
{
	std::string	pathToIcon = "./Images/Meteocons/Icons/";
	if (wxDataEntry->useDaytime)
	{
		pathToIcon += wxCodeTableNew[wxDataEntry->code].iconFileDay;
		wxDataEntry->description = wxCodeTableNew[wxDataEntry->code].descriptionDay;
	}
	else
	{
		pathToIcon += wxCodeTableNew[wxDataEntry->code].iconFileNight;
		wxDataEntry->description = wxCodeTableNew[wxDataEntry->code].descriptionNight;
	}
	pathToIcon += ".png";
	if (decalTarget->sprite->LoadFromFile(pathToIcon) != 1)
		return false;
	decalTarget->Update();
	return true;
}
/*
bool DragonWx::SaveConfigFile()
{
	std::ofstream configFile("DragonWx.conf");
	if (!configFile.is_open())
	{
		PRINT_DEBUG("Error: Could not open config file for writing.\n");
		return false;
	}

	configFile << "DragonWx Config File v1.0" << std::endl << std::endl;

	configFile << "RTL433_PATH=" << pathToExec << std::endl;
	configFile << "RTL433_PARAMS=" << sdrExtraArguments << std::endl;
	configFile << "SDR_GAIN=" << sdrGainSetting << std::endl;
	configFile << "SDR_ANTENNA=" << sdrAntennaSetting << std::endl << std::endl;

	configFile << "OUTDOOR_SENSOR_ID=" << outdoorSensor.ID << std::endl;
	configFile << "OUTDOOR_TEMP_OFFSET_C=" << outdoorSensor.tempOffset.GetValue(metricUnits) << std::endl;		// Always store the calibration value in Metric Celsius
	configFile << "OUTDOOR_HUMIDITY_OFFSET=" << outdoorSensor.humidityOffset << std::endl << std::endl;

	configFile << "INDOOR_SENSOR_ID=" << indoorSensor.ID << std::endl;
	configFile << "INDOOR_TEMP_OFFSET_C=" << indoorSensor.tempOffset.GetValue(metricUnits) << std::endl;		// Always store the calibration value in Metric Celsius
	configFile << "INDOOR_HUMIDITY_OFFSET=" << indoorSensor.humidityOffset << std::endl << std::endl;

	configFile << "FULLSCREEN=" << fullscreenToggle << std::endl;
	configFile << "STATION_NAME=" << strWxStationName << std::endl;
	configFile << "UNITS=" << useMetricUnits << std::endl << std::endl;

	configFile << "USE_WEB_FORECAST=" << webWxEnabled << std::endl;
	configFile << "LOCATION_LAT=" << webWxLocationLat << std::endl;
	configFile << "LOCATION_LON=" << webWxLocationLon << std::endl;

	configFile.close();

	return true;
}
*/