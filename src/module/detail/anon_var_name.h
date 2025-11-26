#pragma once

#define _MODULE_DETAIL_CONCAT2(x, y) x##y
#define _MODULE_DETAIL_CONCAT(x, y) _MODULE_DETAIL_CONCAT2(x, y)

// expands to name<some unique number>
#define MODULE_ANON_VAR(name) _MODULE_DETAIL_CONCAT(name, __COUNTER__)
