#include "SensorsMain.h"

SensorsMain::SensorsMain(float loadCellCalibrationFactor, float loadCellOffset, float emptyTankWeight, float furnaceOnThreshold, float furnaceOffThreshold)
: loadCellCalibrationFactor(loadCellCalibrationFactor), loadCellOffset(loadCellOffset), emptyTankWeight(emptyTankWeight),
  furnaceOnThreshold(furnaceOnThreshold), furnaceOffThreshold(furnaceOffThreshold), currentValues(2, std::vector<float>(windowSize, 0.0f))
{
	sht3x = Adafruit_SHT31();
	furnaceState = false;
	relayState = false;
	furnaceStateHasChanged = true;
	weightAfterFurnaceProcess = weightBeforeFurnaceProcess = 0;
}


void SensorsMain::SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin, float currentOffset)
{
	currentSensor[sensorNumber] = ACS712(sensorType, sensorPin);
	currentSensor[sensorNumber].SetCurrentOffset(currentOffset);
}


void SensorsMain::SetupDallasTempSensor(const int sensorPin)
{
	oneWire = OneWire(sensorPin);
	dallasSensor = DallasTemperature(&oneWire);
	dallasSensor.begin();
}


void SensorsMain::SetupSHT3xTempSensor(const int sensorAddress)
{
	sht3xAddress = sensorAddress;
	sht3x.begin(sht3xAddress);
}


void SensorsMain::SetupHX711WeightSensor(const int doutPin, const int sckPin)
{
	loadCell.begin(doutPin, sckPin);
	loadCell.set_scale(loadCellCalibrationFactor);
	loadCell.set_offset(loadCellOffset);
}

float SensorsMain::CalculateConsumption()
{
	if (weightBeforeFurnaceProcess == 0)
	{
		if (furnaceState)
			weightBeforeFurnaceProcess = GetWeightGrams(10);
	}
	else
	// furnace is working, get measurement when it has finished
	if (!furnaceState)
	{
		weightAfterFurnaceProcess = GetWeightGrams(10);
		float consumption = weightBeforeFurnaceProcess - weightAfterFurnaceProcess;
		weightAfterFurnaceProcess = weightBeforeFurnaceProcess = 0;
		return consumption;
	}

	return 0;
}


void SensorsMain::SetupRelay(const int relayPin)
{
	this->relayPin = relayPin;
	pinMode(relayPin, OUTPUT);
	SetRelayOutput();
	UpdateRelayState();
}


void SensorsMain::ToggleRelayState()
{
	relayState = !relayState;
	SetRelayOutput();
	UpdateRelayState(); // Maybe the change didn't go through, check it
}


void SensorsMain::UpdateRelayState()
{
	if (digitalRead(relayPin) == HIGH)
		relayState = true;
	else
		relayState = false;
}


void SensorsMain::SetRelayOutput()
{
	if(relayState)
		digitalWrite(relayPin, HIGH);
	else
		digitalWrite(relayPin, LOW);
}

void SensorsMain::SetFurnaceState()
{
	filteredValuesDelta = filteredValues[0] - filteredValues[1];
	if (filteredValuesDelta > furnaceOnThreshold)
		{
			if (!furnaceState) furnaceStateHasChanged = true;
			furnaceState = true;
		}
	else
		if (filteredValuesDelta < furnaceOffThreshold)
		{
			if (furnaceState) furnaceStateHasChanged = true;
			furnaceState = false;
		}
}

bool SensorsMain::FurnaceStateHasChanged()
{
	if (furnaceStateHasChanged)
	{
		furnaceStateHasChanged = false;
		return true;
	}
	return false;
}

std::pair<float, float> SensorsMain::GetCurrent(const int sensorNumber, const int numMeasurements)
{
	float I = 0;

	// average over numMeasurements, remove 2 potential outliers
	std::vector<float> measurements;
	for (int i = 0; i < numMeasurements; ++i)
		measurements.push_back(currentSensor[sensorNumber].getCurrentAC());
	std::sort(measurements.begin(), measurements.end());

	if (measurements.size() >= 3)
		I = std::accumulate(measurements.begin() + 1, measurements.end() - 1, 0.0f) / (numMeasurements - 2);
	else
		I = measurements[0];

	currentValues[sensorNumber].push_back(I);

    if (currentValues[sensorNumber].size() > windowSize)
        currentValues[sensorNumber].erase(currentValues[sensorNumber].begin());

	std::vector<float> temp = currentValues[sensorNumber];
	
	std::sort(temp.begin(), temp.end());

	if (temp.size() >= 3)
		filteredValues[sensorNumber] = std::accumulate(temp.begin() + 1, temp.end() - 1, 0.0f) / (windowSize - 2);
	
	SetFurnaceState();

	return std::make_pair(filteredValues[sensorNumber], I);
}

float SensorsMain::GetFilteredValuesDelta()
{
	return filteredValuesDelta;
}

float SensorsMain::GetDallasTemp(const int numMeasurements)
{
	dallasSensor.requestTemperatures();
	std::vector<float> measurements(numMeasurements);

	for (int i = 0; i < numMeasurements; ++i)
		measurements[i] = dallasSensor.getTempCByIndex(0);

	std::sort(measurements.begin(), measurements.end());

	return measurements[floor(measurements.size() / 2)];
}


std::pair<float, float> SensorsMain::GetSHT3xTempHumidity(const int numMeasurements)
{
	std::pair<float, float> result = {-77, -77};

	if (!sht3x.begin())
	{
		sht3x.begin(sht3xAddress);
		return result;
	}

	float temp = 0;
	for (int i = 0; i < numMeasurements; ++i)
		temp += sht3x.readTemperature();
	result.first = temp / numMeasurements;

	float humidity = 0;
	for (int i = 0; i < numMeasurements; ++i)
		humidity += sht3x.readHumidity();
	result.second = humidity / numMeasurements;

	return result;
}


// Returns median of numMeasurements of 3 measurements
float SensorsMain::GetWeightGrams(const int numMeasurements)
{
	std::vector<float> measurements(numMeasurements);

	for (int i = 0; i < numMeasurements; ++i)
		measurements[i] = loadCell.get_units(3);

	std::sort(measurements.begin(), measurements.end());

	return measurements[floor(measurements.size() / 2)];
}


float SensorsMain::GetPelletWeightGrams(const int numMeasurements)
{
	return GetWeightGrams(numMeasurements) - emptyTankWeight;
}


float SensorsMain::GetLoadCellCalibrationFactor() const
{
	return loadCellCalibrationFactor;
}


void SensorsMain::SetConfig(float newLoadCellCalibrationFactor, float newLoadCellOffset, float newEmptyTankWeight, float newFurnaceOnThreshold, float newFurnaceOffThreshold)
{	
	loadCellCalibrationFactor = newLoadCellCalibrationFactor;
	loadCellOffset = newLoadCellOffset;
	emptyTankWeight = newEmptyTankWeight;
	furnaceOnThreshold = newFurnaceOnThreshold;
	furnaceOffThreshold = newFurnaceOffThreshold;
}


void SensorsMain::ResetConfig()
{
	loadCellCalibrationFactor = DefaultConfig::loadCellCalibrationFactor;
	loadCellOffset = DefaultConfig::loadCellOffset;
	emptyTankWeight = DefaultConfig::emptyTankWeight;
	furnaceOnThreshold = DefaultConfig::furnaceOnThreshold;
	furnaceOffThreshold = DefaultConfig::furnaceOffThreshold;
}

StaticJsonDocument<200> SensorsMain::GetConfig() const
{
	StaticJsonDocument<200> sensorConfig;
	
    sensorConfig["loadCellCalibrationFactor"] = loadCellCalibrationFactor;
    sensorConfig["loadCellOffset"] = loadCellOffset;
    sensorConfig["emptyTankWeight"] = emptyTankWeight;

    sensorConfig["furnaceOnThreshold"] = furnaceOnThreshold;
    sensorConfig["furnaceOffThreshold"] = furnaceOffThreshold;

    return sensorConfig;
}

bool SensorsMain::GetRelayState() const
{
	return int(relayState);
}


int SensorsMain::GetFurnaceState() const
{
	return int(furnaceState);
}
