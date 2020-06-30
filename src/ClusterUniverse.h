#ifndef CLUSTERUNIVERSE_H
#define CLUSTERUNIVERSE_H


template <class T> class ClusterUniverse {
	private:
		class ClusterHashNode<T> ** hash_table;

		uint32_t num_nodes;
		uint32_t num_slots;
		uint32_t table_mask;

	private:
		uint32_t KazLibHash(char *);

	public:
		ClusterUniverse();
		ClusterUniverse(uint32_t);
		~ClusterUniverse();

		double compute_entropy(uint32_t);
		void display();
		void delete_nodes();

		class Cluster<T> ** convert_to_array();
		void insert_node(class UPMProduct *);
		void insert_node(class L2DVEntity *);
		void insert_cluster(uint32_t, char *, T *);
		struct match ** create_pairwise_matches(uint32_t *);

		uint32_t get_num_nodes();
};

#endif // CLUSTERUNIVERSE_H
