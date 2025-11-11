#pragma once

#include <string>
#include <deque>
#include <numbers>
#include <cstdio>
#include <iostream>
#include <thread>
#include <chrono>

#include "Structs.h"
#include "json.hpp"

#define USE_WINDOWS_PIPE

#if defined(_DEBUG) || defined(_CONSOLE)
#define PRINT_DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINT_DEBUG(fmt, ...)
#endif

void StripCharacters(std::string&, const char*);
olc::vi2d GetSystemResolution();
bool CheckFileDependencies();
void WorkerThreadHandler();
void StartPipeRTL433();
void CallbackHandlerRTL433(const char* bytes, size_t n);
bool StopWorkerThread();
bool ConvertTimeToLocal(std::tm*, std::time_t);
std::string GetFormattedLocalTime(std::string, std::time_t*);
std::string GetTimestamp();
bool GetWebForecast(std::string, std::string*);
size_t WriteOutCurlResponse(char*, size_t, size_t, std::string*);
bool LoadConfigFile();
bool SaveConfigFile();
void WriteMsgToErrorLog(std::string outputString);

#ifdef _DEBUG
// Debug/testing related functions
void populateDemoData();
static void ReadFileJSON(std::string, std::string*);
static void TestJSON(std::string);
#endif

// Conversion/translation related functions
double ConvertedTempCtoF(float tempC);
double ConvertedTempFtoC(float tempF);
double degreesToRadians(double);

// Calculation functions
void CalculateFeelsLikeMetrics();
float CalculateTrendSlope(std::deque<float>* dequeSource);
