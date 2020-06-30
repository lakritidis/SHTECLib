#ifndef ENTITYCLUSTER_H
#define ENTITYCLUSTER_H

/// Class which is used to store additional information Entity-Cluster relationship
template <class T> class EntityCluster {
	private:
		T * e;
		class Cluster<T> * c;
		uint32_t index;

	public:
		EntityCluster(T *);
		~EntityCluster();

		void set_cluster(class Cluster<T> *, uint32_t);

		T * get_entity();
		class Cluster<T> * get_cluster();
		uint32_t get_index();
};

#endif // ENTITYCLUSTER_H
