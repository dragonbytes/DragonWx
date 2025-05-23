
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TTF
#include "./olcPGEX_TTF-main/olcPGEX_TTF.h"

#ifndef _DEBUG
#define OLC_PGEX_SPLASHSCREEN
#include "olcPGEX_SplashScreen.h"
#endif

#include "Main.h"
#include "DragonWx.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <tchar.h>
#include <curl/curl.h>
#else
#include <curl/curl.h>
#endif
#include <cstdio>
#include <numeric>
#include <filesystem>

#if !defined(_DEBUG) && defined(_WIN32)
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

int main()
{
	// If error log file exists from previous session, delete it
	if (std::filesystem::exists(errorLogFilename))
	{
		if (std::filesystem::remove(errorLogFilename))
			PRINT_DEBUG("%s found and deleted succesfully.\n", errorLogFilename.c_str());
		else
			PRINT_DEBUG("%s found but could NOT be deleted.\n", errorLogFilename.c_str());
	}

	invalidConfigFileState = !LoadConfigFile();

	// Since large sections of app layout are fixed pixel coordinates/size, 720p effective resolution is required.
	// If config file specifies fullscreen but screen resolution is NOT 720p, force windowed mode.
	// Eventually I will make the layout all adapative, but for now this makes sure things look right
	if (fullscreenToggle)
		isValidResolution = (GetSystemResolution() == olc::vi2d(1280, 720));

	if (useRealPipe && !invalidConfigFileState && !StartPipeRTL433())
	{
		PRINT_DEBUG("Failed to run external command.\n");
		rtl433_failedExecState = true;
	}
	
	if (!useRealWebRequests)
		GetWebForecast(strLocationURL, &curlResponseBuffer);

	while (!appShouldExit)
	{
		PRINT_DEBUG("About to launch Pixel Game Engine...\n");
		DragonWx demo;
		if (demo.Construct(1280, 720, 1, 1, (fullscreenToggle && isValidResolution), true))
			demo.Start();
	}

	PRINT_DEBUG("Pixel Game Engine exited.\n");

	if (rtl433_pipeIsRunning)
	{
		ClosePipeRTL433();
		StopThreadRTL433();
	}

	if (errorLogFile.is_open())
		errorLogFile.close();

	return 0;
}

olc::vi2d GetSystemResolution()
{
	olc::vi2d effectiveResolution;

	#ifdef _WIN32
	effectiveResolution.x = GetSystemMetrics(SM_CXSCREEN);
	effectiveResolution.y = GetSystemMetrics(SM_CYSCREEN);
	#else
	#endif

	return effectiveResolution;
}

bool StartPipeRTL433()
{
	if (!pathToExec.empty())
	{
		cliFullCommand = sdrExtraArguments;
		if (!sdrGainSetting.empty())
			cliFullCommand += " -g" + sdrGainSetting;
		cliFullCommand += " -R40 -F json";

		#ifdef _WIN32
		cliFullCommand = "\"" + pathToExec + "\" -v " + cliFullCommand;
		#ifdef USE_WINDOWS_PIPE
		rtl433_pipeIsRunning = StartProcess(procRTL_433, cliFullCommand.c_str());
		#else
		pipeRTL_433 = _popen(cliFullCommand.c_str(), "r");
		if (pipeRTL_433 == nullptr)
		{
			PRINT_DEBUG("Failed to run external command.\n");
			return EXIT_FAILURE;
		}
		#endif
		#else
		cliFullCommand = pathToExec + " -v " + cliFullCommand;
		pipeRTL_433 = popen(cliFullCommand.c_str(), "r");
		if (pipeRTL_433 == nullptr)
		{
			PRINT_DEBUG("Failed to run external command.\n");
			return EXIT_FAILURE;
		}
		#endif

		PRINT_DEBUG("CLI Full Command = %s\n", cliFullCommand.c_str());

		if (rtl433_pipeIsRunning)
		{
			rtl433_thread = std::thread(readWeatherData);
			rtl433_threadRunning = true;
		}
	}
	return rtl433_pipeIsRunning;
}

bool ClosePipeRTL433()
{
	#if defined(_WIN32) && defined(USE_WINDOWS_PIPE)
	rtl433_pipeIsRunning = !StopProcess(procRTL_433);
	#elif defined(_WIN32)
	_pclose(pipeRTL_433);
	#else
	pclose(pipeRTL_433);
	#endif
	return true;
}

bool GetOutputRTL433()
{
#if defined(_WIN32) && defined(USE_WINDOWS_PIPE)
	if (ReadFile(procRTL_433.hStdOutRead, buffer, sizeof(buffer) - 1, &bufferLength, NULL))
	{
		buffer[bufferLength] = '\0';		// We have to manually add a null-terminator since ReadFile() reads raw bytes, not strings
		return true;
	}
	return false;
#else
	if (fgets(buffer, sizeof(buffer), pipeRTL_433) != buffer)
		return false;
	bufferLength = strlen(buffer);
	return true;
#endif
}

#ifdef _WIN32
bool StartProcess(ProcessHandle& proc, const char* cmd)
{
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	// Create pipe for STDOUT
	HANDLE hStdOutRead, hStdOutWrite;
	if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0))
		return false;
	SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

	// Configure process startup info
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi = { 0 };

	// Start the process
	if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		CloseHandle(hStdOutRead);
		CloseHandle(hStdOutWrite);
		return false;
	}

	// Close write end of pipe (not needed in parent)
	CloseHandle(hStdOutWrite);

	// Store process handle and pipe read handle
	proc.hProcess = pi.hProcess;
	proc.hStdOutRead = hStdOutRead;

	CloseHandle(pi.hThread);
	return true;
}

bool StopProcess(ProcessHandle& proc)
{
	if (proc.hProcess) {
		TerminateProcess(proc.hProcess, 1);
		CloseHandle(proc.hProcess);
		proc.hProcess = NULL;
	}
	if (proc.hStdOutRead) {
		CloseHandle(proc.hStdOutRead);
		proc.hStdOutRead = NULL;
	}
	return true;
}
#endif

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

void readWeatherData()
{
	while (rtl433_threadRunning)
	{
		if (webWxRequested)
		{
			webWxNewDataReady = GetWebForecast(strLocationURL, &curlResponseBuffer);
			webWxRequested = false;
		}

		if (useRealPipe)
		{
			if (GetOutputRTL433() && (bufferLength > 0))
			{
				wxDataMessage += buffer;
				if ((buffer[bufferLength - 1] == 0x0A) || (buffer[bufferLength - 1] == 0x0D))
				{
					//PRINT_DEBUG("Complete wxDataMessage:\n%s\n", wxDataMessage.c_str());
					if (nlohmann::json::accept(wxDataMessage))			// Check if our complete message contains valid JSON before trying to parse
					{
						jsonWxTelemetry = nlohmann::json::parse(wxDataMessage);
						// First make sure this telemetry is coming from our target weather station ID
						if (jsonWxTelemetry.contains("id"))
						{
							if (jsonWxTelemetry["id"].dump() == outdoorSensor.ID)
							{
								if (jsonWxTelemetry.contains("time") && jsonWxTelemetry["time"] != outdoorPacketTimestamp.previous)
								{
									outdoorPacketTimestamp.current = jsonWxTelemetry["time"];

									if (!outdoorSensor.telemetryStarted)
									{
										// Now that we know we are receiving live telemetry, do some init stuff
										PRINT_DEBUG("%s Outdoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
										elapsedTimeCounter = 0.0f;			// To hopefully syncronize the PGE's timing loop with the internal clock on weather station sensor
										outdoorSensor.telemetryStarted = true;
									}

									if (jsonWxTelemetry.contains("sequence_num"))
										//packetSequenceNum = std::stoi(jsonParameterValue);
										packetSequenceNum = jsonWxTelemetry["sequence_num"].get<int>();
									PRINT_DEBUG("Debug: Packet Sequence Number = %u\n", packetSequenceNum);

									if (jsonWxTelemetry.contains("model"))
									{
										outdoorSensor.name = jsonWxTelemetry["model"];
										PRINT_DEBUG("%s Outdoor Sensor: %s\n", GetTimestamp().c_str(), outdoorSensor.name.c_str());
									}

									if (jsonWxTelemetry.contains("channel"))
										outdoorSensor.channel = jsonWxTelemetry["channel"];

									if (jsonWxTelemetry.contains("temperature_C"))
									{
										outdoorSensor.temperature.Update(jsonWxTelemetry["temperature_C"], metricUnits);
										CalculateFeelsLikeMetrics();
										PRINT_DEBUG("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorSensor.temperature.current.imperial);
									}

									if (jsonWxTelemetry.contains("temperature_F"))
									{
										outdoorSensor.temperature.Update(jsonWxTelemetry["temperature_F"], imperialUnits);
										CalculateFeelsLikeMetrics();
										PRINT_DEBUG("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), outdoorSensor.temperature.current.imperial);
									}

									if (jsonWxTelemetry.contains("humidity"))
									{
										outdoorSensor.humidity.Update(jsonWxTelemetry["humidity"]);
										CalculateFeelsLikeMetrics();
										PRINT_DEBUG("%s Outdoor Humidity: %u%%\n", GetTimestamp().c_str(), outdoorSensor.humidity.current);
									}

									if (jsonWxTelemetry.contains("wind_avg_mi_h"))
									{
										windSpeedValue.Update(dequeWindSpeedSamples, jsonWxTelemetry["wind_avg_mi_h"], imperialUnits);
										CalculateFeelsLikeMetrics();
										PRINT_DEBUG("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %lu)\n", GetTimestamp().c_str(), windSpeedValue.current.mph, windSpeedValue.average.mph, dequeWindSpeedSamples.size());
									}

									if (jsonWxTelemetry.contains("wind_avg_km_h"))
									{
										windSpeedValue.Update(dequeWindSpeedSamples, jsonWxTelemetry["wind_avg_km_h"], metricUnits);
										CalculateFeelsLikeMetrics();
										PRINT_DEBUG("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %lu)\n", GetTimestamp().c_str(), windSpeedValue.current.mph, windSpeedValue.average.mph, dequeWindSpeedSamples.size());
									}

									if (jsonWxTelemetry.contains("wind_dir_deg"))
									{
										if (dequeWindDirections.size() >= 3)
											dequeWindDirections.pop_back();
										dequeWindDirections.push_front(jsonWxTelemetry["wind_dir_deg"]);
										if (dequeWindDirections.size() > 1)
										{
											//windAnimationIncrement = true;
											windDirAnimatedPosition = dequeWindDirections.at(1);
										}
										PRINT_DEBUG("%s \x1b[1;96mOutdoor Wind Direction: %.0f\xF8\n\x1b[0m", GetTimestamp().c_str(), dequeWindDirections.front());
									}

									if (jsonWxTelemetry.contains("rain_in"))
									{
										if (!debugState)
											rainfallSensorValue.current = jsonWxTelemetry["rain_in"];
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

									if (jsonWxTelemetry.contains("strike_count"))
									{
										if (lightningStrikeCount.Update(jsonWxTelemetry["strike_count"]))
											PRINT_DEBUG("%s Outdoor Lightning Strike Count: %u\n", GetTimestamp().c_str(), lightningStrikeCount.current);
									}

									if (jsonWxTelemetry.contains("strike_distance"))
									{
										if (lightningStrikeDistance.Update(jsonWxTelemetry["strike_distance"]));
											PRINT_DEBUG("%s Outdoor Lightning Strike Distance: %u\n", GetTimestamp().c_str(), lightningStrikeDistance.current);
									}

									if (jsonWxTelemetry.contains("uv"))
									{
										uvIndex.Update(jsonWxTelemetry["uv"]);
										PRINT_DEBUG("%s Outdoor UV Index: %u\n", GetTimestamp().c_str(), uvIndex.current);
									}

									if (jsonWxTelemetry.contains("lux"))
									{
										lightLevelLux.Update(jsonWxTelemetry["lux"]);
										PRINT_DEBUG("%s Outdoor Light Level (Lux): %u (Raw JSON: %s)\n", GetTimestamp().c_str(), lightLevelLux.current, jsonWxTelemetry["lux"].dump());
									}

									if (jsonWxTelemetry.contains("battery_ok"))
									{
										outdoorSensor.batteryStatus = jsonWxTelemetry["battery_ok"];
										PRINT_DEBUG("%s Outdoor Sensor Battery: %s\n", GetTimestamp().c_str(), outdoorSensor.batteryStatus ? "Normal" : "Low");
									}

									outdoorPacketTimestamp.previous = outdoorPacketTimestamp.current;
									outdoorSensor.recentlyUpdated = true;
									if (outdoorSensor.packetCounter < 1)
										outdoorSensor.packetCounter = 1;
								}
							}
							else if (jsonWxTelemetry["id"].dump() == indoorSensor.ID)
							{
								if (jsonWxTelemetry.contains("time") && (jsonWxTelemetry["time"] != indoorPacketTimestamp.previous))
								{
									indoorPacketTimestamp.current = jsonWxTelemetry["time"];
									if (!indoorSensor.telemetryStarted)
									{
										// Now that we know we are receiving live telemetry, do some init stuff
										PRINT_DEBUG("%s Indoor Sensor: Now receiving telemetry.\n", GetTimestamp().c_str());
										indoorSensor.telemetryStarted = true;
									}

									if (jsonWxTelemetry.contains("model"))
									{
										//indoorSensor.name = jsonParameterValue;
										indoorSensor.name = jsonWxTelemetry["model"];
										PRINT_DEBUG("%s Indoor Sensor: %s\n", GetTimestamp().c_str(), indoorSensor.name.c_str());
									}

									if (jsonWxTelemetry.contains("channel"))
										indoorSensor.channel = jsonWxTelemetry["channel"];

									if (jsonWxTelemetry.contains("temperature_C"))
									{
										indoorSensor.temperature.Update(jsonWxTelemetry["temperature_C"], metricUnits);
										PRINT_DEBUG("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorSensor.temperature.current.imperial);
									}

									if (jsonWxTelemetry.contains("temperature_F"))
									{
										indoorSensor.temperature.Update(jsonWxTelemetry["temperature_F"], imperialUnits);
										PRINT_DEBUG("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp().c_str(), indoorSensor.temperature.current.imperial);
									}

									if (jsonWxTelemetry.contains("humidity"))
									{
										indoorSensor.humidity.Update(jsonWxTelemetry["humidity"]);
										PRINT_DEBUG("%s Indoor Humidity: %u%%\n", GetTimestamp().c_str(), indoorSensor.humidity.current);
									}

									if (jsonWxTelemetry.contains("battery_ok"))
									{
										//indoorSensor.batteryStatus = (jsonParameterValue == "1") ? 1 : 0;
										indoorSensor.batteryStatus = jsonWxTelemetry["battery_ok"];
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
	}
	PRINT_DEBUG("RTL433 thread is exiting...\n");
}

bool StopThreadRTL433()
{
	if (rtl433_threadRunning && rtl433_thread.joinable())
	{
		rtl433_threadRunning = false;
		rtl433_thread.join();
		PRINT_DEBUG("RTL_433 thread exited.\n");
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
{ return (degrees * (std::numbers::pi / 180.0)); }

bool LoadConfigFile()
{
	std::string inputLine, strParamName, strParamValue;
	std::ifstream configFile("DragonWx.conf");
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
				std::erase(strParamValue, '\r');	// Strip out CR's
				std::erase(strParamValue, '\n');	// Strip out LF's
				std::erase(strParamValue, '\"');	// Strip out quotes

				for (int i = 0; i < configFileParams.size(); i++)
				{
					if (strParamName == configFileParams[i].keyword)
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
							continue;			// Skips the printf() statement below and moves on to next item in while() loop
							break;
						}
					}
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
	std::ofstream configFile("DragonWx.conf");
	size_t equalsSymbolIndex, paramValueIndex, paramNameEndIndex;
	if (!configFile.is_open())
	{
		PRINT_DEBUG("Error: Could not write to config file.\n");
		return false;
	}

	configFile << "DragonWx Config File v1.0" << std::endl << std::endl;

	for (int i = 0; i < configFileParams.size(); i++)
	{
		if ((configFileParams[i].keyword == "RTL433_PATH") || (configFileParams[i].keyword == "OUTDOOR_SENSOR_ID") || 
			(configFileParams[i].keyword == "INDOOR_SENSOR_ID") || (configFileParams[i].keyword == "USE_WEB_FORECAST"))
				configFile << std::endl;		// Add an extra line between relevant sections

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
	}

	configFile.close();

	return true;
}

void populateTestData()
{
	outdoorSensor.name = "Acurite Atlas";
	outdoorSensor.temperature.current.SetValue(76.2f, imperialUnits);
	outdoorSensor.temperature.low.SetValue(53.2f, imperialUnits);
	outdoorSensor.temperature.high.SetValue(86.7f, imperialUnits);
	outdoorSensor.humidity.current = 76;
	dequeWindDirections.push_back(220.0f);
	dequeWindDirections.push_back(170.0f);
	dequeWindDirections.push_back(200.0f);
	windSpeedValue.current.SetValue(7.4, imperialUnits);
	rainfallTotalToday.inches = 0.14;
	uvIndex.Update(4);
	lightLevelLux.Update(9000);

	indoorSensor.temperature.current.SetValue(69.3f, imperialUnits);
	indoorSensor.humidity.current = 49;

	outdoorSensor.batteryStatus = batteryStatusNormal; 
	outdoorSensor.packetCounter = 4;
	outdoorSensor.channel = "A";

	indoorSensor.batteryStatus = batteryStatusNormal;
	indoorSensor.packetCounter = 4;
	indoorSensor.channel = "B";

	rainEventStartTime = std::time(nullptr);
}

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
			epochTime = jsonWxWebReply["daily"]["time"][i].get<int>();
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
		errorLogFile.open(errorLogFilename, std::ios::in | std::ios::out | std::ios::trunc);
		errorLogFile << "DragonWx encountered a problem during launch." << std::endl << std::endl;
	}

	errorLogFile << outputString << std::endl;
}