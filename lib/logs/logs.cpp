//
// Created by Michal on 06/04/16.
//
#include "constants.h"
#include "constants.h"
#include <ctime>
#include <fstream>
#include <string>
#include <boost/filesystem.hpp>

using namespace std;

string generateHumanTimestamp() {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    string year = to_string(now->tm_year + 1900);
    string month = to_string(now->tm_mon + 1);
    string day = to_string(now->tm_mday);

    return year+"-"+month+"-"+day;
}

