#ifndef ENTITIESGROUP_H
#define ENTITIESGROUP_H

/// T: Entities to group by and integer identifier group_key
/// group by object
template <class T> class EntitiesGroup {
	protected:
		uint32_t group_key;
		uint32_t num_entities;
		uint32_t num_alloc_entities;

		struct entity {
			T * e;
			uint32_t order;
			_score_t score;
		} * entities;

	private:
		/// Comparison callback function for QuickSort (score-based cluster sorting)
        static int cmp_entities(const void * a, const void * b) {
			struct EntitiesGroup::entity * x = (struct EntitiesGroup::entity *)a;
			struct EntitiesGroup::entity * y = (struct EntitiesGroup::entity *)b;

			if (x->score > y->score) {
				return -1;
			} else if (x->score < y->score) {
				return 1;
			}
			return 0;
		}

	public:
		EntitiesGroup();
		EntitiesGroup(uint32_t);
		~EntitiesGroup();

		void display();
		void insert_entity(T *);
		void delete_entity(uint32_t);
		void prepare(T *);

		uint32_t get_group_key();
		uint32_t get_num_entities();
		T * get_entity(uint32_t);
};

#endif // ENTITIESGROUP_H
