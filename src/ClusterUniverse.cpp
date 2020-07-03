#include "ClusterUniverse.h"

/// Default Constructor
template <class T> ClusterUniverse<T>::ClusterUniverse() :
	hash_table(NULL), num_nodes(0), num_slots(0), table_mask(0) {
}

/// Constructor
template <class T> ClusterUniverse<T>::ClusterUniverse(uint32_t nslots) :
	hash_table(NULL), num_nodes(0), num_slots(nslots), table_mask(nslots - 1) {

		this->hash_table = new ClusterHashNode<T> * [this->num_slots];
		for (uint32_t i = 0; i < this->num_slots; i++) {
			this->hash_table[i] = NULL;
		}
}

/// Destructor
template <class T> ClusterUniverse<T>::~ClusterUniverse() {
	delete [] this->hash_table;
}


/// Convert the hash table of clusters into an array of clusters
template <class T> class Cluster<T> ** ClusterUniverse<T>::convert_to_array() {
	uint32_t num_clusters = 0;
	class ClusterHashNode<T> * q;
	class Cluster<T> ** clusters = (class Cluster<T> **)malloc(this->num_nodes * sizeof(class Cluster<T> *));

	for (uint32_t i = 0; i < num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				clusters[num_clusters++] = (class Cluster<T> *) q;
//				q->display();
			}
		}
	}

	return clusters;
}

/// Insert a cluster into the hash table and place an entity inside it
template <class T> void ClusterUniverse<T>::insert_cluster(uint32_t cid, char * c, T * e) {
	/// Find the hash value of the input cluster
	uint32_t HashValue = cid & this->table_mask;

	/// Now search in the hash table to check whether this cluster exists or not
	if (this->hash_table[HashValue] != NULL) {
		class ClusterHashNode<T> * q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (q->get_id() == cid) {
//printf("Cluster exists! ID: %d (%d), %s\n", cid, HashValue, q->get_label());getchar();

				q->insert_entity(e);

				return; /// Return and exit
			}
		}
	}

	this->num_nodes++;

	/// Create a new record and re-assign the linked list's head
	class ClusterHashNode<T> * record = new ClusterHashNode<T>(cid);
	record->set_label(c);
	record->insert_entity(e);

	/// Reassign the chain's head
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;
}


/// Insert a UPM product into the hash table
template <class T> void ClusterUniverse<T>::insert_node(class UPMProduct * p) {
	/// Get the literal of the highest scoring combination of this UPMProduct. This is the hash Key
	char * c = p->get_predicted_cluster()->get_signature();

	/// Find the hash value of the input cluster
	uint32_t HashValue = KazLibHash(c) & this->table_mask;

	/// Now search in the hash table to check whether this term exists or not
	if (this->hash_table[HashValue] != NULL) {
		class ClusterHashNode< EntitiesGroup<UPMProduct> > * q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_label(), c) == 0) {

				q->insert_entity(p);

				return; /// Return and exit
			}
		}
	}

	this->num_nodes++;

	/// Create a new record and re-assign the linked list's head
	class ClusterHashNode<T> * record = new ClusterHashNode<T>( this->num_nodes );
	record->set_label(c);
	record->insert_entity(p);

	/// Reassign the chain's head
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;
}

/// Insert a VEPHCEntity into the hash table
template <class T> void ClusterUniverse<T>::insert_node(class VEPHCEntity * p) {
	/// Get the literal of the highest scoring combination of this UPMProduct. This is the hash Key
	char * c = p->get_predicted_cluster()->get_signature();

	/// Find the hash value of the input cluster
	uint32_t HashValue = KazLibHash(c) & this->table_mask;

	/// Now search in the hash table to check whether this term exists or not
	if (this->hash_table[HashValue] != NULL) {
		class ClusterHashNode< VEPHCEntity > * q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_label(), c) == 0) {

				q->insert_entity(p);

				return; /// Return and exit
			}
		}
	}

	this->num_nodes++;

	/// Create a new record and re-assign the linked list's head
	class ClusterHashNode<T> * record = new ClusterHashNode<T>( this->num_nodes );
	record->set_label(c);
	record->insert_entity(p);

	/// Reassign the chain's head
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;
}

/// Compute the entropy of all clusters (used in computation of NMI)
template <class T> double ClusterUniverse<T>::compute_entropy(uint32_t num_entities) {
	class ClusterHashNode<T> * q;
	double Hy = 0.0, prob = 0.0;

	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				prob = (double)q->get_num_entities() / (double)num_entities;

				Hy += -prob * log2(prob);

//				printf("Num Entities: %d, This Cluster: %d, prob: %5.3f, Hy = %5.3f\n",
//					num_entities, q->get_num_entities(), prob, Hy); getchar();
			}
		}
	}
	return Hy;
}

/// Display the cluster universe entries
template <class T> void ClusterUniverse<T>::display() {
	class ClusterHashNode<T> * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->display();
				getchar();
			}
		}
	}
}

/// Iterate through the clusters of the universe and create all the implied pairwise matches
template <class T> struct match ** ClusterUniverse<T>::create_pairwise_matches(uint32_t * n_m) {
	uint32_t num_alloc_matches = 65536, i = 0, j = 0, k = 0, e1_id = 0, e2_id = 0;
	struct match ** matches = (struct match **)malloc(num_alloc_matches * sizeof(struct match *));

	class ClusterHashNode<T> * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
	//			printf("Cluster %d with %d entities\n", q->get_id(), q->get_num_entities());
				for (j = 0; j < q->get_num_entities(); j++) {
					if (q->get_entity(j)) {
						for (k = j + 1; k < q->get_num_entities(); k++) {
							if (q->get_entity(k)) {

								e1_id = q->get_entity(j)->get_id();
								e2_id = q->get_entity(k)->get_id();

								matches[*n_m] = (struct match *)malloc(sizeof(struct match));

								if (e1_id < e2_id) {
									matches[*n_m]->e1_id = e1_id;
									matches[*n_m]->e2_id = e2_id;
								} else {
									matches[*n_m]->e1_id = e2_id;
									matches[*n_m]->e2_id = e1_id;
								}

//								printf("\tMatch %d : (%d, %d)\n", *n_m, matches[*n_m]->e1_id, matches[*n_m]->e2_id);
								(*n_m)++;

								if (*n_m >= num_alloc_matches) {
									num_alloc_matches *= 2;
									matches = (struct match **)realloc
											(matches, num_alloc_matches * sizeof(struct match *));
								}
							}
						}
					}
				}
//				getchar();
			}
		}
	}

	return matches;
}

/// Delete the nodes
template <class T> void ClusterUniverse<T>::delete_nodes() {
	class ClusterHashNode<T> * q;

	for (uint32_t i = 0; i < this->num_slots; i++) {
		while (this->hash_table[i] != NULL) {
			q = this->hash_table[i]->get_next();
			if (this->hash_table[i]) {
				delete this->hash_table[i];
			}
			this->hash_table[i] = q;
		}
	}
}

/// The Hash Function
template <class T> uint32_t ClusterUniverse<T>::KazLibHash (char *key) {
   static unsigned long randbox[] = {
       0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
       0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
       0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
       0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
	};

	char *str = key;
	uint32_t acc = 0;

	while (*str) {
		acc ^= randbox[(*str + acc) & 0xf];
		acc = (acc << 1) | (acc >> 31);
		acc &= 0xffffffffU;
		acc ^= randbox[((*str++ >> 4) + acc) & 0xf];
		acc = (acc << 2) | (acc >> 30);
		acc &= 0xffffffffU;
	}
	return acc;
}

template <class T> uint32_t ClusterUniverse<T>::get_num_nodes() { return this->num_nodes; }
