#include "ClusterHashNode.h"

template <class T> ClusterHashNode<T>::ClusterHashNode(uint32_t i) : Cluster<T>(i), next(NULL) { }

template <class T> ClusterHashNode<T>::~ClusterHashNode() { }

template <class T> class ClusterHashNode<T> * ClusterHashNode<T>::get_next() { return this->next; }
template <class T> void ClusterHashNode<T>::set_next(class ClusterHashNode<T> * v) { this->next = v; }
