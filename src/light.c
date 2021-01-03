#include "light.h"

light_t light = {
    .direction = { 0, 0, 1 }
};

uint32_t light_apply_intensity(uint32_t original_color, float factor)
{
  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;
  uint32_t a = (original_color & 0xFF000000);
  uint32_t r = (original_color & 0x00FF0000) * factor;
  uint32_t g = (original_color & 0x0000FF00) * factor;
  uint32_t b = (original_color & 0x000000FF) * factor;
  uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
  return new_color;
}
