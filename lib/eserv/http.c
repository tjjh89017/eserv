#include "cgi.h"
#include "misc.h"
#include "http.h"
#include "hash.h"
#include "session.h"
#include "tpool.h"

static int ex_hashcasecmp_str(const char *s1, const char *s2)
{
	return (0 == strcasecmp(s1, s2));
}

void ex_sleep(size_t ms)
{
	usleep(ms * 1000);
}

int ex_uninit()
{
	ExContext.quitFlag = 1;
	while (ExContext.threadCnt > 0)
		ex_sleep(EX_SOCK_RTT);

	cgi_uninit();

	ex_hash_clear(&ExContext.mimeMap);
	ex_hash_clear(&ExContext.pageMap);
	printf("eServ terminated.\n");
	return 0;
}

extern void requestHandler(void * s);

static void do_request(struct bufferevent *bufev, void *arg)
{
	DBG("do_request");
	if(ex_tpool_add((ex_tpool*)arg, requestHandler, bufev)){
		perror("request error");
	}
}

static void do_end(struct bufferevent *bufev, void *arg)
{

}

static void do_event(struct bufferevent *bufev, short event, void *arg)
{
	if(event & BEV_EVENT_READING)
		DBG("BEV_EVENT_READING");
	if(event & BEV_EVENT_WRITING)
		DBG("BEV_EVENT_WRITING");
	if(event & BEV_EVENT_TIMEOUT)
		DBG("BEV_EVENT_TIMEOUT");
	if(event & BEV_EVENT_ERROR)
		DBG("BEV_EVENT_ERROR");
	if(event & BEV_EVENT_EOF)
		DBG("BEV_EVENT_EOF");
	if(event & BEV_EVENT_CONNECTED)
		DBG("BEV_EVENT_CONNECTED");

	//bufferevent_free(bufev);
}

static void do_accept(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int event_code, void* arg)
{
	struct event_base *base = evconnlistener_get_base(listener);
	DBG("start create bufferevent");
	struct bufferevent *bufev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    bufferevent_setcb(bufev, do_request, do_end, do_event, arg);
    bufferevent_enable(bufev, EV_READ | EV_WRITE);
}

void handleSigint(evutil_socket_t fd, short event_code, void* arg)
{
	evconnlistener_free((struct evconnlistener*) arg);
	//shutDown(0);
	exit(0);
}

static int ex_http_start()
{
	struct event_base *base;
	SOCKET ser_fd, cli_fd;  /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in ser_addr, cli_addr; /* connector's address information */
	int opt, sin_size;
	struct evconnlistener *listener;
	ex_tpool *tpool = NULL;

	/* Setup the default values */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = EX_SOCK_RTT * 1000;

	/*
	* Setup the sockets and wait and process connections
	*/
#if 0
	if ((ser_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
#endif
	/* Let the kernel reuse the socket address. This lets us run
	   twice in a row without waiting for the (ip, port) tuple
	   to time out. */
#if 0
	opt = 1;
	setsockopt(ser_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
	setsockopt(ser_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
#endif
	ser_addr.sin_family = AF_INET;		/* host byte order */
	ser_addr.sin_port = htons(PORT);	/* short, network byte order */
	ser_addr.sin_addr.s_addr = INADDR_ANY;	/* auto-fill with my IP */
#if 0
	if (bind(ser_fd, (struct sockaddr *) &ser_addr,
	         sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(ser_fd, EX_SOCK_BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
#endif
	if (chdir(RootPath) != 0) {
		perror("chdir");
		exit(1);
	}

	/*evutil_make_listen_socket_reuseable(ser_fd);
	evutil_make_socket_nonblocking(ser_fd);*/
	evthread_use_pthreads();

	base = event_base_new();

	if((tpool = ex_tpool_new(EX_MAX_THREADS, EX_MAX_QUEUE, 1)) == NULL){
		perror("tpool new fail");
		exit(1);
	}

	if(!(listener = evconnlistener_new_bind(base, do_accept, tpool, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, LISTEN_BACKLOG, (struct sockaddr*)&ser_addr, sizeof(ser_addr)))){
		perror("listener fail");
		exit(1);
	}
	/* evconnlistener_set_error_cb(listener, NULL); */



	struct event *listen_event;
	listen_event = evsignal_new(base, SIGINT, handleSigint, listener);
	evsignal_add(listen_event, NULL);

	DBG("\n " SERVER " is running.  Port: %d\n", PORT);

	event_base_dispatch(base);
	event_del(listen_event);
#if 0
	sin_size = sizeof(struct sockaddr_in);
	while (1) {  /* main accept() loop */
		if (ExContext.quitFlag == 1)
			break;
		if ((cli_fd = accept(ser_fd,
		                     (struct sockaddr *) & cli_addr,
				     (socklen_t *) &sin_size)) == -1)
			continue;
		while (1) {
			if (ExContext.threadCnt < EX_MAX_THREADS) {
				start_thread((void *) requestHandler,
				             (void *) &cli_fd);
				break;
			} else
				ex_sleep(50);
		}
	}
#endif

	return 0;
}

const char *get_mime_type(const char *path)
{
	const char *extension;
	const char *ret = NULL;

	if (path != NULL &&
	    (extension = strrchr(path, '.')) != NULL) {
		ret = ex_hash_find(&ExContext.mimeMap , ++extension);
	}
	return (ret != NULL) ? ret : ("text/plain");
}

void cgi_page_add(const char * pageName, void *f)
{
	ex_hash_add(&ExContext.pageMap, pageName, f);
}

void* cgi_page_find(const char *pageName)
{
	return ex_hash_find(&ExContext.pageMap , pageName);
}

int ex_init()
{
	int i = -1;
	static char pool[2048];
	static ex_mpool mpool;

	ex_mpool_init(&mpool, pool, sizeof(pool));
	ex_hash_init(&ExContext.mimeMap, &mpool, 97);
	ex_hash_init(&ExContext.pageMap, &mpool, 97);
	ExContext.pageMap.hashcmp = (void *) ex_hashcasecmp_str;
	ExContext.mimeMap.hashcmp = (void *) ex_hashcasecmp_str;
	/* add MIME type map */
	while (mmt[++i].ext != NULL) {
		ex_hash_add(&ExContext.mimeMap, mmt[i].ext, mmt[i].type);
	}

	ExContext.quitFlag = 0;
	ExContext.threadCnt = 0;

	cgi_init();
	ex_session_init();
	return start_thread((void *) ex_http_start, NULL);
}

