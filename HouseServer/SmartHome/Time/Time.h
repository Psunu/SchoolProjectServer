#ifndef __SUNWOO_TIME_H
#define __SUNWOO_TIME_H
#include <ctime>

class Time {
private:
    time_t curr_time;
    struct tm *curr_tm;

public:
    Time()
    {
    	curr_time = time(NULL);
    	curr_tm = localtime(&curr_time);
    }

	// Update current time
    void update()
    {
    	curr_time = time(NULL);
    	curr_tm = localtime(&curr_time);
    }

    const struct tm * Get_Time() const;
    int Get_year();
    int Get_mon();
    int Get_day();
    int Get_hour();
    int Get_min();
    int Get_sec();
};

#endif
