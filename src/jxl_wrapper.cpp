#include "jxl_wrapper.h"
#include "version.h"
#include <cstdint>
#include <vector>

#define JXL_NO_STDIO
#define JXL_IMPLEMENTATION

#include "qoi.h" //TODO: remove me!

//jxl specific imports:
#include <jxl/codestream_header.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/color_encoding.h>
#include <jxl/types.h>
#include <jxl/encode.h>
#include <jxl/encode_cxx.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>
#include <jxl/types.h>



using namespace godot;

//in theory, i only need to change the encode and decode_to_image functions. everything else here remains unchanged...


namespace jxl_functions { //my own tomfoolery goes here

	Error decode(const uint8_t *data_in,size_t data_length,const Ref<Image> &out_image){
		//much, but not all of this function is derived from: https://github.com/libjxl/libjxl/blob/main/examples/decode_oneshot.cc
		//for now i am just doing this directly, the real fun begins later.
		
		JxlResizableParallelRunnerPtr runner =JxlResizableParallelRunnerMake(nullptr);
		JxlDecoderPtr decoder = JxlDecoderMake(nullptr);
		std::vector<uint8_t> _icc_profile; //i have no plans currently to use this. but it needs to be here to decode the image.
		std::vector<uint8_t> *icc_profile = &_icc_profile; //the code below expects a pointer to a vector.
		size_t _xsize, _ysize, *xsize, *ysize; //much like with the icc profile, these variables are required as pointers.
		xsize = &_xsize;
		ysize = &_ysize;
		std::vector<float> _pixels;//a third time for good luck! this is the output buffer.
		std::vector<float> *pixels = _pixels;
		if (JXL_DEC_SUCCESS !=
			JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO |
													 JXL_DEC_COLOR_ENCODING |
													 JXL_DEC_FULL_IMAGE)) {
		  fprintf(stderr, "JxlDecoderSubscribeEvents failed\n");
		  return Error::FAILED;
		}
		if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(decoder.get(),
														   JxlResizableParallelRunner,
														   runner.get())) {
		  fprintf(stderr, "JxlDecoderSetParallelRunner failed\n");
		  return Error::FAILED;
		}
		JxlBasicInfo info;
		JxlPixelFormat format = {4, JXL_TYPE_FLOAT, JXL_NATIVE_ENDIAN, 0};	  
		for(;;){
			JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());
			switch (status) { //my main notable decision is to convert the decoder from an if-else ladder to a switch to make it easier to reason about.
				case JXL_DEC_ERROR:
					fprintf(stderr, "JXL: Decoder error\n");
					return Error::FAILED;
					break;
				case JXL_DEC_NEED_MORE_INPUT:
					fprintf(stderr, "JXL: Error, already provided all input\n");
					return Error::ERR_FILE_CORRUPT;
					break;
				case JXL_DEC_BASIC_INFO:
					if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(decoder.get(), &info)) {
						fprintf(stderr, "JxlDecoderGetBasicInfo failed\n");
						return Error::ERR_FILE_CORRUPT;
				  	}
					// out_image.resize(info.xsize,info.ysize);
				  	*xsize = info.xsize;
				  	*ysize = info.ysize;
				  	JxlResizableParallelRunnerSetThreads(
						runner.get(),
						JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
					break;
				case JXL_DEC_COLOR_ENCODING:
					// Get the ICC color profile of the pixel data
					size_t icc_size;
					if (JXL_DEC_SUCCESS !=
						JxlDecoderGetICCProfileSize(decoder.get(), JXL_COLOR_PROFILE_TARGET_DATA,
													&icc_size)) {
					fprintf(stderr, "JxlDecoderGetICCProfileSize failed\n");
					return Error::ERR_FILE_CORRUPT;
					}
					icc_profile->resize(icc_size);
					if (JXL_DEC_SUCCESS != JxlDecoderGetColorAsICCProfile(
											decoder.get(), JXL_COLOR_PROFILE_TARGET_DATA,
											icc_profile->data(), icc_profile->size())) {
					fprintf(stderr, "JxlDecoderGetColorAsICCProfile failed\n");
					return Error::ERR_FILE_CORRUPT;
					}
					break;
				case JXL_DEC_NEED_IMAGE_OUT_BUFFER:
					size_t buffer_size;
					if (JXL_DEC_SUCCESS !=
						JxlDecoderImageOutBufferSize(decoder.get(), &format, &buffer_size)) {
						fprintf(stderr, "JxlDecoderImageOutBufferSize failed\n");
						return Error::ERR_CANT_CREATE;
					}
					if (buffer_size != *xsize * *ysize * 16) {
						fprintf(stderr, "JXL: Invalid out buffer size %d %d\n",
								static_cast<int>(buffer_size),
								static_cast<int>(*xsize * *ysize * 16));
						return Error::ERR_CANT_CREATE;
					}
					pixels->resize(*xsize * *ysize * 4);
					void* pixels_buffer = static_cast<void*>(pixels->data());
					size_t pixels_buffer_size = pixels->size() * sizeof(float);
					if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format,
																		pixels_buffer,
																		pixels_buffer_size)) {
						fprintf(stderr, "JxlDecoderSetImageOutBuffer failed\n");
						return Error::ERR_CANT_CREATE;
					}
					break;
				case JXL_DEC_FULL_IMAGE: //frame is complete
					//i am not supporting animated images. that is a right and proper can of worms we are absolutely not ready for.
					//instead, we will halt on the first completed frame.
				case JXL_DEC_SUCCESS: //full image is complete
					goto IMAGE_DECODE_COMPLETE;
					break;
				default:
					fprintf(stderr, "JXL: Unknown decoder status\n");
					return Error::FAILED;
					break;
			}
		}
	IMAGE_DECODE_COMPLETE:
		//ok, by this point we have a fully decoded image in float format.
		const_cast<Image *>(out_image.ptr())->set_data(_xsize, _ysize, false,Image::Format::FORMAT_RGBAF, pixels.len(), pixels->data());
	}
}


void JXL::_bind_methods() {
	ClassDB::bind_static_method(NAMEOF(JXL), D_METHOD(NAMEOF(_get_version)), &JXL::_get_version);
	ClassDB::bind_static_method(NAMEOF(JXL), D_METHOD(NAMEOF(write), "path", "image"), &JXL::write);
	ClassDB::bind_static_method(NAMEOF(JXL), D_METHOD(NAMEOF(encode), "image"), &JXL::encode);
	ClassDB::bind_static_method(NAMEOF(JXL), D_METHOD(NAMEOF(read), "path"), &JXL::read);
	ClassDB::bind_static_method(NAMEOF(JXL), D_METHOD(NAMEOF(decode), "data"), &JXL::decode);
}

String JXL::_get_version() {
	return GJXL_VERSION_STR;
}

Ref<Image> JXL::read(String path) {
	ERR_FAIL_COND_V_MSG(!FileAccess::file_exists(path), Ref<Image>(), "File does not exist: " + path);

	Ref<FileAccess> f = FileAccess::open(path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(f.is_null(), Ref<Image>(), "Could not open the file for reading: " + path + ". Error: " + String::num_int64((int)FileAccess::get_open_error()));

	PackedByteArray data = f->get_buffer(f->get_length());
	ERR_FAIL_COND_V_MSG(f->get_error() != Error::OK, Ref<Image>(), "Failed to write data to file " + path + ". Error: " + String::num_int64((int)f->get_error()));
	f.unref();

	return decode(data);
}

Ref<Image> JXL::decode(const PackedByteArray &data) {
	ERR_FAIL_COND_V_MSG(data.size() == 0, Ref<Image>(), "Image data cannot be empty");

	Ref<Image> img;
	img.instantiate();

	if (decode_to_image(data, img) != Error::OK)
		return Ref<Image>();

	return img;
}

Error JXL::decode_to_image(const PackedByteArray &data, const Ref<Image> &out_image) {
	// jxl_desc desc;
	// void *out;

	// out = jxl_functions::decode(data.ptr(), (int)data.size(), &desc, 0);
	Error result = jxl_functions::decode(data->data() , data->len() ,out_image);
	return result;
	// ERR_FAIL_COND_V_MSG(out == NULL, ERR_FILE_CORRUPT, "Unable to decode data");

	// PackedByteArray img_data;

	// int64_t size = desc.channels * desc.width * desc.height;
	// img_data.resize(size);

	// if (img_data.size() != size) {
	// 	::free(out);
	// 	ERR_FAIL_V_MSG(ERR_OUT_OF_MEMORY, "Unable to resize PackedByteArray");
	// }

	// memcpy(img_data.ptrw(), out, size);
	// ::free(out);

	// const_cast<Image *>(out_image.ptr())->set_data(desc.width, desc.height, false, desc.channels == 3 ? Image::Format::FORMAT_RGB8 : Image::Format::FORMAT_RGBA8, img_data);
	// return Error::OK;
}

Error JXL::write(String path, Ref<Image> img) {
	ERR_FAIL_COND_V_MSG(img.is_null(), Error::ERR_INVALID_PARAMETER, "Image cannot be null");
	ERR_FAIL_COND_V_MSG(img->is_empty(), Error::ERR_INVALID_PARAMETER, "Image cannot be empty");

	PackedByteArray b = encode(img);
	ERR_FAIL_COND_V_MSG(b.size() == 0, Error::FAILED, "Image cannot be null or empty");

	Ref<FileAccess> f = FileAccess::open(path, FileAccess::WRITE);

	ERR_FAIL_COND_V_MSG(f.is_null(), FileAccess::get_open_error(), "Could not open the file for writing: " + path + ". Error: " + String::num_int64((int)FileAccess::get_open_error()));

	f->store_buffer(b);
	auto err = f->get_error();
	f.unref();

	ERR_FAIL_COND_V_MSG(err != Error::OK, f->get_error(), "Failed to write data to file " + path + ". Error: " + String::num_int64((int)err));

	return Error::OK;
}

PackedByteArray JXL::encode(Ref<Image> img) {
	//TODO: convert this to jxl
	ERR_FAIL_COND_V_MSG(img.is_null(), PackedByteArray(), "Image cannot be null");
	ERR_FAIL_COND_V_MSG(img->is_empty(), PackedByteArray(), "Image cannot be empty");

	bool has_alpha = img->detect_alpha();

	if (img->get_format() != Image::Format::FORMAT_RGB8 && img->get_format() != Image::Format::FORMAT_RGBA8) {
		// try to convert
		img->convert(has_alpha ? Image::Format::FORMAT_RGBA8 : Image::Format::FORMAT_RGB8);

		ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGB8 && img->get_format() != Image::FORMAT_RGBA8, PackedByteArray(), "Unsupported image format");
	} else {
		img->convert(has_alpha ? Image::Format::FORMAT_RGBA8 : Image::Format::FORMAT_RGB8);
	}

	jxl_desc enc = {
		(uint32_t)img->get_width(),
		(uint32_t)img->get_height(),
		(uint8_t)(img->get_format() == Image::Format::FORMAT_RGB8 ? 3 : 4),
		JXL_SRGB
	};

	int len = 0;
	void *out;

	out = jxl_functions::encode(img->get_data().ptr(), &enc, &len);
	ERR_FAIL_COND_V_MSG(out == NULL, PackedByteArray(), "Unable to encode the image");

	PackedByteArray res;
	res.resize(len);
	if (res.size() != len) {
		::free(out);
		ERR_FAIL_V_MSG(PackedByteArray(), "Unable to resize PackedByteArray");
	}

	memcpy(res.ptrw(), out, len);
	::free(out);

	return res;
}
