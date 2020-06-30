#include "UPM.h"

/// UPM class Constructor: This calls the constructor of ClusteringAlgorithm<T,U> which in turn
/// calls the constructor of ClusteringMethod<T>
UPM::UPM(class Settings * p) :
	ClusteringAlgorithm<UPMProduct, EntitiesGroup<UPMProduct> >(p),
	tokens_index(NULL),
	units_lexicon(new TokensLexicon(256)),
	combinations_lexicon(new CombinationsLexicon(16 * 1048576)),
	K(p->get_upm_k()),
	alpha(p->get_upm_a()),
	beta(p->get_upm_b()) {

		this->units_lexicon->load_units();
}

/// UPM Destructor
UPM::~UPM() {
	if (this->combinations_lexicon) {
		delete this->combinations_lexicon;
	}

	if (this->tokens_index) {
		delete [] this->tokens_index;
	}

	if (this->units_lexicon) {
		delete this->units_lexicon;
	}
}


/// Specialization : Read the products from an input file
void UPM::read_from_file(char * filepath) {
	uint32_t nread = 0, product_id = 0, len = 0, v_id = 0, nven = 100, f = 0, c_id = 0;
	uint32_t fl = strlen(filepath), cbuflen = 0, abuflen = 1048576, block_size = 4096, field = 0, i = 0, x = 0;
	char buf[8192], e_id_c[16], title[4096], c_id_c[512], c_label[4096];

	this->v_ids = (uint32_t *)malloc(nven * sizeof(uint32_t));

	FILE *fp = fopen(filepath, "rb");
	if (fp) {

		char * lbuf = (char*)malloc(abuflen * sizeof(char));

		this->eval_clusters = new ClusterUniverse<UPMProduct>(8192);

		/// ///////////////////////////////////////////////////////////////////////////////////////
		/// Parse the file as CSV
		if ((filepath[fl-4] == '.' && filepath[fl-3] == 'c' && filepath[fl-2] == 's' && filepath[fl-1] == 'v') ||
			(filepath[fl-4] == '.' && filepath[fl-3] == 'C' && filepath[fl-2] == 'S' && filepath[fl-1] == 'V')) {

			while (!feof(fp)) {
				nread += fread(lbuf + cbuflen, sizeof(char), block_size, fp);
				cbuflen += block_size;
				if (cbuflen >= abuflen) {
					abuflen *= 2;
					lbuf = (char *)realloc(lbuf, abuflen * sizeof(char));
				}
			}
			lbuf[nread] = 0;

            for (i = 0; i < nread; i++) {
				if (lbuf[i] == 9) {
					if (field == 0) { e_id_c[x] = 0; product_id = atoi(e_id_c); }
					if (field == 1) { title[x] = 0; len = x; }
					if (field == 5) { buf[x] = 0; }

					x = 0;
					field++;

				} else if (lbuf[i] == 10) {
					this->e[this->num_entities] = new UPMProduct();
					this->e[this->num_entities]->set_id(product_id);
					this->e[this->num_entities]->tokenize(title, len, this->lex, this->stoplist, this->stats);

					field = 0;
					this->num_entities++;
					if (this->num_entities >= this->num_alloc_entities) { break; }
				}
				if (field == 0) { if (x < 16) { e_id_c[x++] = lbuf[i]; } }
				if (field == 1) { if (x < 4096) { title[x++] = lbuf[i]; } }
				if (field == 5) { if (x < 8192) { buf[x++] = lbuf[i]; } }
            }


		/// ///////////////////////////////////////////////////////////////////////////////////////
		/// Parse the file as XML
		} else
		if ((filepath[fl-4] == '.' && filepath[fl-3] == 'x' && filepath[fl-2] == 'm' && filepath[fl-1] == 'l') ||
			(filepath[fl-4] == '.' && filepath[fl-3] == 'X' && filepath[fl-2] == 'M' && filepath[fl-1] == 'L')) {

			while (!feof(fp)) {
				nread += fread(lbuf + cbuflen, sizeof(char), block_size, fp);
				cbuflen += block_size;

				if (cbuflen >= abuflen) {
					abuflen *= 2;
					lbuf = (char *)realloc(lbuf, abuflen * sizeof(char));
				}
			}
			lbuf[nread] = 0;

            for (i = 0; i < nread; i++) {
				/// Create a new Entity as UPMProduct
				if (strncmp(lbuf + i, "<entity>", 8) == 0) {
					i += 8;
					this->e[this->num_entities] = new UPMProduct();
				}

				/// Read the ID of the UPMProduct
				if (strncmp(lbuf + i, "<id>", 4) == 0) {
					i += 4; x = 0;
					while(strncmp(lbuf + i, "</id>", 5) != 0) { e_id_c[x++] = lbuf[i++]; }
					i += 5;
					e_id_c[x] = 0;
					product_id = atoi(e_id_c);

					this->e[this->num_entities]->set_id(product_id);
				}

				/// Read the title of the UPMProduct and tokenize it
				if (strncmp(lbuf + i, "<title><![CDATA[", 16) == 0) {
					i += 16; x = 0;
					while(strncmp(lbuf + i, "]]></title>", 11) != 0) { title[x++] = lbuf[i++]; }
					i += 11;
					title[x] = 0;

					this->e[this->num_entities]->tokenize(title, x, this->lex, this->stoplist, this->stats);
				}

				/// Read the vendor of the UPMProduct
				if (strncmp(lbuf + i, "<vendor>", 8) == 0) {
					i += 8; x = 0;
					while(strncmp(lbuf + i, "</vendor>", 9) != 0) { buf[x++] = lbuf[i++]; }
					i += 9;
					buf[x] = 0;

					v_id = atoi(buf);

					f = 0;
					for (uint32_t j = 0; j < this->num_vendors; j++) { if (this->v_ids[j] == v_id) {
						f = 1; break;
					} }

					if (f == 0) { this->v_ids[this->num_vendors++] = v_id;
						if (this->num_vendors >= nven) {
							nven *= 2;
							this->v_ids = (uint32_t *)realloc(this->v_ids, nven * sizeof(uint32_t));
						}
					}

					this->e[this->num_entities]->set_vendor_id(v_id);
				}

				/// Read the ID of the cluster where the UPMProduct really belongs
				if (strncmp(lbuf + i, "<cluster_id><![CDATA[", 21) == 0) {
					i += 21; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_id>", 16) != 0) { c_id_c[x++] = lbuf[i++]; }
					i += 16;
					c_id_c[x] = 0;
					c_id = atoi(c_id_c);
				}

				/// Read the label of the cluster where the UPMProduct really belongs
				if (strncmp(lbuf + i, "<cluster_label><![CDATA[", 24) == 0) {
					i += 24; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_label>", 19) != 0) { c_label[x++] = lbuf[i++]; }
					i += 19;
					c_label[x] = 0;
				}

				if (strncmp(lbuf + i, "</entity>", 9) == 0) {
					i += 9;
//						printf("ID: %d -- TITLE: %s -- VENDOR: %d -- REAL CLUSTER: %d, %s\n",
//							product_id, title, v_id, c_id, c_label); fflush(NULL); // getchar();

					this->eval_clusters->insert_cluster(c_id, c_label, this->e[this->num_entities]);
					this->num_entities++;
					if (this->num_entities >= this->num_alloc_entities) { break; }
				}
			}
		}

		qsort(this->v_ids, this->num_vendors, sizeof(uint32_t), cmp_int);

		double Hy = this->eval_clusters->compute_entropy( this->num_entities );
		this->evaluators[0].set_Hy(Hy);

		free(lbuf);
		fclose(fp);
	} else {
		printf("Error Opening %s file...\n", filepath); fflush(NULL);
		exit(-1);
	}
}

/// Execute UPM
void UPM::exec() {
	uint32_t i = 0, num_slots = 1048576;

	this->initialize();

#ifdef __linux__
	struct timespec ts, te;
	clock_gettime(CLOCK_REALTIME, &ts);
	double duration = 0.0;
#endif

	/// Convert TokensLexicon to a perfect hash table for constant O(1) time searches by token ID.
	this->tokens_index = this->lex->reform(this->num_entities, this->stats);

	/// Create the combinations from each product title
	printf("Computing k-Combinations...\n"); fflush(NULL);
	for (i = 0; i < this->num_entities; i++) {
		this->e[i]->construct_combinations(this->K, this->combinations_lexicon, this->stats);
		this->e[i]->sort_feature_vector();
	}

	/// Compute the scores of the combinations; identify the dominating combination and declare it
	/// as the cluster of the product.
	printf("Running '%s' on '%s'...\n", this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset());
	fflush(NULL);

	this->cluster_universe = new ClusterUniverse<EntitiesGroup<UPMProduct> >(num_slots);

	this->stats->compute_final_values();

	for (i = 0; i < this->num_entities; i++) {
		this->e[i]->score_combinations(this->PARAMS, this->stats, this->tokens_index);
		this->cluster_universe->insert_node(this->e[i]);
	}

	this->num_clusters = this->cluster_universe->get_num_nodes();
	this->num_alloc_clusters = this->num_clusters;
	this->clusters = this->cluster_universe->convert_to_array();

	/// Make UPM a ClusteringAlgorithm to run its verification stage
	class ClusteringAlgorithm<Product, EntitiesGroup<Product> > * cal =
		(class ClusteringAlgorithm<Product, EntitiesGroup<Product> > *)this;

	/// Run the verification stage (products only)
	if (this->verification) {
		cal->perform_verification(2);
	}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		this->evaluators[0].set_duration(duration);
		printf("\t -> %10.3f sec\n", duration); fflush(NULL);
#else
		this->evaluators[0].set_duration(0);
		printf("\n"); fflush(NULL);
#endif

	/// Store the pairwise matches to the appropriate file
	this->evaluators[0].set_num_algo_clusters(this->num_clusters);
	this->evaluators[0].set_nmi( this->evaluate_nmi(0) );
	uint32_t num_matches = 0;

	for (i = 0; i < this->num_clusters; i++) {
		num_matches += this->clusters[i]->create_pairwise_matches (this->output_files[0]);
		delete this->clusters[i];
	}

	this->evaluators[0].set_num_algo_matches(num_matches);
	free(this->clusters);

	delete this->cluster_universe;

	this->finalize();
}

