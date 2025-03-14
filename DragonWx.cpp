
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

	spriteLayers[0] = CreateLayer();
	spriteLayers[1] = CreateLayer();

	outdoorSensor.ID = "774";
	outdoorSensor.recentlyUpdated = false;
	outdoorSensor.packetCounter = -1;
	outdoorSensor.batteryStatus = undefinedValue;

	indoorSensor.ID = "7594";
	indoorSensor.recentlyUpdated = false;
	indoorSensor.packetCounter = -1;
	indoorSensor.batteryStatus = undefinedValue;

	if (!useRealPipe)
		populateTestData();
	else
		dequeWindDirections.push_front(0.0f);		// Set initial wind arrow direction to North

	if (!useRealLocationInfo)
		//strLocationURL = "https://wttr.in/lincoln,rhode%20island?format=j1";
		strLocationURL = "https://api.open-meteo.com/v1/forecast?latitude=41.8907&longitude=-71.3923&current=temperature_2m,is_day,weather_code,surface_pressure&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_probability_max&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch&timezone=auto&forecast_days=3&timeformat=unixtime";

	colorLabelText = { 48, 139, 151 };
	rainGaugeBorderColor = { 53, 157, 242 };
	areasBorderColor = { 100, 100, 100 };

	rainGaugeFilledPercentage = 0.0f;
	rainGaugeFilledHeight = (rainGaugeTotalHeight * rainGaugeFilledPercentage);

	positionWindowCenter = { float(GetWindowSize().x / 2.0f), float(GetWindowSize().y / 2.0f) };
	printf("Window Center: x = %f, y = %f\n", positionWindowCenter.x, positionWindowCenter.y);
	// Called once at the start, so create things here
	olc::Font::init();

	fontSize18 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 18);
	fontSize22 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 22);
	fontSize24 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 24);
	fontSize32 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 32);
	fontSize40 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 40);
	fontSize56 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 56);
	fontSize72 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 72);
	fontSize96 = olc::Font("./Fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf", 96);

	spacerFontSize18 = fontSize18.GetStringBounds(U"-").size.x;
	spacerFontSize24 = fontSize24.GetStringBounds(U"-").size.x;
	spacerFontSize32 = fontSize32.GetStringBounds(U"-").size.x;

	textWindSpeedUnits = "mph";

	printf("Testing\n");
	backgroundSprite = new olc::Sprite("./images/gradient_3840x2160.png");
	backgroundDecal = new olc::Decal(backgroundSprite);
	backgroundAlphaSprite = new olc::Sprite("./images/transparent_1280x720.png");
	backgroundAlphaDecal = new olc::Decal(backgroundAlphaSprite);

	spriteSettingsScreen = new olc::Sprite(GetWindowSize().x, GetWindowSize().y);
	decalSettingsScreen = new olc::Decal(spriteSettingsScreen);

	spriteThermometer = new olc::Sprite("./images/thermometer_32.png");
	decalThermometer = new olc::Decal(spriteThermometer);
	spriteWaterDrop = new olc::Sprite("./images/drop_32.png");
	decalWaterDrop = new olc::Decal(spriteWaterDrop);
	spriteRainfallIcon = new olc::Sprite("./images/raincloud_32.png");
	decalRainfallIcon = new olc::Decal(spriteRainfallIcon);

	//spriteWindDir = new olc::Sprite("./images/WindArrow256x256.png");
	spriteWindDir = new olc::Sprite(46, 46);
	SetDrawTarget(spriteWindDir);
	Clear(olc::BLANK);
	FillTriangle({ 0, 0 }, { spriteWindDir->Size().x, 0 }, { spriteWindDir->Size().x / 2, spriteWindDir->Size().y }, olc::WHITE);
	decalWindDir = new olc::Decal(spriteWindDir);
	centerPointWindDir = { float(spriteWindDir->width) / 2.0f, float(spriteWindDir->height / 2.0f) };

	//spriteWindCircle = new olc::Sprite(258, 258);
	//SetDrawTarget(spriteWindCircle);
	//Clear(olc::BLANK);
	//DrawCircle({ spriteWindCircle->Size().x / 2, spriteWindCircle->Size().y / 2 }, 128, rainGaugeBorderColor);
	//decalWindCircle = new olc::Decal(spriteWindCircle);

	//spriteRainGaugeBorder = new olc::Sprite("./images/RainGaugeBorder.png");
	//decalRainGaugeBorder = new olc::Decal(spriteRainGaugeBorder);
	spriteRainGauge = new olc::Sprite("./images/RainGaugeWater2.png");
	decalRainGauge = new olc::Decal(spriteRainGauge);
	spriteRainGaugeClear = new olc::Sprite(spriteRainGauge->width, spriteRainGauge->height);

	//spriteSignalStrength = new olc::Sprite("C:\\Users\\TekTodd\\Desktop\\signal1.png");
	std::string filePathSignalIcon;
	for (int i = 0; i < 5; i++)
	{
		filePathSignalIcon = "./images/signal_only" + std::to_string(i) + ".png";
		spriteSignalStrength[i] = new olc::Sprite(filePathSignalIcon);
		decalSignalStrength[i] = new olc::Decal(spriteSignalStrength[i]);
	}

	// Load and setup the sprites/decals for the trending arrows (up, down, and steady)
	spriteTrendArrowUp = new olc::Sprite("./images/TrendUpWhite.png");
	decalTrendArrowUp = new olc::Decal(spriteTrendArrowUp);
	spriteTrendArrowSteady = new olc::Sprite("./images/TrendSteadyWhite.png");
	decalTrendArrowSteady = new olc::Decal(spriteTrendArrowSteady);
	spriteTrendArrowDown = new olc::Sprite("./images/TrendDownWhite.png");
	decalTrendArrowDown = new olc::Decal(spriteTrendArrowDown);

	spriteSettingsIcon = new olc::Sprite("./images/gear3.png");
	decalSettingsIcon = new olc::Decal(spriteSettingsIcon);

	spriteWebConditionsImage = new olc::Sprite();
	decalWebConditionsImage = new olc::Decal(spriteWebConditionsImage);

	for (int i = 0; i < 3; i++)
	{
		spriteWebForecastImages[i] = new olc::Sprite();
		decalWebForecastImages[i] = new olc::Decal(spriteWebForecastImages[i]);
	}

	strFeelsLikeLabel = U"Feels Like";

	screenPaddingOffsetY = GetWindowSize().y * 0.04f;
	outdoorAreaStartScaleX = 0.03125f;
	//outdoorAreaStartScaleY = 0.08333f;
	outdoorAreaStartScaleY = 0.07f;
	//outdoorAreaSizeScaleX = 0.265625f;
	//outdoorAreaSizeScaleX = 0.28f;
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

	positionRainAreaStart = { positionSensorAreaStart.x, positionOutdoorAreaStart.y };
	positionRainAreaSize = { positionSensorAreaSize.x, GetWindowSize().y * 0.63f };

	olc::vf2d positionOutdoorTempOffset = positionOutdoorAreaSize * olc::vf2d(0.62f, 0.14468f);
	olc::vf2d positionOutdoorHumidityOffset = positionOutdoorAreaSize * olc::vf2d(0.62f, 0.60764f);
	olc::vf2d positionOutdoorLowLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.35f, 0.260412f);	// Target distance of 110 pixels between High/Low
	olc::vf2d positionOutdoorHighLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.02f, 0.260412f);

	//positionOutdoorTrendOffset = positionOutdoorAreaSize * olc::vf2d(0.18f, 0.115f);
	float trendOffsetY = (fontSize96.GetStringBounds(U"0").size.y / 2.0f) - ((spriteTrendArrowSteady->height * 0.29f) / 2.0f);
	positionOutdoorTrendOffset = { positionOutdoorAreaSize.x * 0.18f, trendOffsetY };

	positionOutdoorTempValueF = positionOutdoorAreaStart + positionOutdoorTempOffset;
	positionOutdoorTempLowValue = positionOutdoorTempValueF + positionOutdoorLowLabelOffset;
	positionOutdoorTempHighValue = positionOutdoorTempValueF + positionOutdoorHighLabelOffset;

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


	positionInfoAreaSize = { GetWindowSize().x * 0.30f, positionSensorAreaSize.y };
	positionInfoAreaStart = { positionWindowCenter.x - (positionInfoAreaSize.x / 2), positionIndoorAreaStart.y + positionIndoorAreaSize.y - positionInfoAreaSize.y };
	positionInfoAreaGearIcon = positionInfoAreaStart + (positionInfoAreaSize * olc::vf2d(0.90f, 0.50f)) - (spriteSettingsIcon->Size() / olc::vi2d(2, 2));

	infoAreaDividerX = positionInfoAreaStart.x + (positionInfoAreaSize.x * 0.3f);
	infoAreaDividerStartY = positionInfoAreaStart.y + (positionInfoAreaSize.y * 0.23f);
	infoAreaDividerEndY = positionInfoAreaStart.y + positionInfoAreaSize.y - (positionInfoAreaSize.y * 0.2f);

	windCircleCenterPoint = { positionWindowCenter.x, positionWindowCenter.y - (GetWindowSize().y * 0.06f) };
	windCircleRadius = 128.0f;
	printf("Debug: Wind Circle Radius = %.1f\n", windCircleRadius);
	//positionWindSpeedAvgLabel = { 545, (windCircleRadius * 1.1875f) + windCircleCenterPoint.y };
	//positionWindSpeedHighLabel = { 730, (windCircleRadius * 1.1875f) + windCircleCenterPoint.y }; 
	olc::vf2d positionWindAvgOffset = { windCircleRadius * -1.2f, windCircleRadius * 1.1875f };
	olc::vf2d positionWindHighOffset = { windCircleRadius * 0.7f, windCircleRadius * 1.1875f };
	positionWindSpeedAvgLabel = windCircleCenterPoint + positionWindAvgOffset;
	positionWindSpeedHighLabel = windCircleCenterPoint + positionWindHighOffset;

	rainGaugeTotalSize = { rainGaugeTotalWidth, rainGaugeTotalHeight };

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
	printf("Debug: Outdoor Area Start Coords = %.1f, %.1f\n", positionOutdoorAreaStart.x, positionOutdoorAreaStart.y);
	printf("Debug: Outdoor Area Size = %.1f, %.1f\n", positionOutdoorAreaSize.x, positionOutdoorAreaSize.y);
	printf("Debug: Outdoor High/Low Offset = %.1f, %.1f\n", positionOutdoorHighLabelOffset.x, positionOutdoorHighLabelOffset.y);
	printf("Debug: Outdoor Temp Value Coords = %.1f, %.1f\n", positionOutdoorTempValueF.x, positionOutdoorTempValueF.y);
	printf("Debug: Outdoor Temp Low Coords = %.1f, %.1f\n", positionOutdoorTempLowValue.x, positionOutdoorTempLowValue.y);
	printf("Debug: Outdoor Temp High Coords = %.1f, %.1f\n", positionOutdoorTempHighValue.x, positionOutdoorTempHighValue.y);
	printf("Debug: Outdoor Humidity Value Coords = %.1f, %.1f\n", positionOutdoorHumidityValue.x, positionOutdoorHumidityValue.y);
	printf("Debug: Outdoor Humidity Low Coords = %.1f, %.1f\n", positionHumidityLowValue.x, positionHumidityLowValue.y);
	printf("Debug: Outdoor Humidity High Coords = %.1f, %.1f\n", positionHumidityHighValue.x, positionHumidityHighValue.y);
	printf("Debug: Outdoor Trend Arrow Offset = %.1f, %.1f\n", positionOutdoorTrendOffset.x, positionOutdoorTrendOffset.y);

	windSpeedHighValueMPH = 0.0;
	windSpeedAvgValueMPH = 0.0;

	renderableLowLabel18 = fontSize18.RenderStringToRenderable(U"Low ", colorLabelText);
	renderableHighLabel18 = fontSize18.RenderStringToRenderable(U"High ", colorLabelText);
	renderableLowLabel24 = fontSize24.RenderStringToRenderable(U"Low ", colorLabelText);
	renderableHighLabel24 = fontSize24.RenderStringToRenderable(U"High ", colorLabelText);

	SetDrawTarget(spriteSettingsScreen);
	Clear(olc::BLANK);
	decalSettingsScreen->Update();

	SetDrawTarget(spriteRainGaugeClear);
	Clear(olc::BLANK);

	SetDrawTarget(backgroundAlphaSprite);
	positionOutdoorTitle = DrawBoxTitle(strPanelNameOutdoor32, &fontSize40, positionOutdoorAreaStart, positionOutdoorAreaSize, 20, areasBorderColor);
	positionIndoorTitle = DrawBoxTitle(strPanelNameIndoor32, &fontSize32, positionIndoorAreaStart, positionIndoorAreaSize, 20, areasBorderColor);
	positionSensorInfoTitle = DrawBoxTitle(strPanelNameSensors32, &fontSize32, positionSensorAreaStart, positionSensorAreaSize, 20, areasBorderColor);
	positionRainAreaTitle = DrawBoxTitle(strPanelNameRainfall32, &fontSize32, positionRainAreaStart, positionRainAreaSize, 20, areasBorderColor);
	positionInfoAreaTitle = DrawBoxTitle(strPanelNameConditions32, &fontSize32, positionInfoAreaStart, positionInfoAreaSize, 20, areasBorderColor);
	sensorAreaDividerX = positionSensorAreaStart.x + (positionSensorAreaSize.x / 2);
	int sensorAreaDividerStartY = positionSensorAreaStart.y + (positionSensorAreaSize.y * 0.18f);
	int sensorAreaDividerEndY = positionSensorAreaStart.y + positionSensorAreaSize.y - (positionSensorAreaSize.y * 0.18f);
	DrawLine(sensorAreaDividerX, sensorAreaDividerStartY, sensorAreaDividerX, sensorAreaDividerEndY, areasBorderColor);

	//olc::vf2d positionLeftDividerStart = { positionLeftSideDivider.x + (positionOutdoorAreaSize.x * 0.06f), positionLeftSideDivider.y };
	//olc::vf2d positionLeftDividerEnd = { positionLeftSideDivider.x + positionOutdoorAreaSize.x - (positionOutdoorAreaSize.x * 0.06f), positionLeftSideDivider.y};
	//DrawLine(positionLeftDividerStart, positionLeftDividerEnd, areasBorderColor);
	//DrawLineDecal(positionLeftDividerStart, positionLeftDividerEnd, areasBorderColor);

	DrawCircle(windCircleCenterPoint, windCircleRadius, rainGaugeBorderColor);


	//DrawCircle(positionWindowCenter, 129, rainGaugeBorderColor);

	//DrawWindDirPreviousArc(positionWindowCenter, 120, olc::RED, 0xFF);
	//DrawWindDirPreviousArc(positionWindowCenter, 119, olc::RED, 0xFF);
	//DrawWindDirPreviousArc(positionWindowCenter, 118, olc::RED, 0xFF);
	//DrawCircle(positionWindowCenter, 117, olc::RED, 0x80);


	backgroundAlphaDecal->Update();

	for (int x = 0; x < trendSampleSize; x++)
	{
		xSum += x;
		xSumSquare += (x * x);
	}
	sumsBottomEquation = (trendSampleSize * xSumSquare) - (xSum * xSum);

	//for (int i = 0; i < 30; i++)
	//	dequeOutdoorTemps.push_back(tempData[i]);

	//printf("Test Slope = %f\n", CalculateTrendSlope(&dequeOutdoorTemps));

	decalTrendOutdoorTemp = decalTrendArrowSteady;
	decalTrendOutdoorHumidity = decalTrendArrowSteady;

	systemTimePrevious = std::time(nullptr);	// Set initial value for previous timestamp

	printf("Pixel Value = (%u, %u, %u, %u)\n", olc::GREY.r, olc::GREY.g, olc::GREY.b, olc::GREY.a);

	std::srand(std::time(nullptr));  // Seed based on current time

	return true;
}

bool DragonWx::OnUserUpdate(float fElapsedTime)
{
	if (GetKey(olc::Key::SPACE).bPressed)
		debugKeyPressed = !debugKeyPressed;

	if (GetMouse(olc::Mouse::LEFT).bPressed)
	{
		olc::vf2d positionMouseCursor = GetMousePos();
		if (positionMouseCursor.y < screenPaddingOffsetY)
		{
			fullscreenToggle = !fullscreenToggle;
			return false;
		}
		else if (mouseWithinArea(positionMouseCursor, positionInfoAreaGearIcon, spriteSettingsIcon->Size()))
			debugKeyPressed = !debugKeyPressed;
		else if (mouseWithinArea(positionMouseCursor, positionFeelsLikeLabel, fontSize32.GetStringBounds(strFeelsLikeLabel).size))
			useFeelsLikeLabel = !useFeelsLikeLabel;
		else if (lightLevelLux.current != -1)
		{
			if (mouseWithinArea(positionMouseCursor, positionLightLevelValue, fontSize24.GetStringBounds(strLightLevelValue).size) ||
				mouseWithinArea(positionMouseCursor, positionLightLevelLabel, fontSize24.GetStringBounds(U"Light").size))
				useLuxValue = !useLuxValue;
		}
	}


	if (GetKey(olc::Key::Q).bHeld)
	{
		appExitRequested = true;
		return false;
	}

	if (GetKey(olc::Key::R).bPressed && !prevKeyPressed)
	{
		std::cout << "Sending Web Wx Request to wttr.in... " << strLocationURL << std::endl;
		webWxRequested = true;

		//debugKeyPressed = !debugKeyPressed;
		//rainfallDataValueInches.current += 0.01;
		/*
		if ((rainfallTotalTodayValue / rainGaugeCapacityIn) >= 0.90)
			rainGaugeCapacityIn += 1.0;
		rainGaugeFilledPercentage = rainfallTotalTodayValue / rainGaugeCapacityIn;
		*/
		prevKeyPressed = true;
		//debugState = true;
		//printf("Debug: Rainfall = %.2f, Filled Percentage = %.2f\n", rainfallTotalTodayValue, rainGaugeFilledPercentage);
	}
	else if (!GetKey(olc::Key::R).bPressed)
		prevKeyPressed = false;

	elapsedTimeCounter += fElapsedTime;
	animationElapsedTime += fElapsedTime;

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
			printf("%s Debug: New random wind value = %.0f\n", GetTimestamp(), newRandomWindDirection);
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
				printf("%s Debug: Moving from %.0f to %.0f\n", GetTimestamp(), windDirAnimatedPosition, newRandomWindDirection);
			}
		}

		if (outdoorSensor.recentlyUpdated)
		{
			if (outdoorSensor.packetCounter < 4)
				outdoorSensor.packetCounter++;

			minuteTimeCounter++;
			if (minuteTimeCounter >= 4)			// Effectively manages a 1 minute timer out of the 15 second one
			{
				minuteTimeCounter = 0;
				decalTrendOutdoorTemp = UpdateTrendData(&dequeOutdoorTemps, outdoorTempValueF.current, decalTrendOutdoorTemp, "Outdoor Temp");
				decalTrendOutdoorHumidity = UpdateTrendData(&dequeOutdoorHumidity, outdoorHumidityValue.current, decalTrendOutdoorHumidity, "Outdoor Humidity");
			}

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
		printf("%s Outdoor Sensor: Consecutive packets = %u\n", GetTimestamp(), outdoorSensor.packetCounter);
		printf("%s Indoor Sensor: Consecutive packets = %u\n", GetTimestamp(), indoorSensor.packetCounter);
	}

	// Render blank background image
	DrawDecal({ 0.0f, 0.0f }, backgroundDecal, { 0.333f, 0.333f });

	if (debugKeyPressed)
	{
		olc::vf2d boxSize = { 600, 400 };
		olc::vf2d positionSettingsBoxTitle;
		SetDrawTarget(spriteSettingsScreen);
		positionSettingsBoxTitle = DrawBoxTitle(U"DragonWx Settings", &fontSize32, GetCenteredStartPosition(GetWindowSize(), boxSize), boxSize, 20, areasBorderColor);
		decalSettingsScreen->Update();
		DrawDecal({ 0, 0 }, decalSettingsScreen);
		RenderString32(U"DragonWx Settings", &fontSize32, olc::GREY, &renderableSettingsLabel, positionSettingsBoxTitle);


		return true;
	}

	DrawDecal({ 0, 0 }, backgroundAlphaDecal);
	RenderString32(strPanelNameOutdoor32, &fontSize40, olc::GREY, &renderableOutdoorLabel, positionOutdoorTitle);
	RenderString32(strPanelNameIndoor32, &fontSize32, olc::GREY, &renderableIndoorLabel, positionIndoorTitle);
	RenderString32(strPanelNameSensors32, &fontSize32, olc::GREY, &renderableSensorInfoLabel, positionSensorInfoTitle);
	RenderString32(strPanelNameRainfall32, &fontSize32, olc::GREY, &renderableRainfallLabel, positionRainAreaTitle);
	RenderString32(strPanelNameConditions32, &fontSize32, olc::GREY, &renderableAppNameLabel, positionInfoAreaTitle);

	//SetDecalMode(olc::DecalMode::WIREFRAME);
	//DrawPolygonDecal(nullptr, { {10,10},{100,10},{100,50},{10,50} },
	//	{ {0,0},{0,0},{0,0},{0,0} }, olc::GREEN);
	//SetDecalMode(olc::DecalMode::NORMAL);

	//DrawDecal({ 900, 300 }, decalTest);


	// Check the current system time and if it has changed since last check, render/display it
	systemTimeNow = std::time(nullptr);
	if (systemTimeNow != systemTimePrevious)
	{
		std::tm systemTimeLocalNow, systemTimeLocalPrevious;

		ConvertTimeToLocal(&systemTimeLocalNow, systemTimeNow);
		ConvertTimeToLocal(&systemTimeLocalPrevious, systemTimePrevious);

		// Check for Midnight rollover to reset statistics, etc
		if ((systemTimeLocalNow.tm_hour == 0) && (systemTimeLocalPrevious.tm_hour != 0))
		{
			MidnightDailyReset();
			printf("%s Time: Midnight rollover\n", GetTimestamp());
		}

		int dateResult = std::strftime(strDateWeekMonthDay, sizeof(strDateWeekMonthDay), "%A, %B %d ", &systemTimeLocalNow);
		int timeResult = std::strftime(strFormattedTime, sizeof(strFormattedTime), "%I:%M %p", &systemTimeLocalNow);
		if (dateResult && timeResult)
		{
			strFullyFormattedDate = strDateWeekMonthDay + std::to_string(systemTimeLocalNow.tm_year + 1900);
			strFullyFormattedTime = strFormattedTime;
			if (strFullyFormattedTime.at(0) == '0')
				strFullyFormattedTime.erase(0, 1);		// Strip off any leading zeros on the hours value
		}
		systemTimePrevious = systemTimeNow;
	}
	RenderStringCentered(strFullyFormattedDate, &fontSize40, olc::WHITE, &renderableDateText, { positionWindowCenter.x, positionOutdoorAreaStart.y });
	RenderStringCentered(strFullyFormattedTime, &fontSize72, olc::WHITE, &renderableTimeText, { positionWindowCenter.x, positionOutdoorAreaStart.y + 50.0f });

	// Display the Thermometer and Water Drop icons in Outdoor area
	olc::vf2d tempPos = { positionOutdoorAreaStart.x + 20, (positionOutdoorTempValueF.y + (fontSize96.GetStringBounds(U"1").size.y / 2)) - (spriteThermometer->height / 2) };
	DrawDecal(olc::vf2d(positionOutdoorAreaStart.x + 20, positionOutdoorTempValueF.y + 10), decalThermometer);
	DrawDecal(olc::vf2d(positionOutdoorAreaStart.x + 20, positionOutdoorHumidityValue.y + 10), decalWaterDrop);

	// Display the temperature
	if (outdoorTempValueF.current != undefinedValue)
	{
		//RenderStringRightJustified(std::format("{:.1f}", outdoorTempValueF.current), &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF);
		tempString = std::format("{:.0f}", outdoorTempValueF.current);
		RenderStringRightJustified(tempString, &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF);
		tempString32 = ConvertedString32(std::format("{:.1f}", outdoorTempValueF.current - int(outdoorTempValueF.current)));
		tempString32.erase(tempString32.begin());
		positionTemp = olc::vf2d(9, fontSize96.GetStringBounds(ConvertedString32(tempString)).size.y - fontSize40.GetStringBounds(tempString32).size.y - 5);
		RenderString32(tempString32, &fontSize40, olc::WHITE, &renderableTempDecimalValue, positionOutdoorTempValueF + positionTemp);
	}
	else
		RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF + positionDashCorrection96 - olc::vf2d(15, 0));
	RenderString32(strDegreesUnitsF, &fontSize32, colorLabelText, &renderableLabelTempF, positionOutdoorTempValueF + olc::vf2d(9, 0));

	// Display the Outdoor Temperature's current 24-hour low/high
	RenderHighOrLowValue("Low", highLowOutdoorTempF.low, &renderableOutdoorTempLowValue, positionOutdoorTempLowValue, false);
	RenderHighOrLowValue("High", highLowOutdoorTempF.high, &renderableOutdoorTempHighValue, positionOutdoorTempHighValue, false);

	// Display the current Outdoor temperature trend arrow
	if (outdoorTempValueF.current != undefinedValue)
		//DrawDecal(positionOutdoorTempValueF + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));
		DrawDecal(positionOutdoorTempValueF + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));

	// Display the Outdoor Humidity
	if (outdoorHumidityValue.current != undefinedValue)
		RenderStringRightJustified(std::format("{:d}", outdoorHumidityValue.current), &fontSize96, olc::WHITE, &renderableHumidityValue, positionOutdoorHumidityValue);
	else
		RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &renderableHumidityValue, positionOutdoorHumidityValue + positionDashCorrection96 - olc::vf2d(15, 0));
	RenderString("%", &fontSize32, colorLabelText, &renderableHumidityUnits, positionOutdoorHumidityValue + olc::vf2d(9, 0));

	// Display the Outdoor Humidity's 24-hour low/high
	RenderHighOrLowValue("Low", highLowOutdoorHumidity.low, &renderableHumidityLowValue, positionHumidityLowValue, false);
	RenderHighOrLowValue("High", highLowOutdoorHumidity.high, &renderableHumidityHighValue, positionHumidityHighValue, false);

	// Display the Outdoor Humidity's current trend arrow
	if (outdoorHumidityValue.current != undefinedValue)
		DrawDecal(positionOutdoorHumidityValue + positionOutdoorTrendOffset, decalTrendOutdoorHumidity, { 0.33f, 0.33f }, olc::Pixel(0, 77, 230));

	// Display the calculated dewpoint
	RenderString("Dewpoint", &fontSize32, olc::GREY, &renderableLabelDewpoint, positionDewPointLabel);
	std::u32string strDewPointValue, strDewPointUnits;
	if (useMetricUnits)
	{
		if (dewpointValueC != undefinedValue)
		{
			strDewPointValue = ConvertedString32(std::format("{:.1f}", dewpointValueC));
			RenderString32(strDewPointValue, &fontSize32, olc::WHITE, &renderableDewpointValue, positionDewPointValue);
		}
		else
			RenderString32(U"-  -", &fontSize32, olc::GREY, &renderableDewpointValue, positionDewPointValue + positionDashCorrection32);
		olc::vf2d positionDewPointUnits = positionDewPointValue + olc::vf2d(fontSize32.GetStringBounds(strDewPointValue).size.x + spacerFontSize24, 0);
		RenderString32(strDegreesUnitsC, &fontSize24, colorLabelText, &renderableDewPointUnits, positionDewPointUnits);
	}
	else
	{
		if (dewpointValueF != undefinedValue)
		{
			strDewPointValue = ConvertedString32(std::format("{:.1f}", dewpointValueF));
			RenderString32(strDewPointValue, &fontSize32, olc::WHITE, &renderableDewpointValue, positionDewPointValue);
			olc::vf2d positionDewPointUnits = positionDewPointValue + olc::vf2d(fontSize32.GetStringBounds(strDewPointValue).size.x + spacerFontSize24, 0);
			RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableDewPointUnits, positionDewPointUnits);
		}
		else
		{
			strDewPointValue = U"-  -";
			RenderString32(strDewPointValue, &fontSize32, olc::GREY, &renderableDewpointValue, positionDewPointValue + positionDashCorrection32);
			olc::vf2d positionDewPointUnits = positionDewPointValue + olc::vf2d(fontSize32.GetStringBounds(strDewPointValue).size.x + spacerFontSize24 + spacerFontSize24, 0);
			RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableDewPointUnits, positionDewPointUnits);
		}
	}

	// Display the "Feels Like" temperature (or the name of the actual method being used if toggled on by user)
	std::u32string strFeelsLikeValue;
	strFeelsLikeLabel = U"Feels Like";
	if ((outdoorTempValueF.current < 50.0) && (windSpeedValueMPH.current >= 3.0))
	{
		if (!useFeelsLikeLabel)
			strFeelsLikeLabel = U"Wind Chill";
		strFeelsLikeValue = ConvertedString32(std::format("{:.1f}", calculatedWindChillF));
	}
	else if ((outdoorTempValueF.current >= 80.0) && (outdoorHumidityValue.current >= 40))
	{
		if (!useFeelsLikeLabel)
			strFeelsLikeLabel = U"Heat Index";
		strFeelsLikeValue = ConvertedString32(std::format("{:.1f}", calculatedHeatIndexF));
	}
	else
	{
		if ((outdoorTempValueF.current >= 50.0) && (outdoorTempValueF.current < 80.0))
		{
			if (!useFeelsLikeLabel)
				strFeelsLikeLabel = U"Apparent";
			strFeelsLikeValue = ConvertedString32(std::format("{:.1f}", calculatedApparentTempF));
		}
		else
		{
			if (!useFeelsLikeLabel)
				strFeelsLikeLabel = U"Actual";
			strFeelsLikeValue = ConvertedString32(std::format("{:.1f}", outdoorTempValueF.current));
		}
	}
	RenderString32(strFeelsLikeLabel, &fontSize32, olc::GREY, &renderableFeelsLikeLabel, positionFeelsLikeLabel);
	if (strFeelsLikeValue != U"300.0")
	{
		RenderString32(strFeelsLikeValue, &fontSize32, olc::WHITE, &renderableFeelsLikeValue, positionFeelsLikeValue);
		olc::vf2d positionFeelsLikeUnits = positionFeelsLikeValue + olc::vf2d(fontSize32.GetStringBounds(strFeelsLikeValue).size.x + spacerFontSize24, 0);
		RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableFeelsLikeUnits, positionFeelsLikeUnits);
	}
	else
	{
		strFeelsLikeValue = U"-  -";
		RenderString32(strFeelsLikeValue, &fontSize32, olc::GREY, &renderableFeelsLikeValue, positionFeelsLikeValue + positionDashCorrection32);
		olc::vf2d positionFeelsLikeUnits = positionFeelsLikeValue + olc::vf2d(fontSize32.GetStringBounds(strFeelsLikeValue).size.x + spacerFontSize24 + spacerFontSize24, 0);
		RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableFeelsLikeUnits, positionFeelsLikeUnits);
	}

	// Display Light Intensity information and UV Index (if applicable to the weather sensor being used)
	if ((uvIndex.current != -1) || (lightLevelLux.current != -1))
	{
		// First draw a dividing line between the above temperature-related info and the light info to come below
		olc::vf2d positionLeftDividerStart = { positionLeftSideDivider.x + (positionOutdoorAreaSize.x * 0.00f), positionLeftSideDivider.y };
		olc::vf2d positionLeftDividerEnd = { positionLeftSideDivider.x + positionOutdoorAreaSize.x - (positionOutdoorAreaSize.x * 0.00f), positionLeftSideDivider.y };
		DrawLineDecal(positionLeftDividerStart, positionLeftDividerEnd, areasBorderColor);

		positionLightInfoNext = positionLeftSideDivider + olc::vf2d(0, 20);		// Set initial position for first light-measurement-related info

		// First show the Light Intensity (if applicable)
		if (lightLevelLux.current != -1)
		{
			positionLightLevelLabel = positionLightInfoNext;
			positionLightLevelValue = positionLightLevelLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);
			RenderString32(U"Light", &fontSize24, olc::GREY, &renderableLightLevelLabel, positionLightLevelLabel);
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
				RenderString32(strLightLevelValue, &fontSize24, olc::WHITE, &renderableLightLevelValue, positionLightLevelValue);
			}
			else
			{
				strLightLevelValue = ConvertedString32(std::to_string(lightLevelLux.current));
				RenderString32(strLightLevelValue, &fontSize24, olc::WHITE, &renderableLightLevelValue, positionLightLevelValue);
				RenderString32(U"lux", &fontSize18, colorLabelText, &renderableLightLeveilUnits, positionLightLevelValue + olc::vf2d(fontSize24.GetStringBounds(strLightLevelValue + U"0").size.x, 0));
			}
			positionLightInfoNext += olc::vf2d(0, 40);
		}

		// Then display UV Index information and graph (if applicable)
		if (uvIndex.current != -1)
		{
			std::u32string strUVindex = ConvertedString32(std::to_string(uvIndex.current));
			positionUVindexValue = positionLightInfoNext + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);
			RenderString32(U"UV Index", &fontSize24, olc::GREY, &renderableUVindexLabel, positionLightInfoNext);
			RenderString32(strUVindex, &fontSize24, olc::WHITE, &renderableUVindexValue, positionUVindexValue);
			DrawUVindexGraph(positionUVindexValue, strUVindex);
		}
	}

	olc::Pixel windArrowColors[3] = { { 170, 0, 0 }, { 93, 147, 201 }, { 23, 77, 131 } };
	olc::Pixel tempPixel = { 3, 57, 111 };
	//olc::Pixel tempPixel = { 100, 100, 100 };
	olc::Pixel tempPixelAdjustment1 = { 90, 90, 90 };
	olc::Pixel tempPixelAdjustment2 = { 20, 20, 20 };
	// Display the Wind Direction compass and render the current and previous directional arrows in the right places


	//for (int i = (dequeWindDirections.size() - 1); i >= 0 ; i--)
	//	DrawWindDirectionArrow(dequeWindDirections.at(i), windArrowColors[i]);

	if (windAnimationIsMoving)
	{
		if (animationElapsedTime >= (1.0f / 60.0f))
		{
			//float tempNum = (windDirHalfDistance * 2.0f) - windDirDistanceLeft;

			//if (windAnimationIsMoving)
			//	printf("Distance left = %.2f, Current speed = %.2f (Halfway = %.2f)\n", windDirDistanceLeft, windDirAnimatedSpeed, windDirHalfDistance);

			//windDirAnimatedSpeed = std::sinf((std::numbers::pi * tempNum) / (windDirHalfDistance * 2.0f));
			//printf("Speed calc = %.3f (%.3f)\n", windDirAnimatedSpeed, (std::numbers::pi* tempNum) / (windDirHalfDistance * 2.0f));

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
		RenderStringCentered(GetWindDirectionName(windDirAnimatedPosition), &fontSize40, olc::GREY, &renderableWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	}
	else
	{
		DrawWindDirectionArrow(dequeWindDirections.at(0), windArrowColors[0]);
		RenderStringCentered(GetWindDirectionName(dequeWindDirections.at(0)), &fontSize40, olc::GREY, &renderableWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	}

	//olc::vf2d windArrowOffset = olc::vf2d(windCircleRadius * std::sin(degreesToRadians(arrowCircleCounter)), (windCircleRadius * std::cos(degreesToRadians(arrowCircleCounter))) * -1.0f);
	//DrawRotatedDecal(windCircleCenterPoint + windArrowOffset, decalWindDir, degreesToRadians(arrowCircleCounter), centerPointWindDir, { 1.0f, 1.0f }, windArrowColors[0]);
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(33, 77, 125));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(192, 192, 192, 120));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(70, 104, 140));
	//DrawWindDirectionArrow(dequeWindDirections.at(i), olc::Pixel(47, 88, 131));

	// Display Wind Speed and speed units label
	if (windSpeedValueMPH.current != undefinedValue)
		RenderStringCentered(std::to_string(int(windSpeedValueMPH.current)), &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 30));
	else
		RenderStringCentered("- -", &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 2));
	RenderStringCentered(textWindSpeedUnits, &fontSize32, colorLabelText, &renderableWindSpeedUnits, windCircleCenterPoint + olc::vf2d(0, 70));

	// Display Wind Speed current average and high
	RenderString32(U"Avg", &fontSize24, colorLabelText, &renderableLabelWindSpeedAvg, positionWindSpeedAvgLabel);
	RenderString(std::format("{:.0f}", windSpeedAvgValueMPH), &fontSize24, olc::WHITE, &renderableWindSpeedAvg, positionWindSpeedAvgLabel + olc::vf2d(fontSize24.GetStringBounds(U"Avg").size.x + 9, 0));
	RenderString32(U"Peak", &fontSize24, colorLabelText, &renderableWindSpeedHighLabel, positionWindSpeedHighLabel);
	RenderString(std::format("{:.0f}", windSpeedHighValueMPH), &fontSize24, olc::WHITE, &renderableWindSpeedHighValue, positionWindSpeedHighLabel + olc::vf2d(fontSize24.GetStringBounds(U"Peak").size.x + 9, 0));

	// Display the rain gauge stuff
	std::u32string strRainfallValue32 = ConvertedString32(std::format("{:.2f}", rainfallTotalTodayValue));
	olc::vf2d positionRainGauge = { positionSensorAreaStart.x + (positionSensorAreaSize.x / 2), positionRainAreaStart.y + 40 };
	olc::vf2d positionRainfallValue = { positionRainGauge.x + (rainGaugeTotalWidth / 2), positionRainGauge.y + rainGaugeTotalHeight + 20 };
	olc::vf2d positionRainfallTodayLabel = positionRainAreaStart + olc::vf2d(25, positionRainAreaSize.y - 90);
	olc::vf2d positionRainfallTodayValue = { positionRainGauge.x, positionRainfallTodayLabel.y };
	olc::vf2d positionRainfallRateLabel = positionRainfallTodayLabel + olc::vf2d(0, 45);
	olc::vf2d positionRainfallRateValue = { positionRainGauge.x, positionRainfallRateLabel.y };

	// Display rainfall icon
	DrawDecal(positionRainAreaStart + olc::vf2d(25, 40), decalRainfallIcon);

	if (rainGaugeFilledPercentage >= 0.01f)
		FillRect(positionRainGauge + rainGaugeFilledStartPrev, rainGaugeFilledSizePrev, olc::nDefaultAlpha);

	if (useMetricUnits)
		rainGaugeFilledPercentage = rainfallTotalTodayValue / rainGaugeCapacityMm;
	else
		rainGaugeFilledPercentage = rainfallTotalTodayValue / rainGaugeCapacityIn;

	rainGaugeFilledHeight = (rainGaugeTotalHeight * rainGaugeFilledPercentage);
	rainGaugeFilledStart = { 0, rainGaugeTotalHeight - rainGaugeFilledHeight };
	rainGaugeFilledSize = { rainGaugeTotalWidth, rainGaugeFilledHeight };
	olc::vf2d waterSpriteSourceStart = { 0, spriteRainGauge->Size().y - rainGaugeFilledHeight };

	DrawPartialSprite(positionRainGauge + rainGaugeFilledStart, spriteRainGauge, waterSpriteSourceStart, rainGaugeFilledSize);
	//DrawPartialDecal(positionRainGauge + rainGaugeFilledStart, rainGaugeFilledSize, decalRainGauge, rainGaugeFilledStart, rainGaugeFilledSize);
	DrawRainGauge(rainGaugeCapacityIn, positionRainGauge - olc::vf2d(1, 1), rainGaugeTotalSize + olc::vf2d(1, 1), 10, rainGaugeBorderColor);
	backgroundAlphaDecal->Update();
	rainGaugeFilledStartPrev = rainGaugeFilledStart;
	rainGaugeFilledSizePrev = rainGaugeFilledSize;

	RenderString32(U"Today", &fontSize32, olc::GREY, &renderbleRainfallTodayLabel, positionRainfallTodayLabel);
	RenderString32(strRainfallValue32, &fontSize32, olc::WHITE, &renderableRainfallValue, positionRainfallTodayValue);
	RenderString32(U"in", &fontSize32, colorLabelText, &renderableRainTodayUnitsLabel, positionRainfallTodayValue + olc::vf2d(fontSize32.GetStringBounds(strRainfallValue32).size.x + 15, 0));
	RenderString32(U"Rate", &fontSize32, olc::GREY, &renderableRainfallRateLabel, positionRainfallRateLabel);
	tempString32 = ConvertedString32(std::format("{:.2f}", rainfallRateInchesPerHour));
	RenderString32(tempString32, &fontSize32, olc::WHITE, &renderableRainfallRateValue, positionRainfallRateValue);
	RenderString32(U"in/hr", &fontSize32, colorLabelText, &renderableRainfallRateUnits, positionRainfallRateValue + olc::vf2d(fontSize32.GetStringBounds(tempString32).size.x + 15, 0));
	//RenderStringCentered(std::format("{:.2f} in", rainfallTotalTodayValue), &fontSize32, olc::WHITE, &renderableRainfallValue, positionRainfallValue);

	// Display the signal reliability meter and render the corresponding icon
	RenderStringCentered("Outdoor", &fontSize24, colorLabelText, &renderableSensorOutdoorLabel, positionSignalOutdoorCenter);
	RenderStringCentered("Indoor", &fontSize24, colorLabelText, &renderableSensorIndoorLabel, positionSignalIndoorCenter);

	// Display signal meters for both Outdoor and Indoor sensors representing how stable the telemetry is
	olc::Pixel sensorSignalColors[5] = { olc::Pixel(255, 62, 46), olc::WHITE, olc::WHITE, olc::WHITE, olc::GREEN };
	RenderStringRightJustified("Signal:", &fontSize18, olc::GREY, &renderableSignalOutdoorLabel, positionSignalOutdoorCenter + olc::vf2d(-5, 50));
	RenderStringRightJustified("Signal:", &fontSize18, olc::GREY, &renderableSignalIndoorLabel, positionSignalIndoorCenter + olc::vf2d(-5, 50));
	DrawDecal(positionSignalMeterOutdoor, decalSignalStrength[0], { 0.0625f , 0.0625f });
	DrawDecal(positionSignalMeterIndoor, decalSignalStrength[0], { 0.0625f , 0.0625f });
	DrawDecal(positionSignalMeterOutdoor, decalSignalStrength[outdoorSensor.packetCounter], { 0.0625f , 0.0625f }, sensorSignalColors[outdoorSensor.packetCounter]);
	DrawDecal(positionSignalMeterIndoor, decalSignalStrength[indoorSensor.packetCounter], { 0.0625f , 0.0625f }, sensorSignalColors[indoorSensor.packetCounter]);

	// Next display each sensor's battery condition
	RenderStringRightJustified("Battery:", &fontSize18, olc::GREY, &renderableBatteryOutdoorLabel, positionBatteryOutdoorLabel);
	RenderStringRightJustified("Battery:", &fontSize18, olc::GREY, &renderableBatteryIndoorLabel, positionBatteryIndoorLabel);
	RenderStringRightJustified("Channel: ", &fontSize18, olc::GREY, &renderableChannelOutdoorLabel, positionChannelOutdoorLabel);
	RenderStringRightJustified("Channel: ", &fontSize18, olc::GREY, &renderableChannelIndoorLabel, positionChannelIndoorLabel);
	if (outdoorSensor.batteryStatus == undefinedValue)
		RenderString32(U"-  -", &fontSize18, olc::GREY, &renderableBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	else if (outdoorSensor.batteryStatus == batteryStatusNormal)
		RenderString32(U"Normal", &fontSize18, olc::GREEN, &renderableBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0));
	else
		RenderString32(U"Low", &fontSize18, olc::Pixel(255, 62, 46), &renderableBatteryOutdoorValue, positionBatteryOutdoorLabel + olc::vf2d(14, 0));
	if (indoorSensor.batteryStatus == undefinedValue)
		RenderString32(U"-  -", &fontSize18, olc::GREY, &renderableBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	else if (indoorSensor.batteryStatus == batteryStatusNormal)
		RenderString32(U"Normal", &fontSize18, olc::GREEN, &renderableBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0));
	else
		RenderString32(U"Low", &fontSize18, olc::Pixel(255, 62, 46), &renderableBatteryIndoorValue, positionBatteryIndoorLabel + olc::vf2d(14, 0));

	// Finally display each sensor's current channel value
	if (outdoorSensor.channel != "")
		RenderString(outdoorSensor.channel, &fontSize18, olc::WHITE, &renderableChannelOutdoorValue, positionChannelOutdoorLabel + olc::vf2d(14, 0));
	else
		RenderString("-  -", &fontSize18, olc::GREY, &renderableChannelOutdoorValue, positionChannelOutdoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);
	if (indoorSensor.channel != "")
		RenderString(indoorSensor.channel, &fontSize18, olc::WHITE, &renderableChannelIndoorValue, positionChannelIndoorLabel + olc::vf2d(14, 0));
	else
		RenderString("-  -", &fontSize18, olc::GREY, &renderableChannelIndoorValue, positionChannelIndoorLabel + olc::vf2d(14, 0) + positionDashCorrection18);

	// Display Indoor Temperature and Humidity
	//RenderString32(U"Indoor", &fontSize32, olc::GREY, &renderableIndoorLabel, { positionOutdoorAreaStart.x, 600 });
	olc::vf2d positionIndoorTempValue = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.39f), (positionIndoorAreaSize.y * 0.32f) + positionIndoorAreaStart.y };
	olc::vf2d positionIndoorHumidityValue = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.80f), (positionIndoorAreaSize.y * 0.30f) + positionIndoorAreaStart.y };
	if (indoorTempValueF.current != undefinedValue)
		RenderStringRightJustified(std::format("{:.1f}", indoorTempValueF.current), &fontSize56, olc::WHITE, &renderableIndoorTempValue, positionIndoorTempValue);
	else
		RenderStringRightJustified("- -", &fontSize56, olc::GREY, &renderableIndoorTempValue, positionIndoorTempValue + olc::vf2d(-10, positionDashCorrection56.y));
	RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableIndoorTempUnits, positionIndoorTempValue + olc::vf2d(8, 0));

	//DrawDecal(positionTemp + olc::vf2d(0, fontSize56.GetStringBounds(tempString32).size.y + 10), renderableLabelTempLow.Decal());
	//RenderHighOrLowValue("Low", highLowIndoorTempF.low, &renderableIndoorTempLowValue, positionTemp + olc::vf2d(10, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
	//RenderHighOrLowValue("High", highLowIndoorTempF.high, &renderableIndoorTempHighValue, positionTemp + olc::vf2d(85, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
	//RenderString32(U"Low  69    High  72", &fontSize18, colorLabelText, &renderableIndoorHighLabel, positionTemp + olc::vf2d(-10, fontSize56.GetStringBounds(tempString32).size.y + 10));
	//positionTemp += olc::vf2d(fontSize56.GetStringBounds(tempString32).size.x + spacerFontSize18, 0);
	//RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableIndoorTempUnits, positionTemp);
	if (indoorHumidityValue.current != undefinedValue)
		RenderStringRightJustified(std::format("{:d}", indoorHumidityValue.current), &fontSize56, olc::WHITE, &renderableIndoorHumidityValue, positionIndoorHumidityValue);
	else
		RenderStringRightJustified("- -", &fontSize56, olc::GREY, &renderableIndoorHumidityValue, positionIndoorHumidityValue + olc::vf2d(-10, positionDashCorrection56.y));
	RenderString32(U"%", &fontSize24, colorLabelText, &renderableIndoorHumidityUnits, positionIndoorHumidityValue + olc::vf2d(8, 0));

	// Display the Settings/Info Area assets
	//DrawDecal(positionInfoAreaGearIcon, decalSettingsIcon);


	//DrawDecal(positionInfoAreaStart + positionConditionsNowOffset, decalCurConditionsImage);
	//RenderStringCentered(webWxCurConditionsDesc, &fontSize18, olc::WHITE, &renderableConditionsDesc, positionInfoAreaStart + positionConditionsNowOffset + olc::vf2d(spriteCurConditionsImage->width / 2, spriteCurConditionsImage->height));

	float infoAreaThirdsWidth = positionInfoAreaSize.x * 0.33333f;
	float infoAreaThirdsOffset = infoAreaThirdsWidth / 2.0f;
	float infoAreaIconStartY = positionInfoAreaStart.y + 65;
	olc::vf2d positionInfoNowCenter = positionInfoAreaStart + olc::vf2d(infoAreaThirdsWidth / 2.0f, 33);
	olc::vf2d positionInfoTodayCenter = positionInfoAreaStart + olc::vf2d(infoAreaThirdsWidth + infoAreaThirdsOffset, 33);
	olc::vf2d positionInfoTomorrowCenter = positionInfoAreaStart + olc::vf2d(infoAreaThirdsWidth + infoAreaThirdsWidth + infoAreaThirdsOffset, 33);
	olc::vf2d positionInfoDividerTop = positionInfoAreaStart + olc::vf2d(infoAreaThirdsWidth, positionInfoAreaSize.y * 0.23f);
	olc::vf2d positionInfoDividerBottom = { positionInfoDividerTop.x, positionInfoAreaStart.y + positionInfoAreaSize.y - (positionInfoAreaSize.y * 0.2f) };
	//olc::vf2d positionInfo3dayCenter = { infoAreaDividerX + ((positionInfoAreaStart.x + positionInfoAreaSize.x - infoAreaDividerX) / 2.0f), positionInfoAreaStart.y + 30 };
	RenderStringCentered("Now", &fontSize22, colorLabelText, &renderableForecastNowLabel, positionInfoNowCenter);
	RenderStringCentered("Today", &fontSize22, colorLabelText, &renderableForecastToday, positionInfoTodayCenter);
	RenderStringCentered("Tomorrow", &fontSize22, colorLabelText, &renderableForecastTomorrow, positionInfoTomorrowCenter);
	// Draw divider line betweeen "Current" conditions and the "Today/Tomorrow" forecasts
	DrawLineDecal(positionInfoDividerTop, positionInfoDividerBottom, areasBorderColor);

	if (webWxDataReady)
	{
		LoadWebWxAssets(&webWxCurrentConditions, decalWebConditionsImage);
		for (int i = 0; i < 2; i++)
			LoadWebWxAssets(&webWxDailyForecasts[i], decalWebForecastImages[i]);
		curlResponseBuffer.clear();
		webWxDataReady = false;
	}

	RenderCenteredWxCondition({ positionInfoNowCenter.x, infoAreaIconStartY }, decalWebConditionsImage, &renderableConditionsDesc);
	RenderCenteredWxForecast({ positionInfoTodayCenter.x, infoAreaIconStartY }, decalWebForecastImages[0], &webWxDailyForecasts[0], &renderableForecastText[0]);
	RenderCenteredWxForecast({ positionInfoTomorrowCenter.x, infoAreaIconStartY }, decalWebForecastImages[1], &webWxDailyForecasts[1], &renderableForecastText[1]);

	return true;
}

bool DragonWx::OnUserDestroy()
{
	appShouldExit = appExitRequested;
	return true;
}

bool DragonWx::mouseWithinArea(olc::vf2d mousePos, olc::vf2d areaStart, olc::vf2d areaSize)
{
	if ((mousePos.x > areaStart.x) && (mousePos.x < (areaStart.x + areaSize.x)) && (mousePos.y > areaStart.y) && (mousePos.y < (areaStart.y + areaSize.y)))
		return true;
	else
		return false;
}

olc::vf2d DragonWx::GetCenteredStartPosition(olc::vf2d totalAreaSize, olc::vf2d objectAreaSize)
{
	return olc::vf2d((totalAreaSize.x / 2) - (objectAreaSize.x / 2), (totalAreaSize.y / 2) - (objectAreaSize.y / 2));
}

std::u32string DragonWx::ConvertedString32(std::string inputString)
{
	return std::u32string(inputString.begin(), inputString.end());
}

olc::vf2d DragonWx::GetTextOffsetPosition(olc::vf2d startPos, olc::Font* fontToUse, std::string inputString)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	return olc::vf2d(startPos.x + fontToUse->GetStringBounds(tempString32).size.x, startPos.y);
}

olc::vf2d DragonWx::GetTextOffsetPosition32(olc::vf2d startPos, olc::Font* fontToUse, std::u32string inputString32)
{
	return olc::vf2d(startPos.x + fontToUse->GetStringBounds(inputString32).size.x, startPos.y);
}

void DragonWx::RenderString(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
	DrawDecal(leftPos, renderable->Decal());
}

void DragonWx::RenderString32(std::u32string inputString32, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos)
{
	*renderable = fontToUse->RenderStringToRenderable(inputString32 + U" ", textColor);
	DrawDecal(leftPos, renderable->Decal());
}

void DragonWx::RenderStringCentered(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d centerPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
	olc::vf2d leftPos = centerPos - olc::vf2d((fontToUse->GetStringBounds(tempString32).size.x / 2), 0);
	DrawDecal(leftPos, renderable->Decal());
}

void DragonWx::RenderStringRightJustified(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d rightPos)
{
	std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
	*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
	olc::vf2d leftPos = rightPos - olc::vf2d(fontToUse->GetStringBounds(tempString32).size.x, 0);
	DrawDecal(leftPos, renderable->Decal());
}

void DragonWx::RenderHighOrLowValue(std::string labelText, double highLowValue, olc::Renderable* renderableValue, olc::vf2d valuePos, bool useSmallerText)
{
	if (useSmallerText)
	{
		if (labelText == "Low")
			DrawDecal(valuePos - olc::vf2d(fontSize18.GetStringBounds(U"Low ").size.x, 0), renderableLowLabel18.Decal());
		else if (labelText == "High")
			DrawDecal(valuePos - olc::vf2d(fontSize18.GetStringBounds(U"High ").size.x, 0), renderableHighLabel18.Decal());
		if (highLowValue != undefinedValue)
			RenderString(std::format("{:.0f}", highLowValue), &fontSize18, olc::WHITE, renderableValue, valuePos + olc::vf2d(8, 0));
		else
			RenderString32(U"-  -", &fontSize18, olc::WHITE, renderableValue, valuePos + positionDashCorrection18 + olc::vf2d(8, 0));
	}
	else
	{
		if (labelText == "Low")
			DrawDecal(valuePos - olc::vf2d(fontSize24.GetStringBounds(U"Low ").size.x, 0), renderableLowLabel24.Decal());
		else if (labelText == "High")
			DrawDecal(valuePos - olc::vf2d(fontSize24.GetStringBounds(U"High ").size.x, 0), renderableHighLabel24.Decal());

		if (highLowValue != undefinedValue)
			RenderString(std::format("{:.0f}", highLowValue), &fontSize24, olc::WHITE, renderableValue, valuePos + olc::vf2d(9, 0));
		else
			RenderString32(U"-  -", &fontSize24, olc::WHITE, renderableValue, valuePos + positionDashCorrection24 + olc::vf2d(9, 0));
	}
}

void DragonWx::DrawWindDirectionArrow(double degreesBearing, olc::Pixel arrowColor)
{
	olc::vf2d windArrowOffset = olc::vf2d(windCircleRadius * std::sin(degreesToRadians(degreesBearing)), (windCircleRadius * std::cos(degreesToRadians(degreesBearing))) * -1.0f);
	DrawRotatedDecal(windCircleCenterPoint + windArrowOffset, decalWindDir, degreesToRadians(degreesBearing), centerPointWindDir, { 1.0f, 1.0f }, arrowColor);
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

olc::vf2d DragonWx::DrawBoxTitle(std::u32string strSectionTitle, olc::Font* fontToUse, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor)
{
	double titlePaddingFactor = 0.14;
	int xRightEdge = posUpperLeft.x + rectSize.x;
	int yBottomEdge = posUpperLeft.y + rectSize.y;
	int textOffset = (rectSize.x - fontToUse->GetStringBounds(strSectionTitle).size.x) / 2;
	//int topSegLength = ((rectSize.x - (rectSize.x * titlePaddingFactor)) - fontToUse->GetStringBounds(strSectionTitle).size.x) / 2;
	int topSegLength = rectSize.x - (fontToUse->GetStringBounds(strSectionTitle).size.x + (rectSize.x * titlePaddingFactor) + radius);
	int topSegRightStart = xRightEdge - topSegLength;
	int distSquared, radiusSquared;

	//DrawLine(olc::vi2d(posUpperLeft.x + radius, posUpperLeft.y), olc::vi2d(posUpperLeft.x + topSegLength, posUpperLeft.y), pixelColor);
	DrawLine(olc::vi2d(topSegRightStart, posUpperLeft.y), olc::vi2d(xRightEdge - radius, posUpperLeft.y), pixelColor);

	DrawLine(olc::vi2d(posUpperLeft.x + radius, yBottomEdge), olc::vi2d(xRightEdge - radius, yBottomEdge), pixelColor);
	DrawLine(olc::vi2d(posUpperLeft.x, posUpperLeft.y + radius), olc::vi2d(posUpperLeft.x, yBottomEdge - radius), pixelColor);
	DrawLine(olc::vi2d(xRightEdge, posUpperLeft.y + radius), olc::vi2d(xRightEdge, yBottomEdge - radius), pixelColor);

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
				Draw(olc::vi2d(posUpperLeft.x + radius - dx, posUpperLeft.y + radius - dy), pixelColor);
				Draw(olc::vi2d(xRightEdge - radius + dx, posUpperLeft.y + radius - dy), pixelColor);
				Draw(olc::vi2d(posUpperLeft.x + radius - dx, yBottomEdge - radius + dy), pixelColor);
				Draw(olc::vi2d(xRightEdge - radius + dx, yBottomEdge - radius + dy), pixelColor);
			}
		}
	//return olc::vf2d(posUpperLeft.x + textOffset, (posUpperLeft.y - fontToUse->GetStringBounds(strSectionTitle).size.y / 2));
	return olc::vf2d(posUpperLeft.x + radius + ((rectSize.x * titlePaddingFactor) / 2), (posUpperLeft.y - fontToUse->GetStringBounds(strSectionTitle).size.y / 2));
}

void DragonWx::DrawRainGauge(double gaugeFullValue, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor)
{
	int xRightEdge = posUpperLeft.x + rectSize.x;
	int yBottomEdge = posUpperLeft.y + rectSize.y;
	int distSquared, radiusSquared;
	olc::Font* fontToUse = &fontSize18;
	int fontCenterOffset = std::round(fontToUse->GetStringBounds(U"1").size.y / 2) + 2;
	int positionY;
	int gaugeSingleTickPixels = rectSize.y / rainGaugeMarksTotal;
	double gaugeSingleTickValue = gaugeFullValue / rainGaugeMarksTotal;

	//SetDrawTarget(backgroundAlphaSprite);

	// First draw the bottom and top unit labels
	RenderString32(U"0.00 in", fontToUse, olc::GREY, &renderableRainGaugeUnits[0], olc::vf2d(posUpperLeft.x - 60, yBottomEdge - fontToUse->GetStringBounds(U"0.00 in").size.y));
	RenderString(std::format("{:.2f} in", gaugeFullValue), fontToUse, olc::GREY, &renderableRainGaugeUnits[rainGaugeMarksTotal], olc::vf2d(posUpperLeft.x - 60, posUpperLeft.y));

	// Now draw the other unit labels along with line markings
	for (int i = 1; i < rainGaugeMarksTotal; i++)
	{
		positionY = yBottomEdge - (i * gaugeSingleTickPixels);
		DrawLineDecal(olc::vf2d(posUpperLeft.x, positionY), olc::vf2d(posUpperLeft.x + 10, positionY), olc::GREY);
		if ((i % 2) == 0)
			RenderString(std::format("{:.2f} in", i * gaugeSingleTickValue), fontToUse, olc::GREY, &renderableRainGaugeUnits[i], olc::vf2d(posUpperLeft.x - 60, positionY - fontCenterOffset));
	}

	DrawLine(olc::vi2d(posUpperLeft.x + radius, posUpperLeft.y), olc::vi2d(xRightEdge - radius, posUpperLeft.y), pixelColor);
	DrawLine(olc::vi2d(posUpperLeft.x + radius, yBottomEdge), olc::vi2d(xRightEdge - radius, yBottomEdge), pixelColor);
	DrawLine(olc::vi2d(posUpperLeft.x, posUpperLeft.y + radius), olc::vi2d(posUpperLeft.x, yBottomEdge - radius), pixelColor);
	DrawLine(olc::vi2d(xRightEdge, posUpperLeft.y + radius), olc::vi2d(xRightEdge, yBottomEdge - radius), pixelColor);

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
				Draw(olc::vi2d(posUpperLeft.x + radius - dx, posUpperLeft.y + radius - dy), pixelColor);
				Draw(olc::vi2d(xRightEdge - radius + dx, posUpperLeft.y + radius - dy), pixelColor);
				Draw(olc::vi2d(posUpperLeft.x + radius - dx, yBottomEdge - radius + dy), pixelColor);
				Draw(olc::vi2d(xRightEdge - radius + dx, yBottomEdge - radius + dy), pixelColor);
			}
		}
}

void DragonWx::DrawUVindexGraph(olc::vf2d startPos, std::u32string strValue)
{
	// Configure dimensions and position info for the graph decals
	uvGraphTotalHeight = fontSize24.GetStringBounds(strValue).size.y * 0.65f;
	//positionUVindexLabel = positionLightLevelLabel + olc::vf2d(0, 35);
	positionUVindexGraph = startPos + olc::vf2d(fontSize24.GetStringBounds(strValue).size.x + 20.0f, (fontSize24.GetStringBounds(strValue).size.y - uvGraphTotalHeight) / 2.0f);
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
	olc::vf2d positionUVarrow = { uvIndex.current * uvSegmentLength, uvGraphTotalHeight };
	FillTriangleDecal(positionUVindexGraph + positionUVarrow, positionUVindexGraph + positionUVarrow + olc::vf2d(5.0f, 13.0f), positionUVindexGraph + positionUVarrow + olc::vf2d(-5.0f, 13.0f), olc::RED);

	//for (int i = 0; i < 4; i++)
	//	GradientFillRectDecal(positionUVindexGraph + olc::vf2d(uvSegmentOffsets[i], 0), { uvSegmentSizes[i], uvGraphTotalHeight }, uvPixelColors[i], uvPixelColors[i], uvPixelColors[i + 1], uvPixelColors[i + 1]);
}

void DragonWx::RenderCenteredWxCondition(olc::vf2d centeredPos, olc::Decal* decalToDraw, olc::Renderable* renderable)
{
	DrawDecal(centeredPos - olc::vf2d(decalToDraw->sprite->width / 2, 0), decalToDraw);
	RenderStringCentered(webWxCurrentConditions.description, &fontSize18, olc::WHITE, renderable, centeredPos + olc::vf2d(0, decalToDraw->sprite->height + 4));
}

void DragonWx::RenderCenteredWxForecast(olc::vf2d centeredPos, olc::Decal* decalToDraw, wxWebEntry* wxWebEntryPtr, olc::Renderable* renderableText)
{
	int forecastHigh = std::round(wxWebEntryPtr->tempMax);
	int forecastLow = std::round(wxWebEntryPtr->tempMin);
	std::string strForecastInfo = std::to_string(forecastHigh) + " / " + std::to_string(forecastLow) + " (" + std::to_string(wxWebEntryPtr->precipPercent) + "%)";
	DrawDecal(centeredPos - olc::vf2d(decalToDraw->sprite->width / 2, 0), decalToDraw);
	RenderStringCentered(strForecastInfo, &fontSize18, olc::WHITE, renderableText, centeredPos + olc::vf2d(0, decalToDraw->sprite->height + 4));
}

olc::Decal* DragonWx::UpdateTrendData(std::deque<double>* sourceDeque, double dataValue, olc::Decal* decalTarget, std::string debugTextLabel)
{
	if (sourceDeque->size() >= trendSampleSize)
	{
		sourceDeque->pop_front();
		sourceDeque->push_back(dataValue);

		double trendSlope = CalculateTrendSlope(sourceDeque);
		// DEBUG: Show all the samples
		printf("%s \x1b[1;34m%s Sample Points: (0,%.1f)", GetTimestamp(), debugTextLabel.c_str(), sourceDeque->at(0));
		for (int i = 1; i < trendSampleSize; i++)
			printf(", (%u,%.1f)", i, sourceDeque->at(i));
		//printf("\n\x1b[0m");
		printf("%s \n\x1b[1;32m%s Trend Slope: %.3f\n\x1b[0m", GetTimestamp(), debugTextLabel.c_str(), trendSlope);

		if (trendSlope >= 0.02)
			return decalTrendArrowUp;
		else if (trendSlope <= -0.02)
			return decalTrendArrowDown;
		else
			return decalTrendArrowSteady;
	}
	else if (dataValue != undefinedValue)
	{
		sourceDeque->push_back(dataValue);
		printf("%s \x1b[1;31m%s Trend Sample Size: %u\n\x1b[0m", GetTimestamp(), debugTextLabel.c_str(), sourceDeque->size());
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

void DragonWx::MidnightDailyReset()
{
	highLowOutdoorTempF = { outdoorTempValueF.current, outdoorTempValueF.current };
	highLowOutdoorTempC = { outdoorTempValueC.current, outdoorTempValueC.current };
	highLowOutdoorHumidity = highLowRange(outdoorHumidityValue.current, outdoorHumidityValue.current);

	highLowIndoorTempF = { indoorTempValueF.current, indoorTempValueF.current };
	highLowIndoorTempC = { indoorTempValueC.current, indoorTempValueC.current };
	highLowIndoorHumidity = highLowRange(indoorHumidityValue.current, indoorHumidityValue.current);

	windSpeedHighValueMPH = windSpeedValueMPH.current;
	windSpeedHighValueKPH = windSpeedValueKPH.current;

	rainfallTotalTodayValue = 0.0;
}

bool DragonWx::LoadWebWxAssets(wxWebEntry* wxDataEntry, olc::Decal* decalTarget)
{
	std::string	pathToIcon = "./images/Conditions/";
	if (wxDataEntry->useDaytime)
	{
		pathToIcon += wxCodeTable[wxDataEntry->code].iconFileDay;
		wxDataEntry->description = wxCodeTable[wxDataEntry->code].descriptionDay;
	}
	else
	{
		pathToIcon += wxCodeTable[wxDataEntry->code].iconFileNight;
		wxDataEntry->description = wxCodeTable[wxDataEntry->code].descriptionNight;
	}
	pathToIcon += ".png";
	if (decalTarget->sprite->LoadFromFile(pathToIcon) != 1)
		return false;
	decalTarget->Update();
	return true;
}

bool DragonWx::SaveConfigFile()
{
	std::ofstream configFile("DragonWx.conf");
	if (!configFile.is_open())
	{
		std::cout << "Error: Could not open config file for writing." << std::endl;
		return false;
	}

	configFile << "DragonWx Config File v1.0" << std::endl << std::endl;
	configFile << "RTL433_PATH=" << pathToExec << std::endl;
	configFile << "RTL433_PARAMS=" << sdrExtraArguments << std::endl;
	configFile << "SDR_GAIN=" << sdrGainSetting << std::endl;
	configFile << "SDR_ANTENNA=" << sdrAntennaSetting << std::endl;
	configFile << "FULLSCREEN=" << (fullscreenToggle ? "1" : "0") << std::endl;
	configFile << "UNITS=" << useMetricUnits << std::endl;
	configFile << "STATION_NAME=" << strWxStationName << std::endl;

	configFile.close();
	return true;
}