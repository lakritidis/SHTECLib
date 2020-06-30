#include "Settings.h"

/// Constructor: Set default execution values
Settings::Settings() :
	algorithm_name{'\0'}, distance_metric(2),
	algorithm(6), dataset("uci_news_agg_1.xml"),
	dataset_path{'\0'}, results_path{'\0'}, max_entities(1048576),
	similarity_threshold(0.5), min_distance(1000.0), num_zones(6), low_threshold(0), high_threshold(9),
	entities_type(1),
	upm_k(2), upm_a(1.0), upm_b(1.0),
	gsdmm_a(0.1), gsdmm_b(0.1),
	clusters(200), minpoints(2), iterations(10), linkage(3), kmeans_type(3),
	perform_verification(true), spectral_autotune(false), verbose(true),
	supported_algorithms(), supported_distances() {

#ifdef __linux__
		strcpy(this->dataset_path, "/home/leo/Desktop/datasets/clustering/");
		strcpy(this->results_path, "/home/leo/Desktop/Clustering/results/");
#elif _WIN32
		strcpy(this->dataset_path, "C:/Users/Leon/Documents/datasets/TextClustering/");
		strcpy(this->results_path, "C:/Users/Leon/Documents/Clustering/results/");
#endif

		/// k-Means, UPM/L2DV, and GSDMM are not affected by similarity thresholds.
		if (this->algorithm == 11 || this->algorithm == 13 || this->algorithm == 14 || this->algorithm == 15) {
			this->low_threshold = 0;
			this->high_threshold = 1;
		}

		this->supported_algorithms.push_back("cosine");		/// 1
		this->supported_algorithms.push_back("jaccard");	/// 2
		this->supported_algorithms.push_back("dice");		/// 3
		this->supported_algorithms.push_back("edit");		/// 4
		this->supported_algorithms.push_back("wcosine");	/// 5
		this->supported_algorithms.push_back("wjaccard");	/// 6
		this->supported_algorithms.push_back("wdice");		/// 7
		this->supported_algorithms.push_back("leader");		/// 8
		this->supported_algorithms.push_back("agglomerative");	/// 9
		this->supported_algorithms.push_back("dbscan");		/// 10
		this->supported_algorithms.push_back("kmeans");		/// 11
		this->supported_algorithms.push_back("spectral");	/// 12
		if (this->entities_type == 1) {						/// 13
			this->supported_algorithms.push_back("l2dv"); } else {
			this->supported_algorithms.push_back("upm"); }
		this->supported_algorithms.push_back("gsdmm");		/// 14
		this->supported_algorithms.push_back("manual");		/// 15

		strcpy(this->algorithm_name, supported_algorithms[this->algorithm - 1].c_str());

		this->supported_distances.push_back("euclidean");
		this->supported_distances.push_back("w-cosine");

		/// Switch between normal Entities ( = 1) and Products ( = 2).
		if (this->entities_type == 1) {
			this->perform_verification = false;
		}
}

/// Destructor
Settings::~Settings() {
}

void Settings::read_from_input(int argc, char ** argv) {

	/// ///////////////////////////////////////////////////////////////////////////////////////////
	/// STEP 1: PREPARE THE VARIABLES OF USER-DEFINED PARAMETERS
	TCLAP::CmdLine cmd("Text Clustering Library", ' ', "0.9");

	/// Algorithm (Literal)
	TCLAP::ValuesConstraint<std::string> allowedAlgs(supported_algorithms);
	TCLAP::ValueArg<std::string> AlgArg("a","algorithm",
		"The clustering algorithm to execute", true, this->algorithm_name, &allowedAlgs, cmd);

	/// Distance Metric (Literal)
	TCLAP::ValuesConstraint<std::string> allowedDiss(supported_distances);
	TCLAP::ValueArg<std::string> DisArg("D","distance",
		"The distance metric to apply", true, "w-cosine", &allowedDiss, cmd);

	/// Dataset
	TCLAP::ValueArg<std::string> DatasetArg("d", "dataset", "Dataset", true, "cpus", "string", cmd);

	/// Paths: Dataset & Results
	TCLAP::ValueArg<std::string> DatasetpathArg("p", "dataset-path",
		"Path to input dataset", false, this->dataset_path, "string", cmd);
	TCLAP::ValueArg<std::string> ResultspathArg("r", "results-path",
		"Path to directory where the results are stored", false, this->results_path, "string", cmd);

	/// The maximum number of input objects to process
	TCLAP::ValueArg<int32_t> MaxentitiesArg("e", "max-entities",
		"Maximum number of input objects to process", false, this->max_entities, "int", cmd);

	/// The similarity threshold which determines if two entities match or not (Verification Stage)
	TCLAP::ValueArg<_score_t> SimTArg("t", "verif-sim-threshold",
		"The similarity threshold determines if two entities match or not (Refinement Stage)",
		false, this->similarity_threshold, "double", cmd);

	/// Execute the algorithm for a range of similarity thresholds: Low/High threshold (Range start/end)
	TCLAP::ValueArg<int32_t> LowSimTArg("l", "low-sim-threshold",
		"Execute the algorithm for a range of similarity thresholds: Low threshold (Range start)",
		false, this->low_threshold, "int", cmd);
	TCLAP::ValueArg<int32_t> HighSimTArg("k", "high-sim-threshold",
		"Execute the algorithm for a range of similarity thresholds: High threshold (Range end)",
		false, this->high_threshold, "int", cmd);

	/// Hyperparameters
	TCLAP::ValueArg<_score_t> upmaArg("A", "upm-a", "Hyperparameter: UPM a",
		false, this->upm_a, "double", cmd);
	TCLAP::ValueArg<_score_t> upmbArg("B", "upm-b", "Hyperparameter: UPM b",
		false, this->upm_b, "double", cmd);
	TCLAP::ValueArg<_score_t> gsdmmaArg("GA", "gsdmm-a", "Hyperparameter: GSDMM a",
		false, this->gsdmm_a, "double", cmd);
	TCLAP::ValueArg<_score_t> gsdmmbArg("GB", "gsdmm-b", "Hyperparameter: GSDMM b",
		false, this->gsdmm_b, "double", cmd);
	TCLAP::ValueArg<uint32_t> clusArg("C", "clusters", "Hyperparameter: Number of clusters",
		false, this->clusters, "int", cmd);
	TCLAP::ValueArg<uint32_t> mptsArg("M", "minpoints", "Hyperparameter: DBSCAN minpoints",
		false, this->minpoints, "int", cmd);
	TCLAP::ValueArg<uint16_t> iterArg("I", "iterations", "Hyperparameter: Number of iterations",
		false, this->iterations, "short", cmd);
	TCLAP::ValueArg<uint16_t> linkArg("L", "linkage", "Hyperparameter: Linkage Type",
		false, this->linkage, "short", cmd);
	TCLAP::ValueArg<uint16_t> kMTArg("u", "kmeans-type", "Type of k-Means algorithm",
		false, this->kmeans_type, "short", cmd);

	/// Switches
	TCLAP::SwitchArg VerifSwitch("v", "verify", "Execute Clustering Verification", cmd, false);
	TCLAP::SwitchArg ATSwitch("s", "autotune", "Self-tuning Spectral Clustering", cmd, false);
	TCLAP::SwitchArg VerSwitch("V", "verbose", "Print detailed messages", cmd, false);

	/// ///////////////////////////////////////////////////////////////////////////////////////////
	/// STEP 2: READ THE USER PARAMETERS (COMMENT TO IGNORE AND USE THE DEFAULT VALUES)
	cmd.parse( argc, argv );


	/// ///////////////////////////////////////////////////////////////////////////////////////////
	/// 3. MOVE THE USER-DEFINED PARAMETERS TO LOCAL VARIABLES
	std::vector<std::string>::iterator it;
	it = std::find(supported_algorithms.begin(), supported_algorithms.end(), AlgArg.getValue().c_str());
	this->algorithm = std::distance(supported_algorithms.begin(), it) + 1;
	strcpy(this->algorithm_name, AlgArg.getValue().c_str());

	it = std::find(supported_distances.begin(), supported_distances.end(), DisArg.getValue().c_str());
	this->distance_metric = std::distance(supported_distances.begin(), it) + 1;

	strcpy(this->dataset, DatasetArg.getValue().c_str());
	strcpy(this->dataset_path, DatasetpathArg.getValue().c_str());
	strcpy(this->results_path, ResultspathArg.getValue().c_str());
	this->max_entities = MaxentitiesArg.getValue();
	this->similarity_threshold = SimTArg.getValue();
	this->low_threshold = LowSimTArg.getValue();
	this->high_threshold = HighSimTArg.getValue();

	this->upm_a = upmaArg.getValue();
	this->upm_b = upmbArg.getValue();
	this->clusters = clusArg.getValue();
	this->iterations = iterArg.getValue();
	this->minpoints = mptsArg.getValue();
	this->linkage = linkArg.getValue();
	this->kmeans_type = kMTArg.getValue();

	this->perform_verification = VerifSwitch.getValue();
	this->spectral_autotune = ATSwitch.getValue();
	this->verbose = VerSwitch.getValue();

	printf("Algorithm: %d (%s)\n", this->algorithm, AlgArg.getValue().c_str());
	printf("Distance Metric: %d (%s)\n", this->distance_metric, DisArg.getValue().c_str());
	printf("Dataset: %s\nDataset Path: %s\n",this->dataset, this->dataset_path);
	printf("Results Path: %s\nMax Entities: %d\n", this->results_path, this->max_entities);
	printf("Sim-Threshold: %5.3f\n", this->similarity_threshold);
	printf("Low-Threshold: %d\nHigh-Threshold: %d\n", this->low_threshold, this->high_threshold);

	printf("UPM K: %d\nUPM a: %5.3f\nUPM b: %5.3f\n", this->upm_k, this->upm_a, this->upm_b);
	printf("GSDMM a: %5.3f\nGSDMM b: %5.3f\n", this->gsdmm_a, this->gsdmm_b);
	printf("Clusters K: %d\n", this->clusters);
	printf("Iterations: %d\nMin-Points: %d\n", this->iterations, this->minpoints);
	printf("Linkage: %d\nk-Means Type: %d\n", this->linkage, this->kmeans_type);
	printf("Verification: %d\nSpectral Autotune: %d\n", this->perform_verification, this->spectral_autotune);
}

/// Mutators
void Settings::set_algorithm(uint16_t v) { this->algorithm = v; }
void Settings::set_algorithm_name(char * v) { strcpy(this->algorithm_name, v); }
void Settings::set_distance_metric(uint16_t v) { this->distance_metric = v; }
void Settings::set_dataset(char * v) { strcpy(this->dataset, v); }
void Settings::set_dataset_path(char * v) { strcpy(this->dataset_path, v); }
void Settings::set_results_path(char * v) { strcpy(this->results_path, v); }
void Settings::set_max_entities(uint32_t v) { this->max_entities = v; }
void Settings::set_similarity_threshold(_score_t v) { this->similarity_threshold = v; }
void Settings::set_min_distance(_score_t v) { this->min_distance = v; }
void Settings::set_num_zones(uint16_t v) { this->num_zones = v; }
void Settings::set_low_threshold(uint16_t v) { this->low_threshold = v; }
void Settings::set_high_threshold(uint16_t v) { this->high_threshold = v; }
void Settings::set_entities_type(uint16_t v) { this->entities_type = v; }
void Settings::set_upm_k(uint16_t v) { this->upm_k = v; }
void Settings::set_upm_a(_score_t v) { this->upm_a = v; }
void Settings::set_upm_b(_score_t v) { this->upm_b = v; }
void Settings::set_gsdmm_a(_score_t v) { this->gsdmm_a = v; }
void Settings::set_gsdmm_b(_score_t v) { this->gsdmm_b = v; }
void Settings::set_clusters(uint32_t v) { this->clusters = v; }
void Settings::set_minpoints(uint32_t v) { this->minpoints = v; }
void Settings::set_iterations(uint16_t v) { this->iterations = v; }
void Settings::set_linkage(uint16_t v) { this->linkage = v; }
void Settings::set_perform_verification(bool v) { this->perform_verification = v; }
void Settings::set_kmeans_type(uint16_t v) { this->kmeans_type = v; }
void Settings::set_spectral_autotune(bool v) { this->spectral_autotune = v; }
void Settings::set_verbose(bool v) { this->verbose = v; }

/// Accessors
uint16_t Settings::get_algorithm() { return this->algorithm; }
char * Settings::get_algorithm_name() { return this->algorithm_name; }
uint16_t Settings::get_distance_metric() { return this->distance_metric; }
char * Settings::get_dataset() { return this->dataset; }
char * Settings::get_dataset_path() { return this->dataset_path; }
char * Settings::get_results_path() { return this->results_path; }
uint32_t Settings::get_max_entities() { return this->max_entities; }
_score_t Settings::get_similarity_threshold() { return this->similarity_threshold; }
_score_t Settings::get_min_distance() { return this->min_distance; }
uint16_t Settings::get_num_zones() { return this->num_zones; }
uint16_t Settings::get_low_threshold() { return this->low_threshold; }
uint16_t Settings::get_high_threshold() { return this->high_threshold; }
uint16_t Settings::get_entities_type() { return this->entities_type; }
uint16_t Settings::get_upm_k() { return this->upm_k; }
_score_t Settings::get_upm_a() { return this->upm_a; }
_score_t Settings::get_upm_b() { return this->upm_b; }
_score_t Settings::get_gsdmm_a() { return this->gsdmm_a; }
_score_t Settings::get_gsdmm_b() { return this->gsdmm_b; }
uint32_t Settings::get_clusters() { return this->clusters; }
uint32_t Settings::get_minpoints() { return this->minpoints; }
uint16_t Settings::get_iterations() { return this->iterations; }
uint16_t Settings::get_linkage() { return this->linkage; }
bool Settings::get_perform_verification() { return this->perform_verification; }
uint16_t Settings::get_kmeans_type() { return this->kmeans_type; }
bool Settings::get_spectral_autotune() { return this->spectral_autotune; }
bool Settings::get_verbose() { return this->verbose; }

