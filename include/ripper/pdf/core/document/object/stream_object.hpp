#pragma once

/// The stream_object class definition lives in object.hpp alongside
/// object, due to a mutual-recursion cycle (stream_object holds a
/// dictionary_object, whose unordered_map<object> needs object complete,
/// while object's variant holds unique_ptr<stream_object>).
///
/// This header exists for API symmetry — every PDF object type has a
/// *_object.hpp entry point.

#include "ripper/pdf/core/document/object/object.hpp"
