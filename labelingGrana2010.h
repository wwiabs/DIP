// Copyright(c) 2016 - 2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
// 
// * Neither the name of YACCLAB nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

//#include "image.h"

// "STANDARD" VERSION
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

//Find the root of the tree of the node i and compress the path in the process
template<typename LabelT>
inline static
LabelT find(LabelT *P, LabelT i){
	LabelT root = findRoot(P, i);
	setRoot(P, i, root);
	return root;
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

template<typename T>
inline static
void firstScanBBDT(const Image &img, Image_<T>& imgLabels, T* P, T &lunique) {
	int w(img.width), h(img.height);

	for (int r = 0; r<h; r += 2) {
		// Get rows pointer
		const uchar* const img_row = img.ptr(r);
		const uchar* const img_row_prev = (uchar *)(((char *)img_row) - img.stepsize);
		const uchar* const img_row_prev_prev = (uchar *)(((char *)img_row_prev) - img.stepsize);
		const uchar* const img_row_fol = (uchar *)(((char *)img_row) + img.stepsize);
		T* const imgLabels_row = imgLabels.ptr(r);
		T* const imgLabels_row_prev_prev = (T *)(((char *)imgLabels_row) - imgLabels.stepsize - imgLabels.stepsize);
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
#define condition_d c+1<w && r-2>=0 && img_row_prev_prev[c+1]>0
#define condition_e c+2<w && r-2>=0 && img_row_prev_prev[c+2]>0

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
			// Actions are available after the tree.

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

#undef condition_b
#undef condition_c
#undef condition_d
#undef condition_e

#undef condition_g
#undef condition_h
#undef condition_i
#undef condition_j
#undef condition_k

#undef condition_m
#undef condition_n
#undef condition_o
#undef condition_p

#undef condition_r
#undef condition_s
#undef condition_t

}

template<typename T>
int BBDT(const Image &img, Image_<T> &imgLabels) {
	//A quick and dirty upper bound for the maximimum number of labels.
	const size_t Plength = (img.height + 1) * (img.width + 1) / 4;
	//Tree of labels
	T *P = new T[Plength];

	//Background
	P[0] = 0;
	T lunique = 1;

	firstScanBBDT(img, imgLabels, P, lunique);

	T nLabel = flattenL(P, lunique);

	// Second scan
	if (imgLabels.height & 1){
		if (imgLabels.width & 1){
			//Case 1: both rows and cols odd
			for (unsigned r = 0; r<imgLabels.height; r += 2) {
				// Get rows pointer
				const uchar* const img_row = img.ptr(r);
				const uchar* const img_row_fol = (uchar *)(((char *)img_row) + img.stepsize);

				T* const imgLabels_row = imgLabels.ptr(r);
				T* const imgLabels_row_fol = (T *)(((char *)imgLabels_row) + imgLabels.stepsize);
				// Get rows pointer
				for (unsigned c = 0; c<imgLabels.width; c += 2) {
					int iLabel = imgLabels_row[c];
					if (iLabel>0) {
						iLabel = P[iLabel];
						if (img_row[c]>0)
							imgLabels_row[c] = iLabel;
						else
							imgLabels_row[c] = 0;
						if (c + 1<imgLabels.width) {
							if (img_row[c + 1]>0)
								imgLabels_row[c + 1] = iLabel;
							else
								imgLabels_row[c + 1] = 0;
							if (r + 1<imgLabels.height) {
								if (img_row_fol[c]>0)
									imgLabels_row_fol[c] = iLabel;
								else
									imgLabels_row_fol[c] = 0;
								if (img_row_fol[c + 1]>0)
									imgLabels_row_fol[c + 1] = iLabel;
								else
									imgLabels_row_fol[c + 1] = 0;
							}
						}
						else if (r + 1<imgLabels.height) {
							if (img_row_fol[c]>0)
								imgLabels_row_fol[c] = iLabel;
							else
								imgLabels_row_fol[c] = 0;
						}
					}
					else {
						imgLabels_row[c] = 0;
						if (c + 1<imgLabels.width) {
							imgLabels_row[c + 1] = 0;
							if (r + 1<imgLabels.height) {
								imgLabels_row_fol[c] = 0;
								imgLabels_row_fol[c + 1] = 0;
							}
						}
						else if (r + 1<imgLabels.height) {
							imgLabels_row_fol[c] = 0;
						}
					}
				}
			}
		}//END Case 1
		else{
			//Case 2: only rows odd
			for (unsigned r = 0; r<imgLabels.height; r += 2) {
				// Get rows pointer
				const uchar* const img_row = img.ptr(r);
				const uchar* const img_row_fol = (uchar *)(((char *)img_row) + img.stepsize);

				T* const imgLabels_row = imgLabels.ptr(r);
				T* const imgLabels_row_fol = (T *)(((char *)imgLabels_row) + imgLabels.stepsize);
				// Get rows pointer
				for (unsigned c = 0; c<imgLabels.width; c += 2) {
					int iLabel = imgLabels_row[c];
					if (iLabel>0) {
						iLabel = P[iLabel];
						if (img_row[c]>0)
							imgLabels_row[c] = iLabel;
						else
							imgLabels_row[c] = 0;
						if (img_row[c + 1]>0)
							imgLabels_row[c + 1] = iLabel;
						else
							imgLabels_row[c + 1] = 0;
						if (r + 1<imgLabels.height) {
							if (img_row_fol[c]>0)
								imgLabels_row_fol[c] = iLabel;
							else
								imgLabels_row_fol[c] = 0;
							if (img_row_fol[c + 1]>0)
								imgLabels_row_fol[c + 1] = iLabel;
							else
								imgLabels_row_fol[c + 1] = 0;
						}
					}
					else {
						imgLabels_row[c] = 0;
						imgLabels_row[c + 1] = 0;
						if (r + 1<imgLabels.height) {
							imgLabels_row_fol[c] = 0;
							imgLabels_row_fol[c + 1] = 0;
						}
					}
				}
			}
		}// END Case 2
	}
	else{
		if (imgLabels.width & 1){
			//Case 3: only cols odd
			for (unsigned r = 0; r<imgLabels.height; r += 2) {
				// Get rows pointer
				const uchar* const img_row = img.ptr(r);
				const uchar* const img_row_fol = (uchar *)(((char *)img_row) + img.stepsize);

				T* const imgLabels_row = imgLabels.ptr(r);
				T* const imgLabels_row_fol = (T *)(((char *)imgLabels_row) + imgLabels.stepsize);
				// Get rows pointer
				for (unsigned c = 0; c<imgLabels.width; c += 2) {
					int iLabel = imgLabels_row[c];
					if (iLabel>0) {
						iLabel = P[iLabel];
						if (img_row[c]>0)
							imgLabels_row[c] = iLabel;
						else
							imgLabels_row[c] = 0;
						if (img_row_fol[c]>0)
							imgLabels_row_fol[c] = iLabel;
						else
							imgLabels_row_fol[c] = 0;
						if (c + 1<imgLabels.width) {
							if (img_row[c + 1]>0)
								imgLabels_row[c + 1] = iLabel;
							else
								imgLabels_row[c + 1] = 0;
							if (img_row_fol[c + 1]>0)
								imgLabels_row_fol[c + 1] = iLabel;
							else
								imgLabels_row_fol[c + 1] = 0;
						}
					}
					else{
						imgLabels_row[c] = 0;
						imgLabels_row_fol[c] = 0;
						if (c + 1<imgLabels.width) {
							imgLabels_row[c + 1] = 0;
							imgLabels_row_fol[c + 1] = 0;
						}
					}
				}
			}
		}// END case 3
		else{
			//Case 4: nothing odd
			for (unsigned r = 0; r < imgLabels.height; r += 2) {
				// Get rows pointer
				const uchar* const img_row = img.ptr(r);
				const uchar* const img_row_fol = (uchar *)(((char *)img_row) + img.stepsize);

				T* const imgLabels_row = imgLabels.ptr(r);
				T* const imgLabels_row_fol = (T *)(((char *)imgLabels_row) + imgLabels.stepsize);
				// Get rows pointer
				for (unsigned c = 0; c<imgLabels.width; c += 2) {
					int iLabel = imgLabels_row[c];
					if (iLabel>0) {
						iLabel = P[iLabel];
						if (img_row[c] > 0)
							imgLabels_row[c] = iLabel;
						else
							imgLabels_row[c] = 0;
						if (img_row[c + 1] > 0)
							imgLabels_row[c + 1] = iLabel;
						else
							imgLabels_row[c + 1] = 0;
						if (img_row_fol[c] > 0)
							imgLabels_row_fol[c] = iLabel;
						else
							imgLabels_row_fol[c] = 0;
						if (img_row_fol[c + 1] > 0)
							imgLabels_row_fol[c + 1] = iLabel;
						else
							imgLabels_row_fol[c + 1] = 0;
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

	delete[] P;
	return nLabel;
}


// "STANDARD" VERSION


