#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define TIME_BUF_SIZE 64

struct HTTPHeaderFiled {
	char *name;
	char *value;
	struct HTTPHeaderField *next;
};

struct HTTPRequest {
	int protocol_minor_version;
	char *method;
	char *path;
	struct HTTPHeaderFiled *header;
	char *body;
	long length;
};

struct FileInfo {
	char *path;
	long size;
	int ok;
};

static void log_exit(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfrpintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
	exit(1);
}

static void *xmalloc(size_t sz)
{
	void *p;
	p = malloc(sz);
	if (!p) log_exit("failed to allocate memory");
	return p;
}

static void trap_signal(int sig, sighandler_t handler)
{
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(sig, &act, NULL) < 0)
		log_exit("sigaction() failed: %s", strerr(errno));
}

static void signal_exit(int sig)
{
	log_exit("exit by signal %d", sig);
}

static void install_signal_handles(void)
{
	trap_signal(SIGPIPE, signal_exit);
}

static void service(FILE *in, FILE *out, char *docroot)
{
	struct HTTPRequest *req;

	req = read_request(in);
	respond_to(req, out, docroot);
	free_request(req);
}

static void free_request(struct HTTPRequest *req)
{
	struct HTTPHeaderField *h, *head;

	head = req->header;
	while (head) {
		h = head;
		head = head->next;
		free(h->name);
		free(h->value);
		free(h);
	}
	free(req->method);
	free(req->path);
	free(req->body);
	free(req);
}

static struct HTTPRequest *read_request(FILE *in)
{
	struct HTTPRequest *req;
	struct HTTPHeaderField *h;

	req = xmalloc(sizeof(struct HTTPRequest));
	read_request_line(req, in);
	req->header = NULL;
	while (h = read_header_field(in)){
		h->next = req->header;
		req->header = h;
	}
	req->length = content_length(req);
	if (req->length != 0) {
		if (req->length > MAX_REQUEST_BODY_LENGTH)
			log_exit("request body too long");
		req->body = xmalloc(req->length);
		if (fread(req->body, req->length, 1, in) < 1)
			log_exit("failed o read request body");
	} else {
		req->body = NULL;
	}
	return req;
}

static void read_request_line(struct HTTPRequest *req, FILE *in)
{
	char buf[LINE_BUF_SIZE];
	char *path;

	if (!fgets(buf, LINE_BUF_SIZE, in))
		log_exit("no request line");
	p = strchr(buf, ' ');
	if (!p) log_exit("parse error on request line (1): %s", buf);
	*p++ = '\0';
	req->method = xmalloc(p - buf);
	strcpy(req->method, buf);
	upcase(req->method);

	path = p;
	p = strchr(path, ' ');
	if (!p) log_exit("parse error on request line (2): %s", buf);
	*p++ = '\0';
	req->path = xmalloc(p - path);
	strcpy(req->path, path);

	if (strncasecmp(p, "HTTP/1.", strlen("HTTP/1.")) != 0)
		log_exit("parse error on request line (3): %s", buf);
	p += strlen("HTTP/1.");
	req->protocol_minor_version = atoi(p);
}

static struct HTTPHeaderField *read_header_field(FILE *in)
{
	struct HTTPHeaderFiled *h;
	char buf[LINE_BUF_SIZE];
	char *p;

	if (!fgets(buf, LINE_BUF_SIZE, in)){
		log_exit("failed to read request header filed: %s", strerror(errno));
	}
	if ((buf[0] == '\n') || (strcmp(buf, "\r\n") == 0))
		return NULL;
	p = strchr(buf, ':');
	if (!p) log_exit("parse error on request header field: %s", buf);
	*p++ = '\0';
	h = xmalloc(sizeof(struct HTTPHeaderField));
	h->name = xmalloc(p - buf);
	strcpy(h->name, buf);

	p += strspn(p, "\t");
	h->value = xmalloc(strlen(p) + 1);
	strcpy(h->value, p);

	return h;
}

static long content_length(struct HTTPRequest *req)
{
	char *val;
	long len;
	val = lookup_header_field_value(req, "Content-Length");
	if (!val) return 0;
	len = atol(val);
	if (len < 0) log_exit("negative Content-Length value");
	return len;
}

static char *lookup_header_field_value(struct HTTPRequest *req, char *name)
{
	struct HTTPHeaderField *h;

	for (h = req->header; h; h = h->next)
	{
		if (strcasecmp(h->name, name) == 0)
			return h->value;
	}
	return NULL;
}

static struct FileInfo * get_fileinfo(char *docroot, char *urlpath)
{
	struct FileInfo *info;
	struct stat st;

	info = xmalloc(sizeof(struct FileInfo));
	info->path = build_fspath(docroot, urlpath);
	info->ok = 0;
	if (lstat(info->path, &st) < 0) return info;
	is (!S_ISREG(st.st_mode)) return info;
	info->ok = 1;
	info->size = st.st_size;
	return info;
}

static char *build_fspath(char *docroot, char *urlpath)
{
	char *path;
	
	path = xmalloc(strlen(docroot) + 1 + strlen(urlpath) + 1);
	sprintf(path, "%s/%s", docroot, urlpath);
	return path;
}

static void respond_to(struct HTTPRequest *req, FILE *out, char *docroot)
{
	if (strcmp(req->method, "GET") == 0)
		do_file_response(req, out, docroot);
	else if (strcmp(req->method, "HEAD") == 0)
		do_file_response(req, out, docroot);
	else if (strcmp(req->method, "POST") == 0)
		method_not_allowed(req,out);
	else
		not_implemented(req, out);
}

static void do_file_response(struct HTTPRequest *req, FILE *out, char *docroot)
{
	struct FileInfo *info;

	info = get_fileinfo(docroot, req->path);
	if (!info->ok) {
		free_fileinfo(info);
		nor_found(req, out);
		return ;
	}
	output_common_header_fields(req, out, "200 OK");
	fprintf(out, "Content-Length: %d\r\n", info->size);
	fprintf(out, "Content-Type: %s\r\n", guess_content_type(info));
	fprintf(out, "\r\n");
	if (strcmp(req->method, "HEAD") != 0) {
		int fd;
		char buf[BLOCK_BUF_SIZE];
		ssize_t n;

		fd = open(info->path, O_RDONLY);
		if (fd < 0)
			log_exit("failed to open %s: %s", info->path, strerror(errno));
		for(;;)
		{
			n = read(fd, buf, BLOCK_BUF_SIZE);
			if (n < 0)
				log_exit("failed to read %s: %s", info->path, strerror(errno));
			if (n == 0)
				break ;
			if (fwrite(buf, n, 1, out) < n)
				log_exit("failed to write to socket: %s", strerror(errno));
		}
		close(fd);
	}
	fflush(out);
	free_fileinfo(info);
}

static void output_common_header_fields(struct HTTPRequest *req, FILE *out, char *status)
{
	time_t t;
	struct tm *tm;
	char buf[TIME_BUF_SIZE];

	t = time(NULL);
	tm = gmtime(&t);
	if (!tm) log_exit("gmtime() failed: %s", strerror(errno));
	strftime(buf, TIME_BUF_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tm);
	fprintf(out, "HTTP/1.%d %s\r\n", HTTP_MINOR_VERSION, status);
	fprintf(out, "Date: %s\r\n", buf);
	fprintf(out, "Server: %s/%s\r\n", SERVER_NAME, SERVER_VERSION);
	fprintf(out, "Connection: close\r\n");
}

int main(int argc, char *argv[])
{
	if (argc != 2){
		fprintf(stderr, "Usage: %s <docroot>\n", argv[0]);
		exit(1);
	}
	install_signal_handlers();
	service(stdin, stdout, argv[1]);
	exit(0);
}

