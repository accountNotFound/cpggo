#include "../event.h"

namespace cppgo {

Event operator|(Event a, Event b) { return static_cast<Event>(static_cast<size_t>(a) | static_cast<size_t>(b)); }

}  // namespace cppgo
