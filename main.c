#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
// this is test comment 1
// this is test comment 2

char* date_formatter(char* date);
char* time_unit_converter(int hrs);
char* date_formatter(char* date);
void file_handler();
void file_formatter(char* f_start_date, char* f_end_date, char* f_start_time, char* f_end_time);
long int get_epoch(int day, int month, int year, int hour, int minute);
long int get_epoch_from_string(char datestring[64], char timestring[64]);

int main(){
    char start_date[64] = {0};
    char end_date[64] = {0};
    int start_time = 0;
    int end_time = 0;

    char* formatted_start_time = "";
    char* formatted_end_time = "";
    char* formatted_start_date = "";
    char* formatted_end_date = "";

    // program doesn't yet calculate the average length of each message...

    printf("Enter the start date (DD-MM-YYYY): ");
    fgets(start_date, sizeof(start_date), stdin);
    start_date[strcspn(start_date, "\n")] = 0;          //strcspn?
    formatted_start_date = date_formatter(start_date);

    printf("Enter the start time in hrs: ");
    scanf(" %d", &start_time);
    formatted_start_time = time_unit_converter(start_time);
    getchar();              // clear the buffer (removes the Enter key).

    printf("Enter the end date (DD-MM-YYYY): ");
    fgets(end_date, sizeof(end_date), stdin);
    end_date[strcspn(end_date, "\n")] = 0;
    formatted_end_date = date_formatter(end_date);

    printf("Enter the end time in hrs: ");
    scanf(" %d", &end_time);
    formatted_end_time = time_unit_converter(end_time);
    getchar();

    printf("formatted start time:  %s\n", formatted_start_time);
    printf("formatted end time: %s", formatted_end_time);

    file_formatter(formatted_start_date, formatted_end_date, formatted_start_time, formatted_end_time);
    file_handler();

    free(formatted_end_time);       // time strings are correctly freed up
    free(formatted_start_time);

    return 0;
}


char* date_formatter(char* date){
    int d, m, y;

    if (sscanf(date, "%d-%d-%d", &d, &m, &y) == 3 ||
        sscanf(date, "%d/%d/%d", &d, &m, &y) == 3 ||
        sscanf(date, "%d %d %d", &d, &m, &y) == 3){
            if (y < 100){
                y += 2000;
            }
            sprintf(date, "%02d/%02d/%04d", d, m, y);           //sprintf?
    }

    return date;
}


char* time_unit_converter(int hrs){
    int hours = (hrs / 100);     // account for if the user enters something like 1675, then add an hour and make the last two digits into 15
    int minutes = hrs % 100;
    char* meridian = hrs > 1200 ? "pm" : "am";

    if (minutes >= 60){
        hours = hours + (minutes / 60);
        minutes = minutes % 60;  
    }

    hours = hours % 12;
    if (hours == 0) hours = 12;

    char *time_string = malloc(11 * sizeof(char));
    if (time_string == NULL) return NULL;

    snprintf(time_string, 11, "%02d:%02d %s", hours, minutes, meridian);     //snprintf?

    return time_string;
}


void file_formatter(char* f_start_date, char* f_end_date, char* f_start_time, char* f_end_time){
    FILE *chats_file = fopen("WhatsApp Chat.txt", "r");
    FILE *temp_file = fopen("temp.txt", "w");

    char reading_buffer[1024] = {0};

    if (chats_file == NULL){
        printf("could not open the source file");
        return;
    }

    if (temp_file == NULL){
        printf("could not create destination file");
        return;
    }

    while (fgets(reading_buffer, sizeof(reading_buffer), chats_file) != NULL){
        if (isdigit(reading_buffer[1]) &&
            isdigit(reading_buffer[0]) &&
            reading_buffer[2] == '/') {
            if (reading_buffer[13] == ':'){
                int len = strlen(reading_buffer);             
                for (int i = len; i >= 12; i--){
                    reading_buffer[i+1] = reading_buffer[i];
                }
                reading_buffer[12] = '0';           //formatted file is returned with the correct format, that the time appears to be 09:22 am
            }
            fputs(reading_buffer, temp_file);      
        }
    }

    fclose(chats_file);
    fclose(temp_file);

    // check if using the same name, ie using Formatted Text.txt in place of temp.txt would work.

    FILE *formatted_file = fopen("Formatted Text.txt", "w");
    FILE *temp = fopen("temp.txt", "r");
    int d = 0, m = 0, y = 0, hr = 0, min = 0;
    time_t current_epoch = 0;       // somehow try to mix the getting of start epoch, end epoch and current epoch with a centralised function.
    //long int current_epoch = time(NULL);
    long int start_epoch = get_epoch_from_string(f_start_date, f_start_time);         // convert the start and end timestamps into corresponding epoch times.
    long int end_epoch = get_epoch_from_string(f_end_date, f_end_time);

    printf("\nstarting epoch, %ld", start_epoch);
    printf("\nending epoch, %ld", end_epoch);
    //printf("\ncurrent epoch, %ld", current_epoch);
    
    char meridian[8] = {0};
    char temp_mem[1024] = {0};

    while (fgets(temp_mem, 1024, temp) != NULL){
        sscanf(temp_mem, "%d/%d/%d %d:%d %s", &d, &m, &y, &hr, &min, meridian);
        if (strcmp(meridian, "pm") == 0 && hr != 12) hr+= 12;
        if (strcmp(meridian, "am") == 0 && hr == 12) hr = 0;
        current_epoch = get_epoch(d, m, y, hr, min);
        
        if (start_epoch <= current_epoch && end_epoch >= current_epoch){
            fputs(temp_mem, formatted_file);
        }
    }
    

    fclose(temp);
    remove("temp.txt");
    fclose(formatted_file);

    // get the stripped file such that it only contains the chats between the specified timestamps.

    FILE *stripped_file = fopen("Stripped Text.txt", "w");          // contains only the text, the date and timestamps removed.
    FILE *formatted_file_ = fopen("Formatted Text.txt", "r");

    char buffer[1024] = {0};

    if (formatted_file_ == NULL){
        printf("could not open the formatted file");
        return;
    }

    if (stripped_file == NULL){
        printf("could not create destination file");
        return;
    }

    while (fgets(buffer, 1024, formatted_file_) != NULL){
        fputs(buffer + 25, stripped_file);
    }

    fclose(stripped_file);
    fclose(formatted_file_);
}

long int get_epoch_from_string(char datestring[64], char timestring[64]){ // calculate the size of datestring and timestring, and use that instead of *
    int d = 0, m = 0, y = 0, hr = 0, min = 0;
    char meridian[3] = {0};
    char temp_mem[64] = {0};
    snprintf(temp_mem, sizeof(temp_mem), "%s %s", datestring, timestring);
    sscanf(temp_mem, "%d/%d/%d %d:%d %s", &d, &m, &y, &hr, &min, &meridian);
    if (strcmp(meridian, "pm") == 0 && hr != 12) hr+= 12;
    if (strcmp(meridian, "am") == 0 && hr == 12) hr = 0;
    return get_epoch(d, m, y, hr, min);
}

long get_epoch(int day, int month, int year, int hour, int minute){
    struct tm t = {0};
    time_t t_of_day;
    
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = 0;
    t.tm_isdst = -1;            // is daylight saving time in effect

    t_of_day = mktime(&t);

    return (long)t_of_day;
}

void file_handler(){
    
}



















