#include "stdafx.h"
#include "Evaluation.h"

/*GAME PHASE*/
void Evaluation::setGamePhase(const Board& b) {
	if (gamePhase == OPENING) {
		int score = 0;
		//Sparse backrank --> more developed pieces
		for (int i = 21; i < 29; i++) {
			if (b.mailbox[i] == 0) {
				score += 2;
			}
		}
		for (int i = 91; i < 99; i++) {
			if (b.mailbox[i] == 0) {
				score += 2;
			}
		}

		//Castled
		for (int i = 0; i < 2; i++) {
			if (b.castled[i]) {
				score += 2;
			}
		}

		if (score >= 16) {
			gamePhase = MIDGAME;
		}
	}
	else if (gamePhase == MIDGAME) {
		int count = 0;
		for (int i = 21; i < 99; i++) {
			if (b.mailbox[i] != 0 && abs(b.mailbox[i]) != WP && abs(b.mailbox[i]) != WK) {
				count++;
			}
		}
		//Less than eight major/minor pieces
		if (count < 8) {
			gamePhase = ENDGAME;
		}
	}
}

/*PAWN STRUCTURE*/
int Evaluation::openFiles(const Board& b) {
	bool filearray[8] = { true, true, true, true, true, true, true, true };
	int filecount = 0;
	for (int i = 21; i < 99; i++) {
		if (abs(b.mailbox[i]) == WP) {
			filearray[i % 10 - 1] = false;
		}
	}
	for (int i = 0; i < 8; i++) {
		if (filearray[i]) {
			filecount++;
		}
	}
	return filecount;
}
int Evaluation::semiOpenFiles(const Board& b) {
	int filearray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int filecount = 0;
	for (int i = 21; i < 99; i++) {
		if (abs(b.mailbox[i]) == WP) {
			filearray[i % 10 - 1]++;
		}
	}
	for (int i = 0; i < 8; i++) {
		if (filearray[i] == 1) {
			filecount++;
		}
	}
	return filecount;
}
int Evaluation::blockedPawns(const Board& b) {
	int bpcount = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WP && b.mailbox[i - 10] == BP && b.mailbox[i - 11] >= 0 && b.mailbox[i - 9] >= 0) {
			bpcount++;
		}
		if (b.mailbox[i] == BP && b.mailbox[i + 10] == WP && b.mailbox[i + 11] <= 0 && b.mailbox[i + 9] <= 0) {
			bpcount++;
		}
	}
	return bpcount;
}
int Evaluation::doubledPawns(const Board& b, int color) {
	int filearray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int pcount = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WP * color) {
			filearray[i % 10 - 1]++;
		}
	}
	for (int i = 0; i < 8; i++) {
		if (filearray[i] >= 2) {
			pcount++;
		}
	}
	return pcount * DOUBLED_P_PENALTY;
}
int Evaluation::isolatedPawns(const Board& b, int color) {
	int filearray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int pcount = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WP * color) {
			filearray[i % 10 - 1]++;
		}
	}
	for (int i = 0; i < 8; i++) {
		if (i >= 1 && i <= 6) {
			if (filearray[i] == WP * color && filearray[i - 1] == 0 && filearray[i + 1] == 0) {
				pcount++;
			}
		}
		else if (i = 0) {
			if (filearray[0] == WP * color && filearray[1] == 0) {
				pcount++;
			}
		}
		else if (i = 7) {
			if (filearray[7] == WP * color && filearray[6] == 0) {
				pcount++;
			}
		}
	}
	return pcount * ISOLATED_P_PENALTY;
}
int Evaluation::protectedPawns(const Board& b, int color) {
	int pcount = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WP * color) {
			if (b.mailbox[i - color * 9] == WP * color || b.mailbox[i - color * 11] == WP * color) {
			pcount++;
			}
		}
	}
	return pcount * PROTECTED_P_BONUS;
}
int Evaluation::passedPawns(const Board& b, int color) {
	int pcount = 0;
	int rank, file;
	int sides = 0;
	bool none = true;
	bool haspawn = false;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WP * color) {
			file = i % 10;
			rank = (i - file) / 10;
			haspawn = true;
		}
	}
	if (haspawn) {
		for (int i = 20 + file - 1; i <= 90 + file - 1; i += 10) {
			if (b.mailbox[i] == WP * -color) {
				if ((i - i % 10) / 10 >= rank) {
					sides++;
				}
				none = false;
				break;
			}
		}
	}
	if (none) {
		sides++;
	}
	for (int i = 20 + file + 1; i <= 90 + file + 1; i += 10) {
		if (b.mailbox[i] == WP * -color) {
			if ((i - i % 10) / 10 >= rank) {
				sides++;
			}
			none = false;
			break;
		}
	}
	if (none) {
		sides++;
	}
	if (sides == 2) {
		pcount++;
	}
	return pcount * PASSED_P_BONUS;
}


/*PIECE VALUES*/
int Evaluation::baseMaterial(const Board& b, int color) {
	int mval = 0;
	for (int i = 21; i < 99; i++) {
		if (color * b.mailbox[i] > 0) {
			switch (abs(b.mailbox[i])) {
			case WP:
				mval += P_BASE_VAL;
				break;
			case WN:
				mval += N_BASE_VAL;
				break;
			case WB:
				mval += B_BASE_VAL;
				break;
			case WR:
				mval += R_BASE_VAL;
				break;
			case WQ:
				mval += Q_BASE_VAL;
				break;
			}
		}
	}
	return mval;
}
int Evaluation::comboMaterial(const Board& b, int color) {
	int mval = 0;
	int ncount = 0;
	int bcount = 0;
	int rcount = 0;
	for (int i = 21; i < 99; i++) {
		if (color * b.mailbox[i] > 0) {
			switch (abs(b.mailbox[i])) {
			case WN:
				ncount += 1;
				break;
			case WB:
				bcount += 1;
				break;
			case WR:
				rcount += 1;
				break;
			}
		}
	}
	//Knight pair penalty
	if (ncount >= 2) {
		mval -= 10;
	}
	//Bishop pair bonus
	if (bcount >= 2) {
		mval += 50;
	}
	//Rook pair penalty
	if (rcount >= 2) {
		mval -= 10;
	}
	return mval;
}
int Evaluation::structureMaterial(const Board& b, int color) {
	int mval = 0;
	for (int i = 21; i < 99; i++) {
		if (color * b.mailbox[i] > 0) {
			switch (abs(b.mailbox[i])) {
			case WN:
				//Knights are better in closed positions
				mval += blockedPawns(b) * 5;
				break;
			case WB:
				//Bishops are worse in closed positions
				mval -= blockedPawns(b) * 5;
				break;
			case WR:
				//Rooks are better on open and semi-open files
				mval += isOpenFile(b, i) * R_OPEN_FILE_BONUS;
				break;
			}
		}
	}
	return mval;
}

/*POSITION*/
int Evaluation::flipTableValue(int square) {
	int unit = square % 10;
	int tenth = (square - unit) / 10;
	int rval = (11 - tenth) * 10 + unit;
	return rval;
}
int Evaluation::piecePosition(Board& b, int color) {
	int pval = 0;
	for (int i = 21; i < 99; i++) {
		if (color * b.mailbox[i] > 0) {
			switch (b.mailbox[i]) {
			case WP:
				pval += pawnTable[b.to64Coord(i)];
				break;
			case WN:
				pval += knightTable[b.to64Coord(i)];
				break;
			case WB:
				pval += bishopTable[b.to64Coord(i)];
				break;
			case WR:
				pval += rookTable[b.to64Coord(i)];
				break;
			case WQ:
				if (gamePhase == OPENING) {
					pval += queenOpeningTable[b.to64Coord(i)];
				}
				else if (gamePhase >= MIDGAME) {
					pval += queenNormalTable[b.to64Coord(i)];
				}
				break;
			case WK:
				if (gamePhase <= MIDGAME) {
					pval += kingNormalTable[b.to64Coord(i)];
				}
				else if (gamePhase == ENDGAME) {
					pval += kingEndTable[b.to64Coord(i)];
				}
				break;
			case BP:
				pval += pawnTable[b.to64Coord(flipTableValue(i))];
				break;
			case BN:
				pval += knightTable[b.to64Coord(flipTableValue(i))];
				break;
			case BB:
				pval += bishopTable[b.to64Coord(flipTableValue(i))];
				break;
			case BR:
				pval += rookTable[b.to64Coord(flipTableValue(i))];
				break;
			case BQ:
				if (gamePhase == OPENING) {
					pval += queenOpeningTable[b.to64Coord(flipTableValue(i))];
				}
				else if (gamePhase >= MIDGAME) {
					pval += queenNormalTable[b.to64Coord(flipTableValue(i))];
				}
				break;
			case BK:
				if (gamePhase <= MIDGAME) {
					pval += kingNormalTable[b.to64Coord(flipTableValue(i))];
				}
				else if (gamePhase == ENDGAME) {
					pval += kingEndTable[b.to64Coord(flipTableValue(i))];
				}
				break;
			}
		}
	}
	return pval;
}
int Evaluation::space(const Board&b, int color) {
	int file;
	int rank = 0;
	int sval = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == color * WP && rank == 0) {
			file = i % 10;
			rank = (i - file) / 10;
			if (color == WHITE) {
				sval = (8 - rank) * SPACE_BONUS;
			}
			else {
				sval = (rank - 3) * SPACE_BONUS;
			}
			rank = 0;
		}
	}
	return sval;
}
int Evaluation::kingSafety(Board& b, int color) {
	b.setKingSquare(b.mailbox);
	int rval = 0;
	int pval = 0;
	int cval = 0;
	if (color == WHITE) {
		if (b.castled[0]) {
			cval++;
		}
		rval = isOpenFile(b, b.kingSquareWhite) + isOpenFile(b, b.kingSquareWhite + 1) + isOpenFile(b, b.kingSquareWhite - 1);
		if (gamePhase <= MIDGAME && b.castled[0]) {
			if (b.mailbox[b.kingSquareWhite - 10] != WP) {
				pval++;
			}
			if (b.mailbox[b.kingSquareWhite - 11] != WP) {
				pval++;
			}
			if (b.mailbox[b.kingSquareWhite - 9] != WP) {
				pval++;
			}
		}
	}
	else {
		if (b.castled[1]) {
			cval++;
		}
		rval = isOpenFile(b, b.kingSquareBlack) + isOpenFile(b, b.kingSquareBlack + 1) + isOpenFile(b, b.kingSquareBlack - 1);
		if (gamePhase <= MIDGAME && b.castled[1]) {
			if (b.mailbox[b.kingSquareBlack + 10] != BP) {
				pval++;
			}
			if (b.mailbox[b.kingSquareBlack + 11] != BP) {
				pval++;
			}
			if (b.mailbox[b.kingSquareBlack + 9] != BP) {
				pval++;
			}
		}
	}
	return rval * K_OPEN_FILE_PENALTY + pval * K_P_SHIELD_PENALTY + cval * K_CASTLED_BONUS;
}

/*CENTER*/
int Evaluation::pawnCenterControl(const Board& b, int color) {
	int pcount = 0;
	for (int i = 54; i < 66; i++) {
		if (i == 54 || i == 55 || i == 64 || i == 65) {
			if (b.mailbox[i] == WP * color || b.mailbox[i - color * 9] == WP * color || b.mailbox[i - color * 11] == WP * color) {
				pcount++;
			}
		}
	}
	return pcount * P_CENTER_BONUS;
}
int Evaluation::pawnExtendedCenterControl(const Board& b, int color) {
	int pcount = 0;
	for (int i = 43; i < 77; i++) {
		if (i % 10 >= 3 && i % 10 <= 6) {
			if (b.mailbox[i] == WP * color || b.mailbox[i - color * 9] == WP * color || b.mailbox[i - color * 11] == WP * color) {
				pcount++;
			}
		}
	}
	return pcount * P_EXTENDED_CENTER_BONUS;
}
int Evaluation::pieceExtendedCenterControl(Board& b, int color) {
	int pcount = 0;
	for (int i = 21; i < 99; i++) {
		if (b.mailbox[i] == WN * color || b.mailbox[i] == WB * color) {
			for (int j = 43; j < 77; j++) {
				if (j % 10 >= 3 && j % 10 <= 6) {
					if (b.checkAttack(i, j, b.mailbox)) {
						pcount++;
					}
				}
			}
		}
	}
	return pcount * PIECE_EXTENDED_CENTER_BONUS;
}

/*GETTERS*/
int Evaluation::isOpenFile(const Board& b, int square) {
	int file = square % 10;
	int pcount = 0;
	if (b.mailbox[square] != -9) {
		for (int i = 2; i <= 9; i++) {
			pcount = 0;
			if (abs(b.mailbox[i * 10 + file] == WP)) {
				pcount++;
			}
		}
	}
	if (pcount > 1) {
		return 0;
	}
	else if (pcount == 1) {
		return 1;
	}
	else if (pcount == 0) {
		return 2;
	}
}

int Evaluation::totalEvaluation(Board& b, int color) {
	//Reevaluates game phase
	setGamePhase(b);
	int material = baseMaterial(b, WHITE) + comboMaterial(b, WHITE) + structureMaterial(b, WHITE) - baseMaterial(b, BLACK) - comboMaterial(b, BLACK) - structureMaterial(b, BLACK);
	int pawns = doubledPawns(b, WHITE) + isolatedPawns(b, WHITE) + protectedPawns(b, WHITE) + passedPawns(b, WHITE) - doubledPawns(b, BLACK) - isolatedPawns(b, BLACK) - protectedPawns(b, BLACK) - passedPawns(b, BLACK);
	int position = piecePosition(b, WHITE) + space(b, WHITE) - piecePosition(b, BLACK) - space(b, BLACK);
	int center = pawnCenterControl(b, WHITE) + pawnExtendedCenterControl(b, WHITE) + pieceExtendedCenterControl(b, WHITE) - pawnCenterControl(b, BLACK) - pawnExtendedCenterControl(b, BLACK) - pieceExtendedCenterControl(b, BLACK);
	int total = material + pawns + position + center;
	return total;
}