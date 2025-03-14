#pragma once

//#include "DragonWx.h"
#include <string>
#include <deque>
#include <numbers>
#include <cstdio>
#include <iostream>
#include <thread>

struct sensorStatus
{
	bool recentlyUpdated;
	int packetCounter;
	int batteryStatus;
	std::string ID;
	std::string name;
	std::string channel;
};

struct wxWebEntry
{
	std::tm dateTime;
	int code;
	std::string description;
	bool useDaytime = true;
	float tempMin;
	float tempMax;
	int precipPercent;
};

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

struct intHighLowRange
{
	int high;
	int low;
	int current;
};

struct highLowRange
{
	double high;
	double low;
};

struct wxCodeStruct
{
	//std::string wwoCode;
	std::string descriptionDay;
	std::string descriptionNight;
	std::string iconFileDay;
	std::string iconFileNight;
};

std::string getOrdinalSuffix(int);
float GetUpdatedAverageDeque(std::deque<float>*, float, int);
float GetUpdatedHighDeque(std::deque<float>*, float, int);
void CalculateFeelsLikeMetrics();
void CalculateDewpoint(double, int, double);
void CalculateHeatIndex(double, int);
void CalculateWindChill(double, double, double, double);
double CalculateTrendSlope(std::deque<double>*);
double ConvertedTempCtoF(double);
double ConvertedTempFtoC(double);
double degreesToRadians(double);
void UpdateHighLowValues(double, highLowRange*);
void UpdateHighLowValues(int, intHighLowRange*);
char* GetTimestamp();
void readWeatherData();
//size_t jsonGetParameter(std::string);
//size_t jsonGetParameter(std::string, std::string, size_t);
bool LoadConfigFile();
void populateTestData();
bool ConvertTimeToLocal(std::tm*, std::time_t);
bool GetWebForecast(std::string, std::string*);
size_t WriteOutCurlResponse(char*, size_t, size_t, std::string*);
static void ReadFileJSON(std::string, std::string*);
static void TestJSON(std::string);

// Debug-related variables
bool useRealPipe = true;
bool useRealWebRequests = true;
bool useRealLocationInfo = false;
bool fullscreenToggle = false;
bool debugKeyPressed = false;
bool prevKeyPressed = true;
bool debugState = false;
bool useSDRplay = true;
bool debugMode = true;

// Application-related constants and variables
char buffer[512], strDateWeekMonthDay[64], strFormattedTime[16];
std::string tempString, wxDataMessage, strFullyFormattedDate, strFullyFormattedTime, sdrExtraArguments, sdrGainSetting, sdrAntennaSetting, strWxStationName;
std::u32string tempString32;
bool appExitRequested = false;
bool appShouldExit = false;
bool threadKeepAlive = false;
bool useMetricUnits = false;
bool newIndoorTelemetry = false;
bool newOutdoorTelemetry = false;
double tempDouble;
int packetSequenceNum = -1;
int minuteTimeCounter = 0;		// This allows me to use single 15 second interval to also handle events once per minute
int secondsCounter30 = 0;		// This allows me to use single 15 second interval to also handle events once per 30 seconds
float elapsedTimeCounter = 0.0f;
const double undefinedValue = 300;
const int rainGaugeMarksTotal = 8;
std::u32string strDegreesUnitsF = U"\u00B0F";
std::u32string strDegreesUnitsC = U"\u00B0C";
highLowRange highLowOutdoorTempF = { undefinedValue, undefinedValue };
highLowRange highLowIndoorTempF = { undefinedValue, undefinedValue };
highLowRange highLowOutdoorTempC = { undefinedValue, undefinedValue };
highLowRange highLowIndoorTempC = { undefinedValue, undefinedValue };
highLowRange highLowOutdoorHumidity = { undefinedValue, undefinedValue };
highLowRange highLowIndoorHumidity = { undefinedValue, undefinedValue };

// Dewpoint-related constants and variables
const float referenceVaporPressure = 6.112f;
const float magnusCoefficient = 17.67f;
const float magnusTempOffset = 243.5f;
const float eulersNumber = 2.718f;
const float standardPressure_hPa = 1013.25f;
float convertedTempC = undefinedValue, saturationVaporPressure = undefinedValue, actualVaporPressure = undefinedValue, calculatedDewpointC = undefinedValue;
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
double calculatedHeatIndexF = undefinedValue, calculatedHeatIndexC = undefinedValue;

// "Apprent" Temperature related variables
double calculatedApparentTempC = undefinedValue, calculatedApparentTempF = undefinedValue;

// RTL_433 CLI and thread-related variables
FILE* pipeRTL_433;
std::thread rtl433_thread;
std::string pathToExec, cliFullCommand;

// Wireless Sensor related variables
sensorStatus outdoorSensor;
sensorStatus indoorSensor;
stringPrevCur outdoorPacketTimestamp, indoorPacketTimestamp;
std::string textWindSpeedUnits, strWindDirectionName;
doublePrevCur outdoorTempValueF = { undefinedValue, undefinedValue }, outdoorTempValueC = { undefinedValue, undefinedValue };
doublePrevCur indoorTempValueF = { undefinedValue, undefinedValue }, indoorTempValueC = { undefinedValue, undefinedValue };
intPrevCur outdoorHumidityValue = { undefinedValue, undefinedValue }, indoorHumidityValue = { undefinedValue, undefinedValue };
doublePrevCur windSpeedValueMPH = { undefinedValue, undefinedValue }, windSpeedValueKPH = { undefinedValue, undefinedValue };
double windSpeedHighValueMPH = undefinedValue, windSpeedAvgValueMPH = undefinedValue, windSpeedHighValueKPH = undefinedValue, windSpeedAvgValueKPH = undefinedValue;
std::deque<float> dequeWindSpeedSamples;
std::deque<float> dequeWindDirections;
float windDirAnimatedPosition, windDirHalfDistance, windDirDistanceLeft, windDirAnimatedSpeed;
bool windAnimationIsMoving = false, windAnimationReversed = false, windAnimationDirPositive;
doublePrevCur rainfallDataValueInches = { undefinedValue, undefinedValue };
double rainfallTotalTodayValue = 0.0, rainfallRateInchesPerHour = 0.0, rainfallRateMmPerHour = 0.0, rainfallRateAverage = 0.0;
double rainGaugeCapacityIn = 1.0;
double rainGaugeCapacityMm = 25.0;
std::deque<float> dequeRainRateSamples;
intPrevCur lightningStrikeCount = { -1, -1 }, lightningStrikeDistance = { -1, -1 };
intHighLowRange uvIndex = { -1, -1, -1 }, lightLevelLux = { -1, -1, -1 };

const int batteryStatusLow = 0;
const int batteryStatusNormal = 1;

// Trend-related constants and variables
const int trendSampleSize = 120;
const int trendingUp = 1;
const int trendingSteady = 0;
const int trendingDown = -1;
doublePrevCur outdoorTempTrendSample;
intPrevCur intTrendSample;
double doubleTempDelta;
int intTrendCountUp = 0, intTrendCountSteady = 0, intTrendCountDown = 0;
intPrevCur trendDirOutdoorTemp = { trendingSteady, trendingSteady };
intPrevCur trendDirIndoorTemp = { trendingSteady, trendingSteady };
double sumsBottomEquation, xSum = 0, xSumSquare = 0;

// Network-based telemetry variables
bool webWxIsDaylight = true;
bool webWxRequested = false, webWxDataReady = false;
double curPressureValue_hPa = standardPressure_hPa;
std::string strLocationURL, curlResponseBuffer;
wxWebEntry webWxCurrentConditions, webWxDailyForecasts[3];

std::deque<double> dequeWindGustAvg, dequeOutdoorTemps, dequeIndoorTemps, dequeOutdoorHumidity, dequeIndoorHumidity;