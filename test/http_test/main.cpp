#include "HttpRequest.h"
#include <string.h>
#include <iostream>


int main(void)
{
    HttpRequest* Http;
    char http_return[4096] = {0};
    char http_msg[4096] = {0};
    strcpy(http_msg, "http://127.0.0.1:2375/containers/bestwish/top");

    if(Http->HttpGet(http_msg, http_return)){
        std::cout << http_return << std::endl;
    }
    return 0;
}