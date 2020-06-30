#ifndef CLUSTERINGMETHOD_H
#define CLUSTERINGMETHOD_H

template <class T> class ClusteringMethod {
	protected:
		uint32_t id;
		class Settings * PARAMS;
		class TokensLexicon * lex;
		class TokensLexicon * stoplist;
		class Statistics * stats;

		uint32_t num_entities;
		uint32_t num_alloc_entities;
		T ** e;

		uint32_t num_vendors;
		uint32_t * v_ids;

		class ClusterUniverse<T> * eval_clusters;

		struct matches {
			uint32_t num_alloc_matches;
			uint32_t num_matches;
			struct match ** matches_array;
		} ** csim_matches;

		FILE * output_files[9];

		class EvaluationMeasures evaluators[10];

	protected:
		uint32_t factorial(uint32_t, uint32_t);

		static int cmp_matches(const void * a, const void * b) {
			struct match *x = *(struct match **)a;
			struct match *y = *(struct match **)b;

			if (x->e1_id > y->e1_id) return 1;
			if (x->e1_id == y->e1_id) { return x->e2_id - y->e2_id; }
			if (x->e1_id < y->e1_id) return -1;

			return 0;
		}

		static int cmp_int(const void * a, const void * b) {
			uint32_t x = * (uint32_t *)a;
			uint32_t y = * (uint32_t *)b;
			return x - y;
		}

	public:
		ClusteringMethod(class Settings *);
		~ClusteringMethod();

		void read_from_file(char *);
		void insert_entity_vector( );
		void insert_match(uint32_t, uint32_t, _score_t);
		void initialize();
		void finalize();
		void evaluate_f1();
		double evaluate_f1(uint32_t);

		uint32_t get_id();
		char *get_name();
};

#endif // CLUSTERINGMETHOD_H
