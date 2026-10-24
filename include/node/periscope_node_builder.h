#pragma once

// Historically node-only; fluent create now lives in object/periscope_object_builder.h
// (shared object_builder<node|note|class_def, …>). Prefer including periscope_graph.h /
// periscope.h — this header remains as a stable redirect.
#include "object/periscope_object_builder.h"
