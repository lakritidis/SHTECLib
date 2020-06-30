#include "Product.h"

/// Default Constructor
Product::Product() : Entity(), vendor_id(0) { }

/// Default Constructor
Product::Product(uint32_t s) : Entity (s), vendor_id(0) { }

/// Destructor
Product::~Product() { }

/// Mutators
inline void Product::set_vendor_id(uint32_t v) { this->vendor_id = v; }

/// Accessors
inline uint32_t Product::get_vendor_id() { return this->vendor_id; }
