#include "EntitiesGroup.h"

/// Default Constructor
template <class T> EntitiesGroup<T>::EntitiesGroup() :
	group_key(0), num_entities(0), num_alloc_entities(0), entities(NULL) {
}

/// Working Constructor
template <class T> EntitiesGroup<T>::EntitiesGroup(uint32_t i) :
	group_key(i), num_entities(0), num_alloc_entities(2), entities(NULL) {

		this->entities = (struct EntitiesGroup::entity *)malloc
			(this->num_alloc_entities * sizeof(struct EntitiesGroup::entity));
}

/// Destructor
template <class T> EntitiesGroup<T>::~EntitiesGroup() {
	if (this->entities) {
		free (this->entities);
	}
}

/// Insert an entity into the corresponding array
template <class T> void EntitiesGroup<T>::insert_entity(T * e) {
	if (e) {
		this->entities[this->num_entities].e = e;
		this->entities[this->num_entities].order = this->num_entities;
		this->entities[this->num_entities].score = 0.0;

		this->num_entities++;
		if (this->num_entities >= this->num_alloc_entities) {
			this->num_alloc_entities *= 2;
			this->entities = (struct EntitiesGroup::entity *)realloc(this->entities,
					this->num_alloc_entities * sizeof(struct EntitiesGroup::entity));
		}
	} else {
		this->entities[this->num_entities].e = NULL;
		this->entities[this->num_entities].order = 0;
		this->entities[this->num_entities].score = 0.0;
	}
}


/// Prepare
template <class T> void EntitiesGroup<T>::prepare(T * e) {
	for (uint32_t i = 0; i < this->num_entities; i++) {
		if (e == this->entities[i].e) {
			this->entities[i].score = 1.0;
		} else {
			this->entities[i].score = e->cosine_similarity(this->entities[i].e);
		}
	}
	qsort(this->entities, this->num_entities, sizeof(struct EntitiesGroup::entity), cmp_entities);
}


/// Delete a product from the corresponding array
template <class T> void EntitiesGroup<T>::delete_entity(uint32_t i) {
	this->entities[i].e = NULL;
}

/// Display the EntityVendor data and its provided products
template <class T> void EntitiesGroup<T>::display() {
	printf("Group Key: %d, Products: %d\n", this->group_key, this->num_entities); fflush(NULL);
	for (uint32_t i = 0; i < this->num_entities; i++) {
		if (this->entities[i].e) {
			printf("\t\t\t%d. ", i + 1); this->entities[i].e->display();
		} else {
			printf("\t\t\t%d. Entity has been deleted\n", i + 1);
		}
	}
}

/// Accessors
template <class T> inline uint32_t EntitiesGroup<T>::get_group_key() { return this->group_key; }
template <class T> inline uint32_t EntitiesGroup<T>::get_num_entities() { return this->num_entities; }
template <class T> inline T * EntitiesGroup<T>::get_entity(uint32_t i) { return this->entities[i].e; }
