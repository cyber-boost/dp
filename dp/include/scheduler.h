#ifndef SCHEDULER_H
#define SCHEDULER_H

// Function declarations for scheduling functionality
typedef struct {
    int hour;
    int minute;
    char *wallpaper_path;
} TimeSlot;

int start_scheduler(int interval_seconds);
void stop_scheduler(void);
TimeSlot* get_time_based_wallpaper(void);

#endif // SCHEDULER_H