#include <stdlib.h>

//Find the root of the tree of node i
template<typename LabelT>
inline static
LabelT findRoot(const LabelT *P, LabelT i){
	LabelT root = i;
	while (P[root] < root){
		root = P[root];
	}
	return root;
}

//Make all nodes in the path of node i point to root
template<typename LabelT>
inline static
void setRoot(LabelT *P, LabelT i, LabelT root){
	while (P[i] < i){
		LabelT j = P[i];
		P[i] = root;
		i = j;
	}
	P[i] = root;
}

//unite the two trees containing nodes i and j and return the new root
template<typename LabelT>
inline static
LabelT set_union(LabelT *P, LabelT i, LabelT j){
	LabelT root = findRoot(P, i);
	if (i != j){
		LabelT rootj = findRoot(P, j);
		if (root > rootj){
			root = rootj;
		}
		setRoot(P, j, root);
	}
	setRoot(P, i, root);
	return root;
}

//Flatten the Union Find tree and relabel the components
template<typename LabelT>
inline static
LabelT flattenL(LabelT *P, LabelT length){
	LabelT k = 1;
	for (LabelT i = 1; i < length; ++i){
		if (P[i] < i){
			P[i] = P[P[i]];
		}
		else{
			P[i] = k; k = k + 1;
		}
	}
	return k;
}

typedef unsigned char uint8_t;
// Based on "Optimized  Block-based Connected Components Labeling with Decision Trees", Costantino Grana et al
// Only for 8-connectivity
template<typename LabelT>
LabelT LabelingGrana(const uint8_t* pimg, int w, int h, int stride, LabelT* pimgLabels){
	//A quick and dirty upper bound for the maximum number of labels.
	//Following formula comes from the fact that a 2x2 block in 8-connectivity case
	//can never have more than 1 new label and 1 label for background.
	//Worst case image example pattern:
	//1 0 1 0 1...
	//0 0 0 0 0...
	//1 0 1 0 1...
	//............
	const size_t Plength = size_t(((h + 1) / 2) * size_t((w + 1) / 2)) + 1;

	LabelT *P = (LabelT *)malloc(sizeof(LabelT)*Plength);
	P[0] = 0;
	LabelT lunique = 1;

	// First scan
	for (int r = 0; r < h; r += 2) {
		// Get rows pointer
		const uint8_t * const img_row = pimg + r*stride;
		const uint8_t * const img_row_prev = img_row - stride;
		const uint8_t * const img_row_prev_prev = img_row_prev - stride;
		const uint8_t * const img_row_fol = img_row + stride;
		LabelT * const imgLabels_row = pimgLabels + r*w;
		LabelT * const imgLabels_row_prev_prev = imgLabels_row - w - w;
		for (int c = 0; c < w; c += 2) {

			// We work with 2x2 blocks
			// +-+-+-+
			// |P|Q|R|
			// +-+-+-+
			// |S|X|
			// +-+-+

			// The pixels are named as follows
			// +---+---+---+
			// |a b|c d|e f|
			// |g h|i j|k l|
			// +---+---+---+
			// |m n|o p|
			// |q r|s t|
			// +---+---+

			// Pixels a, f, l, q are not needed, since we need to understand the
			// the connectivity between these blocks and those pixels only metter
			// when considering the outer connectivities

			// A bunch of defines used to check if the pixels are foreground,
			// without going outside the image limits.
#define condition_b c-1>=0 && r-2>=0 && img_row_prev_prev[c-1]>0
#define condition_c r-2>=0 && img_row_prev_prev[c]>0
#define condition_d c+1<w&& r-2>=0 && img_row_prev_prev[c+1]>0
#define condition_e c+2<w  && r-1>=0 && img_row_prev[c-1]>0

#define condition_g c-2>=0 && r-1>=0 && img_row_prev[c-2]>0
#define condition_h c-1>=0 && r-1>=0 && img_row_prev[c-1]>0
#define condition_i r-1>=0 && img_row_prev[c]>0
#define condition_j c+1<w && r-1>=0 && img_row_prev[c+1]>0
#define condition_k c+2<w && r-1>=0 && img_row_prev[c+2]>0

#define condition_m c-2>=0 && img_row[c-2]>0
#define condition_n c-1>=0 && img_row[c-1]>0
#define condition_o img_row[c]>0
#define condition_p c+1<w && img_row[c+1]>0

#define condition_r c-1>=0 && r+1<h && img_row_fol[c-1]>0
#define condition_s r+1<h && img_row_fol[c]>0
#define condition_t c+1<w && r+1<h && img_row_fol[c+1]>0

			// This is a decision tree which allows to choose which action to
			// perform, checking as few conditions as possible.
			// Actions: the blocks label are provisionally stored in the top left
			// pixel of the block in the labels image

			if (condition_o) {
				if (condition_n) {
					if (condition_j) {
						if (condition_i) {
							//Action_6: Assign label of block S
							imgLabels_row[c] = imgLabels_row[c - 2];
							continue;
						}
						else {
							if (condition_c) {
								if (condition_h) {
									//Action_6: Assign label of block S
									imgLabels_row[c] = imgLabels_row[c - 2];
									continue;
								}
								else {
									if (condition_g) {
										if (condition_b) {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
										else {
											//Action_11: Merge labels of block Q and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
											continue;
										}
									}
									else {
										//Action_11: Merge labels of block Q and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
										continue;
									}
								}
							}
							else {
								//Action_11: Merge labels of block Q and S
								imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
								continue;
							}
						}
					}
					else {
						if (condition_p) {
							if (condition_k) {
								if (condition_d) {
									if (condition_i) {
										//Action_6: Assign label of block S
										imgLabels_row[c] = imgLabels_row[c - 2];
										continue;
									}
									else {
										if (condition_c) {
											if (condition_h) {
												//Action_6: Assign label of block S
												imgLabels_row[c] = imgLabels_row[c - 2];
												continue;
											}
											else {
												if (condition_g) {
													if (condition_b) {
														//Action_6: Assign label of block S
														imgLabels_row[c] = imgLabels_row[c - 2];
														continue;
													}
													else {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
												}
												else {
													//Action_12: Merge labels of block R and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
													continue;
												}
											}
										}
										else {
											//Action_12: Merge labels of block R and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
											continue;
										}
									}
								}
								else {
									//Action_12: Merge labels of block R and S
									imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
									continue;
								}
							}
							else {
								//Action_6: Assign label of block S
								imgLabels_row[c] = imgLabels_row[c - 2];
								continue;
							}
						}
						else {
							//Action_6: Assign label of block S
							imgLabels_row[c] = imgLabels_row[c - 2];
							continue;
						}
					}
				}
				else {
					if (condition_r) {
						if (condition_j) {
							if (condition_m) {
								if (condition_h) {
									if (condition_i) {
										//Action_6: Assign label of block S
										imgLabels_row[c] = imgLabels_row[c - 2];
										continue;
									}
									else {
										if (condition_c) {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
										else {
											//Action_11: Merge labels of block Q and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
											continue;
										}
									}
								}
								else {
									if (condition_g) {
										if (condition_b) {
											if (condition_i) {
												//Action_6: Assign label of block S
												imgLabels_row[c] = imgLabels_row[c - 2];
												continue;
											}
											else {
												if (condition_c) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
										}
										else {
											//Action_11: Merge labels of block Q and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
											continue;
										}
									}
									else {
										//Action_11: Merge labels of block Q and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
										continue;
									}
								}
							}
							else {
								if (condition_i) {
									//Action_11: Merge labels of block Q and S
									imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
									continue;
								}
								else {
									if (condition_h) {
										if (condition_c) {
											//Action_11: Merge labels of block Q and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
											continue;
										}
										else {
											//Action_14: Merge labels of block P, Q and S
											imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c]), imgLabels_row[c - 2]);
											continue;
										}
									}
									else {
										//Action_11: Merge labels of block Q and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
										continue;
									}
								}
							}
						}
						else {
							if (condition_p) {
								if (condition_k) {
									if (condition_m) {
										if (condition_h) {
											if (condition_d) {
												if (condition_i) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													if (condition_c) {
														//Action_6: Assign label of block S
														imgLabels_row[c] = imgLabels_row[c - 2];
														continue;
													}
													else {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
												}
											}
											else {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											if (condition_d) {
												if (condition_g) {
													if (condition_b) {
														if (condition_i) {
															//Action_6: Assign label of block S
															imgLabels_row[c] = imgLabels_row[c - 2];
															continue;
														}
														else {
															if (condition_c) {
																//Action_6: Assign label of block S
																imgLabels_row[c] = imgLabels_row[c - 2];
																continue;
															}
															else {
																//Action_12: Merge labels of block R and S
																imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
																continue;
															}
														}
													}
													else {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
												}
												else {
													//Action_12: Merge labels of block R and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												if (condition_i) {
													if (condition_g) {
														if (condition_b) {
															//Action_12: Merge labels of block R and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
															continue;
														}
														else {
															//Action_16: labels of block Q, R and S
															imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
															continue;
														}
													}
													else {
														//Action_16: labels of block Q, R and S
														imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
														continue;
													}
												}
												else {
													//Action_12: Merge labels of block R and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
													continue;
												}
											}
										}
									}
									else {
										if (condition_i) {
											if (condition_d) {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
											else {
												//Action_16: labels of block Q, R and S
												imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											if (condition_h) {
												if (condition_d) {
													if (condition_c) {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
													else {
														//Action_15: Merge labels of block P, R and S
														imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
														continue;
													}
												}
												else {
													//Action_15: Merge labels of block P, R and S
													imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
										}
									}
								}
								else {
									if (condition_h) {
										if (condition_m) {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
										else {
											// ACTION_9 Merge labels of block P and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row[c - 2]);
											continue;
										}
									}
									else {
										if (condition_i) {
											if (condition_m) {
												if (condition_g) {
													if (condition_b) {
														//Action_6: Assign label of block S
														imgLabels_row[c] = imgLabels_row[c - 2];
														continue;
													}
													else {
														//Action_11: Merge labels of block Q and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
														continue;
													}
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_11: Merge labels of block Q and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
									}
								}
							}
							else {
								if (condition_h) {
									if (condition_m) {
										//Action_6: Assign label of block S
										imgLabels_row[c] = imgLabels_row[c - 2];
										continue;
									}
									else {
										// ACTION_9 Merge labels of block P and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row[c - 2]);
										continue;
									}
								}
								else {
									if (condition_i) {
										if (condition_m) {
											if (condition_g) {
												if (condition_b) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_11: Merge labels of block Q and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											//Action_11: Merge labels of block Q and S
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
											continue;
										}
									}
									else {
										//Action_6: Assign label of block S
										imgLabels_row[c] = imgLabels_row[c - 2];
										continue;
									}
								}
							}
						}
					}
					else {
						if (condition_j) {
							if (condition_i) {
								//Action_4: Assign label of block Q
								imgLabels_row[c] = imgLabels_row_prev_prev[c];
								continue;
							}
							else {
								if (condition_h) {
									if (condition_c) {
										//Action_4: Assign label of block Q
										imgLabels_row[c] = imgLabels_row_prev_prev[c];
										continue;
									}
									else {
										//Action_7: Merge labels of block P and Q
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c]);
										continue;
									}
								}
								else {
									//Action_4: Assign label of block Q
									imgLabels_row[c] = imgLabels_row_prev_prev[c];
									continue;
								}
							}
						}
						else {
							if (condition_p) {
								if (condition_k) {
									if (condition_i) {
										if (condition_d) {
											//Action_5: Assign label of block R
											imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
											continue;
										}
										else {
											// ACTION_10 Merge labels of block Q and R
											imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]);
											continue;
										}
									}
									else {
										if (condition_h) {
											if (condition_d) {
												if (condition_c) {
													//Action_5: Assign label of block R
													imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
													continue;
												}
												else {
													//Action_8: Merge labels of block P and R
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c + 2]);
													continue;
												}
											}
											else {
												//Action_8: Merge labels of block P and R
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c - 2], imgLabels_row_prev_prev[c + 2]);
												continue;
											}
										}
										else {
											//Action_5: Assign label of block R
											imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
											continue;
										}
									}
								}
								else {
									if (condition_i) {
										//Action_4: Assign label of block Q
										imgLabels_row[c] = imgLabels_row_prev_prev[c];
										continue;
									}
									else {
										if (condition_h) {
											//Action_3: Assign label of block P
											imgLabels_row[c] = imgLabels_row_prev_prev[c - 2];
											continue;
										}
										else {
											//Action_2: New label (the block has foreground pixels and is not connected to anything else)
											imgLabels_row[c] = lunique;
											P[lunique] = lunique;
											lunique = lunique + 1;
											continue;
										}
									}
								}
							}
							else {
								if (condition_i) {
									//Action_4: Assign label of block Q
									imgLabels_row[c] = imgLabels_row_prev_prev[c];
									continue;
								}
								else {
									if (condition_h) {
										//Action_3: Assign label of block P
										imgLabels_row[c] = imgLabels_row_prev_prev[c - 2];
										continue;
									}
									else {
										//Action_2: New label (the block has foreground pixels and is not connected to anything else)
										imgLabels_row[c] = lunique;
										P[lunique] = lunique;
										lunique = lunique + 1;
										continue;
									}
								}
							}
						}
					}
				}
			}
			else {
				if (condition_s) {
					if (condition_p) {
						if (condition_n) {
							if (condition_j) {
								if (condition_i) {
									//Action_6: Assign label of block S
									imgLabels_row[c] = imgLabels_row[c - 2];
									continue;
								}
								else {
									if (condition_c) {
										if (condition_h) {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
										else {
											if (condition_g) {
												if (condition_b) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_11: Merge labels of block Q and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
												continue;
											}
										}
									}
									else {
										//Action_11: Merge labels of block Q and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
										continue;
									}
								}
							}
							else {
								if (condition_k) {
									if (condition_d) {
										if (condition_i) {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
										else {
											if (condition_c) {
												if (condition_h) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													if (condition_g) {
														if (condition_b) {
															//Action_6: Assign label of block S
															imgLabels_row[c] = imgLabels_row[c - 2];
															continue;
														}
														else {
															//Action_12: Merge labels of block R and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
															continue;
														}
													}
													else {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
												}
											}
											else {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
										}
									}
									else {
										//Action_12: Merge labels of block R and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
										continue;
									}
								}
								else {
									//Action_6: Assign label of block S
									imgLabels_row[c] = imgLabels_row[c - 2];
									continue;
								}
							}
						}
						else {
							if (condition_r) {
								if (condition_j) {
									if (condition_m) {
										if (condition_h) {
											if (condition_i) {
												//Action_6: Assign label of block S
												imgLabels_row[c] = imgLabels_row[c - 2];
												continue;
											}
											else {
												if (condition_c) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
										}
										else {
											if (condition_g) {
												if (condition_b) {
													if (condition_i) {
														//Action_6: Assign label of block S
														imgLabels_row[c] = imgLabels_row[c - 2];
														continue;
													}
													else {
														if (condition_c) {
															//Action_6: Assign label of block S
															imgLabels_row[c] = imgLabels_row[c - 2];
															continue;
														}
														else {
															//Action_11: Merge labels of block Q and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
															continue;
														}
													}
												}
												else {
													//Action_11: Merge labels of block Q and S
													imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_11: Merge labels of block Q and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
												continue;
											}
										}
									}
									else {
										//Action_11: Merge labels of block Q and S
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
										continue;
									}
								}
								else {
									if (condition_k) {
										if (condition_d) {
											if (condition_m) {
												if (condition_h) {
													if (condition_i) {
														//Action_6: Assign label of block S
														imgLabels_row[c] = imgLabels_row[c - 2];
														continue;
													}
													else {
														if (condition_c) {
															//Action_6: Assign label of block S
															imgLabels_row[c] = imgLabels_row[c - 2];
															continue;
														}
														else {
															//Action_12: Merge labels of block R and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
															continue;
														}
													}
												}
												else {
													if (condition_g) {
														if (condition_b) {
															if (condition_i) {
																//Action_6: Assign label of block S
																imgLabels_row[c] = imgLabels_row[c - 2];
																continue;
															}
															else {
																if (condition_c) {
																	//Action_6: Assign label of block S
																	imgLabels_row[c] = imgLabels_row[c - 2];
																	continue;
																}
																else {
																	//Action_12: Merge labels of block R and S
																	imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
																	continue;
																}
															}
														}
														else {
															//Action_12: Merge labels of block R and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
															continue;
														}
													}
													else {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
												}
											}
											else {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											if (condition_i) {
												if (condition_m) {
													if (condition_h) {
														//Action_12: Merge labels of block R and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
														continue;
													}
													else {
														if (condition_g) {
															if (condition_b) {
																//Action_12: Merge labels of block R and S
																imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
																continue;
															}
															else {
																//Action_16: labels of block Q, R and S
																imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
																continue;
															}
														}
														else {
															//Action_16: labels of block Q, R and S
															imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
															continue;
														}
													}
												}
												else {
													//Action_16: labels of block Q, R and S
													imgLabels_row[c] = set_union(P, set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]), imgLabels_row[c - 2]);
													continue;
												}
											}
											else {
												//Action_12: Merge labels of block R and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c + 2], imgLabels_row[c - 2]);
												continue;
											}
										}
									}
									else {
										if (condition_i) {
											if (condition_m) {
												if (condition_h) {
													//Action_6: Assign label of block S
													imgLabels_row[c] = imgLabels_row[c - 2];
													continue;
												}
												else {
													if (condition_g) {
														if (condition_b) {
															//Action_6: Assign label of block S
															imgLabels_row[c] = imgLabels_row[c - 2];
															continue;
														}
														else {
															//Action_11: Merge labels of block Q and S
															imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
															continue;
														}
													}
													else {
														//Action_11: Merge labels of block Q and S
														imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
														continue;
													}
												}
											}
											else {
												//Action_11: Merge labels of block Q and S
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row[c - 2]);
												continue;
											}
										}
										else {
											//Action_6: Assign label of block S
											imgLabels_row[c] = imgLabels_row[c - 2];
											continue;
										}
									}
								}
							}
							else {
								if (condition_j) {
									//Action_4: Assign label of block Q
									imgLabels_row[c] = imgLabels_row_prev_prev[c];
									continue;
								}
								else {
									if (condition_k) {
										if (condition_i) {
											if (condition_d) {
												//Action_5: Assign label of block R
												imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
												continue;
											}
											else {
												// ACTION_10 Merge labels of block Q and R
												imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]);
												continue;
											}
										}
										else {
											//Action_5: Assign label of block R
											imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
											continue;
										}
									}
									else {
										if (condition_i) {
											//Action_4: Assign label of block Q
											imgLabels_row[c] = imgLabels_row_prev_prev[c];
											continue;
										}
										else {
											//Action_2: New label (the block has foreground pixels and is not connected to anything else)
											imgLabels_row[c] = lunique;
											P[lunique] = lunique;
											lunique = lunique + 1;
											continue;
										}
									}
								}
							}
						}
					}
					else {
						if (condition_r) {
							//Action_6: Assign label of block S
							imgLabels_row[c] = imgLabels_row[c - 2];
							continue;
						}
						else {
							if (condition_n) {
								//Action_6: Assign label of block S
								imgLabels_row[c] = imgLabels_row[c - 2];
								continue;
							}
							else {
								//Action_2: New label (the block has foreground pixels and is not connected to anything else)
								imgLabels_row[c] = lunique;
								P[lunique] = lunique;
								lunique = lunique + 1;
								continue;
							}
						}
					}
				}
				else {
					if (condition_p) {
						if (condition_j) {
							//Action_4: Assign label of block Q
							imgLabels_row[c] = imgLabels_row_prev_prev[c];
							continue;
						}
						else {
							if (condition_k) {
								if (condition_i) {
									if (condition_d) {
										//Action_5: Assign label of block R
										imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
										continue;
									}
									else {
										// ACTION_10 Merge labels of block Q and R
										imgLabels_row[c] = set_union(P, imgLabels_row_prev_prev[c], imgLabels_row_prev_prev[c + 2]);
										continue;
									}
								}
								else {
									//Action_5: Assign label of block R
									imgLabels_row[c] = imgLabels_row_prev_prev[c + 2];
									continue;
								}
							}
							else {
								if (condition_i) {
									//Action_4: Assign label of block Q
									imgLabels_row[c] = imgLabels_row_prev_prev[c];
									continue;
								}
								else {
									//Action_2: New label (the block has foreground pixels and is not connected to anything else)
									imgLabels_row[c] = lunique;
									P[lunique] = lunique;
									lunique = lunique + 1;
									continue;
								}
							}
						}
					}
					else {
						if (condition_t) {
							//Action_2: New label (the block has foreground pixels and is not connected to anything else)
							imgLabels_row[c] = lunique;
							P[lunique] = lunique;
							lunique = lunique + 1;
							continue;
						}
						else {
							// Action_1: No action (the block has no foreground pixels)
							imgLabels_row[c] = 0;
							continue;
						}
					}
				}
			}
		}

	}

	// Second scan 
	LabelT nLabels = flattenL(P, lunique);

	if (h & 1){
		if (w & 1){
			//Case 1: both rows and cols odd
			for (int r = 0; r < h; r += 2) {
				// Get rows pointer
				const uint8_t * const img_row = pimg + r*stride;
				const uint8_t * const img_row_fol = img_row + stride;
				LabelT * const imgLabels_row = pimgLabels + r*w;
				LabelT * const imgLabels_row_fol = imgLabels_row + w;

				for (int c = 0; c < w; c += 2) {
					LabelT iLabel = imgLabels_row[c];
					if (iLabel > 0) {
						iLabel = P[iLabel];
						if (img_row[c] > 0){
							imgLabels_row[c] = iLabel;
						}
						else{
							imgLabels_row[c] = 0;
						}
						if (c + 1 < w) {
							if (img_row[c + 1] > 0){
								imgLabels_row[c + 1] = iLabel;
							}
							else{
								imgLabels_row[c + 1] = 0;
							}
							if (r + 1 < h) {
								if (img_row_fol[c] > 0){
									imgLabels_row_fol[c] = iLabel;
								}
								else{
									imgLabels_row_fol[c] = 0;
								}
								if (img_row_fol[c + 1] > 0){
									imgLabels_row_fol[c + 1] = iLabel;
								}
								else{
									imgLabels_row_fol[c + 1] = 0;
								}
							}
						}
						else if (r + 1 < h) {
							if (img_row_fol[c] > 0){
								imgLabels_row_fol[c] = iLabel;
							}
							else{
								imgLabels_row_fol[c] = 0;
							}
						}
					}
					else {
						imgLabels_row[c] = 0;
						if (c + 1 < w) {
							imgLabels_row[c + 1] = 0;
							if (r + 1 < h) {
								imgLabels_row_fol[c] = 0;
								imgLabels_row_fol[c + 1] = 0;
							}
						}
						else if (r + 1 < h) {
							imgLabels_row_fol[c] = 0;
						}
					}
				}
			}
		}//END Case 1
		else{
			//Case 2: only rows odd
			for (int r = 0; r < h; r += 2) {
				// Get rows pointer
				const uint8_t * const img_row = pimg + r*stride;
				const uint8_t * const img_row_fol = img_row + stride;
				LabelT * const imgLabels_row = pimgLabels + r*w;
				LabelT * const imgLabels_row_fol = imgLabels_row + w;

				for (int c = 0; c < w; c += 2) {
					LabelT iLabel = imgLabels_row[c];
					if (iLabel > 0) {
						iLabel = P[iLabel];
						if (img_row[c] > 0){
							imgLabels_row[c] = iLabel;
						}
						else{
							imgLabels_row[c] = 0;
						}
						if (img_row[c + 1] > 0){
							imgLabels_row[c + 1] = iLabel;
						}
						else{
							imgLabels_row[c + 1] = 0;
						}
						if (r + 1 < h) {
							if (img_row_fol[c] > 0){
								imgLabels_row_fol[c] = iLabel;
							}
							else{
								imgLabels_row_fol[c] = 0;
							}
							if (img_row_fol[c + 1] > 0){
								imgLabels_row_fol[c + 1] = iLabel;
							}
							else{
								imgLabels_row_fol[c + 1] = 0;
							}
						}
					}
					else {
						imgLabels_row[c] = 0;
						imgLabels_row[c + 1] = 0;
						if (r + 1 < h) {
							imgLabels_row_fol[c] = 0;
							imgLabels_row_fol[c + 1] = 0;
						}
					}
				}
			}
		}// END Case 2
	}
	else{
		if (w & 1){
			//Case 3: only cols odd
			for (int r = 0; r < h; r += 2) {
				// Get rows pointer
				const uint8_t * const img_row = pimg + r*stride;
				const uint8_t * const img_row_fol = img_row + stride;
				LabelT * const imgLabels_row = pimgLabels + r*w;
				LabelT * const imgLabels_row_fol = imgLabels_row + w;

				for (int c = 0; c < w; c += 2) {
					LabelT iLabel = imgLabels_row[c];
					if (iLabel > 0) {
						iLabel = P[iLabel];
						if (img_row[c] > 0){
							imgLabels_row[c] = iLabel;
						}
						else{
							imgLabels_row[c] = 0;
						}
						if (img_row_fol[c] > 0){
							imgLabels_row_fol[c] = iLabel;
						}
						else{
							imgLabels_row_fol[c] = 0;
						}
						if (c + 1 < w) {
							if (img_row[c + 1] > 0){
								imgLabels_row[c + 1] = iLabel;
							}
							else{
								imgLabels_row[c + 1] = 0;
							}
							if (img_row_fol[c + 1] > 0){
								imgLabels_row_fol[c + 1] = iLabel;
							}
							else{
								imgLabels_row_fol[c + 1] = 0;
							}
						}
					}
					else{
						imgLabels_row[c] = 0;
						imgLabels_row_fol[c] = 0;
						if (c + 1 < w) {
							imgLabels_row[c + 1] = 0;
							imgLabels_row_fol[c + 1] = 0;
						}
					}
				}
			}
		}// END case 3
		else{
			//Case 4: nothing odd
			for (int r = 0; r < h; r += 2) {
				// Get rows pointer
				const uint8_t * const img_row = pimg + r*stride;
				const uint8_t * const img_row_fol = img_row + stride;
				LabelT * const imgLabels_row = pimgLabels + r*w;
				LabelT * const imgLabels_row_fol = imgLabels_row + w;

				for (int c = 0; c < w; c += 2) {
					LabelT iLabel = imgLabels_row[c];
					if (iLabel > 0) {
						iLabel = P[iLabel];
						if (img_row[c] > 0){
							imgLabels_row[c] = iLabel;
						}
						else{
							imgLabels_row[c] = 0;
						}
						if (img_row[c + 1] > 0){
							imgLabels_row[c + 1] = iLabel;
						}
						else{
							imgLabels_row[c + 1] = 0;
						}
						if (img_row_fol[c] > 0){
							imgLabels_row_fol[c] = iLabel;
						}
						else{
							imgLabels_row_fol[c] = 0;
						}
						if (img_row_fol[c + 1] > 0){
							imgLabels_row_fol[c + 1] = iLabel;
						}
						else{
							imgLabels_row_fol[c + 1] = 0;
						}
					}
					else {
						imgLabels_row[c] = 0;
						imgLabels_row[c + 1] = 0;
						imgLabels_row_fol[c] = 0;
						imgLabels_row_fol[c + 1] = 0;
					}
				}
			}
		}//END case 4
	}

	free(P);

	return nLabels;
}//End LabelingGrana