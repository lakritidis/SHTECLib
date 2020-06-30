#include "Cluster.h"

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// PRIMARY TEMPLATE
/// ///////////////////////////////////////////////////////////////////////////////////////////////

/// Default constructor
template <class T> Cluster<T>::Cluster(uint32_t i) :
	id(i), label(NULL), num_entities(0), num_alloc_entities(8), entities(NULL), representative(NULL), tokens_distr(NULL) {

		this->entities = (T **)malloc(this->num_alloc_entities * sizeof(T *));
}

/// Destructor
template <class T> Cluster<T>::~Cluster() {
	if (this->label) { delete [] this->label; }
	if (this->entities) { free(this->entities); }

	if (this->representative) {
		if (this->representative->get_type() == 9999) {
			delete this->representative;
		}
	}

	if (this->tokens_distr) {
		delete this->tokens_distr;
	}
}

/// Insert a new Entity into the cluster.
template <class T> uint32_t Cluster<T>::insert_entity(T * e) {
	this->entities[this->num_entities++] = e;

	if (this->num_entities >= this->num_alloc_entities) {
		this->num_alloc_entities *= 2;
		this->entities = (T **)realloc(this->entities, this->num_alloc_entities * sizeof(T *));
	}
	return this->num_entities;
}

/// Insert a new Entity into the cluster AND update the token distribution
template <class T> uint32_t Cluster<T>::insert_entity(T * e, bool td) {
	/// If td flag is true, then create the token distribution (if it has not already been created).
	if (td) {
		if (!this->tokens_distr) {
			this->tokens_distr = new TokensLexicon(256);
		}

		for (uint32_t i = 0; i < e->get_num_tokens(); i++) {
			this->tokens_distr->insert(e->get_token(i)->get_str(), 1, 1);
		}
	}

	/// Insert the entity into the cluster
	this->entities[this->num_entities++] = e;

	if (this->num_entities >= this->num_alloc_entities) {
		this->num_alloc_entities *= 2;
		this->entities = (T **)realloc(this->entities, this->num_alloc_entities * sizeof(T *));
	}
	return this->num_entities;
}

/// "Delete" a Entity from the cluster (we just ground the pointer).
template <class T> inline void Cluster<T>::delete_entity(uint32_t i) {
	if (this->tokens_distr && this->entities[i]) {
		for (uint32_t x = 0; x < this->entities[i]->get_num_tokens(); x++) {
			this->tokens_distr->erase(this->entities[i]->get_token(x)->get_str());
		}
	}

	this->entities[i] = NULL;
}


/// Compute the cluster's clustroid: this is, the record that has the minimum distance (i.e. maximum
/// similarity) from all the other records.
template <class T> void Cluster<T>::compute_clustroid() {
	uint32_t p1 = 0, p2 = 0;
	_score_t max_sim = 0.0, sum_sim = 0.0;
	T * e = NULL;

	this->representative = NULL;
	if (this->num_entities == 1) {
		this->representative = this->entities[0];

	} else if (this->num_entities == 2) {
		this->representative = NULL;

	} else {
		for (p1 = 0; p1 < this->num_entities; p1++) {
			sum_sim = 0.0;
			e = this->entities[p1];
			if (e) {
				for (p2 = 0; p2 < this->num_entities; p2++) {
					if (this->entities[p2]) {
						sum_sim += e->cosine_similarity(this->entities[p2]);
					}
				}

				if (sum_sim > max_sim) {
					max_sim = sum_sim;
					this->representative = e;
				} else if (sum_sim == max_sim && this->representative) {
					if (this->representative->get_num_tokens() < e->get_num_tokens()) {
						this->representative = e;
					}
				}
			}
		}
	}

	max_sim = 0.0;
	if (!this->representative) {
		for (p1 = 0; p1 < this->num_entities; p1++) {
			e = this->entities[p1];
			if (e) {
				if (e->get_num_tokens() > max_sim) {
					max_sim = e->get_num_tokens();
					this->representative = e;
				}
			}
		}
	}
}

/// Compute the cluster's centroid: this is, an artificial point with co-ordinates equal to the
/// average of the cluster's components.
template <class T> void Cluster<T>::compute_centroid(uint16_t centroid_type) {
	uint32_t max_dim = 0, p = 0, x = 0, real_num_entities = 0;
	uint16_t i = 0, init_dim = 0;
	_score_t weight_sum = 0.0;
	T * e = NULL;

	/// Delete the previous Centroid (if it exists)
	if (this->representative) {
		delete this->representative;
		this->representative = NULL;
	}

	/// Get the total number of features (with duplicates)
	for (p = 0; p < this->num_entities; p++) {
		if (this->entities[p]) {
			real_num_entities++;
			init_dim += this->entities[p]->get_num_tokens();
		}
	}

	max_dim = 2 << (8 * sizeof(init_dim) - 1);

	if (init_dim > max_dim) { init_dim = max_dim; }

	/// Temporary storage of features (with duplicates)
	class Feature * t_comps = new Feature[init_dim];

	/// Initially, insert all the tokens of all entities into the Centroid.
	for (p = 0; p < this->num_entities; p++) {
		e = this->entities[p];
//		e->display_feature_vector();
		if (e) {
			for (i = 0; i < e->get_num_tokens(); i++) {
				if (x < init_dim) {
					t_comps[x].set_token(e->get_token(i));
					t_comps[x].set_weight(e->get_token_weight(i));
					x++;
				}
			}
		}
	}

	/// Create the centroid from the temporary storage (features in increasing ID order and with
	/// duplicates removed)
	this->representative = new T(init_dim);
	this->representative->set_type(9999);

	/// Sort the temporary storage in increasing feature ID order.
	qsort(t_comps, init_dim, sizeof(class Feature), &cmp_features);

	for (p = 0; p < init_dim; p++) {
		weight_sum = t_comps[p].get_weight();

		while(1) {
			if (p >= init_dim - 1) {
				this->representative->insert_token(t_comps[p].get_token(), weight_sum);
				break;
			} else {
				if (t_comps[p].get_id() == t_comps[p + 1].get_id()) {
					p++;

					weight_sum += t_comps[p].get_weight();
				} else {
//					printf("Token ID: %d - %s - Weight: %5.3f\n", t_comps[p].get_token()->get_id(),
//						t_comps[p].get_token()->get_str(), t_comps[p].get_weight());

					this->representative->insert_token(t_comps[p].get_token(), weight_sum);
					break;
				}
			}
		}
	}

	/// Standard k-Means: Average centroid component coefficients.
	if (centroid_type == 1) {
		this->representative->divide_weights( (double)real_num_entities );
	/// Spherical k-Means: Normalize centroid component coefficients to turn it into a unit vector.
	} else if (centroid_type == 3) {
		this->representative->normalize_feature_vector();
	}

	this->representative->compute_entity_weight();

	delete [] t_comps;
//	this->display(); getchar();
}

/// Compute the linkage distance(similarity) between two clusters
/// Linkage Types: 1: Single Linkage, 2: Complete Linkage, 3: Average Linkage
template <class T> _score_t Cluster<T>::compute_linkage(class Cluster<T> * m, uint32_t linkage_type) {
	_score_t sim = 0.0, sum_sim = 0.0, max_sim = 0.0, min_sim = 1.1;
	T * e = NULL;

	for (uint32_t p1 = 0; p1 < this->num_entities; p1++) {
		e = this->entities[p1];
		for (uint32_t p2 = 0; p2 < m->get_num_entities(); p2++) {
			sim = e->cosine_similarity(m->get_entity(p2));
			sum_sim += sim;
			if (sim > max_sim) { max_sim = sim; }
			if (sim < min_sim) { min_sim = sim; }
		}
	}

	if (linkage_type == 1) {
		return max_sim;
	} else if (linkage_type == 2) {
		return min_sim;
	} else if (linkage_type == 3) {
		return sum_sim / (_score_t)(this->num_entities * m->get_num_entities());
	}

	return this->representative->cosine_similarity(m->get_representative());
}

/// Merge two clusters into one
template <class T> void Cluster<T>::merge_with(class Cluster<T> * m) {
	for (uint32_t p = 0; p < m->get_num_entities(); p++) {
//		this->entities[p]->display(); getchar();
		this->insert_entity( m->get_entity(p) );
	}
}

/// Remove the elements of a cluster.
template <class T> void Cluster<T>::empty(bool keep_representative) {
	T * temp = this->representative;

	for (uint32_t p = 0; p < this->num_entities; p++) {
//		this->entities[p]->display(); getchar();
		this->entities[p] = NULL;
	}

	if (keep_representative) {
		this->entities[0] = temp;
		this->num_entities = 1;
	} else {
		this->num_entities = 0;
	}
}

/// Create the matching pairs (Entity1,Entity2) for this cluster and write them to the input file.
template <class T> uint32_t Cluster<T>::create_pairwise_matches(FILE *fp) {
	uint32_t i = 0, j = 0, e1_id = 0, e2_id = 0, num = 0;

	for (i = 0; i < this->num_entities; i++) {
		if (this->entities[i]) {
			for (j = i + 1; j < this->num_entities; j++) {
				if (this->entities[j]) {
					e1_id = this->entities[i]->get_id();
					e2_id = this->entities[j]->get_id();

					if (e1_id < e2_id) {
						fwrite( &e1_id, sizeof(uint32_t), 1, fp);
						fwrite( &e2_id, sizeof(uint32_t), 1, fp);
					} else {
						fwrite( &e2_id, sizeof(uint32_t), 1, fp);
						fwrite( &e1_id, sizeof(uint32_t), 1, fp);
					}

					num++;
				}
			}
		}
	}

	return num;
//	printf("Written matches: %d\n", num); getchar();
}

/// Compute the entropy of this cluster given the total number of entities in the collection.
template <class T> double Cluster<T>::compute_entropy(uint32_t tot_entities) {
	uint32_t real_num_entities = this->get_non_deleted_entities();
	double entropy = 0.0, prob = 0.0;

	if (real_num_entities > 0) {
		prob = (double)real_num_entities / (double)tot_entities;
		entropy = -prob * log2(prob);
	}

	return entropy;
}

/// Compute the conditional entropy of this cluster.
template <class T> double Cluster<T>::compute_conditional_entropy(uint32_t tot_entities) {
	uint32_t i = 0, j = 0, f = 0, num_real_clusters = 0, num_real_entities = 0;
	double c_entropy = 0.0, frac = 0.0;

	struct real_cluster {
		uint32_t id;
		uint32_t num_entities;
	} real_clusters[this->num_entities];

	for (i = 0; i < this->num_entities; i++) {
		if (this->entities[i]) {
			num_real_entities++;
			f = 0;
			for (j = 0; j < num_real_clusters; j++) {
				if (this->entities[i]->get_real_cluster_id() == real_clusters[j].id) {
					f = 1;
					real_clusters[j].num_entities++;
					break;
				}
			}

			if (f == 0) {
				real_clusters[num_real_clusters].id = this->entities[i]->get_real_cluster_id();
				real_clusters[num_real_clusters].num_entities = 1;
				num_real_clusters++;
			}
		}
	}

//	this->display(); printf("\nReal Clusters:\n\n");
	for (j = 0; j < num_real_clusters; j++) {
		frac = (double)real_clusters[j].num_entities / (double)this->num_entities;
		if (frac < 1) {
			c_entropy += -frac * log2(frac);
		}
//		printf("Class %d - Cluster ID: %d - num_entities: %d, frac: %4.3f, Hyc: %5.4f\n",
//			j, real_clusters[j].id, real_clusters[j].num_entities, frac, c_entropy * (double)this->num_entities / (double)tot_entities);
	}
//	getchar();
	c_entropy *= (double)num_real_entities / (double)tot_entities;

	return c_entropy;
}

/// Display the cluster contents
template <class T> void Cluster<T>::display() {
	printf("===================================================================================\n");
	if (this->label) {
		printf("CLUSTER ID: %d\n\tLABEL: %s\n", this->id, this->label);
	} else {
		printf("CLUSTER ID: %d\n\tLABEL: [ NOT SET ]\n", this->id);
	}

	if (this->representative) {
		if (this->representative->get_type() == 9999) {
			printf("\tCENTROID: ");
			this->representative->display_feature_vector();
		} else {
			char title[1024] = {0};
			this->representative->get_text(title);
			printf("\tREPRESENTATIVE: %s\n", title);
		}
	} else {
		printf("\tREPRESENTATIVE: [ NOT SET ]\n");
		printf("\tCENTROID: [ NOT SET ]\n");
	}

	printf("\tNUM ENTITIES: %d\n", this->num_entities);
	for (uint32_t i = 0; i < this->num_entities; i++) {
		printf("\t\t");
		if (this->entities[i]) {
			printf("%d. ", i); this->entities[i]->display();
		} else {
			printf("%d. ", i); printf("Deleted\n");
		}
	}
	printf("\n");
	printf("===================================================================================\n");
}


/// Accessors
template <class T> uint32_t Cluster<T>::get_id() { return this->id; }
template <class T> char * Cluster<T>::get_label() { return this->label; }
template <class T> uint32_t Cluster<T>::get_num_entities() { return this->num_entities; }
template <class T> uint32_t Cluster<T>::get_non_deleted_entities() {
	uint32_t real_num_entities = 0;
	for (uint32_t p = 0; p < this->num_entities; p++) {
		if (this->entities[p]) {
			real_num_entities++;
		}
	}
	return real_num_entities;
}

template <class T> T * Cluster<T>::get_entity(uint32_t i) { return this->entities[i]; }
template <class T> T * Cluster<T>::get_representative() { return this->representative; }
template <class T> class TokensLexicon * Cluster<T>::get_token_distribution() {
	return this->tokens_distr;
}

/// Mutators
template <class T> void Cluster<T>::set_id(uint32_t v) { this->id = v; }
template <class T> void Cluster<T>::set_label(char * v) {
	this->label = new char [strlen(v) + 1];
	strcpy(this->label, v);
}
template <class T> void Cluster<T>::set_representative(T * v) { this->representative = v; }



/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// SPECIALIZED TEMPLATE
/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <class T> Cluster< EntitiesGroup<T> >::Cluster(uint32_t i) :
	id(i), label(NULL), num_entities(0), num_groups(0), num_alloc_groups(4), groups(NULL), representative(NULL), tokens_distr(NULL) {

		this->groups = (class EntitiesGroup<T> **)malloc(this->num_alloc_groups * sizeof(class EntitiesGroup<T> *));
}

/// Destructor
template <class T> Cluster< EntitiesGroup<T> >::~Cluster() {
	if (this->label) {
		delete [] this->label;
	}

	if (this->groups) {
		for (uint32_t i = 0; i < this->num_groups; i++) {
			delete this->groups[i];
		}
		free(this->groups);
	}

	if (this->representative) {
		if (this->representative->get_type() == 9999) {
			delete this->representative;
		}
	}

	if (this->tokens_distr) {
		delete this->tokens_distr;
	}
}

/// Insert a new Entity into the cluster
template <class T> uint32_t Cluster< EntitiesGroup<T> >::insert_entity(T * e) {
	uint32_t i = 0;

	for (i = 0; i < this->num_groups; i++) {
		if (this->groups[i]->get_group_key() == e->get_vendor_id()) {
//			printf("Found group %d: Inserting %s\n", e->get_vendor_id(), e->get_text()); fflush(NULL);
			this->groups[i]->insert_entity(e);
			this->num_entities++;
			return this->num_groups;
		}
	}

//	printf("Create New group: Inserting %s with vendor ID: %d\n", e->get_text(), e->get_vendor_id());fflush(NULL);
	this->groups[this->num_groups] = new EntitiesGroup<T>( e->get_vendor_id() );
	this->groups[this->num_groups]->insert_entity(e);
	this->num_groups++;

	if (this->num_groups >= this->num_alloc_groups) {
		this->num_alloc_groups *= 2;
		this->groups = (class EntitiesGroup<T> **)realloc(this->groups,
				this->num_alloc_groups * sizeof(class EntitiesGroup<T> *));
	}

	this->num_entities++;
	return this->num_groups;
}

/// Insert a new Entity into the cluster AND update the token distribution
template <class T> uint32_t Cluster< EntitiesGroup<T> >::insert_entity(T * e, bool td) {
	uint32_t i = 0;

	/// If td flag is true, then create the token distribution (if it has not already been created).
	if (td) {
		if (!this->tokens_distr) {
			this->tokens_distr = new TokensLexicon(256);
		}

		for (i = 0; i < e->get_num_tokens(); i++) {
			this->tokens_distr->insert(e->get_token(i)->get_str(), 1, 1);
		}
	}

	for (i = 0; i < this->num_groups; i++) {
		if (this->groups[i]->get_group_key() == e->get_vendor_id()) {
//			printf("Found group %d: Inserting %s\n", e->get_vendor_id(), e->get_text()); fflush(NULL);
			this->groups[i]->insert_entity(e);
			this->num_entities++;
			return this->num_groups;
		}
	}

//	printf("Create New group: Inserting %s with vendor ID: %d\n", e->get_text(), e->get_vendor_id());fflush(NULL);
	this->groups[this->num_groups] = new EntitiesGroup<T>( e->get_vendor_id() );
	this->groups[this->num_groups]->insert_entity(e);
	this->num_groups++;

	if (this->num_groups >= this->num_alloc_groups) {
		this->num_alloc_groups *= 2;
		this->groups = (class EntitiesGroup<T> **)realloc(this->groups,
				this->num_alloc_groups * sizeof(class EntitiesGroup<T> *));
	}

	this->num_entities++;
	return this->num_groups;
}

/// Insert a new Entity into the cluster (the vendors are in increasing order)
template <class T> uint32_t Cluster< EntitiesGroup<T> >::insert_entity_sorted(T * e) {
	int32_t t = this->binsearch_group(e->get_vendor_id(), 0, this->num_groups - 1);

	this->groups[t]->insert_entity(e);
	this->num_entities++;

	return this->num_groups;
}

/// Given a (sorted) array of vendors and an ID, (binary) search for the vendor in the array.
template <class T> int32_t Cluster< EntitiesGroup<T> >::binsearch_group(uint32_t g_id, int32_t l, int32_t r) {
	if (r >= l) {
		int32_t mid = l + (r - l) / 2;

		if (this->groups[mid]->get_group_key() == g_id) {
			return mid;
		} else if (this->groups[mid]->get_group_key() > g_id) {
			return this->binsearch_group(g_id, l, mid - 1);
		} else {
			return this->binsearch_group(g_id, mid + 1, r);
		}
	}

    return -1;
}

/// "Delete" a GroupedEntity from the cluster (we just ground the pointer)
template <class T> inline void Cluster< EntitiesGroup<T> >::delete_entity(uint32_t i, uint32_t j) {
	T * e = this->groups[i]->get_entity(j);

	if (this->tokens_distr && e) {
		for (uint32_t x = 0; x < e->get_num_tokens(); x++) {
			this->tokens_distr->erase(e->get_token(x)->get_str());
		}
	}

	this->groups[i]->delete_entity(j);
}

/// "Delete" the first GroupedEntity from the cluster (we just ground the pointer)
template <class T> inline void Cluster< EntitiesGroup<T> >::delete_entity(uint32_t j) {
	T * e = this->groups[0]->get_entity(j);

	if (this->tokens_distr && e) {
		for (uint32_t x = 0; x < e->get_num_tokens(); x++) {
			this->tokens_distr->erase(e->get_token(x)->get_str());
		}
	}

	this->groups[0]->delete_entity(j);
}

/// Prepare the cluster for the verification stage
template <class T> void Cluster< EntitiesGroup<T> >::prepare(uint16_t type) {
	if (type == 1) {
		this->compute_centroid(type);
	} else {
		this->compute_clustroid();
	}

	/// Strangely, if we delete the following three lines we get better results in some cases.
	for (uint32_t i = 0; i < this->num_groups; i++) {
		this->groups[i]->prepare(this->representative);
	}
}

/// Compute the cluster's clustroid: this is, the record that has the minimum distance (i.e. maximum
/// similarity) from all the other records.
template <class T> void Cluster< EntitiesGroup<T> >::compute_clustroid() {
	uint32_t v1 = 0, v2 = 0, p1 = 0, p2 = 0;
	_score_t max_sim = 0.0, sum_sim = 0.0, loc_sim = 0.0, max_loc_sim = 0.0;
	T * e = NULL;

	this->representative = NULL;
	if (this->num_entities == 1) {
		this->representative = this->groups[0]->get_entity(0);

	} else if (this->num_entities == 2) {
		this->representative = NULL;

	} else {
		for (v1 = 0; v1 < this->num_groups; v1++) {
			for (p1 = 0; p1 < this->groups[v1]->get_num_entities(); p1++) {
				e = this->groups[v1]->get_entity(p1);

				if (e) {
					sum_sim = 0.0;

					for (v2 = 0; v2 < this->num_groups; v2++) {
						loc_sim = 0.0;
						max_loc_sim = 0.0;

						if (v1 != v2) {
							for (p2 = 0; p2 < this->groups[v2]->get_num_entities(); p2++) {

								if (this->groups[v2]->get_entity(p2)) {
									loc_sim = e->cosine_similarity(this->groups[v2]->get_entity(p2));

									if (loc_sim >= max_loc_sim) {
										max_loc_sim = loc_sim;
									}
								}
							}
						}
						sum_sim += max_loc_sim;
					}

					if (sum_sim > max_sim) {
						max_sim = sum_sim;
						this->representative = e;
					} else if (sum_sim == max_sim && this->representative) {
						if (this->representative->get_num_tokens() < e->get_num_tokens()) {
							this->representative = e;
						}
					}
				}
			}
		}
	}

	max_sim = 0.0;
	if (!this->representative) {
//	printf("No clustroid - num products: %d, num_vendors: %d\n", this->num_products, this->num_vendors); getchar();
		for (v1 = 0; v1 < this->num_groups; v1++) {
			for (p1 = 0; p1 < this->groups[v1]->get_num_entities(); p1++) {
				if (this->groups[v1]->get_entity(p1)->get_num_tokens() > max_sim) {
					max_sim = this->groups[v1]->get_entity(p1)->get_num_tokens();
					this->representative = this->groups[v1]->get_entity(p1);
				}
			}
		}
	}
}

/// Compute the cluster's centroid: this is, an artificial point with co-ordinates equal to the
/// average of the cluster's components.
template <class T> void Cluster< EntitiesGroup<T> >::compute_centroid(uint16_t centroid_type) {
	uint32_t g = 0, p = 0, x = 0, real_num_entities = 0, max_dim = 0;
	uint16_t i = 0, init_dim = 0;

	_score_t weight_sum = 0.0;
	T * e = NULL;

	/// Delete the previous Centroid (if it exists)
	if (this->representative) {
		delete this->representative;
		this->representative = NULL;
	}

	/// Get the total number of features (with duplicates)
	for (g = 0; g < this->num_groups; g++) {
		for (p = 0; p < this->groups[g]->get_num_entities(); p++) {
			if (this->groups[g]->get_entity(p)) {
				real_num_entities++;
				init_dim += this->groups[g]->get_entity(p)->get_num_tokens();
			}
		}
	}

	max_dim = 2 << (8 * sizeof(init_dim) - 1);

	if (init_dim > max_dim) { init_dim = max_dim; }

	/// Temporary storage of features (with duplicates)
	class Feature * t_comps = new Feature[init_dim];

	/// Initially, insert all the tokens of all entities into the Centroid.
	for (g = 0; g < this->num_groups; g++) {
		for (p = 0; p < this->groups[g]->get_num_entities(); p++) {
			e = this->groups[g]->get_entity(p);

//			e->display_feature_vector();
			if (e) {
				for (i = 0; i < e->get_num_tokens(); i++) {
					if (x < init_dim) {
						t_comps[x].set_token(e->get_token(i));
						t_comps[x].set_weight(e->get_token_weight(i));
						x++;
					}
				}
			}
		}
	}

	/// Create the centroid from the temporary storage (features in increasing ID order and with
	/// duplicates removed)
	this->representative = new T(init_dim);
	this->representative->set_type(9999);

	/// Sort the temporary storage in increasing feature ID order.
	qsort(t_comps, init_dim, sizeof(class Feature), &cmp_features);

	for (p = 0; p < init_dim; p++) {
		weight_sum = t_comps[p].get_weight();

		while(1) {
			if (p >= init_dim - 1) {
				this->representative->insert_token(t_comps[p].get_token(), weight_sum);
				break;
			} else {
				if (t_comps[p].get_id() == t_comps[p + 1].get_id()) {
					p++;

					weight_sum += t_comps[p].get_weight();
				} else {
//					printf("Token ID: %d - %s - Weight: %5.3f\n", t_comps[p].get_token()->get_id(),
//						t_comps[p].get_token()->get_str(), t_comps[p].get_weight());
					this->representative->insert_token(t_comps[p].get_token(), weight_sum);
					break;
				}
			}
		}
	}

	/// Standard k-Means: Average centroid component coefficients.
	if (centroid_type == 1) {
		this->representative->divide_weights( (double)real_num_entities );
	/// Spherical k-Means: Normalize centroid component coefficients to turn it into a unit vector.
	} else if (centroid_type == 3) {
		this->representative->normalize_feature_vector();
	}

	this->representative->compute_entity_weight();

	delete [] t_comps;
//	this->display(); getchar();
}

/// Compute the linkage distance(similarity) between two clusters
/// Linkage Types: 1: Single Linkage, 2: Complete Linkage, 3: Average Linkage
template <class T> _score_t Cluster< EntitiesGroup<T> >::compute_linkage(class Cluster< EntitiesGroup<T> > * m, uint32_t linkage_type) {
	_score_t sim = 0.0, sum_sim = 0.0, max_sim = 0.0, min_sim = 1.0;
	T * e;

	for (uint32_t v1 = 0; v1 < this->num_groups; v1++) {
		for (uint32_t p1 = 0; p1 < this->groups[v1]->get_num_entities(); p1++) {
			e = this->get_entity(v1, p1);

			for (uint32_t v2 = 0; v2 < m->get_num_groups(); v2++) {
				for (uint32_t p2 = 0; p2 < m->get_group(v2)->get_num_entities(); p2++) {
					sim = e->cosine_similarity(m->get_entity(v2, p2));

					sum_sim += sim;
					if (sim >= max_sim) { max_sim = sim; }
					if (sim <= min_sim) { min_sim = sim; }
				}
			}
		}
	}

	if (linkage_type == 1) {
		return max_sim;
	} else if (linkage_type == 2) {
		return min_sim;
	} else if(linkage_type == 3) {
		return sum_sim / (_score_t)(this->num_entities * m->get_num_entities());
	}

	return this->representative->cosine_similarity(m->get_representative());
}

/// Create empty groups into this cluster
template <class T> void Cluster< EntitiesGroup<T> >::create_empty_groups(uint32_t n, uint32_t * g_ids) {
	for (uint32_t i = 0; i < n; i++) {
		this->groups[this->num_groups] = new EntitiesGroup<T>( g_ids[i] );
		this->groups[this->num_groups]->insert_entity(NULL);
		this->num_groups++;

		if (this->num_groups >= this->num_alloc_groups) {
			this->num_alloc_groups *= 2;
			this->groups = (class EntitiesGroup<T> **)realloc(this->groups,
					this->num_alloc_groups * sizeof(class EntitiesGroup<T> *));
		}
	}
}

/// Merge two clusters into one
template <class T> void Cluster< EntitiesGroup<T> >::merge_with(class Cluster< EntitiesGroup<T> > * m) {
	for (uint32_t v = 0; v < m->get_num_groups(); v++) {
		for (uint32_t p = 0; p < m->get_group(v)->get_num_entities(); p++) {
//			printf("(%d, %d)\n", x, y); m->get_entity(x, y)->display(); getchar();
			this->insert_entity( m->get_entity(v, p) );
		}
	}
}

/// Remove the elements of a cluster.
template <class T> void Cluster< EntitiesGroup<T> >::empty(bool keep_representative) {
	T * temp = this->representative;

	for (uint32_t v = 0; v < this->num_groups; v++) {
		delete this->groups[v];
		this->groups[v] = NULL;
	}

	this->num_entities = 0;
	this->num_groups = 0;

	if (keep_representative) {
		this->insert_entity(temp);
		this->representative = temp;
	}

//	this->groups[0]->get_entity(0)->display(); getchar();
}

/// Search for a group within the cluster
template <class T> bool Cluster< EntitiesGroup<T> >::has_group(uint32_t g) {
	for (uint32_t v = 0; v < this->num_groups; v++) {
		if (this->groups[v]->get_group_key() == g) return true;
	}
	return false;
/*
	if (this->binsearch_group(g, 0, this->num_groups - 1) == -1) {
		return false;
	}
	return true;
*/
}

/// Create the matching pairs (Entity1,Entity2) for this cluster and write them to the input file.
template <class T> uint32_t Cluster< EntitiesGroup<T> >::create_pairwise_matches(FILE *fp) {
	uint32_t i = 0, j = 0, k = 0, l = 0, e1_id = 0, e2_id = 0, num = 0;

	num = 0;
	for (i = 0; i < this->num_groups; i++) {
		for (j = 0; j < this->groups[i]->get_num_entities(); j++) {
			if (this->groups[i]->get_entity(j)) {
				/// Match the jth entity of the ith vendor with all the other products of this vendor
				for (k = j + 1; k < this->groups[i]->get_num_entities(); k++) {
					if (this->groups[i]->get_entity(k)) {
						e1_id = this->groups[i]->get_entity(j)->get_id();
						e2_id = this->groups[i]->get_entity(k)->get_id();

						if (e1_id < e2_id) {
							fwrite( &e1_id, sizeof(uint32_t), 1, fp);
							fwrite( &e2_id, sizeof(uint32_t), 1, fp);
						} else {
							fwrite( &e2_id, sizeof(uint32_t), 1, fp);
							fwrite( &e1_id, sizeof(uint32_t), 1, fp);
						}
//						printf("\t\t[%d, %d]\n", e1_id, e2_id);
						num++;
					}
				}

				/// Match the jth entity of the ith vendor with all the other products of all the
				/// other vendors
				for (k = i + 1; k < this->num_groups; k++) {
					for (l = 0; l < this->groups[k]->get_num_entities(); l++) {
						if (this->groups[k]->get_entity(l)) {
							e1_id = this->groups[i]->get_entity(j)->get_id();
							e2_id = this->groups[k]->get_entity(l)->get_id();

							if (e1_id < e2_id) {
								fwrite( &e1_id, sizeof(uint32_t), 1, fp);
								fwrite( &e2_id, sizeof(uint32_t), 1, fp);
							} else {
								fwrite( &e2_id, sizeof(uint32_t), 1, fp);
								fwrite( &e1_id, sizeof(uint32_t), 1, fp);
							}
//							printf("\t\t[%d, %d]\n", e1_id, e2_id);
							num++;
						}
					}
				}
			}
		}
	}

	return num;
//	printf("Written matches: %d\n", num); getchar();
}

/// Compute the entropy of this cluster given the total number of entities in the collection.
template <class T> double Cluster< EntitiesGroup<T> >::compute_entropy(uint32_t tot_entities) {
	uint32_t real_num_entities = this->get_non_deleted_entities();
	double entropy = 0.0, prob = 0.0;

	if (real_num_entities > 0) {
		prob = (double)real_num_entities / (double)tot_entities;
		entropy = -prob * log2(prob);
	}

	return entropy;
}

/// Compute the conditional entropy of this cluster.
template <class T> double Cluster< EntitiesGroup<T> >::compute_conditional_entropy(uint32_t tot_entities) {
	uint32_t i = 0, j = 0, k = 0, f = 0, num_real_clusters = 0, num_real_entities = 0;
	double c_entropy = 0.0, frac = 0.0;
	T * e;

	struct real_cluster {
		uint32_t id;
		uint32_t num_entities;
	} real_clusters[this->num_entities];

	for (i = 0; i < this->num_groups; i++) {
		for (j = 0; j < this->groups[i]->get_num_entities(); j++) {
			e = this->groups[i]->get_entity(j);
			if (e) {
				num_real_entities++;
				f = 0;
				for (k = 0; k < num_real_clusters; k++) {
					if (e->get_real_cluster_id() == real_clusters[j].id) {
						f = 1;
						real_clusters[j].num_entities++;
						break;
					}
				}

				if (f == 0) {
					real_clusters[num_real_clusters].id = e->get_real_cluster_id();
					real_clusters[num_real_clusters].num_entities = 1;
					num_real_clusters++;
				}
			}
		}
	}

//	this->display(); printf("\nReal Clusters:\n\n");
	for (j = 0; j < num_real_clusters; j++) {
		frac = (double)real_clusters[j].num_entities / (double)this->num_entities;
		if (frac < 1) {
			c_entropy += -frac * log2(frac);
		}
//		printf("Class %d - Cluster ID: %d - num_entities: %d, frac: %4.3f, Hyc: %5.4f\n",
//			j, real_clusters[j].id, real_clusters[j].num_entities, frac, c_entropy);
	}
//	getchar();
	c_entropy *= (double)num_real_entities / (double)tot_entities;

	return c_entropy;
}

/// Display the cluster contents
template <class T> void Cluster< EntitiesGroup<T> >::display() {
	printf("===================================================================================\n");
	if (this->label) {
		printf("CLUSTER ID: %d\n\tLABEL: %s\n", this->id, this->label);
	} else {
		printf("CLUSTER ID: %d\n\tLABEL: [ NOT SET ]\n", this->id);
	}

	if (this->representative) {
		if (this->representative->get_type() == 9999) {
			printf("\tCENTROID: ");
			this->representative->display_feature_vector();
		} else {
			char title[1024] = {0};
			this->representative->get_text(title);
			printf("\tREPRESENTATIVE: %s\n", title);
		}
	} else {
		printf("\tREPRESENTATIVE: [ NOT SET ]\n");
		printf("\tCENTROID: [ NOT SET ]\n");
	}

	printf("\tNUM ENTITIES: %d\n", this->num_entities);
	printf("\tNUM GROUPS: %d\n", this->num_groups);
	for (uint32_t i = 0; i < this->num_groups; i++) {
		printf("\t\t"); this->groups[i]->display();
	}
	printf("\n");
	printf("===================================================================================\n");
}

/// Accessors
template <class T> uint32_t Cluster< EntitiesGroup<T> >::get_id() { return this->id; }
template <class T> char * Cluster< EntitiesGroup<T> >::get_label() { return this->label; }
template <class T> uint32_t Cluster< EntitiesGroup<T> >::get_num_entities() { return this->num_entities; }
template <class T> uint32_t Cluster< EntitiesGroup<T> >::get_non_deleted_entities() {
	uint32_t g = 0, p = 0, real_num_entities = 0;

	for (g = 0; g < this->num_groups; g++) {
		for (p = 0; p < this->groups[g]->get_num_entities(); p++) {
			if (this->groups[g]->get_entity(p)) {
				real_num_entities++;
			}
		}
	}
	return real_num_entities;
}

template <class T> uint32_t Cluster< EntitiesGroup<T> >::get_num_groups() { return this->num_groups; }
template <class T> EntitiesGroup<T> * Cluster< EntitiesGroup<T> >::get_group(uint32_t i) { return this->groups[i]; }
template <class T> T * Cluster< EntitiesGroup<T> >::get_entity(uint32_t i, uint32_t j) { return this->groups[i]->get_entity(j); }
template <class T> T * Cluster< EntitiesGroup<T> >::get_representative() { return this->representative; }
template <class T> class TokensLexicon * Cluster<EntitiesGroup<T> >::get_token_distribution() {
	return this->tokens_distr;
}

/// Mutators
template <class T> inline void Cluster< EntitiesGroup<T> >::set_id(uint32_t v) { this->id = v; }
template <class T> inline void Cluster< EntitiesGroup<T> >::set_label(char * v) {
	this->label = new char [strlen(v) + 1];
	strcpy(this->label, v);
}
template <class T> inline void Cluster< EntitiesGroup<T> >::set_representative(T * v) { this->representative = v; }
