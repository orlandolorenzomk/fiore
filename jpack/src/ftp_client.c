#include <ftp_client.h>
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

static size_t write_to_file(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t write_to_memory(void *ptr, size_t size, size_t nmemb, void *userdata);

bool ftp_list_dir(const char *remote_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing curl\n");
        return false;
    }

    // FTP_REMOTE_IP + "/" + remote_path
    char url[512];
    snprintf(url, sizeof(url), "%s/java/", FTP_REMOTE_IP);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, FTP_USER);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, FTP_PASSWORD);
    curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "FTP list error: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

static size_t write_to_memory(void *ptr, size_t size, size_t nmemb, void *userdata) {
    return fwrite(ptr, size, nmemb, (FILE *)userdata);  // write directly to stdout
}

bool ftp_download_file(const char *remote_path, const char *local_path) {
    CURL *curl;
    CURLcode res;
    char url[256];
    FILE *fp = fopen(local_path, "wb");
    if(!fp) return -1;

    snprintf(url, sizeof(url), "%s/%s", FTP_REMOTE_IP, remote_path);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "(ftp download) Error while initializing curl\n");
        fclose(fp);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, FTP_USER);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, FTP_PASSWORD);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0L);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    fclose(fp);

    return (res == CURLE_OK) ? true : false;
}


static size_t write_to_file(void *ptr, size_t size, size_t nmemb, void *stream) {
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}