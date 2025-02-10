#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TTF
#include "./olcPGEX_TTF-main/olcPGEX_TTF.h"

#define OLC_PGEX_SPLASHSCREEN
#include "olcPGEX_SplashScreen.h"

#include <numbers>
#include <thread>
#include <chrono>
#include <deque>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <tchar.h>
#endif
#include <cstdio>
#include <numeric>
//#include <cmath>

// Debug-related variables
bool useRealPipe = false;
bool fullscreenToggle = false;
bool debugKeyPressed = false;
bool prevKeyPressed = true;
bool debugState = false;
bool useSDRplay = true;
bool alwaysShowWindChill = true;

// Application-related constants and variables
char buffer[512], strDateWeekMonthDay[64], strFormattedTime[16];
std::string tempString, wxDataMessage, dataString, strFullyFormattedDate, strFullyFormattedTime;
std::u32string tempString32;
FILE* pipeRTL;
bool threadKeepAlive = false;
bool useMetricUnits = false;
bool newIndoorTelemetry = false;
bool newOutdoorTelemetry = false;
double tempDouble;
int packetSequenceNum = -1;
int minuteTimeCounter = 0;		// This allows me to use single 15 second interval to also handle events once per minute
int secondsCounter30 = 0;		// This allows me to use single 15 second interval to also handle events once per 30 seconds
std::thread rtl433_thread;
float elapsedTimeCounter = 0.0f;
const double undefinedValue = 300;
const int rainGaugeMarksTotal = 8;

struct sensorStatus
{
	bool recentlyUpdated;
	int packetCounter;
	int batteryStatus;
	std::string ID;
	std::string name;
	std::string channel;
};
sensorStatus outdoorSensor;
sensorStatus indoorSensor;

struct highLowRange
{
	double high;
	double low;
};
highLowRange highLowOutdoorTempF = { undefinedValue, undefinedValue };
highLowRange highLowIndoorTempF = { undefinedValue, undefinedValue };
highLowRange highLowOutdoorTempC = { undefinedValue, undefinedValue };
highLowRange highLowIndoorTempC = { undefinedValue, undefinedValue };
highLowRange highLowOutdoorHumidity = { undefinedValue, undefinedValue };
highLowRange highLowIndoorHumidity = { undefinedValue, undefinedValue };

//std::u32string strDegreesUnitsF = U"°F";
//std::u32string strDegreesUnitsC = U"°C";
std::u32string strDegreesUnitsF = U"\u00B0F";
std::u32string strDegreesUnitsC = U"\u00B0C";

// Dewpoint-related constants and variables
const float referenceVaporPressure = 6.112f;
const float magnusCoefficient = 17.67f;
const float magnusTempOffset = 243.5f;
const float eulersNumber = 2.718f;
const float standardPressure_hPa = 1013.25f;
float convertedTempC, saturationVaporPressure, actualVaporPressure, calculatedDewpointC;
double dewpointValueF = undefinedValue, dewpointValueC = undefinedValue;

// Wind Chill related constants and variables
const float windChillCoEffecient = 0.16f;
const float windChillBaselineF = 35.74f;
const float windChillBaselineC = 13.12f;
const float windChillTempContrib = 0.6215f;
const float windChillSpeedFactorF1 = 35.75f;
const float windChillSpeedFactorF2 = 0.4275f;
const float windChillSpeedFactorC1 = 11.37f;
const float windChillSpeedFactorC2 = 0.3965f;
double calculatedWindChillF = undefinedValue, calculatedWindChillC = undefinedValue;

// Heat Index related constants and variables
const float heatIndexConst1 = -42.379f;
const float heatIndexConst2 = 2.04901523f;
const float heatIndexConst3 = 10.14333127f;
const float heatIndexConst4 = -0.22475541f;
const float heatIndexConst5 = -6.83783 * std::pow(10.0f, -3.0f);
const float heatIndexConst6 = -5.481717 * std::pow(10.0f, -2.0f);
const float heatIndexConst7 = 1.22874 * std::pow(10.0f, -3.0f);
const float heatIndexConst8 = 8.5282 * std::pow(10.0f, -4.0f);
const float heatIndexConst9 = -1.99 * std::pow(10.0f, -6.0f);
double calculatedHeatIndexF = undefinedValue, convertedHeatIndexC = undefinedValue;

// RTL_433 executable paths and variables
std::string pathToExec;
#ifdef WIN32
//std::string pathToExecRTLSDR = "C:\\Users\\TekTodd\\Portable Apps\\rtl_433-win-msvc-x64-22.11\\rtl_433-rtlsdr-soapysdr.exe";
std::string pathToExecRTLSDR = "C:\\Users\\DragonEmby\\Downloads\\rtl_433-win-msvc-x64-24.10\\rtl_433-rtlsdr.exe";
std::string pathToExecSDRplay = "C:\\Program Files\\PothosSDR\\bin\\rtl_433-rtlsdr-soapysdr.exe";
#else
std::string pathToExecRTLSDR = "C:\\Users\\TekTodd\\Portable Apps\\rtl_433-win-msvc-x64-22.11\\rtl_433-rtlsdr-soapysdr.exe";
std::string pathToExecSDRplay = "~/Downloads/rtl_433/build/src/rtl_433";
#endif

// Wireless-sensor-related variables
struct doublePrevCur
{
	double previous;
	double current;
};
struct intPrevCur
{
	int previous;
	int current;
};
struct stringPrevCur
{
	std::string previous;
	std::string current;
};
stringPrevCur outdoorPacketTimestamp, indoorPacketTimestamp;
std::string textWindSpeedUnits, strWindDirectionName;
doublePrevCur outdoorTempValueF = { undefinedValue, undefinedValue }, outdoorTempValueC = { undefinedValue, undefinedValue };
doublePrevCur indoorTempValueF = { undefinedValue, undefinedValue }, indoorTempValueC = { undefinedValue, undefinedValue };
intPrevCur outdoorHumidityValue = { undefinedValue, undefinedValue }, indoorHumidityValue = { undefinedValue, undefinedValue };
doublePrevCur windSpeedValueMPH = { undefinedValue, undefinedValue }, windSpeedValueKPH = { undefinedValue, undefinedValue };
double windSpeedHighValueMPH = undefinedValue, windSpeedAvgValueMPH = undefinedValue, windSpeedHighValueKPH = undefinedValue, windSpeedAvgValueKPH = undefinedValue, windDirectionValue;
doublePrevCur rainfallDataValueInches = { undefinedValue, undefinedValue };
double rainfallTotalTodayValue = 0.0, rainfallRateInchesPerHour = 0.0, rainfallRateMmPerHour = 0.0, rainfallRateAverage = 0.0;
std::deque<float> dequeRainRateSamples;
std::time_t rainRateTimeNow, rainRateTimePrevious;

const int batteryStatusLow = 0;
const int batteryStatusNormal = 1;

// Trend-related constants and variables
const int trendSampleSize = 60;
const int trendingUp = 1;
const int trendingSteady = 0;
const int trendingDown = -1;
doublePrevCur outdoorTempTrendSample;
intPrevCur intTrendSample;
double doubleTempDelta;
int intTrendCountUp = 0, intTrendCountSteady = 0, intTrendCountDown = 0;
intPrevCur trendDirOutdoorTemp = { trendingSteady, trendingSteady };
intPrevCur trendDirIndoorTemp = { trendingSteady, trendingSteady };
//int trendDirOutdoorTemp = trendingSteady, trendDirIndoorTemp = trendingSteady;
int trendDirOutdoorHumidity = trendingSteady, trendDirIndoorHumidity = trendingSteady;
double sumsBottomEquation, xSum = 0, xSumSquare = 0;

// Network-based telemetry variables
double curPressureValue_hPa = standardPressure_hPa;

std::chrono::seconds windSamplingPeriod(15);
std::chrono::seconds telemetryTimeoutPeriod(5);
std::chrono::steady_clock::time_point windSamplingTimePrevious, windSamplingTimeCurrent;
std::chrono::steady_clock::time_point indoorTelemetryTimerPrevious, indoorTelemetryTimerCurrent;

olc::Font fontSize18, fontSize24, fontSize32, fontSize40, fontSize56, fontSize72, fontSize96;

olc::Renderable renderableDateText, renderableTimeText, renderableIndoorLabel, renderableOutdoorLabel, renderableAppNameLabel, renderableSettingsLabel;

olc::Renderable renderableLowLabel18, renderableHighLabel18, renderableLowLabel24, renderableHighLabel24;
olc::Renderable renderableIndoorLowValue, renderableIndoorHighValue;

olc::Renderable renderableLabelTempF, renderableDewPointUnits, renderableLabelDewpoint;
olc::Renderable renderableTempValue, renderableOutdoorTempLowValue, renderableOutdoorTempHighValue;
olc::Renderable renderableIndoorTempUnits, renderableIndoorTempValue, renderableIndoorTempLowValue, renderableIndoorTempHighValue;
olc::Renderable renderableWindChillLabel, renderableWindChillValue, renderableWindChillUnits;

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

std::deque<double> dequeWindGustAvg, dequeOutdoorTemps, dequeIndoorTemps, dequeOutdoorHumidity, dequeIndoorHumidity;

std::string getOrdinalSuffix(int);
void CalculateDewpoint(double, int, double);
void CalculateHeatIndex(double, int);
void CalculateWindChill(double, double, double, double);
double CalculateTrendSlope(std::deque<double>*);
double ConvertedTempCtoF(double);
double ConvertedTempFtoC(double);
double degreesToRadians(double);
void UpdateHighLowValues(double, highLowRange*);
std::string GetTimestamp();
void readWeatherData();
bool jsonGetField(std::string);
void populateTestData();

typedef std::basic_stringstream<char32_t> u32stringstream;

// Override base class with your custom functionality
class DragonWx : public olc::PixelGameEngine
{
public:
	//olc::SplashScreen splash;

	DragonWx()
	{
		// Name your application
		sAppName = "DragonWeather";
	}

	olc::Key keyPrevious;
	std::string strDewpointLabel = "Dewpoint";

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
	olc::Sprite* spriteWindLastDirection;
	olc::Decal* decalWindLastDirection;

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

	olc::vf2d positionTemp, positionSensorInfoTitle, positionSensorAreaStart, positionSensorAreaSize;
	olc::vf2d positionWindowCenter, positionOutdoorAreaStart, positionOutdoorAreaSize, positionOutdoorTitle;
	olc::vf2d positionIndoorAreaStart, positionIndoorAreaSize, positionIndoorTitle;
	olc::vf2d positionOutdoorTempValueF, positionTempLabelF, positionOutdoorTempHighValue, positionOutdoorTempLowValue, positionOutdoorTrendOffset;
	olc::vf2d positionOutdoorHumidityValue, positionHumidityLowValue, positionHumidityHighValue;
	olc::vf2d positionDewPointLabel, positionDewPointValue, positionWindChillLabel, positionWindChillValue;
	olc::vf2d windCircleTopLeftPos, windCircleCenterPoint, positionWindSpeedAvgLabel, positionWindSpeedHighLabel;	// Wind-related position definitions

	olc::vf2d positionRainAreaStart, positionRainAreaSize, positionRainAreaTitle;
	olc::vf2d rainGaugeFilledStart, rainGaugeFilledSize, rainGaugeTotalSize, rainGaugeFilledStartPrev, rainGaugeFilledSizePrev;

	olc::vf2d positionInfoAreaStart, positionInfoAreaSize, positionInfoAreaTitle, positionInfoAreaGearIcon;

	int spacerFontSize18, spacerFontSize24, spacerFontSize32;
	int sensorAreaDividerX;

	olc::vf2d positionDashCorrection18 = { 2, 7 };
	olc::vf2d positionDashCorrection24 = { 2, 9 };
	olc::vf2d positionDashCorrection32 = { 0, 11 };
	olc::vf2d positionDashCorrection40 = { 0, 13 };
	olc::vf2d positionDashCorrection96 = { 0, 12 };

	float windCircleRadius;

	float screenPaddingOffsetY;
	float outdoorAreaStartScaleX, outdoorAreaStartScaleY;
	float outdoorAreaSizeScaleX, outdoorAreaSizeScaleY;

	float outdoorMainValueScaleX, outdoorTempValueScaleY, outdoorHumidityValueScaleY;

	float outdoorHighLowOffsetY, outdoorTrendOffsetX, outdoorTrendOffsetY;

	std::time_t systemTimeNow, systemTimePrevious;

	float rainGaugeFilledPercentage;
	float rainGaugeTotalWidth = 60.0f;
	//float rainGaugeTotalHeight = 352.0f;
	float rainGaugeTotalHeight = 300.0f;
	float rainGaugeFilledHeight;
	double rainGaugeCapacityIn = 1.0;
	double rainGaugeCapacityMm = 25.0;

	//double tempData[30] = { 18.0, 18.0, 18.1, 18.1, 18.1, 18.5, 18.5, 18.5, 18.6, 18.6, 18.5, 18.7, 18.5, 18.7, 18.9,
	//						18.9, 19.0, 18.9, 19.0, 18.9, 18.9, 18.8, 18.8, 18.9, 18.8, 18.9, 19.0, 18.9, 19.0, 19.1 };

	//double tempData[30] = { 18.0, 18.0, 18.1, 18.1, 18.1, 18.2, 18.1, 18.2, 18.3, 18.2, 18.3, 18.3, 18.3, 18.3, 18.3,
	//						18.4, 18.4, 18.4, 18.4, 18.4, 18.5, 18.5, 18.5, 18.5, 18.5, 18.6, 18.6, 18.6, 18.6, 18.6 };

public:
	bool OnUserCreate() override
	{
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

		spriteWindDir = new olc::Sprite("./images/WindArrow256x256.png");
		decalWindDir = new olc::Decal(spriteWindDir);
		centerPointWindDir = { float(spriteWindDir->width) / 2.0f, float(spriteWindDir->height / 2.0f) };

		spriteWindCircle = new olc::Sprite("./images/WindCircle256.png");
		decalWindCircle = new olc::Decal(spriteWindCircle);

		spriteWindLastDirection = new olc::Sprite(256, 256);

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

		screenPaddingOffsetY = GetWindowSize().y * 0.06f;
		outdoorAreaStartScaleX = 0.03125f;
		//outdoorAreaStartScaleY = 0.08333f;
		outdoorAreaStartScaleY = 0.07f;
		//outdoorAreaSizeScaleX = 0.265625f;
		outdoorAreaSizeScaleX = 0.28f;
		outdoorAreaSizeScaleY = 0.5f;
		float sensorAreaStartScaleY = 0.75f;

		positionOutdoorAreaStart = { GetWindowSize().x * outdoorAreaStartScaleX, screenPaddingOffsetY };
		positionOutdoorAreaSize = { GetWindowSize().x * outdoorAreaSizeScaleX, GetWindowSize().y * outdoorAreaSizeScaleY };
		//positionIndoorAreaStart = { std::round(GetWindowSize().x * outdoorAreaStartScaleX), 615 };
		//positionIndoorAreaStart = GetWindowSize() * olc::vf2d(0.03125f, 0.85417f);
		//positionIndoorAreaStart = GetWindowSize() * olc::vf2d(0.03125f, 0.80f);
		//positionIndoorAreaSize = { std::round(GetWindowSize().x * outdoorAreaSizeScaleX), 160 };
		//positionIndoorAreaSize = { GetWindowSize().x * outdoorAreaSizeScaleX, (GetWindowSize().y * 0.75f) - positionIndoorAreaStart.y };
		//positionSensorAreaStart = { 930, GetWindowSize().y * sensorAreaStartScaleY };
		positionSensorAreaStart = GetWindowSize() * olc::vf2d(0.72656f, 0.73611f);
		positionSensorAreaSize = { 310, 170 };
		positionIndoorAreaStart = GetWindowSize() * olc::vf2d(0.03125f, 0.80f);
		positionIndoorAreaSize = { GetWindowSize().x * outdoorAreaSizeScaleX, (positionSensorAreaStart.y + positionSensorAreaSize.y) - positionIndoorAreaStart.y };
		//positionSensorAreaStart = GetWindowSize() * olc::vf2d(0.72656f, 0.70778f);
		//positionSensorAreaSize = { 310, (GetWindowSize().y - screenPaddingOffsetY) - positionSensorAreaStart.y };

		positionRainAreaStart = { positionSensorAreaStart.x, positionOutdoorAreaStart.y };
		positionRainAreaSize = { positionSensorAreaSize.x, GetWindowSize().y * 0.63f };

		//outdoorMainValueScaleX = 0.620589f;
		//outdoorMainValueScaleX = 0.69f;
		//outdoorTempValueScaleY = 0.16667f;
		//outdoorHumidityValueScaleY = 0.77778f;
		//olc::vf2d positionOutdoorTempOffset = positionOutdoorAreaSize * olc::vf2d(0.69f, 0.16667f);
		olc::vf2d positionOutdoorTempOffset = positionOutdoorAreaSize * olc::vf2d(0.69f, 0.13889f);
		//olc::vf2d positionOutdoorHumidityOffset = positionOutdoorAreaSize * olc::vf2d(0.69f, 0.61111f);
		olc::vf2d positionOutdoorHumidityOffset = positionOutdoorAreaSize * olc::vf2d(0.69f, 0.58333f);
		olc::vf2d positionOutdoorLowLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.42179f, 0.25f);
		olc::vf2d positionOutdoorHighLabelOffset = positionOutdoorAreaSize * olc::vf2d(-0.11453f, 0.25f);
		//positionOutdoorTrendOffset = positionOutdoorAreaSize * olc::vf2d(0.14f, 0.09f);
		positionOutdoorTrendOffset = positionOutdoorAreaSize * olc::vf2d(0.08f, 0.11f);

		//outdoorHighLowOffsetY = std::round(GetWindowSize().y * 0.125f);
		//outdoorTrendOffsetX = std::round(GetWindowSize().x * 0.046875f);
		//outdoorTrendOffsetY = std::round(GetWindowSize().y * 0.02778f);
		//outdoorTrendOffsetX = std::round(GetWindowSize().x * 0.042969f);
		//outdoorTrendOffsetX = std::round(positionOutdoorAreaSize.x * 0.14f);
		//outdoorTrendOffsetY = std::round(positionOutdoorAreaSize.y * 0.09f);
		//outdoorTrendOffsetY = std::round(GetWindowSize().y * 0.03333f);

		//positionTempValueF = { 100, 100 };
		//positionOutdoorTempValueF = { 251, 120 };
		//positionOutdoorTempValueF = { (positionOutdoorAreaSize.x * outdoorMainValueScaleX) + positionOutdoorAreaStart.x, positionOutdoorAreaSize.y * outdoorTempValueScaleY) + positionOutdoorAreaStart.y };
		positionOutdoorTempValueF = positionOutdoorAreaStart + positionOutdoorTempOffset;
		positionOutdoorTempLowValue = positionOutdoorTempValueF + positionOutdoorLowLabelOffset;
		positionOutdoorTempHighValue = positionOutdoorTempValueF + positionOutdoorHighLabelOffset;
		//positionTempLowLabel = { 100, 210 };
		//positionTempLowLabel = { 100, positionOutdoorTempValueF.y + outdoorHighLowOffsetY };
		//positionTempLowLabel = { positionOutdoorTempValueF.x - 151, positionOutdoorTempValueF.y + outdoorHighLowOffsetY };
		
		//positionTempHighLabel = { 210, positionOutdoorTempValueF.y + outdoorHighLowOffsetY };
		//positionTempHighLabel = { positionOutdoorTempValueF.x - 41, positionOutdoorTempValueF.y + outdoorHighLowOffsetY };
		//positionOutdoorTrendOffset = { outdoorTrendOffsetX, outdoorTrendOffsetY };

		//positionTemp = { positionOutdoorAreaSize.x *  }
		//positionOutdoorHumidityValue = { std::round((positionOutdoorAreaSize.x * outdoorMainValueScaleX) + positionOutdoorAreaStart.x), 280 };
		positionOutdoorHumidityValue = positionOutdoorAreaStart + positionOutdoorHumidityOffset;
		positionHumidityLowValue = positionOutdoorHumidityValue + positionOutdoorLowLabelOffset;
		positionHumidityHighValue = positionOutdoorHumidityValue + positionOutdoorHighLabelOffset;
		//positionHumidityLowLabel = { 100, positionOutdoorHumidityValue.y + outdoorHighLowOffsetY };
		//positionHumidityHighLabel = { 210, positionOutdoorHumidityValue.y + outdoorHighLowOffsetY };
		//positionHumidityLowLabel = { positionOutdoorHumidityValue.x - 151, positionOutdoorHumidityValue.y + outdoorHighLowOffsetY };
		//positionHumidityHighLabel = { positionOutdoorHumidityValue.x - 41, positionOutdoorHumidityValue.y + outdoorHighLowOffsetY };

		positionDewPointLabel = positionOutdoorAreaStart + olc::vf2d(0, 380);
		positionDewPointValue = positionDewPointLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);

		positionWindChillLabel = positionDewPointLabel + olc::vf2d(0, 50);
		positionWindChillValue = positionWindChillLabel + olc::vf2d(positionOutdoorAreaSize.x / 2, 0);

		positionInfoAreaSize = { GetWindowSize().x * 0.30f, positionIndoorAreaSize.y };
		positionInfoAreaStart = { positionWindowCenter.x - (positionInfoAreaSize.x / 2), positionIndoorAreaStart.y };
		positionInfoAreaGearIcon = positionInfoAreaStart + (positionInfoAreaSize * olc::vf2d(0.90f, 0.50f)) - (spriteSettingsIcon->Size() / olc::vi2d(2, 2));

		//windCircleTopLeftPos = { 500, 200 };
		//windCircleCenterPoint = { positionWindowCenter.x, 328 };
		windCircleCenterPoint = { positionWindowCenter.x, positionWindowCenter.y };
		windCircleRadius = spriteWindCircle->width / 2;		// This assumes the sprite containing the wind direction circle is a perfect square and an even number
		//windCircleCenterPoint = { (windCircleTopLeftPos.x + windCircleRadius), (windCircleTopLeftPos.y + windCircleRadius) };
		windCircleTopLeftPos = { (windCircleCenterPoint.x - windCircleRadius), (windCircleCenterPoint.y - windCircleRadius) };
		printf("Wind Circle Center Pos: X = %f, Y = %f\n", windCircleCenterPoint.x, windCircleCenterPoint.y);
		positionWindSpeedAvgLabel = { 545, (windCircleRadius * 1.1875f) + windCircleCenterPoint.y };
		positionWindSpeedHighLabel = { 730, (windCircleRadius * 1.1875f) + windCircleCenterPoint.y }; 

		rainGaugeTotalSize = { rainGaugeTotalWidth, rainGaugeTotalHeight };

		// DEBUG: Report calculated Position coordinates
		printf("Debug: Outdoor Area Start Coords = %.1f, %.1f\n", positionOutdoorAreaStart.x, positionOutdoorAreaStart.y);
		printf("Debug: Outdoor Area Size = %.1f, %.1f\n", positionOutdoorAreaSize.x, positionOutdoorAreaSize.y);
		printf("Debug: Outdoor Temp Value Coords = %.1f, %.1f\n", positionOutdoorTempValueF.x, positionOutdoorTempValueF.y);
		printf("Debug: Outdoor Temp Low Coords = %.1f, %.1f\n", positionOutdoorTempLowValue.x, positionOutdoorTempLowValue.y);
		printf("Debug: Outdoor Temp High Coords = %.1f, %.1f\n", positionOutdoorTempHighValue.x, positionOutdoorTempHighValue.y);
		printf("Debug: Outdoor Humidity Value Coords = %.1f, %.1f\n", positionOutdoorHumidityValue.x, positionOutdoorHumidityValue.y);
		printf("Debug: Outdoor Humidity Low Coords = %.1f, %.1f\n", positionHumidityLowValue.x, positionHumidityLowValue.y);
		printf("Debug: Outdoor Humidity High Coords = %.1f, %.1f\n", positionHumidityHighValue.x, positionHumidityHighValue.y);

		windSpeedHighValueMPH = 0.0;
		windSpeedAvgValueMPH = 0.0;

		renderableLowLabel18 = fontSize18.RenderStringToRenderable(U"Low ", colorLabelText);
		renderableHighLabel18 = fontSize18.RenderStringToRenderable(U"High ", colorLabelText);
		renderableLowLabel24 = fontSize24.RenderStringToRenderable(U"Low ", colorLabelText);
		renderableHighLabel24 = fontSize24.RenderStringToRenderable(U"High ", colorLabelText);

		SetDrawTarget(spriteWindLastDirection);


		SetDrawTarget(spriteSettingsScreen);
		Clear(olc::BLANK);
		decalSettingsScreen->Update();

		SetDrawTarget(spriteRainGaugeClear);
		Clear(olc::BLANK);

		SetDrawTarget(backgroundAlphaSprite);
		positionOutdoorTitle = DrawBoxTitle(U"Outdoor", &fontSize40, positionOutdoorAreaStart, positionOutdoorAreaSize, 20, areasBorderColor);
		positionIndoorTitle = DrawBoxTitle(U"Indoor", &fontSize32, positionIndoorAreaStart, positionIndoorAreaSize, 20, areasBorderColor);
		positionSensorInfoTitle = DrawBoxTitle(U"Sensors", &fontSize32, positionSensorAreaStart, positionSensorAreaSize, 20, areasBorderColor);
		positionRainAreaTitle = DrawBoxTitle(U"Rainfall", &fontSize32, positionRainAreaStart, positionRainAreaSize, 20, areasBorderColor);
		positionInfoAreaTitle = DrawBoxTitle(U"DragonWx", &fontSize32, positionInfoAreaStart, positionInfoAreaSize, 20, areasBorderColor);
		sensorAreaDividerX = positionSensorAreaStart.x + (positionSensorAreaSize.x / 2);
		int sensorAreaDividerStartY = positionSensorAreaStart.y + (positionSensorAreaSize.y * 0.18f);
		int sensorAreaDividerEndY = positionSensorAreaStart.y + positionSensorAreaSize.y - (positionSensorAreaSize.y * 0.18f);
		DrawLine(sensorAreaDividerX, sensorAreaDividerStartY, sensorAreaDividerX, sensorAreaDividerEndY, areasBorderColor);

		DrawCircle(positionWindowCenter, 128, rainGaugeBorderColor);
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

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::SPACE).bPressed)
			debugKeyPressed = !debugKeyPressed;

		olc::vf2d positionMouseCursor = GetMousePos();
		if ((positionMouseCursor.x >= positionInfoAreaGearIcon.x) && (positionMouseCursor.x <= (positionInfoAreaGearIcon.x + spriteSettingsIcon->Size().x)))
			if ((positionMouseCursor.y >= positionInfoAreaGearIcon.y) && (positionMouseCursor.y <= (positionInfoAreaGearIcon.y + spriteSettingsIcon->Size().y)))
				if (GetMouse(olc::Mouse::LEFT).bPressed)
					debugKeyPressed = !debugKeyPressed;

		if (GetKey(olc::Key::R).bPressed && !prevKeyPressed)
		{
			//debugKeyPressed = !debugKeyPressed;
			rainfallDataValueInches.current += 0.01;
			/*
			if ((rainfallTotalTodayValue / rainGaugeCapacityIn) >= 0.90)
				rainGaugeCapacityIn += 1.0;
			rainGaugeFilledPercentage = rainfallTotalTodayValue / rainGaugeCapacityIn;
			*/
			prevKeyPressed = true;
			debugState = true;
			//printf("Debug: Rainfall = %.2f, Filled Percentage = %.2f\n", rainfallTotalTodayValue, rainGaugeFilledPercentage);
		}
		else if (!GetKey(olc::Key::R).bPressed)
			prevKeyPressed = false;

		elapsedTimeCounter += fElapsedTime;
		if (elapsedTimeCounter >= 15.0f)
		{
			printf("Time interval error = %.5f\n", elapsedTimeCounter - 15.0f);
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

			secondsCounter30++;
			if (secondsCounter30 >= 3)
			{
				secondsCounter30 = 0;
				if (dequeRainRateSamples.size() > 0)
					dequeRainRateSamples.pop_front();	// Remove oldest sample since we missed a packet
			}

			elapsedTimeCounter = 0;
			printf("%s Outdoor Sensor: Consecutive packets = %u\n", GetTimestamp().c_str(), outdoorSensor.packetCounter);
			printf("%s Indoor Sensor: Consecutive packets = %u\n", GetTimestamp().c_str(), indoorSensor.packetCounter);
		}

		// Render blank background image
		DrawDecal({ 0.0f, 0.0f }, backgroundDecal, { 0.333f, 0.333f });

		if (debugKeyPressed)
		{
			olc::vf2d boxSize = { 600, 400 };
			olc::vf2d positionSettingsBoxTitle;
			SetDrawTarget(spriteSettingsScreen);
			positionSettingsBoxTitle = DrawBoxTitle(U"DragonWx Settings", &fontSize32, GetCenteredStartPosition(GetWindowSize(), boxSize), boxSize, 20, olc::GREY);
			decalSettingsScreen->Update();
			DrawDecal({ 0, 0 }, decalSettingsScreen);
			RenderString32(U"DragonWx Settings", &fontSize32, olc::GREY, &renderableSettingsLabel, positionSettingsBoxTitle);


			return true;
		}

		DrawDecal({ 0, 0 }, backgroundAlphaDecal);
		RenderString32(U"Outdoor", &fontSize40, olc::Pixel(0, 0, 255), &renderableOutdoorLabel, positionOutdoorTitle);
		RenderString32(U"Indoor", &fontSize32, olc::GREY, &renderableIndoorLabel, positionIndoorTitle);
		RenderString32(U"Sensors", &fontSize32, olc::GREY, &renderableSensorInfoLabel, positionSensorInfoTitle);
		RenderString32(U"Rainfall", &fontSize32, olc::GREY, &renderableRainfallLabel, positionRainAreaTitle);
		RenderString32(U"DragonWx", &fontSize32, olc::GREY, &renderableAppNameLabel, positionInfoAreaTitle);

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

			#ifdef WIN32
			if (localtime_s(&systemTimeLocalNow, &systemTimeNow) != 0)
				throw std::runtime_error("Failed to get local time.");
			if (localtime_s(&systemTimeLocalPrevious, &systemTimePrevious) != 0)
				throw std::runtime_error("Failed to get local time.");
			#else
			if (localtime_r(&systemTimeNow, &systemTimeLocalNow) == nullptr)
				throw std::runtime_error("Failed to get local time.");
			if (localtime_r(&systemTimePrevious, &systemTimeLocalPrevious) == nullptr)
				throw std::runtime_error("Failed to get local time.");
			#endif

			// Check for Midnight rollover to reset statistics, etc
			if ((systemTimeLocalNow.tm_hour == 0) && (systemTimeLocalPrevious.tm_hour != 0))
			{
				MidnightDailyReset();
				printf("%s Time: Midnight rollover\n", GetTimestamp().c_str());
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
		DrawDecal(olc::vf2d(positionOutdoorAreaStart.x + 15, positionOutdoorTempValueF.y + 10), decalThermometer);
		DrawDecal(olc::vf2d(positionOutdoorAreaStart.x + 15, positionOutdoorHumidityValue.y + 10), decalWaterDrop);

		// Display the temperature
		if (outdoorTempValueF.current != undefinedValue)
			RenderStringRightJustified(std::format("{:.1f}", outdoorTempValueF.current), &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF);
		else
			RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &renderableTempValue, positionOutdoorTempValueF + positionDashCorrection96 - olc::vf2d(15, 0));
		RenderString32(strDegreesUnitsF, &fontSize40, colorLabelText, &renderableLabelTempF, positionOutdoorTempValueF + olc::vf2d(9, 0));

		// Display the Outdoor Temperature's current 24-hour low/high
		RenderHighOrLowValue("Low", highLowOutdoorTempF.low, &renderableOutdoorTempLowValue, positionOutdoorTempLowValue, false);
		RenderHighOrLowValue("High", highLowOutdoorTempF.high, &renderableOutdoorTempHighValue, positionOutdoorTempHighValue, false);

		// Display the current Outdoor temperature trend arrow
		if (outdoorTempValueF.current != undefinedValue)
			//DrawDecal(positionOutdoorTempValueF + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));
			DrawDecal(positionOutdoorTempValueF + positionOutdoorTrendOffset, decalTrendOutdoorTemp, { 0.29f, 0.29f }, olc::Pixel(0x2F, 0xBE, 0x1A));

		// Display the Outdoor Humidity
		if (outdoorHumidityValue.current != undefinedValue)
			RenderStringRightJustified(std::to_string(outdoorHumidityValue.current), &fontSize96, olc::WHITE, &renderableHumidityValue, positionOutdoorHumidityValue);
		else
			RenderStringRightJustified("- -", &fontSize96, olc::WHITE, &renderableHumidityValue, positionOutdoorHumidityValue + positionDashCorrection96 - olc::vf2d(15, 0));
		RenderString("%", &fontSize40, colorLabelText, &renderableHumidityUnits, positionOutdoorHumidityValue + olc::vf2d(9, 0));

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
				olc::vf2d positionDewPointUnits = positionDewPointValue + olc::vf2d(fontSize32.GetStringBounds(strDewPointValue).size.x + spacerFontSize24, 0);
				RenderString32(strDegreesUnitsC, &fontSize24, colorLabelText, &renderableDewPointUnits, positionDewPointUnits);
			}
			else
				RenderString32(U"-  -", &fontSize32, olc::WHITE, &renderableDewpointValue, positionDewPointValue + positionDashCorrection32);
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
				RenderString32(U"-  -", &fontSize32, olc::WHITE, &renderableDewpointValue, positionDewPointValue + positionDashCorrection32);
		}

		// Display the Wind Chill if applicable (Temperature is 50F or below)
		if (useMetricUnits)
		{

		}
		else
		{
			if (outdoorTempValueF.current <= 50.0f)
			{
				std::u32string strWindChillValue;
				RenderString32(U"Wind Chill", &fontSize32, olc::GREY, &renderableWindChillLabel, positionWindChillLabel);
				if ((calculatedWindChillF == undefinedValue) || (windSpeedValueMPH.current == undefinedValue))
					RenderString32(U"-  -", &fontSize32, olc::WHITE, &renderableWindChillValue, positionWindChillValue + positionDashCorrection32);
				else 
				{
					if (windSpeedValueMPH.current >= 3.0)
						strWindChillValue = ConvertedString32(std::format("{:.1f}", calculatedWindChillF));
					else
						strWindChillValue = ConvertedString32(std::format("{:.1f}", outdoorTempValueF.current));
					RenderString32(strWindChillValue, &fontSize32, olc::WHITE, &renderableWindChillValue, positionWindChillValue);
					olc::vf2d positionWindChillUnits = positionWindChillValue + olc::vf2d(fontSize32.GetStringBounds(strWindChillValue).size.x + spacerFontSize24, 0);
					RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableWindChillUnits, positionWindChillUnits);
				}
			}
		}

		// Display the Wind Direction compass and render the arrow in the right place
		//DrawDecal(windCircleTopLeftPos, decalWindCircle);
		float windArrowOffsetX = windCircleRadius * std::sin(degreesToRadians(windDirectionValue));
		float windArrowOffsetY = (windCircleRadius * std::cos(degreesToRadians(windDirectionValue))) * -1.0f;
		olc::vf2d arrowCirclePos = { windCircleCenterPoint.x + windArrowOffsetX, windCircleCenterPoint.y + windArrowOffsetY };
		DrawRotatedDecal(arrowCirclePos, decalWindDir, degreesToRadians(windDirectionValue), centerPointWindDir, { 0.18f, 0.18f });
		strWindDirectionName = GetWindDirectionName(windDirectionValue);
		RenderStringCentered(strWindDirectionName, &fontSize40, olc::GREY, &renderableWindDirName, windCircleCenterPoint - olc::vf2d(0, 92));
	
		// Display Wind Speed and speed units label
		if (windSpeedValueMPH.current != undefinedValue)
			RenderStringCentered(std::to_string(int(windSpeedValueMPH.current)), &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 30));
		else
			RenderStringCentered("- -", &fontSize96, olc::WHITE, &windSpeedText, windCircleCenterPoint - olc::vf2d(0, 2));
		RenderStringCentered(textWindSpeedUnits, &fontSize32, colorLabelText, &renderableWindSpeedUnits, windCircleCenterPoint + olc::vf2d(0, 70));

		// Display Wind Speed current average and high
		RenderStringRightJustified("Avg", &fontSize24, colorLabelText, &renderableLabelWindSpeedAvg, positionWindSpeedAvgLabel);
		RenderString(std::to_string(int(windSpeedAvgValueMPH)), &fontSize24, olc::WHITE, &renderableWindSpeedAvg, positionWindSpeedAvgLabel + olc::vf2d(9, 0));
		RenderStringRightJustified("High", &fontSize24, colorLabelText, &renderableWindSpeedHighLabel, positionWindSpeedHighLabel);
		RenderString(std::to_string(int(windSpeedHighValueMPH)), &fontSize24, olc::WHITE, &renderableWindSpeedHighValue, positionWindSpeedHighLabel + olc::vf2d(9, 0));

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

		//SetDrawTarget(backgroundAlphaSprite);
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
		tempString32 = ConvertedString32(std::format("{:.1f}", rainfallRateInchesPerHour));
		RenderString32(tempString32, &fontSize32, olc::WHITE, &renderableRainfallRateValue, positionRainfallRateValue);
		RenderString32(U"in/hr", &fontSize32, colorLabelText, &renderableRainfallRateUnits, positionRainfallRateValue + olc::vf2d(fontSize32.GetStringBounds(tempString32).size.x + 15, 0));
		//RenderStringCentered(std::format("{:.2f} in", rainfallTotalTodayValue), &fontSize32, olc::WHITE, &renderableRainfallValue, positionRainfallValue);

		// Display the signal reliability meter and render the corresponding icon
		olc::vf2d positionSignalOutdoorCenter = positionSensorAreaStart + olc::vf2d(positionSensorAreaSize.x * 0.25f, 35);
		olc::vf2d positionSignalIndoorCenter = { positionSensorAreaStart.x + positionSensorAreaSize.x - (positionSensorAreaSize.x * 0.25f), positionSensorAreaStart.y + 35 };
		olc::vf2d positionSignalMeterOutdoor = positionSignalOutdoorCenter + olc::vf2d(9, 50 - (fontSize18.GetStringBounds(U"Signal:").size.y / 2));
		olc::vf2d positionSignalMeterIndoor = positionSignalIndoorCenter + olc::vf2d(9, 50 - (fontSize18.GetStringBounds(U"Signal:").size.y / 2));

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
		olc::vf2d positionBatteryOutdoorLabel = positionSignalOutdoorCenter + olc::vf2d(-5, 75);
		olc::vf2d positionBatteryIndoorLabel = positionSignalIndoorCenter + olc::vf2d(-5, 75);
		olc::vf2d positionChannelOutdoorLabel = positionSignalOutdoorCenter + olc::vf2d(-5, 100);
		olc::vf2d positionChannelIndoorLabel = positionSignalIndoorCenter + olc::vf2d(-5, 100);
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
		olc::vf2d positionIndoorLeftOffset = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.25f), (positionIndoorAreaSize.y * 0.30f) + positionIndoorAreaStart.y };
		olc::vf2d positionIndoorRightOffset = { positionOutdoorAreaStart.x + (positionOutdoorAreaSize.x * 0.65f), (positionIndoorAreaSize.y * 0.30f) + positionIndoorAreaStart.y };
		tempString32 = ConvertedString32(std::format("{:.1f}", indoorTempValueF.current));
		positionTemp = positionIndoorLeftOffset - olc::vf2d(fontSize56.GetStringBounds(tempString32).size.x / 2, 0);
		RenderString32(tempString32, &fontSize56, olc::WHITE, &renderableIndoorTempValue, positionTemp);
		//DrawDecal(positionTemp + olc::vf2d(0, fontSize56.GetStringBounds(tempString32).size.y + 10), renderableLabelTempLow.Decal());
		RenderHighOrLowValue("Low", highLowIndoorTempF.low, &renderableIndoorTempLowValue, positionTemp + olc::vf2d(10, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
		RenderHighOrLowValue("High", highLowIndoorTempF.high, &renderableIndoorTempHighValue, positionTemp + olc::vf2d(85, fontSize56.GetStringBounds(tempString32).size.y + 10), true);
		//RenderString32(U"Low  69    High  72", &fontSize18, colorLabelText, &renderableIndoorHighLabel, positionTemp + olc::vf2d(-10, fontSize56.GetStringBounds(tempString32).size.y + 10));
		positionTemp += olc::vf2d(fontSize56.GetStringBounds(tempString32).size.x + spacerFontSize18, 0);
		RenderString32(strDegreesUnitsF, &fontSize24, colorLabelText, &renderableIndoorTempUnits, positionTemp);
		tempString32 = ConvertedString32(std::to_string(indoorHumidityValue.current));
		positionTemp = positionIndoorRightOffset - (fontSize56.GetStringBounds(tempString32).size.x / 2, 0);
		RenderString32(tempString32, &fontSize56, olc::WHITE, &renderableIndoorHumidityValue, positionTemp);
		//DrawDecal(positionTemp + olc::vf2d(0, fontSize56.GetStringBounds(tempString32).size.y + 10), renderableLabelTempLow.Decal());
		positionTemp += olc::vf2d(fontSize56.GetStringBounds(tempString32).size.x + spacerFontSize18, 0);
		RenderString32(U"%", &fontSize24, colorLabelText, &renderableIndoorHumidityUnits, positionTemp);

		// Display the Indoor Temperature's current 24-hour low/high
		//RenderString32(U"Low ", &fontSize18, colorLabelText, &renderableIndoorLowLabel, positionIndoorAreaStart + olc::vf2d(15, 85));
		//RenderString32(U"High ", &fontSize18, colorLabelText, &renderableIndoorHighLabel, positionIndoorAreaStart + olc::vf2d(15, 100));


		DrawDecal(positionInfoAreaGearIcon, decalSettingsIcon);

		return true;
	}

	olc::vf2d GetCenteredStartPosition(olc::vf2d totalAreaSize, olc::vf2d objectAreaSize)
	{ return olc::vf2d((totalAreaSize.x / 2) - (objectAreaSize.x / 2), (totalAreaSize.y / 2) - (objectAreaSize.y / 2)); }

	std::u32string ConvertedString32(std::string inputString)
	{ return std::u32string(inputString.begin(), inputString.end()); }

	olc::vf2d GetTextOffsetPosition(olc::vf2d startPos, olc::Font* fontToUse, std::string inputString)
	{
		std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
		return olc::vf2d(startPos.x + fontToUse->GetStringBounds(tempString32).size.x, startPos.y);
	}

	olc::vf2d GetTextOffsetPosition32(olc::vf2d startPos, olc::Font* fontToUse, std::u32string inputString32)
	{ return olc::vf2d(startPos.x + fontToUse->GetStringBounds(inputString32).size.x, startPos.y); }

	void RenderString(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos)
	{
		std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
		*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		DrawDecal(leftPos, renderable->Decal());
	}

	void RenderString32(std::u32string inputString32, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d leftPos)
	{
		*renderable = fontToUse->RenderStringToRenderable(inputString32 + U" ", textColor);
		DrawDecal(leftPos, renderable->Decal());
	}

	void RenderStringCentered(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d centerPos)
	{
		std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
		*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		olc::vf2d leftPos = centerPos - olc::vf2d((fontToUse->GetStringBounds(tempString32).size.x / 2), 0);
		DrawDecal(leftPos, renderable->Decal());
	}

	void RenderStringRightJustified(std::string inputString, olc::Font* fontToUse, olc::Pixel textColor, olc::Renderable* renderable, olc::vf2d rightPos)
	{
		std::u32string tempString32 = std::u32string(inputString.begin(), inputString.end()) + U" ";
		*renderable = fontToUse->RenderStringToRenderable(tempString32, textColor);
		olc::vf2d leftPos = rightPos - olc::vf2d(fontToUse->GetStringBounds(tempString32).size.x, 0);
		DrawDecal(leftPos, renderable->Decal());
	}

	void RenderHighOrLowValue(std::string labelText, double highLowValue, olc::Renderable* renderableValue, olc::vf2d valuePos, bool useSmallerText)
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

	void DrawCircleArc(olc::vf2d startPos, int radius, double startAngle, double endAngle, olc::Pixel pixelColor)
	{
		for (double angle = startAngle; angle <= endAngle; angle += 0.1)	// Increment in degrees
		{				
			double rad = angle * (std::numbers::pi / 180.0);				// Convert to radians
			int x = startPos.x + radius * cos(rad);
			int y = startPos.y + radius * sin(rad);
			Draw({ x, y }, olc::RED);
		}
	}

	void DrawWindDirPreviousArc(olc::vf2d startPos, int radius, olc::Pixel pixelColor, uint8_t mask)
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

	olc::vf2d DrawBoxTitle(std::u32string strSectionTitle, olc::Font* fontToUse, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor)
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

	void DrawRainGauge(double gaugeFullValue, olc::vi2d posUpperLeft, olc::vi2d rectSize, int radius, olc::Pixel pixelColor)
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

	olc::Decal* UpdateTrendData(std::deque<double>* sourceDeque, double dataValue, olc::Decal* decalTarget, std::string debugTextLabel)
	{
		if (sourceDeque->size() >= trendSampleSize)
		{
			sourceDeque->pop_front();
			sourceDeque->push_back(dataValue);

			double trendSlope = CalculateTrendSlope(sourceDeque);
			// DEBUG: Show all the samples
			printf("%s \x1b[1;34m%s Sample Points: (0,%.1f)", GetTimestamp().c_str(), debugTextLabel.c_str(), sourceDeque->at(0));
			for (int i = 1; i < 60; i++)
				printf(", (%u,%.1f)", i, sourceDeque->at(i));
			//printf("\n\x1b[0m");
			printf("%s \n\x1b[1;32m%s Trend Slope: %.3f\n\x1b[0m", GetTimestamp().c_str(), debugTextLabel.c_str(), trendSlope);

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
			printf("%s \x1b[1;31m%s Trend Sample Size: %u\n\x1b[0m", GetTimestamp().c_str(), debugTextLabel.c_str(), sourceDeque->size());
		}

		// Return the original pointer, unchanged
		return decalTarget;
	}

	std::string GetWindDirectionName(double windDirDegrees)
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

	void MidnightDailyReset()
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
};

int main()
{
	if (useRealPipe)
	{
		#ifdef WIN32
		if (useSDRplay)
			pathToExec = "\"" + pathToExecSDRplay + "\" -v -d driver=sdrplay -g10 -F json";
		//pathToExec = "\"" + pathToExecSDRplay + "\" -g50 -F json";
		else
			pathToExec = "\"" + pathToExecRTLSDR + "\" -g50 -F json";
		pipeRTL = _popen(pathToExec.c_str(), "r");
		#else
		if (useSDRplay)
			pathToExec = pathToExecSDRplay + " -v -d driver=sdrplay -g10 -F json";
		//pathToExec = "\"" + pathToExecSDRplay + "\" -g50 -F json";
		else
			pathToExec = pathToExecRTLSDR + " -g50 -F json";
		pipeRTL = popen(pathToExec.c_str(), "r");
		#endif
		if (!pipeRTL)
		{
			std::cout << "Failed to run external command." << std::endl;
			return 1;
		}

		printf("%s\n", pathToExec.c_str());
		threadKeepAlive = true;
		rtl433_thread = std::thread(readWeatherData);
	}

	DragonWx demo;
	if (demo.Construct(1280, 720, 1, 1, fullscreenToggle, true))
		demo.Start();

	printf("Finished PGE window thread.\n");
	threadKeepAlive = false;
	rtl433_thread.join();
	printf("Thread finished\n");
	#ifdef WIN32
	_pclose(pipeRTL);
	#else
	pclose(pipeRTL);
	#endif

	return 0;
}

std::string GetTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	// Convert to local time
	std::tm local_time = *std::localtime(&time_t_now);
	int timeResult = std::strftime(strFormattedTime, sizeof(strFormattedTime), "%H:%M:%S", &local_time);

	return strFormattedTime;
}

void readWeatherData()
{
	size_t bufferLength;
	char* stringPtr;

	//if (!threadKeepAlive)
	//	return;

	while (threadKeepAlive)
	{
		stringPtr = fgets(buffer, sizeof(buffer), pipeRTL);
		bufferLength = strlen(buffer);
		if (stringPtr != buffer)
		{
			std::cout << "Invalid read" << std::endl;
			break;
		}
		else if (bufferLength > 0)
		{
			wxDataMessage += buffer;
			if ((buffer[bufferLength - 1] == 0x0A) || (buffer[bufferLength - 1] == 0x0D))
			{
				/*
				if (debugKeyPressed)
				{
					std::cout << wxDataMessage << std::endl;
					debugKeyPressed = false;
				}
				*/

				// First make sure this telemetry is coming from our target weather station ID
				if (jsonGetField("id"))
				{
					if (dataString == outdoorSensor.ID)
					{
						if (jsonGetField("time") && dataString != outdoorPacketTimestamp.previous)
						{
							outdoorPacketTimestamp.current = dataString;

							if (outdoorSensor.packetCounter == -1)
							{
								// Now that we know we are receiving live telemetry, do some init stuff
								printf("%s Outdoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
								windSamplingTimePrevious = std::chrono::steady_clock::now();
								elapsedTimeCounter = 0.0f;			// To hopefully syncronize the PGE's timing loop with the internal clock on weather station sensor
								outdoorSensor.packetCounter = 0;
							}

							if (jsonGetField("sequence_num"))
								packetSequenceNum = std::stoi(dataString);

							printf("Debug: Packet Sequence Number = %u\n", packetSequenceNum);

							if (jsonGetField("model"))
							{
								outdoorSensor.name = dataString;
								printf("%s Outdoor Sensor: %s\n", GetTimestamp().c_str(), outdoorSensor.name.c_str());
							}

							if (jsonGetField("channel"))
								outdoorSensor.channel = dataString;

							if (jsonGetField("temperature_C"))
							{
								outdoorTempValueC.current = std::stod(dataString);
								if ((outdoorTempValueC.current != outdoorTempValueC.previous))
								{
									outdoorTempValueF.current = ConvertedTempCtoF(outdoorTempValueC.current);
									CalculateDewpoint(outdoorTempValueC.current, outdoorHumidityValue.current, curPressureValue_hPa);
									UpdateHighLowValues(outdoorTempValueF.current, &highLowOutdoorTempF);
									UpdateHighLowValues(outdoorTempValueC.current, &highLowOutdoorTempC);
									printf("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorTempValueF.current);
								}
							}

							if (jsonGetField("temperature_F"))
							{
								outdoorTempValueF.current = std::stod(dataString);
								if (outdoorTempValueF.current != outdoorTempValueF.previous)
								{
									outdoorTempValueC.current = ConvertedTempFtoC(outdoorTempValueF.current);
									CalculateDewpoint(outdoorTempValueC.current, outdoorHumidityValue.current, curPressureValue_hPa);
									UpdateHighLowValues(outdoorTempValueF.current, &highLowOutdoorTempF);
									UpdateHighLowValues(outdoorTempValueC.current, &highLowOutdoorTempC);
									if ((outdoorTempValueF.current <= 50.0f) && (windSpeedValueMPH.current != 0.0))
										CalculateWindChill(outdoorTempValueF.current, outdoorTempValueC.current, windSpeedValueMPH.current, windSpeedValueKPH.current);
									else if ((outdoorTempValueF.current >= 80.0f) && (outdoorHumidityValue.current != undefinedValue))
										CalculateHeatIndex(outdoorTempValueF.current, outdoorHumidityValue.current);
									printf("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorTempValueF.current);
								}
							}

							if (jsonGetField("humidity"))
							{
								outdoorHumidityValue.current = std::stoi(dataString);
								if (outdoorHumidityValue.current != outdoorHumidityValue.previous)
								{
									UpdateHighLowValues(outdoorHumidityValue.current, &highLowOutdoorHumidity);
									if ((outdoorTempValueF.current != undefinedValue) && (outdoorTempValueF.current >= 80.0f))
										CalculateHeatIndex(outdoorTempValueF.current, outdoorHumidityValue.current);
									printf("%s Outdoor Humidity: %u%%\n", GetTimestamp().c_str(), outdoorHumidityValue.current);
								}
							}

							if (jsonGetField("wind_avg_mi_h"))
							{
								windSpeedValueMPH.current = std::stod(dataString);
								if ((outdoorTempValueF.current <= 50.0f) && (windSpeedValueMPH.current != 0.0))
									CalculateWindChill(outdoorTempValueF.current, outdoorTempValueC.current, windSpeedValueMPH.current, windSpeedValueKPH.current);
								if (windSpeedValueMPH.current > windSpeedHighValueMPH)
									windSpeedHighValueMPH = windSpeedValueMPH.current;
								printf("%s Outdoor Wind Speed: %.0f mph\n", GetTimestamp().c_str(), windSpeedValueMPH.current);
							}

							if (jsonGetField("wind_dir_deg"))
							{
								windDirectionValue = std::stod(dataString);
								printf("%s Outdoor Wind Direction: %.0f\xF8\n", GetTimestamp().c_str(), windDirectionValue);
							}

							if (jsonGetField("rain_in"))
							{
								int rainfallRateSampleCount = 0;

								rainRateTimeNow = std::time(nullptr);
								if (!debugState)
									rainfallDataValueInches.current = std::stod(dataString);
								if (rainfallDataValueInches.previous != undefinedValue)
								{
									double rainfallDeltaValue = 0.0;
									if (rainfallDataValueInches.current > rainfallDataValueInches.previous)
										rainfallDeltaValue = (rainfallDataValueInches.current - rainfallDataValueInches.previous);
									else if (rainfallDataValueInches.current < rainfallDataValueInches.previous)
										rainfallDeltaValue = (5.12 - rainfallDataValueInches.previous) + rainfallDataValueInches.current;
									rainfallTotalTodayValue += rainfallDeltaValue;
									if (dequeRainRateSamples.size() >= 2)
										dequeRainRateSamples.pop_front();
									dequeRainRateSamples.push_back(rainfallDeltaValue);
									printf("New Delta = %f\n", rainfallDeltaValue);
									rainfallRateInchesPerHour =std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0) * 60.0;
									printf("%s \x1b[1;34mOutdoor Rainfall Rate: %.2f in/hr (Sum = %.2f, Sample Count = %u)\n\x1b[0m", GetTimestamp().c_str(), rainfallRateInchesPerHour, std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0), dequeRainRateSamples.size());
								}
								rainfallDataValueInches.previous = rainfallDataValueInches.current;
								printf("%s \x1b[1;34mOutdoor Rainfall: %.2f inches\n\x1b[0m", GetTimestamp().c_str(), rainfallTotalTodayValue);
							}

							if (jsonGetField("battery_ok"))
							{
								outdoorSensor.batteryStatus = (dataString == "1") ? 1 : 0;
								printf("%s Outdoor Sensor Battery: %s\n", GetTimestamp().c_str(), outdoorSensor.batteryStatus ? "Normal" : "Low");
							}

							windSamplingTimeCurrent = std::chrono::steady_clock::now();
							if ((windSamplingTimeCurrent - windSamplingTimePrevious) >= windSamplingPeriod)
							{
								windSamplingTimePrevious = windSamplingTimeCurrent;
								if (dequeWindGustAvg.size() >= 40)
									dequeWindGustAvg.pop_front();
								dequeWindGustAvg.push_back(windSpeedValueMPH.current);

								windSpeedAvgValueMPH = 0.0;
								for (int i = 0; i < dequeWindGustAvg.size(); i++)
									windSpeedAvgValueMPH += dequeWindGustAvg.at(i);
								windSpeedAvgValueMPH /= dequeWindGustAvg.size();

								printf("%s Wind Avg: Sample Size = %u, Avg = %f\n", GetTimestamp().c_str(), dequeWindGustAvg.size(), windSpeedAvgValueMPH);
							}

							outdoorPacketTimestamp.previous = outdoorPacketTimestamp.current;
							outdoorSensor.recentlyUpdated = true;
							if (outdoorSensor.packetCounter < 1)
								outdoorSensor.packetCounter = 1;
						}
					}
					else if (dataString == indoorSensor.ID)
					{
						// Debug timer so console isn't spammed with repetitive text
						/*
						indoorTelemetryTimerCurrent = std::chrono::steady_clock::now();
						if ((indoorTelemetryTimerCurrent - indoorTelemetryTimerPrevious) >= telemetryTimeoutPeriod)
						{
							newIndoorTelemetry = true;
							indoorTelemetryTimerPrevious = indoorTelemetryTimerCurrent;
						}
						else
							newIndoorTelemetry = false;
						*/
						if (jsonGetField("time") && (dataString != indoorPacketTimestamp.previous))
						{
							indoorPacketTimestamp.current = dataString;
							if (indoorSensor.packetCounter == -1)
							{
								// Now that we know we are receiving live telemetry, do some init stuff
								printf("%s Indoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
								indoorSensor.packetCounter = 0;
							}

							if (jsonGetField("model"))
							{
								indoorSensor.name = dataString;
								printf("%s Indoor Sensor: %s\n", GetTimestamp().c_str(), indoorSensor.name.c_str());
							}

							if (jsonGetField("channel"))
								indoorSensor.channel = dataString;

							if (jsonGetField("temperature_C"))
							{
								indoorTempValueC.current = std::stod(dataString);
								if (!useMetricUnits)
									indoorTempValueF.current = ConvertedTempCtoF(indoorTempValueC.current);
								UpdateHighLowValues(indoorTempValueC.current, &highLowIndoorTempC);
								UpdateHighLowValues(indoorTempValueF.current, &highLowIndoorTempF);
								printf("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorTempValueF.current);
							}

							if (jsonGetField("temperature_F"))
							{
								indoorTempValueF.current = std::stod(dataString);
								if (useMetricUnits)
									indoorTempValueC.current = ConvertedTempFtoC(indoorTempValueF.current);
								UpdateHighLowValues(indoorTempValueF.current, &highLowIndoorTempF);
								UpdateHighLowValues(indoorTempValueC.current, &highLowIndoorTempC);
								printf("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorTempValueF);
							}

							if (jsonGetField("humidity"))
							{
								indoorHumidityValue.current = std::stoi(dataString);
								UpdateHighLowValues(indoorHumidityValue.current, &highLowIndoorHumidity);
								printf("%s Indoor Humidity: %u%%\n", GetTimestamp().c_str(), indoorHumidityValue.current);
							}

							if (jsonGetField("battery_ok"))
							{
								indoorSensor.batteryStatus = (dataString == "1") ? 1 : 0;
								printf("%s Indoor Sensor Battery: %s\n", GetTimestamp().c_str(), indoorSensor.batteryStatus ? "Normal" : "Low");
							}

							indoorPacketTimestamp.previous = indoorPacketTimestamp.current;
							indoorSensor.recentlyUpdated = true;
							if (indoorSensor.packetCounter < 1)
								indoorSensor.packetCounter = 1;
						}
					}
				}
				wxDataMessage.clear();
			}
		}
	}

	std::cout << "Here at end of fgets()" << std::endl;
}

std::string getOrdinalSuffix(int day)
{
	if (day % 10 == 1 && day % 100 != 11)
		return "st";
	if (day % 10 == 2 && day % 100 != 12)
		return "nd";
	if (day % 10 == 3 && day % 100 != 13)
		return "rd";
	return "th";
}

void CalculateDewpoint(double tempC, int relativeHumidity, double measuredPressure_hPa)
{
	// If input parameters are invalid, just return undefinedValue
	if ((tempC == undefinedValue) || (relativeHumidity == undefinedValue))
		return;

	saturationVaporPressure = referenceVaporPressure * std::exp((magnusCoefficient * tempC) / (tempC + magnusTempOffset));
	// Next calculate the "Actual Vapor Pressure"
	actualVaporPressure = relativeHumidity * (saturationVaporPressure / 100.0f) * (measuredPressure_hPa / standardPressure_hPa);
	// Next calculate our actual dewpoint in celsius
	dewpointValueC = (magnusTempOffset * std::log(actualVaporPressure / referenceVaporPressure)) / (magnusCoefficient - std::log(actualVaporPressure / referenceVaporPressure));
	dewpointValueF = ConvertedTempCtoF(dewpointValueC);
}

void CalculateHeatIndex(double tempF, int relHumidity)
{
	calculatedHeatIndexF = heatIndexConst1 + (heatIndexConst2 * tempF) + (heatIndexConst3 * relHumidity) + (heatIndexConst4 * tempF * relHumidity) +
		(heatIndexConst5 * tempF * tempF) + (heatIndexConst6 * relHumidity * relHumidity) + (heatIndexConst7 * tempF * tempF * relHumidity) +
		(heatIndexConst8 * tempF * relHumidity * relHumidity) + (heatIndexConst9 * tempF * tempF * relHumidity * relHumidity);
	convertedHeatIndexC = ConvertedTempFtoC(calculatedHeatIndexF);
}

void CalculateWindChill(double tempF, double tempC, double windSpeedMPH, double windSpeedKPH)
{
	// First do the calculation based off of Fahrenheit/MPH units 
	float windWithCoeffecient = std::pow(windSpeedMPH, windChillCoEffecient);
	calculatedWindChillF = (windChillBaselineF + (windChillTempContrib * tempF) - (windChillSpeedFactorF1 * windWithCoeffecient) + (windChillSpeedFactorF2 * tempF * windWithCoeffecient));
	// Now do the calculation for Metric scale
	windWithCoeffecient = std::pow(windSpeedKPH, windChillCoEffecient);
	calculatedWindChillC = (windChillBaselineC + (windChillTempContrib * tempC) - (windChillSpeedFactorC1 * windWithCoeffecient) + (windChillSpeedFactorC2 * tempC * windWithCoeffecient));
}

double CalculateTrendSlope(std::deque<double>* dequeSource)
{
	double ySum = 0, xySum = 0;
	for (int x = 0; x < dequeSource->size(); x++)
	{
		xySum += (x * dequeSource->at(x));
		ySum += dequeSource->at(x);
	}
	return ((dequeSource->size() * xySum) - (xSum * ySum)) / sumsBottomEquation;
}

double ConvertedTempCtoF(double tempC)
{ return ((tempC * 1.8) + 32); }

double ConvertedTempFtoC(double tempF)
{ return ((tempF - 32) / 1.8); }

double degreesToRadians(double degrees)
{ return (degrees * (std::numbers::pi / 180.0)); }

void UpdateHighLowValues(double sourceValue, highLowRange* destHighLowRange)
{
	if ((destHighLowRange->high == undefinedValue) || (sourceValue > destHighLowRange->high))
		destHighLowRange->high = sourceValue;
	if ((destHighLowRange->low == undefinedValue) || (sourceValue < destHighLowRange->low))
		destHighLowRange->low = sourceValue;
}

bool jsonGetField(std::string keyword)
{
	//if (keyword == "id")
	//	printf("breakpoint\n");
	size_t keywordStart, fieldDivider, dataStart, dataEnd;
	keywordStart = wxDataMessage.find(keyword);
	if (keywordStart == std::string::npos)
		return false;
	
	fieldDivider = wxDataMessage.find(':', keywordStart);
	if (fieldDivider == std::string::npos)
		return false;

	dataStart = wxDataMessage.find_first_not_of(' ', fieldDivider + 1);
	if (dataStart == std::string::npos)
		return false;

	dataEnd = wxDataMessage.find(',', dataStart);
	if (dataEnd == std::string::npos)
		return false;

	dataString = wxDataMessage.substr(dataStart, (dataEnd - dataStart));

	// If value is in quotes, strip them off
	if ((dataString.front() == '\"') && (dataString.back() == '\"'))
		std::erase(dataString, '\"');

	return true;
}

void populateTestData()
{
	outdoorSensor.name = "Acurite Atlas";
	outdoorTempValueF.current = 100.2f;
	highLowOutdoorTempF.low = 53.2f;
	highLowOutdoorTempF.high = 101.7f;
	outdoorHumidityValue.current = 76;
	windDirectionValue = 220.0f;
	windSpeedValueMPH.current = 7.4;
	rainfallTotalTodayValue = 0.14;

	indoorTempValueF.current = 69.1f;
	indoorHumidityValue.current = 49;

	outdoorSensor.batteryStatus = batteryStatusNormal; 
	outdoorSensor.packetCounter = 4;
	outdoorSensor.channel = "A";

	indoorSensor.batteryStatus = batteryStatusNormal;
	indoorSensor.packetCounter = 4;
	indoorSensor.channel = "B";
}