#pragma once

#include <variant>

constexpr float undefinedFloatValue = 300.0f;
constexpr int undefinedIntValue = 300;

struct temperatureUnitsStruct
{
	std::u32string imperial = U"\u00B0F";
	std::u32string metric = U"\u00B0C";
	std::u32string Label(bool ifUseMetric) const { return (ifUseMetric ? metric : imperial); }
};

struct windSpeedUnitsStruct
{
	std::string unitsMPH = "mph";
	std::string unitsKPH = "kph";
	std::string Label(bool ifUseMetric) const { return (ifUseMetric ? unitsKPH : unitsMPH); }
};

struct rainfallUnitsStruct
{
	struct
	{
		std::u32string imperial = U"in";
		std::u32string metric = U"mm";
		std::u32string Label(bool ifUseMetric) { return (ifUseMetric ? metric : imperial); }
	} amount;
	struct
	{
		std::u32string imperial = U"in/hr";
		std::u32string metric = U"mm/hr";
		std::u32string Label(bool ifUseMetric) { return (ifUseMetric ? metric : imperial); }
	} rate;
};

struct textObject
{
	std::u32string string32;
	olc::Renderable renderable;
	olc::vi2d posOffset = { 0, 0 };
	olc::vi2d posStart = { 0, 0 };
	int width = 0;
	int height = 0;
};

struct titleBox
{
	std::u32string text32;
	olc::Font* fontPtr;
	olc::Pixel color;
	olc::vi2d posStart;
	olc::vi2d size;
	int radius;			// Determines how rounded the corners are
	int titlePadding;
	olc::vi2d posTitle;
	textObject textObj;
};

struct inputBoxStruct
{
	bool isEnabled;
	struct
	{
		std::u32string text32;
		olc::vi2d pos;
		textObject textObj;
	} label;
	struct
	{
		olc::vi2d pos;
		olc::vi2d size;
		int type;
		std::string text;
		textObject textObj;
	} value;
};

struct buttonStruct
{
	bool isEnabled;
	olc::Font* fontPtr;
	olc::Pixel textColor;
	olc::vi2d pos;
	olc::vi2d size;
	std::u32string text32;
	textObject textObj;
};

struct dialogBox
{
	bool showInForeground;
	olc::vi2d size;
	std::string textArray[4];
	bool textCenteredArray[4] = { true, false, false, false };
	int textLineOffsetArray[4];
	textObject textObjArray[4];
};

struct rainGaugeTickMark
{
	olc::vi2d pos;
	textObject textObj;
};

struct tempOffsetValuePair
{
	float imperial = 0.0f;
	float metric = 0.0f;
	float GetValue(bool ifMetric) const { return ifMetric ? metric : imperial; }
	void SetValue(float newValue, bool isMetric)
	{
		if (isMetric)
		{
			metric = newValue;
			imperial = (metric * 9.0f) / 5.0f;
		}
		else
		{
			imperial = newValue;
			metric = (imperial * 5.0f) / 9.0f;
		}
	}
	void SetZero()
	{
		metric = 0.0f;
		imperial = 0.0f;
	}
};

struct rainGaugeStruct
{
	float inches = 1.0f;
	float millimeters = 25.0f;
	float GetValue(bool isMetric) { return (isMetric ? millimeters : inches); }
	void GrowCapacity()
	{
		inches += 1.0f;
		millimeters += 25.0f;
	}
	void ResetCapacity()
	{
		inches = 1.0f;
		millimeters = 25.0f;
	}
};

struct rainfallAmountValuePair
{
	float inches = 0.0f;
	float millimeters = 0.0f;
	float GetValue(bool ifMetric) const { return ifMetric ? millimeters : inches; }
	void SetValue(float newValue, bool isMetric)
	{
		if (isMetric)
		{
			millimeters = newValue;
			inches = millimeters / 25.4f;
		}
		else
		{
			inches = newValue;
			millimeters = inches * 25.4f;
		}
	}
	void AddValue(float newValue, bool isMetric)
	{
		if (isMetric)
		{
			millimeters += newValue;
			inches = millimeters / 25.4f;
		}
		else
		{
			inches += newValue;
			millimeters = inches * 25.4f;
		}
	}
	void SetZero()
	{
		millimeters = 0.0f;
		inches = 0.0f;
	}
};

struct rainfallRateStruct
{
	std::time_t timePrevious = 0, timeCurrent = 0, timeDelta = 0;
	rainfallAmountValuePair rainfallRate;

	void Update(std::time_t newTime, float newRainfallDelta, bool ifSourceMetric)
	{
		timePrevious = timeCurrent;
		timeCurrent = newTime;
		if (timePrevious != 0)
		{
			timeDelta = timeCurrent - timePrevious;
			rainfallRate.SetValue((3600.0f / timeDelta) * newRainfallDelta, ifSourceMetric);
		}
	}
};

struct configEntry
{
	std::string keyword;
	std::string padding;
	std::variant<bool*, std::string*, int*, tempOffsetValuePair*> varPtr;
};

struct floatPrevCur
{
	float previous;
	float current;
};

struct stringPrevCur
{
	std::string previous;
	std::string current;
};

struct intRangeStruct
{
	int high = undefinedIntValue;
	int low = undefinedIntValue;
	int current = undefinedIntValue;
	int previous = undefinedIntValue;
	int offset = 0;

	bool Update(int newValue)								// Returns true if the current value changed, false if it's the same as before
	{
		newValue += offset;									// Adjust our raw input number from the sensor by the calibration offset

		if ((high == undefinedIntValue) || (newValue > high))
			high = newValue;
		if ((low == undefinedIntValue) || (newValue < low))
			low = newValue;

		if (newValue != current)
		{
			previous = current;
			current = newValue;
			return true;
		}
		return false;
	}
	void Reset()
	{
		high = current;
		low = current;
	}
};

struct tempUnitsPairStruct
{
	float imperial = 300.0f, metric = 300.0f;
	float GetValue(bool ifSourceMetric) const { return ifSourceMetric ? metric : imperial; }
	void SetValue(float newValue, bool sourceMetric)
	{
		if (sourceMetric)
		{
			metric = newValue;
			imperial = (metric * 1.8) + 32;
		}
		else
		{
			imperial = newValue;
			metric = (imperial - 32) / 1.8;
		}
	}
	void SetZero()
	{
		metric = 0.0f;
		imperial = 0.0f;
	}
	bool IsDefined() const { return ((imperial != undefinedFloatValue) || (metric != undefinedFloatValue)); }
};

struct temperatureStruct
{
	tempUnitsPairStruct high, low, current, previous;
	tempOffsetValuePair offset;

	void Update(float newValue, bool ifSourceMetric)		// Returns true if the current value changed, false if it's the same as before
	{
		newValue += offset.GetValue(ifSourceMetric);		// Adjust our raw input number from the sensor by the calibration offset

		if ((high.GetValue(ifSourceMetric) == undefinedFloatValue) || (newValue > high.GetValue(ifSourceMetric)))
			high.SetValue(newValue, ifSourceMetric);
		if ((low.GetValue(ifSourceMetric) == undefinedFloatValue) || (newValue < low.GetValue(ifSourceMetric)))
			low.SetValue(newValue, ifSourceMetric);

		previous = current;
		current.SetValue(newValue, ifSourceMetric);
	}
	void Reset()
	{
		high = current;
		low = current;
	}
};

struct windSpeedUnitsPair
{
	float mph = undefinedFloatValue, kph = undefinedFloatValue;
	float GetValue(bool ifUnitsMetric) { return (ifUnitsMetric ? kph : mph); }
	void SetValue(float newValue, bool ifUnitsMetric)
	{
		if (ifUnitsMetric)
		{
			kph = newValue;
			mph = newValue / 1.6093483909479f;
		}
		else
		{
			mph = newValue;
			kph = newValue * 1.6093483909479f;
		}
	}
	void Clear()
	{
		mph = undefinedFloatValue;
		kph = undefinedFloatValue;
	}
	bool IsDefined() const { return ((mph != undefinedFloatValue) || (kph != undefinedFloatValue));  }
};

struct windSpeedStruct
{
	windSpeedUnitsPair average, peak, current, previous;

	void Update(std::deque<float>& dequeWindSpeed, float newValue, bool ifSourceMetric)
	{
		if (dequeWindSpeed.size() >= 360)
			dequeWindSpeed.pop_back();
		dequeWindSpeed.push_front(newValue);

		float samplesSum = 0.0f;
		int sampleSize = (dequeWindSpeed.size() < 12) ? dequeWindSpeed.size() : 12;

		for (int i = 0; i < sampleSize; i++)
			samplesSum += dequeWindSpeed[i];

		average.SetValue(samplesSum / sampleSize, ifSourceMetric);
		peak.SetValue(*std::max_element(dequeWindSpeed.begin(), dequeWindSpeed.end()), ifSourceMetric);

		previous = current;
		current.SetValue(newValue, ifSourceMetric);
	}
};

struct sensorStatus
{
	bool telemetryStarted;
	bool recentlyUpdated;
	int packetCounter;
	int batteryStatus;
	std::string ID;
	std::string name;
	std::string channel;
	temperatureStruct temperature;
	intRangeStruct humidity;
};

struct wxWebEntry
{
	std::tm dateTime;
	std::time_t sunrise = 0;
	std::time_t sunset = 0;
	int code = -1;
	std::string description;
	bool useDaytime = true;
	float tempMin = 300;
	float tempMax = 300;
	int precipPercent = -1;
};

struct wxCodeStruct
{
	//std::string wwoCode;
	std::string descriptionDay;
	std::string descriptionNight;
	std::string iconFileDay;
	std::string iconFileNight;
};

#ifdef _WIN32
struct ProcessHandle
{
	HANDLE hProcess = NULL;
	HANDLE hStdOutRead = NULL;
};
#endif