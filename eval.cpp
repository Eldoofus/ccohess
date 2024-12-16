#define S(mg, eg) ((int)((unsigned int)(eg) << 16) + (mg))
#define MgScore(s) ((int16_t)((uint16_t)((unsigned)((s)))))
#define EgScore(s) ((int16_t)((uint16_t)((unsigned)((s) + 0x8000) >> 16)))

class evaluator {
  public:
    static inline struct {
        ull attackedBy[2][7];
        ull passedPawns;
        ull mobilityArea[2];
        ull kingZone[2];
        int16_t attackPower[2];
        int16_t attackCount[2];
    } EvalInfo;

    enum PieceValue {
        P_MG =  104, P_EG =  204,
        N_MG =  420, N_EG =  632,
        B_MG =  427, B_EG =  659,
        R_MG =  569, R_EG = 1111,
        Q_MG = 1485, Q_EG = 1963
    };
    static inline constexpr int PieceTypeValue[6] = { S(P_MG, P_EG), S(N_MG, N_EG), S(B_MG, B_EG), S(R_MG, R_EG), S(Q_MG, Q_EG), 0};
    static inline constexpr int PieceValue[2][12] = {
        { P_MG, N_MG, B_MG, R_MG, Q_MG, 0, P_MG, N_MG, B_MG, R_MG, Q_MG, 0 },
        { P_EG, N_EG, B_EG, R_EG, Q_EG, 0, P_EG, N_EG, B_EG, R_EG, Q_EG, 0 }
    };
    static inline constexpr int PieceSqValue[6][64] = { //blacked POV ;)
        { S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
        S( 57,  1), S( 49, 22), S( 34, 65), S( 74, 31), S( 76, 36), S( 85, 27), S(-34, 91), S(-49, 63),
        S( 15, 77), S( 14, 84), S( 42, 43), S( 47, 17), S( 64, 21), S(125, 22), S( 98, 68), S( 43, 76),
        S(-15, 43), S(-13, 20), S( -8,  2), S( -1,-22), S( 21,-15), S( 29,-14), S(  5, 13), S(  8, 18),
        S(-27, 15), S(-32, 10), S(-17, -8), S( -8,-18), S( -2,-15), S(  3,-15), S(-15, -5), S(-10, -7),
        S(-34,  4), S(-35, -2), S(-27, -5), S(-24, -7), S(-14,  0), S(-13, -1), S(  2,-16), S(-12,-14),
        S(-19, 12), S( -9, 13), S(-11,  8), S(-11, 16), S( -4, 31), S( 15,  9), S( 31, -2), S( -6,-13),
        S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0) },

        { S(-201,-70), S(-116,-15), S(-145, 31), S(-58,  1), S( -7,  9), S(-133, 34), S(-83,-15), S(-153,-115),
        S( -9,-24), S( -6,  3), S( 38,-11), S( 46, 16), S( 52, -2), S( 69,-34), S(-12,  0), S(  8,-47),
        S(-15, -7), S( 23,  7), S( 24, 46), S( 43, 45), S( 84, 20), S( 69, 23), S( 33,-15), S( -3,-25),
        S( 13,  6), S( 23, 23), S( 45, 50), S( 52, 62), S( 40, 57), S( 72, 39), S( 25, 17), S( 39, -7),
        S( 10, 12), S( 22, 14), S( 27, 52), S( 36, 49), S( 35, 54), S( 44, 41), S( 48, 11), S( 30, 14),
        S(-13,-31), S( -5,  6), S(  9, 22), S( 12, 44), S( 24, 40), S( 17, 16), S( 21,  4), S( 11, -8),
        S(-32,-29), S(-30,  1), S(-13, -5), S(  0, 16), S(  0,  9), S(-10, -5), S(-15,-10), S( -1,  0),
        S(-82,-53), S(-18,-37), S(-30, -9), S( -1, 12), S(  5, 14), S(  5,-16), S(-14,-12), S(-45,-30) },

        { S(-35, 47), S(-57, 38), S(-128, 54), S(-126, 56), S(-128, 52), S(-143, 43), S(-28, 21), S(-44, 18),
        S(-14, 15), S( 10, 18), S(  1, 19), S(-25, 30), S( -2, 13), S(-15, 25), S(-28, 27), S(-56, 23),
        S(  2, 26), S( 28, 17), S( 43, 12), S( 32,  9), S( 37, 12), S( 52, 24), S( 19, 24), S( 19, 12),
        S( -7, 16), S( 29, 14), S( 27, 15), S( 49, 40), S( 39, 25), S( 31, 19), S( 33,  7), S( -5, 17),
        S( 13,  0), S( 10,  5), S( 23, 23), S( 39, 29), S( 29, 25), S( 34, 11), S( 16,  7), S( 41,-11),
        S(  9, -3), S( 43, 21), S( 28, 18), S( 18, 27), S( 27, 32), S( 33, 20), S( 47,  9), S( 37, -9),
        S( 30, -6), S( 24,-23), S( 29,-12), S(  5, 12), S( 14, 12), S( 16, -3), S( 36,-19), S( 35,-44),
        S( 26,-16), S( 41,  4), S( 25, 17), S(  6, 15), S( 21, 14), S( 15, 13), S( 29, -2), S( 39,-27) },

        { S( 24, 56), S( 24, 67), S(-12, 88), S( -8, 76), S(  3, 75), S(  7, 77), S( 25, 73), S( 40, 64),
        S(-13, 63), S(-26, 81), S( -1, 83), S(  7, 76), S( -4, 72), S(  8, 52), S( -3, 56), S( 19, 42),
        S(-13, 62), S( 39, 47), S( 15, 58), S( 39, 35), S( 57, 24), S( 52, 32), S( 81, 17), S( 23, 35),
        S(-13, 67), S(  8, 59), S( 14, 59), S( 32, 37), S( 27, 31), S( 33, 25), S( 35, 27), S( 16, 34),
        S(-22, 51), S(-26, 58), S(-23, 55), S(-16, 47), S(-18, 41), S(-30, 38), S( -1, 29), S(-15, 26),
        S(-25, 32), S(-23, 36), S(-21, 35), S(-18, 30), S(-13, 26), S(-11, 11), S( 14, -3), S( -8,  5),
        S(-35, 33), S(-16, 31), S( -5, 33), S( -4, 25), S(  2, 17), S(-14, 11), S(  5,  4), S(-31, 21),
        S(-11, 38), S(-13, 35), S(-11, 39), S( -1, 21), S(  4, 14), S(  2, 21), S(  5, 20), S(-11, 20) },

        { S(-42, 81), S(-21, 93), S(-12,125), S(  4,130), S( -3,148), S( 35,138), S( 33,125), S( 18,115),
        S( -9, 54), S(-48,103), S(-28,113), S(-84,202), S(-88,236), S(-28,170), S(-43,163), S( 11,146),
        S( -3, 50), S(  5, 40), S(  1, 89), S(-10,128), S( -3,159), S( 29,157), S( 49,114), S( 13,140),
        S(  6, 25), S( 13, 65), S( -1, 79), S(-11,130), S(-14,162), S( -9,171), S( 27,155), S(  9,123),
        S( 20,  5), S(  5, 50), S(  5, 57), S(  0, 99), S( -1,104), S(  6, 82), S( 26, 70), S( 29, 69),
        S( 10,-21), S( 18, 10), S(  7, 37), S(  7, 32), S( 13, 35), S( 18, 32), S( 43,  1), S( 31, -7),
        S( 13,-31), S( 16,-22), S( 22,-26), S( 23,  8), S( 26, -1), S( 22,-83), S( 40,-115), S( 33,-73),
        S( -2,-39), S( -9,-38), S(  1,-31), S( 11,-35), S(  7,-37), S( -9,-44), S(  7,-77), S( 14,-55) },

        { S(-24,-68), S( 23, 15), S( 10, 45), S(  7, 82), S(  1, 62), S( 26, 76), S( 45, 91), S( 16,-57),
        S( -7, 34), S( 52,112), S( 53,118), S( 81,107), S( 85,108), S( 94,125), S( 98,139), S( 47, 54),
        S( 31, 63), S(129,112), S(114,133), S( 91,148), S(133,144), S(165,148), S(150,133), S( 49, 72),
        S( 33, 59), S(102, 93), S( 95,135), S( 58,164), S( 71,164), S(123,139), S(114,111), S(  0, 79),
        S( 25, 31), S(114, 68), S(119,107), S( 36,146), S( 79,134), S(114,108), S(111, 74), S(-41, 68),
        S( 32, 25), S(117, 55), S( 86, 83), S( 63,103), S( 81, 99), S( 69, 90), S( 86, 67), S( -3, 52),
        S( 85, 25), S( 84, 50), S( 72, 66), S( 19, 86), S( 31, 86), S( 47, 75), S( 84, 50), S( 60, 25),
        S( 37,-41), S( 89, -1), S( 64, 30), S(-39, 43), S( 33, 17), S(-18, 50), S( 66,  3), S( 36,-41) },
    };
    static inline constexpr int Tempo = 18;
    static inline constexpr int PawnDoubled  = S(-11,-48);
    static inline constexpr int PawnDoubled2 = S(-10,-25);
    static inline constexpr int PawnIsolated = S(-8,-16);
    static inline constexpr int PawnSupport  = S(22, 17);
    static inline constexpr int PawnThreat   = S(80, 34);
    static inline constexpr int PushThreat   = S(25, 6);
    static inline constexpr int PawnOpen     = S(-14,-19);
    static inline constexpr int BishopPair   = S(33,110);
    static inline constexpr int KingAtkPawn  = S(-16, 45);
    static inline constexpr int OpenForward  = S(28, 31);
    static inline constexpr int SemiForward  = S(17, 15);
    static inline constexpr int NBBehindPawn = S(9, 32);
    static inline constexpr int BishopBadP   = S(-1, -5);
    static inline constexpr int Shelter      = S(31,-12);
    static inline constexpr int PawnPassed[8] = { S(0, 0), S(-14, 25), S(-19, 40), S(-72,115), S(-38,146), S(60,175), S(311,218), S(0, 0) };
    static inline constexpr int PassedDefended[8] = { S(0, 0), S(0, 0), S(3,-14), S(3,-10), S(0, 33), S(32,103), S(158, 96), S(0, 0) };
    static inline constexpr int PassedBlocked[4] = { S(1, -4), S(-6, 6), S(-11,-11), S(-54,-52) };
    static inline constexpr int PassedFreeAdv[4] = { S(-4, 25), S(-12, 58), S(-23,181), S(-70,277) };
    static inline constexpr int PassedDistUs[4] = { S(18,-32), S(17,-44), S(2,-49), S(-19,-37) };
    static inline constexpr int PassedDistThem = S(-4, 19);
    static inline constexpr int PassedRookBack = S(21, 46);
    static inline constexpr int PassedSquare   = S(-26,422);
    static inline constexpr int PawnPhalanx[8] = { S(0, 0), S(10, -2), S(20, 14), S(35, 37), S(72,121), S(231,367), S(168,394), S(0, 0) };
    static inline constexpr int ThreatByMinor[8] = { S(0, 0), S(0, 0), S(27, 42), S(39, 42), S(67, 11), S(59,-17), S(0, 0), S(0, 0) };
    static inline constexpr int ThreatByRook[8] = { S(0, 0), S(0, 0), S(24, 43), S(31, 45), S(-17, 47), S(93,-73), S(0, 0), S(0, 0) };
    static inline constexpr int KingLineDanger[28] = {
        S(0, 0), S(0, 0), S(15, 0), S(11, 21),
        S(-16, 35), S(-25, 30), S(-29, 29), S(-37, 38),
        S(-48, 41), S(-67, 43), S(-67, 40), S(-80, 43),
        S(-85, 43), S(-97, 44), S(-109, 46), S(-106, 41),
        S(-116, 41), S(-123, 37), S(-126, 34), S(-131, 29),
        S(-138, 28), S(-155, 26), S(-149, 23), S(-172, 9),
        S(-148, -8), S(-134,-26), S(-130,-32), S(-135,-34)
    };
    static inline constexpr int Mobility[4][28] = {
        { S(-44,-139), S(-31, -7), S(-10, 60), S(0, 86), S(13, 89), S(22,102), S(32,102), S(43,101), S(54, 81) },
        { S(-51,-81), S(-26,-40), S(-11, 19), S(-3, 53), S(9, 65), S(21, 83), S(26, 98), S(32,104), S(32,114), S(35,116), S(41,115), S(57,106), S(50,115), S(100, 76) },
        { S(-105,-146), S(-15, 18), S(-1, 82), S(5, 88), S(2,121), S(6,133), S(5,144), S(12,146), S(14,152), S(19,157), S(24,164), S(23,171), S(25,177), S(36,177), S(72,154) },
        { S(-63,-48), S(-97,-54), S(-89,-107), S(-17,-127), S(0,-52), S(-8, 72), S(-2,142), S(-2,184), S(1,215), S(5,230), S(7,243), S(9,254), S(15,255), S(15,268), S(16,279), S(18,283), S(16,294), S(16,302), S(12,313), S(13,321), S(23,314), S(22,318), S(48,298), S(58,279), S(122,221), S(135,193), S(146,166), S(125,162) }
    };
    static inline constexpr int AttackPower[4] = { 36, 22, 23, 78 };
    static inline constexpr int CheckPower[4]  = { 68, 44, 88, 92 };
    static inline constexpr int CountModifier[8] = { 0, 0, 63, 126, 96, 124, 124, 128 };

    template<bool white>
    static inline int EvalPawns(const board& b) {
        int count, eval = 0;
        ull pawns = Pawns<white>(b);
        ull pawnAttacks = Pawn_Attacks<white>(pawns);
        // Doubled pawns (one directly in front of the other)
        count = bitCount(pawns & Pawn_Forward<white>(pawns));
        eval += PawnDoubled * count;
        // Doubled pawns (one square between them)
        count = bitCount(pawns & Pawn_Forward2<white>(pawns));
        eval += PawnDoubled2 * count;
        // Pawns defending pawns
        count = bitCount(pawns & Pawn_Attacks<!white>(Pawns<!white>(b)));
        eval += PawnSupport * count;
        // Open pawns
        ull open = ~Ahead<!white>(Pawns<!white>(b));
        count = bitCount(pawns & open & ~pawnAttacks);
        eval += PawnOpen * count;
        // Phalanx
        ull phalanx = pawns & ((pawns & Pawns_NotLeft()) >> 1);
        if constexpr (white) forBits(phalanx) eval += PawnPhalanx[square(phalanx) / 8];
        else forBits(phalanx) eval += PawnPhalanx[7 - square(phalanx) / 8];
        // Evaluate each individual pawn
        forBits(pawns){
            ull sq = square(pawns);
            // isolated pawns
            if (!(isomask[sq] & Pawns<white>(b))) eval += PawnIsolated;
            // passed pawns
            if (!((pmask[white][sq]) & Pawns<!white>(b))) {
                eval += PawnPassed[sq / 8];
                if ((1ull << sq) & pawnAttacks) eval += PassedDefended[sq / 8];
                EvalInfo.passedPawns |= 1ull << sq;
            }
        }

        return eval;
    }

    template<bool white, boardPiece pt>
    static inline ull MobilityXray(const ull& loc, const board& b){
        ull occ = b.occupied ^ (b.bQueens | b.wQueens);
        if constexpr (pt == boardPiece::bishop || pt == boardPiece::queen) occ ^= Bishops<!white>(b);
        if constexpr (pt == boardPiece::rook || pt == boardPiece::queen) occ ^= Rooks<!white>(b);
        if constexpr (pt == boardPiece::knight) return Lookup::Knight(loc);
        else if constexpr (pt == boardPiece::bishop) return Lookup::Bishop(loc, occ);
        else if constexpr (pt == boardPiece::rook) return Lookup::Rook(loc, occ);
        else if constexpr (pt == boardPiece::queen) return Lookup::Queen(loc, occ);
    }

    // Evaluates knights, bishops, rooks, or queens
    template<bool white, boardPiece pt>
    static inline int EvalPiece(const board& b) {
        int eval = 0;

        ull pieces;
        if constexpr (pt == boardPiece::knight) pieces = Knights<white>(b);
        if constexpr (pt == boardPiece::bishop) pieces = Bishops<white>(b);
        if constexpr (pt == boardPiece::rook) pieces = Rooks<white>(b);
        if constexpr (pt == boardPiece::queen) pieces = Queens<white>(b);

        // Bishop pair
        if constexpr (pt == boardPiece::bishop) if((pieces & lightSq) && (pieces & darkSq)) eval += BishopPair;

        // Minor behind pawn
        if constexpr (pt == boardPiece::knight || pt == boardPiece::bishop) {
            int count = bitCount(pieces & (Pawn_Backward<white>(b.wPawns | b.bPawns)));
            eval += count * NBBehindPawn;
        }

        EvalInfo.attackedBy[white][(int)pt] = 0;

        // Evaluate each individual piece
        forBits (pieces) {
            ull sq = square(pieces);

            // Mobility
            ull attackBB = MobilityXray<white, pt>(sq, b);
            ull mobilityBB = attackBB & EvalInfo.mobilityArea[white];
            int mob = bitCount(mobilityBB);
            eval += Mobility[(int)pt-1][mob];

            // Attacks for king safety calculations
            int attacks = bitCount(mobilityBB & EvalInfo.kingZone[!white]);
            int checks;
            if constexpr (pt == boardPiece::knight) checks  = bitCount(mobilityBB & Lookup::Knight(square(King<!white>(b))));
            else if constexpr (pt == boardPiece::bishop) checks  = bitCount(mobilityBB & Lookup::Bishop(square(King<!white>(b)), b.occupied));
            else if constexpr (pt == boardPiece::rook) checks  = bitCount(mobilityBB & Lookup::Rook(square(King<!white>(b)), b.occupied));
            else if constexpr (pt == boardPiece::queen) checks  = bitCount(mobilityBB & Lookup::Queen(square(King<!white>(b)), b.occupied));

            if (attacks > 0 || checks > 0) {
                EvalInfo.attackCount[white]++;
                EvalInfo.attackPower[white] +=  attacks * AttackPower[(int)pt-1]
                                            +   checks  * CheckPower[(int)pt-1];
            }

            EvalInfo.attackedBy[white][(int)pt]  |= attackBB;
            EvalInfo.attackedBy[white][6] |= attackBB;

            // Penalty for having pawns on the same color squares as a bishop
            if constexpr (pt == boardPiece::bishop) {
                ull bishopSquares   = ((1ull << sq) & darkSq) ? darkSq : lightSq;
                ull badPawns        = Pawns<white>(b) & bishopSquares;
                ull blockedBadPawns = Pawn_Backward<white>(b.occupied) & Pawns<white>(b) & fileMid;

                int count = bitCount(badPawns) * bitCount(blockedBadPawns);
                eval += count * BishopBadP;
            }

            // Forward mobility for rooks
            if (pt == boardPiece::rook) {
                ull forward = Ahead<white>(1ull << sq);
                if (!(forward & (b.bPawns | b.wPawns))) eval += OpenForward;
                else if (!(forward & Pawns<white>(b))) eval += SemiForward;
            }
        }

        return eval;
    }

    // Evaluates kings
    template<bool white>
    static inline int EvalKings(const board& b) {
        int eval = 0;

        ull kingSq = square(King<white>(b));

        // Open lines from the king
        ull SafeLine = FirstRank<white>();
        int count = bitCount(~SafeLine & Lookup::Queen(kingSq, OwnColor<white>(b) | Pawns<!white>(b)));
        eval += KingLineDanger[count];

        // King threatening a pawn
        if (Lookup::King(kingSq) & Pawns<!white>(b)) {
            eval += KingAtkPawn;
        }

        // King safety
        int danger =  EvalInfo.attackPower[!white]
                    * CountModifier[min(7, (int)EvalInfo.attackCount[!white])];

        eval -= S(danger / 128, 0);

        // Pawn shelter
        ull pawnsInFront = (b.bPawns | b.wPawns) & pmask[white][kingSq];
        ull ourPawns = pawnsInFront & OwnColor<white>(b) & ~Pawn_Attacks<!white>(Pawns<!white>(b));

        count = bitCount(ourPawns);
        eval += count * Shelter;

        return eval;
    }

    // Evaluates all non-pawns
    static inline int EvalPieces(const board& b) {
        return EvalPiece<true, boardPiece::knight>(b)
             - EvalPiece<false, boardPiece::knight>(b)
             + EvalPiece<true, boardPiece::bishop>(b)
             - EvalPiece<false, boardPiece::bishop>(b)
             + EvalPiece<true, boardPiece::rook>(b)
             - EvalPiece<false, boardPiece::rook>(b)
             + EvalPiece<true, boardPiece::queen>(b)
             - EvalPiece<false, boardPiece::queen>(b)
             + EvalKings<true>(b)
             - EvalKings<false>(b);
    }

    // Evaluates passed pawns
    template<bool white, bool toPlay>
    static inline int EvalPassedPawns(const board& b) {
        int eval = 0, count;

        ull passers = OwnColor<white>(b) & EvalInfo.passedPawns;

        forBits (passers) {
            ull sq = square(passers);
            ull forward = Pawn_Forward<white>(sq);
            int rank = sq / 8;
            if constexpr (!white) rank = 7 - rank;
            int r = rank - 3;
            ull promoSq = sq % 8;
            if constexpr (!white) promoSq += 56;

            if (rank < 3) continue;

            // Square rule
            if (OwnColor<!white>(b) == (Pawns<!white>(b) | King<!white>(b)) && ::distance[sq*64 + promoSq] < ::distance[square(King<!white>(b))*64 + promoSq] - ((!white) == toPlay))
                eval += PassedSquare;

            // distance to own king
            count = ::distance[forward * 64 + square(King<white>(b))];
            eval += count * PassedDistUs[r];

            // distance to enemy king
            count = (rank - 2) * ::distance[forward * 64 + square(King<!white>(b))];
            eval += count * PassedDistThem;

            // Blocked from advancing
            if (b.occupied & (1ull << forward)) eval += PassedBlocked[r];
            // Free to advance
            else if (!((1ull << forward) & EvalInfo.attackedBy[!white][6])) eval += PassedFreeAdv[r];

            // Rook supporting from behind
            if (Rooks<white>(b) & Ahead<!white>(1ull << sq)) eval += PassedRookBack;
        }

        return eval;
    }

    // Evaluates threats
    template<bool white>
    static inline int EvalThreats(const board& b) {
        int count, eval = 0;
        ull threats;

        // Our pawns threatening their non-pawns
        ull ourPawns = Pawns<white>(b);
        ull theirNonPawns = OwnColor<!white>(b) ^ Pawns<!white>(b);

        count = bitCount(Pawn_Attacks<white>(ourPawns) & theirNonPawns);
        eval += PawnThreat * count;

        // Our pawns that can step forward to threaten their non-pawns
        ull pawnPushes = Pawn_Forward<white>(ourPawns) & ~b.occupied;

        count = bitCount(Pawn_Forward<white>(pawnPushes) & theirNonPawns);
        eval += PushThreat * count;

        // Threats by minor pieces
        ull targets = theirNonPawns & ~King<!white>(b);
        threats = targets & (EvalInfo.attackedBy[white][(int)boardPiece::knight] | EvalInfo.attackedBy[white][(int)boardPiece::bishop]);
        while (threats) eval += ThreatByMinor[(int)b.getPiece(popBit(threats))];

        // Threats by rooks
        targets &= ~EvalInfo.attackedBy[!white][(int)boardPiece::pawn];
        threats = targets & EvalInfo.attackedBy[white][(int)boardPiece::rook];
        while (threats) eval += ThreatByRook[(int)b.getPiece(popBit(threats))];

        return eval;
    }

    // Initializes the eval info struct
    template<bool white>
    static inline void InitEvalInfo(const board& b) {
        ull m, pawns = Pawns<white>(b);

        // Mobility area is defined as any square not attacked by an enemy pawn, nor
        // occupied by our own pawn either on its starting square or blocked from advancing.
        m = pawns & (Pawns_FirstRank<white>() | Pawn_Backward<white>(b.occupied));
        EvalInfo.mobilityArea[white] = ~(m | Pawn_Attacks<!white>(Pawns<!white>(b)));

        // King Safety
        EvalInfo.kingZone[white] = Lookup::King(square(King<white>(b)));

        EvalInfo.attackPower[white] = -30;
        EvalInfo.attackCount[white] = 0;

        // Clear passed pawns, filled in during pawn eval
        EvalInfo.passedPawns = 0;

        EvalInfo.attackedBy[white][(int)boardPiece::king] = Lookup::King(square(King<white>(b)));
        EvalInfo.attackedBy[white][(int)boardPiece::pawn] = Pawn_Attacks<white>(pawns);
        EvalInfo.attackedBy[white][6] = EvalInfo.attackedBy[white][(int)boardPiece::king] | EvalInfo.attackedBy[white][(int)boardPiece::pawn];
    }

    // Calculate scale factor to lower overall eval based on various features
    static inline int ScaleFactor(const board& b, const int eval) {

        // Scale down eval the fewer pawns the stronger side has
        bool strong = eval > 0 ? true : false;
        ull strongPawns = Pawns<false>(b);
        if (strong) strongPawns = Pawns<true>(b);

        int strongPawnCount = bitCount(strongPawns);
        int x = 8 - strongPawnCount;
        int pawnScale = 128 - x * x;

        // Scale down when there aren't pawns on both sides of the board
        if (!(strongPawns & queenside) || !(strongPawns & kingside)) pawnScale -= 20;

        int wnP = bitCount(b.whites ^ (b.wKings | b.wPawns));
        int bnP = bitCount(b.blacks ^ (b.bKings | b.bPawns));
        // Opposite-colored bishop
        if (   wnP <= 2
            && bnP <= 2
            && wnP == bnP
            && bitCount(b.wBishops) == 1
            && bitCount(b.bBishops) == 1
            && bitCount((b.wBishops | b.bBishops) & darkSq) == 1)
            return min((wnP == 1 ? 64 : 96), pawnScale);

        return pawnScale;
    }

    // Calculate a static evaluation of a position
    static inline int EvalMaterial(const board& b) {
        ull wboards[6] = {
			b.wPawns,
			b.wKnights,
			b.wBishops,
			b.wRooks,
			b.wQueens,
			b.wKings
		};

		ull bboards[6] = {
			b.bPawns,
			b.bKnights,
			b.bBishops,
			b.bRooks,
			b.bQueens,
			b.bKings
		};

        int eval = 0;

        for(int i=0;i<6;i++){
            eval += bitCount(wboards[i]) * PieceTypeValue[i];
            forBits(wboards[i]){
                ull sq = square(wboards[i]);
                eval += PieceSqValue[i][sq ^ 56];
            }
        }
        for(int i=0;i<6;i++){
            eval -= bitCount(bboards[i]) * PieceTypeValue[i];
            forBits(bboards[i]){
                ull sq = square(bboards[i]);
                eval -= PieceSqValue[i][sq];
            }
        }

        return eval;
    }

    static inline int EvalPhase(const board& b){
        int value = 0;
        value += bitCount(b.wKnights);
        value += bitCount(b.wBishops);
        value += bitCount(b.bKnights);
        value += bitCount(b.bBishops);
        value += bitCount(b.wRooks) * 2;
        value += bitCount(b.bRooks) * 2;
        value += bitCount(b.wQueens) * 4;
        value += bitCount(b.bQueens) * 4;

        return (value * 256 + 12) / 24;
    }

    template<class status status> // side to play, en passant rights, castling rights
    static long long eval(const board& b){
        InitEvalInfo<true>(b);
        InitEvalInfo<false>(b);

        // Material (includes PSQT) + trend
        int eval = EvalMaterial(b);

        // Evaluate pawns
        eval += EvalPawns<true>(b) - EvalPawns<false>(b);

        // Evaluate pieces
        eval += EvalPieces(b);

        // Evaluate passed pawns
        eval +=  EvalPassedPawns<true, status.wMove>(b) - EvalPassedPawns<false, status.wMove>(b);

        // Evaluate threats
        eval +=  EvalThreats<true>(b) - EvalThreats<false>(b);

        // Adjust eval by scale factor
        int scale = ScaleFactor(b, eval);

        // Evaluate Phase
        int phase = EvalPhase(b);

        // Adjust score by phase
        eval = (MgScore(eval) * phase + EgScore(eval) * (256 - phase) * scale / 128) / 256;

        // Return the evaluation, negated if we are black + tempo bonus
        return eval + (status.wMove ? Tempo : -Tempo);
    }
    EvalToTemplate(eval);
};