#include "csapp.h"
#define MAX_CACHE 10
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define WEB_PREFIX "http://"
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void handlerequest(int);
void Clienterror(int , char* , char* , char* , char* );
int MakeClientRequest(rio_t* , char* , char*, char* , char* , char* , char*, char*);
int checkGetMethod(char* , char* , char* );
void replaceHTTPVersion(char* );
void parseLine(char* , char*, char* , char* , char* , char*, char*);
void *thread(void *v);
void rwlock_init();
int readcache(int fd,char *key);
void writecache(char *buf,char *key);
struct rwlock_t{
    sem_t lock;
    sem_t writelock;
    int readers;
};

struct Cache{
    int used;
    char key[MAXLINE];
    char value[MAX_OBJECT_SIZE];
};

int nowpointer;
struct Cache cache[MAXLINE];
struct rwlock_t *rw;

int main(int argc,char **argv)
{
    rw= Malloc(sizeof(struct rwlock_t));
    pthread_t tid;
    int listenfd,connfd;
    rwlock_init();
    char hostname[MAXLINE],port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    //判断是否是两个命令
    if(argc!=2){
        fprintf(stderr,"usage: Fuck ,can you input only two string ?\nsuch as%s <port>\n", argv[0]);
       // printf("Fuck ,input right message \n");
        exit(1);
    }

    listenfd =Open_listenfd(argv[1]);   //listen a port
    while(1){
        clientlen =sizeof (clientaddr);
        connfd = Accept(listenfd ,(SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr ,clientlen,hostname ,MAXLINE,port,MAXLINE,0);
        printf("Accepted connection from (%s , %s)\n", hostname ,port);// accept了

        //Connection Succeed
       Pthread_create(&tid,NULL,(void*) thread,(void*)&connfd);
    }
    return 0;
}
void rwlock_init(){
    rw->readers=0;
    sem_init(&rw->lock,0,1);
    /*
    sem_init 是一个用于初始化信号量的函数。这里初始化了 rw->lock 信号量。
    0 表示信号量用于线程间同步（而非进程间同步）。
    1 表示信号量的初始值为 1，即该信号量是一个二值信号量，用于互斥访问。
    */
    sem_init(&rw->writelock,0,1);
}
void *thread(void *v){
    int fd=*(int*)v;
    Pthread_detach(pthread_self());
    handlerequest(fd);
   // Free(v);
    Close(fd);
    return ;
}
void handlerequest(int fd){
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];// buf 
    char filename[MAXLINE];
    //request header
    char host[MAXLINE],port[MAXLINE];
    char clientRequest[MAXLINE];
    // IO for proxy--client ,proxy--server
    rio_t rio,riotiny;
    // Read request line and headers

    // step1: read request from client
    Rio_readinitb(&rio,fd);

    if(!Rio_readlineb(&rio ,buf, MAXLINE))//把输入的命令给到buf缓冲区
    {
        printf("empty request \n");
        return ;// empty-> close
    }
    // # HTTP/1.1 --> HTTp/1.0
    replaceHTTPVersion(buf);
   parseLine(buf,host,port,method,uri,version,filename);
    if(strcasecmp(method, "GET")){
        Clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return ;
    }
    // parse uri from GET request 
    if (readcache(fd,uri)!=0)return ;

    int rv=MakeClientRequest(&rio, clientRequest,host,port,method,uri,version,filename);
    if(rv==0)return ;

    printf("========= we have formatted the reqeust into ---------\n");
    printf("%s",clientRequest);

    char hostName[100];
    char* colon = strstr(host, ":");
    strncpy(hostName, host, colon - host);
    printf("host is %s\n", hostName);
    printf("port is %s\n", port);
    //模拟一个clientfd
    int clientfd = Open_clientfd(hostName, port);

    Rio_readinitb(&riotiny, clientfd);
    Rio_writen(riotiny.rio_fd, clientRequest, strlen(clientRequest));

    /** step4: read the response from tiny and send it to the client */
    char cache[MAX_OBJECT_SIZE];
    int sum=0;

    printf("---prepare to get the response---- \n");
    char tinyResponse[MAXLINE];

    int n;
    
    while( (n = Rio_readlineb(&riotiny, tinyResponse, MAXLINE)) != 0){
        Rio_writen(fd, tinyResponse, n);
        sum+=n;
        strcat(cache,tinyResponse);

    }
    printf("proxy send %d bytes to client\n",sum);
    if(sum<MAX_OBJECT_SIZE)
     writecache(cache,uri);
    close(clientfd);
    return ;

    
}
void writecache(char * buf,char *key){
    sem_wait(&rw->writelock);
    int index;
    while(cache[nowpointer].used!=0){
        cache[nowpointer].used=0;
        nowpointer=(nowpointer+1)%MAX_CACHE;

    }
    index =nowpointer;
    cache[index].used=1;
    strcpy(cache[index].key,key);
    strcpy(cache[index].value,buf);
    sem_post(&rw->writelock);
    return;
}
int readcache(int fd,char *uri){
    sem_wait(&rw->lock);
    if(rw->readers==0)sem_wait(&rw->writelock);
    rw->readers++;
    sem_post(&rw->lock);
    int i;
    int flag=0;
    for(i=0;i<MAX_CACHE;i++){
        if(strcmp(uri,cache[i].key)==0){
            Rio_writen(fd,cache[i].value,strlen(cache[i].value));
        //    printf("proxy send %d bytes to client\n", strlen(cache[i].value));
            cache[i].used=1;
            flag=1;
            break;

        }
    }
    sem_wait(&rw->lock);
    rw->readers--;
    if(rw->readers==0)
    sem_post(&rw->writelock);
    sem_post(&rw->lock);
    return flag;

}
int MakeClientRequest(rio_t* rio, char* clientRequest, char* Host, char* port,
                        char* method, char* uri, char* version, char* fileName){
    int UserAgent = 0, Connection = 0, ProxyConnection = 0, HostInfo = 0;
    char buf[MAXLINE / 2];
    int n;

    /* 1. add GET HOSTNAME HTTP/1.0 to header && Host Info */
    sprintf(clientRequest, "GET %s HTTP/1.0\r\n", fileName);

    n = Rio_readlineb(rio, buf, MAXLINE);// n是读取的字节数
    printf("receive buf %s\n", buf);
    printf("n == %d\n", n);
    char* findp;
    while(strcmp("\r\n", buf) != 0 && n != 0){
        strcat(clientRequest, buf);
        printf("receive buf %s\n", buf);

        if( (findp = strstr(buf, "User-Agent:")) != NULL){
            UserAgent = 1;
        }else if( (findp = strstr(buf, "Proxy-Connection:")) != NULL){
            ProxyConnection = 1;
        }else if( (findp = strstr(buf, "Connection:")) != NULL){
            Connection = 1;
        }else if( (findp = strstr(buf, "Host:")) != NULL){
            HostInfo = 1;
        }

        n = Rio_readlineb(rio, buf, MAXLINE);
    }

    if(n == 0){
        return 0;
    }

    if(HostInfo == 0){
        sprintf(buf, "Host: %s\r\n", Host);
        strcat(clientRequest, buf);
    }

    /** append User-Agent */
    if(UserAgent == 0){
        strcat(clientRequest, user_agent_hdr);
    }
    
    /** append Connection */
    if(Connection == 0){
        sprintf(buf, "Connection: close\r\n");
        strcat(clientRequest, buf);
    }
    
    /** append Proxy-Connection */
    if(ProxyConnection == 0){
        sprintf(buf, "Proxy-Connection: close\r\n");
        strcat(clientRequest, buf);
    }

    /* add terminator for request */
    strcat(clientRequest, "\r\n");
    return 1;
}
void replaceHTTPVersion(char *buf){
    char *pos =NULL;
    if((pos=strstr(buf,"HTTP/1.1"))!=NULL){
        buf[pos-buf+strlen("HTTP/1.1")-1]='0';
    }
}

void parseLine(char* buf, char* host, char* port, char* method, char* uri, char* version, char* filename){
    sscanf(buf, "%s %s %s",method,uri,version);
    //method = "GET", uri = "http://localhost:15213/home.html", version = "HTTP1.0"
    char* hostp = strstr(uri, WEB_PREFIX) + strlen(WEB_PREFIX);
    char* slash = strstr(hostp, "/");
    char* colon = strstr(hostp, ":");
    //get host name
    strncpy(host, hostp, slash - hostp);  
    //get port number
    strncpy(port, colon + 1, slash - colon - 1);
    //get file name
    strcpy(filename, slash);
    /*
    host : localhost:15214
    filename: /home.html
    */

}
// html headers and response body
void Clienterror(int fd,char *cause,char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];
    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

