//структуры даты и пакета данных
#pragma once

#pragma pack(push,1)
struct data{
uint8_t year:7;
uint8_t month:4;
uint8_t day:5;
uint8_t hour:5;
uint8_t minute:6;
uint8_t second:6;
};

struct packet
{
    uint8_t comm;
    uint64_t message:40;
    
};
#pragma pack(pop)