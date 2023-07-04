/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "dht_iot.h"
#include "wifi_iot.h"
#include "http_sever_app.h"
#include "output_iot.h"
#include "ledc_app.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "80"

static const char *TAG = "WIFI_STA_HDT";
int hum = 10;
static float fhumidity;
static float ftemperature;
static float fhumidity22;
static float ftemperature22;
char REQUEST[512];
char RESPONSE[512];
char SUBREQUEST[100];
char recv_buf[512];
static char *name_wifi = "HADENDONGVAOLAMCHO";
static char *pass_wifi = "Captain11";

// void ledc(void* par)
// {
// for(;;)
// {
// set_duty_ledc(100,LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0);
// vTaskDelay(10/portTICK_PERIOD_MS);
// }
// }

void set_callback_dht11(void)
{
    sprintf(REQUEST, "{\"temperature\": %3.2f,\"humidity\": %3.2f,\"temperature1\": %3.2f,\"humidity1\": %3.2f}", ftemperature, fhumidity, ftemperature22, fhumidity22);
    const char *resp_str = (const char *)REQUEST;
    respoin_data_dht11(resp_str);
}
void set_callback_swich(char *data, int leng)
{
    if (*data == '1')
    {
        gpio_set_level(GPIO_NUM_2, 1);
    }
    else if (*data == '0')
    {
        gpio_set_level(GPIO_NUM_2, 0);
    }
}
// void set_callback_slider(char *data, int leng)
// {
// char number_str[10];
// memcpy(number_str, data, leng + 1);
// int duty = atoi(number_str);
// printf("data : %s\n", data);
// set_duty_ledc(duty, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
// }
void set_callback_slider1(char *data, int leng)
{
    char number_str[10];
    memcpy(number_str, data, leng + 1);
    int duty = atoi(number_str);
    printf("data : %s\n", data);
    set_duty_ledc(duty, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3);
}
int conver_hex(char *data)
{
    char dec[20];
    memcpy(dec, data, strlen(data));
    int sum = 0;
    for (int i = 0; i < 2; i++)
    {
        if (data[i] == 'A')
        {
            sum += 10 * pow(16, i);
        }
        else if (data[i] == 'B')
        {
            sum += 11 * pow(16, i);
        }
        else if (data[i] == 'C')
        {
            sum += 12 * pow(16, i);
        }
        else if (data[i] == 'D')
        {
            sum += 13 * pow(16, i);
        }
        else if (data[i] == 'E')
        {
            sum += 14 * pow(16, i);
        }
        else if (data[i] == 'F')
        {
            sum += 15 * pow(16, i);
        }
        else
        {
            sum += ((int)data[i] - 48) * pow(16, i);
        }
    }
    return sum;
}
void set_callback_color(char *data, int leng)
{
    char data_red[3] = {data[1], data[0]};
    char data_green[3] = {data[3], data[2]};
    char data_blue[3] = {data[5], data[4]};
    // printf("data1:%s\n",data_red);
    // printf("data1:%s\n",data_green);
    // printf("data1:%s\n",data_blue);
    int duty_red = conver_hex(data_red);
    int duty_green = conver_hex(data_green);
    int duty_blue = conver_hex(data_blue);
    set_duty_ledc(duty_red, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    set_duty_ledc(duty_blue, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    set_duty_ledc(duty_green, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
    printf("dec:%d\n", duty_red);
    printf("dec:%d\n", duty_green);
    printf("dec:%d\n", duty_blue);
}
static void datahandel(void *pvParameters)
{
    // for (;;)
    //{
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);
    //     dht_read_float_data(DHT_TYPE_DHT11, GPIO_NUM_4, &fhumidity, &ftemperature);
    //     fhumidity -= 10;
    //     ftemperature -=3;
    //     printf("humidity:%f\n", fhumidity);
    //    printf("temperature:%f\n\n", ftemperature);
    //}
    for (;;)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        dht_read_float_data(DHT_TYPE_AM2301, GPIO_NUM_15, &fhumidity22, &ftemperature22);
        dht_read_float_data(DHT_TYPE_DHT11, GPIO_NUM_5, &fhumidity, &ftemperature);
        fhumidity -= 10;
        ftemperature -= 3;
        fhumidity22 -= 10;
        ftemperature22 -= 3;
        // recive_data_dht(ftemperature, fhumidity, ftemperature22, fhumidity22);
        printf("humidity:%f\n", fhumidity);
        printf("temperature:%f\n", ftemperature);
        printf("humidity22:%f\n", fhumidity22);
        printf("temperature22:%f\n\n", ftemperature22);
    }
}

static void http_get_task(void *pvParameters)
{
    // cấu trúc chứa các giá trị đầu vào đặt socktype và giao thức;
    const struct addrinfo hints = {
        .ai_family = AF_INET,       // họ địa chỉ của socket
        .ai_socktype = SOCK_STREAM, // loại socket TCP
    };
    struct addrinfo *res; // con trỏ tới một con trỏ nơi lưu trữ kết quả (được đặt thành NULL khi không thành công)
    struct in_addr *addr;
    int s, r;

    while (1)
    {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res); // khai báo địa link và port trả về 0 là thành công

        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;         // tạo 1 con trỏ để lấy dữ liệu địa chỉ
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr)); // inet_ntoa hàm trả về địa chỉ ip của socket

        s = socket(res->ai_family, res->ai_socktype, 0); // khởi tạo socket
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res); // giải phóng cấu trúc thông tin
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0) // kết nối socket vao mạng
        {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        // sprintf(SUBREQUEST, "api_key=MSHYVU6IDID4EC8O&field1=%d&field2=%d", 50, 50);
        // sprintf(REQUEST, "POST /update.json HTTP/1.1\nHost:api.thingspeak.com\nConnection: close\nContent-Type: application/x-www-form-urlencoded\nContent-Length:%d\n\n%s\n", strlen(SUBREQUEST), SUBREQUEST);
        sprintf(REQUEST, "GET http://api.thingspeak.com/update.json?api_key=MSHYVU6IDID4EC8O&field1=%.2f&field2=%.2f\n\n", ftemperature, fhumidity);
        // sprintf(REQUEST, "GET http://api.thingspeak.com/update.json?api_key=MSHYVU6IDID4EC8O&field1=%f&field2=%f\n\n",ftemperature,fhumidity);
        // sprintf(REQUEST, "GET https://api.thingspeak.com/channels/2201180/feeds.json?api_key=HZ56AF540YSX0TMV&results=2\n\n");

        if (write(s, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout; // struct time
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, // set timeout,soc_socket mức độ ưu tiên socket
                       sizeof(receiving_timeout)) < 0)
        {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            for (int i = 0; i < r; i++)
            {
                putchar(recv_buf[i]);
            }
        } while (r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        for (int countdown = 10; countdown >= 0; countdown--)
        {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // ESP_ERROR_CHECK(example_connect());
    wifi_connect(name_wifi, pass_wifi, 3, WIFI_MODE_STA, ESP_IF_WIFI_STA);
    start_webserver();
    output_io_creat(GPIO_NUM_2);
    http_set_callback_swich(&set_callback_swich);
    http_set_callback_dht11(&set_callback_dht11);
    // http_set_callback_slider(&set_callback_slider);
    http_set_callback_slider1(&set_callback_slider1);
    http_set_callback_color(&set_callback_color);
    ledc_init(LEDC_TIMER_13_BIT, 5000, LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1);
    add_pin_ledc(LEDC_CHANNEL_0, GPIO_NUM_33, 0, LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1);
    add_pin_ledc(LEDC_CHANNEL_1, GPIO_NUM_32, 0, LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1);
    add_pin_ledc(LEDC_CHANNEL_2, GPIO_NUM_19, 0, LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1);
    add_pin_ledc(LEDC_CHANNEL_3, GPIO_NUM_18, 0, LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1);
    // set_duty_ledc(100,LEDC_HIGH_SPEED_MODE,LEDC_TIMER_1);
    xTaskCreate(http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    xTaskCreate(datahandel, "datahandel", 2048, NULL, 4, NULL);
    // xTaskCreate(ledc, "ledchandel", 2048, NULL, 4, NULL);
}