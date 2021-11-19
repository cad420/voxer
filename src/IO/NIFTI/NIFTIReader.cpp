#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <voxer/IO/NIFTIReader.hpp>
#include <voxer/IO/utils.hpp>

using namespace std;

namespace voxer {

NIFTIReader::NIFTIReader(const char *filepath) {


    fs.open(filepath, ios::binary);
    if (!fs.is_open()) {
        throw runtime_error(string("cannot load file: ") + filepath);
    }
    //头长度
    int header;

	int bytes_read=0;
    fs.read(reinterpret_cast<char*>(&header), sizeof(header));
	bytes_read += fs.gcount();
    
	if (header != 348)
	{
    	throw std::runtime_error(std::string("Invalid header size: should be 348 bytes\n"));
	}

    // //数据类型
    char data_type[10];
    
    fs.read(data_type, sizeof(data_type));
	bytes_read += fs.gcount();

    char db_name[18];
    fs.read(db_name, sizeof(db_name));
	bytes_read += fs.gcount();

	int extents;
    fs.read(reinterpret_cast<char*>(&extents), sizeof(extents));
	bytes_read += fs.gcount();

	short session_error;
    fs.read(reinterpret_cast<char*>(&session_error), sizeof(session_error));
	bytes_read += fs.gcount();

	char regular;
    fs.read(&regular, sizeof(regular));
	bytes_read += fs.gcount();

	char dim_info;
    fs.read(&dim_info, sizeof(dim_info));
	bytes_read += fs.gcount();

	// Data array dimensions
	short dim[8];
    fs.read(reinterpret_cast<char*>(dim), sizeof(dim));
	bytes_read += fs.gcount();

	float intent_p1;
    fs.read(reinterpret_cast<char*>(&intent_p1), sizeof(intent_p1));
	bytes_read += fs.gcount();

	float intent_p2;
    fs.read(reinterpret_cast<char*>(&intent_p2), sizeof(intent_p2));
	bytes_read += fs.gcount();

	float intent_p3;
    fs.read(reinterpret_cast<char*>(&intent_p3), sizeof(intent_p3));
	bytes_read += fs.gcount();

	short intent_code;
    fs.read(reinterpret_cast<char*>(&intent_code), sizeof(intent_code));
	bytes_read += fs.gcount();

	// Data type 
    //  (code=2 -- type=uint8)
    //  (code=4 -- type=int16)
    //  (code=16 -- type=float)
    //  (code=512 -- type=uint16)
    //  (code=768 -- type=uint32)
    //  (code=1280 -- type=uint64)
	short datatype;
    fs.read(reinterpret_cast<char*>(&datatype), sizeof(datatype));
	bytes_read += fs.gcount();

	// Number of bits per voxel (code=512 -- bitpix=16 -- type=unsigned short)
	short bitpix;
    fs.read(reinterpret_cast<char*>(&bitpix), sizeof(bitpix));
	bytes_read += fs.gcount();

	short slice_start;
    fs.read(reinterpret_cast<char*>(&slice_start), sizeof(slice_start));
	bytes_read += fs.gcount();

	float pixdim[8];
    fs.read(reinterpret_cast<char*>(pixdim), sizeof(pixdim));
	bytes_read += fs.gcount();

	float vox_offset;
    fs.read(reinterpret_cast<char*>(&vox_offset), sizeof(vox_offset));
	bytes_read += fs.gcount();

	float scl_slope;
    fs.read(reinterpret_cast<char*>(&scl_slope), sizeof(scl_slope));
	bytes_read += fs.gcount();

	float scl_inter;
    fs.read(reinterpret_cast<char*>(&scl_inter), sizeof(scl_inter));
	bytes_read += fs.gcount();

	short slice_end;
    fs.read(reinterpret_cast<char*>(&slice_end), sizeof(slice_end));
	bytes_read += fs.gcount();

	char slice_code;
    fs.read(&slice_code, sizeof(slice_code));
	bytes_read += fs.gcount();

	char xyzt_units;
    fs.read(&xyzt_units, sizeof(xyzt_units));
	bytes_read += fs.gcount();

	float cal_max;
    fs.read(reinterpret_cast<char*>(&cal_max), sizeof(cal_max));
	bytes_read += fs.gcount();

	float cal_min;
    fs.read(reinterpret_cast<char*>(&cal_min), sizeof(cal_min));
	bytes_read += fs.gcount();

	float slice_duration;
    fs.read(reinterpret_cast<char*>(&slice_duration), sizeof(slice_duration));
	bytes_read += fs.gcount();

	float toffset;
    fs.read(reinterpret_cast<char*>(&toffset), sizeof(toffset));
	bytes_read += fs.gcount();

	int glmax;
    fs.read(reinterpret_cast<char*>(&glmax), sizeof(glmax));
	bytes_read += fs.gcount();

	int glmin;
    fs.read(reinterpret_cast<char*>(&glmin), sizeof(glmin));
	bytes_read += fs.gcount();

	char descrip[80];
    fs.read(descrip, sizeof(descrip));
	bytes_read += fs.gcount();

	char aux_file[24];
    fs.read(aux_file, sizeof(aux_file));
	bytes_read += fs.gcount();

	short qform_code;
    fs.read(reinterpret_cast<char*>(&qform_code), sizeof(qform_code));
	bytes_read += fs.gcount();

	short sform_code;
    fs.read(reinterpret_cast<char*>(&sform_code), sizeof(sform_code));
	bytes_read += fs.gcount();

	float quatern_b;
    fs.read(reinterpret_cast<char*>(&quatern_b), sizeof(quatern_b));
	bytes_read += fs.gcount();

	float quatern_c;
    fs.read(reinterpret_cast<char*>(&quatern_c), sizeof(quatern_c));
	bytes_read += fs.gcount();

	float quatern_d;
    fs.read(reinterpret_cast<char*>(&quatern_d), sizeof(quatern_d));
	bytes_read += fs.gcount();

	float qoffset_x;
    fs.read(reinterpret_cast<char*>(&qoffset_x), sizeof(qoffset_x));
	bytes_read += fs.gcount();

	float qoffset_y;
    fs.read(reinterpret_cast<char*>(&qoffset_y), sizeof(qoffset_y));
	bytes_read += fs.gcount();

	float qoffset_z;
    fs.read(reinterpret_cast<char*>(&qoffset_z), sizeof(qoffset_z));
	bytes_read += fs.gcount();

	float srow_x[4];
    fs.read(reinterpret_cast<char*>(srow_x), sizeof(srow_x));
	bytes_read += fs.gcount();

	float srow_y[4];
    fs.read(reinterpret_cast<char*>(srow_y), sizeof(srow_y));
	bytes_read += fs.gcount();

	float srow_z[4];
    fs.read(reinterpret_cast<char*>(srow_z), sizeof(srow_z));
	bytes_read += fs.gcount();

	char intent_name[16];
    fs.read(intent_name, sizeof(intent_name));
	bytes_read += fs.gcount();

	char magic[4];
    fs.read(magic, sizeof(magic));
	bytes_read += fs.gcount();
	
	if (bytes_read != 348)
	{
    	throw std::runtime_error(std::string("Error reading header\n"));
	}

	// Data type 
    //  (code=2 -- type=uint8)
    //  (code=4 -- type=int16)
    //  (code=16 -- type=float)
    //  (code=512 -- type=uint16)
    //  (code=768 -- type=uint32)
    //  (code=1280 -- type=uint64)

    if (datatype == 2) {
        value_type = ValueType::UINT8;
    } else if (datatype == 512) {
        value_type = ValueType::UINT16;
    } else if (datatype == 4) {
        value_type = ValueType::INT16;
    } else if (datatype == 16) {
        value_type = ValueType::FLOAT;
    } else {
        throw runtime_error(string("only support value type:uint8 uint16 int16 float"));
    }
    dimensions[0]=dim[1];
    dimensions[1]=dim[2];
    dimensions[2]=dim[3];
}

auto NIFTIReader::load() -> unique_ptr<StructuredGrid> {
    auto dataset = make_unique<StructuredGrid>();
    dataset->info.dimensions = dimensions;
    uint64_t total = dimensions[0] * dimensions[1] * dimensions[2];
    if (value_type == ValueType::UINT8) {
        dataset->original_range = {0.0f, 255.0f};
        dataset->buffer.reserve(dataset->info.byte_count());
        dataset->buffer.insert(dataset->buffer.begin(),
                            istream_iterator<uint8_t>(fs),
                            istream_iterator<uint8_t>());
    }else if (value_type == ValueType::UINT16) {
        vector<uint16_t> buffer;
        buffer.resize(total);
        fs.read(reinterpret_cast<char *>(buffer.data()), total * sizeof(uint16_t));
        dataset->buffer =
            crop_others_to_uint8<uint16_t>(buffer.data(), total, dataset->original_range);
    }  else if (value_type == ValueType::INT16) {
        vector<int16_t> buffer;
        buffer.resize(total);
        fs.read(reinterpret_cast<char *>(buffer.data()), total * sizeof(int16_t));
        dataset->buffer =
            crop_others_to_uint8<int16_t>(buffer.data(), total, dataset->original_range);
    } else if (value_type == ValueType::FLOAT) {
        vector<float> buffer;
        buffer.resize(total);
        fs.read(reinterpret_cast<char *>(buffer.data()), total * sizeof(float));
        dataset->buffer =
            crop_others_to_uint8<float>(buffer.data(), total, dataset->original_range);
    } else {
        throw runtime_error(string("unsupported value type"));
    }
    dataset->info.value_type = ValueType::UINT8;
    return dataset;
}

auto NIFTIReader::load_region(const std::array<uint16_t, 3> &begin,
                            const std::array<uint16_t, 3> &end)
    -> std::unique_ptr<StructuredGrid> {
    // TODO: load subregion
    throw runtime_error("not support loading subregion");
}

} // namespace voxer
