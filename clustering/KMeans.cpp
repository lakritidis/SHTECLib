/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// k-Means family of clustering algorithms
/// The family includes three algorithms
/// 1. Standard k-Means (kmeans_type = 1)
/// 2. Standard k-Means, but with centroids replaced by clustroids (kmeans_type = 2)
/// 3. Spherical k-Means

#include "KMeans.h"
#include <map>
#include <vector>

/// kMeans Constructor: This calls the constructor of ClusteringAlgorithm<T,U> which in turn calls
/// the constructor of ClusteringMethod<T>
template <class T, class U> KMeans<T,U>::KMeans(class Settings * p) :
	ClusteringAlgorithm<T,U>(p),
	K_clusters(p->get_clusters()),
	iterations(p->get_iterations()),
	type(p->get_kmeans_type()) {

}

/// KMeans Destructor
template <class T, class U> KMeans<T,U>::~KMeans() { }

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// kMeans Clustering
template <class T, class U> void KMeans<T,U>::exec() {
	if (this->type == 1) {
		this->exec_standard();
	} else if (this->type == 2) {
		this->exec_standard_clustroids();
	} else if (this->type == 3) {
		this->exec_spherical();
	}
}

/// standard kMeans Clustering - with input vectors, centroids and Euclidean distance.
template <class T, class U> void KMeans<T,U>::exec_standard() {
	uint32_t i = 0, j = 0, found = 0, index = 0, iter = 0, cand = 0;
	_score_t dis = 0.0, min_distance = INIT_DISTANCE;

	this->initialize();

	printf("Running '%s' on '%s'...\nIteration: [ ", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
	clock_gettime(CLOCK_REALTIME, &ts);
#endif

	this->num_alloc_clusters = this->K_clusters;
	this->num_clusters = this->num_alloc_clusters;
	uint32_t random_entities[this->num_clusters];

	/// Clusters Initialization: Create K clusters, and then pick K random points. Make each point
	/// the centroid of a cluster.
	this->clusters = (class Cluster<U> **)malloc (this->num_clusters * sizeof(class Cluster<U> *));

	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i] = new Cluster<U>(i);
		while(true) {
			index = rand() % this->num_entities;

			found = 0;
			for (j = 0; j < i; j++) {
				if (random_entities[j] == index) {
					found = 1;
					break;
				}
			}

			if (found == 0) {
				random_entities[i] = index;
				this->clusters[i]->insert_entity(this->e[index]);
				break;
			}
		}
		this->clusters[i]->compute_centroid(1);
		this->clusters[i]->delete_entity(0);
	}

	/// Start iterations
	iter = 1;
	while(1) {
		printf("%d ", iter); fflush(NULL);

		/// Add all points to their nearest cluster
		for (i = 0; i < this->num_entities; i++) {

			cand = 0;
			min_distance = INIT_DISTANCE;

//			printf("Entity "); this->e[i]->display();

			/// Find the most proximal cluster
			for (j = 0; j < this->num_clusters; j++) {
//				this->clusters[j]->display();

				dis = this->e[i]->euclidean_distance(this->clusters[j]->get_representative());

				if (dis < min_distance) {
					min_distance = dis;
					cand = j;
				}
			}

//			printf("Most proximal Cluster: "); this->clusters[cand]->display(); getchar();
			this->clusters[cand]->insert_entity( this->e[i] );
		}

		/// Recalculating the center of each cluster
		for (j = 0; j < this->num_clusters; j++) {
			this->clusters[j]->compute_centroid(this->type);
		}

		/// In case the maximum number of iterations have been reached, exit;
		if (iter >= this->iterations) {
			break;
		/// Otherwise, prepare the clusters for the next iteration (i.e. delete all elements from
		/// each cluster. but preserve its centroid/clustroid)
		} else {
			for (j = 0; j < this->num_clusters; j++) {
				this->clusters[j]->empty(false);
			}
		}

		iter++;
	}

	/// Run the verification stage (products only)
	if (this->verification) {
		this->perform_verification(this->type);
	}

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	this->evaluators[0].set_duration(duration);
	t_duration += duration;
	printf("]\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
	this->evaluators[0].set_duration(0);
	printf("\n"); fflush(NULL);
#endif

	/// Store the pairwise matches to the appropriate file
	this->evaluators[0].set_num_algo_clusters(this->num_clusters);
	this->evaluators[0].set_nmi( this->evaluate_nmi(0) );
	uint32_t num_matches = 0;

	for (i = 0; i < this->num_clusters; i++) {
		num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[0] );
		delete this->clusters[i];
	}

	this->evaluators[0].set_num_algo_matches(num_matches);
	free(this->clusters);

	this->finalize();
}

/// standard kMeans Clustering with clustroids - with clustroids and cosine similarity
template <class T, class U> void KMeans<T,U>::exec_standard_clustroids() {
	uint32_t i = 0, j = 0, found = 0, index = 0, iter = 0, cand = 0;
	_score_t sim = 0.0, max_similarity = 0.0;

	this->initialize();

	printf("Running '%s (standard with Clustroids)' on '%s'...\nIteration: [ ",
		this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset()); fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
	clock_gettime(CLOCK_REALTIME, &ts);
#endif

	this->num_alloc_clusters = this->K_clusters;
	this->num_clusters = this->num_alloc_clusters;
	uint32_t random_entities[this->num_clusters];

	/// Clusters Initialization: Create K clusters, and then pick K random points. Make each point
	/// the centroid of a cluster.
	this->clusters = (class Cluster<U> **)malloc (this->num_clusters * sizeof(class Cluster<U> *));

	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i] = new Cluster<U>(i);
		while(true) {
			index = rand() % this->num_entities;

			found = 0;
			for (j = 0; j < i; j++) {
				if (random_entities[j] == index) {
					found = 1;
					break;
				}
			}

			if (found == 0) {
				random_entities[i] = index;
				this->clusters[i]->set_representative(this->e[index]);
				this->clusters[i]->insert_entity(this->e[index]);
//				this->clusters[i]->display();
				break;
			}
		}
		this->clusters[i]->compute_clustroid();
	}

	/// Start iterations
	iter = 1;
	while(1) {
		printf("%d ", iter); fflush(NULL);

		/// Add all points to their nearest cluster
		for (i = 0; i < this->num_entities; i++) {

			cand = 0;
			max_similarity = 0.0;
//			printf("Entity "); this->e[i]->display();

			/// Find the most similar cluster
			for (j = 0; j < this->num_clusters; j++) {
				sim = this->e[i]->cosine_similarity(this->clusters[j]->get_representative());

				if (sim > max_similarity) {
					max_similarity = sim;
					cand = j;
				}
			}

//			printf("Most proximal Cluster: "); this->clusters[cand]->display();
			if (this->e[i] != this->clusters[cand]->get_representative()) {
				this->clusters[cand]->insert_entity( this->e[i] );
			} else {
//				printf("== CLUSTER %d CANNOT INSERT Entity %d", cand, i); this->e[i]->display(); getchar();
			}
		}

		/// Recalculating the center of each cluster
		for (j = 0; j < this->num_clusters; j++) {
			this->clusters[j]->compute_clustroid();
		}

		/// In case the maximum number of iterations have been reached, exit;
		if (iter >= this->iterations) {
			break;
		/// Otherwise, prepare the clusters for the next iteration (i.e. delete all elements from
		/// each cluster. but preserve its centroid/clustroid)
		} else {
			for (j = 0; j < this->num_clusters; j++) {
				this->clusters[j]->empty(true);
			}
		}

		iter++;
	}

	if (this->verification) {
		this->perform_verification(this->type);
	}

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	this->evaluators[0].set_duration(duration);
	t_duration += duration;
	printf("]\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
	this->evaluators[0].set_duration(0);
	printf("\n"); fflush(NULL);
#endif

	/// Store the pairwise matches to the appropriate file
	this->evaluators[0].set_num_algo_clusters(this->num_clusters);
	this->evaluators[0].set_nmi( this->evaluate_nmi(0) );
	uint32_t num_matches = 0;

	for (i = 0; i < this->num_clusters; i++) {
		num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[0] );
		delete this->clusters[i];
	}

	this->evaluators[0].set_num_algo_matches(num_matches);
	free(this->clusters);

	this->finalize();
}

/// Spherical kMeans Clustering - With input vectors and centroids normalized to unit vectors and
/// cosine dissimilarity as distance function.
template <class T, class U> void KMeans<T,U>::exec_spherical() {
	uint32_t i = 0, j = 0, found = 0, index = 0, iter = 0, cand = 0;
	_score_t dis = 0.0, min_distance = INIT_DISTANCE;

	printf("Preprocessing...\n"); fflush(NULL);

	this->lex->compute_weights(this->num_entities);

	for (uint32_t i = 0; i < this->num_entities; i++) {
		this->e[i]->compute_token_scores();
		this->e[i]->normalize_feature_vector();
		this->e[i]->compute_entity_weight();
	}

	printf("Running 'Spherical k-Means' on '%s'\nIteration: [ ", this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
	clock_gettime(CLOCK_REALTIME, &ts);
#endif

	this->num_alloc_clusters = this->K_clusters;
	this->num_clusters = this->num_alloc_clusters;
	uint32_t random_entities[this->num_clusters];

	/// Clusters Initialization: Create K clusters, and then pick K random points. Make each point
	/// the centroid of a cluster.
	this->clusters = (class Cluster<U> **)malloc (this->num_clusters * sizeof(class Cluster<U> *));

	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i] = new Cluster<U>(i);
		while(true) {
			index = rand() % this->num_entities;

			found = 0;
			for (j = 0; j < i; j++) {
				if (random_entities[j] == index) {
					found = 1;
					break;
				}
			}

			if (found == 0) {
				random_entities[i] = index;
				this->clusters[i]->insert_entity(this->e[index]);
				break;
			}
		}
		this->clusters[i]->compute_centroid(this->type);
		this->clusters[i]->delete_entity(0);
	}

	/// Start iterations
	iter = 1;
	while(1) {
		printf("%d ", iter); fflush(NULL);

		/// Add all points to their nearest cluster
		for (i = 0; i < this->num_entities; i++) {

			cand = 0;
			min_distance = INIT_DISTANCE;

//			printf("Entity "); this->e[i]->display();

			/// Find the most proximal cluster
			for (j = 0; j < this->num_clusters; j++) {
//				this->clusters[j]->display();

				/// Spherical clustering employs cosine dissimilarity
				dis = 1.0 - this->e[i]->cosine_similarity(this->clusters[j]->get_representative());

				if (dis < min_distance) {
					min_distance = dis;
					cand = j;
				}
			}

//			printf("Most proximal Cluster %d: ", cand); this->clusters[cand]->display(); getchar();
			this->clusters[cand]->insert_entity( this->e[i] );
		}

		/// Recalculating the center of each cluster
		for (j = 0; j < this->num_clusters; j++) {
			this->clusters[j]->compute_centroid(this->type);
		}

		/// In case the maximum number of iterations have been reached, exit;
		if (iter >= this->iterations) {
			break;
		/// Otherwise, prepare the clusters for the next iteration (i.e. delete all elements from
		/// each cluster. but preserve its centroid/clustroid)
		} else {
			for (j = 0; j < this->num_clusters; j++) {
				this->clusters[j]->empty(false);
			}
		}

		iter++;
	}

	/// Run the verification stage (products only)
	if (this->verification) {
		this->perform_verification(this->type);
	}

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	t_duration += duration;
	this->evaluators[0].set_duration(duration);
	printf("]\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
	this->evaluators[0].set_duration(0);
	printf("]\n"); fflush(NULL);
#endif

	/// Store the pairwise matches to the appropriate file
	this->evaluators[0].set_num_algo_clusters(this->num_clusters);
	this->evaluators[0].set_nmi( this->evaluate_nmi(0) );
	uint32_t num_matches = 0;

	for (i = 0; i < this->num_clusters; i++) {
		num_matches += this->clusters[i]->create_pairwise_matches( this->output_files[0] );
		delete this->clusters[i];
	}

	this->evaluators[0].set_num_algo_matches(num_matches);
	free(this->clusters);

	this->finalize();
}
