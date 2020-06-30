#ifndef CLUSTER_H
#define CLUSTER_H

struct match {
	uint32_t e1_id;
	uint32_t e2_id;
};

/// PRIMARY TEMPLATE
/// T: what do we cluster (Entities, Products, etc)
template <class T> class Cluster {
	private:
		uint32_t id;
		char * label;
		uint32_t num_entities;
		uint32_t num_alloc_entities;
		T ** entities;
		T * representative;
		class TokensLexicon * tokens_distr;

	private:
		Cluster(const Cluster&);

		static int cmp_features(const void * a, const void * b) {
			class Feature x = * (class Feature *)a;
			class Feature y = * (class Feature *)b;
			return x.get_token()->get_id() - y.get_token()->get_id();
		}

	public:
		Cluster(uint32_t);
		~Cluster();

		uint32_t insert_entity(T *);
		uint32_t insert_entity(T *, bool);
		void delete_entity(uint32_t);

		void compute_clustroid();
		void compute_centroid(uint16_t);
		_score_t compute_linkage(class Cluster<T> *, uint32_t);
		void merge_with(class Cluster<T> *);
		void empty(bool);

		uint32_t create_pairwise_matches(FILE *);
		double compute_entropy(uint32_t);
		double compute_conditional_entropy(uint32_t);
		void display();

		uint32_t get_id();
		char * get_label();
		uint32_t get_num_entities();
		uint32_t get_non_deleted_entities();
		T * get_entity(uint32_t);
		T * get_representative();
		class TokensLexicon * get_token_distribution();

		void set_id(uint32_t);
		void set_label(char *);
		void set_representative(T *);
};


/// SPECIALIZED TEMPLATE
/// T: what do we cluster (Entities, Products, etc)
/// EntitiesGroup<T>: The T entities are stored in groups within the cluster
template <class T> class Cluster< EntitiesGroup<T> > {
	private:
		uint32_t id;
		char * label;
		uint32_t num_entities;
		uint32_t num_groups;
		uint32_t num_alloc_groups;
		EntitiesGroup<T> ** groups;
		T * representative;
		class TokensLexicon * tokens_distr;

	private:
		Cluster(const Cluster&);

        static int cmp_groups (const void * a, const void * b) {
			class EntitiesGroup<T> * x = *(class EntitiesGroup<T> **)a;
			class EntitiesGroup<T> * y = *(class EntitiesGroup<T> **)b;

			if (x->get_group_key() > y->get_group_key()) {
				return 1;
			}
			return -1;
		}

		static int cmp_features(const void * a, const void * b) {
			class Feature x = * (class Feature *)a;
			class Feature y = * (class Feature *)b;
			return x.get_token()->get_id() - y.get_token()->get_id();
		}

	public:
		Cluster(uint32_t);
		~Cluster();

		uint32_t insert_entity(T *);
		uint32_t insert_entity(T *, bool);
		int32_t binsearch_group(uint32_t, int32_t, int32_t);
		uint32_t insert_entity_sorted(T *);
		void delete_entity(uint32_t);
		void delete_entity(uint32_t, uint32_t);

		void prepare(uint16_t);
		void compute_clustroid();
		void compute_centroid(uint16_t);
		_score_t compute_linkage(class Cluster< EntitiesGroup<T> > *, uint32_t);
		void create_empty_groups(uint32_t, uint32_t *);
		void merge_with(class Cluster< EntitiesGroup<T> > *);
		void empty(bool);
		bool has_group(uint32_t);

		uint32_t create_pairwise_matches(FILE *);
		double compute_entropy(uint32_t);
		double compute_conditional_entropy(uint32_t);
		void display();

		uint32_t get_id();
		char * get_label();
		uint32_t get_num_entities();
		uint32_t get_non_deleted_entities();
		uint32_t get_num_groups();
		EntitiesGroup<T> * get_group(uint32_t);
		T * get_entity(uint32_t, uint32_t);
		T * get_representative();
		class TokensLexicon * get_token_distribution();

		void set_id(uint32_t);
		void set_label(char *);
		void set_representative(T *);
};
#endif // CLUSTER_H
