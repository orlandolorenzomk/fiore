#ifndef FTP_CLIENT_H
#define  FTP_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#define FTP_USER        "fiore-ftp-user"
#define FTP_PASSWORD    "password"
#define FTP_REMOTE_IP   "ftp://31.97.46.28"

bool ftp_list_dir(const char *remote_path);
bool ftp_download_file(const char *remote_path, const char *local_path);

#endif // FTP_CLIENT_H