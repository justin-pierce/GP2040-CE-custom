#include "addons/cleancircularinputs.h"
#include "storagemanager.h"
// #include "GamepadEnums.h"

#define GAMEPAD_MASK_UPLEFT (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT)
#define GAMEPAD_MASK_UPRIGHT (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT)
#define GAMEPAD_MASK_DOWNLEFT (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT)
#define GAMEPAD_MASK_DOWNRIGHT (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT)

bool CleanCircularInputs::available() {
	// BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    // return boardOptions.cleanCircularInputsEnabled;

    return true;
}

void CleanCircularInputs::setup()
{
    // Setup Clean Circular Inputs
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();

    isActive = false;
    lastCleanDpad = 0;
    lastRealDpad = 0;
    // cleanInputDelay = boardOptions.cleanCircularInputsDelay;
    // cleanInputDelay = boardOptions.cleanCircularInputsDelayJitter;
    cleanInputDelay = 30;
    // cleanInputJitter = 5;
    nextTimer = 0;

    pinButtonToggleActive = 22;
    pinActivityLED = 25;

    gpio_init(pinButtonToggleActive);             // Initialize pin
    gpio_set_dir(pinButtonToggleActive, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinButtonToggleActive);          // Set as PULLUP

    // steal turbo LED pin until we get web configurator
    pinActivityLED = boardOptions.pinTurboLED;
    if (pinActivityLED != -1)
    {
        gpio_init(pinActivityLED);              // Initialize pin
        gpio_set_dir(pinActivityLED, GPIO_OUT); // Set as OUTPUT
        gpio_put(pinActivityLED, isActive ? 1: 0);            // Set initial value
    }

    

    // srand(getMillis());
}

// ←↑→↓↖↗↘↙
uint16_t CleanCircularInputs::GetInputForSequence(uint16_t first, uint16_t second)
{
    
    if(first == GAMEPAD_MASK_DOWNLEFT && second == GAMEPAD_MASK_DOWNRIGHT) // ↙↓↘
    {
        return GAMEPAD_MASK_DOWN;
    }
    else if(first == GAMEPAD_MASK_DOWNRIGHT && second == GAMEPAD_MASK_DOWNLEFT) // ↘↓↙
    {
        return GAMEPAD_MASK_DOWN;
    }
    else if(first == GAMEPAD_MASK_DOWNLEFT && second == GAMEPAD_MASK_UPLEFT) // ↙←↖
    {
        return GAMEPAD_MASK_LEFT;
    }
    else if(first == GAMEPAD_MASK_UPLEFT && second == GAMEPAD_MASK_DOWNLEFT) // ↖←↙
    {
        return GAMEPAD_MASK_LEFT;
    }
    else if(first == GAMEPAD_MASK_DOWNRIGHT && second == GAMEPAD_MASK_UPRIGHT) // ↘→↗
    {
        return GAMEPAD_MASK_RIGHT;
    }
    else if(first == GAMEPAD_MASK_UPRIGHT && second == GAMEPAD_MASK_DOWNRIGHT) // ↗→↘
    {
        return GAMEPAD_MASK_RIGHT;
    }
    else if(first == GAMEPAD_MASK_UPLEFT && second == GAMEPAD_MASK_UPRIGHT) // ↖↑↗
    {
        return GAMEPAD_MASK_UP;
    }
    else if(first == GAMEPAD_MASK_UPRIGHT && second == GAMEPAD_MASK_UPLEFT) // ↗↑↖
    {
        return GAMEPAD_MASK_UP;
    }
    else
    {
        return 0;
    }    

    // see if we can adopt clean switch case formatting

    // can probably do this: https://stackoverflow.com/questions/8644096/multiple-conditions-in-switch-case

    // switch (gamepad->options.inputMode)
	// {
	// 	case INPUT_MODE_HID:    statusBar += "DINPUT"; break;
	// 	case INPUT_MODE_SWITCH: statusBar += "SWITCH"; break;
	// 	case INPUT_MODE_XINPUT: statusBar += "XINPUT"; break;
	// 	case INPUT_MODE_CONFIG: statusBar += "CONFIG"; break;
    //  default: 0; 
	// }
}

void CleanCircularInputs::ResetTimer()
{
    // int randomDelay = 0;

    // int nextRand = rand() % cleanInputJitter;
    // int nextSign = rand() % cleanInputJitter;
    // if (nextSign < cleanInputJitter / 2)
    // {
    //     randomDelay = -nextRand;
    // }
    // else
    // {
    //     randomDelay = nextRand;
    // }

    // randomDelay += cleanInputDelay;

    // randomDelay = std::min(1, randomDelay);

    // nextTimer = getMillis() + randomDelay;

    nextTimer = getMillis() + cleanInputDelay;
}

void CleanCircularInputs::process()
{
    // detect if ToggleActive button has been pressed
    if(gpio_get(pinButtonToggleActive) && idlePinButtonToggleActive)
    {
        // BoardOptions boardOptions = Storage::getInstance().getBoardOptions();

        // boardOptions.showDisplayMenu = !boardOptions.showDisplayMenu;

        // update LED to reflect state
        // gpio_put(pinActivityLED, boardOptions.showDisplayMenu ? 1 : 0);

        // Storage::getInstance().setBoardOptions(boardOptions);



        isActive = !isActive;
        
        // clear input queue
        while(!inputQueue.empty()) inputQueue.pop();

        // update LED
        gpio_put(pinActivityLED, isActive ? 1 : 0);

        // so we can wait until next distinct press
        idlePinButtonToggleActive = false;
    }

    // check if they've let go of button
    if(!gpio_get(pinButtonToggleActive))
        idlePinButtonToggleActive = true;

    if(!isActive)
        return;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    currentDpad = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    uint16_t cleanDpad = currentDpad;

    // check if we're ready for next item in queue
    if(!inputQueue.empty() && getMillis() > nextTimer)
    {
        inputQueue.pop();
        
        if(!inputQueue.empty())
        {
            ResetTimer();
            gpio_put(pinActivityLED, 0);
        }
        else
        {
            gpio_put(pinActivityLED, 1);
        }
    }    

    // if queue still not empty, override input and queue up next inputs that happen while we're injecting cleaned inputs
    if(!inputQueue.empty())
    {
        cleanDpad = inputQueue.front();     

        if(lastRealDpad != currentDpad)
        {
            uint16_t inputToInject = GetInputForSequence(lastRealDpad, currentDpad);
            if(inputToInject == 0) // no cleaning needed
            {
                inputQueue.push(currentDpad);
            }
            else // add both cleaned input and next input
            {
                inputQueue.push(inputToInject);
                inputQueue.push(currentDpad);
            }
        }
    }
    else // we're processing inputs in realtime, check if we need to clean anything
    {
        if(lastRealDpad != currentDpad)
        {
            uint16_t inputToInject = GetInputForSequence(lastRealDpad, currentDpad);
            if(inputToInject != 0) // found new input to clean
            {
                // add clean input and next input to queue
                inputQueue.push(inputToInject);
                inputQueue.push(currentDpad);

                // set next timer
                ResetTimer();

                // set this tick's input  
                cleanDpad = inputQueue.front();
            }
            
        }
    }

    lastCleanDpad = cleanDpad;
    lastRealDpad = currentDpad;

    if(cleanDpad != currentDpad)
    {
        gamepad->state.dpad = cleanDpad;
    }
}
