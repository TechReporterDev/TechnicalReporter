#include "stdafx.h"
#include "Counters.h"

int Arg::copy_counter = 0;
int Arg::move_counter = 0;
int Result::copy_counter = 0;
int Result::move_counter = 0;

void ResetCounters()
{
    Arg::copy_counter = 0;
    Arg::move_counter = 0;
    Result::copy_counter = 0;
    Result::move_counter = 0;
}