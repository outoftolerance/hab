#ifndef Timer_h
#define Timer_h

#include <Arduino.h>

#define DEFAULT_INTERVAL 1000;

/**
 * @brief Timer class definition. Class provides a structure to allow timing of program elements.
 */
class Timer 
{
private:
	unsigned long last_reset_;      /**< Time of last timer reset in ms */
	unsigned long interval_;        /**< Timer interval in ms */
        bool is_set_;                   /**< Timer is set and ready status */
        bool is_running_;               /**< Timer is currently started and running */
public:
        /**
         * @brief Timer constructor function
         */
	Timer();

        /**
         * @brief Timer constructor function with interval input
         * param interval The timer interval in ms
         */
	Timer(unsigned long interval);

        /**
         * @brief Timer constructor function
         * param interval The timer interval in ms
         * @return True if interval set successfully
         */
	bool setInterval(unsigned long interval);

        /**
         * @brief Timer constructor function
         * return The currently set timer interval in ms
         */
	unsigned long getInterval();

        /**
         * @brief Checks if timer is set correctly
         * @return True if timer interval is set, false if not
         */
        bool isSet();

        /**
         * @brief Checks if the timer is running currently
         * @return True if running, false if not
         */
        bool isStarted();

        /**
         * @brief Starts the timer if it's stopped and if it's set corrently
         * @return True if timer transitions from stopped to started. False otherwise.
         */
        bool start();

        /**
         * @brief Stops the timer if it's started
         * @return True if timer transitions from started to stopped. False otherwise.
         */
        bool stop();

        /**
         * @brief Checks the timer if it's elapsed beyond interval, does not reset timer when called
         * @return True if timer is elapsed, false otherwise.
         */
	bool check();

        /**
         * @brief Resets the timer if it's elapsed. Won't reset if Timer::check() returns false.
         * @return True if reset success.
         */
	bool reset();

        /**
         * @brief Resets the timer no matter the status.
         * @return True if reset success.
         */
        bool forceReset();
};

#endif
