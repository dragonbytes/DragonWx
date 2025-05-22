#pragma once

#include "Main.h"

// Debug-related variables
#ifdef _DEBUG
inline bool useRealPipe = false;
inline bool useRealWebRequests = false;
inline bool fullscreenToggle = false;
#else
inline bool useRealPipe = true;
inline bool useRealWebRequests = true;
inline bool fullscreenToggle = true;
#endif
inline bool debugKeyPressed = false;
inline bool prevKeyPressed = true;
inline bool debugState = false;
inline bool useSDRplay = true;
inline bool debugFunctionFlag = false;

// Application-related constants and variables
inline std::fstream errorLogFile;
inline std::string errorLogFilename = "error.log";
inline char buffer[512], strDateWeekMonthDay[64], strFormattedTime[16], timeFormatCharBuffer[64];
inline std::string pathToExec, tempString, wxDataMessage, strFullyFormattedDate, strFullyFormattedTime, strRainEventStartTime, strRainEventStopTime;
inline std::string sdrExtraArguments, sdrGainSetting, sdrAntennaSetting, strWxStationName, webWxLocationLat, webWxLocationLon;                   
inline std::u32string tempString32;
inline constexpr bool metricUnits = true;
inline constexpr bool imperialUnits = false;
inline bool rtl433_failedExecState = false;
inline bool rtl433_pipeIsRunning = false;
inline bool appExitRequested = false;
inline bool appShouldExit = false;
inline bool rtl433_threadRunning = false;
inline bool activeRainfallEvent = false;
inline bool currentUnits = false;
inline bool webWxEnabled = false;
inline bool settingsPageIsForeground = false;
inline bool infoPageIsForeground = false;
inline bool invalidConfigFileState = false;
inline bool appInitFailed, assetsNotFound = false;
inline double tempFloat;
inline int packetSequenceNum = -1;
inline float elapsedTimeCounter = 0.0f;
inline const int rainGaugeMarksTotal = 8;

// RTL_433 CLI and thread-related variables
inline unsigned long bufferLength;
#if defined(_WIN32) && defined(USE_WINDOWS_PIPE)
inline ProcessHandle procRTL_433;
#else
inline FILE* pipeRTL_433;
#endif
inline std::thread rtl433_thread;
inline std::string cliFullCommand;
inline nlohmann::json jsonWxTelemetry;

// Dewpoint-related constants and variables
inline const float referenceVaporPressure = 6.112f;
inline const float magnusCoefficient = 17.67f;
inline const float magnusTempOffset = 243.5f;
inline const float eulersNumber = 2.718f;
inline const float standardPressure_hPa = 1013.25f;
inline float convertedTempC = undefinedFloatValue, saturationVaporPressure = undefinedFloatValue, actualVaporPressure = undefinedFloatValue, calculatedDewpointC = undefinedFloatValue;
inline tempUnitsPairStruct dewpointValue;

// Wind Chill related constants and variables
inline const float windChillCoEffecient = 0.16f;
inline const float windChillBaselineF = 35.74f;
inline const float windChillBaselineC = 13.12f;
inline const float windChillTempContrib = 0.6215f;
inline const float windChillSpeedFactorF1 = 35.75f;
inline const float windChillSpeedFactorF2 = 0.4275f;
inline const float windChillSpeedFactorC1 = 11.37f;
inline const float windChillSpeedFactorC2 = 0.3965f;
inline float calculatedWindChill;

// Heat Index related constants and variables
inline const float heatIndexConst1 = -42.379f;
inline const float heatIndexConst2 = 2.04901523f;
inline const float heatIndexConst3 = 10.14333127f;
inline const float heatIndexConst4 = -0.22475541f;
inline const float heatIndexConst5 = -6.83783 * std::pow(10.0f, -3.0f);
inline const float heatIndexConst6 = -5.481717 * std::pow(10.0f, -2.0f);
inline const float heatIndexConst7 = 1.22874 * std::pow(10.0f, -3.0f);
inline const float heatIndexConst8 = 8.5282 * std::pow(10.0f, -4.0f);
inline const float heatIndexConst9 = -1.99 * std::pow(10.0f, -6.0f);
inline tempUnitsPairStruct calculatedHeatIndex;

// "Apprent" Temperature related variables
inline tempUnitsPairStruct calculatedApparentTemp;

// Wireless Sensor related variables
inline sensorStatus outdoorSensor, indoorSensor;
inline stringPrevCur outdoorPacketTimestamp, indoorPacketTimestamp;
inline std::string strWindDirectionName;
inline windSpeedStruct windSpeedValue;                                                      // Holds all the output values and functions that calculate them
inline std::deque<float> dequeWindSpeedSamples;                                             // For calculating average and peak wind speed
inline std::deque<float> dequeWindDirections, dequeOutdoorTemps, dequeOutdoorHumidity;      // For wind direction history, and outdoor temp and humidity trend calculations
inline float windDirAnimatedPosition, windDirHalfDistance, windDirDistanceLeft, windDirAnimatedSpeed;
inline bool windAnimationIsMoving = false, windAnimationReversed = false, windAnimationDirPositive;
inline floatPrevCur rainfallSensorValue = { undefinedFloatValue, undefinedFloatValue };
inline rainfallAmountValuePair rainfallTotalToday, rainfallTotalEvent;
//inline rainfallAmountValuePair rainfallRatePerHour;
//inline floatPrevCur rainfallDataValueInches = { undefinedFloatValue, undefinedFloatValue };
//inline floatRainUnits rainfallTotalToday = { 0.0, 0.0 }, rainfallTotalEvent = { 0.0, 0.0 };
inline rainGaugeStruct rainGaugeCapacity;
//inline std::deque<float> dequeRainRateSamples;
inline rainfallRateStruct rainfallData;
inline std::time_t rainEventStartTime = 0, rainEventStopTime = 0, rainEventLastUpdateTime = 0;
inline const std::time_t rainEventClearedInterval = (120 * 60);      // Measured in seconds
inline const std::time_t rainRateClearedInterval = (45 * 60);        // Measured in seconds
inline intRangeStruct lightningStrikeCount, lightningStrikeDistance, uvIndex, lightLevelLux;

inline const int batteryStatusLow = 0;
inline const int batteryStatusNormal = 1;

// Trend-related constants and variables
inline const int trendSampleSize = 60 * 3;     // Measured at 1 sample per minute
inline const int trendingUp = 1;
inline const int trendingSteady = 0;
inline const int trendingDown = -1;
//inline floatPrevCur outdoorTempTrendSample;
//inline intPrevCur intTrendSample;
inline double doubleTempDelta;
inline int intTrendCountUp = 0, intTrendCountSteady = 0, intTrendCountDown = 0;
//inline intPrevCur trendDirOutdoorTemp = { trendingSteady, trendingSteady };
//inline intPrevCur trendDirIndoorTemp = { trendingSteady, trendingSteady };
inline double sumsBottomEquation, xSum = 0, xSumSquare = 0;

inline std::array<configEntry, 15> configFileParams = {{
    { "UNITS", "\t\t\t\t\t\t", &currentUnits },                             { "FULLSCREEN", "\t\t\t\t\t", &fullscreenToggle },
    { "STATION_NAME", "\t\t\t\t", &strWxStationName },                      { "RTL433_PATH", "\t\t\t\t\t", &pathToExec },
    { "RTL433_PARAMS", "\t\t\t\t", &sdrExtraArguments },                    { "SDR_GAIN", "\t\t\t\t\t", &sdrGainSetting },
    { "OUTDOOR_SENSOR_ID", "\t\t\t", &outdoorSensor.ID },                   { "OUTDOOR_TEMP_OFFSET_C", "\t\t", &outdoorSensor.temperature.offset },
    { "OUTDOOR_HUMIDITY_OFFSET", "\t\t", &outdoorSensor.humidity.offset },  { "INDOOR_SENSOR_ID", "\t\t\t", &indoorSensor.ID },
    { "INDOOR_TEMP_OFFSET_C", "\t\t", &indoorSensor.temperature.offset},    { "INDOOR_HUMIDITY_OFFSET", "\t\t", &indoorSensor.humidity.offset },
    { "USE_WEB_FORECAST", "\t\t\t", &webWxEnabled },                        { "LOCATION_LAT", "\t\t\t\t", &webWxLocationLat },
    { "LOCATION_LON", "\t\t\t\t", &webWxLocationLon } }};

// Network-based telemetry variables
inline bool webWxIsDaylight = true;
inline bool webWxRequested = false, webWxNewDataReady = false;
inline double curPressureValue_hPa = standardPressure_hPa;
inline std::string strLocationURL, curlResponseBuffer;
inline wxWebEntry webWxCurrentConditions, webWxDailyForecasts[3];

// Weather Code lookup table
inline wxCodeStruct wxCodeTable[100] = {
    { "Sunny", "Clear", "wsymbol_0001_sunny", "wsymbol_0008_clear_sky_night" },                                                 // 00
    { "Mostly Sunny", "Mostly Clear", "wsymbol_0001_sunny", "wsymbol_0008_clear_sky_night" },
    { "Partly Cloudy", "Partly CLoudy", "wsymbol_0002_sunny_intervals", "wsymbol_0041_partly_cloudy_night" },
    { "Overcast", "Overcast", "wsymbol_0003_white_cloud", "wsymbol_0003_white_cloud" },
    { "", "", "wsymbol_0055_smoke", "wsymbol_0073_smoke_night" },
    { "", "", "wsymbol_0005_hazy_sun", "wsymbol_0063_mist_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "Mist", "Mist", "wsymbol_0006_mist", "wsymbol_0063_mist_night" },                                                         // 10
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0060_windy", "wsymbol_0078_windy_night" },
    { "", "", "wsymbol_0079_tornado", "wsymbol_0079_tornado" },
    { "", "", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },                                                           // 20
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0049_freezing_drizzle", "wsymbol_0067_freezing_drizzle_night" },
    { "", "", "wsymbol_0009_light_rain_showers", "wsymbol_0025_light_rain_showers_night" },
    { "", "", "wsymbol_0011_light_snow_showers", "wsymbol_0027_light_snow_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },                                                       // 30
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },                                                                   // 40
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "Light Drizzle", "Light Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },                                 // 50
    { "Light Drizzle", "Light Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Drizzle", "Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Drizzle", "Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Heavy Drizzle", "Heavy Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Heavy Drizzle", "Heavy Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Freezing Drizzle", "Freezing Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Freezing Drizzle", "Freezing Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Light Drizzle", "Light Drizzle", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "Heavy Drizzle", "Heavy Drizzle", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "Light Rain", "Light Rain", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },         // 60
    { "Light Rain", "Light Rain", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "Rain", "Rain", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "Rain", "Rain", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "Heavy Rain", "Heavy Rain", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "Heavy Rain", "Heavy Rain", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "Freezing Rain", "Freezing Rain", "wsymbol_0050_freezing_rain", "wsymbol_0068_freezing_rain_night" },
    { "Freezing Rain", "Freezing Rain", "wsymbol_0050_freezing_rain", "wsymbol_0068_freezing_rain_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "Light Snow", "Light Snow", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },         // 70
    { "Light Snow", "Light Snow", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow", "Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Snow", "Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Heavy Snow", "Heavy Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Heavy Snow", "Heavy Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Diamond Dust", "Diamond Dust", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow Grains", "Snow Grains", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow Crystals", "Snow Crystals", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Ice Pellets", "Ice Pellets", "wsymbol_0022_cloudy_with_light_hail", "wsymbol_0038_cloudy_with_light_hail_night" },
    { "", "", "wsymbol_0009_light_rain_showers", "wsymbol_0025_light_rain_showers_night" },                                     // 80
    { "", "", "wsymbol_0010_heavy_rain_showers", "wsymbol_0028_heavy_snow_showers_night" },
    { "", "", "wsymbol_0085_extreme_rain_showers", "wsymbol_0086_extreme_rain_showers_night" },
    { "", "", "wsymbol_0013_sleet_showers", "wsymbol_0029_sleet_showers_night" },
    { "", "", "wsymbol_0013_sleet_showers", "wsymbol_0029_sleet_showers_night" },
    { "", "", "wsymbol_0011_light_snow_showers", "wsymbol_0027_light_snow_showers_night" },
    { "", "", "wsymbol_0012_heavy_snow_showers", "wsymbol_0028_heavy_snow_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0015_heavy_hail_showers", "wsymbol_0031_heavy_hail_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0015_heavy_hail_showers", "wsymbol_0031_heavy_hail_showers_night" },
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0059_thunderstorms_with_hail", "wsymbol_0077_thunderstorms_with_hail_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0059_thunderstorms_with_hail", "wsymbol_0077_thunderstorms_with_hail_night" }
};

// Weather Code lookup table
inline wxCodeStruct wxCodeTableNew[100] = {
    { "Sunny", "Clear", "clear-day", "clear-night" },                                                 // 00
    { "Mostly Sunny", "Mostly Clear", "clear-day", "clear-night" },
    { "Partly Cloudy", "Partly CLoudy", "partly-cloudy-day", "partly-cloudy-night" },
    { "Overcast", "Overcast", "overcast", "overcast" },
    { "", "", "smoke", "smoke" },
    { "", "", "haze-day", "haze-night" },
    { "", "", "dust-day", "dust-night" },
    { "", "", "dust-day", "dust-night" },
    { "", "", "dust-day", "dust-night" },
    { "", "", "dust-day", "dust-night" },
    { "Mist", "Mist", "mist", "mist" },                                                         // 10
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "rain", "rain" },
    { "", "", "rain", "rain" },
    { "", "", "rain", "rain" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0060_windy", "wsymbol_0078_windy_night" },
    { "", "", "wsymbol_0079_tornado", "wsymbol_0079_tornado" },
    { "", "", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },                                                           // 20
    { "", "", "rain", "rain" },
    { "", "", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0049_freezing_drizzle", "wsymbol_0067_freezing_drizzle_night" },
    { "", "", "wsymbol_0009_light_rain_showers", "wsymbol_0025_light_rain_showers_night" },
    { "", "", "wsymbol_0011_light_snow_showers", "wsymbol_0027_light_snow_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0007_fog", "wsymbol_0064_fog_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },                                                       // 30
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0056_dust_sand", "wsymbol_0074_dust_sand_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "", "", "wsymbol_0053_blowing_snow", "wsymbol_0071_blowing_snow_night" },
    { "Fog", "Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },                                                       // 40
    { "Patchy Fog", "Patchy Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Light Fog", "Light Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Light Fog", "Light Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Steady Fog", "Steady Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Steady Fog", "Steady Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Heavy Fog", "Heavy Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Heavy Fog", "Heavy Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Fog", "Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Fog", "Fog", "partly-cloudy-day-fog", "partly-cloudy-night-fog" },
    { "Light Drizzle", "Light Drizzle", "drizzle", "drizzle" },                                 // 50
    { "Light Drizzle", "Light Drizzle", "drizzle", "drizzle" },
    { "Drizzle", "Drizzle", "drizzle", "drizzle" },
    { "Drizzle", "Drizzle", "drizzle", "drizzle" },
    { "Heavy Drizzle", "Heavy Drizzle", "extreme-drizzle", "extreme-drizzle" },
    { "Heavy Drizzle", "Heavy Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Freezing Drizzle", "Freezing Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Freezing Drizzle", "Freezing Drizzle", "wsymbol_0048_drizzle", "wsymbol_0066_drizzle_night" },
    { "Light Drizzle", "Light Drizzle", "drizzle", "drizzle" },
    { "Heavy Drizzle", "Heavy Drizzle", "extreme-day-drizzle", "extreme-night-drizzle" },
    { "Light Rain", "Light Rain", "partly-cloudy-day-rain", "partly-cloudy-night-rain" },         // 60
    { "Light Rain", "Light Rain", "partly-cloudy-day-rain", "partly-cloudy-night-rain" },
    { "Rain", "Rain", "rain", "rain" },
    { "Rain", "Rain", "rain", "rain" },
    { "Heavy Rain", "Heavy Rain", "extreme-rain", "extreme-rain" },
    { "Heavy Rain", "Heavy Rain", "extreme-rain", "extreme-rain" },
    { "Freezing Rain", "Freezing Rain", "wsymbol_0050_freezing_rain", "wsymbol_0068_freezing_rain_night" },
    { "Freezing Rain", "Freezing Rain", "wsymbol_0050_freezing_rain", "wsymbol_0068_freezing_rain_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "Light Snow", "Light Snow", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },         // 70
    { "Light Snow", "Light Snow", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow", "Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Snow", "Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Heavy Snow", "Heavy Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Heavy Snow", "Heavy Snow", "wsymbol_0020_cloudy_with_heavy_snow", "wsymbol_0036_cloudy_with_heavy_snow_night" },
    { "Diamond Dust", "Diamond Dust", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow Grains", "Snow Grains", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Snow Crystals", "Snow Crystals", "wsymbol_0019_cloudy_with_light_snow", "wsymbol_0035_cloudy_with_light_snow_night" },
    { "Ice Pellets", "Ice Pellets", "wsymbol_0022_cloudy_with_light_hail", "wsymbol_0038_cloudy_with_light_hail_night" },
    { "", "", "wsymbol_0009_light_rain_showers", "wsymbol_0025_light_rain_showers_night" },                                     // 80
    { "", "", "wsymbol_0010_heavy_rain_showers", "wsymbol_0028_heavy_snow_showers_night" },
    { "", "", "wsymbol_0085_extreme_rain_showers", "wsymbol_0086_extreme_rain_showers_night" },
    { "", "", "wsymbol_0013_sleet_showers", "wsymbol_0029_sleet_showers_night" },
    { "", "", "wsymbol_0013_sleet_showers", "wsymbol_0029_sleet_showers_night" },
    { "", "", "wsymbol_0011_light_snow_showers", "wsymbol_0027_light_snow_showers_night" },
    { "", "", "wsymbol_0012_heavy_snow_showers", "wsymbol_0028_heavy_snow_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0015_heavy_hail_showers", "wsymbol_0031_heavy_hail_showers_night" },
    { "", "", "wsymbol_0014_light_hail_showers", "wsymbol_0030_light_hail_showers_night" },
    { "", "", "wsymbol_0015_heavy_hail_showers", "wsymbol_0031_heavy_hail_showers_night" },
    { "", "", "wsymbol_0017_cloudy_with_light_rain", "wsymbol_0033_cloudy_with_light_rain_night" },
    { "", "", "wsymbol_0018_cloudy_with_heavy_rain", "wsymbol_0034_cloudy_with_heavy_rain_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0021_cloudy_with_sleet", "wsymbol_0037_cloudy_with_sleet_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0059_thunderstorms_with_hail", "wsymbol_0077_thunderstorms_with_hail_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0024_thunderstorms", "wsymbol_0040_thunderstorms_night" },
    { "", "", "wsymbol_0059_thunderstorms_with_hail", "wsymbol_0077_thunderstorms_with_hail_night" }
};

/*
std::string wxCodesWMO[100] = {
    "Clear",                      	// 0
    "Mostly Clear",               	// 1
    "Partly Cloudy",              	// 2
    "Overcast",                   	// 3
    "Smoke/Haze",                 	// 4
    "Haze",                       	// 5
    "Dust",                       	// 6
    "Blowing Dust/Sand",          	// 7
    "Dust Whirls",                	// 8
    "Sandstorm",                  	// 9
    "Mist",                       	// 10
    "Shallow Fog",                	// 11
    "Shallow Fog",                	// 12
    "Distant Lightning",          	// 13
    "Distant Rain",               	// 14
    "Distant Showers",            	// 15
    "Distant Thunderstorm",       	// 16
    "Thunderstorm",               	// 17
    "Squalls",                    	// 18
    "Funnel Cloud",               	// 19
    "Blowing Snow",               	// 20
    "Blowing Snow",               	// 21
    "Blizzard",                   	// 22
    "Blowing Dust",               	// 23
    "Patchy Fog",                 	// 24
    "Light Fog",                  	// 25
    "Dense Fog",                  	// 26
    "Fog Dissipating",            	// 27
    "Fog Forming",                	// 28
    "Past Thunderstorm",          	// 29
    "Past Sandstorm",             	// 30
    "Growing Sandstorm",          	// 31
    "Fading Sandstorm",           	// 32
    "Severe Sandstorm",           	// 33
    "Intensifying Sandstorm",     	// 34
    "Weakening Sandstorm",        	// 35
    "Distant Fog",                	// 36
    "Patchy Fog",                 	// 37
    "Thick Fog",                  	// 38
    "Light Fog",                  	// 39
    "Dense Fog",                  	// 40
    "Fog",                        	// 41
    "Thin Fog",                   	// 42
    "Thick Fog",                  	// 43
    "Icy Fog",                    	// 44
    "Light Drizzle",              	// 45
    "Moderate Drizzle",           	// 46
    "Heavy Drizzle",              	// 47
    "Light Freezing Drizzle",     	// 48
    "Heavy Freezing Drizzle",     	// 49
    "Light Rain",                 	// 50
    "Moderate Rain",              	// 51
    "Heavy Rain",                 	// 52
    "Light Freezing Rain",        	// 53
    "Heavy Freezing Rain",        	// 54
    "Rain/Snow Mix",              	// 55
    "Heavy Rain/Snow Mix",        	// 56
    "Light Snow",                 	// 57
    "Moderate Snow",              	// 58
    "Heavy Snow",                 	// 59
    "Light Ice Pellets",          	// 60
    "Snow Grains",                	// 61
    "Ice Crystals",               	// 62
    "Light Showers",              	// 63
    "Moderate Showers",           	// 64
    "Heavy Showers",              	// 65
    "Light Snow Showers",         	// 66
    "Heavy Snow Showers",         	// 67
    "Light Thunderstorm",         	// 68
    "Moderate Thunderstorm",      	// 69
    "Heavy Thunderstorm",         	// 70
    "Thunderstorm with Light Hail", 	// 71
    "Thunderstorm with Moderate Hail", 	// 72
    "Thunderstorm with Heavy Hail", 	// 73
    "Violent Thunderstorm",       	// 74
    "Thunderstorm with Heavy Hail",	// 75
    "Drizzle",                    	// 76
    "Freezing Drizzle",           	// 77
    "Rain",                       	// 78
    "Heavy Rain",                 	// 79
    "Snow",                       	// 80
    "Heavy Snow",                 	// 81
    "Ice Pellets",                	// 82
    "Hail",                       	// 83
    "Small Hail",                 	// 84
    "Thunderstorm",               	// 85
    "Severe Thunderstorm",        	// 86
    "Tornado",                    	// 87
    "Hurricane",                  	// 88
    "Tropical Storm",             	// 89
    "Extreme Cold",               	// 90
    "Extreme Heat",               	// 91
    "Dust Storm",                 	// 92
    "Sandstorm",                  	// 93
    "Volcanic Ash",               	// 94
    "Windy",                      	// 95
    "Gale",                       	// 96
    "Storm",                      	// 97
    "Severe Storm",               	// 98
    "Violent Storm"               	// 99
};
*/
