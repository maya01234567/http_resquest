#include "http_sever_app.h"
/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"

#include <esp_http_server.h>

get_dht11_callback_t get_dht11_callback = NULL;
get_color_callback_t get_color_callback = NULL;
post_swich_callback_t post_swich_callback = NULL;
post_slider_callback_t post_slider_callback = NULL;
post_slider1_callback_t post_slider1_callback = NULL;
/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static httpd_req_t *REG;
static const char *TAG = "HTTP_SEVER";
static httpd_handle_t server = NULL;
// static float ftemperature, fhumidity;
// static float ftemperature1, fhumidity1;
// static char REQUEST[512];
// extern const uint8_t index_html_start[] asm("_binary_test1_jpg_start");
// extern const uint8_t index_html_end[] asm("_binary_test1_jpg_end");
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char *resp_str = (const char *)"Hello World"; // req->user_ctx;
    // httpd_resp_send(req, resp_str, strlen(resp_str));
    // httpd_resp_set_type(req, "image/jpg");
    // httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_data_dht11 = {
    .uri = "/dht11",
    .method = HTTP_GET,
    .handler = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = NULL};

static esp_err_t data_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    printf("data: %s\n", buf);
    // end respont
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_data = {
    .uri = "/data",
    .method = HTTP_POST,
    .handler = data_post_handler,
    .user_ctx = NULL};

void respoin_data_dht11(const char *data)
{
    httpd_resp_send(REG, data, strlen(data));
}

static esp_err_t get_data_dht21_handler(httpd_req_t *req)
{
    REG = req;
    get_dht11_callback();
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t get_data_dht21 = {
    .uri = "/getdatadht11",
    .method = HTTP_GET,
    .handler = get_data_dht21_handler,
    .user_ctx = NULL};

static esp_err_t swich_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    printf("data: %s\n", buf);
    post_swich_callback(buf, req->content_len);
    // end respont
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t swich_post_data = {
    .uri = "/swich1",
    .method = HTTP_POST,
    .handler = swich_post_handler,
    .user_ctx = NULL};

static esp_err_t slider_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    printf("data: %s\n", buf);
    post_slider_callback(buf, req->content_len);
    // end respont
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t slider_post_data = {
    .uri = "/slider",
    .method = HTTP_POST,
    .handler = slider_post_handler,
    .user_ctx = NULL};

static esp_err_t slider1_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    printf("data: %s\n", buf);
    post_slider1_callback(buf, req->content_len);
    // end respont
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t slider1_post_data = {
    .uri = "/slider1",
    .method = HTTP_POST,
    .handler = slider1_post_handler,
    .user_ctx = NULL};

static esp_err_t get_pickker_color_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            printf("Found URL query => %s\n", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "color", param, sizeof(param)) == ESP_OK)
            {
                printf("Found URL query parameter => color=%s\n", param);
                printf("Found URL query parameter => color=%d\n", strlen(param));
                get_color_callback(param,strlen(param));
            }
        }
        free(buf);
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t get_pickker_color = {
    .uri = "/rgb",
    .method = HTTP_GET,
    .handler = get_pickker_color_handler,
    .user_ctx = NULL};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/dht11", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/dht11 URI is not available");
        return ESP_OK;
    }
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &post_data);
        httpd_register_uri_handler(server, &get_data_dht11);
        httpd_register_uri_handler(server, &get_data_dht21);
        httpd_register_uri_handler(server, &swich_post_data);
        httpd_register_uri_handler(server, &slider_post_data);
        httpd_register_uri_handler(server, &slider1_post_data);
        httpd_register_uri_handler(server, &get_pickker_color);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void)
{
    httpd_stop(server);
}

// void recive_data_dht(float pftemperature, float pfhumidity, float pftemperature1, float pfhumidity1)
// {
// ftemperature = pftemperature;
// fhumidity = pfhumidity;
// ftemperature1 = pftemperature1;
// fhumidity1 = pfhumidity1;
// }
void http_set_callback_swich(void *cb)
{
    post_swich_callback = cb; // gán địa chỉ của hàm cho con trỏ input_callback
}
void http_set_callback_dht11(void *cb)
{
    get_dht11_callback = cb; // gán địa chỉ của hàm cho con trỏ input_callback
}
void http_set_callback_slider(void *cb)
{
    post_slider_callback = cb; // gán địa chỉ của hàm cho con trỏ input_callback
}
void http_set_callback_slider1(void *cb)
{
    post_slider1_callback = cb; // gán địa chỉ của hàm cho con trỏ input_callback
}
void http_set_callback_color(void *cb)
{
    get_color_callback = cb; // gán địa chỉ của hàm cho con trỏ input_callback
}
