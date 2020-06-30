#include "GSDMM.h"

/// Gibbs Sampling on Dirichlet Mixture Model (GSDMM) class Constructor: This calls the constructor
/// of ClusteringAlgorithm<T,U> which in turn calls the constructor of ClusteringMethod<T>.
template <class T, class U> GSDMM<T,U>::GSDMM(class Settings * p) :
	ClusteringAlgorithm<T,U>(p),
	iterations(p->get_iterations()),
	K_clusters(p->get_clusters()),
	alpha(p->get_gsdmm_a()), beta(p->get_gsdmm_b()) {

}

/// Destructor
template <class T, class U> GSDMM<T,U>::~GSDMM() {

}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// GSDMM short text clustering Algorithm
/// ///////////////////////////////////////////////////////////////////////////////////////////////
template <class T, class U> void GSDMM<T,U>::exec() {
	uint32_t i = 0, j = 0, k = 0, cluster_index = 0, iter = 0, total_transfers = 0, record_index = 0;

	double * prob_distr = new double [this->K_clusters];
	uint32_t * distr_samples = new uint32_t [this->K_clusters], cluster_count_new = 0, cluster_count = 0;

	class Cluster<T> ** temp_clusters = NULL;
	class EntityCluster<T> ** entities_info = new EntityCluster<T> * [this->num_entities];

	/// Initialize GSL and random number generator for Gibbs sampling from multinomial distribution
	gsl_rng_env_setup();
	gsl_rng * r = gsl_rng_alloc(gsl_rng_default);

	this->initialize();

	for (i = 0; i < this->num_entities; i++) {
		entities_info[i] = new EntityCluster<T> (this->e[i]);
	}

	printf("Running '%s' on '%s'...\nIteration: [ ", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
	clock_gettime(CLOCK_REALTIME, &ts);
#endif

	/// Initialize the clusters & the probability distribution.
	this->num_clusters = this->K_clusters;
	temp_clusters = new class Cluster<T> * [this->K_clusters];
	for (i = 0; i < this->K_clusters; i++) {
		prob_distr[i] = 1.0 / (double)this->K_clusters;
		distr_samples[i] = 0;
		temp_clusters[i] = new Cluster<T>(i);
	}

	/// Randomly assign the entities to clusters.
	for (i = 0; i < this->num_entities; i++) {
		cluster_index = this->sample(r, prob_distr, distr_samples);
		record_index = temp_clusters[cluster_index]->insert_entity(this->e[i], true);
		temp_clusters[cluster_index]->set_representative(this->e[i]);

		/// Store the cluster and the index of the entity within that cluster
		entities_info[i]->set_cluster(temp_clusters[cluster_index], record_index - 1);
	}

	cluster_count_new = this->get_num_nonempty_clusters(temp_clusters);

	/// Start iterations
	iter = 1;
	while(1) {
		printf("%d ", iter); fflush(NULL);

		total_transfers = 0;

		/// For each entity
		for (i = 0; i < this->num_entities; i++) {

			/// Remove the entity from it's current cluster.
//printf("Deleting entity %d from Cluster %d (Index %d)\n", i, entities_info[i]->get_cluster()->get_id(), entities_info[i]->get_index());
//entities_info[i]->get_cluster()->display();
			entities_info[i]->get_cluster()->delete_entity( entities_info[i]->get_index() );
//entities_info[i]->get_cluster()->display(); getchar();

			/// Draw sample from distribution to find new cluster.
			this->score(entities_info[i]->get_entity(), prob_distr, temp_clusters);
			cluster_index = this->sample(r, prob_distr, distr_samples);

			/// Transfer entity to the new cluster.
			record_index = temp_clusters[cluster_index]->insert_entity(this->e[i], true);

			if (entities_info[i]->get_cluster() == temp_clusters[cluster_index]) {
				total_transfers++;
			}

// temp_clusters[cluster_index]->display();
			entities_info[i]->set_cluster( temp_clusters[cluster_index], record_index - 1);
// printf("Inserted to Cluster %d (Index %d)\n", cluster_index, record_index - 1); getchar();
// temp_clusters[cluster_index]->display(); getchar();
		}

		cluster_count_new = this->get_num_nonempty_clusters(temp_clusters);

//		printf("In stage %d: transferred %d clusters with %d clusters populated\n",
//			iter, total_transfers, cluster_count_new);

		/// In case the maximum number of iterations have been reached, exit;
		if (iter >= this->iterations || (iter > 25 && cluster_count == cluster_count_new && total_transfers == 0) ) {
			break;
		}

		cluster_count = cluster_count_new;
		iter++;
	}


	/// Create the normal clusters by copying the non-empty temp clusters. Then, delete temp clusters.
	this->num_alloc_clusters = this->get_num_nonempty_clusters(temp_clusters);
	this->clusters = (class Cluster<U> **) malloc(this->num_alloc_clusters * sizeof(class Cluster<U> *));

	j = 0;
	for (i = 0; i < this->num_clusters; i++) {
		if (temp_clusters[i]->get_non_deleted_entities() > 0) {
			this->clusters[j] = new Cluster<U>(j);

			for (k = 0; k < temp_clusters[i]->get_num_entities(); k++) {
				if (temp_clusters[i]->get_entity(k)) {
					this->clusters[j]->insert_entity(temp_clusters[i]->get_entity(k));
					this->clusters[j]->set_representative(temp_clusters[i]->get_entity(k));
				}
			}
			j++;
		}
		delete temp_clusters[i];
	}
	delete [] temp_clusters;

	this->num_clusters = this->num_alloc_clusters;

	/// Run the verification stage (products only)
	if (this->verification) {
		this->perform_verification(2);
	}

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	this->evaluators[0].set_duration(duration);
	t_duration += duration;
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

	for (i = 0; i < this->num_entities; i++) {
		delete entities_info[i];
	}

	delete [] entities_info;
	delete [] prob_distr;
	delete [] distr_samples;

	gsl_rng_free(r);

	this->finalize();
}

/// Count the non empty clusters passed in the argument
template <class T, class U> uint32_t GSDMM<T,U>::get_num_nonempty_clusters(class Cluster<T> ** tc) {
	uint32_t num_nonempty_clusters = 0;
	for (uint32_t j = 0; j < this->num_clusters; j++) {
		if (tc[j]->get_non_deleted_entities() > 0) {
			num_nonempty_clusters++;
		}
	}
	return num_nonempty_clusters;
}

/// Sample with probability vector p from a multinomial distribution: p is a list of probabilities
/// representing probability vector for the multinomial distribution. It returns the index of a
/// randomly selected output
template <class T, class U> uint32_t GSDMM<T,U>::sample(const gsl_rng * r, double * p, uint32_t * s) {

	uint32_t i = 0, ret_index = 0;
	gsl_ran_multinomial(r, this->K_clusters, 1, p, s);

//	printf("["); for (i = 0; i < this->K_clusters; i++) { printf(" %e", p[i]); } printf("]\n");
//	printf("["); for (i = 0; i < this->K_clusters; i++) { printf(" %d", s[i]); } printf("]\n");

	for (i = 0; i < this->K_clusters; i++) {
		if (s[i] != 0) {
			ret_index = i;
		}
	}

	return ret_index;
}

/// Score a document. Implements formula (3) of Yin and Wang 2014.
/// http://dbgroup.cs.tsinghua.edu.cn/wangjy/papers/KDD14-GSDMM.pdf
template <class T, class U> void GSDMM<T,U>::score(T * entity, double * p, class Cluster<T> ** clus) {

	uint32_t i = 0, j = 0, tok_freq = 0, cluster_words = 0;
	double lD1 = 0.0, lD2 = 0.0, lN1 = 0.0, lN2 = 0.0, pnorm = 0.0;

	uint32_t doc_size = entity->get_num_tokens();
	class TokensLexicon * cl_tok_distr = NULL;
	class Token * tok = NULL;

	/// Reset the probability vector
	for (i = 0; i < this->K_clusters; i++) { p[i] = 0.0; }

	/// Compute the probability vector
	/// p = N1 * N2 / (D1 * D2) = exp(lN1 - lD1 + lN2 - lD2)
	/// lN1 = log(m_z[z] + alpha)
	/// lD1 = log(D - 1 + K*alpha)
	/// lN2 = log(product(n_z_w[w] + beta)) = sum(log(n_z_w[w] + beta))
	/// lD2 = log(product(n_z[d] + V*beta + i -1)) = sum(log(n_z[d] + V*beta + i -1))
	lD1 = log(this->num_entities - 1.0 + this->K_clusters * this->alpha);
//	printf("lD1 = %5.3f\n", lD1);

	for (i = 0; i < this->K_clusters; i++) {
//		clus[i]->display();
		lN1 = log(clus[i]->get_non_deleted_entities() + this->alpha);
		lN2 = 0.0;
		lD2 = 0.0;

		cl_tok_distr = clus[i]->get_token_distribution();
		if (cl_tok_distr) {
			cluster_words = cl_tok_distr->get_num_tokens();
		} else {
			cluster_words = 0;
		}

//		printf("Word Count: %d, lN1 = %5.5f\n", cluster_words, lN1);

		for (j = 0; j < doc_size; j++) {

			if (cl_tok_distr) {
				tok = cl_tok_distr->get_node( entity->get_token(j)->get_str() );
				if (tok) {
					tok_freq = tok->get_freq();
				} else {
					tok_freq = 0;
				}
			} else {
				tok_freq = 0;
			}

			lN2 += log(tok_freq + this->beta);
			lD2 += log(cluster_words + this->lex->get_num_nodes() * this->beta + j);

//			printf("\tWord %d/%d=%s, Frequency=%d, lN2=%5.5f, lD2=%5.5f\n",
//				j+1, cluster_words, entity->get_token(j)->get_str(), tok_freq, lN2, lD2);
		}

		p[i] = exp(lN1 - lD1 + lN2 - lD2);
		pnorm += p[i];
//		printf("\n\nlD1=%5.5f, lN1=%5.5f, lD2=%5.5f, lN2=%5.5f, p=%e\n", lD1, lN1, lD2, lN2, p[i]);
	}

	/// Normalize the probability vector
	if (pnorm > 0) {
		for (i = 0; i < this->K_clusters; i++) {
			p[i] = p[i] / pnorm;
		}
	}
}
