/* A little program to test the limits of your system's time functions
 * See Porting/README.y2038 for details
 */

#include <time.h>
#include <stdio.h>
#include <math.h>

time_t Time_Zero = 0;

/* Visual C++ 2008's difftime() can't do negative times */
double my_difftime(time_t left, time_t right) {
        double diff = (double)left - (double)right;
        return diff;
}

void check_date_max( struct tm * (*date_func)(const time_t *), char *func_name ) {
    struct tm *date;
    time_t time = 0;
    time_t last_time = 0;
    time_t time_change;
    int i;

    for (i = 0; i <= 63; i++) {
        date = (*date_func)(&time);

        /* date_func() broke or tm_year overflowed */
        if(date == NULL || date->tm_year < 69)
          break;

        last_time = time;
        time += time + 1;

        /* time_t overflowed */
        if( time < last_time )
            break;
    }

    /* Binary search for the exact failure point */
    time = last_time;
    time_change = last_time / 2;

    do {
        time += time_change;

        date = (*date_func)(&time);

        /* date_func() broke or tm_year overflowed or time_t overflowed */
        if(date == NULL || date->tm_year < 69 || time < last_time) {
            time = last_time;
            time_change = time_change / 2;
        }
        else {
            last_time = time;
        }
    } while(time_change > 0);

    printf("%20s max %.0f\n", func_name, my_difftime(last_time, Time_Zero));
}


void check_date_min( struct tm * (*date_func)(const time_t *), char *func_name ) {
    struct tm *date;
    time_t time = -1;
    time_t last_time = 0;
    time_t time_change;
    int i;

    for (i = 1; i <= 63; i++) {
        date = (*date_func)(&time);

        /* date_func() broke or tm_year underflowed */
        if(date == NULL || date->tm_year > 70)
            break;

        last_time = time;
        time += time;

        /* time_t underflowed */
        if( time > last_time )
            break;
    }

    /* Binary search for the exact failure point */
    time = last_time;
    time_change = last_time / 2;

    do {
        time += time_change;

        date = (*date_func)(&time);

        /* gmtime() broke or tm_year overflowed or time_t overflowed */
        if(date == NULL || date->tm_year > 70 || time > last_time) {
            time = last_time;
            time_change = time_change / 2;
        }
        else {
            last_time = time;
        }
    } while(time_change < 0);

    printf("%20s min %.0f\n", func_name, my_difftime(last_time, Time_Zero));
}


int main(void) {
    check_date_max(gmtime, "gmtime");
    check_date_max(localtime, "localtime");
    check_date_min(gmtime, "gmtime");
    check_date_min(localtime, "localtime");

    return 0;
}
