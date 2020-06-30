#include "ClusteringAlgorithm.h"

/// Constructor
template <class T, class U> ClusteringAlgorithm<T,U>::ClusteringAlgorithm(class Settings * p) :
	ClusteringMethod<T>(p),
	num_clusters(0), num_alloc_clusters(0), clusters(NULL),
	verification(p->get_perform_verification()), distance_metric(p->get_distance_metric()) {

		uint16_t d = p->get_algorithm();

		if (d != 8 && d != 9 && d != 9 && d != 10 && d != 11 && d != 12 && d != 13 && d != 14 && d != 15) {
			printf("Invalid Algorithm ID\nPlease use\n\t08 : for Leader Clustering\n");
			printf("\t09 : for Agglomerative Clustering\n\t10 : for DBSCAN\n");
			printf("\t11 : for kMeans\n\t12 : for Spectral CLustering\n\t13 : for UPM/L2DV\n");
			printf("\t14: for GSDMM\n\t15: for Manual Clustering\n");

			printf("\n"); fflush(NULL);
			exit(-1);
		}

		this->lex = new TokensLexicon(1048576);

		this->num_entities = 0;
		this->num_alloc_entities = p->get_max_entities();
		this->e = new T * [this->num_alloc_entities]();
}

/// Destructor
template <class T, class U> ClusteringAlgorithm<T,U>::~ClusteringAlgorithm() {
	delete this->lex;
	if (this->e) {
		for (uint32_t i = 0; i < this->num_entities; i++) {
			this->e[i]->set_num_tokens(0);
			delete this->e[i];
		}
		delete [] this->e;
	}
}


/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// VERIFICATION STAGE : CLUSTERS' REFINEMENT ALGORITHM
/// ///////////////////////////////////////////////////////////////////////////////////////////////

/// Verification Stage
template <class T, class U> void ClusteringAlgorithm<T,U>::perform_verification(uint16_t u) {
	if (u) { printf(" - Cannot run verification on non-grouped entities"); }
}

/// Verification Stage
template <> void ClusteringAlgorithm<Product, EntitiesGroup<Product> >::perform_verification(uint16_t u) {
	uint32_t i = 0, j = 0, k = 0, t = 0, onum_clusters = 0;
	_score_t sim = 0.0, max_sim = 0.0;

	class Product * e, * rep;
	class EntitiesGroup<Product> * v;
	class Cluster< EntitiesGroup<Product> > * c, * r;

	printf("\tRunning Verification Stage..."); fflush(NULL);

	/// Initialize the Cluster of Deletions (CoD)
	class Cluster< EntitiesGroup<Product> > * CoD = new Cluster< EntitiesGroup<Product> >(0);
	CoD->create_empty_groups(this->num_vendors, this->v_ids);

	/// For each cluster, sort the vendors by ID, compute the clustroid and identify the products
	/// that are most similar to the clustroid.
	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i]->prepare(u);
	}

	/// Delete all (except 1) the products from violating vendors. Insert these products into CoD.
	for (i = 0; i < this->num_clusters; i++) {
		for (j = 0; j < this->clusters[i]->get_num_groups(); j++) {
			v = this->clusters[i]->get_group(j);

			if (v->get_num_entities() > 1) {
				for (k = 1; k < v->get_num_entities(); k++) {
					CoD->insert_entity_sorted(v->get_entity(k));
					this->clusters[i]->delete_entity(j, k);
				}
			}
		}
	}

	/// For each cluster, find the most similar product from each vendor: If the vendor is valid and
	/// the similarity exceeds the threshold, insert the product into the cluster.
	for (i = 0; i < this->num_clusters; i++) {
		c = this->clusters[i];
		rep = c->get_representative();

		if (rep) {
//			c->display();
			for (j = 0; j < this->num_vendors; j++) {
				v = CoD->get_group(j);

				if ( !c->has_group( v->get_group_key() ) ) {
					max_sim = 0.0;
					t = 0;
					e = NULL;

					for (k = 0; k < v->get_num_entities(); k++) {
						if (v->get_entity(k)) {
							sim = rep->cosine_similarity(v->get_entity(k));
	//						printf("%d == Vs == %d (Sim: %5.3f)\n", c->get_representative()->get_id(),
	//								v->get_entity(k)->get_id(), sim); fflush(NULL); getchar();
							if (sim > max_sim) {
								max_sim = sim;
								if (sim > SIMILARITY_THRESHOLD) {
									t = k;
									e = v->get_entity(k);
								}
							}
						}
					}

					if (e) {
						c->insert_entity(e);
						v->delete_entity(t);
					}
				}
			}
//			c->display(); getchar();
		}
	}

	/// Take care of the rest of the evicted products which were not inserted into a cluster. That
	/// is, we will create new clusters and insert the most similar products there.
	onum_clusters = this->num_clusters;

	for (j = 0; j < this->num_vendors; j++) {
		v = CoD->get_group(j);

		for (k = 0; k < v->get_num_entities(); k++) {
			if ( v->get_entity(k) ) {

				if (j == 0) {
					if (this->num_clusters >= this->num_alloc_clusters) {
						this->num_alloc_clusters *= 2;
						this->clusters = (class Cluster< EntitiesGroup<Product> > **)realloc
							(this->clusters, this->num_alloc_clusters * sizeof(class Cluster< EntitiesGroup<Product> > *));
					}

					this->clusters[this->num_clusters] = new Cluster< EntitiesGroup<Product> >(this->num_clusters);
					this->clusters[this->num_clusters]->insert_entity( v->get_entity(k) );
					this->clusters[this->num_clusters]->set_representative( v->get_entity(k) );
					this->num_clusters++;

				} else {
					r = NULL;
					max_sim = 0.0;

					for (i = onum_clusters; i < this->num_clusters; i++) {
						if (! this->clusters[i]->has_group( v->get_group_key() )) {
							sim = this->clusters[i]->get_representative()->cosine_similarity(v->get_entity(k));
//							printf("%s == Vs == %s ( Sim: %5.3f )\n", this->clusters[i]->get_representative()->get_text(),
//									v->get_entity(k)->get_text(), sim); getchar();

							if (sim > max_sim) {
								max_sim = sim;
								if (sim > SIMILARITY_THRESHOLD) {
									r = this->clusters[i];
								}
							}
						}
					}

					if (r) {
						r->insert_entity(v->get_entity(k));
					} else {
						if (this->num_clusters >= this->num_alloc_clusters) {
							this->num_alloc_clusters *= 2;
						this->clusters = (class Cluster< EntitiesGroup<Product> > **)realloc
							(this->clusters, this->num_alloc_clusters * sizeof(class Cluster< EntitiesGroup<Product> > *));
						}

						this->clusters[this->num_clusters] = new Cluster< EntitiesGroup<Product> >(this->num_clusters);
						this->clusters[this->num_clusters]->insert_entity( v->get_entity(k) );
						this->clusters[this->num_clusters]->set_representative( v->get_entity(k) );
						this->num_clusters++;
					}
				}
			}
			v->delete_entity(k);
		}
	}

	delete CoD;
}


/// MergeSplit Stage
template <class T, class U> void ClusteringAlgorithm<T,U>::MergeSplit() {
	uint32_t i = 0, j = 0, k = 0, t = 0, onum_clusters = 0;
	_score_t sim = 0.0, max_sim = 0.0;

	T * ety, * rep;
	class Cluster<T> * c = NULL, * cand = NULL, * CoD = new Cluster<T>(0);

	printf("\tRunning MergeSplit Stage..."); fflush(NULL);

	double Ts = 0.1, Tm = 0.1;

	/// For each cluster, sort the vendors by ID, compute the clustroid and identify the products
	/// that are most similar to the clustroid.
	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i]->compute_clustroid();
	}

	uint32_t deleted_entities = 0;

	/// Delete the elements from each cluster that are not very similar to the clustroid
	for (i = 0; i < this->num_clusters; i++) {
		c = this->clusters[i];
		rep = c->get_representative();

//		c->display();
		for (j = 0; j < c->get_num_entities(); j++) {
			ety = c->get_entity(j);

			if (ety) {
				sim = rep->cosine_similarity( ety );

				if (sim <= Ts) {
					CoD->insert_entity( ety );
					this->clusters[i]->delete_entity(j);
					deleted_entities++;
//					printf("Remove item %d : %d", j, ety->get_id()); getchar();
				}
			}
		}
	}
//	printf("Deleted Entities: %d\n", deleted_entities);

	/// Find a more suitable (similar) cluster for the deleted elements
	for (j = 0; j < CoD->get_num_entities(); j++) {
		max_sim = 0.0;
		cand = NULL;
		ety = CoD->get_entity(j);

		for (i = 0; i < this->num_clusters; i++) {
			sim = ety->cosine_similarity( this->clusters[i]->get_representative() );

			if (sim > max_sim) {
				k = j;
				max_sim = sim;
				cand = this->clusters[i];
			}
		}

		/// Insert the element in the cluster only if their similarity exceeds the threshold
		if (max_sim >= Tm && cand != NULL) {
			cand->insert_entity(ety);
			CoD->delete_entity(k);
			deleted_entities--;
		}
	}

//	for (i = 0; i < this->num_clusters; i++) {
//		this->clusters[i]->compute_clustroid();
//	}

	/// Take care of the rest of the evicted products which were not inserted into a cluster. That
	/// is, we will create new clusters and insert the most similar products there.
	onum_clusters = this->num_clusters;

	for (j = 0; j < CoD->get_num_entities(); j++) {
		ety = CoD->get_entity(j);

		if (ety) {
			if (this->num_clusters >= this->num_alloc_clusters) {
				this->num_alloc_clusters *= 2;
				this->clusters = (class Cluster<T> **)realloc
					(this->clusters, this->num_alloc_clusters * sizeof(class Cluster<T> *));
			}

			if (j == 0) {
				this->clusters[this->num_clusters] = new Cluster<T>(this->num_clusters);
				this->clusters[this->num_clusters]->insert_entity(ety);
				this->clusters[this->num_clusters]->set_representative(ety);
				this->num_clusters++;

			} else {
				cand = NULL;
				max_sim = 0.0;

				for (i = onum_clusters; i < this->num_clusters; i++) {
					sim = this->clusters[i]->get_representative()->cosine_similarity(ety);

					if (sim > max_sim) {
						max_sim = sim;
						if (sim >= Tm) {
							cand = this->clusters[i];
						}
					}
				}

				if (cand) {
					cand->insert_entity(ety);
				} else {
					this->clusters[this->num_clusters] = new Cluster<T>(this->num_clusters);
					this->clusters[this->num_clusters]->insert_entity(ety);
					this->clusters[this->num_clusters]->set_representative(ety);
					this->num_clusters++;
				}
			}

			CoD->delete_entity(j);
		}
	}

	delete CoD;
}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// ALGORITHM EVALUATION
/// ///////////////////////////////////////////////////////////////////////////////////////////////

/// Evaluation Wrapper
template <class T, class U> void ClusteringAlgorithm<T,U>::evaluate() {
	uint32_t i = 0;
	uint32_t low_t = this->PARAMS->get_low_threshold(), high_t = this->PARAMS->get_high_threshold();

	struct plot {
		double th;
		double f1;
		double nmi;
		double precision;
		double recall;
	} plots [high_t + 1];

	for (uint32_t i = low_t; i < high_t; i++) {
		plots[i].th = (double)(i + 1) / 10;
		plots[i].f1 = this->evaluate_f1(i + 1);
		plots[i].nmi = this->evaluators[i].get_nmi();
		plots[i].precision = this->evaluators[i].get_precision();
		plots[i].recall = this->evaluators[i].get_recall();
	}

	printf("\n\nPlots:");
	printf("\nPRC:\t");
	for (i = low_t; i < high_t; i++) { printf("(%2.1f,%4.3f)", plots[i].th, plots[i].precision); }

	printf("\nREC:\t");
	for (i = low_t; i < high_t; i++) { printf("(%2.1f,%4.3f)", plots[i].th, plots[i].recall); }

	printf("\nF1:\t");
	for (i = low_t; i < high_t; i++) { printf("(%2.1f,%4.3f)", plots[i].th, plots[i].f1); }

	printf("\nNMI:\t");
	for (i = low_t; i < high_t; i++) { printf("(%2.1f,%4.3f)", plots[i].th, plots[i].nmi); }
}

/// NMI Evaluation
template <class T, class U> double ClusteringAlgorithm<T,U>::evaluate_nmi(uint32_t k) {
	uint32_t i = 0;
	double nmi = 0.0, Hyc = 0.0, Hc = 0.0, Hy = this->evaluators[k].get_Hy();

	/// Entropy of Cluster labels
	for (i = 0; i < this->num_clusters; i++) {
		Hc += this->clusters[i]->compute_entropy( this->num_entities );

		Hyc += this->clusters[i]->compute_conditional_entropy( this->num_entities );
	}

//	printf("Hc: %5.3f, Hy: %5.3f, Hyc: %5.3f\n", Hc, Hy, Hyc);
	nmi = 2.0 * (Hy - Hyc) / ( Hy + Hc );

	this->evaluators[k].set_Hc(Hc);
	this->evaluators[k].set_Hyc(Hyc);
	this->evaluators[k].set_nmi(nmi);

	return nmi;
}
