#include "http_routes.h"

const struct http_route http_routes[] = {
  { "GET",  "",                http_dist_index_handler, NULL },
  { "GET",  "dist/",           http_dist_handler,       NULL },
  { "GET",  "config.ini",      config_get_handler,      NULL },
  { "POST", "config.ini",      config_post_handler,     NULL },

  { "GET",  "api/config",         config_api_get,  NULL },
  { "POST", "api/config",         config_api_post, NULL },

  { "GET",  "api/system",         system_api_handler,         NULL },
  { "GET",  "api/system/tasks",   system_api_tasks_handler,   NULL },
  { "POST", "api/system/restart", system_api_restart_handler, NULL },

  {}
};
