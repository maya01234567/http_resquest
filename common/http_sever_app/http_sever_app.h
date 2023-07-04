#ifndef __HTTP_SEVER_APP_H
#define __HTTP_SEVER_APP_H

typedef void (*get_dht11_callback_t) (); //khai báo con trỏ hàm funtion pointer
typedef void (*post_swich_callback_t) (char* data,int leng);
typedef void (*post_slider_callback_t) (char* data,int leng);
typedef void (*post_slider1_callback_t) (char* data,int leng);
typedef void (*get_color_callback_t) (char* data,int leng);

void stop_webserver(void);
void start_webserver(void);
//void recive_data_dht(float pftemperature, float pfhumidity, float pftemperature1, float pfhumidity1);
void http_set_callback_swich(void *cb);
void http_set_callback_dht11(void *cb);
void http_set_callback_slider(void *cb);
void http_set_callback_slider1(void *cb);
void http_set_callback_color(void *cb);
void respoin_data_dht11(const char* data);
#endif