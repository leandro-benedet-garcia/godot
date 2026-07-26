/**************************************************************************/
/*  lean_object.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "lean_object.h"

#include "core/object/class_db.h"
#include "core/object/script_instance.h"
#include "core/object/script_language.h"

#ifdef DEBUG_ENABLED

struct _ObjectDebugLock {
	ObjectID obj_id;

	_ObjectDebugLock(LeanObject *p_obj) {
		obj_id = p_obj->get_instance_id();
		p_obj->_lock_index.ref();
	}
	~_ObjectDebugLock() {
		LeanObject *obj_ptr = ObjectDB::get_instance(obj_id);
		if (likely(obj_ptr)) {
			obj_ptr->_lock_index.unref();
		}
	}
};

#define OBJ_DEBUG_LOCK _ObjectDebugLock _debug_lock(this);

#else

#define OBJ_DEBUG_LOCK

#endif

void LeanObject::get_property_list(List<PropertyInfo> *p_list, bool p_reversed) const {
	if (script_instance && p_reversed) {
		script_instance->get_property_list(p_list);
	}

	if (_extension) {
		const ObjectGDExtension *current_extension = _extension;
		while (current_extension) {
			p_list->push_back(PropertyInfo(Variant::NIL, current_extension->class_name, PROPERTY_HINT_NONE, current_extension->class_name, PROPERTY_USAGE_CATEGORY));

			ClassDB::get_property_list(current_extension->class_name, p_list, true, this);

			if (current_extension->get_property_list) {
#ifdef TOOLS_ENABLED
				// If this is a placeholder, we can't call into the GDExtension on the parent class,
				// because we don't have a real instance of the class to give it.
				if (likely(!_extension->is_placeholder)) {
#endif
					uint32_t pcount;
					const GDExtensionPropertyInfo *pinfo = current_extension->get_property_list(_extension_instance, &pcount);
					for (uint32_t i = 0; i < pcount; i++) {
						p_list->push_back(PropertyInfo(pinfo[i]));
					}
					if (current_extension->free_property_list2) {
						current_extension->free_property_list2(_extension_instance, pinfo, pcount);
					}
#ifndef DISABLE_DEPRECATED
					else if (current_extension->free_property_list) {
						current_extension->free_property_list(_extension_instance, pinfo);
					}
#endif // DISABLE_DEPRECATED
#ifdef TOOLS_ENABLED
				}
#endif
			}

			current_extension = current_extension->parent;
		}
	}

	_get_property_listv(p_list, p_reversed);

	const uint32_t base_script_usage = is_class(Script::get_class_static()) ? PROPERTY_USAGE_NO_EDITOR : PROPERTY_USAGE_DEFAULT;
	p_list->push_back(PropertyInfo(Variant::OBJECT, "script", PROPERTY_HINT_RESOURCE_TYPE, Script::get_class_static(), base_script_usage | PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NEVER_DUPLICATE));

	if (script_instance && !p_reversed) {
		script_instance->get_property_list(p_list);
	}

	for (const KeyValue<StringName, Variant> &K : metadata) {
		PropertyInfo pi = PropertyInfo(K.value.get_type(), "metadata/" + K.key.string());
		if (K.value.get_type() == Variant::OBJECT) {
			pi.hint = PROPERTY_HINT_RESOURCE_TYPE;
			Object *obj = K.value;
			if (Object::cast_to<Script>(obj)) {
				pi.hint_string = Script::get_class_static();
				pi.usage |= PROPERTY_USAGE_NEVER_DUPLICATE;
			} else {
				pi.hint_string = Resource::get_class_static();
			}
		}
		p_list->push_back(pi);
	}
}

bool LeanObject::is_class(const StringName &p_class) const {
	return get_gdtype().get_name_hierarchy().has(p_class);
}

LeanObject::~LeanObject() {
	if (_instance_id != ObjectID()) {
		ObjectDB::remove_instance(this);
		_instance_id = ObjectID();
	}
	_predelete_ok = true;

	if (_instance_bindings != nullptr) {
		for (uint32_t i = 0; i < _instance_binding_count; i++) {
			if (_instance_bindings[i].free_callback) {
				_instance_bindings[i].free_callback(_instance_bindings[i].token, this, _instance_bindings[i].binding);
			}
		}
		memfree(_instance_bindings);
	}
}
