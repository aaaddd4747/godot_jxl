#include "jxl_import.h"
#include "jxl_wrapper.h"

PackedStringArray JXLImport::_get_recognized_extensions() const {
	return PackedStringArray(TypedArray<String>::make("jxl"));
}

Error JXLImport::_load_image(const Ref<Image> &image, const Ref<FileAccess> &fileaccess, BitField<ImageFormatLoader::LoaderFlags> flags, float scale) {
	PackedByteArray data = fileaccess->get_buffer(fileaccess->get_length());
	ERR_FAIL_COND_V_MSG(fileaccess->get_error() != Error::OK, fileaccess->get_error(), "Failed to read data from file: " + fileaccess->get_path());

	return (Error)JXL::decode_to_image(data, image);
}
