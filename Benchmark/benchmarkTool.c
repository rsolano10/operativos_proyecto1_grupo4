#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#include <pthread.h>
#include <time.h>
#include <curl/curl.h>

double start_time, run_time;

struct thread_args
{
    char *machine;
    char *port;
    char *file;
    int cycles;
};

struct thread_args dataStruct;
double fileSize;
char contentType[1000];

// Auxilar function for curl library
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// count the number of lines in the file called stats.csv
int countLines()
{
    FILE *fp = fopen("stats.csv", "r");
    char filechar[40], ch;
    int lines = 0;

    if (fp == NULL)
    {
        return 0;
    }
    lines++;
    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
        {
            lines++;
        }
    }
    fclose(fp);
    return lines;
}

// write the stats in csv file
void writeFile(char **argv, char *content, double size)
{
    FILE *f;
    if (countLines() == 0) // if the file doesnt exist, writes the header or columns names
    {
        f = fopen("stats.csv", "w");
        fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s\n", "DateTime", "Machine", "Port", "File", "Content-Type", "Size (bytes)", "Threads", "Cycles", "Duration (Seconds)");
        fclose(f);
    }
    f = fopen("stats.csv", "a");
    char date[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(f, "%s,%s,%s,%s,%s,%f,%s,%s,%f\n", date, argv[1], argv[2], argv[3], content, size, argv[4], argv[5], run_time); // save the stats in the file
    fclose(f);
}

// Execute the request n-cycles of times
void *makeRequest(void *arg)
{
    char *machine = NULL, *port = NULL, *file = NULL;
    struct thread_args data = *(struct thread_args *)arg;

    machine = data.machine;
    port = data.port;
    file = data.file;
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char link[1000];
    sprintf(link, "%s:%s/%s", machine, port, file); //build the url

    int cycles = data.cycles;
    for (int i = 0; i < cycles; i++) // execute the requests
    {
        curl = curl_easy_init();
        if (curl)
        {
            fp = fopen(file, "wb"); // file for download 
            curl_easy_setopt(curl, CURLOPT_URL, link);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            if (!res)
            {
                char *ct = NULL;
                curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &fileSize); /* check the size */
                curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);        /* check the content type */
                sprintf(contentType, "%s", ct);
            }
            /* always cleanup */
            curl_easy_cleanup(curl);
            fclose(fp);
        }
    }
}

//Build the request and create each thread
void buildRequest(char **argv)
{
    long threads = 0, cycles = 0;
    dataStruct.machine = argv[1];
    dataStruct.port = argv[2];
    dataStruct.file = argv[3];
    threads = strtol(argv[4], NULL, 10);
    cycles = strtol(argv[5], NULL, 10);
    dataStruct.cycles = cycles;
    pthread_t tt[threads];
    for (int i = 0; i < threads; i++)
    {
        if (pthread_create(&tt[i], NULL, makeRequest, &dataStruct) != 0)
            printf("Failed to create thread\n");
    }
    for (int i = 0; i < threads; i++)
        pthread_join(tt[i], NULL);
}

//Main Function
int main(int argc, char **argv)
{
    if (argc < 1)
    {
        return 0;
    }
    start_time = omp_get_wtime();
    buildRequest(argv);
    run_time = omp_get_wtime() - start_time; // Get the execution time
    printf("Result *********************** \n");
    printf("Time: %f\t", run_time);
    printf("Content-Type: %s\t", contentType);
    printf("Size: %0.0f Bytes\t\n", fileSize);
    writeFile(argv, contentType, fileSize); // Save the stats
    return 0;
}