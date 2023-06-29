#include "ink/render/resource.h"
#include "ink/core/assert.h"

using namespace ink;

// Prevent compiler from generating RTTI everywhere.
ink::GpuResource::~GpuResource() noexcept {}
