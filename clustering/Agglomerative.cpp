#include "Agglomerative.h"

/// Agglomerative Clustering class Constructor: This calls the constructor of ClusteringAlgorithm<T,U>
/// which in turn calls the constructor of ClusteringMethod<T>
template <class T, class U> Agglomerative<T,U>::Agglomerative(class Settings * p) :
	ClusteringAlgorithm<T,U>(p),
	linkage_type( p->get_linkage() ) {

}

/// Destructor
template <class T, class U> Agglomerative<T,U>::~Agglomerative() {

}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Hierarchical Agglomerative Clustering Algorithm
template <class T, class U> void Agglomerative<T,U>::exec() {
	uint32_t i = 0, j = 0, k = 0;
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	int32_t x = 0, cand_1 = -1, cand_2 = -1;
	_score_t simt = 0.0, sim = 0.0, max_sim = 0.0;
	bool continue_merging = true;
	class Cluster<U> ** temp_clusters = NULL;

	struct max_similarity {
		int32_t num_cluster;
		_score_t max_sim;
	};

	this->initialize();

	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	/// For 10 similarity thresholds
	for (k = low_t; k < high_t; k++) {
#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &ts);
#endif

		simt = (_score_t)(k + 1) / 10;
		printf("\tSimilarity Threshold : %5.1f", simt); fflush(NULL);

		/// Agglomerative clustering: Initially each entity is placed in its own singleton cluster
		/// Namely, n_clusters=n_entities. These singleton clusters will progressively merge.
		this->num_clusters = this->num_entities;
		temp_clusters = new Cluster<U> * [this->num_entities];

		for (i = 0; i < this->num_entities; i++) {
			temp_clusters[i] = new Cluster<U>(i);
			temp_clusters[i]->insert_entity(this->e[i]);
			temp_clusters[i]->set_representative(this->e[i]);
		}

		/// An array which for each cluster, stores the most similar cluster and the similarity value
		struct max_similarity * max_similarities = (struct max_similarity *)malloc
				(this->num_entities * sizeof(struct max_similarity));

		for (i = 0; i < this->num_entities; i++) {
			max_similarities[i].num_cluster = -1;
			max_similarities[i].max_sim = 0.0;
		}

		/// Initially, for each (singleton) cluster compute the most similar (singleton) cluster
		for (i = 0; i < this->num_entities; i++) {
			max_sim = 0.0;
			for (j = 0; j < this->num_entities; j++) {
				if (i != j) {
					sim = temp_clusters[i]->compute_linkage(temp_clusters[j], this->linkage_type);
					if (sim > max_sim) {
						max_sim = sim;
						max_similarities[i].num_cluster = j;
						max_similarities[i].max_sim = max_sim;
					}
				}
			}
		}

		/// Now start merging the clusters
		continue_merging = true;
		while(continue_merging) {
			continue_merging = false;
			max_sim = 0.0;
			cand_1 = -1;
			cand_2 = -1;

			max_sim = 0.0;

			/// Find the pair of the most similar clusters...
			for (i = 0; i < this->num_entities; i++) {
				if (max_similarities[i].num_cluster >= 0 && max_similarities[i].max_sim >= max_sim) {
					if (temp_clusters[max_similarities[i].num_cluster]) {
						max_sim = max_similarities[i].max_sim;
						cand_1 = i;
						cand_2 = max_similarities[i].num_cluster;
					}
				}
			}

			/// ... and if their similarity exceeds the threshold...
			if (max_sim > simt) {
				continue_merging = true;

//				printf("Merging cluster %d (%d) with\ncluster %d (%d)\n\n", cand_1,
//					temp_clusters[cand_1]->get_representative()->get_id(), cand_2,
//					temp_clusters[cand_2]->get_representative()->get_id());

				/// ... merge them into one. Here we replace the first cluster of the pair with the
				/// merged one...
				temp_clusters[cand_1]->merge_with(temp_clusters[cand_2]);

				/// ... and we delete the second one...
				delete temp_clusters[cand_2];
				temp_clusters[cand_2] = NULL;
				max_similarities[cand_2].num_cluster = -1;
				max_similarities[cand_2].max_sim = 0.0;
				this->num_clusters--;

				/// For the new merged cluster, compute its distances (similarities) with all the
				/// other clusters and preserve only the most similar cluster.
				max_sim = 0.0;
				for (i = 0; i < this->num_entities; i++) {
					x = i;
					if (x != cand_1 && temp_clusters[i]) {
						sim = temp_clusters[cand_1]->compute_linkage(temp_clusters[i], this->linkage_type);
//						printf("Computing Similarity between %d and %d = %5.3f\n", cand_1, i, sim); fflush(NULL);

						if (sim >= max_sim) {
							max_sim = sim;
							max_similarities[cand_1].num_cluster = i;
							max_similarities[cand_1].max_sim = max_sim;
						}
					}
				}
//				printf("Max Sim: %5.3f\n", max_sim); getchar();
			}
		}

		/// Make the temporary clusters real clusters
		this->num_alloc_clusters = this->num_clusters;
		this->clusters = (class Cluster<U> **)malloc(this->num_alloc_clusters * sizeof(class Cluster<U> *));

		x = 0;
		for (i = 0; i < this->num_entities; i++) {
			if (temp_clusters[i]) {
				this->clusters[x++] = temp_clusters[i];
			}
		}

		/// Run the verification stage (products only)
		if (this->verification) {
			this->perform_verification(2);
		}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		this->evaluators[k].set_duration(duration);
		t_duration += duration;
		printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
		this->evaluators[k].set_duration(0);
		printf("\n"); fflush(NULL);
#endif

		/// Store the pairwise matches to the appropriate file
		this->evaluators[k].set_num_algo_clusters(this->num_clusters);
		this->evaluators[k].set_nmi( this->evaluate_nmi(k) );
		uint32_t num_matches = 0;

		for (i = 0; i < this->num_clusters; i++) {
			num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[k] );
			delete this->clusters[i];
		}

		this->evaluators[k].set_num_algo_matches(num_matches);
		free(this->clusters);

		delete [] temp_clusters;
		free(max_similarities);
	}

	this->finalize();
}
