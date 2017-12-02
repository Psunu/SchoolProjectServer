#include "Time.h"

const struct tm * Time::Get_Time() const
{
    return curr_tm;
}

int Time::Get_year()
{
    update();
    return curr_tm->tm_year + 1900;
}

int Time::Get_mon() 
{
    update();
    return curr_tm->tm_mon + 1;
}

int Time::Get_day() 
{
    update();
    return curr_tm->tm_mday;
}

int Time::Get_hour() 
{
    update();
    return curr_tm->tm_hour;
}

int Time::Get_min() 
{
    update();
    return curr_tm->tm_min;
}

int Time::Get_sec() 
{
    update();
    return curr_tm->tm_sec;
}

int Time::Get_week()
{
	update();
	return curr_tm->tm_wday;
}
