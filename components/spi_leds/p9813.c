#include "p9813.h"

#include <logging.h>

#include <stdlib.h>

#define P9813_START_FRAME (struct p9813_frame){ 0x00, 0x00, 0x00, 0x00 }
#define P9813_STOP_FRAME (struct p9813_frame){ 0x00, 0x00, 0x00, 0x00 }

#define P9813_CONTROL_BYTE(b, g, r) (0xC0 | ((~(b) & 0xC0) >> 2) | ((~(g) & 0xC0) >> 4) | ((~(r) & 0xC0) >> 6))

int p9813_new_packet(struct p9813_packet **packetp, size_t *sizep, unsigned count)
{
  unsigned stopframes = 1; // single 32-bit frame3
  size_t size = (1 + count + stopframes) * sizeof(struct p9813_frame);
  struct p9813_packet *packet;

  if (!(packet = malloc(size))) {
    LOG_ERROR("malloc");
    return -1;
  }

  *packetp = packet;
  *sizep = size;

  // frames
  packet->start = P9813_START_FRAME;

  for (unsigned i = 0; i < count; i++) {
    packet->frames[i] = (struct p9813_frame){ P9813_CONTROL_BYTE(0, 0, 0), 0, 0, 0 }; // off
  }

  for (unsigned i = count; i < count + stopframes; i++) {
    packet->frames[i] = P9813_STOP_FRAME;
  }

  return 0;
}

void p9813_set_frame(struct p9813_packet *packet, unsigned index, struct spi_led_color color)
{
  packet->frames[index] = (struct p9813_frame) {
    .control = P9813_CONTROL_BYTE(color.b, color.g, color.r),
    .b = color.b,
    .g = color.g,
    .r = color.r,
  };
}

void p9813_set_frames(struct p9813_packet *packet, unsigned count, struct spi_led_color color)
{
  for (unsigned index = 0; index < count; index++) {
    packet->frames[index] = (struct p9813_frame) {
      .control = P9813_CONTROL_BYTE(color.b, color.g, color.r),
      .b = color.b,
      .g = color.g,
      .r = color.r,
    };
  }
}

static inline bool p9813_frame_active(const struct p9813_frame frame)
{
  return frame.b || frame.g || frame.r;
}

unsigned p9813_count_active(struct p9813_packet *packet, unsigned count)
{
  unsigned active = 0;

  for (unsigned index = 0; index < count; index++) {
    if (p9813_frame_active(packet->frames[index])) {
      active++;
    }
  }

  return active;
}
