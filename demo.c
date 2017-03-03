#include "http_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

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

int
main (void)
{
  const char *buf;
  int i;
  float start, end;
  size_t parsed;

  parser = malloc(sizeof(http_parser));

  buf = "GET /cookies HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.56 Safari/537.17\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: en-US,en;q=0.8\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\nCookie: name=wookie\r\n\r\n";

  start = (float)clock()/CLOCKS_PER_SEC;
  for (i = 0; i < 1; i++) {
    http_parser_init(parser, HTTP_REQUEST);
    parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));
  }
  end = (float)clock()/CLOCKS_PER_SEC;

  free(parser);
  parser = NULL;

  printf("Elapsed %f seconds.\n", (end - start));

  return 0;
}
