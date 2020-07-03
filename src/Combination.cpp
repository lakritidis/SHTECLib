#include "Combination.h"

/// Default Constructor
Combination::Combination() :
	signature(NULL), num_tokens(0), freq(0), dist_acc(0.0), next(NULL) {
}

/// Constructor 2
Combination::Combination(char * v, _score_t dis, uint16_t nt) :
	signature(NULL), num_tokens(nt), freq(1), dist_acc(dis), next(NULL) {

		this->signature = new char[strlen(v) + 1];
		strcpy(this->signature, v);
}

/// Destructor
Combination::~Combination(){
	if (this->signature) {
		delete [] this->signature;
	}
}

/// Display a k-Combination object
inline void Combination::display(class TokensLexicon *h, char *t) {
	if (!h && !t) {
		printf("Cluster: %s, Words: %d, Frequency: %d, Avg Distance: %5.3f",
			this->signature, this->num_tokens, this->freq, this->dist_acc / this->freq);
	} else if (h && t) {
		printf("Title: %s - Cluster: %s, Words: %d, Frequency: %d, Avg Distance: %5.3f",
			t, this->signature, this->num_tokens, this->freq, this->dist_acc / this->freq);
	}
}


/// Compute the score of the k-Combination
_score_t Combination::compute_score(class Settings * sets, class Statistics * stats, class Token ** tindex) {

	char * s = this->signature, buf[10];
	uint32_t i = 0, x = 0, y = 0, id = 0, zone_lengths[NUM_ZONES] = { 0 }, w = 0, l = strlen(s);
	_score_t ir_score = 0.0, token_weight = 0.0, zone_weight = 0.0, denom = 0.0;

	/// Retrieve the tokens of the k-Combination
	class Token * toks [ this->num_tokens ];
	for (i = 0; i < l; i++) {
		if (s[i] == ' ') {
			buf[x] = 0;
			x = 0;
			id = atoi(buf);
			toks[y++] = tindex[id];
		} else {
			buf[x++] = s[i];
		}
	}
	buf[x] = 0;
	id = atoi(buf);
	toks[y++] = tindex[id];

	/// Compute zone lengths for this k-Combination
	for (i = 0; i < this->num_tokens; i++) {
		zone_lengths [ toks[i]->get_sem() ]++;
	}

//	for (i = 0; i < this->num_tokens; i++) { printf("%s ", toks[i]->get_str()); } printf("\n");
//	for (i = 0; i < NUM_ZONES; i++) { printf("Zone %d: %d\n", i, zone_lengths[i]); }
//	getchar();

	/// IR Score
	for (i = 0; i < this->num_tokens; i++) {
		w = toks[i]->get_sem();

//		zone_weight = (1.00 / stats->get_avg_zone_tokens(w)) * (this->num_tokens / zone_lengths[w]);
//		zone_weight = 1.0 / zone_lengths[w];
		zone_weight = log10(sets->get_upm_k() / zone_lengths[w]);

//		denom = 1.00 * (1.00 - sets->get_upm_b() + sets->get_upm_b() * zone_lengths[w] / stats->get_avg_zone_tokens(w)) + zone_weight;
		denom = 1.00 - sets->get_upm_b() + sets->get_upm_b() * this->num_tokens;

		token_weight = zone_weight / denom;

		ir_score += token_weight * toks[i]->get_weight();
//		printf("Zone Weight: %5.3f T-score : %5.3f\n", zone_weight, t_score); getchar();
	}

	/// Average Distance form the beginning of the titles
	_score_t avg_dist = this->dist_acc / this->freq;

	_score_t score =
		(this->num_tokens / (sets->get_upm_a() + avg_dist)) *	/// Distance
		pow(ir_score, 2) *							/// IR score
		log10 (this->freq);							/// Frequency

//	printf("Combination: %s\nFrequency: %d, Average Distance: %5.3f, IR Score: %5.3f - Score: %5.3f\n",
//		this->signature, this->freq, avg_dist, t_score, score); getchar();

	return score;
}

/// Compute the score of the k-Combination
_score_t Combination::compute_score_2(class Settings * sets, class Statistics * stats, class Token ** tindex) {

	char * s = this->signature, buf[10];
	uint32_t i = 0, x = 0, y = 0, id = 0, l = strlen(s), upm_k = sets->get_upm_k();
	_score_t ir_score = 0.0, n_score = 0.0, token_weight = 0.0;

	/// Retrieve the tokens of the k-Combination
	class Token * toks [ this->num_tokens ];
	for (i = 0; i < l; i++) {
		if (s[i] == ' ') {
			buf[x] = 0;
			x = 0;
			id = atoi(buf);
			toks[y++] = tindex[id];
		} else {
			buf[x++] = s[i];
		}
	}
	buf[x] = 0;
	id = atoi(buf);
	toks[y++] = tindex[id];

	/// IR Score
	for (i = 0; i < this->num_tokens; i++) {
		token_weight = 1.0;

		ir_score += token_weight * toks[i]->get_weight();

		n_score += pow(token_weight * toks[i]->get_weight(), upm_k);
//		n_score += token_weight * toks[i]->get_weight();
//		printf("T-score : %5.3f\n", toks[i]->get_weight()); getchar();
	}

	_score_t score = n_score * this->freq / this->num_tokens;
//	_score_t score = n_score * log10(this->freq) / (double)this->num_tokens;
//	printf("Combination: %s\nIR Score: %5.3f - Score: %5.3f\n", this->signature, ir_score, score); getchar();

	return score;
}

/// Increase the frequency of the Combination by a defined value
inline void Combination::increase_frequency(uint32_t v) { this->freq += v; }

/// Add a new distance value to the distances accumulator
inline void Combination::increase_dist_acc(_score_t v) { this->dist_acc += v; }

/// Accessors
inline char * Combination::get_signature() { return this->signature; }
inline uint32_t Combination::get_freq() { return this->freq; }
inline uint16_t Combination::get_num_tokens() { return this->num_tokens; }
inline _score_t Combination::get_dist_acc() { return this->dist_acc; }
inline _score_t Combination::get_avg_dist() { return this->dist_acc / this->freq; }
inline Combination* Combination::get_next() { return this->next; }

/// Mutators
inline void Combination::set_signature(char *v) {
	this->signature = new char[strlen(v) + 1];
	strcpy(this->signature, v);
}
inline void Combination::set_freq(uint32_t v) { this->freq = v; }
inline void Combination::set_dist_acc(_score_t v) { this->dist_acc = v; }
inline void Combination::set_next(class Combination * v) { this->next = v; }
