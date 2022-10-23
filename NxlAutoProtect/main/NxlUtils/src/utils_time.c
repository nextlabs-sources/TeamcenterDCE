#include "utils_time.h"
#include "utils_log.h"


const char *time_tostring(time_t time)
{
	static char timeBuf[19+1];//YYYY-mm-dd HH:MM:ss\0
	struct tm tm;
#if defined(WNT)
	localtime_s(&tm, &time);
#elif defined(__linux__)
	localtime_r(&time, &tm);
#endif
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

	return timeBuf;
}

time_t time_convert(struct tm tm, float timezone)
{
	time_t outputTime = mktime(&tm);
	DTLLOG("Converting %s to Timezone('%.1f')", time_tostring(outputTime), timezone);
	if(timezone >=-14 && timezone <= 12)
	{
		long diff;
		long dstbias = 0, tz = 0;
#if defined(WNT)
		_get_dstbias(&dstbias);
		_get_timezone(&tz);
		_tzset();//initial timezone varaiables
#elif defined(__linux__)
		time_t t = time(NULL);
		struct tm lt = {0};
		localtime_r(&t, &lt);
		DBGLOG("Timezone='%s' GMTOff=%l DST=%d", lt.tm_zone, lt.tm_gmtoff, lt.tm_isdst);
		if(lt.tm_isdst > 0)
		{
			dstbias = 3600;
		}
		tz = lt.tm_gmtoff;
#endif
		DTLLOG("==>Target Timezone=%.1fH Current Timezone=%ds(%.1fH) Current DaylightSavingBias=%ds(%.1fH)",
			timezone,
			tz*-1, tz*-1/3600.0,
			dstbias*-1, dstbias*-1/3600.0);
		//convert time as expected timezone
		diff = (tz + dstbias)*-1 - (int)(timezone*3600);
		DTLLOG("==>Time Difference is %lds(%.1fH)", diff, diff/3600.0);
		if(diff != 0)
		{
			outputTime += diff;
		}
	}
	DTLLOG("==>Converted to %s", time_tostring(outputTime));
	return outputTime;

}