#include "ClusteringMethod.h"

/// Constructor 2
template <class T> ClusteringMethod<T>::ClusteringMethod(class Settings * p) :
	id (p->get_algorithm()), PARAMS(p), lex(NULL),
	stoplist(new TokensLexicon(256)),
	stats(new Statistics()),
	num_entities(0), num_alloc_entities(0),
	e(NULL),
	num_vendors(0), v_ids(NULL),
	eval_clusters(NULL),
	csim_matches(NULL),
	output_files{NULL},
	evaluators(){

		printf("Initializing ...\n"); fflush(NULL);
		char buf[4096], null_t[] = "null";
		uint32_t null_len = 4;
		uint32_t low_t = this->PARAMS->get_low_threshold();
		uint32_t high_t = this->PARAMS->get_high_threshold();

		this->stoplist->load_stopwords();

		this->csim_matches = (struct matches **)malloc(high_t * sizeof(matches *));

		for (uint32_t i = low_t; i < high_t; i++) {
			sprintf(buf, "%s%s_%s_%d_matches", this->PARAMS->get_results_path(),
				this->PARAMS->get_dataset(), this->PARAMS->get_algorithm_name(), i + 1);
			this->output_files[i] = fopen(buf, "wb");

			fwrite(&null_len, sizeof(uint32_t), 1, this->output_files[i]);
			fwrite(&null_t, sizeof(char), null_len, this->output_files[i]);
			fwrite(&i, sizeof(uint32_t), 1, this->output_files[i]);

			this->csim_matches[i] = (struct matches *)malloc(sizeof(struct matches));
			this->csim_matches[i]->num_matches = 0;
			this->csim_matches[i]->num_alloc_matches = 1048576;
			this->csim_matches[i]->matches_array = (struct match **)malloc
					(this->csim_matches[i]->num_alloc_matches * sizeof(struct match *));

			for (uint32_t j = 0; j < this->csim_matches[i]->num_alloc_matches; j++) {
				this->csim_matches[i]->matches_array[j] = (struct match *)malloc(sizeof(struct match));
			}
		}
}

/// Destructor
template <class T> ClusteringMethod<T>::~ClusteringMethod() {
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	for (uint32_t i = low_t; i < high_t; i++) {
		for (uint32_t j = 0; j < this->csim_matches[i]->num_alloc_matches; j++) {
			free(this->csim_matches[i]->matches_array[j]);
		}
		free(this->csim_matches[i]->matches_array);
		free(this->csim_matches[i]);
	}
	free(this->csim_matches);

	if (this->num_vendors > 0) {
		if (this->v_ids) {
			free(this->v_ids);
		}
	}

	if (this->stats) {
		delete this->stats;
	}

	if (this->stoplist) {
		delete this->stoplist;
	}

	if (this->eval_clusters) {
		this->eval_clusters->delete_nodes();
		delete this->eval_clusters;
	}
}

/// Read the entities from an input file
template <class T> void ClusteringMethod<T>::read_from_file(char * filepath) {
	uint32_t nread = 0, e_id = 0, len = 0, c_id = 0, res = 0;
	uint32_t fl = strlen(filepath), cbuflen = 0, abuflen = 1048576, block_size = 4096, field = 0, i = 0, x = 0;
	char buf[8192], e_id_c[16], title[4096], c_id_c[512], c_label[4096];

	T * candidate = NULL;

	FILE * fp = fopen(filepath, "rb");
	if (fp) {
		printf("Reading & Vectorizing Dataset '%s'...\n", this->PARAMS->get_dataset()); fflush(NULL);

		char * lbuf = (char*)malloc(abuflen * sizeof(char));
		this->eval_clusters = new ClusterUniverse<T>(8192);

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
					if (field == 0) { e_id_c[x] = 0; e_id = atoi(e_id_c); }
					if (field == 1) { title[x] = 0; len = x; }
					if (field == 5) { buf[x] = 0; }

					x = 0;
					field++;

				} else if (lbuf[i] == 10) {
					this->e[this->num_entities] = new T();
					this->e[this->num_entities]->set_id(e_id);
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
				if (strncmp(lbuf + i, "<entity>", 8) == 0) {
					i += 8;
					candidate = new T();
				}

				/// Read the ID of T
				if (strncmp(lbuf + i, "<id>", 4) == 0) {
					i += 4; x = 0;
					while(strncmp(lbuf + i, "</id>", 5) != 0) { e_id_c[x++] = lbuf[i++]; }
					i += 5;
					e_id_c[x] = 0;
					e_id = atoi(e_id_c);

					candidate->set_id(e_id);
				}

				/// Read the title of T
				if (strncmp(lbuf + i, "<title><![CDATA[", 16) == 0) {
					i += 16; x = 0;
					while(strncmp(lbuf + i, "]]></title>", 11) != 0) { title[x++] = lbuf[i++]; }
					i += 11;
					title[x] = 0;

					if (x > 0) {
						res = candidate->tokenize(title, x, this->lex, this->stoplist, this->stats);
					} else {
						res = 0;
					}
				}

				/// Read the ID of the cluster where T really belongs
				if (strncmp(lbuf + i, "<cluster_id><![CDATA[", 21) == 0) {
					i += 21; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_id>", 16) != 0) { c_id_c[x++] = lbuf[i++]; }
					i += 16;
					c_id_c[x] = 0;
					c_id = atoi(c_id_c);
					candidate->set_real_cluster_id(c_id);
				}

				/// Read the label of the cluster where T really belongs
				if (strncmp(lbuf + i, "<cluster_label><![CDATA[", 24) == 0) {
					i += 24; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_label>", 19) != 0) { c_label[x++] = lbuf[i++]; }
					i += 19;
					c_label[x] = 0;
				}

				if (strncmp(lbuf + i, "</entity>", 9) == 0) {
					i += 9;
//						printf("ID: %d -- TITLE: %s -- REAL CLUSTER: %d, %s\n",
//							e_id, title, c_id, c_label); fflush(NULL); getchar();
					if (res > 0) {
						this->e[this->num_entities] = candidate;
						this->eval_clusters->insert_cluster(c_id, c_label, this->e[this->num_entities]);
						this->num_entities++;
					} else {
						delete candidate;
						candidate = NULL;
					}
					res = 0;
					if (this->num_entities >= this->num_alloc_entities) { break; }
				}
			}
		}

		double Hy = this->eval_clusters->compute_entropy( this->num_entities );
		for (uint32_t k = 0; k < 10; k++) {
			this->evaluators[k].set_Hy(Hy);
		}

		free(lbuf);
		fclose(fp);
	} else {
		printf("Error Opening %s file...\n", filepath); fflush(NULL);
		exit(-1);
	}
}

/// ////////////////////////////////////////////////////////////////////////////////////////////////
/// Specialization : Read the products from an input file
template <> void ClusteringMethod<Product>::read_from_file(char * filepath) {
	uint32_t nread = 0, e_id = 0, len = 0, v_id = 0, nven = 100, f = 0, c_id = 0, res = 0;
	uint32_t fl = strlen(filepath), cbuflen = 0, abuflen = 1048576, block_size = 4096, field = 0, i = 0, x = 0;
	char buf[8192], e_id_c[16], title[4096], c_id_c[512], c_label[4096];

	class Product * candidate = NULL;

	this->v_ids = (uint32_t *)malloc(nven * sizeof(uint32_t));

	FILE *fp = fopen(filepath, "rb");
	if (fp) {
		printf("Reading & Vectorizing Dataset '%s'...\n", this->PARAMS->get_dataset()); fflush(NULL);

		char * lbuf = (char*)malloc(abuflen * sizeof(char));

		this->eval_clusters = new ClusterUniverse<Product>(8192);

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
					if (field == 0) { e_id_c[x] = 0; e_id = atoi(e_id_c); }
					if (field == 1) { title[x] = 0; len = x; }
					if (field == 5) { buf[x] = 0; }

					x = 0;
					field++;

				} else if (lbuf[i] == 10) {
					this->e[this->num_entities] = new Product();
					this->e[this->num_entities]->set_id(e_id);
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
				if (strncmp(lbuf + i, "<entity>", 8) == 0) {
					i += 8;
					candidate = new Product();
				}

				/// Read the ID of the Product
				if (strncmp(lbuf + i, "<id>", 4) == 0) {
					i += 4; x = 0;
					while(strncmp(lbuf + i, "</id>", 5) != 0) { e_id_c[x++] = lbuf[i++]; }
					i += 5;
					e_id_c[x] = 0;
					e_id = atoi(e_id_c);

					candidate->set_id(e_id);
				}

				/// Read the title of the Product
				if (strncmp(lbuf + i, "<title><![CDATA[", 16) == 0) {
					i += 16; x = 0;
					while(strncmp(lbuf + i, "]]></title>", 11) != 0) { title[x++] = lbuf[i++]; }
					i += 11;
					title[x] = 0;

					if (x > 0) {
						res = candidate->tokenize(title, x, this->lex, this->stoplist, this->stats);
					} else {
						res = 0;
					}
				}

				/// Read the vendor of the Product
				if (strncmp(lbuf + i, "<vendor>", 8) == 0) {
					i += 8; x = 0;
					while(strncmp(lbuf + i, "</vendor>", 9) != 0) { buf[x++] = lbuf[i++]; }
					i += 9;
					buf[x] = 0;

					v_id = atoi(buf);

					f = 0;
					for (uint32_t j = 0; j < this->num_vendors; j++) {
						if (this->v_ids[j] == v_id) {
							f = 1;
							break;
						}
					}

					if (f == 0) { this->v_ids[this->num_vendors++] = v_id;
						if (this->num_vendors >= nven) {
							nven *= 2;
							this->v_ids = (uint32_t *)realloc(this->v_ids, nven * sizeof(uint32_t));
						}
					}

					candidate->set_vendor_id(v_id);
				}

				/// Read the ID of the cluster where the Product really belongs
				if (strncmp(lbuf + i, "<cluster_id><![CDATA[", 21) == 0) {
					i += 21; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_id>", 16) != 0) { c_id_c[x++] = lbuf[i++]; }
					i += 16;
					c_id_c[x] = 0;
					c_id = atoi(c_id_c);
					candidate->set_real_cluster_id(c_id);
				}

				/// Read the label of the cluster where the product really belongs
				if (strncmp(lbuf + i, "<cluster_label><![CDATA[", 24) == 0) {
					i += 24; x = 0;
					while(strncmp(lbuf + i, "]]></cluster_label>", 19) != 0) { c_label[x++] = lbuf[i++]; }
					i += 19;
					c_label[x] = 0;
				}

				if (strncmp(lbuf + i, "</entity>", 9) == 0) {
					i += 9;
//						printf("ID: %d -- TITLE: %s -- REAL CLUSTER: %d, %s\n",
//							e_id, title, c_id, c_label); fflush(NULL); getchar();
					if (res > 0) {
						this->e[this->num_entities] = candidate;
						this->eval_clusters->insert_cluster(c_id, c_label, this->e[this->num_entities]);
						this->num_entities++;
					} else {
						delete candidate;
						candidate = NULL;
					}
					res = 0;
					if (this->num_entities >= this->num_alloc_entities) { break; }
				}
			}
		}

		qsort(this->v_ids, this->num_vendors, sizeof(uint32_t), cmp_int);

		double Hy = this->eval_clusters->compute_entropy( this->num_entities );
		for (uint32_t k = 0; k < 10; k++) {
			this->evaluators[k].set_Hy(Hy);
		}

		free(lbuf);
		fclose(fp);
	} else {
		printf("Error Opening %s file...\n", filepath); fflush(NULL);
		exit(-1);
	}
}

template <class T> uint32_t ClusteringMethod<T>::factorial(uint32_t n, uint32_t k) {
	uint32_t ret = 1;
	for (uint32_t i = 0; i < k; i++) {
		ret *= (n - i);
	}
	return ret;
}

/// Initialize a clustering method: Compute feature vector, feature weights and entity weights
template <class T> void ClusteringMethod<T>::initialize() {
	printf("Preprocessing...\n"); fflush(NULL);

	this->lex->compute_weights(this->num_entities);

	for (uint32_t i = 0; i < this->num_entities; i++) {
		this->e[i]->compute_token_scores();
		this->e[i]->compute_entity_weight();
	}
}

/// Finalize procedure: write the remaining matches to disk
template <class T> void ClusteringMethod<T>::finalize() {
	uint32_t null_len = 4, i = 0, nc = 0;
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	for (i = low_t; i < high_t; i++) {
		/// Write the remaining matches which are still in memory
		for (uint32_t j = 0; j < csim_matches[i]->num_matches; j++) {
			fwrite( &(this->csim_matches[i]->matches_array[j]->e1_id), sizeof(uint32_t), 1, this->output_files[i]);
			fwrite( &(this->csim_matches[i]->matches_array[j]->e2_id), sizeof(uint32_t), 1, this->output_files[i]);
		}
		nc = this->evaluators[i].get_num_algo_clusters();
		fwrite( &nc, sizeof(uint32_t), 1, this->output_files[i]);

		/// Go to the beginning of the file and write the number of matches
		fseek(this->output_files[i], sizeof(uint32_t) + null_len * sizeof(char), SEEK_SET);
		nc = this->evaluators[i].get_num_algo_matches();
		fwrite( &nc, sizeof(uint32_t), 1, this->output_files[i]);
		fclose(this->output_files[i]);
	}
}

/// Insert a pairwise similarity value between two entities into the intimate matches array
template <class T> void ClusteringMethod<T>::insert_match(uint32_t e1, uint32_t e2, _score_t sim) {
#ifndef EFFICIENCY_TESTS
	_score_t simt = 0.0;
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	/// For 10 similarity thresholds
	for (uint32_t k = low_t; k < high_t; k++) {
		/// Similarity threshold
		simt = (_score_t)(k + 1) / 10;

		if (sim >= simt) {
			this->evaluators[k].increase_num_algo_matches();
			this->csim_matches[k]->matches_array[ this->csim_matches[k]->num_matches ]->e1_id = e1;
			this->csim_matches[k]->matches_array[ this->csim_matches[k]->num_matches ]->e2_id = e2;
			this->csim_matches[k]->num_matches++;

			/// In case the array becomes full, write it to disk in the appropriate file
			if (this->csim_matches[k]->num_matches >= this->csim_matches[k]->num_alloc_matches) {
				for (uint32_t x = 0; x < this->csim_matches[k]->num_matches; x++) {
					fwrite( &(csim_matches[k]->matches_array[x]->e1_id), sizeof(uint32_t), 1, this->output_files[k]);
					fwrite( &(csim_matches[k]->matches_array[x]->e2_id), sizeof(uint32_t), 1, this->output_files[k]);
				}
				this->csim_matches[k]->num_matches = 0;
			}
		}
	}
#endif
}

/// Evaluation Wrapper
template <class T> void ClusteringMethod<T>::evaluate_f1() {
	uint32_t low_t = this->PARAMS->get_low_threshold();
	uint32_t high_t = this->PARAMS->get_high_threshold();

	struct plot {
		double th;
		double f1;
	} plots [high_t + 1];

	for (uint32_t i = low_t; i < high_t; i++) {
		plots[i].th = (double)(i + 1) / 10;
		plots[i].f1 = this->evaluate_f1(i + 1);
	}

	printf("\n\nF1 Plot:\n");
	for (uint32_t i = low_t; i < high_t; i++) {
		printf("(%2.1f,%4.3f)", plots[i].th, plots[i].f1);
	}
	printf("\n\n");
}

/// Evaluation process for a given similarity threshold k
template <class T> double ClusteringMethod<T>::evaluate_f1(uint32_t k) {
	printf("\nStarting evaluation...\n"); fflush(NULL);
	char filepath[4096], cfilepath[4096];

	sprintf(filepath, "%s%s_matches", this->PARAMS->get_dataset_path(), this->PARAMS->get_dataset());
	sprintf(cfilepath, "%s%s_%s_%d_matches", this->PARAMS->get_results_path(),
		this->PARAMS->get_dataset(), this->PARAMS->get_algorithm_name(), k);

	uint32_t num_alloc_corr_matches = 65536, num_alloc_algo_matches = 65536;
	uint32_t num_algo_matches = 0, num_corr_matches = 0;
	uint32_t num_algo_clusters = 0, num_corr_clusters = this->eval_clusters->get_num_nodes();
	uint32_t i = 0, j = 0, nread = 0, len = 0, num = 0, eloop = 0;
	char buf[8192];

	struct match ** corr_matches = this->eval_clusters->create_pairwise_matches( &num_corr_matches );

	printf("Constructed %d correct matches (%d allocated)\n", num_corr_matches, num_alloc_corr_matches);
	qsort(corr_matches, (size_t)num_corr_matches, sizeof(match *), ClusteringMethod::cmp_matches);

	/// Read the algorithm matches
	struct match ** algo_matches = (struct match **)malloc(num_alloc_algo_matches * sizeof(struct match *));

	FILE *algorithm_matches_file = fopen(cfilepath, "rb");
	if (algorithm_matches_file) {
		while (!feof(algorithm_matches_file)) {
			nread = fread(&len, sizeof(uint32_t), 1, algorithm_matches_file);
			if (nread == 0) {
				break;
			}
			nread = fread(buf, sizeof(char), len, algorithm_matches_file);
			buf[len] = 0;
			nread = fread(&num, sizeof(uint32_t), 1, algorithm_matches_file);

//			printf("%d. %s (num: %d)\n", len, buf, num);
//			getchar();
			eloop = num;

			for (i = 0; i < eloop; i++) {
				algo_matches[num_algo_matches] = (struct match *)malloc(sizeof(struct match));

				nread = fread(&algo_matches[num_algo_matches]->e1_id, sizeof(uint32_t), 1, algorithm_matches_file);
				nread = fread(&algo_matches[num_algo_matches]->e2_id, sizeof(uint32_t), 1, algorithm_matches_file);

				num_algo_matches++;
				if (num_algo_matches >= num_alloc_algo_matches) {
					num_alloc_algo_matches *= 2;
					algo_matches = (struct match **)realloc(algo_matches, num_alloc_algo_matches * sizeof(struct match *));
				}
			}
			nread = fread(&num_algo_clusters, sizeof(uint32_t), 1, algorithm_matches_file);
		}
		fclose(algorithm_matches_file);
	} else {
		printf("error opening algorithm matches file: %s\n", cfilepath);
		exit(-1);
	}
	printf("Read %d algorithm matches (%d allocated)\n", num_algo_matches, num_alloc_algo_matches);
	qsort(algo_matches, (size_t)num_algo_matches, sizeof(struct match *), cmp_matches);

//	for (i = 0; i < num_algo_matches; i++) {
//		printf("%d. (%d,%d)\n", i, algo_matches[i]->p1_id, algo_matches[i]->p2_id);
//	}

	printf("Computing Correct Matches...\n"); fflush(NULL);
	uint32_t correct_algo_matches = 0;

	i = 0;
	j = 0;
	while (i < num_corr_matches) {
//		printf("Checking match %d (%d,%d)...\n", i, corr_matches[i]->p1_id, corr_matches[i]->p2_id);
//		getchar();

		while (algo_matches[j]->e1_id < corr_matches[i]->e1_id || (algo_matches[j]->e1_id == corr_matches[i]->e1_id && algo_matches[j]->e2_id <= corr_matches[i]->e2_id)) {
//			printf("\tPosition: %d (%d,%d)\n", j, algo_matches[j]->p1_id, algo_matches[j]->p2_id);
			if ((corr_matches[i]->e1_id == algo_matches[j]->e1_id && corr_matches[i]->e2_id == algo_matches[j]->e2_id) ||
				(corr_matches[i]->e1_id == algo_matches[j]->e2_id && corr_matches[i]->e2_id == algo_matches[j]->e1_id) ) {
					correct_algo_matches++;
//					printf("\tCorrect Match %d: [%d, %d] - [%d, %d]\n\n", correct_algo_matches,
//						corr_matches[i]->p1_id, corr_matches[i]->p2_id, algo_matches[j]->p1_id, algo_matches[j]->p2_id);

					j++;
					break;
			}
			j++;
			if (j >= num_algo_matches) {
				break;
			}
		}
		i++;
		if (j >= num_algo_matches) {
			break;
		}
	}

	this->evaluators[k - 1].set_precision( (double)correct_algo_matches / (double)num_algo_matches );
	this->evaluators[k - 1].set_recall( (double)correct_algo_matches / (double)num_corr_matches );
	this->evaluators[k - 1].set_f1(
			2.0 * this->evaluators[k-1].get_precision() * this->evaluators[k-1].get_recall() /
			(this->evaluators[k-1].get_precision() + this->evaluators[k-1].get_recall()) );
	this->evaluators[k - 1].set_num_algo_matches(num_algo_matches);
	this->evaluators[k - 1].set_num_algo_clusters(num_algo_clusters);

	printf("===================================================================================\n");
	printf(" RESULTS (Algorithm: %s, Dataset: %s, Sim Threshold: %4.2f)\n",
			this->PARAMS->get_algorithm_name(), this->PARAMS->get_dataset(), (_score_t)k / 10);
	printf("===================================================================================\n");
	printf(" Number of Entities:\t\t%d\n", this->num_entities);
	printf(" Number of Clusters:\t\t%d\n", num_corr_clusters);
	printf(" Max. Entity Length:\t\t%d words\n", this->stats->get_max_title_length());
	printf(" Avg. Entity Length:\t\t%4.2f words\n", this->stats->get_avg_title_length());
	printf("-----------------------------------------------------------------------------------\n");
	printf(" Algorithm Number of Clusters:\t%d\n", num_algo_clusters);
	printf("===================================================================================\n");
	printf(" Algorithm Correct Matches:\t%d\n", correct_algo_matches);
	printf(" Algorithm Total Matches:\t%d\n", num_algo_matches);
	printf(" Real Correct Matches:\t\t%d\n", num_corr_matches);
	printf("-----------------------------------------------------------------------------------\n");
	printf(" Precision:\t\t\t%4.3f\n", this->evaluators[k - 1].get_precision());
	printf(" Recall:\t\t\t%4.3f\n", this->evaluators[k - 1].get_recall());
	printf(" F1 Measure:\t\t\t%4.3f\n", this->evaluators[k - 1].get_f1());
	printf(" NMI Measure:\t\t\t%4.3f\n", this->evaluators[k - 1].get_nmi());
	printf("===================================================================================\n\n");

	/// Deallocate the resources
	for (i = 0; i < num_corr_matches; i++) {
		free(corr_matches[i]);
	}
	free(corr_matches);

	for (i = 0; i < num_algo_matches; i++) {
		free(algo_matches[i]);
	}
	free(algo_matches);

	return this->evaluators[k-1].get_f1();
}

template <class T> char * ClusteringMethod<T>::get_name() { return this->name; }
