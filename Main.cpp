
#include "Main.h"
#include "WxCodeDefs.h"
#include "DragonWx.h"
#include "json.hpp"

//#include "WxCodeDefs.h"

#include <numbers>
#include <chrono>
#include <deque>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <tchar.h>
#include <curl/curl.h>
#endif
#include <cstdio>
#include <numeric>

int main()
{
	LoadConfigFile();
	if (useRealPipe)
	{
		if (!sdrExtraArguments.empty())
			cliFullCommand = sdrExtraArguments;
		if (!sdrGainSetting.empty())
			cliFullCommand += " -g" + sdrGainSetting;
		cliFullCommand += " -F json";

		#ifdef WIN32
		cliFullCommand = "\"" + pathToExec + "\" -v " + cliFullCommand;
		pipeRTL_433 = _popen(cliFullCommand.c_str(), "r");
		#else
		cliFullCommand = pathToExec + " -v " + cliFullCommand;
		pipeRTL_433 = popen(cliFullCommand.c_str(), "r");
		#endif
		if (!pipeRTL_433)
		{
			std::cout << "Failed to run external command." << std::endl;
			return 1;
		}

		printf("%s\n", cliFullCommand.c_str());
	}

	threadKeepAlive = true;
	rtl433_thread = std::thread(readWeatherData);

	while (!appShouldExit)
	{
		DragonWx demo;
		if (demo.Construct(1280, 720, 1, 1, fullscreenToggle, true))
			demo.Start();
	}

	printf("Finished PGE window thread.\n");
	threadKeepAlive = false;

	rtl433_thread.join();
	printf("Thread finished\n");
	#ifdef WIN32
	_pclose(pipeRTL_433);
	#else
	pclose(pipeRTL);
	#endif


	return 0;
}

char* GetTimestamp()
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

	while (threadKeepAlive)
	{
		if (webWxRequested)
		{
			webWxDataReady = GetWebForecast(strLocationURL, &curlResponseBuffer);
			webWxRequested = false;
		}

		if (useRealPipe)
		{
			stringPtr = fgets(buffer, sizeof(buffer), pipeRTL_433);
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
					nlohmann::json jsonWxTelemetry = nlohmann::json::parse(wxDataMessage);
					// First make sure this telemetry is coming from our target weather station ID
					if (jsonWxTelemetry.contains("id"))
					{
						if (jsonWxTelemetry["id"].dump() == outdoorSensor.ID)
						{
							if (jsonWxTelemetry.contains("time") && jsonWxTelemetry["time"] != outdoorPacketTimestamp.previous)
							{
								outdoorPacketTimestamp.current = jsonWxTelemetry["time"];

								if (outdoorSensor.packetCounter == -1)
								{
									// Now that we know we are receiving live telemetry, do some init stuff
									printf("%s Outdoor Sensor: Now receiving telemetry.\n", GetTimestamp());
									elapsedTimeCounter = 0.0f;			// To hopefully syncronize the PGE's timing loop with the internal clock on weather station sensor
									outdoorSensor.packetCounter = 0;
								}

								if (jsonWxTelemetry.contains("sequence_num"))
									//packetSequenceNum = std::stoi(jsonParameterValue);
									packetSequenceNum = jsonWxTelemetry["sequence_num"].get<int>();
								printf("Debug: Packet Sequence Number = %u\n", packetSequenceNum);

								//if (jsonGetParameter("model"))
								if (jsonWxTelemetry.contains("model"))
								{
									//outdoorSensor.name = jsonParameterValue;
									outdoorSensor.name = jsonWxTelemetry["model"];
									printf("%s Outdoor Sensor: %s\n", GetTimestamp(), outdoorSensor.name.c_str());
								}

								if (jsonWxTelemetry.contains("channel"))
									outdoorSensor.channel = jsonWxTelemetry["channel"];

								if (jsonWxTelemetry.contains("temperature_C"))
								{
									//outdoorTempValueC.current = std::stod(jsonParameterValue);
									outdoorTempValueC.current = jsonWxTelemetry["temperature_C"];
									if ((outdoorTempValueC.current != outdoorTempValueC.previous))
									{
										outdoorTempValueF.current = ConvertedTempCtoF(outdoorTempValueC.current);
										//CalculateDewpoint(outdoorTempValueC.current, outdoorHumidityValue.current, curPressureValue_hPa);
										CalculateFeelsLikeMetrics();
										UpdateHighLowValues(outdoorTempValueF.current, &highLowOutdoorTempF);
										UpdateHighLowValues(outdoorTempValueC.current, &highLowOutdoorTempC);
										printf("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp(), outdoorTempValueF.current);
									}
								}

								if (jsonWxTelemetry.contains("temperature_F"))
								{
									//outdoorTempValueF.current = std::stod(jsonParameterValue);
									outdoorTempValueF.current = jsonWxTelemetry["temperature_F"];
									if (outdoorTempValueF.current != outdoorTempValueF.previous)
									{
										outdoorTempValueC.current = ConvertedTempFtoC(outdoorTempValueF.current);
										//CalculateDewpoint(outdoorTempValueC.current, outdoorHumidityValue.current, curPressureValue_hPa);
										CalculateFeelsLikeMetrics();
										UpdateHighLowValues(outdoorTempValueF.current, &highLowOutdoorTempF);
										UpdateHighLowValues(outdoorTempValueC.current, &highLowOutdoorTempC);
										/*
										if ((outdoorTempValueF.current <= 50.0f) && (windSpeedValueMPH.current != 0.0))
											CalculateWindChill(outdoorTempValueF.current, outdoorTempValueC.current, windSpeedValueMPH.current, windSpeedValueKPH.current);
										else if ((outdoorTempValueF.current >= 80.0f) && (outdoorHumidityValue.current != undefinedValue))
											CalculateHeatIndex(outdoorTempValueF.current, outdoorHumidityValue.current);
										*/
										printf("%s Outdoor Temperature: %.1f\xF8""F\n", GetTimestamp(), outdoorTempValueF.current);
									}
								}

								if (jsonWxTelemetry.contains("humidity"))
								{
									//outdoorHumidityValue.current = std::stoi(jsonParameterValue);
									outdoorHumidityValue.current = jsonWxTelemetry["humidity"];
									if (outdoorHumidityValue.current != outdoorHumidityValue.previous)
									{
										UpdateHighLowValues(outdoorHumidityValue.current, &highLowOutdoorHumidity);
										CalculateFeelsLikeMetrics();
										//if ((outdoorTempValueF.current != undefinedValue) && (outdoorTempValueF.current >= 80.0f))
										//	CalculateHeatIndex(outdoorTempValueF.current, outdoorHumidityValue.current);
										printf("%s Outdoor Humidity: %u%%\n", GetTimestamp(), outdoorHumidityValue.current);
									}
								}

								if (jsonWxTelemetry.contains("wind_avg_mi_h"))
								{
									//windSpeedValueMPH.current = std::stod(jsonParameterValue);
									windSpeedValueMPH.current = jsonWxTelemetry["wind_avg_mi_h"];
									windSpeedValueKPH.current = windSpeedValueMPH.current * 1.6093483909479;
									CalculateFeelsLikeMetrics();
									windSpeedAvgValueMPH = GetUpdatedAverageDeque(&dequeWindSpeedSamples, windSpeedValueMPH.current, 12);
									windSpeedHighValueMPH = GetUpdatedHighDeque(&dequeWindSpeedSamples, windSpeedValueMPH.current, 360);
									printf("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %u)\n", GetTimestamp(), windSpeedValueMPH.current, windSpeedAvgValueMPH, dequeWindSpeedSamples.size());
								}

								if (jsonWxTelemetry.contains("wind_avg_km_h"))
								{
									//windSpeedValueKPH.current = std::stod(jsonParameterValue);
									windSpeedValueKPH.current = jsonWxTelemetry["wind_avg_km_h"];
									windSpeedValueMPH.current = windSpeedValueKPH.current / 1.6093483909479;
									CalculateFeelsLikeMetrics();
									//windSpeedAvgValueKPH = GetUpdatedAverageDeque(&dequeWindSpeedSamples, windSpeedValueKPH.current, 12);
									//windSpeedHighValueKPH = GetUpdatedHighDeque(&dequeWindSpeedSamples, windSpeedValueKPH.current, 360);
									windSpeedAvgValueMPH = GetUpdatedAverageDeque(&dequeWindSpeedSamples, windSpeedValueMPH.current, 12);
									windSpeedHighValueMPH = GetUpdatedHighDeque(&dequeWindSpeedSamples, windSpeedValueMPH.current, 360);
									printf("%s Outdoor Wind Speed: %.0f mph (Average: %.0f mph, Samples = %u)\n", GetTimestamp(), windSpeedValueMPH.current, windSpeedAvgValueMPH, dequeWindSpeedSamples.size());
								}

								if (jsonWxTelemetry.contains("wind_dir_deg"))
								{
									if (dequeWindDirections.size() >= 3)
										dequeWindDirections.pop_back();
									//dequeWindDirections.push_front(std::stod(jsonParameterValue));
									dequeWindDirections.push_front(jsonWxTelemetry["wind_dir_deg"]);
									if (dequeWindDirections.size() > 1)
									{
										//windAnimationIncrement = true;
										windDirAnimatedPosition = dequeWindDirections.at(1);
									}

									printf("%s \x1b[1;96mOutdoor Wind Direction: %.0f\xF8\n\x1b[0m", GetTimestamp(), dequeWindDirections.front());
								}

								if (jsonWxTelemetry.contains("rain_in"))
								{
									if (!debugState)
										//rainfallDataValueInches.current = std::stod(jsonParameterValue);
										rainfallDataValueInches.current = jsonWxTelemetry["rain_in"];
									if (rainfallDataValueInches.previous != undefinedValue)
									{
										double rainfallDeltaValue = 0.0;
										if (rainfallDataValueInches.current > rainfallDataValueInches.previous)
											rainfallDeltaValue = (rainfallDataValueInches.current - rainfallDataValueInches.previous);
										else if (rainfallDataValueInches.current < rainfallDataValueInches.previous)
											rainfallDeltaValue = (5.12 - rainfallDataValueInches.previous) + rainfallDataValueInches.current;
										rainfallTotalTodayValue += rainfallDeltaValue;
										if (rainfallTotalTodayValue > (rainGaugeCapacityIn * 0.90f))
											rainGaugeCapacityIn += 1.0f;
										if (dequeRainRateSamples.size() >= 20)
											dequeRainRateSamples.pop_front();
										dequeRainRateSamples.push_back(rainfallDeltaValue);
										printf("New Delta = %f\n", rainfallDeltaValue);
										// Below code calculates the average rainfall stored in our deque and then scales the average up to fit in a 60 minute time period
										rainfallRateInchesPerHour = std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0) * (120.0 / dequeRainRateSamples.size());
										printf("%s \x1b[1;34mOutdoor Rainfall Rate: %.2f in/hr (Sum = %.2f, Sample Count = %u)\n\x1b[0m", GetTimestamp(), rainfallRateInchesPerHour, std::accumulate(dequeRainRateSamples.begin(), dequeRainRateSamples.end(), 0.0), dequeRainRateSamples.size());
										printf("\x1b[1;34mRainfall Rate Samples = ");
										for (int i = 0; i < dequeRainRateSamples.size(); i++)
											printf("%.2f ", dequeRainRateSamples.at(i));
										printf("\n\x1b[0m");

									}
									rainfallDataValueInches.previous = rainfallDataValueInches.current;
									printf("%s \x1b[1;34mOutdoor Rainfall: %.2f inches\n\x1b[0m", GetTimestamp(), rainfallTotalTodayValue);
								}

								if (jsonWxTelemetry.contains("strike_count"))
								{
									//lightningStrikeCount.current = std::stoi(jsonParameterValue);
									lightningStrikeCount.current = jsonWxTelemetry["strike_count"];
									if (lightningStrikeCount.previous != lightningStrikeCount.current)
									{
										printf("%s Outdoor Lightning Strike Count: %u\n", GetTimestamp(), lightningStrikeCount.current);
										lightningStrikeCount.previous = lightningStrikeCount.current;
									}
								}

								if (jsonWxTelemetry.contains("strike_distance"))
								{
									//lightningStrikeDistance.current = std::stoi(jsonParameterValue);
									lightningStrikeDistance.current = jsonWxTelemetry["strike_distance"];
									if (lightningStrikeDistance.previous != lightningStrikeDistance.current)
									{
										printf("%s Outdoor Lightning Strike Distance: %u\n", GetTimestamp(), lightningStrikeDistance.current);
										lightningStrikeDistance.previous = lightningStrikeDistance.current;
									}
								}

								if (jsonWxTelemetry.contains("uv"))
								{
									//uvIndex.current = std::stoi(jsonParameterValue);
									uvIndex.current = jsonWxTelemetry["uv"];
									UpdateHighLowValues(uvIndex.current, &uvIndex);
									printf("%s Outdoor UV Index: %u\n", GetTimestamp(), uvIndex.current);
								}

								if (jsonWxTelemetry.contains("lux"))
								{
									//lightLevelLux.current = std::stoi(jsonParameterValue);
									lightLevelLux.current = jsonWxTelemetry["lux"];
									UpdateHighLowValues(lightLevelLux.current, &lightLevelLux);
									printf("%s Outdoor Light Level (Lux): %u\n", GetTimestamp(), lightLevelLux.current);
								}

								if (jsonWxTelemetry.contains("battery_ok"))
								{
									//outdoorSensor.batteryStatus = (jsonParameterValue == "1") ? 1 : 0;
									outdoorSensor.batteryStatus = jsonWxTelemetry["battery_ok"];
									printf("%s Outdoor Sensor Battery: %s\n", GetTimestamp(), outdoorSensor.batteryStatus ? "Normal" : "Low");
								}

								outdoorPacketTimestamp.previous = outdoorPacketTimestamp.current;
								outdoorSensor.recentlyUpdated = true;
								if (outdoorSensor.packetCounter < 1)
									outdoorSensor.packetCounter = 1;
							}
						}
						else if (jsonWxTelemetry["id"].dump() == indoorSensor.ID)
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
							if (jsonWxTelemetry.contains("time") && (jsonWxTelemetry["time"] != indoorPacketTimestamp.previous))
							{
								indoorPacketTimestamp.current = jsonWxTelemetry["time"];
								if (indoorSensor.packetCounter == -1)
								{
									// Now that we know we are receiving live telemetry, do some init stuff
									printf("%s Indoor Sensor: Now receiving telemetry.\n", GetTimestamp());
									indoorSensor.packetCounter = 0;
								}

								if (jsonWxTelemetry.contains("model"))
								{
									//indoorSensor.name = jsonParameterValue;
									indoorSensor.name = jsonWxTelemetry["model"];
									printf("%s Indoor Sensor: %s\n", GetTimestamp(), indoorSensor.name.c_str());
								}

								if (jsonWxTelemetry.contains("channel"))
									//indoorSensor.channel = jsonParameterValue;
									indoorSensor.channel = jsonWxTelemetry["channel"];

								if (jsonWxTelemetry.contains("temperature_C"))
								{
									//indoorTempValueC.current = std::stod(jsonParameterValue);
									indoorTempValueC.current = jsonWxTelemetry["temperature_C"];
									if (!useMetricUnits)
										indoorTempValueF.current = ConvertedTempCtoF(indoorTempValueC.current);
									UpdateHighLowValues(indoorTempValueC.current, &highLowIndoorTempC);
									UpdateHighLowValues(indoorTempValueF.current, &highLowIndoorTempF);
									printf("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp(), indoorTempValueF.current);
								}

								if (jsonWxTelemetry.contains("temperature_F"))
								{
									//indoorTempValueF.current = std::stod(jsonParameterValue);
									indoorTempValueF.current = jsonWxTelemetry["temperature_F"];
									if (useMetricUnits)
										indoorTempValueC.current = ConvertedTempFtoC(indoorTempValueF.current);
									UpdateHighLowValues(indoorTempValueF.current, &highLowIndoorTempF);
									UpdateHighLowValues(indoorTempValueC.current, &highLowIndoorTempC);
									printf("%s Indoor Temperature: %.1f\xF8""F\n", GetTimestamp(), indoorTempValueF);
								}

								if (jsonWxTelemetry.contains("humidity"))
								{
									//indoorHumidityValue.current = std::stoi(jsonParameterValue);
									indoorHumidityValue.current = jsonWxTelemetry["humidity"];
									UpdateHighLowValues(indoorHumidityValue.current, &highLowIndoorHumidity);
									printf("%s Indoor Humidity: %u%%\n", GetTimestamp(), indoorHumidityValue.current);
								}

								if (jsonWxTelemetry.contains("battery_ok"))
								{
									//indoorSensor.batteryStatus = (jsonParameterValue == "1") ? 1 : 0;
									indoorSensor.batteryStatus = jsonWxTelemetry["battery_ok"];
									printf("%s Indoor Sensor Battery: %s\n", GetTimestamp(), indoorSensor.batteryStatus ? "Normal" : "Low");
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

float GetUpdatedAverageDeque(std::deque<float>* dequeTarget, float newSample, int maxSampleSize)
{
	if (dequeTarget->size() >= maxSampleSize)
		dequeTarget->pop_front();
	dequeTarget->push_back(newSample);

	return (std::accumulate(dequeTarget->begin(), dequeTarget->end(), 0.0f) / dequeTarget->size());
}

float GetUpdatedHighDeque(std::deque<float>* dequeTarget, float newSample, int maxSampleSize)
{
	float highestValue = 0.0f;
	if (dequeTarget->size() >= maxSampleSize)
		dequeTarget->pop_front();
	dequeTarget->push_back(newSample);

	for (int i = 0; i < dequeTarget->size(); i++)
		if (dequeTarget->at(i) > highestValue)
			highestValue = dequeTarget->at(i);

	return highestValue;
}

void CalculateFeelsLikeMetrics()
{
	double tempC = outdoorTempValueC.current;
	double tempF = outdoorTempValueF.current;
	int relativeHumidity = outdoorHumidityValue.current;
	double windSpeedKPH = windSpeedValueKPH.current;
	double windSpeedMPH = windSpeedValueMPH.current;
	float windWithCoeffecient, windSpeedMetersPerSecond;

	// First, calculate the dewpoint (parameters needed are Temperature in Celsius, Relative Humdity, and Pressure)
	if ((tempC != undefinedValue) && (relativeHumidity != undefinedValue))
	{
		saturationVaporPressure = referenceVaporPressure * std::exp((magnusCoefficient * tempC) / (tempC + magnusTempOffset));
		// Next calculate the "Actual Vapor Pressure"
		actualVaporPressure = relativeHumidity * (saturationVaporPressure / 100.0f) * (curPressureValue_hPa / standardPressure_hPa);
		// Next calculate our actual dewpoint in celsius
		dewpointValueC = (magnusTempOffset * std::log(actualVaporPressure / referenceVaporPressure)) / (magnusCoefficient - std::log(actualVaporPressure / referenceVaporPressure));
		dewpointValueF = ConvertedTempCtoF(dewpointValueC);
	}

	// Next calculate the Heat Index (parameters needed are Temperature in Fahrenheit and Relative Humidity)
	if ((tempF != undefinedValue) && (relativeHumidity != undefinedValue))
	{
		calculatedHeatIndexF = heatIndexConst1 + (heatIndexConst2 * tempF) + (heatIndexConst3 * relativeHumidity) + (heatIndexConst4 * tempF * relativeHumidity) +
			(heatIndexConst5 * tempF * tempF) + (heatIndexConst6 * relativeHumidity * relativeHumidity) + (heatIndexConst7 * tempF * tempF * relativeHumidity) +
			(heatIndexConst8 * tempF * relativeHumidity * relativeHumidity) + (heatIndexConst9 * tempF * tempF * relativeHumidity * relativeHumidity);
		calculatedHeatIndexC = ConvertedTempFtoC(calculatedHeatIndexF);
	}

	// Next calculate the Wind Chill (parameters needed are Temperature in both C and F, and Wind Speed in both MPH and KPH)
	if ((tempF != undefinedValue) && (windSpeedMPH != undefinedValue))
	{
		windWithCoeffecient = std::pow(windSpeedMPH, windChillCoEffecient);
		calculatedWindChillF = (windChillBaselineF + (windChillTempContrib * tempF) - (windChillSpeedFactorF1 * windWithCoeffecient) + (windChillSpeedFactorF2 * tempF * windWithCoeffecient));
	}
	if ((tempC != undefinedValue) && (windSpeedKPH != undefinedValue))
	{
		windWithCoeffecient = std::pow(windSpeedKPH, windChillCoEffecient);
		calculatedWindChillC = (windChillBaselineC + (windChillTempContrib * tempC) - (windChillSpeedFactorC1 * windWithCoeffecient) + (windChillSpeedFactorC2 * tempC * windWithCoeffecient));
	}

	// Finally, calculate the general "Apparent" temperature used for when neither Heat Index nor Wind Chill apply
	// Note: This formula requires a valid actualVaporPressure value calculated in the Dewpoint code above
	if ((tempC != undefinedValue) && (windSpeedKPH != undefinedValue) && (actualVaporPressure != undefinedValue))
	{
		windSpeedMetersPerSecond = (windSpeedKPH * 1000.0f) / 3600.0f;
		calculatedApparentTempC = tempC + (actualVaporPressure * 0.33f) - (windSpeedMetersPerSecond * 0.7f) - 4.0f;
		calculatedApparentTempF = ConvertedTempCtoF(calculatedApparentTempC);
	}
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
	calculatedHeatIndexC = ConvertedTempFtoC(calculatedHeatIndexF);
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

void CalculateApparentTemperature(double tempC, double windSpeedKPH)
{
	if (actualVaporPressure == undefinedValue)
		return;

	float windSpeedMetersPerSecond = (windSpeedKPH * 1000.0f) / 3600.0f;
	calculatedApparentTempC = tempC + (actualVaporPressure * 0.33f) - (windSpeedMetersPerSecond * 0.7f) - 4.0f;
	calculatedApparentTempF = ConvertedTempCtoF(calculatedApparentTempC);
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

void UpdateHighLowValues(int sourceValue, intHighLowRange* destHighLowRange)
{
	if ((destHighLowRange->high == -1) || (sourceValue > destHighLowRange->high))
		destHighLowRange->high = sourceValue;
	if ((destHighLowRange->low == -1) || (sourceValue < destHighLowRange->low))
		destHighLowRange->low = sourceValue;
}

/*
size_t jsonGetParameter(std::string keyword)
{ return jsonGetParameter(keyword, wxDataMessage, 0); }

size_t jsonGetParameter(std::string keyword, std::string jsonStringBuffer, size_t startPos)
{
	//if (keyword == "id")
	//	printf("breakpoint\n");
	size_t keywordStart, fieldDivider, dataStart, dataEnd;
	// Add quote-marks around the keyword
	keyword.insert(keyword.begin(), '\"');
	keyword.push_back('\"');
	keywordStart = jsonStringBuffer.find(keyword, startPos);
	if (keywordStart == std::string::npos)
		return 0;
	
	fieldDivider = jsonStringBuffer.find(':', keywordStart);
	if (fieldDivider == std::string::npos)
		return 0;

	dataStart = jsonStringBuffer.find_first_not_of(' ', fieldDivider + 1);
	if (dataStart == std::string::npos)
		return 0;

	dataEnd = jsonStringBuffer.find_first_of(",\n\r", dataStart);
	if (dataEnd == std::string::npos)
		return 0;

	jsonParameterValue = jsonStringBuffer.substr(dataStart, (dataEnd - dataStart));

	// If value is in quotes, strip them off
	if ((jsonParameterValue.front() == '\"') && (jsonParameterValue.back() == '\"'))
		std::erase(jsonParameterValue, '\"');

	return dataStart;
}
*/

bool LoadConfigFile()
{
	std::string inputLine, strParamName, strParamValue;
	std::ifstream configFile("DragonWx.conf");
	size_t equalsSymbolIndex, paramValueIndex, paramNameEndIndex;
	if (!configFile.is_open())
	{
		std::cout << "Error: Could not open config file." << std::endl;
		return false;
	}

	while (std::getline(configFile, inputLine))
	{
		equalsSymbolIndex = inputLine.find('=');
		if (equalsSymbolIndex != std::string::npos)
		{
			paramValueIndex = inputLine.find_first_not_of(' ', equalsSymbolIndex + 1);
			if (paramValueIndex != std::string::npos)
			{
				strParamName = inputLine.substr(0, equalsSymbolIndex);
				strParamValue = inputLine.substr(paramValueIndex, inputLine.size() - paramValueIndex);

				// If value is in quotes, strip them off
				if ((strParamValue.front() == '\"') && (strParamValue.back() == '\"'))
					std::erase(strParamValue, '\"');

				if (strParamName == "RTL433_PATH")
				{
					pathToExec = strParamValue;
					printf("Debug: Config param %s loaded as %s\n", strParamName.c_str(), pathToExec.c_str());
				}
				else if (strParamName == "RTL433_PARAMS")
				{
					sdrExtraArguments = strParamValue;
					printf("Debug: Config param %s loaded as %s\n", strParamName.c_str(), sdrExtraArguments.c_str());
				}
				else if (strParamName == "SDR_GAIN")
				{
					sdrGainSetting = strParamValue;
					printf("Debug: Config param %s loaded as %s\n", strParamName.c_str(), sdrGainSetting.c_str());
				}
				else if (strParamName == "SDR_ANTENNA")
				{
					sdrAntennaSetting = strParamValue;
					printf("Debug: Config param %s loaded as %s\n", strParamName.c_str(), sdrAntennaSetting.c_str());
				}
				else if (strParamName == "FULLSCREEN")
				{
					fullscreenToggle = std::stoi(strParamValue);
					printf("Debug: Config param %s loaded as %u\n", strParamName.c_str(), fullscreenToggle);
				}
				else if (strParamName == "UNITS")
				{
					useMetricUnits = std::stoi(strParamValue);
					printf("Debug: Config param %s loaded as %u\n", strParamName.c_str(), useMetricUnits);
				}
				else if (strParamName == "STATION_NAME")
				{
					strWxStationName = strParamValue;
					printf("Debug: Config param %s loaded as %s\n", strParamName.c_str(), strWxStationName.c_str());
				}
			}

		}
	}
}

void populateTestData()
{
	outdoorSensor.name = "Acurite Atlas";
	outdoorTempValueF.current = 76.2f;
	highLowOutdoorTempF.low = 53.2f;
	highLowOutdoorTempF.high = 86.7f;
	outdoorHumidityValue.current = 76;
	dequeWindDirections.push_back(220.0f);
	dequeWindDirections.push_back(170.0f);
	dequeWindDirections.push_back(200.0f);
	windSpeedValueMPH.current = 7.4;
	rainfallTotalTodayValue = 0.14;
	uvIndex.current = 5;
	lightLevelLux.current = 9000;

	indoorTempValueF.current = 69.3f;
	indoorHumidityValue.current = 49;

	outdoorSensor.batteryStatus = batteryStatusNormal; 
	outdoorSensor.packetCounter = 4;
	outdoorSensor.channel = "A";

	indoorSensor.batteryStatus = batteryStatusNormal;
	indoorSensor.packetCounter = 4;
	indoorSensor.channel = "B";
}

bool ConvertTimeToLocal(std::tm* convertedTimePtr, std::time_t timeToConvert)
{
#ifdef WIN32
	if (localtime_s(convertedTimePtr, &timeToConvert) != 0)
		return false;
#else
	if (localtime_r(&systemTimeNow, &timeToConvert) == nullptr)
		return false;
#endif
	return true;
}

bool GetWebForecast(std::string url, std::string* curlOutputBufferPtr)
{
	std::string pathToIcon;
	//std::time_t epochTime;

	if (useRealWebRequests)
	{
		CURL* curl = curl_easy_init();
		if (!curl) {
			std::cerr << "Failed to initialize libcurl\n";
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
		if (res != CURLE_OK) {
			long http_code = 0;
			std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			std::cerr << "HTTP Error Code: " << http_code << std::endl;
		}
		curl_easy_cleanup(curl);  // Clean up
	}
	else
		ReadFileJSON("meteo-epoch.out", curlOutputBufferPtr);

	std::cout << *curlOutputBufferPtr << std::endl;

	nlohmann::json jsonWxWebReply = nlohmann::json::parse(*curlOutputBufferPtr);

	if (!jsonWxWebReply["current"].contains("weather_code"))
		return false;
	if (!jsonWxWebReply["current"].contains("is_day"))
		return false;
	if (!jsonWxWebReply["daily"].contains("weather_code"))
		return false;
	if (!jsonWxWebReply["daily"].contains("time"))
		return false;

	webWxCurrentConditions.code = jsonWxWebReply["current"]["weather_code"];
	webWxCurrentConditions.useDaytime = jsonWxWebReply["current"]["is_day"].get<bool>();

	for (int i = 0; i < jsonWxWebReply["daily"]["time"].size(); i++)
	{
		//epochTime = jsonWxWebReply["daily"]["time"][i];
		if (!ConvertTimeToLocal(&webWxDailyForecasts[i].dateTime, jsonWxWebReply["daily"]["time"][i]))
			return false;
		webWxDailyForecasts[i].code = jsonWxWebReply["daily"]["weather_code"][i];
		webWxDailyForecasts[i].useDaytime = false;
		webWxDailyForecasts[i].tempMin = jsonWxWebReply["daily"]["temperature_2m_min"][i];
		webWxDailyForecasts[i].tempMax = jsonWxWebReply["daily"]["temperature_2m_max"][i];
		webWxDailyForecasts[i].precipPercent = jsonWxWebReply["daily"]["precipitation_probability_max"][i];
	}

	return true;
}

size_t WriteOutCurlResponse(char* readBufferPtr, size_t dataElementSize, size_t dataElementsReceived, std::string* outputStringBuffer)
{
	size_t totalBytesToHandle = dataElementSize * dataElementsReceived;
	outputStringBuffer->append(readBufferPtr, totalBytesToHandle);
	return totalBytesToHandle;
}

static void ReadFileJSON(std::string filePath, std::string* inputBuffer)
{
	std::string fileInputLine;
	std::ifstream jsonFile(filePath);
	if (!jsonFile)
	{
		std::cout << "Error opening the JSON file." << std::endl;
		return;
	}

	inputBuffer->clear();
	while (std::getline(jsonFile, fileInputLine))
		inputBuffer->append(fileInputLine + "\n");

	jsonFile.close();
	std::cout << "Contents of read JSON file:" << std::endl << std::endl << *inputBuffer << std::endl;
}
