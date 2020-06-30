#include "Feature.h"

/// Default Constructor
Feature::Feature() : t (NULL), w(0.0) { }

/// Working Constructor
Feature::Feature(class Token * token, _score_t weight) : t(token), w(weight) { }

/// Destructor
Feature::~Feature() { }

/// Accessors
uint32_t Feature::get_id() { return this->t->get_id(); }
class Token * Feature::get_token() { return this->t; }
_score_t Feature::get_weight() { return this->w; }

/// Mutators
void Feature::set_token(class Token * v) { this->t = v; }
void Feature::set_weight(_score_t v) { this->w = v; }
