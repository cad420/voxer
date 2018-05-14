#include "Encoder.h"
#define TJE_IMPLEMENTATION
#include "third_party/tiny_jpeg.h"
#include <string>
#include <vector>

using namespace std;
using ospcommon::vec2ui;

void _encode(void *context, void *data, int size) {
  auto buf = (vector<unsigned char> *)context;
  auto res = (unsigned char *)data;
  for (auto i = 0; i < size; i++) {
    buf->push_back(*(res + i));
  }
}

vector<unsigned char> Encoder::encode(vector<unsigned char> &data, vec2ui dim,
                                      string format) {
  if (format != "JPEG") {
    throw string("format not suppported!");
  }
  auto start = chrono::steady_clock::now();
  vector<unsigned char> img;
  img.reserve(dim.x * dim.y);
  tje_encode_with_func(_encode, &img, 1, dim.x, dim.y, 4, data.data());

  cout << "encode: "
       << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count()
       << " ms " << endl;
  return img;
}
