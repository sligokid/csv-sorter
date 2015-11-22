/**
 * Wrapper around CSVLine object.
 * Contains field accessers, and make sure the fields are valid and parsed.
 * @class Filterable
 * @todo bypass calls like csvline->location_scheme() : use the direct value instead. Remove corresponding calls in CSVLineBase.
 */

/**
 * Implementation of Filterable class
 * @file Filterable.cc
 */

#include "headers/Filterable.h"
//#include "headers/Report.h"

Filterable::Filterable() {
}
Filterable::Filterable(const shared_ptr<CSVLine>& acsvline) : csvline(acsvline) {
}
