
#define OLC_PGE_APPLICATION

#define OLC_PGEX_TTF
#ifdef __APPLE__
#include <cassert> 							// Required when compiling under macOS
#endif
#include <olcPGEX_TTF-main/olcPGEX_TTF.h>	// Includes "olcPixelGameEngine.h" automatically

#if !defined(_DEBUG)
#define OLC_PGEX_SPLASHSCREEN
#include "olcPGEX_SplashScreen.h"
#endif

#include "Main.h"
#include "DragonWx.h"

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#include <tchar.h>
#else
	#include <signal.h>
	#include <sys/wait.h>
	#ifndef __APPLE__
		#include <X11/Xlib.h>
	#endif
#endif

#include <curl/curl.h>
#include <cstdio>
#include <numeric>
#include <filesystem>

#if !defined(_DEBUG) && defined(_WIN32)
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	#ifdef __APPLE__
		userFilesDirPath = std::string(std::getenv("HOME")) + "/Library/Application Support/DragonWx/";
		assetsDirPath = fs::canonical(argv[0]).parent_path().parent_path().string() + "/Resources/";
	#else
		userFilesDirPath = "./";
		assetsDirPath = "assets/";
	#endif

	imagesDirPath = assetsDirPath + "images/";

	// Delete any error.log file if it exists from a previous a session
	if (fs::exists(userFilesDirPath + errorLogName))
	{
		if (fs::remove(userFilesDirPath + errorLogName))
			PRINT_DEBUG("Debug: %s found and deleted succesfully.\n", errorLogName.c_str());
		else
			PRINT_DEBUG("Debug: %s found but could NOT be deleted.\n", errorLogName.c_str());
	}

	invalidConfigFileState = !LoadConfigFile();
	isValidResolution = (GetSystemResolution() == olc::vi2d(1280, 720));

	assetsNotFound = CheckFileDependencies();

	// Check for any command-line flags/parameters when DragonWx was launched
	if (argc >= 3)
	{
		std::string strFlag = argv[1];

		for (char& c : strFlag)
			c = std::tolower(c);

		// -e flag allows user to specify their own raw command-line string to exec when launching rtl_433 instead of config file/app parameters
		if (strFlag == "-e")
			cliManualExecPath = argv[2];
	}

	// Start main worker thread that handles web forecast lookups and parses any pending JSON telemetry
	mainWorkerThread = std::thread(WorkerThreadHandler);
	workerThreadRunning = true;
	pendingStartRTL433 = true;

	while (appShouldStart)
	{
		PRINT_DEBUG("Info: About to launch Pixel Game Engine...\n");
		appShouldStart = false;			// By default, the app should NOT restart itself when appInstance finishes

		DragonWx appInstance;
		// Since large sections of the app layout are currently fixed pixel coordinates and size, 720p effective resolution is required.
		// So if config file specifies fullscreen but screen resolution is NOT 720p, force windowed mode. Eventually I plan to make the
		// layouts all adapative to resolution, but for now this makes sure things will look right for the user.
		if (appInstance.Construct(1280, 720, 1, 1, (fullscreenToggle && isValidResolution), true))
			appInstance.Start();
	}

	PRINT_DEBUG("Info: Pixel Game Engine exited.\n");

	if (rtl433_isRunning)
		procRTL_433->kill();

	StopWorkerThread();

	if (errorLogFile.is_open())
		errorLogFile.close();

	return 0;
}

void StripCharacters(std::string& inputString, const char* unwantedChars)
{
	for (int i = 0; i < strlen(unwantedChars); i++)
		for (auto it = inputString.begin(); it != inputString.end();)
		{
			if (*it == unwantedChars[i])
				inputString.erase(it);
			else
				++it;
		}
}

olc::vi2d GetSystemResolution()
{
	olc::vi2d effectiveResolution = { 0, 0 };

	#ifdef _WIN32
	effectiveResolution.x = GetSystemMetrics(SM_CXSCREEN);
	effectiveResolution.y = GetSystemMetrics(SM_CYSCREEN);
	#elif !defined(__APPLE__)
	using namespace X11;
	Display* displayPtr = XOpenDisplay(nullptr);
	if (displayPtr == nullptr)
	{
		PRINT_DEBUG("Error: Failed XOpenDisplay()\n");
		return effectiveResolution;
	}
	Screen* screenPtr = ScreenOfDisplay(displayPtr, DefaultScreen(displayPtr));
	effectiveResolution = { screenPtr->width, screenPtr->height };
	XCloseDisplay(displayPtr);
	#endif

	PRINT_DEBUG("Debug: Fullscreen resolution = %u x %u\n", effectiveResolution.x, effectiveResolution.y);
	return effectiveResolution;
}

bool CheckFileDependencies()
{
	bool fileWasMissing = false;
	if (!fs::exists(assetsDirPath + "fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf"))
	{
		WriteMsgToErrorLog("Error: Font file not found (" + assetsDirPath + "fonts/Archivo_Narrow/ArchivoNarrow-Regular.ttf)");
		fileWasMissing = true;
	}

	for (int i = 0; i < imageFileDependencies.size(); i++)
		if (!fs::exists(imagesDirPath + imageFileDependencies.at(i)))
		{
			WriteMsgToErrorLog("Error: Image file not found (" + imagesDirPath + imageFileDependencies.at(i) + ")");
			fileWasMissing = true;
		}

	return fileWasMissing;
}

void StartPipeRTL433()
{
	using namespace TinyProcessLib;
	int processStatus;

	if (rtl433_isRunning)
	{
		procRTL_433->kill();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		rtl433_isRunning = false;
	}

	if (!cliManualExecPath.empty())
		cliFullCommand = cliManualExecPath;
	else
	{
		if (pathToExec.find_first_not_of(" \t\r\n") != std::string::npos)		// This makes sure pathToExec does not ONLY contain whitespace (spaces, tabs, CR, LF only)
		{
			cliFullCommand = pathToExec + " -v ";
			if (!sdrExtraArguments.empty())
				cliFullCommand += sdrExtraArguments;
			if (!sdrGainSetting.empty())
				cliFullCommand += " -g" + sdrGainSetting;
			cliFullCommand += " -F json";
		}
		else
			cliFullCommand.clear();
	}

	if (!cliFullCommand.empty())
	{
		PRINT_DEBUG("Info: CLI Full Command = %s\n", cliFullCommand.c_str());
		procRTL_433 = std::make_shared<Process>(cliFullCommand, "", CallbackHandlerRTL433, CallbackHandlerRTL433, true);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	rtl433_isRunning = !procRTL_433->try_get_exit_status(processStatus);
	
}

void CallbackHandlerRTL433(const char* bytes, size_t n)
{
	wxDataMessageBuffer += std::string(bytes, n);
	size_t wxDataBufferNextEnd = wxDataMessageBuffer.find_first_of("\r\n");
	if ((wxDataBufferNextEnd != std::string::npos) && wxDataMessage.empty())
	{
		// Extract the complete CR/LF terminated JSON message from the buffer
		wxDataMessage = wxDataMessageBuffer.substr(0, wxDataBufferNextEnd) + "\r\n";
		// Now trim the complete extracted message from buffer leaving the rest intact
		wxDataBufferNextEnd = wxDataMessageBuffer.find_first_not_of("\r\n", wxDataBufferNextEnd);
		if (wxDataBufferNextEnd != std::string::npos)
			wxDataMessageBuffer = wxDataMessageBuffer.substr(wxDataBufferNextEnd);
		else
			wxDataMessageBuffer.clear();		
	}
}

bool ConvertTimeToLocal(std::tm* convertedTimePtr, std::time_t timeToConvert)
{
#ifdef WIN32
	if (localtime_s(convertedTimePtr, &timeToConvert) != 0)
		return false;
#else
	if (localtime_r(&timeToConvert, convertedTimePtr) == nullptr)
		return false;
#endif
	return true;
}

std::string GetFormattedLocalTime(std::string formatParams, std::time_t* inputTime)
{
	if (std::strftime(timeFormatCharBuffer, sizeof(timeFormatCharBuffer), formatParams.c_str(), std::localtime(inputTime)))		// Convert to local time
		return timeFormatCharBuffer;
	
	return "";
}

std::string GetTimestamp()
{
	std::time_t currentTime = std::time(nullptr);
	return GetFormattedLocalTime("%H:%M:%S", &currentTime);		// Convert to local time and format it into a string to return
}

void WorkerThreadHandler()
{
	while (workerThreadRunning)
	{
		if (webWxRequested)
		{
			webWxNewDataReady = GetWebForecast(strLocationURL, &curlResponseBuffer);
			webWxRequested = false;
		}

		if (pendingStartRTL433)
		{
			StartPipeRTL433();
			pendingStartRTL433 = false;
		}

		#ifdef _DEBUG
		if (!appDemoMode)
		#endif
		{
			if (!wxDataMessage.empty())
			{
				if (dequeLiveDebugText.size() >= 14)
					dequeLiveDebugText.pop_front();
				dequeLiveDebugText.push_back(wxDataMessage);

				//PRINT_DEBUG("Complete wxDataMessage:\n%s\n", wxDataMessage.c_str());
				if (nlohmann::json::accept(wxDataMessage))			// Check if our complete message contains valid JSON before trying to parse
				{
					jsonWxTelemetry = nlohmann::json::parse(wxDataMessage);
					// First make sure this telemetry is coming from our target weather station ID
					if (jsonWxTelemetry.contains(jsonParamID))
					{
						if (jsonWxTelemetry[jsonParamID].dump() == outdoorSensor.ID)
						{
							if (jsonWxTelemetry.contains(jsonParamTime) && jsonWxTelemetry[jsonParamTime] != outdoorPacketTimestamp.previous)
							{
								outdoorPacketTimestamp.current = jsonWxTelemetry[jsonParamTime];

								if (!outdoorSensor.telemetryStarted)
								{
									// Now that we know we are receiving live telemetry, do some init stuff
									PRINT_DEBUG("%s Outdoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
									elapsedTimeCounter = 0.0f;			// To hopefully syncronize the PGE's timing loop with the internal clock on weather station sensor
									outdoorSensor.telemetryStarted = true;
								}

								if (jsonWxTelemetry.contains(jsonParamSequenceNum))
									//packetSequenceNum = std::stoi(jsonParameterValue);
									packetSequenceNum = jsonWxTelemetry[jsonParamSequenceNum].get<int>();
								PRINT_DEBUG("Debug: Packet Sequence Number = %u\n", packetSequenceNum);

								if (jsonWxTelemetry.contains(jsonParamModel))
								{
									outdoorSensor.name = jsonWxTelemetry[jsonParamModel];
									PRINT_DEBUG("%s Outdoor Sensor: %s\n", GetTimestamp().c_str(), outdoorSensor.name.c_str());
								}

								if (jsonWxTelemetry.contains(jsonParamChannel))
									outdoorSensor.channel = jsonWxTelemetry[jsonParamChannel];

								if (jsonWxTelemetry.contains(jsonParamTempC))
								{
									outdoorSensor.temperature.Update(jsonWxTelemetry[jsonParamTempC], metricUnits);
									CalculateFeelsLikeMetrics();
									PRINT_DEBUG("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorSensor.temperature.current.imperial);
								}

								if (jsonWxTelemetry.contains(jsonParamTempF))
								{
									outdoorSensor.temperature.Update(jsonWxTelemetry[jsonParamTempF], imperialUnits);
									CalculateFeelsLikeMetrics();
									PRINT_DEBUG("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorSensor.temperature.current.imperial);
								}

								if (jsonWxTelemetry.contains(jsonParamHumidity))
								{
									outdoorSensor.humidity.Update(jsonWxTelemetry[jsonParamHumidity]);
									CalculateFeelsLikeMetrics();
									PRINT_DEBUG("%s Outdoor Humidity: %u%%\n", GetTimestamp().c_str(), outdoorSensor.humidity.current);
								}

								if (jsonWxTelemetry.contains(jsonParamWindAvgMPH))
								{
									windSpeedValue.Update(dequeWindSpeedSamples, jsonWxTelemetry[jsonParamWindAvgMPH], imperialUnits);
									CalculateFeelsLikeMetrics();
									PRINT_DEBUG("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %lu)\n", GetTimestamp().c_str(), windSpeedValue.current.mph, windSpeedValue.average.mph, dequeWindSpeedSamples.size());
								}

								if (jsonWxTelemetry.contains(jsonParamWindAvgKPH))
								{
									windSpeedValue.Update(dequeWindSpeedSamples, jsonWxTelemetry[jsonParamWindAvgKPH], metricUnits);
									CalculateFeelsLikeMetrics();
									PRINT_DEBUG("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %lu)\n", GetTimestamp().c_str(), windSpeedValue.current.mph, windSpeedValue.average.mph, dequeWindSpeedSamples.size());
								}

								if (jsonWxTelemetry.contains(jsonParamWindDirDegrees))
								{
									if (dequeWindDirections.size() >= 3)
										dequeWindDirections.pop_back();
									dequeWindDirections.push_front(jsonWxTelemetry[jsonParamWindDirDegrees]);
									if (dequeWindDirections.size() > 1)
									{
										//windAnimationIncrement = true;
										windDirAnimatedPosition = dequeWindDirections.at(1);
									}
									PRINT_DEBUG("%s \x1b[1;96mOutdoor Wind Direction: %.0f\xF8\n\x1b[0m", GetTimestamp().c_str(), dequeWindDirections.front());
								}

								if (jsonWxTelemetry.contains(jsonParamRainInches))
								{
									rainfallSensorValue.current = jsonWxTelemetry[jsonParamRainInches];
									if (rainfallSensorValue.previous != undefinedFloatValue)
									{
										float rainfallDeltaValue = 0.0;
										if (rainfallSensorValue.current > rainfallSensorValue.previous)
											rainfallDeltaValue = (rainfallSensorValue.current - rainfallSensorValue.previous);
										else if (rainfallSensorValue.current < rainfallSensorValue.previous)
											rainfallDeltaValue = (5.12 - rainfallSensorValue.previous) + rainfallSensorValue.current;

										if (rainfallDeltaValue > 0)
										{
											if (!activeRainfallEvent)
											{
												activeRainfallEvent = true;
												rainEventStartTime = std::time(nullptr);
												strRainEventStartTime = GetFormattedLocalTime("%I:%M %p", &rainEventStartTime);
												if (strRainEventStartTime.at(0) == '0')
													strRainEventStartTime.erase(0, 1);		// Strip off any leading zeros on the hours value
												rainfallTotalEvent.SetZero();
												rainEventStopTime = 0;
												//rainfallTotalEvent.inches = 0.0;
											}

											rainfallData.Update(std::time(nullptr), rainfallDeltaValue, imperialUnits);
											rainEventLastUpdateTime = std::time(nullptr);

											rainfallTotalToday.AddValue(rainfallDeltaValue, imperialUnits);
											rainfallTotalEvent.AddValue(rainfallDeltaValue, imperialUnits);

											if (rainfallTotalToday.inches >= (rainGaugeCapacity.inches * 0.90f))
												rainGaugeCapacity.GrowCapacity();
										}

										/*
										if (dequeRainRateSamples.size() >= 20)
											dequeRainRateSamples.pop_front();
										dequeRainRateSamples.push_back(rainfallDeltaValue);
										// Below code calculates the average rainfall stored in our deque and then scales the average up to fit in a 60 minute time period
										rainfallRatePerHour.SetValue(std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0) * (120.0 / dequeRainRateSamples.size()), imperialUnits);
										PRINT_DEBUG("%s \x1b[1;34mOutdoor Rainfall Rate: %.2f in/hr (Sum = %.2f, Sample Count = %lu)\n\x1b[0m", GetTimestamp().c_str(), rainfallRatePerHour.inches, std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0), dequeRainRateSamples.size());
										PRINT_DEBUG("\x1b[1;34mRainfall Rate Samples = ");
										for (int i = 0; i < dequeRainRateSamples.size(); i++)
											PRINT_DEBUG("%.2f ", dequeRainRateSamples.at(i));
										PRINT_DEBUG("\n\x1b[0m");
										*/
									}
									rainfallSensorValue.previous = rainfallSensorValue.current;
									PRINT_DEBUG("%s \x1b[1;34mOutdoor Rainfall: %.2f inches\n\x1b[0m", GetTimestamp().c_str(), rainfallTotalToday.inches);
								}

								if (jsonWxTelemetry.contains(jsonParamStrikeCount))
								{
									if (lightningStrikeCount.Update(jsonWxTelemetry[jsonParamStrikeCount]))
										PRINT_DEBUG("%s Outdoor Lightning Strike Count: %u\n", GetTimestamp().c_str(), lightningStrikeCount.current);
								}

								if (jsonWxTelemetry.contains(jsonParamStrikeDistance))
								{
									if (lightningStrikeDistance.Update(jsonWxTelemetry[jsonParamStrikeDistance]));
									PRINT_DEBUG("%s Outdoor Lightning Strike Distance: %u\n", GetTimestamp().c_str(), lightningStrikeDistance.current);
								}

								if (jsonWxTelemetry.contains(jsonParamUvIndex))
								{
									uvIndex.Update(jsonWxTelemetry[jsonParamUvIndex]);
									PRINT_DEBUG("%s Outdoor UV Index: %u\n", GetTimestamp().c_str(), uvIndex.current);
								}

								if (jsonWxTelemetry.contains(jsonParamLux))
								{
									lightLevelLux.Update(jsonWxTelemetry[jsonParamLux]);
									PRINT_DEBUG("%s Outdoor Light Level (Lux): %u (Raw JSON: %s)\n", GetTimestamp().c_str(), lightLevelLux.current, jsonWxTelemetry["lux"].dump());
								}

								if (jsonWxTelemetry.contains(jsonParamBatteryOK))
								{
									outdoorSensor.batteryStatus = jsonWxTelemetry[jsonParamBatteryOK];
									PRINT_DEBUG("%s Outdoor Sensor Battery: %s\n", GetTimestamp().c_str(), outdoorSensor.batteryStatus ? "Normal" : "Low");
								}

								outdoorPacketTimestamp.previous = outdoorPacketTimestamp.current;
								outdoorSensor.recentlyUpdated = true;
								if (outdoorSensor.packetCounter < 1)
									outdoorSensor.packetCounter = 1;
							}
						}
						else if (jsonWxTelemetry[jsonParamID].dump() == indoorSensor.ID)
						{
							if (jsonWxTelemetry.contains(jsonParamTime) && (jsonWxTelemetry[jsonParamTime] != indoorPacketTimestamp.previous))
							{
								indoorPacketTimestamp.current = jsonWxTelemetry[jsonParamTime];
								if (!indoorSensor.telemetryStarted)
								{
									// Now that we know we are receiving live telemetry, do some init stuff
									PRINT_DEBUG("%s Indoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
									indoorSensor.telemetryStarted = true;
								}

								if (jsonWxTelemetry.contains(jsonParamModel))
								{
									//indoorSensor.name = jsonParameterValue;
									indoorSensor.name = jsonWxTelemetry[jsonParamModel];
									PRINT_DEBUG("%s Indoor Sensor: %s\n", GetTimestamp().c_str(), indoorSensor.name.c_str());
								}

								if (jsonWxTelemetry.contains(jsonParamChannel))
									indoorSensor.channel = jsonWxTelemetry[jsonParamChannel];

								if (jsonWxTelemetry.contains(jsonParamTempC))
								{
									indoorSensor.temperature.Update(jsonWxTelemetry[jsonParamTempC], metricUnits);
									PRINT_DEBUG("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorSensor.temperature.current.imperial);
								}

								if (jsonWxTelemetry.contains(jsonParamTempF))
								{
									indoorSensor.temperature.Update(jsonWxTelemetry[jsonParamTempF], imperialUnits);
									PRINT_DEBUG("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorSensor.temperature.current.imperial);
								}

								if (jsonWxTelemetry.contains(jsonParamHumidity))
								{
									indoorSensor.humidity.Update(jsonWxTelemetry[jsonParamHumidity]);
									PRINT_DEBUG("%s Indoor Humidity: %u%%\n", GetTimestamp().c_str(), indoorSensor.humidity.current);
								}

								if (jsonWxTelemetry.contains(jsonParamBatteryOK))
								{
									//indoorSensor.batteryStatus = (jsonParameterValue == "1") ? 1 : 0;
									indoorSensor.batteryStatus = jsonWxTelemetry[jsonParamBatteryOK];
									PRINT_DEBUG("%s Indoor Sensor Battery: %s\n", GetTimestamp().c_str(), indoorSensor.batteryStatus ? "Normal" : "Low");
								}

								indoorPacketTimestamp.previous = indoorPacketTimestamp.current;
								indoorSensor.recentlyUpdated = true;
								if (indoorSensor.packetCounter < 1)
									indoorSensor.packetCounter = 1;
							}
						}
					}
				}
				wxDataMessage.clear();
			}
		}
	}
	PRINT_DEBUG("Main worker thread is exiting...\n");
}

bool StopWorkerThread()
{
	if (workerThreadRunning && mainWorkerThread.joinable())
	{
		workerThreadRunning = false;
		mainWorkerThread.join();
		PRINT_DEBUG("Main worker thread has exited.\n");
		return true;
	}
	else
		return false;
}

void CalculateFeelsLikeMetrics()
{
	float tempC = outdoorSensor.temperature.current.GetValue(metricUnits);
	float tempF = outdoorSensor.temperature.current.GetValue(imperialUnits);
	int relativeHumidity = outdoorSensor.humidity.current;
	float windWithCoeffecient, windSpeedMetersPerSecond;

	// First, calculate the dewpoint (parameters needed are Temperature in Celsius, Relative Humdity, and Pressure)
	if ((tempC != undefinedFloatValue) && (relativeHumidity != undefinedFloatValue))
	{
		saturationVaporPressure = referenceVaporPressure * std::exp((magnusCoefficient * tempC) / (tempC + magnusTempOffset));
		// Next calculate the "Actual Vapor Pressure"
		actualVaporPressure = relativeHumidity * (saturationVaporPressure / 100.0f) * (curPressureValue_hPa / standardPressure_hPa);
		// Next calculate our actual dewpoint in celsius
		dewpointValue.SetValue((magnusTempOffset * std::log(actualVaporPressure / referenceVaporPressure)) / (magnusCoefficient - std::log(actualVaporPressure / referenceVaporPressure)), metricUnits);
	}

	// Next calculate the Heat Index (parameters needed are Temperature in Fahrenheit and Relative Humidity)
	if ((tempF != undefinedFloatValue) && (relativeHumidity != undefinedFloatValue))
	{
		tempFloat = heatIndexConst1 + (heatIndexConst2 * tempF) + (heatIndexConst3 * relativeHumidity) + (heatIndexConst4 * tempF * relativeHumidity) +
			(heatIndexConst5 * tempF * tempF) + (heatIndexConst6 * relativeHumidity * relativeHumidity) + (heatIndexConst7 * tempF * tempF * relativeHumidity) +
			(heatIndexConst8 * tempF * relativeHumidity * relativeHumidity) + (heatIndexConst9 * tempF * tempF * relativeHumidity * relativeHumidity);
		calculatedHeatIndex.SetValue(tempFloat, imperialUnits);
	}

	// Next calculate the Wind Chill (parameters needed are Temperature in both C and F, and Wind Speed in both MPH and KPH)
	if (!currentUnits && (tempF != undefinedFloatValue) && (windSpeedValue.current.mph != undefinedFloatValue))
	{
		windWithCoeffecient = std::pow(windSpeedValue.current.mph, windChillCoEffecient);
		calculatedWindChill = (windChillBaselineF + (windChillTempContrib * tempF) - (windChillSpeedFactorF1 * windWithCoeffecient) + (windChillSpeedFactorF2 * tempF * windWithCoeffecient));
	}
	if (currentUnits && (tempC != undefinedFloatValue) && (windSpeedValue.current.kph != undefinedFloatValue))
	{
		windWithCoeffecient = std::pow(windSpeedValue.current.kph, windChillCoEffecient);
		calculatedWindChill = (windChillBaselineC + (windChillTempContrib * tempC) - (windChillSpeedFactorC1 * windWithCoeffecient) + (windChillSpeedFactorC2 * tempC * windWithCoeffecient));
	}

	// Finally, calculate the general "Apparent" temperature used for when neither Heat Index nor Wind Chill apply
	// Note: This formula requires a valid actualVaporPressure value calculated in the Dewpoint code above
	if ((tempC != undefinedFloatValue) && (windSpeedValue.current.kph != undefinedFloatValue) && (actualVaporPressure != undefinedFloatValue))
	{
		windSpeedMetersPerSecond = (windSpeedValue.current.kph * 1000.0f) / 3600.0f;
		calculatedApparentTemp.SetValue(tempC + (actualVaporPressure * 0.33f) - (windSpeedMetersPerSecond * 0.7f) - 4.0f, metricUnits);
	}
}

float CalculateTrendSlope(std::deque<float>* dequeSource)
{
	float ySum = 0, xySum = 0;
	for (int x = 0; x < dequeSource->size(); x++)
	{
		xySum += (x * dequeSource->at(x));
		ySum += dequeSource->at(x);
	}
	return ((dequeSource->size() * xySum) - (xSum * ySum)) / sumsBottomEquation;
}

double ConvertedTempCtoF(float tempC)
{ return ((tempC * 1.8) + 32); }

double ConvertedTempFtoC(float tempF)
{ return ((tempF - 32) / 1.8); }

double degreesToRadians(double degrees)
{ return (degrees * (pi / 180.0)); }

bool LoadConfigFile()
{
	std::string inputLine, strParamName, strParamValue;
	std::ifstream configFile(userFilesDirPath + configFileName);
	size_t paramNameEndIndex, paramValueIndex;

	if (!configFile.is_open())
	{
		PRINT_DEBUG("Error: Could not open config file.\n");
		return false;
	}

	while (std::getline(configFile, inputLine))
	{
		paramNameEndIndex = inputLine.find_first_of("\t= ");
		if (paramNameEndIndex != std::string::npos)
		{
			paramValueIndex = inputLine.find_first_not_of("\t= ", paramNameEndIndex + 1);
			if (paramValueIndex != std::string::npos)
			{
				strParamName = inputLine.substr(0, paramNameEndIndex);
				strParamValue = inputLine.substr(paramValueIndex, inputLine.size() - paramValueIndex);
				//std::erase(strParamValue, '\r');	// Strip out CR's
				//std::erase(strParamValue, '\n');	// Strip out LF's
				//std::erase(strParamValue, '\"');	// Strip out quotes
				StripCharacters(strParamValue, "\r\n\"");

				for (int i = 0; i < std::max(configFileParams.size(), configFileJsonParams.size()); i++)
				{
					if ((i < configFileParams.size()) && (strParamName == configFileParams[i].keyword))
					{
						switch (configFileParams[i].varPtr.index())
						{
						case 0:
							*std::get<bool*>(configFileParams[i].varPtr) = std::stoi(strParamValue);
							break;
						case 1:
							*std::get<std::string*>(configFileParams[i].varPtr) = strParamValue;
							break;
						case 2:
							*std::get<int*>(configFileParams[i].varPtr) = std::stoi(strParamValue);
							break;
						case 3:
							std::get<tempOffsetValuePair*>(configFileParams[i].varPtr)->SetValue(std::stod(strParamValue), metricUnits);
							break;
						default:
							break;			// Skips the printf() statement below and moves on to next item in while() loop
						}
					}
					else if ((configFileVersion > 1) && (i < configFileJsonParams.size()) &&
						(strParamName == configFileJsonParams[i].keyword) && (configFileJsonParams[i].varPtr.index() == 1))
							*std::get<std::string*>(configFileJsonParams[i].varPtr) = strParamValue;
				}
				PRINT_DEBUG("Debug: Loaded config param %s\n", strParamName.c_str());
			}
		}
	}

	if (webWxLocationLat.empty() || webWxLocationLon.empty())
		webWxEnabled = false;

	return true;
}

bool SaveConfigFile()
{
	std::ofstream configFile(userFilesDirPath + configFileName);
	size_t equalsSymbolIndex, paramValueIndex, paramNameEndIndex;

	if (!configFile.is_open())
	{
		PRINT_DEBUG("Error: Could not write to config file.\n");
		return false;
	}

	configFile << "-= DragonWx Config File =-" << std::endl << std::endl;
	configFileVersion = 2;

	for (int i = 0; i < configFileParams.size(); i++)
	{
		configFile << configFileParams[i].keyword << configFileParams[i].padding;

		switch (configFileParams[i].varPtr.index())
		{
		case 0:
			configFile << *std::get<bool*>(configFileParams[i].varPtr) << std::endl;
			break;
		case 1:
			configFile << *std::get<std::string*>(configFileParams[i].varPtr) << std::endl;
			break;
		case 2:
			configFile << *std::get<int*>(configFileParams[i].varPtr) << std::endl;
			break;
		case 3:
			configFile << std::get<tempOffsetValuePair*>(configFileParams[i].varPtr)->GetValue(metricUnits) << std::endl;
			break;
		}

		if ((configFileParams[i].keyword == "CONFIG_VERSION") || (configFileParams[i].keyword == "STATION_NAME") ||
			(configFileParams[i].keyword == "SDR_GAIN") || (configFileParams[i].keyword == "OUTDOOR_HUMIDITY_OFFSET") ||
			(configFileParams[i].keyword == "INDOOR_HUMIDITY_OFFSET"))
			configFile << std::endl;		// Add an extra line between relevant sections
	}

	if (configFileVersion > 1)
	{
		configFile << std::endl << "; RTL_433 JSON Keywords" << std::endl << std::endl;
		for (int i = 0; i < configFileJsonParams.size(); i++)
			configFile << configFileJsonParams[i].keyword << configFileJsonParams[i].padding << *std::get<std::string*>(configFileJsonParams[i].varPtr) << std::endl;
	}

	configFile.close();

	return true;
}

#ifdef _DEBUG
void populateDemoData()
{
	demoAppTime = 1748462051;
	webWxLocationLat = "41.8907";
	webWxLocationLon = "-71.3923";

	outdoorSensor.name = "Acurite Atlas";
	outdoorSensor.temperature.current.SetValue(67.2f, imperialUnits);
	outdoorSensor.temperature.low.SetValue(53.2f, imperialUnits);
	outdoorSensor.temperature.high.SetValue(75.7f, imperialUnits);
	outdoorSensor.humidity.current = 89;
	outdoorSensor.humidity.low = 44;
	outdoorSensor.humidity.high = 89;

	dequeWindDirections.push_back(220.0f);
	dequeWindDirections.push_back(190.0f);
	dequeWindDirections.push_back(210.0f);
	windSpeedValue.current.SetValue(7.4, imperialUnits);
	windSpeedValue.average.SetValue(4, imperialUnits);
	windSpeedValue.peak.SetValue(13, imperialUnits);

	rainfallTotalToday.inches = 0.64;
	rainfallData.rainfallRate.SetValue(0.70, imperialUnits);
	rainEventStartTime = 1748456666;
	strRainEventStartTime = GetFormattedLocalTime("%I:%M %p", &rainEventStartTime);
	if (strRainEventStartTime.at(0) == '0')
		strRainEventStartTime.erase(0, 1);		// Strip off any leading zeros on the hours value

	uvIndex.Update(2);
	lightLevelLux.Update(9000);

	indoorSensor.temperature.current.SetValue(69.3f, imperialUnits);
	indoorSensor.humidity.current = 49;

	outdoorSensor.batteryStatus = batteryStatusNormal; 
	outdoorSensor.packetCounter = 4;
	outdoorSensor.channel = "A";

	indoorSensor.batteryStatus = batteryStatusNormal;
	indoorSensor.packetCounter = 4;
	indoorSensor.channel = "B";
}
#endif

bool GetWebForecast(std::string url, std::string* curlOutputBufferPtr)
{
	std::time_t epochTime = 0;

	#ifdef _DEBUG
	if (!useRealWebRequests)
		ReadFileJSON("meteo-epoch.out", curlOutputBufferPtr);
	else
	#endif
	{
		PRINT_DEBUG("Requesting Web Forecast from URL: %s\n", url.c_str());
		CURL* curl = curl_easy_init();
		if (!curl)
		{
			PRINT_DEBUG("Failed to initialize libcurl.\n");
			return false;
		}
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());		// Set URL
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);		// Write directly to file
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteOutCurlResponse);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, curlOutputBufferPtr);
		//curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);		// Fail on HTTP errors (e.g., 404)
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);		// Follow redirects
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);		// Enable SSL verification
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode res = curl_easy_perform(curl);  // Execute request
		if (res != CURLE_OK)
		{
			long http_code = 0;
			PRINT_DEBUG("Download failed: %s\n", curl_easy_strerror(res));
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			PRINT_DEBUG("HTTP Error Code: %ld\n", http_code);
			curl_easy_cleanup(curl);  // Clean up
			return false;
		}
		curl_easy_cleanup(curl);  // Clean up
	}

	if (nlohmann::json::accept(*curlOutputBufferPtr))
	{
		nlohmann::json jsonWxWebReply = nlohmann::json::parse(*curlOutputBufferPtr);

		if (!jsonWxWebReply["current"].contains("weather_code"))
			return false;
		if (!jsonWxWebReply["current"].contains("is_day"))
			return false;
		if (!jsonWxWebReply["daily"].contains("weather_code"))
			return false;
		if (!jsonWxWebReply["daily"].contains("time"))
			return false;
		if (!jsonWxWebReply["daily"].contains("sunrise"))
			return false;
		if (!jsonWxWebReply["daily"].contains("sunset"))
			return false;

		webWxCurrentConditions.code = jsonWxWebReply["current"]["weather_code"];
		webWxCurrentConditions.useDaytime = jsonWxWebReply["current"]["is_day"].get<int>();
		tempString = (webWxCurrentConditions.useDaytime ? wxCodeTableNew[webWxCurrentConditions.code].descriptionDay : wxCodeTableNew[webWxCurrentConditions.code].descriptionNight);
		PRINT_DEBUG("Web Wx: Current Conditions = %s (%s)\n", tempString.c_str(), (webWxCurrentConditions.useDaytime ? "Day" : "Night"));

		for (int i = 0; i < jsonWxWebReply["daily"]["time"].size(); i++)
		{
			//if (!ConvertTimeToLocal(&webWxDailyForecasts[i].dateTime, jsonWxWebReply["daily"]["time"][i]))
			//	return false;
			epochTime = jsonWxWebReply["daily"]["time"][i].get<int64_t>();
			PRINT_DEBUG("Day %u Epoch Time = %lld\r\n", i, epochTime);
			webWxDailyForecasts[i].dateTime = *std::localtime(&epochTime);
			webWxDailyForecasts[i].sunrise = jsonWxWebReply["daily"]["sunrise"][i];
			webWxDailyForecasts[i].sunset = jsonWxWebReply["daily"]["sunset"][i];
			webWxDailyForecasts[i].code = jsonWxWebReply["daily"]["weather_code"][i];
			webWxDailyForecasts[i].useDaytime = true;
			webWxDailyForecasts[i].tempMin = jsonWxWebReply["daily"]["temperature_2m_min"][i];
			webWxDailyForecasts[i].tempMax = jsonWxWebReply["daily"]["temperature_2m_max"][i];
			webWxDailyForecasts[i].precipPercent = jsonWxWebReply["daily"]["precipitation_probability_max"][i];
		}
		PRINT_DEBUG("Web Wx: Forecast Codes = [ %u, %u, %u ]\n", webWxDailyForecasts[0].code, webWxDailyForecasts[1].code, webWxDailyForecasts[2].code);

		return true;
	}

	return false;
}

size_t WriteOutCurlResponse(char* readBufferPtr, size_t dataElementSize, size_t dataElementsReceived, std::string* outputStringBuffer)
{
	size_t totalBytesToHandle = dataElementSize * dataElementsReceived;
	outputStringBuffer->append(readBufferPtr, totalBytesToHandle);
	return totalBytesToHandle;
}

#ifdef _DEBUG
static void ReadFileJSON(std::string filePath, std::string* inputBufferPtr)
{
	std::string fileInputLine;
	std::ifstream jsonFile(filePath);
	if (!jsonFile)
	{
		PRINT_DEBUG("Error opening the JSON file.\n");
		return;
	}

	inputBufferPtr->clear();
	while (std::getline(jsonFile, fileInputLine))
		inputBufferPtr->append(fileInputLine + "\n");

	jsonFile.close();
	PRINT_DEBUG("Contents of read JSON file:\n\n%s\n", (*inputBufferPtr).c_str());
}
#endif

void WriteMsgToErrorLog(std::string outputString)
{
	if (!errorLogFile.is_open())
	{
		if (!fs::exists(userFilesDirPath))
			fs::create_directory(userFilesDirPath);
		errorLogFile.open(userFilesDirPath + errorLogName, std::ios::in | std::ios::out | std::ios::trunc);
		errorLogFile << "DragonWx encountered a problem during launch." << std::endl << std::endl;
	}

	errorLogFile << outputString << std::endl;
}
