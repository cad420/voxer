#include "MRCReader.hpp"
#include "databases/conversion.hpp"
#include "voxer/utils.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

namespace voxer {

#define MRC_IDTYPE_MONO 0
#define MRC_IDTYPE_TILT 1
#define MRC_IDTYPE_TILTS 2
#define MRC_IDTYPE_LINA 3
#define MRC_IDTYPE_LINS 4

#define MRC_SCALE_LINEAR 1
#define MRC_SCALE_POWER 2
#define MRC_SCALE_LOG 3
#define MRC_SCALE_BKG 4

/* DOC_CODE MRC data modes */
/* The modes defined for MRC files in IMOD */
#define MRC_MODE_BYTE 0
#define MRC_MODE_SHORT 1
#define MRC_MODE_FLOAT 2
#define MRC_MODE_COMPLEX_SHORT 3
#define MRC_MODE_COMPLEX_FLOAT 4
#define MRC_MODE_USHORT 6
#define MRC_MODE_RGB 16
#define MRC_MODE_4BIT 101
/* END_CODE */

#define MRC_EXT_TYPE_NONE 0
#define MRC_EXT_TYPE_SERI 1
#define MRC_EXT_TYPE_AGAR 2
#define MRC_EXT_TYPE_FEI 3
#define MRC_EXT_TYPE_UNKNOWN 4

#define PACKED_4BIT_MODE 1
#define PACKED_HALF_XSIZE 2

#define MRC_RAMP_LIN 1
#define MRC_RAMP_EXP 2
#define MRC_RAMP_LOG 3

#define MRC_LABEL_SIZE 80
#define MRC_NEXTRA 16
#define MRC_NLABELS 10
#define MRC_HEADER_SIZE 1024 /* Length of Header is 1024 Bytes. */
#define MRC_MAXCSIZE 3

#define IIUNIT_SWAPPED (1l << 0)
#define IIUNIT_BYTES_SIGNED (1l << 1)
#define IIUNIT_OLD_STYLE (1l << 2)
#define IIUNIT_NINT_BUG (1l << 3)
#define IIUNIT_BAD_MAPCRS (1l << 4)
#define IIUNIT_4BIT_MODE (1l << 5)
#define IIUNIT_HALF_XSIZE (1l << 6)
#define IIUNIT_Y_INVERTED (1l << 7)

/*  Data Field Offset in MRC Header according to MRC2014
        For more infomation, Please read
        http://www.ccpem.ac.uk/mrc_format/mrc_format.php
*/
#define NX_OFFSET 0
#define NY_OFFSET 4
#define NZ_OFFSET 8
#define MODE_OFFSET 12
#define NXSTART_OFFSET 16
#define NYSTART_OFFSET 20
#define NZSTART_OFFSET 24
#define MX_OFFSET 28
#define MY_OFFSET 32
#define MZ_OFFSET 36
/*The follwing three are CELLA in the MRC header description*/
#define XLEN_OFFSET 40
#define YLEN_OFFSET 44
#define ZLEN_OFFSET 48
/*The follwing three are CELLB in the MRC header description*/
#define ALPHA_OFFSET 52
#define BETA_OFFSET 56
#define GAMMA_OFFSET 60

#define MAPC_OFFSET 64
#define MAPR_OFFSET 68
#define MAPS_OFFSET 72
#define DMIN_OFFSET 76
#define DMAX_OFFSET 80
#define DMEAN_OFFSET 84
#define ISPG_OFFSET 88
#define NSYMBT_OFFSET 92
#define FIRST_BLANK_OFFSET 96
#define EXTTYP_OFFSET 104
#define NVERSION_OFFSET 108
//#define SECOND_BLANK_OFFSET     112
#define XORIGIN_OFFSET 196
#define YORIGIN_OFFSET 200
#define ZORIGIN_OFFSET 204
#define MAP_OFFSET 208
#define STAMP_OFFSET 212
#define RMS_OFFSET 216
#define NLABL_OFFSET 220
#define LABEL_OFFSET 224

#define MRC_VERSION_FIELD(year, verion) ((year * 10) + version)

struct MRCHeader {
  int32_t nx;   /*  # of Columns                  */
  int32_t ny;   /*  # of Rows                     */
  int32_t nz;   /*  # of Sections.                */
  int32_t mode; /*  given by #define MRC_MODE...  */
  /*0 8-bit signed integer (range -128 to 127)
  1 16-bit signed integer
  2 32-bit signed real
  3 transform : complex 16-bit integers
  4 transform : complex 32-bit reals
  6 16-bit unsigned integer
  */

  int32_t nxstart; /*  Starting point of sub image.  UNSUPPORTED */
  int32_t nystart;
  int32_t nzstart;

  int32_t mx; /* "Grid size", # of pixels in "unit cell"    */
  int32_t my; /* Keep the same as nx, ny, nz                */
  int32_t mz;

  float xlen; /* length of unit cell in Angstroms           */
  float ylen; /* get scale = xlen/nx ...                    */
  float zlen;

  float alpha; /* cell angles, ignored, set to 90            */
  float beta;
  float gamma;

  int32_t mapc; /* map column  1=x,2=y,3=z.     UNSUPPORTED  */
  int32_t mapr; /* map row     1=x,2=y,3=z.                  */
  int32_t maps; /* map section 1=x,2=y,3=z.                  */

  float dmin;  // Minimum Pixel Value
  float dmax;  // Maximum Pixel Value
  float dmean; // Mean of Pixel Value

  /* 1/12/12: Removed nsymbt and made ispg be 4 bytes to match standard */
  int32_t ispg; /* space group number in the standard */ /* 64 bytes */

  int32_t nsymbt; /* This is nsymbt in the MRC standard. */
  /*Number of bytes in extended header*/

  /*Following definition is not a part of standard.
   * */
  int16_t creatid; /* Used to be creator id, hvem = 1000, now 0 */

  int8_t blank[6]; // Blank data. First two bytes should be 0.

  /**/
  int8_t extType[4]; /* Extended type. */
  int32_t nversion;  /* Version number in MRC 2014 standard */
  /**/

  int8_t blank2[16]; /*Blank*/
  int16_t nint;
  int16_t nreal;

  /*20 bytes blank*/
  int16_t sub;
  int16_t zfac;
  float min2;
  float max2;
  float min3;
  float max3;

  int32_t imodStamp;
  int32_t imodFlags;
  /*Bit flags:
  1 = bytes are stored as signed
  2 = pixel spacing was set from size in extended header
  4 = origin is stored with sign inverted from definition below
  8 = RMS value is negative if it was not computed
  16 = Bytes have two 4-bit values, the first one in the
  low 4 bits and the second one in the high 4 bits*/

  /*  UINT   extra[MRC_NEXTRA];*/

  /* HVEM extra data */
  /* DNM 3/16/01: divide idtype into two shorts */

  int16_t idtype;
  int16_t lens;
  int16_t nd1; /* Devide by 100 to get float value. */
  int16_t nd2;
  int16_t vd1;
  int16_t vd2;
  float tiltangles[6]; /* 0,1,2 = original:  3,4,5 = current */

#ifdef OLD_STYLE_HEADER
  /* before 2.6.20 */
  /* DNM 3/16/01: redefine the last three floats as wavelength numbers */
  int16_t nwave; /* # of wavelengths and values */
  int16_t wave1;
  int16_t wave2;
  int16_t wave3;
  int16_t wave4;
  int16_t wave5;
  float zorg; /* origin */

  float xorg;
  float yorg;
#else
  /* MRC 2000 standard */
  float xorg;
  float yorg;
  float zorg;
  int8_t cmap[4];
  int8_t stamp[4];
  float rms;
#endif

  int32_t nlabl;
  int8_t labels[MRC_NLABELS][MRC_LABEL_SIZE + 1];

  /* Internal data not stored in file header */
  //      b3dUByte *symops;
  //      FILE   *fp;
  //      int    pos;
  //      struct LoadInfo *li;
  //      int    headerSize;
  //      int    sectionSkip;
  //      int    swapped;
  //      int    bytesSigned;
  //      int    yInverted;
  //      int    iiuFlags;
  //      int    packed4bits;

  //      char *pathname;
  //      char *filedesc;
  //      char *userData;
};

union FloatConv {
  float f;
  char byte[4];
};

struct MRCComplexShort {
  int16_t a;
  int16_t b;
};

struct MRCComplexFloat {
  float a;
  float b;
};

static float reverseEndian(float value) {
  FloatConv c1{}, c2{};
  c1.f = value;
  c2.byte[0] = c1.byte[3];
  c2.byte[1] = c1.byte[2];
  c2.byte[2] = c1.byte[1];
  c2.byte[3] = c1.byte[0];
  return c2.f;
}

static uint16_t reverseByte(uint16_t value) {
  const auto v = ((value & 0xffu) << 8u) | ((value & 0xff00u) >> 8u);
  return v;
}

static uint32_t reverseByte(uint32_t value) {
  const auto v = ((value & 0xffU) << 24u) | ((value & 0xff00U) << 8u) |
                 ((value & 0xff0000U) >> 8u) | ((value & 0xff000000U) >> 24u);
  return v;
}

enum class DataType {
  Integer8,
  Integer16,
  Integer32,
  Real32,
  Complex16,
  Complex32
};

static auto get_data_type(const MRCHeader &header) -> DataType {
  switch (header.mode) {
  case MRC_MODE_BYTE:
    return DataType::Integer8;
  case MRC_MODE_FLOAT:
    return DataType::Real32;
  case MRC_MODE_COMPLEX_SHORT:
    return DataType::Complex16;
  case MRC_MODE_COMPLEX_FLOAT:
    return DataType::Complex32;
  case MRC_MODE_SHORT:
  case MRC_MODE_USHORT:
    return DataType::Integer16;
  default:
    return DataType();
  }
}

static auto get_type_size(DataType type) -> size_t {
  switch (type) {
  case DataType::Integer8:
    return sizeof(int8_t);
  case DataType::Integer16:
    return sizeof(int16_t);
  case DataType::Integer32:
    return sizeof(int32_t);
  case DataType::Complex16:
    return sizeof(MRCComplexShort);
  case DataType::Complex32:
    return sizeof(MRCComplexFloat);
  case DataType::Real32:
    return sizeof(float);
  default:
    return 0;
  }
}

static auto read_mrc_header(const vector<uint8_t> &buffer) -> MRCHeader {
  MRCHeader header{};
  memcpy(&header.nx, buffer.data() + NX_OFFSET, sizeof(int32_t));
  memcpy(&header.ny, buffer.data() + NY_OFFSET, sizeof(int32_t));
  memcpy(&header.nz, buffer.data() + NZ_OFFSET, sizeof(int32_t));
  memcpy(&header.mode, buffer.data() + MODE_OFFSET, sizeof(int32_t));
  memcpy(&header.nxstart, buffer.data() + NXSTART_OFFSET, sizeof(int32_t));
  memcpy(&header.nystart, buffer.data() + NYSTART_OFFSET, sizeof(int32_t));
  memcpy(&header.nzstart, buffer.data() + NZSTART_OFFSET, sizeof(int32_t));
  memcpy(&header.mx, buffer.data() + MX_OFFSET, sizeof(int32_t));
  memcpy(&header.my, buffer.data() + MY_OFFSET, sizeof(int32_t));
  memcpy(&header.mz, buffer.data() + MZ_OFFSET, sizeof(int32_t));
  memcpy(&header.xlen, buffer.data() + XLEN_OFFSET, sizeof(float));
  memcpy(&header.ylen, buffer.data() + YLEN_OFFSET, sizeof(float));
  memcpy(&header.zlen, buffer.data() + ZLEN_OFFSET, sizeof(float));
  memcpy(&header.alpha, buffer.data() + ALPHA_OFFSET, sizeof(float));
  memcpy(&header.beta, buffer.data() + BETA_OFFSET, sizeof(float));
  memcpy(&header.gamma, buffer.data() + GAMMA_OFFSET, sizeof(float));
  memcpy(&header.mapc, buffer.data() + MAPC_OFFSET, sizeof(int32_t));
  memcpy(&header.mapr, buffer.data() + MAPR_OFFSET, sizeof(int32_t));
  memcpy(&header.maps, buffer.data() + MAPS_OFFSET, sizeof(int32_t));
  memcpy(&header.dmin, buffer.data() + DMIN_OFFSET, sizeof(float));
  memcpy(&header.dmax, buffer.data() + DMAX_OFFSET, sizeof(float));
  memcpy(&header.dmean, buffer.data() + DMEAN_OFFSET, sizeof(float));
  memcpy(&header.ispg, buffer.data() + ISPG_OFFSET, sizeof(int32_t));
  memcpy(&header.nsymbt, buffer.data() + NSYMBT_OFFSET, sizeof(int32_t));

  // memcpy(hd,buffer,sizeof(24*4));
  memcpy(header.extType, buffer.data() + EXTTYP_OFFSET, sizeof(char) * 4);
  memcpy(&header.nversion, buffer.data() + NVERSION_OFFSET, sizeof(int32_t));
  memcpy(&header.xorg, buffer.data() + XORIGIN_OFFSET, sizeof(float));
  memcpy(&header.yorg, buffer.data() + YORIGIN_OFFSET, sizeof(float));
  memcpy(&header.zorg, buffer.data() + ZORIGIN_OFFSET, sizeof(float));
  memcpy(&header.stamp, buffer.data() + STAMP_OFFSET, sizeof(int32_t));
  memcpy(&header.rms, buffer.data() + RMS_OFFSET, sizeof(float));
  memcpy(&header.nlabl, buffer.data() + NLABL_OFFSET, sizeof(int32_t));
  memcpy(&header.labels, buffer.data() + LABEL_OFFSET, sizeof(int8_t));

  if (header.stamp[0] == 0x11 && header.stamp[1] == 0x11) {
    header.nx = reverseByte((uint32_t)header.nx);
    header.ny = reverseByte((uint32_t)header.ny);
    header.nz = reverseByte((uint32_t)header.nz);
    header.mode = reverseByte((uint32_t)header.mode);
    header.nxstart = reverseByte((uint32_t)header.nxstart);
    header.nystart = reverseByte((uint32_t)header.nystart);
    header.nzstart = reverseByte((uint32_t)header.nzstart);
    header.mx = reverseByte((uint32_t)header.mx);
    header.my = reverseByte((uint32_t)header.my);
    header.mz = reverseByte((uint32_t)header.mz);
    header.xlen = reverseEndian(header.xlen);
    header.ylen = reverseEndian(header.ylen);
    header.zlen = reverseEndian(header.zlen);

    header.alpha = reverseEndian((uint32_t)header.alpha);
    header.beta = reverseEndian(header.beta);
    header.gamma = reverseEndian(header.gamma);
    header.mapc = reverseByte((uint32_t)header.mapc);
    header.mapr = reverseByte((uint32_t)header.mapr);
    header.maps = reverseByte((uint32_t)header.maps);
    header.dmin = reverseEndian(header.dmin);
    header.dmax = reverseEndian(header.dmax);
    header.dmean = reverseEndian(header.dmean);

    header.ispg = reverseByte((uint32_t)header.ispg);
    header.nsymbt = reverseByte((uint32_t)header.nsymbt);
    header.creatid = reverseByte((uint16_t)header.creatid);
    header.nversion = reverseByte((uint32_t)header.nversion);

    // header.nint = reverseByte((uint16_t)header.nint);
    // header.nreal = reverseByte((uint16_t)header.nreal);

    // header.sub = reverseByte((uint16_t)header.sub);
    // header.zfac = reverseByte((uint16_t)header.zfac);
    // header.min2 = reverseByte((uint32_t)header.min2);
    // header.max2 = reverseByte((uint32_t)header.max2);
    // header.min3 = reverseByte((uint32_t)header.min3);
    // header.max3 = reverseByte((uint32_t)header.max3);

    header.xorg = reverseEndian(header.xorg);
    header.yorg = reverseEndian(header.yorg);
    header.zorg = reverseEndian(header.zorg);
    header.rms = reverseEndian(header.rms);

    header.nlabl = reverseByte((uint32_t)header.nlabl);
  }

  return header;
}

MRCReader::MRCReader(const string &filepath) {
  fs.open(filepath, ios::binary);
  if (!fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }
}

auto MRCReader::load() -> Dataset {
  std::vector<uint8_t> header_buffer;
  header_buffer.reserve(MRC_HEADER_SIZE);

  fs.seekg(0, ifstream::beg);
  fs.read(reinterpret_cast<char *>(header_buffer.data()), MRC_HEADER_SIZE);
  if (fs.gcount() != MRC_HEADER_SIZE) {
    throw runtime_error("invalid MRC header");
  }

  auto header = read_mrc_header(header_buffer);

  const size_t offset = MRC_HEADER_SIZE + header.nsymbt;
  fs.seekg(offset, ifstream::beg);
  const size_t count = static_cast<size_t>(header.nx) *
                       static_cast<size_t>(header.ny) *
                       static_cast<size_t>(header.nz); // size_t is important
  const auto elem_size = get_type_size(get_data_type(header));

  vector<uint8_t> data_buffer;
  data_buffer.reserve(header.nx * header.ny * header.nz * elem_size);
  fs.read(reinterpret_cast<char *>(data_buffer.data()), count * elem_size);
  const auto readCount = fs.gcount();
  if (readCount != count * elem_size) {
    throw runtime_error("Runtime Error: Reading size error.>>>" +
                        to_string(__LINE__));
  }

  if (header.stamp[0] == 0x11 && header.stamp[1] == 0x11) {
    // big endian
    if (MRC_MODE_FLOAT == header.mode) {
      auto d = reinterpret_cast<float *>(data_buffer.data());
      for (int i = 0; i < count; i++) {
        d[i] = reverseEndian(d[i]);
      }
    } else if (MRC_MODE_SHORT == header.mode ||
               MRC_MODE_USHORT == header.mode) {
      // big endian
      auto d = reinterpret_cast<uint16_t *>(data_buffer.data());
      for (int i = 0; i < count; i++) {
        d[i] = reverseByte(static_cast<uint16_t>(d[i]));
      }
    }
  }

  Dataset dataset{};
  dataset.id = nanoid(5);
  dataset.info.dimensions = {static_cast<uint16_t>(header.nx),
                             static_cast<uint16_t>(header.ny),
                             static_cast<uint16_t>(header.nz)};
  if (header.mode == MRC_MODE_FLOAT) {
    auto total = header.nx * header.ny * header.nz;
    auto uint8_buffer =
        conversion(reinterpret_cast<const float *>(data_buffer.data()), total,
                   header.dmax, header.dmin);
    dataset.info.value_type = ValueType::UINT8;
    dataset.buffer = move(uint8_buffer);
  } else if (header.mode == MRC_MODE_BYTE) {
    dataset.buffer = move(data_buffer);
  } else {
    throw runtime_error("unsupported MRC data type");
  }

  return dataset;
}

auto MRCReader::load_region(const std::array<uint16_t, 3> &begin,
                            const std::array<uint16_t, 3> &end) -> Dataset {
  throw runtime_error("not support loading subregion");
}

} // namespace voxer