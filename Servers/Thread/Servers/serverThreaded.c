#include <arpa/inet.h> /* inet_ntoa */
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
//#include "MyPthreads/mypthread.h"
#include "config.c"

#define LISTENQ 1024 /* second argument to listen() */
#define MAXLINE 1024 /* max length of a line */
#define RIO_BUFSIZE 1024
#define LOG_PATH ""
#define THREAD 4

typedef struct
{
    int rio_fd;                /* descriptor for this buf */
    int rio_cnt;               /* unread byte in this buf */
    char *rio_bufptr;          /* next unread byte in this buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

typedef struct
{
    char filename[512];
    off_t offset; /* for support Range */
    size_t end;
} http_request;

typedef struct /* struct for files information */
{
    const char *extension;
    const char *mime_type;
} mime_map;

/* extensions a mime types */
mime_map meme_types[] = {
    {".css", "text/css"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".ico", "image/x-icon"},
    {".js", "application/javascript"},
    {".pdf", "application/pdf"},
    {".mp4", "video/mp4"},
    {".png", "image/png"},
    {".svg", "image/svg+xml"},
    {".xml", "text/xml"},
    {".php", "application/php"},
    {NULL, NULL},
};

char thispath[300];
char *default_mime_type = "text/plain";
struct sockaddr_in clientaddr;

void writeFile(char textToPrint[], int numberToPrint)
{
    //printf("listen on port %s\n", (char *)thispath);
    if (fopen(thispath, "a+"))
    {

        FILE *f = fopen(thispath, "a+");
        /* print some text */
        fprintf(f, "%s %d\n", textToPrint, numberToPrint);

        fclose(f);
    }
    else
    {
        setlogmask(LOG_UPTO(LOG_NOTICE));

        openlog("WebServer", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

        syslog(LOG_NOTICE, "%s %d", textToPrint, numberToPrint);

        closelog();
    }
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) <= 0)
        {
            if (errno == EINTR) /* interrupted by sig handler return */
                nwritten = 0;   /* and call write() again */
            else
                return -1; /* errorno set by write() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

/*
 * rio_read - This is a wrapper for the Unix read() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    read() if the internal buffer is empty.
 */
/* $begin rio_read */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;
    while (rp->rio_cnt <= 0)
    { /* refill if buf is empty */

        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
                           sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0)
        {
            if (errno != EINTR) /* interrupted by sig handler return */
                return -1;
        }
        else if (rp->rio_cnt == 0) /* EOF */
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

/*
 * rio_readlineb - robustly read a text line (buffered)
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++)
    {
        if ((rc = rio_read(rp, &c, 1)) == 1)
        {
            *bufp++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        }
        else
            return -1; /* error */
    }
    *bufp = 0;
    return n;
}

/* Format for document size or DIR */
void format_size(char *buf, struct stat *stat)
{
    if (S_ISDIR(stat->st_mode))
    {
        sprintf(buf, "%s", "[DIR]");
    }
    else
    {
        off_t size = stat->st_size;
        if (size < 1024)
        {
            sprintf(buf, "%lu", size);
        }
        else if (size < 1024 * 1024)
        {
            sprintf(buf, "%.1fK", (double)size / 1024);
        }
        else if (size < 1024 * 1024 * 1024)
        {
            sprintf(buf, "%.1fM", (double)size / 1024 / 1024);
        }
        else
        {
            sprintf(buf, "%.1fG", (double)size / 1024 / 1024 / 1024);
        }
    }
}

// Get directory info for show it in the server page
void handle_directory_request(int out_fd, int dir_fd, char *filename)
{
    char buf[MAXLINE], m_time[32], size[16];
    struct stat statbuf;
    sprintf(buf, "HTTP/1.1 200 OK\r\n%s%s%s%s%s",
            "Content-Type: text/html\r\n\r\n",
            "<html><head><style>",
            "body{font-family: monospace; font-size: 13px;}",
            "td {padding: 1.5px 6px;}",
            "</style></head><body><table>\n");
    writen(out_fd, buf, strlen(buf));
    DIR *d = fdopendir(dir_fd);
    struct dirent *dp;
    int ffd;
    while ((dp = readdir(d)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {
            continue;
        }
        if ((ffd = openat(dir_fd, dp->d_name, O_RDONLY)) == -1)
        {
            perror(dp->d_name);
            continue;
        }
        fstat(ffd, &statbuf);
        strftime(m_time, sizeof(m_time),
                 "%Y-%m-%d %H:%M", localtime(&statbuf.st_mtime));
        format_size(size, &statbuf);
        if (S_ISREG(statbuf.st_mode) || S_ISDIR(statbuf.st_mode)) /* validation if is DIR or FILE */
        {
            char *d = S_ISDIR(statbuf.st_mode) ? "/" : "";
            sprintf(buf, "<tr><td><a href=\"%s%s\">%s%s</a></td><td>%s</td><td>%s</td></tr>\n", /* Draw files and dir in server address with own link*/
                    dp->d_name, d, dp->d_name, d, m_time, size);
            writen(out_fd, buf, strlen(buf));
        }
        close(ffd);
    }
    sprintf(buf, "</table></body></html>");
    writen(out_fd, buf, strlen(buf));
    closedir(d);
}

// get fie type or file mie type
static const char *get_mime_type(char *filename)
{
    char *dot = strrchr(filename, '.');
    if (dot)
    { // strrchar Locate last occurrence of character in string
        mime_map *map = meme_types;
        while (map->extension)
        {
            if (strcmp(map->extension, dot) == 0)
            {
                return map->mime_type;
            }
            map++;
        }
    }
    return default_mime_type;
}

/* get the files physical link */
void url_decode(char *src, char *dest, int max)
{
    char *p = src;
    char code[3] = {0};
    while (*p && --max)
    {
        if (*p == '%')
        {
            memcpy(code, ++p, 2);
            *dest++ = (char)strtoul(code, NULL, 16);
            p += 2;
        }
        else
        {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}

/* process the file request clicked in the server */
void parse_request(int fd, http_request *req)
{
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE];
    req->offset = 0;
    req->end = 0; /* default */

    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s", method, uri); /* version is not cared */
    /* read all */
    while (buf[0] != '\n' && buf[1] != '\n')
    { /* \n || \r\n */
        rio_readlineb(&rio, buf, MAXLINE);
        if (buf[0] == 'R' && buf[1] == 'a' && buf[2] == 'n')
        {
            sscanf(buf, "Range: bytes=%lu-%lu", &req->offset, &req->end);
            // Range: [start, end]
            if (req->end != 0)
                req->end++;
        }
    }
    char *filename = uri;
    if (uri[0] == '/')
    {
        filename = uri + 1;
        int length = strlen(filename);
        if (length == 0)
        {
            filename = ".";
        }
        else
        {
            for (int i = 0; i < length; ++i)
            {
                if (filename[i] == '?')
                {
                    filename[i] = '\0';
                    break;
                }
            }
        }
    }
    url_decode(filename, req->filename, MAXLINE);
}

void log_access(int status, struct sockaddr_in *c_addr, http_request *req)
{
    printf("%s:%d %d - %s\n", inet_ntoa(c_addr->sin_addr),
           ntohs(c_addr->sin_port), status, req->filename);

    char buffer[600];
    sprintf(buffer, "%s:%d %d - %s", inet_ntoa(c_addr->sin_addr),
            ntohs(c_addr->sin_port), status, req->filename);
    writeFile(buffer, 0);
}

void client_error(int fd, int status, char *msg, char *longmsg)
{
    char buf[MAXLINE];
    writeFile("HTTP/1.1, status: ", status);
    writeFile("HTTP/1.1, mensaje: ", 0);
    writeFile(msg, 0);
    sprintf(buf, "HTTP/1.1 %d %s\r\n", status, msg);
    sprintf(buf + strlen(buf),
            "Content-length: %lu\r\n\r\n", strlen(longmsg));
    sprintf(buf + strlen(buf), "%s", longmsg);
    writen(fd, buf, strlen(buf));
}

/* manage the request for download or show the files */
void serve_static(int out_fd, int in_fd, http_request *req,
                  size_t total_size)
{
    char buf[256];
    if (req->offset > 0)
    {
        writeFile("HTTP/1.1 206 Partial", 0);
        sprintf(buf, "HTTP/1.1 206 Partial\r\n");
        sprintf(buf + strlen(buf), "Content-Range: bytes %lu-%lu/%lu\r\n",
                req->offset, req->end, total_size);
    }
    else
    {
        writeFile("HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n", 0);
        sprintf(buf, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n");
    }
    writeFile("Cache-Control: no-cache. ", 0);
    sprintf(buf + strlen(buf), "Cache-Control: no-cache\r\n");
    writeFile("Content-length: ", req->end - req->offset);
    sprintf(buf + strlen(buf), "Content-length: %lu\r\n",
            req->end - req->offset);
    sprintf(buf + strlen(buf), "Content-type: %s\r\n\r\n",
            get_mime_type(req->filename));

    writen(out_fd, buf, strlen(buf));
    off_t offset = req->offset; /* copy */
    while (offset < req->end)
    {
        if (sendfile(out_fd, in_fd, &offset, req->end - req->offset) <= 0)
        {
            break;
        }
        writeFile("offset: ", offset);
        printf("offset: %ld \n\n", offset);
        close(out_fd);
        break;
    }
}

/* manage the new connections accepted and their requests */
void *process(void *arg)
{
    int fd = *((int *)arg);
    writeFile("pid is: ", getpid());
    writeFile("accept request sa, pid is %d\n", getppid());
    printf("accept request as, fd is %d, pid is %d\n", fd, getpid());
    http_request req;
    parse_request(fd, &req);
    int flag = 0;

    struct stat sbuf;
    int status = 200, ffd = open(req.filename, O_RDONLY, 0);
    if (ffd <= 0)
    {
        status = 404;
        char *msg = "File not found";
        // client_error(fd, status, "Not found", msg);
    }
    else
    {
        fstat(ffd, &sbuf);
        if (S_ISREG(sbuf.st_mode))
        { //Se abre o descarga un archivo (GET)
            if (req.end == 0)
            {
                req.end = sbuf.st_size;
            }
            if (req.offset > 0)
            {
                status = 206;
            }
            serve_static(fd, ffd, &req, sbuf.st_size);
            flag = 1;
        }
        else if (S_ISDIR(sbuf.st_mode))
        { //Muestra en contenido de un directorio
            status = 200;
            handle_directory_request(fd, ffd, req.filename);
        }
        else
        {
            status = 400;
            char *msg = "Unknow Error";
            //  client_error(fd, status, "Error", msg);
        }
        close(ffd);
    }
    log_access(status, &clientaddr, &req);
    if (flag == 1)
    {
        pthread_t self;
        self = pthread_self();
        printf("Thread terminated, pid: %ld\n", self);
        writeFile("Thread terminated, pid: ", self);
        pthread_exit(NULL);
    }
}

// Socket and port configuration for connection listen
int setConnection(int port)
{
    struct sockaddr_in serveraddr;
    int listenfd = 0;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);
    return listenfd;
}

// Main Function
int main()
{
    struct config_struct *data;
    data = &config;
    read_config_file(data,THREAD);
    struct sockaddr_in clientaddr;
    int default_port = data->port, listenfd, connfd;
    strcpy(thispath, data->path);
    char buf[256];
    char *path = getcwd(buf, 256);
    socklen_t clientlen = sizeof clientaddr;
    listenfd = setConnection(default_port);
    if (listenfd > 0)
    {
        writeFile("listen on port: ", default_port);
        writeFile("fd is: ", listenfd);
        printf("listen on port %d, fd is %d\n", default_port, listenfd);
    }
    else
    {
        writeFile("Error listening port", default_port);
        perror("ERROR");
        exit(listenfd);
    }
    pthread_t tid;
    int i = 0;

    while (1)
    {
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (pthread_create(&tid, NULL, process, &connfd) != 0){
            printf("Failed to create thread\n");
        }
        // pthread_join(tid, NULL);
    }
    // pthread_join(tid, NULL);

    /*while (1)
    {
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (pthread_create(&tid, NULL, process, &connfd) != 0){
            printf("Failed to create thread\n");
        }
        //pthread_join(tid, NULL);
    }*/


    return 0;
}
