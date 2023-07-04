//#pragma once
#ifndef WIFI_IO_H
#define WIFI_IO_H
// C/C++ Headers

// Other Posix headers

// IDF he<der
#include "esp_err.h" //thư viện nhận diện lỗi
#include "hal/gpio_types.h" //thư viện in các tin nhắn debug

// Component header

// Public Headers

// Private Headers

void wifi_connect (char* name_wifi, char* pass_wifi,int time_out,wifi_mode_t mode,wifi_interface_t interface);

#endif