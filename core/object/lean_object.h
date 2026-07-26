/**************************************************************************/
/*  lean_object.h                                                         */
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

#pragma once

#include "core/object/object_gdextension.h"
#include "core/object/object_id.h"
#include "core/object/property_info.h"
#include "core/typedefs.h"

class ScriptInstance;

class LeanObject {
#ifdef DEBUG_ENABLED
	friend struct _ObjectDebugLock;
#endif // DEBUG_ENABLED
	// TODO: Yes, I am aware that those shouldn't be protected it shall be tweaked later
protected:
	ObjectID _instance_id;
	ScriptInstance *script_instance = nullptr;

	struct InstanceBinding {
		void *binding = nullptr;
		void *token = nullptr;
		GDExtensionInstanceBindingFreeCallback free_callback = nullptr;
		GDExtensionInstanceBindingReferenceCallback reference_callback = nullptr;
	};
	InstanceBinding *_instance_bindings = nullptr;
	uint32_t _instance_binding_count = 0;

#ifdef DEBUG_ENABLED
	SafeRefCount _lock_index;
#endif // DEBUG_ENABLED

	bool _predelete_ok : 1;

	HashMap<StringName, Variant> metadata;

	ObjectGDExtension *_extension = nullptr;
	GDExtensionClassInstancePtr _extension_instance = nullptr;

protected:
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const {}

public:
	_FORCE_INLINE_ ObjectID get_instance_id() const { return _instance_id; }
	_FORCE_INLINE_ bool is_ref_counted() const { return false; }

	bool is_class(const StringName &p_class) const;

	void get_property_list(List<PropertyInfo> *p_list, bool p_reversed = false) const;

	virtual ~LeanObject();
};
