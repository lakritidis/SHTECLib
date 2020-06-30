#include "EditSimilarity.h"

/// Default Constructor
template <class T> EditSimilarity<T>::EditSimilarity() : ClusteringMethod<T>(0, "null", 0, 0) {
	this->num_entities = 0;
	this->num_alloc_entities = 0;
	this->e = NULL;
	this->lex = NULL;
}

/// Working Constructor
template <class T> EditSimilarity<T>::EditSimilarity(class Settings * p) : ClusteringMethod<T>(p) {
		this->num_entities = 0;
		this->num_alloc_entities = p->get_max_entities();
		this->e = new T * [this->num_alloc_entities]();
		this->lex = new TokensLexicon(1048576);
}

/// Destructor
template <class T> EditSimilarity<T>::~EditSimilarity() {
	if (this->num_entities > 0) {
		for (uint32_t i = 0; i < this->num_entities; i++) {
			delete this->e[i];
		}
		delete [] this->e;
	}

	if (this->lex) {
		delete this->lex;
	}
}

/// Min3
template <class T> uint32_t EditSimilarity<T>::min3 (uint32_t x, uint32_t y, uint32_t z) {
	return std::min(x, std::min(y,z));
}

/// Levenshtein Distance (Edit distance)
template <class T> uint32_t EditSimilarity<T>::levenshtein_dis(char * s_1, uint32_t l_1, char * s_2, uint32_t l_2) {
	uint32_t x, y, last, old, col[l_1 + 1];

	for (y = 1; y <= l_1; y++) {
		col[y] = y;
	}

	for (x = 1; x <= l_2; x++) {
		col[0] = x;
		for (y = 1, last = x - 1; y <= l_1; y++) {
			old = col[y];
			col[y] = this->min3(col[y] + 1, col[y - 1] + 1, last + (s_1[y - 1] == s_2[x - 1] ? 0 : 1));
			last = old;
		}
	}

	return(col[l_1]);
}

/// Levenshtein Similarity (Edit similarity)
template <class T> _score_t EditSimilarity<T>::levenshtein_sim(char * s_1, uint32_t l_1, char * s_2, uint32_t l_2) {
	return 1 - (_score_t)this->levenshtein_dis(s_1, l_1, s_2, l_2) / (_score_t)std::max(l_1, l_2);
}

/// Execute the algorithm
template <class T> void EditSimilarity<T>::exec() {
	uint32_t i = 0, j = 0, c = 0;
	_score_t sim = 0.0;
	T * e_1, * e_2;
	char t1[4096], t2[4096];

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		e_1 = this->e[i];
		t1[0] = 0;
		e_1->get_text(t1);
		c = strlen(t1);

		for (j = i; j < this->num_entities; j++) {
			e_2 = this->e[j];
			t2[0] = 0;
			e_2->get_text(t2);

			sim = this->levenshtein_sim(t1, c, t2, strlen(t2));
//			printf("%s (%d) - %s (%d)\n", t1, c, t2, strlen(t2)); getchar();
			this->insert_match(e_1->get_id(), e_2->get_id(), sim);
		}
	}
	this->finalize();
}

/// Specialization for Products : Execute the algorithm
template <> void EditSimilarity<Product>::exec() {
	uint32_t i = 0, j = 0;
	_score_t sim = 0.0;
	char t1[4096], t2[4096];

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				t1[0] = 0;
				t2[0] = 0;
				this->e[i]->get_text(t1);
				this->e[j]->get_text(t2);

				sim = this->levenshtein_sim(t1, strlen(t1), t2, strlen(t2));

				this->insert_match(this->e[i]->get_id(), this->e[j]->get_id(), sim);
			}
		}
	}

	this->finalize();
}
