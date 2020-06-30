#include "SpectralClustering.h"
#include <iterator>

/// Spectral Clustering class Constructor: This calls the constructor of ClusteringAlgorithm<T,U>
/// which in turn calls the constructor of ClusteringMethod<T>
template <class T, class U> SpectralClustering<T,U>::SpectralClustering (class Settings * p) :
	ClusteringAlgorithm<T,U>(p),
	mEigenVectors(),
	iterations(p->get_iterations()),
	K_clusters(p->get_clusters()),
	autotune(p->get_spectral_autotune()) {

}

/// Destructor
template <class T, class U> SpectralClustering<T,U>::~SpectralClustering() {

}

/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Spectral Clustering Algorithm
template <class T, class U> void SpectralClustering<T,U>::exec() {
	uint32_t i = 0, j = 0, n = 0, data_cols = 0, data_rows = 0;
	_score_t sim = 0, d = 0;

	printf("Preprocessing Entities...\n"); fflush(NULL);

	this->lex->compute_weights(this->num_entities);

	for (i = 0; i < this->num_entities; i++) {
		this->e[i]->compute_weight();
	}

	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	/// Construct the affinity matrix. Here, the knowledge graph is a full=connected graph: each
	/// entity is connected to all other entities - i.e. its degree this->num_entities - 1.
	printf("\tConstructing Knowledge Graph... "); fflush(NULL);
    Eigen::MatrixXd data = Eigen::MatrixXd::Zero(this->num_entities, this->num_entities);

	for (i = 0; i < this->num_entities; i++) {
		for (j = 0; j < this->num_entities; j++) {
			d = this->e[i]->euclidean_distance( this->e[j] );
			sim = exp(-d * d / 100);
			data(i, j) = sim;
			data(j, i) = sim;
		}
	}
	printf("\tOK\n"); fflush(NULL);

	/// Diagonal Degree Matrix
	printf("\tComputing the Laplacian Matrix..."); fflush(NULL);
	data_cols = data.cols();
	data_rows = data.rows();

	Eigen::MatrixXd Deg = Eigen::MatrixXd::Zero(data_rows, data_cols);

	for (i = 0; i < data_cols; i++) {
		Deg(i, i) = 1 / (sqrt( ( data.row(i).sum() )) );
	}

	/// Normalized Laplacian Matrix
	Eigen::MatrixXd Laplacian = Deg * data * Deg;

	printf("\tOK\n"); fflush(NULL);

	printf("\tPerforming Eigendecomposition..."); fflush(NULL);
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(Laplacian, true);
	Eigen::VectorXd EigenVals = solver.eigenvalues();
	Eigen::MatrixXd EigenVecs = solver.eigenvectors();

	/// Sort eigenvalues/vectors
	n = data.cols();
	for (i = 0; i < n - 1; ++i) {
		int32_t k;
		EigenVals.segment(i, n - i).maxCoeff(&k);
		if (k > 0) {
			std::swap(EigenVals[i], EigenVals[k + i]);
			EigenVecs.col(i).swap(EigenVecs.col(k + i));
		}
	}

	/// Set the number of eigenvectors to consider
	int32_t mNumDims = this->K_clusters;
//	int32_t mNumDims = this->num_entities;
	if (mNumDims < EigenVecs.cols()) {
		mEigenVectors = EigenVecs.block(0, 0, EigenVecs.rows(), mNumDims);
	} else {
		mEigenVectors = EigenVecs;
	}
	printf("\tOK\n"); fflush(NULL);

	std::vector<std::vector<int> > temp_clusters;
	if (this->autotune) {
		/// Auto-tuning clustering
		printf("\tPerforming Self-Tuning...\t\t"); fflush(NULL);
		temp_clusters = this->clusterRotate();
	} else {
		/// How many clusters you want
		printf("\tRunning kMeans Clustering..."); fflush(NULL);
		temp_clusters = this->clusterKmeans(this->K_clusters);
	}

	this->num_clusters = temp_clusters.size();
	this->num_alloc_clusters = this->num_clusters;
	this->clusters = new Cluster<U> * [this->num_clusters];

	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i] = new class Cluster<U>(i);

		for (j = 0; j < temp_clusters[i].size(); j++) {
			this->clusters[i]->insert_entity( this->e [ temp_clusters[i][j] ] );
			this->clusters[i]->set_representative( this->e [ temp_clusters[i][j] ] );
//			std::copy(temp_clusters[i].begin(), temp_clusters[i].end(), std::ostream_iterator<int>(std::cout, ", "));
//			std::cout << std::endl;
		}
//		this->clusters[i]->display();
    }
	printf("\tOK\n"); fflush(NULL);

	if (this->verification) {
		this->perform_verification(2);
	}

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	this->evaluators[0].set_duration(duration);
	t_duration += duration;
	printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
	this->evaluators[0].set_duration(duration);
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
	delete [] this->clusters;

	this->finalize();
}


/// Self-tuning implementation with cluster rotations
template <class T, class U> std::vector<std::vector<int32_t> > SpectralClustering<T,U>::clusterRotate() {
	ClusterRotate * c = new ClusterRotate();
	std::vector<std::vector<int> > clusters = c->cluster(this->mEigenVectors);
	this->num_clusters = clusters.size();

	return clusters;
}

/// K-Means implementation for sparse vectors
template <class T, class U> std::vector<std::vector<int32_t> > SpectralClustering<T,U>::clusterKmeans(int32_t ncentres) {
	Eigen::MatrixXd& data = this->mEigenVectors;
	int ndims = data.cols();
	int ndata = data.rows();

	//int ncentres = 2;
	Eigen::MatrixXd centres = Eigen::MatrixXd::Zero(ncentres, ndims);
	Eigen::MatrixXd old_centres;

	std::vector<int> rands;
	for (int i=0; i < ncentres; i++) {
		//randomly initialise centers
		bool flag;
		do {
			flag = false;
			int randIndex = Eigen::ei_random(0,ndata-1);
			//make sure same row not chosen twice
			for (unsigned int j=0; j < rands.size(); ++j) {
				if (randIndex == rands[j]) {
					flag = true;
					break;
				}
			}
			if (!flag) {
				centres.row(i) = data.row(randIndex);
				rands.push_back(randIndex);
			}
		} while (flag);
	}
	Eigen::MatrixXd id = Eigen::MatrixXd::Identity(ncentres, ncentres);
	//maps vectors to centres.
	Eigen::MatrixXd post(ndata, ncentres);

	Eigen::VectorXd minvals(ndata);

	double old_e = 0;
	int niters = 100;
	for (int n=0; n < niters; n++) {
		//Save old centres to check for termination
		old_centres = centres;

		// Calculate posteriors based on existing centres
		Eigen::MatrixXd d2(ndata, ncentres);
		for (int j = 0; j < ncentres; j++) {
			for (int k=0; k < ndata; k++) {
				d2(k,j) = (data.row(k)-centres.row(j)).squaredNorm();
			}
		}

		int r,c;
		// Assign each point to nearest centre
		for (int k=0; k < ndata; k++) {
			//get centre index (c)
			minvals[k] = d2.row(k).minCoeff(&r, &c);
			//set centre
			post.row(k) = id.row(c);
		}

		Eigen::VectorXd num_points = post.colwise().sum();
		// Adjust the centres based on new posteriors
		for (int j = 0; j < ncentres; j++) {
			if (num_points[j] > 0) {
				Eigen::MatrixXd s = Eigen::MatrixXd::Zero(1,ndims);
				for (int k=0; k<ndata; k++) {
					if (post(k,j) == 1) {
						s += data.row(k);
					}
				}
				centres.row(j) = s / num_points[j];
			}
		}

		// Error value is total squared distance from cluster centres
		double e = minvals.sum();
		double ediff = fabs(old_e - e);
		double cdiff = (centres-old_centres).cwise().abs().maxCoeff();
		std::cout << "Cycle " << n << " Error " << e << " Movement " << cdiff << ", " << ediff << std::endl;

		if (n > 1) {
			//Test for termination
			if (cdiff < 0.0000000001 && ediff < 0.0000000001) {
				break;
			}
		}
		old_e = e;
	}

	//------- finished kmeans ---------

	//find the item closest to the centre for each cluster
	std::vector<std::vector<int> > clustered_items;
	for (int j=0; j < ncentres; j++) {
		//put data into map (multimap because minvals[k] could be the same for multiple units)
		std::multimap<double,int> cluster;
		for (int k=0; k < ndata; k++) {
			if (post(k,j) == 1) {
				cluster.insert(std::make_pair(minvals[k], k));
			}
		}
		//extract data from map
		std::vector<int> units;
		//the map will be sorted based on the key (the minval) so just loop through it
		//to get set of data indices sorted on the minval
		for (std::multimap<double,int>::iterator it = cluster.begin(); it != cluster.end(); it++) {
			units.push_back(it->second);
		}

		clustered_items.push_back(units);
	}
	//return the ordered set of item indices for each cluster centre
	return clustered_items;
}
