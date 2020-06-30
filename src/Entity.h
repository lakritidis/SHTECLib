#ifndef ENTITY_H
#define ENTITY_H

class Entity {
	protected:
        uint32_t id;

		uint16_t type;

		_score_t weight;

		uint32_t real_cluster_id;

		/// The following three elements constitute the Entity's Feature Vector. We implement it in
		/// this way and not as a separate class,  in order to avoid an additional level of pointer
		/// dereferencing.  Compared to  the usage  of an  array of  Feature objects, this approach
		/// accelerates the computation of similarity (or distance) between two entities by 20-30%.
		uint16_t num_tokens;
        class Token ** tokens;
		_score_t * token_weights;

	protected:
		void quicksort(uint16_t, uint16_t);

	public:
		Entity();
		Entity(uint16_t);
		~Entity();

		_score_t cosine_similarity(class Entity *);
		_score_t euclidean_distance(class Entity *);
		uint32_t tokenize(char *, uint32_t, class TokensLexicon *, class TokensLexicon *, class Statistics *);
		void insert_token(uint32_t, char *, class TokensLexicon *, class TokensLexicon *, uint16_t *);
		void insert_token(class Token *, _score_t);
		void display();
		void display_feature_vector();

		void compute_token_scores();
		void compute_entity_weight();
		void divide_weights(double);
		void normalize_feature_vector();
		void sort_feature_vector();

		void set_id(uint32_t);
		void set_type(uint16_t);
		void set_num_tokens(uint16_t);
		void set_real_cluster_id(uint32_t);

		uint32_t get_id();
		void get_text(char * );
		uint16_t get_type();
		uint16_t get_num_tokens();
		uint32_t get_real_cluster_id();
		Token * get_token(uint16_t);
		_score_t get_token_weight(uint16_t);
		_score_t get_weight();
};

#endif // ENTITY_H
