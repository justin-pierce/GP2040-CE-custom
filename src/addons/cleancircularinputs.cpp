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
    // BoardOptions boardOptions = Storage::getInstance().getBoardOptions();

    lastCleanDpad = 0;
    lastRealDpad = 0;
    // cleanInputDelay = boardOptions.cleanCircularInputsDelay;
    // cleanInputDelay = boardOptions.cleanCircularInputsDelayJitter;
    cleanInputDelay = 30;
    // cleanInputJitter = 5;
    nextTimer = 0;

    pinActivityLED = 25;

    gpio_init(pinActivityLED);
    gpio_set_dir(pinActivityLED, GPIO_OUT);
    gpio_put(pinActivityLED, 0);

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
            gpio_put(pinActivityLED, 1);
        }
        else
        {
            gpio_put(pinActivityLED, 0);
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
