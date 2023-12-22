#ifndef ACS712_h
#define ACS712_h

// Modified Library [ACS712 Current Sensor by Ruslan Koptiev]

#include <Arduino.h>

#define ADC_SCALE 4095.0	// Modified
#define VREF 4.8	// Modified
#define DEFAULT_FREQUENCY 50

enum ACS712_type {ACS712_05B, ACS712_20A, ACS712_30A};

class ACS712 {
public:
	ACS712() = default;
	ACS712(ACS712_type type, uint8_t _pin);
	int calibrate();	// Modified
	int calibrateAC();	// Added
	void setZeroPoint(int _zero);
	void setSensitivity(float sens);
	float getCurrentDC();
	float getCurrentAC(uint16_t frequency = 50) const;	// Modified

private:
	float currentOffset = 0;	// Added
	float zero = 3000;	// Modified
	float sensitivity;
	uint8_t pin;
};

#endif