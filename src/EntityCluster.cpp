#include "EntityCluster.h"

/// Class constructor
template <class T> EntityCluster<T>::EntityCluster(T * entity) : e(entity), c(NULL), index(0) {

}

/// Destructor
template <class T> EntityCluster<T>::~EntityCluster() {
	//dtor
}

/// Set the cluster for an entity
template <class T> void EntityCluster<T>::set_cluster(Cluster<T> * cluster, uint32_t ind) {
	this->c = cluster;
	this->index = ind;
}

/// Getters
template <class T> T * EntityCluster<T>::get_entity() { return this->e; }
template <class T> class Cluster<T> * EntityCluster<T>::get_cluster() { return this->c; }
template <class T> uint32_t EntityCluster<T>::get_index() { return this->index; }
