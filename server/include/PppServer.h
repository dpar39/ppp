//
// Created by Darien Pardinas Diaz on 9/25/18.
//

#include "HttpServer.h"

#include "libppp.h"
#include "MultiPartFormParser.h"

class PppServer : public HttpServer
{

    PublicPppEngine _pppEngine;

    void init();
};