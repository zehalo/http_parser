#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#include "http_parser.h"


char content_type_flag = 0;
char content_length_flag = 0;

#define BUFSIZZ 1024*8

#define IMAGE_TYPE_PNG "image/png"
#define IMAGE_TYPE_PNG_EXT ".png"
#define IMAGE_TYPE_JPEG "image/jpeg"
#define IMAGE_TYPE_JPEG_EXT ".jpeg"
#define IMAGE_TYPE_GIF "image/gif"
#define IMAGE_TYPE_GIF_EXT ".gif"
#define IMAGE_TYPE_BMP "image/bmp"
#define IMAGE_TYPE_BMP_EXT ".bmp"

#define IMAGE_TYPE_DEFAULT_EXT ".html"

struct http_content{
	char type[32];
	unsigned int response_size;
	unsigned int content_size;
	char *content_start;
	char header_checked;
	char mem_realloced;
}http_img ;

static http_parser *parser;

int on_message_begin(http_parser* _)
{
	(void)_;
	printf("\n***MESSAGE BEGIN***\n\n");
	return 0;
}

int on_headers_complete(http_parser* _)
{
	(void)_;
	printf("\n***HEADERS COMPLETE***\n\n");
	return 0;
}

int on_message_complete(http_parser* _)
	{
	(void)_;
	printf("\n***MESSAGE COMPLETE***\n\n");
	return 0;
}

int on_url(http_parser* _, const char* at, size_t length)
{
	(void)_;
	printf("Url: %.*s\n", (int)length, at);
	return 0;
}
int on_header_field(http_parser* _, const char* at, size_t length)
{
	(void)_;
	printf("Header field: %.*s\n", (int)length, at);
	if(!memcmp("Content-Type", at, length))
	{
		//printf("Found Content-Type\n");
		content_type_flag = 1;
	}
	if(!memcmp("Content-Length", at, length))
	{
		//printf("Found Content-Length\n");
		content_length_flag = 1;
	}

	return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length)
{
	(void)_;
	printf("Header value: %.*s\n", (int)length, at);
	if(content_type_flag)
	{
		memcpy(http_img.type, at, length);
		//printf("http_img.type = %s\n", http_img.type);
		content_type_flag = 0;
	}

	if(content_length_flag)
	{
		char value[32];
		memcpy(value, at, length);
		//printf("http_img.content_size = %s\n", value);
		http_img.content_size = atoi(value);
		content_length_flag = 0;
	}
	return 0;
}

int on_body(http_parser* _, const char* at, size_t length)
{
	(void)_;
	unsigned int z;
	char *p = at;

	http_img.content_start=at;
//	if(!memcmp("image/png", http_img.type, strlen("image/png")))
//	{
//		printf("Found PNG body!http_img.content_start=%p\n", http_img.content_start);
//	}
	return 0;
}

static http_parser_settings settings_null =
{
	.on_message_begin = on_message_begin,
	.on_header_field = on_header_field,
	.on_header_value = on_header_value,
	.on_url = on_url,
	.on_status = 0,
	.on_body = on_body,
	.on_headers_complete = on_headers_complete,
	.on_message_complete = on_message_complete
};


static double tminterval(struct timeval tmstart)
{
	double ret = 0;
	struct timeval now;

	gettimeofday(&now, NULL);

	ret = ((now.tv_sec + now.tv_usec * 1e-6)
	- (tmstart.tv_sec + tmstart.tv_usec * 1e-6));
	tmstart = now;

	return ret;
}

int main (void)
{
	float start, end;
	size_t parsed;
	struct sockaddr_in address;
	int client_sock;
	int len,result;
	int n;
	char buffer[BUFSIZZ];
	char *host="admin.omsg.cn";
	short port = 80;
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	struct timeval tmstart;
	char *http_buf, *mbuf;
	unsigned int http_buf_len = 0;
	char check_http_header=0;
	char outfile[20] = "logo";
	char *file_ext;
	FILE *fp;
	int mbuf_len = 0;
	char *p;

	parser = malloc(sizeof(http_parser));
	memset(&http_img, 0, sizeof(http_img));
	http_buf = malloc(BUFSIZZ);
	memset(http_buf,0, sizeof(http_buf));
	mbuf = malloc(BUFSIZZ);
	memset(mbuf,0, sizeof(mbuf));


//Connect to server
	/*PNG*///char *buf = "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: admin.omsg.cn\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n";
	/*JPEG*/char *buf = "GET http://pic67.nipic.com/file/20150515/19533051_112209270000_2.jpg HTTP/1.1\r\nHost: pic67.nipic.com\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n";

	if ((he = gethostbyname(host)) == NULL) {  // get the host info
		printf("gethostbyname error\n");
		return 1;
	}
	printf("Official name is: %s\n", he->h_name);
	printf("	  IP addresses: ");
	addr_list = (struct in_addr **)he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) {
		printf("\t%s \n", inet_ntoa(addr_list[0]));
	}

	client_sock=socket(AF_INET,SOCK_STREAM,0);
	address.sin_addr.s_addr=inet_addr(inet_ntoa(*addr_list[0]));
	address.sin_family=AF_INET;
	address.sin_port=htons(port);
	len=sizeof(address);
	result=connect(client_sock,(struct sockaddr *)&address,len);
	if(result==-1){
		printf("error!\n");
		exit(-1);
	}

//Parse request
	http_parser_init(parser, HTTP_REQUEST);
	parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));

//Send request
	n=write(client_sock,buf, strlen(buf));
	if(n<0){
		printf("error write\n");
	}

//Receive response
	do{
		if((http_img.response_size>BUFSIZZ) && (!http_img.mem_realloced))
		{
		//Need more memory, realloc it.
			http_buf = realloc(http_buf,http_img.response_size);
			http_img.mem_realloced = 1;
			printf("realloc memory size to %d\n", http_img.response_size);
		}
		memcpy(http_buf+http_buf_len, mbuf, mbuf_len);
		http_buf_len += mbuf_len;
		mbuf_len=recv(client_sock,mbuf,sizeof(mbuf),0);
		if(!http_img.header_checked && (http_buf_len>1024))
		{
		//Parse http response
			http_parser_init(parser, HTTP_RESPONSE);
			parsed = http_parser_execute(parser, &settings_null, http_buf, strlen(http_buf));
			http_img.response_size = http_img.content_start - http_buf + http_img.content_size;
			printf("response_size=%d content_size=%d\n", http_img.response_size, http_img.content_size);
			http_img.header_checked =1;
		}

	}while(mbuf_len>0);

	//Reparse http response, in case realloc change the http_buf address.
		http_parser_init(parser, HTTP_RESPONSE);
		parsed = http_parser_execute(parser, &settings_null, http_buf, strlen(http_buf));

	//Checkout PNG body, and write to local file.
		p = strtok(http_img.type, ";");
		if(!strcmp(p, IMAGE_TYPE_PNG))
			file_ext = IMAGE_TYPE_PNG_EXT;
		else if(!strcmp(p, IMAGE_TYPE_JPEG))
			file_ext = IMAGE_TYPE_JPEG_EXT;
		else if(!strcmp(p, IMAGE_TYPE_GIF))
			file_ext = IMAGE_TYPE_GIF_EXT;
		else if(!strcmp(p, IMAGE_TYPE_BMP))
			file_ext = IMAGE_TYPE_BMP_EXT;
		else
			file_ext = IMAGE_TYPE_DEFAULT_EXT;
		strcat(outfile, file_ext);
		printf("Content-Type=%s\n", outfile);
		if((fp = fopen(outfile,"wra+"))==NULL)
		{
			printf("can't open abc.txt\n");
		}
		if(fp != NULL)
			if(fwrite(http_img.content_start,sizeof(char),http_img.content_size,fp)!=http_img.content_size)
				printf("can't write %s\n", outfile);
		if(fp != NULL)
			fclose(fp);


#if 0
	buf = "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: admin.omsg.cn\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n";

	start = (float)clock()/CLOCKS_PER_SEC;

	http_parser_init(parser, HTTP_REQUEST);
	parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));

	end = (float)clock()/CLOCKS_PER_SEC;


	buf="HTTP/1.1 200 OK\r\n"
	"Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
	"Server: Apache\r\n"
	"X-Powered-By: Servlet/2.5 JSP/2.1\r\n"
	"Content-Type: text/xml; charset=utf-8\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
	"  <SOAP-ENV:Body>\n"
	"    <SOAP-ENV:Fault>\n"
	"       <faultcode>SOAP-ENV:Client</faultcode>\n"
	"       <faultstring>Client Error</faultstring>\n"
	"    </SOAP-ENV:Fault>\n"
	"  </SOAP-ENV:Body>\n"
	"</SOAP-ENV:Envelope>";

	http_parser_init(parser, HTTP_RESPONSE);
	parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));
#endif

	//Free allocated buffer
		if(parser)
			free(parser);
		if(http_buf)
			free(http_buf);
		if(mbuf)
			free(mbuf);

	printf("Elapsed %f seconds.\n", (end - start));

	return (EXIT_SUCCESS);
}
