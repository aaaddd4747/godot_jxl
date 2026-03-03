/* register_types.cpp */

#include "compiler.h"
#include "jxl_import.h"
#include "jxl_save.h"
#include "jxl_wrapper.h"
#include "version.h"

#include "qoi_shared.h"

GODOT_WARNING_DISABLE()
#include <godot_cpp/classes/project_settings.hpp>
GODOT_WARNING_RESTORE()
using namespace godot;

Ref<JXLImport> jxl_import_plugin;
Ref<JXLResourceSaver> jxl_resource_saver;

#ifdef DEBUG_ENABLED
#include "asset_library_update_checker.h"
Ref<AssetLibraryUpdateChecker> upd_checker;
#ifdef TELEMETRY_ENABLED
#include "dst_modules/GDExtension/usage_time_reporter.h"
DEFINE_TELEMETRY_OBJECT_ID(gjxl_usage_obj_id);
#endif
#endif

/** GDExtension Initialize **/
void GDE_EXPORT initialize_godot_jxl_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {

		ClassDB::register_class<JXL>();
		ClassDB::register_class<JXLImport>();
		ClassDB::register_class<JXLResourceSaver>();

		ProjectSettings *ps = ProjectSettings::get_singleton();

		DEFINE_SETTING_AND_GET(bool importer_enabled, "rendering/textures/jxl/enable_jxl_importer", true, Variant::BOOL);
		DEFINE_SETTING_AND_GET(bool saver_enabled, "rendering/textures/jxl/enable_jxl_saver", true, Variant::BOOL);

		if (importer_enabled) {
			jxl_import_plugin.instantiate();
			jxl_import_plugin->add_format_loader();
		}

		if (saver_enabled) {
			jxl_resource_saver.instantiate();
			ResourceSaver::get_singleton()->add_resource_format_saver(jxl_resource_saver, false);
		}
	}

#ifdef DEBUG_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ClassDB::register_class<AssetLibraryUpdateChecker>();
		upd_checker.instantiate();

#ifdef TELEMETRY_ENABLED
		INIT_EDITOR_TELEMETRY_OBJECT(gjxl_usage_obj_id, "Godot JXL", TELEMETRY_APP_ID, GJXL_VERSION_STR, "JXL/settings/", TELEMETRY_HOST, "telemetry_gjxl.json");
#endif
	}
#endif
}

/** GDExtension Uninitialize **/
void GDE_EXPORT uninitialize_godot_jxl_module(ModuleInitializationLevel p_level) {
	if (jxl_import_plugin.is_valid())
		jxl_import_plugin->remove_format_loader();
	jxl_import_plugin.unref();

	if (jxl_resource_saver.is_valid())
		ResourceSaver::get_singleton()->remove_resource_format_saver(jxl_resource_saver);
	jxl_resource_saver.unref();

#ifdef DEBUG_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		upd_checker.unref();

#ifdef TELEMETRY_ENABLED
		DELETE_EDITOR_TELEMETRY_OBJECT(gjxl_usage_obj_id);
#endif
	}
#endif
}

/** GDExtension Initialize **/
extern "C" {
GDExtensionBool GDE_EXPORT godot_jxl_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_godot_jxl_module);
	init_obj.register_terminator(uninitialize_godot_jxl_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
