#include "user_event.h"
#include "user_led.h"

#include <logging.h>

static const char *user_event_str(enum user_event event)
{
  switch (event) {
    case USER_EVENT_BOOT:         return "BOOT";
    case USER_EVENT_CONNECTING:   return "CONNECTING";
    case USER_EVENT_CONNECTED:    return "CONNECTED";
    case USER_EVENT_DISCONNECTED: return "DISCONNECTED";
    default:                      return "?";
  }
}

void user_event(enum user_event event)
{
  LOG_INFO("%s", user_event_str(event));

  user_led_event(event);
}
