#include "DefaultConfig.h"

namespace DefaultConfig
{
	const float samplingRateSeconds = 7.5;
	const int vibrationResetMqttSeconds = 1;

	const float loadCellCalibrationFactor = -24.29624;
	const float loadCellOffset = 155273;
	const float emptyTankWeight = 44*1000;
	
	const float furnaceOnThreshold = 0.5;
	const float furnaceOffThreshold = 0.0;
}