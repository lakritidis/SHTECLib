#include "Token.h"

/// Default Constructor
Token::Token() : id(0), str(NULL), freq(0), type(0), sem(0), weight(0.0), next(NULL) {
}

/// Constructor 2
Token::Token(char * v, uint32_t t, uint16_t tp, uint16_t s) : id(t), str(NULL), freq(1), type(tp), sem(s),  weight(0.0), next(NULL) {
	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);
}

/// Destructor
Token::~Token(){
	if (this->str) {
		delete [] this->str;
	}
}

/// Display a Token object
inline void Token::display() {
	printf("Token ID: %d Literal: %s, Freq: %d, IDF Weight: %5.3f, Type: %d, Semantics: %d\n",
		this->id, this->str, this->freq, this->weight, this->type, this->sem);
}

/// Compute the Inverse Document Frequency (IDF) of the token
inline void Token::compute_weight(uint32_t N) {
	this->weight = log10( (_score_t)N / this->freq);
}

/// Accessors
inline uint32_t Token::get_id() { return this->id; }
inline char * Token::get_str() { return this->str; }
inline uint32_t Token::get_freq() { return this->freq; }
inline uint16_t Token::get_type() { return this->type; }
inline uint16_t Token::get_sem() { return this->sem; }
inline _score_t Token::get_weight() { return this->weight; }
inline Token* Token::get_next() { return this->next; }

/// Mutators
inline void Token::set_id(uint32_t v) { this->id = v; }
inline void Token::set_str(char *v) {
	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);
}
inline void Token::set_freq(uint32_t v) { this->freq = v; }
inline void Token::set_type(uint16_t v) { this->type = v; }
inline void Token::set_weight(_score_t v) { this->weight = v; }
inline void Token::set_sem(uint16_t v) { this->sem = v; }
inline void Token::set_next(class Token *v) { this->next = v; }
