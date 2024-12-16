static constexpr ull file1 = 0b0000000100000001000000010000000100000001000000010000000100000001ull;
static constexpr ull file8 = 0b1000000010000000100000001000000010000000100000001000000010000000ull;
static constexpr ull fileMid = 0b11110000111100001111000011110000111100001111000011110000111100ull;
static constexpr ull rank2 = 0b0000000000000000000000000000000000000000000000001111111100000000ull;
static constexpr ull rank7 = 0b0000000011111111000000000000000000000000000000000000000000000000ull;
static constexpr ull rankMid = 0x0000FFFFFFFF0000;
static constexpr ull rank_18 = 0xFF000000000000FF;

__attribute__((always_inline)) static constexpr ull Pawns_NotLeft(){
	return ~file1;
}

__attribute__((always_inline)) static constexpr ull Pawns_NotRight(){
	return ~file8;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_Forward(ull mask){
	if constexpr (white) return mask << 8;
	else return mask >> 8;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_Forward2(ull mask){
	if constexpr (white) return mask << 16;
	else return mask >> 16;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_Backward(ull mask){
	return Pawn_Forward<!white>(mask);
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_Backward2(ull mask){
	return Pawn_Forward2<!white>(mask);
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_AttackLeft(ull mask){
	if constexpr (white) return mask << 7;
	else return mask >> 9;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_AttackRight(ull mask){
	if constexpr (white) return mask << 9;
	else return mask >> 7;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_InvertLeft(ull mask){
	return Pawn_AttackRight<!white>(mask);
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_InvertRight(ull mask){
	return Pawn_AttackLeft<!white>(mask);
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawn_Attacks(ull mask){
	return Pawn_AttackLeft<white>(mask & Pawns_NotLeft()) | Pawn_AttackRight<white>(mask & Pawns_NotRight());
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Ahead(ull mask){
	if constexpr (white){
		mask |= mask << 8;
		mask |= mask << 16;
		mask |= mask << 32;
		return mask;
	} else {
		mask |= mask >> 8;
		mask |= mask >> 16;
		mask |= mask >> 32;
		return mask;
	}
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawns_FirstRank(){
	if constexpr (white) return rank2;
	else return rank7;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawns_LastRank(){
	if constexpr (white) return rank7;
	else return rank2;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull FirstRank(){
	if constexpr (white) return rank2 >> 8;
	else return rank7 << 8;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull King(const board& brd){
	if constexpr (white) return brd.wKings;
	else return brd.bKings;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull EnemyKing(const board& brd){
	if constexpr (white) return brd.bKings;
	else return brd.wKings;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Pawns(const board& brd){
	if constexpr (white) return brd.wPawns;
	else return brd.bPawns;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull OwnColor(const board& brd){
	if constexpr (white) return brd.whites;
	return brd.blacks;
}
template<bool white>
__attribute__((always_inline)) static constexpr ull Enemy(const board& brd){
	if constexpr (white) return brd.blacks;
	return brd.whites;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull EnemyRookQueen(const board& brd){
	if constexpr (white) return brd.bRooks | brd.bQueens;
	return brd.wRooks | brd.wQueens;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull RookQueen(const board& brd){
	if constexpr (white) return brd.wRooks | brd.wQueens;
	return brd.bRooks | brd.bQueens;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull EnemyBishopQueen(const board& brd){
	if constexpr (white) return brd.bBishops | brd.bQueens;
	return brd.wBishops | brd.wQueens;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull BishopQueen(const board& brd){
	if constexpr (white) return brd.wBishops | brd.wQueens;
	return brd.bBishops | brd.bQueens;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull EnemyOrEmpty(const board& brd){
	if constexpr (white) return ~brd.whites;
	return ~brd.blacks;
}

__attribute__((always_inline)) static constexpr ull Empty(const board& brd){
	return ~brd.occupied;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Knights(const board& brd){
	if constexpr (white) return brd.wKnights;
	return brd.bKnights;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Rooks(const board& brd){
	if constexpr (white) return brd.wRooks;
	return brd.bRooks;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Bishops(const board& brd){
	if constexpr (white) return brd.wBishops;
	return brd.bBishops;
}

template<bool white>
__attribute__((always_inline)) static constexpr ull Queens(const board& brd){
	if constexpr (white) return brd.wQueens;
	return brd.bQueens;
}