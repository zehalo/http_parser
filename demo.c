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

static http_parser *parser;

int on_message_begin(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE BEGIN***\n\n");
  return 0;
}

int on_headers_complete(http_parser* _) {
  (void)_;
  printf("\n***HEADERS COMPLETE***\n\n");
  return 0;
}

int on_message_complete(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE COMPLETE***\n\n");
  return 0;
}

int on_url(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Url: %.*s\n", (int)length, at);
  return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header field: %.*s\n", (int)length, at);
  return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header value: %.*s\n", (int)length, at);
  return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Body: %.*s\n", (int)length, at);
  return 0;
}

static http_parser_settings settings_null =
  {.on_message_begin = on_message_begin
  ,.on_header_field = on_header_field
  ,.on_header_value = on_header_value
  ,.on_url = on_url
  ,.on_status = 0
  ,.on_body = on_body
  ,.on_headers_complete = on_headers_complete
  ,.on_message_complete = on_message_complete
  };

#define BUFSIZZ 1024*8

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
	const char *buf;
	float start, end;
	size_t parsed;
	struct sockaddr_in address;
	int client_sock;
	int len,result;
	int n,tmp;
	char buffer[BUFSIZZ];
	char *host="proxy.zte.com.cn";
	short port = 80;
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	struct timeval tmstart;

	#define REQUEST_STR "GET http://admin.omsg.cn/inuploadpic/2016121034000012.png HTTP/1.1\r\nHost: admin.omsg.cn\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n"

	if ((he = gethostbyname(host)) == NULL) {  // get the host info
		printf("gethostbyname error\n");
		return 1;
	}
	printf("Official name is: %s\n", he->h_name);
	printf("	  IP addresses: ");
	addr_list = (struct in_addr **)he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) {
		printf("\t%s \n", inet_ntoa(*addr_list[i]));
	}

	client_sock=socket(AF_INET,SOCK_STREAM,0);
	address.sin_addr.s_addr=inet_addr(inet_ntoa(*addr_list[0]));
	address.sin_family=AF_INET;
	address.sin_port=htons(port);
	len=sizeof(address);
	printf("@@@@@ line=%d time consumption=%lf\n", __LINE__, tminterval(tmstart));
	result=connect(client_sock,(struct sockaddr *)&address,len);
	if(result==-1){
		printf("error!");
		exit(-1);
	}
	printf("@@@@@ line=%d time consumption=%lf\n", __LINE__, tminterval(tmstart));
	n=write(client_sock,REQUEST_STR, strlen(REQUEST_STR));
	if(n<0){
		printf("error write/n");
	}
	memset(buffer,0, sizeof(buffer));
	printf("@@@@@ line=%d time consumption=%lf\n", __LINE__, tminterval(tmstart));
	while(tmp=recv(client_sock,buffer,sizeof(buffer),0)){
	if(tmp==-1) break;
		printf("@@@@@ line=%d mbuf_len=%d time consumption=%lf, buffer=%s\n", __LINE__, tmp, tminterval(tmstart), buffer);
	}

	parser = malloc(sizeof(http_parser));

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

	free(parser);
	parser = NULL;

	printf("Elapsed %f seconds.\n", (end - start));

	return (EXIT_SUCCESS);
}
