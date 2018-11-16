#pragma once
#include <fc/exception/exception.hpp>
#include <list>
#include <string>
#include <fc/reflect/reflect.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include "wsFileManager.hpp"

namespace ultrainio { namespace ws {
struct message_info{
    int type;  //0: wsinfo
    int size;
    int sliceId;
};

int lenOfSlice = 2048;

}}

FC_REFLECT(ultrainio::ws::message_info, (type)(size)(sliceId))