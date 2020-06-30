#include "Entity.h"

/// Default constructor
Entity::Entity() :
	id(0), type(0), weight(0.0), real_cluster_id(0), num_tokens(0), tokens(NULL), token_weights(NULL) {
}

/// Default constructor 2
Entity::Entity(uint16_t s) :
	id(0), type(0), weight(0.0), real_cluster_id(0), num_tokens(0), tokens(NULL), token_weights(NULL) {
		this->tokens = (class Token **)malloc(s * sizeof(class Token *));
		this->token_weights = (_score_t *)malloc(s * sizeof(_score_t));
}

/// Destructor
Entity::~Entity() {
	if (this->tokens) {
		free(this->tokens);
	}

	if (this->token_weights) {
		free(this->token_weights);
	}
}

/// Insert a token of the entity into the lexicon and the local temp storage of tokens.
void Entity::insert_token(uint32_t l, char * t, TokensLexicon * lx, TokensLexicon * sw, uint16_t * s) {
	uint16_t token_type = 0, token_sem = 0;

	if (l < 1) {
		return;
	}

	/// Ignore this token if it is a stopword
	if (sw) {
		if (sw->search(t)) {
			return;
		}
	}

	/// Check whether the token has been encountered before in the entity and increase its frequency
	for (uint32_t i = 0; i < this->num_tokens; i++) {
		if (strcmp(this->tokens[i]->get_str(), t) == 0) {
			this->token_weights[i]++;
			return;
		}
	}

	/// Insert the token into the lexicon
	this->tokens[this->num_tokens] = lx->insert(t, token_type, token_sem);
	this->token_weights[this->num_tokens] = 1.0;
	this->num_tokens++;

	/// If the available space of the array/s of tokens is exhausted, provide more space.
	if (this->num_tokens >= (*s)) {
		(*s) *= 2;
		this->tokens = (class Token **)realloc(this->tokens, *s * sizeof(class Token *));
		this->token_weights = (_score_t *)realloc(this->token_weights, *s * sizeof(_score_t));
	}
}

/// Overloaded Insert a token of the entity into the lexicon and the local temp storage of tokens.
void Entity::insert_token(class Token * t, _score_t w) {
	this->tokens[this->num_tokens] = t;
	this->token_weights[this->num_tokens] = w;
	this->num_tokens++;
/*
	/// If the available space of the array/s of tokens is exhausted, provide more space.
	if (this->num_tokens >= (*s)) {
		(*s) *= 2;
		this->tokens = (class Token **)realloc(this->tokens, *s * sizeof(class Token *));
		this->token_weights = (_score_t *)realloc(this->token_weights, *s * sizeof(_score_t));
	}
*/
}

/// Tokenize a string: Split the string in its component words, store the words in an array and
/// sort the array in lexicographical order.
uint32_t Entity::tokenize(char * t, uint32_t nchars, TokensLexicon * lex, TokensLexicon * sw, Statistics * stats) {

	uint16_t alloc_tok = 8;
	uint32_t x = 0, y = 0, i = 0, split = 0;
	char buf[1024], buf2[1024];

	this->tokens = (class Token **)malloc(alloc_tok * sizeof(class Token *));
	this->token_weights = (_score_t *)malloc(alloc_tok * sizeof(_score_t));

	for (i = 0; i < nchars; i++) {
		if (t[i] == '/' || t[i] == '-') {
			buf[x++] = t[i];
			buf2[y] = 0;
			this->insert_token(y, buf2, lex, sw, &alloc_tok);
			y = 0;
			split = 1;

		} else if (t[i] == ' ') {
			buf[x] = 0;
			this->insert_token(x, buf, lex, sw, &alloc_tok);

			if (split == 1) {
				buf2[y] = 0;
				this->insert_token(y, buf2, lex, sw, &alloc_tok);
				split = 0;
			}
			x = 0;
			y = 0;

		} else {
			/// Case folding
			if (t[i] >= 65 && t[i] <= 90) {
				buf[x++] = t[i] + 32;
				buf2[y++] = t[i] + 32;
			} else {
				buf[x++] = t[i];
				buf2[y++] = t[i];
			}
		}
	}
	buf[x] = 0;
	this->insert_token(x, buf, lex, sw, &alloc_tok);

	/// Reallocate this space to occupy exactly the required memory
	if (this->num_tokens > 0) {
		this->tokens = (class Token **)realloc(this->tokens, this->num_tokens * sizeof(class Token *));
		this->token_weights = (_score_t *)realloc(this->token_weights, this->num_tokens * sizeof(_score_t));

		/// Sort the tokens in ascending ID order to allow fast comparisons
		this->sort_feature_vector();
	}

	if (stats) {
		stats->update_titles(this->num_tokens);
	}

	return this->num_tokens;
/*
	printf("\n========================\nTitle: %s --- Tokens: %d\n\n", t, this->num_tokens);
	for (i = 0; i < this->num_tokens; i++) {
		printf("\t"); this->tokens[i]->display();
	}
	printf("=======================================\n\n");
	getchar();
*/
}

/// Cosine Similarity between two entities: This is reused in all clustering algorithms
_score_t Entity::cosine_similarity(class Entity * e_2) {
	uint16_t it_1 = 0, it_2 = 0, n1 = this->num_tokens, n2 = e_2->get_num_tokens();
	class Token * t_1, * t_2;
	_score_t idf_i = 0.0;

	/// The tokens are sorted in increasing token ID order, hence, the computation of their
	/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
	t_1 = this->tokens[0];
	t_2 = e_2->get_token(0);
	while(it_1 < n1 && it_2 < n2) {
//		printf("\t\t%s (%5.3f) == %s (%5.3f)\n",
//			t_1->get_str(), this->token_weights[it_1], t_2->get_str(), e_2->get_token_weight(it_2));
		if (t_1->get_id() == t_2->get_id()) {
			idf_i += this->token_weights[it_1] * e_2->get_token_weight(it_2);

//			printf("\t\tCommon Token: %s (Int-IDF: %5.3f)\n", t_1->get_str(), idf_i);
			it_1++;
			it_2++;
			if (it_1 < n1) { t_1 = this->tokens[it_1]; }
			if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
		} else if (t_1->get_id() < t_2->get_id()) {
			it_1++;
			if (it_1 < n1) { t_1 = this->tokens[it_1]; }
		} else {
			it_2++;
			if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
		}
	}
//	printf("Total IDF-I: %5.3f\n", idf_i); getchar();
	return idf_i / (this->weight * e_2->get_weight());
}

/// Euclidean Distance between two entities
_score_t Entity::euclidean_distance(class Entity * e_2) {
	uint16_t it_1 = 0, it_2 = 0, n1 = this->num_tokens, n2 = e_2->get_num_tokens();
	class Token * t_1, * t_2;
	_score_t dis = 0.0;

	if (e_2->get_num_tokens() == 0) {
		return INIT_DISTANCE;
	}

	/// The tokens are sorted in increasing token ID order, hence, the computation of their
	/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
	t_1 = this->tokens[0];
	t_2 = e_2->get_token(0);
	while(1) {
//		printf("\t\t%d::%s (%d/%d) == %d::%s (%d/%d) == Dis: ", t_1->get_id(), t_1->get_str(),
//			it_1, n1, t_2->get_id(), t_2->get_str(), it_2, n2, dis);
		if (t_1->get_id() == t_2->get_id()) {
			dis += (this->token_weights[it_1] - e_2->get_token_weight(it_2)) *
				(this->token_weights[it_1] - e_2->get_token_weight(it_2));

			it_1++;
			it_2++;
			if (it_1 < n1) { t_1 = this->tokens[it_1]; }
			if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
			if (it_1 >= n1 && it_2 >= n2) { break; }
		} else if (t_1->get_id() < t_2->get_id()) {
			if (it_1 < n1) {
				dis += this->token_weights[it_1] * this->token_weights[it_1];
				it_1++;
				if (it_1 < n1) { t_1 = this->tokens[it_1]; }
			} else {
				if (it_2 < n2) {
					dis += e_2->get_token_weight(it_2) * e_2->get_token_weight(it_2);
					it_2++;
					if (it_2 < n2) {t_2 = e_2->get_token(it_2); }
				} else {
					break;
				}
			}
		} else {
			if (it_2 < n2) {
				dis += e_2->get_token_weight(it_2) * e_2->get_token_weight(it_2);
				it_2++;
				if (it_2 < n2) { t_2 = e_2->get_token(it_2); }
			} else {
				if (it_1 < n1) {
					dis += this->token_weights[it_1] * this->token_weights[it_1];
					it_1++;
					if (it_1 < n1) {t_1 = this->tokens[it_1]; }
				} else {
					break;
				}
			}
		}
//		printf("%5.3f\n", dis);
	}
	dis = sqrt(dis);
	return dis;
}

/// Score the tokens be either tf-idf, or plain idf scoring method.
void Entity::compute_token_scores() {
	_score_t wght = 0.0;
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		wght = this->tokens[i]->get_weight();

		/// Here we update the weights in the feature vector.
		/// idf-only scoring
//		this->token_weights[i] = wght;

		/// tf-idf scoring
		this->token_weights[i] = this->token_weights[i] * wght;
	}
}

/// This function 1: updates the weights in the feature vector, and 2: computes the weight of the
/// entire entity as the sqrt of the squared sums of the weights of the feature vector.
void Entity::compute_entity_weight() {
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		this->weight += this->token_weights[i] * this->token_weights[i];
	}
	this->weight = sqrt(this->weight);
}

/// Normalize the weights given an integer sum (yhis is used in case the entity is a centroid).
void Entity::divide_weights(double pop) {
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		this->token_weights[i] /= pop;
	}
}

/// Normalize the feature vector and turn it into a unit vector.
void Entity::normalize_feature_vector() {
	_score_t weight_sum = 0.0;
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		weight_sum += this->token_weights[i] * this->token_weights[i];
	}
	weight_sum = sqrt(weight_sum);

	for (uint16_t i = 0; i < this->num_tokens; i++) {
		this->token_weights[i] /= weight_sum;
	}
}

/// Sort the feature vector in increasing token ID order.  Unfortunately qsort does not do the job,
/// because if  we sort the the tokens  by their ID,  then we also need to sort the weights of the
/// feature vector accordingly. This is impossible with qsort so we use a custom QuickSort version.
void Entity::sort_feature_vector() {
	this->quicksort(0, this->num_tokens - 1);
}

/// Display the Entity's attributes
void Entity::display() {
	char title[1024] = { 0 };
	this->get_text(title);

	printf("ID: %d, Text: %s, Weight: %5.3f, Length: %d, Real_Cluster: %d\n",
			this->id, title, this->weight, this->num_tokens, this->real_cluster_id);
}

/// Display the Entity's feature vector
void Entity::display_feature_vector() {
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		printf("(%d, %5.3f) ", this->tokens[i]->get_id(), this->token_weights[i]);
	}
	printf("\n");
}

/// Accessors
inline uint32_t Entity::get_id() { return this->id; }
inline void Entity::get_text(char * title) {
	uint32_t x = 0;
	for (uint16_t i = 0; i < this->num_tokens; i++) {
		x = strlen(title);
		if (x + strlen(this->tokens[i]->get_str()) + 1 < 1024) {
			sprintf(title + x, "%s ", this->tokens[i]->get_str());
		}
	}
	title[strlen(title) - 1] = 0;
}

uint16_t Entity::get_num_tokens() { return this->num_tokens; }
uint32_t Entity::get_real_cluster_id() { return this->real_cluster_id; }
uint16_t Entity::get_type() { return this->type; }
Token * Entity::get_token(uint16_t i) { return this->tokens[i]; }
_score_t Entity::get_token_weight(uint16_t i) { return this->token_weights[i]; }
_score_t Entity::get_weight() { return this->weight; }

/// Mutators
void Entity::set_id(uint32_t v) { this->id = v; }
void Entity::set_type(uint16_t v) { this->type = v; }
void Entity::set_num_tokens(uint16_t v) { this->num_tokens = v; }
void Entity::set_real_cluster_id(uint32_t v) { this->real_cluster_id = v; }

/// QuickSort private function for sorting the entire feature vector (i.e., both the tokens and the
/// weights).
void Entity::quicksort(uint16_t first, uint16_t last) {
	uint16_t i = first, j = last, pivot = 0;
	class Token * temp_token = 0;
	_score_t temp_weight = 0.0;

	if (first < last) {
		pivot = last;
		i = first;
		j = last;

		while (i < j) {
			while (this->tokens[i]->get_id() <= this->tokens[pivot]->get_id() && i < last) {
				i++;
			}
			while (this->tokens[j]->get_id() > this->tokens[pivot]->get_id()) {
				j--;
			}

			if (i < j) {
				temp_token = this->tokens[i];
				this->tokens[i] = this->tokens[j];
				this->tokens[j] = temp_token;

				temp_weight = this->token_weights[i];
				this->token_weights[i] = this->token_weights[j];
				this->token_weights[j] = temp_weight;
			}
		}

		temp_token = this->tokens[pivot];
		this->tokens[pivot] = this->tokens[j];
		this->tokens[j] = temp_token;

		temp_weight = this->token_weights[pivot];
		this->token_weights[pivot] = this->token_weights[j];
		this->token_weights[j] = temp_weight;

		quicksort(first, j - 1);
		quicksort(j + 1, last);
	}
}
