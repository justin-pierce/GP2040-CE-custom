#ifndef _CleanCircularInputs_H_
#define _CleanCircularInputs_H_

#include "gpaddon.h"
#include <queue>

#ifndef CLEAN_CIRCULAR_INPUTS_ENABLED
#define CLEAN_CIRCULAR_INPUTS_ENABLED true
#endif  // CLEAN_CIRCULAR_INPUTS_ENABLED

#ifndef CLEAN_CIRCULAR_INPUTS_DELAY
#define CLEAN_CIRCULAR_INPUTS_DELAY 30
#endif  // CLEAN_CIRCULAR_INPUTS_DELAY

// Reverse Module Name
#define CleanCircularInputsName "Clean Circular Inputs"


class CleanCircularInputs : public GPAddon {
public:
	virtual bool available();   // GPAddon available
	virtual void setup();       // Reverse Button Setup
	virtual void process();     // Reverse process
    virtual std::string name() { return CleanCircularInputsName; }
private:

	// settings
	uint32_t cleanInputDelay;
	uint32_t cleanInputJitter;
	uint8_t pinButtonToggleActive;
	uint8_t pinActivityLED;

	// working vars
	bool isActive;
	bool idlePinButtonToggleActive;
	uint16_t lastCleanDpad;
	uint16_t lastRealDpad;
	uint16_t currentDpad;
	uint32_t nextTimer;
	std::queue<uint16_t> inputQueue;

	// methods
	uint16_t GetInputForSequence(uint16_t first, uint16_t second);
	void ResetTimer();

};

#endif // _CleanCircularInputs_H