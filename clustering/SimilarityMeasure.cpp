#include "SimilarityMeasure.h"

/// Default Constructor
template <class T> SimilarityMeasure<T>::SimilarityMeasure() : ClusteringMethod<T>(0, "null") {
	this->num_entities = 0;
	this->num_alloc_entities = 0;
	this->e = NULL;
	this->lex = NULL;
}

/// Working Constructor 1
template <class T> SimilarityMeasure<T>::SimilarityMeasure(uint32_t n, uint32_t d, const char * s, uint32_t l, uint32_t h) :
	ClusteringMethod<T>(d, s, l, h) {
		this->num_entities = 0;
		this->num_alloc_entities = n;
		this->e = new T * [this->num_alloc_entities]();
		this->lex = new TokensLexicon(1048576);
}

/// Working Constructor 2
template <class T> SimilarityMeasure<T>::SimilarityMeasure(Settings * p) : ClusteringMethod<T>(p) {
	this->num_entities = 0;
	this->num_alloc_entities = p->get_max_entities();
	this->e = new T * [this->num_alloc_entities]();
	this->lex = new TokensLexicon(1048576);
}

/// Destructor
template <class T> SimilarityMeasure<T>::~SimilarityMeasure() {
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

/// Execution Driver : Reroute execution according to the called metric
template <class T> void SimilarityMeasure<T>::exec() {
	if (this->id == 1 || this->id == 2 || this->id == 3) {
		this->exec_baseline();
	} else if (this->id == 5 || this->id == 6 || this->id == 7) {
		this->exec_weighted();
	}
}


/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// BASELINE STRING SIMILARITY METRICS
/// ///////////////////////////////////////////////////////////////////////////////////////////////

/// General template function : Compute the value of a baseline string similarity metric.
template <class T> void SimilarityMeasure<T>::exec_baseline() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, n1 = 0, n2 = 0, t1 = 0, t2 = 0, common_tokens = 0;
	_score_t sim = 0.0;
	T * e_1, * e_2;

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			it_1 = 0; it_2 = 0;
			common_tokens = 0;
			e_1 = this->e[i];
			e_2 = this->e[j];

			n1 = e_1->get_num_tokens();
			n2 = e_2->get_num_tokens();

			/// The tokens are sorted in lexicographical order, hence, the computation of their
			/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
			t1 = e_1->get_token(0)->get_id();
			t2 = e_2->get_token(0)->get_id();
			while(it_1 < n1 && it_2 < n2) {
				if (t1 == t2) {
					common_tokens++;
					it_1++;
					it_2++;
					if (it_1 < n1) { t1 = e_1->get_token(it_1)->get_id(); }
					if (it_2 < n2) { t2 = e_2->get_token(it_2)->get_id(); }

				} else if (t1 < t2) {
					it_1++;
					if (it_1 < n1) { t1 = e_1->get_token(it_1)->get_id(); }
				} else {
					it_2++;
					if (it_2 < n2) { t2 = e_2->get_token(it_2)->get_id(); }
				}
			}

			/// Cosine Similarity
			if (this->id == 1) {
				sim = (_score_t)common_tokens / ( sqrt(e_1->get_num_tokens()) * sqrt(e_2->get_num_tokens()) );

			/// Jaccard Index
			} else if (this->id == 2) {
				sim = (_score_t)common_tokens / ( e_1->get_num_tokens() + e_2->get_num_tokens() - common_tokens );

			/// Dice Coefficient
			} else if (this->id == 3) {
				sim = 2.00 * (_score_t)common_tokens / ( e_1->get_num_tokens() + e_2->get_num_tokens() );
			}

			this->insert_match(e_1->get_id(), e_2->get_id(), sim);
		}
	}
	this->finalize();
}

/// Specialization for products : Compute the value of a baseline string similarity metric.
template <> void SimilarityMeasure<Product>::exec_baseline() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, n1 = 0, n2 = 0, t1 = 0, t2 = 0, common_tokens = 0;
	_score_t sim = 0.0;
	class Product * e_1, * e_2;

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two products originate from the same vendor, do not compare them, but
			/// consider them as non-matching
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				common_tokens = 0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				n1 = e_1->get_num_tokens();
				n2 = e_2->get_num_tokens();

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				t1 = e_1->get_token(0)->get_id();
				t2 = e_2->get_token(0)->get_id();
				while(it_1 < n1 && it_2 < n2) {
					if (t1 == t2) {
						common_tokens++;
						it_1++;
						it_2++;
						if (it_1 < n1) { t1 = e_1->get_token(it_1)->get_id(); }
						if (it_2 < n2) { t2 = e_2->get_token(it_2)->get_id(); }

					} else if (t1 < t2) {
						it_1++;
						if (it_1 < n1) { t1 = e_1->get_token(it_1)->get_id(); }
					} else {
						it_2++;
						if (it_2 < n2) { t2 = e_2->get_token(it_2)->get_id(); }
					}
				}

				/// Cosine Similarity
				if (this->id == 1) {
					sim = (_score_t)common_tokens / ( sqrt(e_1->get_num_tokens()) * sqrt(e_2->get_num_tokens()) );

				/// Jaccard Index
				} else if (this->id == 2) {
					sim = (_score_t)common_tokens / ( e_1->get_num_tokens() + e_2->get_num_tokens() - common_tokens );

				/// Dice Coefficient
				} else if (this->id == 3) {
					sim = 2.00 * (_score_t)common_tokens / ( e_1->get_num_tokens() + e_2->get_num_tokens() );
				}

				this->insert_match(e_1->get_id(), e_2->get_id(), sim);
			}
		}
	}

	this->finalize();
}


/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// WEIGHTED STRING SIMILARITY METRICS : REPLACE PLAIN TOKEN COUNTS BY IDF-BASED TOKEN WEIGHTS
/// ///////////////////////////////////////////////////////////////////////////////////////////////

/// General template function : Compute the value of a weighted string similarity metric.
template <class T> void SimilarityMeasure<T>::exec_weighted() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, n1 = 0, n2 = 0;
	_score_t sim = 0.0, idf_i = 0.0;
	class Token * t_1, * t_2;
	T * e_1, * e_2;

	this->initialize();

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset()); fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			it_1 = 0; it_2 = 0;
			idf_i = 0.0;
			e_1 = this->e[i];
			e_2 = this->e[j];

			n1 = e_1->get_num_tokens();
			n2 = e_2->get_num_tokens();

			/// The tokens are sorted in lexicographical order, hence, the computation of their
			/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
			t_1 = e_1->get_token(0);
			t_2 = e_2->get_token(0);
			while(it_1 < n1 && it_2 < n2) {
//				printf("\t\t%s == %s\n", t_1->get_str(), t_2->get_str());
				if (t_1->get_id() == t_2->get_id()) {
					idf_i += t_1->get_weight() * t_1->get_weight();
//					printf("\t\tCommon Token: %s (IDF: %5.3f)\n", t_1->get_str(), idf_i);
					it_1++;
					it_2++;
					if (it_1 < n1) { t_1 = e_1->get_token(it_1); }
					if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
				} else if (t_1->get_id() < t_2->get_id()) {
					it_1++;
					if (it_1 < n1) { t_1 = e_1->get_token(it_1); }
				} else {
					it_2++;
					if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
				}
			}

			/// Recall : e->get_entity() returns the sqrt of the weight of the entity
			/// Weighted Cosine Similarity
			if (this->id == 5) {
				sim = idf_i / (e_1->get_weight() * e_2->get_weight());

			/// Weighted Jaccard Index
			} else if (this->id == 6) {
				sim = idf_i / ( e_1->get_weight() * e_1->get_weight() + e_2->get_weight() * e_2->get_weight() - idf_i);

			/// Weighted Dice Coefficient
			} else if (this->id == 7) {
				sim = 2.00 * idf_i / ( e_1->get_weight() * e_1->get_weight() + e_2->get_weight() * e_2->get_weight() );
			}


			this->insert_match(e_1->get_id(), e_2->get_id(), sim);
		}
	}
	this->finalize();
}


/// Specialization for products : Compute the value of a weighted string similarity metric.
template <> void SimilarityMeasure<Product>::exec_weighted() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, n1 = 0, n2 = 0;
	_score_t sim = 0.0, idf_i = 0.0;
	class Token * t_1, * t_2;
	Product * e_1, * e_2;

	this->initialize();

	printf("Running '%s' on '%s'...", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;

			} else {

				it_1 = 0; it_2 = 0;
				idf_i = 0.0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				n1 = e_1->get_num_tokens();
				n2 = e_2->get_num_tokens();

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				t_1 = e_1->get_token(0);
				t_2 = e_2->get_token(0);
				while(it_1 < n1 && it_2 < n2) {
	//				printf("\t\t%s == %s\n", t_1->get_str(), t_2->get_str());
					if (t_1->get_id() == t_2->get_id()) {
						idf_i += t_1->get_weight() * t_1->get_weight();
	//					printf("\t\tCommon Token: %s (IDF: %5.3f)\n", t_1->get_str(), idf_i);
						it_1++;
						it_2++;
						if (it_1 < n1) { t_1 = e_1->get_token(it_1); }
						if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
					} else if (t_1->get_id() < t_2->get_id()) {
						it_1++;
						if (it_1 < n1) { t_1 = e_1->get_token(it_1); }
					} else {
						it_2++;
						if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
					}
				}

				/// Recall : e->get_entity() returns the sqrt of the weight of the entity
				/// Weighted Cosine Similarity
				if (this->id == 5) {
					sim = idf_i / (e_1->get_weight() * e_2->get_weight());

				/// Weighted Jaccard Index
				} else if (this->id == 6) {
					sim = idf_i / ( e_1->get_weight() * e_1->get_weight() + e_2->get_weight() * e_2->get_weight() - idf_i);

				/// Weighted Dice Coefficient
				} else if (this->id == 7) {
					sim = 2.00 * idf_i / ( e_1->get_weight() * e_1->get_weight() + e_2->get_weight() * e_2->get_weight() );
				}

				this->insert_match(e_1->get_id(), e_2->get_id(), sim);
			}
		}
	}

	this->finalize();
}
