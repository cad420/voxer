#include "ospray/ospray.h"
#include <errno.h>

#define TJE_IMPLEMENTATION
#include "third_party/tiny_jpeg.h"

void writeJPEG(const char *fileName, const int &width, const int &height,
               const uint32_t *pixel) {
  FILE *file = fopen(fileName, "wb");
  if (file == nullptr) {
    fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
    return;
  }

  if (!tje_encode_to_file_at_quality(fileName, 3, width, height, 4,
                                     (const unsigned char *)pixel)) {
    fprintf(stderr, "Could not write JPEG\n");
  }

  fclose(file);
}

int main() {
  // load datasets

  // waiting for connect

  // connected

  // accpet & parse params

  // render

  // return images to client

  return 0;
}