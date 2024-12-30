#include<bits/stdc++.h>
#include<x86intrin.h>
using namespace std;

typedef unsigned long long ull;

#define square(x) _tzcnt_u64(x)
#define forBits(x) for(;x;x=_blsr_u64(x))
#define bitCount(x) _popcnt64(x)

inline static ull popBit(ull& val) {
    ull lsb = _blsi_u64(val);
    val ^= lsb;
    return lsb;
}

#include"map.cpp"
#include"board.cpp"
#include"pieces.cpp"

void addMove(ull from, ull to, char promo = ' ');
//void addEval(ull e, const char* str = __builtin_FUNCTION());
void addEval(long long e);

chrono::high_resolution_clock::time_point start, end;
ull searchtime;

namespace Lookup {
    __attribute__((always_inline)) static constexpr ull King(ull loc){
        return kMap[loc];
    }

    __attribute__((always_inline)) static constexpr ull Knight(ull loc){
        return nMap[loc];
    }

    __attribute__((always_inline)) static constexpr ull Rook(ull loc, ull occ){
        return rMap[loc * 4096 + _pext_u64(occ, rMask[loc])];
    }

    __attribute__((always_inline)) static constexpr ull Bishop(ull loc, ull occ){
        return bMap[loc * 512 + _pext_u64(occ, bMask[loc])];
    }

    __attribute__((always_inline)) static constexpr ull Rook_Xray(ull loc, ull occ){
        return rXray[loc * 4096 + _pext_u64(occ, rMask[loc])];
    }

    __attribute__((always_inline)) static constexpr ull Bishop_Xray(ull loc, ull occ){
        return bXray[loc * 512 + _pext_u64(occ, bMask[loc])];
    }

    __attribute__((always_inline)) static constexpr ull Queen(ull loc, ull occ){
        return Rook(loc, occ) | Bishop(loc, occ);
    }

    template<class status status>
    static constexpr ull zobrist(const board& brd, const ull& EP){
        ull pieces[12] = {
            brd.bPawns,
            brd.wPawns,
            brd.bKnights,
            brd.wKnights,
            brd.bBishops,
            brd.wBishops,
            brd.bRooks,
            brd.wRooks,
            brd.bQueens,
            brd.wQueens,
            brd.bKings,
            brd.wKings
        }, z = 0;

        if constexpr (status.wMove) z ^= z64[780];
        if constexpr (status.wCastleR) z ^= z64[768];
        if constexpr (status.wCastleL) z ^= z64[769];
        if constexpr (status.bCastleR) z ^= z64[770];
        if constexpr (status.bCastleL) z ^= z64[771];
        if constexpr (status.hasEP) {
            if constexpr (status.wMove) { if (brd.wPawns & epCheck[square(EP) % 8]) z ^= z64[772 + square(EP) % 8]; }
            else { if (brd.bPawns & epCheck[8 + square(EP) % 8]) z ^= z64[772 + square(EP) % 8]; }
        }

        for(int i=0;i<12;i++) forBits(pieces[i]) z ^= z64[square(pieces[i]) + i * 64];

        return z;
    }
};

#include"eval.cpp"

namespace mGen {
    ull checks[256];
    ull myking[256];
    ull enking[256];
};

int butterfly[2][64][64] = { };

typedef long long (*mptr)(const board&, ull, ull, int);

struct move {
    mptr move;
    unsigned int ft;

    __attribute__((always_inline)) inline ull from(){
        return 1ull << ((ft >> 6) & 0b111111);
    }
    __attribute__((always_inline)) inline ull to(){
        return 1ull << (ft & 0b111111);
    }
    __attribute__((always_inline)) inline ull score(){
        return ft >> 17;
    }

    __attribute__((always_inline)) inline long long operator()(const board& b, const int& depth){
        return this->move(b, from(), to(), depth);
    }

    __attribute__((always_inline)) inline bool operator==(const ::move& m){
        return move == m.move && (ft & 0b111111111111) == (m.ft & 0b111111111111);
    }
    __attribute__((always_inline)) inline bool operator>(const ::move& m){
        return (ft >> 17) > (m.ft >> 17);
    }

    __attribute__((always_inline)) inline void operator()(const bool w, mptr m, const ull& from, const ull& to, const char& p = 0){
        move = m;
        ft = ((square(from) & 0b111111) << 6) | (square(to) & 0b111111) | ((p & 0b11111) << 12) | ((butterfly[w][square(from)][square(to)] & 0b111111111111111) << 17);
    }
};

struct ttEntry {
    ull hash;
    ::move m;
    long long score;
    int depth, bound;
};

int pvtable[256][256] = { };
ttEntry TT[0x1000000] = { };

namespace mList {
    ull epSquare = { };

    ull rPin = { };
    ull bPin = { };

    template<class status status>
    __attribute__((always_inline)) inline void init(const board& brd, int depth){
        constexpr bool white = status.wMove;
        constexpr bool enemy = !status.wMove;
        mGen::myking[depth] =  Lookup::King(square(King<white>(brd)));
        mGen::enking[depth] = Lookup::King(square(King<enemy>(brd)));

        const ull pl = Pawn_AttackLeft<enemy>(Pawns<enemy>(brd) & Pawns_NotLeft());
        const ull pr = Pawn_AttackRight<enemy>(Pawns<enemy>(brd) & Pawns_NotRight());

        if (pl & King<white>(brd)) mGen::checks[depth] = Pawn_AttackRight<white>(King<white>(brd));
        else if (pr & King<white>(brd)) mGen::checks[depth] = Pawn_AttackLeft<white>(King<white>(brd));
        else mGen::checks[depth] = 0xffffffffffffffffull;

        ull knightcheck = Lookup::Knight(square(King<white>(brd))) & Knights<enemy>(brd);
        if (knightcheck) mGen::checks[depth] = knightcheck;

        for(int i=0;i<65536;i++) pvtable[i / 256][i % 256] = 0;
    }

    __attribute__((always_inline)) inline void initEP(ull ep){
        epSquare = ep;
    }

    template<class status status>
    __attribute__((always_inline)) inline void pinD12(ull king, ull enemy, const board& brd, ull& epTarget){
        const ull pinmask = pMap[king * 64 + enemy];

        if constexpr (status.hasEP) if (pinmask & epTarget) epTarget = 0;
        if (pinmask & OwnColor<status.wMove>(brd)) bPin |= pinmask;
    }

    template<class status status>
    __attribute__((always_inline)) inline void pinHV(ull king, ull enemy, const board& brd){
        const ull pinmask = pMap[king * 64 + enemy];
        if (pinmask & OwnColor<status.wMove>(brd)) rPin |= pinmask;
    }


    template<bool white>
    __attribute__((always_inline)) static constexpr ull epRank(){
        if constexpr (white) return 0xFFull << 32;
        else return 0xFFull << 24;
    }

    template<class status status>
    __attribute__((always_inline)) inline void pinEP(ull kingsquare, ull king, ull enemyRQ, const board& brd){
        constexpr bool white = status.wMove;
        const ull pawns = Pawns<status.wMove>(brd);

        if ((epRank<white>() & king) && (epRank<white>() & enemyRQ) && (epRank<white>() & pawns)){
            ull epLpawn = pawns & ((epSquare & Pawns_NotRight()) << 1);
            ull epRpawn = pawns & ((epSquare & Pawns_NotLeft()) >> 1);

            if (epLpawn) {
                ull afterEPocc = brd.occupied & ~(epSquare | epLpawn);
                if ((Lookup::Rook(kingsquare, afterEPocc) & epRank<white>()) & enemyRQ) epSquare = 0;
            }
            if (epRpawn) {
                ull afterEPocc = brd.occupied & ~(epSquare | epRpawn);
                if ((Lookup::Rook(kingsquare, afterEPocc) & epRank<white>()) & enemyRQ) epSquare = 0;
            }
        }
    }

    __attribute__((always_inline)) inline void sliderCheck(ull king, ull enemy, ull& kingban, ull& checkmask) {
        if (checkmask == 0xffffffffffffffffull) checkmask = pMap[king * 64 + enemy];
        else checkmask = 0;
        kingban |= cMap[king * 64 + enemy];
    }

    template<class status status>
    __attribute__((always_inline)) inline ull refresh(const board& brd, ull& kingban, ull& checkmask, int depth){
        constexpr bool white = status.wMove;
        constexpr bool enemy = !status.wMove;
        const ull king = King<white>(brd);
        const ull kingsq = square(king);

        rPin = 0; bPin = 0;

        if (rMap[kingsq * 4096] & EnemyRookQueen<white>(brd)){
            ull atkHV = Lookup::Rook(kingsq, brd.occupied) & EnemyRookQueen<white>(brd);
            forBits(atkHV) {
                ull sq = square(atkHV);
                sliderCheck(kingsq, sq, kingban, checkmask);
            }

            ull pinnersHV = Lookup::Rook_Xray(kingsq, brd.occupied) & EnemyRookQueen<white>(brd);
            forBits(pinnersHV) pinHV<status>(kingsq, square(pinnersHV), brd);
        }
        if (bMap[kingsq * 512] & EnemyBishopQueen<white>(brd)){
            ull atkD12 = Lookup::Bishop(kingsq, brd.occupied) & EnemyBishopQueen<white>(brd);
            forBits(atkD12) {
                ull sq = square(atkD12);
                sliderCheck(kingsq, sq, kingban, checkmask);
            }

            ull pinnersD12 = Lookup::Bishop_Xray(kingsq, brd.occupied) & EnemyBishopQueen<white>(brd);
            forBits(pinnersD12) pinD12<status>(kingsq, square(pinnersD12), brd, mList::epSquare);
        }

        if constexpr (status.hasEP) pinEP<status>(kingsq, king, EnemyRookQueen<white>(brd), brd);

        mGen::checks[depth - 1] = 0xffffffffffffffffull;

        ull king_atk = mGen::myking[depth] & EnemyOrEmpty<status.wMove>(brd) &~kingban;
        if (king_atk == 0) return 0;

        ull knights = Knights<enemy>(brd);
        forBits(knights) kingban |= Lookup::Knight(square(knights));

        const ull pl = Pawn_AttackLeft<enemy>(Pawns<enemy>(brd) & Pawns_NotLeft());
        const ull pr = Pawn_AttackRight<enemy>(Pawns<enemy>(brd) & Pawns_NotRight());

        kingban |= (pl | pr);

        ull bishops = BishopQueen<enemy>(brd);
        forBits(bishops) {
            const ull sq = square(bishops);
            ull atk = Lookup::Bishop(sq, brd.occupied);
            kingban |= atk;
        }

        ull rooks = RookQueen<enemy>(brd);
        forBits(rooks) {
            const ull sq = square(rooks);
            ull atk = Lookup::Rook(sq, brd.occupied);
            kingban |= atk;
        }

        return king_atk & ~kingban;
    }

    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneLeft(ull& pawn, const ull pinD1D2){
        const ull pinned = pawn & Pawn_InvertLeft<white>(pinD1D2 & Pawns_NotRight());
        const ull unpinned = pawn & ~pinD1D2;

        pawn = (pinned | unpinned);
    }

    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneRight(ull& pawn, const ull pinD1D2){
        const ull pinned = pawn & Pawn_InvertRight<white>(pinD1D2 & Pawns_NotLeft());
        const ull unpinned = pawn & ~pinD1D2;

        pawn = (pinned | unpinned);
    }


    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneLeftEP(ull& pawn, const ull pinD1D2){
        const ull pinned = pawn & Pawn_InvertLeft<white>(pinD1D2 & Pawns_NotRight());
        const ull unpinned = pawn & ~pinD1D2;

        pawn = (pinned | unpinned);
    }

    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneRightEP(ull& pawn, const ull pinD1D2){
        const ull pinned = pawn & Pawn_InvertRight<white>(pinD1D2 & Pawns_NotLeft());
        const ull unpinned = pawn & ~pinD1D2;

        pawn = (pinned | unpinned); 
    }


    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneMove(ull& pawn, const ull pinHV){
        const ull pinned = pawn & Pawn_Backward<white>(pinHV);
        const ull unpinned = pawn & ~pinHV;

        pawn = (pinned | unpinned);
    }

    template<bool white>
    __attribute__((always_inline)) inline void Pawn_PruneMove2(ull& pawn, const ull pinHV){
        const ull pinned = pawn & Pawn_Backward2<white>(pinHV);
        const ull unpinned = pawn & ~pinHV;

        pawn = (pinned | unpinned);
    }

    template<bool white>
    __attribute__((always_inline)) inline void _pawntakes(mptr Pq, mptr Pr, mptr Pb, mptr Pn, mptr Patk, ::move moves[], int& cnt, ull Lpawns, ull Rpawns, const ull& filter){
        ull NoPromote_Left, NoPromote_Right;

        if ((Lpawns | Rpawns) & Pawns_LastRank<white>()){
            ull Promote_Left =  Pawn_AttackLeft<white>(Lpawns & Pawns_LastRank<white>()) & filter;
            ull Promote_Right = Pawn_AttackRight<white>(Rpawns & Pawns_LastRank<white>()) & filter;
            while (Promote_Left)    { const ull pos = popBit(Promote_Left);
                moves[cnt++](white, Pq, Pawn_AttackRight<!white>(pos), pos, 'q');
                moves[cnt++](white, Pr, Pawn_AttackRight<!white>(pos), pos, 'r');
                moves[cnt++](white, Pb, Pawn_AttackRight<!white>(pos), pos, 'b');
                moves[cnt++](white, Pn, Pawn_AttackRight<!white>(pos), pos, 'n');
            }
            while (Promote_Right)   { const ull pos = popBit(Promote_Right);
                moves[cnt++](white, Pq, Pawn_AttackLeft<!white>(pos), pos, 'q');
                moves[cnt++](white, Pr, Pawn_AttackLeft<!white>(pos), pos, 'r');
                moves[cnt++](white, Pb, Pawn_AttackLeft<!white>(pos), pos, 'b');
                moves[cnt++](white, Pn, Pawn_AttackLeft<!white>(pos), pos, 'n');
            }
            NoPromote_Left =  Pawn_AttackLeft<white>(Lpawns & ~Pawns_LastRank<white>()) & filter;
            NoPromote_Right = Pawn_AttackRight<white>(Rpawns & ~Pawns_LastRank<white>()) & filter;
        } else {
            NoPromote_Left =  Pawn_AttackLeft<white>(Lpawns) & filter;
            NoPromote_Right = Pawn_AttackRight<white>(Rpawns) & filter;
        }

        while (NoPromote_Left)  { const ull pos = popBit(NoPromote_Left);
            moves[cnt++](white, Patk, Pawn_AttackRight<!white>(pos), pos);
        }
        while (NoPromote_Right) { const ull pos = popBit(NoPromote_Right);
            moves[cnt++](white, Patk, Pawn_AttackLeft<!white>(pos), pos);
        }
    }
    template<bool white>
    __attribute__((always_inline)) inline void _knightmoves(mptr P, ::move moves[], int& cnt, ull knights, const ull& filter){
        forBits(knights) {
            const ull sq = square(knights);
            ull m = Lookup::Knight(sq) & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
    }
    template<bool white>
    __attribute__((always_inline)) inline void _bishopmoves(mptr P, const ull& pinD12, const board& brd, ::move moves[], int& cnt, ull bish_pinned, ull bish_nopin, const ull& filter){
        forBits(bish_pinned) {
            const ull sq = square(bish_pinned);
            ull m = Lookup::Bishop(sq, brd.occupied) & pinD12 & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
        forBits(bish_nopin) {
            const ull sq = square(bish_nopin);
            ull m = Lookup::Bishop(sq, brd.occupied) & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
    }
    template<bool white>
    __attribute__((always_inline)) inline void _rookmoves(mptr P, const ull& pinHV, const board& brd, ::move moves[], int& cnt, ull rook_pinned, ull rook_nopin, const ull& filter){
        forBits(rook_pinned) {
            const ull sq = square(rook_pinned);
            ull m = Lookup::Rook(sq, brd.occupied) & pinHV & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
        forBits(rook_nopin) {
            const ull sq = square(rook_nopin);
            ull m = Lookup::Rook(sq, brd.occupied) & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
    }
    template<bool white>
    __attribute__((always_inline)) inline void _queenmoves(mptr P, const board& brd, ::move moves[], int& cnt, ull queens2, const ull& filter){
        forBits(queens2) {
            const ull sq = square(queens2);
            ull m = Lookup::Queen(sq, brd.occupied) & filter;
            while (m) moves[cnt++](white, P, 1ull << sq, popBit(m));
        }
    }
    template<bool white>
    __attribute__((always_inline)) inline void _kingmoves(mptr P, ::move moves[], int& cnt, const ull& king, ull kingatk, const ull& filter){
        kingatk &= filter;
        while (kingatk) moves[cnt++](white, P, king, popBit(kingatk));
    }

    template<class status status, class C, template<class ::status, class> class F, bool caps = false>
    __attribute__((always_inline)) inline int _movegen(const board& brd, ull& kingatk, const ull& kingban, const ull& checkmask, ::move moves[], int depth){
        constexpr bool white = status.wMove;
        constexpr bool enemy = !status.wMove;
        const bool noCheck = (checkmask == 0xffffffffffffffffull);

        const ull pinHV  = rPin;
        const ull pinD12 = bPin;
        const ull movableSquare = EnemyOrEmpty<white>(brd) & checkmask;
        const ull Queenfilter = Queens<enemy>(brd) & checkmask;
        const ull Rookfilter = Rooks<enemy>(brd) & checkmask;
        const ull Bishopfilter = Bishops<enemy>(brd) & checkmask;
        const ull Knightfilter = Knights<enemy>(brd) & checkmask;
        const ull Pawnfilter = Pawns<enemy>(brd) & checkmask;
        const ull Emptyfilter = Empty(brd) & checkmask;

        const ull epTarget = epSquare;
        int cnt = 0;

        mGen::enking[depth - 1] = mGen::myking[depth];

        const ull pawnsLR = Pawns<white>(brd) & ~pinHV;
        const ull pawnsHV = Pawns<white>(brd) & ~pinD12;
        
        ull Lpawns = pawnsLR & Pawn_InvertLeft<white> (Enemy<white>(brd) & Pawns_NotRight() & checkmask);
        ull Rpawns = pawnsLR & Pawn_InvertRight<white>(Enemy<white>(brd) & Pawns_NotLeft() & checkmask);
        ull Fpawns = pawnsHV & Pawn_Backward<white>(Empty(brd));
        ull Ppawns = Fpawns & Pawns_FirstRank<white>() & Pawn_Backward2<white>(Empty(brd) & checkmask);
        
        Fpawns &= Pawn_Backward<white>(checkmask);

        Pawn_PruneLeft<white>(Lpawns, pinD12);
        Pawn_PruneRight<white>(Rpawns, pinD12);
        Pawn_PruneMove<white>(Fpawns, pinHV);
        Pawn_PruneMove2<white>(Ppawns, pinHV);

        const ull knights = Knights<white>(brd) &~ (pinHV | pinD12);

        const ull bishops = Bishops<white>(brd) &~ pinHV;
        const ull bish_pinned = bishops & pinD12;
        const ull bish_nopin = bishops & ~pinD12;

        const ull rooks = Rooks<white>(brd) & ~pinD12;
        const ull rook_pinned = rooks & pinHV;
        const ull rook_nopin = rooks & ~pinHV;

        const ull queens = Queens<white>(brd);
        const ull queensD12 = queens & pinD12;
        const ull queensHV = queens & pinHV;
        const ull queens2 = queens & ~(pinHV | pinD12);

        const ull king = King<white>(brd);
        
        _pawntakes<white>(C::template Pawnpromote<status, F, boardPiece::queen>, C::template Pawnpromote<status, F, boardPiece::rook>, C::template Pawnpromote<status, F, boardPiece::bishop>, C::template Pawnpromote<status, F, boardPiece::knight>, C::template Pawnatk<status, F>, moves, cnt, Lpawns, Rpawns, Queenfilter);
        _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Queenfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Queenfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Queenfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Queenfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Queenfilter);
        _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Queenfilter);
        _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, Queens<enemy>(brd));

        _pawntakes<white>(C::template Pawnpromote<status, F, boardPiece::queen>, C::template Pawnpromote<status, F, boardPiece::rook>, C::template Pawnpromote<status, F, boardPiece::bishop>, C::template Pawnpromote<status, F, boardPiece::knight>, C::template Pawnatk<status, F>, moves, cnt, Lpawns, Rpawns, Rookfilter);
        _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Rookfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Rookfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Rookfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Rookfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Rookfilter);
        _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Rookfilter);
        _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, Rooks<enemy>(brd));

        _pawntakes<white>(C::template Pawnpromote<status, F, boardPiece::queen>, C::template Pawnpromote<status, F, boardPiece::rook>, C::template Pawnpromote<status, F, boardPiece::bishop>, C::template Pawnpromote<status, F, boardPiece::knight>, C::template Pawnatk<status, F>, moves, cnt, Lpawns, Rpawns, Bishopfilter);
        _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Bishopfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Bishopfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Bishopfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Bishopfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Bishopfilter);
        _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Bishopfilter);
        _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, Bishops<enemy>(brd));

        _pawntakes<white>(C::template Pawnpromote<status, F, boardPiece::queen>, C::template Pawnpromote<status, F, boardPiece::rook>, C::template Pawnpromote<status, F, boardPiece::bishop>, C::template Pawnpromote<status, F, boardPiece::knight>, C::template Pawnatk<status, F>, moves, cnt, Lpawns, Rpawns, Knightfilter);
        _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Knightfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Knightfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Knightfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Knightfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Knightfilter);
        _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Knightfilter);
        _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, Knights<enemy>(brd));

        if constexpr (status.hasEP) {
            ull epLpawn = pawnsLR & Pawns_NotLeft()  & ((epTarget & checkmask) << 1);
            ull epRpawn = pawnsLR & Pawns_NotRight() & ((epTarget & checkmask) >> 1);
            if (epLpawn | epRpawn) {
                Pawn_PruneLeftEP<white>(epLpawn, pinD12);
                Pawn_PruneRightEP<white>(epRpawn, pinD12);

                if (epLpawn) moves[cnt++](white, C::template PawnEnpassantTake<status, F>, epLpawn, Pawn_AttackLeft<white>(epLpawn));
                if (epRpawn) moves[cnt++](white, C::template PawnEnpassantTake<status, F>, epRpawn, Pawn_AttackRight<white>(epRpawn));
            }
        }
        _pawntakes<white>(C::template Pawnpromote<status, F, boardPiece::queen>, C::template Pawnpromote<status, F, boardPiece::rook>, C::template Pawnpromote<status, F, boardPiece::bishop>, C::template Pawnpromote<status, F, boardPiece::knight>, C::template Pawnatk<status, F>, moves, cnt, Lpawns, Rpawns, Pawnfilter);
        _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Pawnfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Pawnfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Pawnfilter);
        _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Pawnfilter);
        _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Pawnfilter);
        _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Pawnfilter);
        _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, Pawns<enemy>(brd));

        if constexpr (!caps){
            int captures = cnt;

            if constexpr (status.canCastleLeft()) if (noCheck && status.canCastleLeft(kingban, brd.occupied, Rooks<white>(brd)))
                moves[cnt++](white, C::template KingCastle<status, F, status.rookswitchL()>, King<white>(brd), King<white>(brd) >> 2);
            if constexpr (status.canCastleRight()) if (noCheck && status.canCastleRight(kingban, brd.occupied, Rooks<white>(brd)))
                moves[cnt++](white, C::template KingCastle<status, F, status.rookswitchR()>, King<white>(brd), King<white>(brd) << 2);
            if (Fpawns & Pawns_LastRank<white>()) {
                ull Promote_Move =  Fpawns & Pawns_LastRank<white>();
                ull NoPromote_Move =  Fpawns & ~Pawns_LastRank<white>();

                while (Promote_Move)    { const ull pos = popBit(Promote_Move);
                    moves[cnt++](white, C::template Pawnpromote<status, F, boardPiece::queen>, pos, Pawn_Forward<white>(pos), 'q');
                    moves[cnt++](white, C::template Pawnpromote<status, F, boardPiece::rook>, pos, Pawn_Forward<white>(pos), 'r');
                    moves[cnt++](white, C::template Pawnpromote<status, F, boardPiece::bishop>, pos, Pawn_Forward<white>(pos), 'b');
                    moves[cnt++](white, C::template Pawnpromote<status, F, boardPiece::knight>, pos, Pawn_Forward<white>(pos), 'n');
                }
                while (NoPromote_Move)  { const ull pos = popBit(NoPromote_Move);
                    moves[cnt++](white, C::template Pawnmove<status, F>, pos, Pawn_Forward<white>(pos));
                }
                while (Ppawns)          { const ull pos = popBit(Ppawns); 
                    moves[cnt++](white, C::template Pawnpush<status, F>, pos, Pawn_Forward2<white>(pos));
                }
            } else {
                while (Fpawns) { const ull pos = popBit(Fpawns);
                    moves[cnt++](white, C::template Pawnmove<status, F>, pos, Pawn_Forward<white>(pos));
                }
                while (Ppawns) { const ull pos = popBit(Ppawns);
                    moves[cnt++](white, C::template Pawnpush<status, F>, pos, Pawn_Forward2<white>(pos));
                }
            }
            _knightmoves<white>(C::template Knightmove<status, F>, moves, cnt, knights, Emptyfilter);
            _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, bish_pinned, bish_nopin, Emptyfilter);
            _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, rook_pinned, rook_nopin, Emptyfilter);
            _bishopmoves<white>(C::template Bishopmove<status, F>, pinD12, brd, moves, cnt, queensD12, 0, Emptyfilter);
            _rookmoves<white>(C::template Rookmove<status, F>, pinHV, brd, moves, cnt, queensHV, 0, Emptyfilter);
            _queenmoves<white>(C::template Queenmove<status, F>, brd, moves, cnt, queens2, Emptyfilter);
            _kingmoves<white>(C::template Kingmove<status, F>, moves, cnt, king, kingatk, ~brd.occupied);

            sort(moves + captures, moves + cnt, greater());
        }

        return cnt;
    }

    class VoidClass{};
        
    template<class status status, class C> struct QuiesceMoves;

    template<class status status, class C>
    __attribute__((always_inline)) inline long long _quiesce(const board& brd, ull kingatk, const ull kingban, const ull checkmask, int depth){
        if (depth <= 0) {
            if constexpr (status.wMove) return evaluator::template eval<status>(brd);
            else return -evaluator::template eval<status>(brd);
        } else {
            ull z = Lookup::zobrist<status>(brd, epSquare), oldAlpha = C::alpha[depth];
            ttEntry& tt = TT[z & 0xFFFFFF];
            const bool ttHit = tt.hash == z, incheck = checkmask != 0xffffffffffffffffull;

            if(ttHit){
                if(tt.depth >= depth && (
                    (tt.bound == 0 && tt.score >= C::beta[depth]) ||
                    (tt.bound == 1 && tt.score <= C::alpha[depth]) ||
                    (tt.bound == 2)
                )) {C::skips++; return tt.score;}
            }

            long long eval, best;
            if constexpr (status.wMove) eval = evaluator::template eval<status>(brd);
            else eval = -evaluator::template eval<status>(brd);

            best = eval;
            ::move moves[219], bestmove;

            C::alpha[depth] = max(eval, C::alpha[depth]);
            if (best >= C::beta[depth]) {
                C::cuts++;
                for(int i=depth;i>-1;i--) pvtable[depth][i] = 0;
                return best;
            }

            int cnt;
            if(incheck) cnt = _movegen<status, C, QuiesceMoves>(brd, kingatk, kingban, checkmask, moves, depth);
            else cnt = _movegen<status, C, QuiesceMoves, true>(brd, kingatk, kingban, checkmask, moves, depth);

            if (ttHit) for(int i=0;i<cnt;i++){
                if (tt.m == moves[i]) {
                    moves[i] = moves[0];
                    moves[0] = tt.m;
                }
            }

            long long beta = C::beta[depth];

            for(int i=0;i<cnt;i++){
                //if(!incheck && (moves[i].to() & ~Enemy<status.wMove>(brd))) continue;
                if(i != 0 /*&& C::beta[depth] != C::alpha[depth] + 1*/) {
                    C::beta[depth] = C::alpha[depth] + 1;
                    eval = -moves[i](brd, depth);
                    if(eval > C::alpha[depth]) {
                        C::beta[depth] = beta;
                        eval = -moves[i](brd, depth);
                    }
                } else eval = -moves[i](brd, depth);
                if(eval > best){
                    best = eval;
                    C::alpha[depth] = max(eval, C::alpha[depth]);
                    bestmove = moves[i];
                    pvtable[depth][depth] = bestmove.ft & 0b11111111111111111;
                    for(int j=depth-1;j>-1;j--) pvtable[depth][j] = pvtable[depth-1][j];
                    if(eval >= C::beta[depth]){
                        if(moves[i].to() & ~brd.occupied) butterfly[status.wMove][moves[i].ft >> 6 & 0b111111][moves[i].ft & 0b111111] += depth * depth - butterfly[status.wMove][moves[i].ft >> 6 & 0b111111][moves[i].ft & 0b111111] * depth * depth / 16384;
                        C::cuts++;
                        break;
                    }
                }
            }

            tt.hash = z;
            tt.m = C::alpha[depth] > oldAlpha ? bestmove : tt.m;
            tt.score = best;
            tt.depth = depth;
            tt.bound = best >= C::beta[depth] ? 0 : (C::alpha[depth] == oldAlpha ? 1 : 2);
            
            return best;
        }
    }

    template<class status status, class C>
    struct QuiesceMoves {
        __attribute__((noinline)) static long long e(const board& brd, int depth){
            C::qnodes++;
            ull checkmask = mGen::checks[depth];
            ull kingban = mGen::myking[depth - 1] = mGen::enking[depth];
            ull kingatk = refresh<status>(brd, kingban, checkmask, depth);
            constexpr bool white = status.wMove;

            if (checkmask != 0) return _quiesce<status, C>(brd, kingatk, kingban, checkmask, depth);
            else {
                if (depth == 0) {
                    if constexpr (status.wMove) return evaluator::template eval<status>(brd);
                    else return -evaluator::template eval<status>(brd);
                } else {
                    ull z = Lookup::zobrist<status>(brd, epSquare), oldAlpha = C::alpha[depth];
                    ttEntry& tt = TT[z & 0xFFFFFF];
                    const bool ttHit = tt.hash == z;

                    if(ttHit){
                        if(tt.depth >= depth && (
                            (tt.bound == 0 && tt.score >= C::beta[depth]) ||
                            (tt.bound == 1 && tt.score <= C::alpha[depth]) ||
                            (tt.bound == 2)
                        )) {C::skips++; return tt.score;}
                    }

                    long long eval, best;
                    if constexpr (status.wMove) eval = evaluator::template eval<status>(brd);
                    else eval = -evaluator::template eval<status>(brd);

                    best = eval;
                    ::move moves[8], bestmove;

                    C::alpha[depth] = max(eval, C::alpha[depth]);
                    if (best >= C::beta[depth]) {
                        C::cuts++;
                        for(int i=depth;i>-1;i--) pvtable[depth][i] = 0;
                        return best;
                    }

                    int cnt = 0;
                    const ull king = King<white>(brd);
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Queens<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Rooks<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Bishops<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Knights<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Pawns<!white>(brd));

                    int captures = cnt;
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, 0xffffffffffffffffull);
                    sort(moves + captures, moves + cnt, greater());

                    if (ttHit) for(int i=0;i<cnt;i++){
                        if (tt.m == moves[i]) {
                            moves[i] = moves[0];
                            moves[0] = tt.m;
                        }
                    }
                    
                    long long beta = C::beta[depth];

                    for(int i=0;i<cnt;i++){
                        if(i != 0 /*&& C::beta[depth] != C::alpha[depth] + 1*/) {
                            C::beta[depth] = C::alpha[depth] + 1;
                            eval = -moves[i](brd, depth);
                            if(eval > C::alpha[depth]) {
                                C::beta[depth] = beta;
                                eval = -moves[i](brd, depth);
                            }
                        } else eval = -moves[i](brd, depth);
                        if(eval > best){
                            best = eval;
                            C::alpha[depth] = max(eval, C::alpha[depth]);
                            bestmove = moves[i];
                            pvtable[depth][depth] = bestmove.ft & 0b11111111111111111;
                            for(int j=depth-1;j>-1;j--) pvtable[depth][j] = pvtable[depth-1][j];
                            if(eval >= C::beta[depth]){
                                if(i >= captures) butterfly[white][square(king)][moves[i].ft & 0b111111] += depth * depth - butterfly[white][square(king)][moves[i].ft & 0b111111] * depth * depth / 16384;
                                C::cuts++;
                                break;
                            }
                        }
                    }

                    tt.hash = z;
                    tt.m = C::alpha[depth] > oldAlpha ? bestmove : tt.m;
                    tt.score = best;
                    tt.depth = depth;
                    tt.bound = best >= C::beta[depth] ? 0 : (C::alpha[depth] == oldAlpha ? 1 : 2);
                    
                    return best;
                }
            }
        }
    };
    
    template<class status status, class C> struct EnumerateMoves;

    template<class status status, class C>
    __attribute__((always_inline)) inline long long _enumerate(const board& brd, ull kingatk, const ull kingban, const ull checkmask, int depth){
        if (depth == 25) {
            C::enodes--;
            C::qnodes++;
            return _quiesce<status, C>(brd, kingatk, kingban, checkmask, depth);
        } else {
            long long eval, best = -21474836470ll;
            ::move moves[219], bestmove;

            ull z = Lookup::zobrist<status>(brd, epSquare), oldAlpha = C::alpha[depth];
            ttEntry& tt = TT[z & 0xFFFFFF];
            const bool ttHit = tt.hash == z;

            for(int i=C::fullmove-1;i>C::halfmove;i--){
                if (C::reps[i] == z) return 0;
            }

            if(ttHit){
                if(tt.depth >= depth && (
                    (tt.bound == 0 && tt.score >= C::beta[depth]) ||
                    (tt.bound == 1 && tt.score <= C::alpha[depth]) ||
                    (tt.bound == 2 && C::alpha[depth] + 1 == C::beta[depth])
                )) {C::skips++; return tt.score;}
            }

            int cnt = _movegen<status, C, EnumerateMoves>(brd, kingatk, kingban, checkmask, moves, depth);

            //cout << "_e: " << cnt << " depth: " << depth << endl;
            if(cnt == 0){
                if(checkmask == 0xffffffffffffffffull) return 0;
                else return -2147483647ll - depth;
            }
            
            if (ttHit) for(int i=0;i<cnt;i++){
                if (tt.m == moves[i]) {
                    moves[i] = moves[0];
                    moves[0] = tt.m;
                }
            }

            C::reps[C::fullmove] = z;

            long long beta = C::beta[depth];
            
            for(int i=0;i<cnt;i++){
                if(i != 0 /*&& C::beta[depth] != C::alpha[depth] + 1*/) {
                    C::beta[depth] = C::alpha[depth] + 1;
                    eval = -moves[i](brd, depth);
                    if(eval > C::alpha[depth]) {
                        C::beta[depth] = beta;
                        eval = -moves[i](brd, depth);
                    }
                } else eval = -moves[i](brd, depth);
                if(eval > best){
                    best = eval;
                    C::alpha[depth] = max(eval, C::alpha[depth]);
                    bestmove = moves[i];
                    pvtable[depth][depth] = bestmove.ft & 0b11111111111111111;
                    for(int j=depth-1;j>-1;j--) pvtable[depth][j] = pvtable[depth-1][j];
                    if(eval >= C::beta[depth]){
                        if(moves[i].to() & ~brd.occupied) butterfly[status.wMove][moves[i].ft >> 6 & 0b111111][moves[i].ft & 0b111111] += depth * depth - butterfly[status.wMove][moves[i].ft >> 6 & 0b111111][moves[i].ft & 0b111111] * depth * depth / 16384;
                        C::cuts++;
                        break;
                    }
                }
            }

            tt.hash = z;
            tt.m = C::alpha[depth] > oldAlpha ? bestmove : tt.m;
            tt.score = best;
            tt.depth = depth;
            tt.bound = best >= C::beta[depth] ? 0 : (C::alpha[depth] == oldAlpha ? 1 : 2);
            
            return best;
        }
    }

    template<class status status, class C>
    struct EnumerateMoves {
        __attribute__((noinline)) static long long e(const board& brd, int depth){
            ::end = chrono::high_resolution_clock::now();
            ull e = chrono::duration_cast<chrono::milliseconds>(::end - ::start).count();
            if(e > searchtime - 2000) throw 0;
            C::enodes++;
            ull checkmask = mGen::checks[depth];
            ull kingban = mGen::myking[depth - 1] = mGen::enking[depth];
            ull kingatk = refresh<status>(brd, kingban, checkmask, depth);
            constexpr bool white = status.wMove;
            
            if(C::fullmove - C::halfmove > 100) return 0;
            if (checkmask != 0) return _enumerate<status, C>(brd, kingatk, kingban, checkmask, depth);
            else {
                if (depth == 25) {
                    C::enodes--;
                    return QuiesceMoves<status, C>::e(brd, 25);
                } else {
                    ull z = Lookup::zobrist<status>(brd, epSquare), oldAlpha = C::alpha[depth];
                    ttEntry& tt = TT[z & 0xFFFFFF];
                    const bool ttHit = tt.hash == z;

                    for(int i=C::fullmove-1;i>C::halfmove;i--){
                        if (C::reps[i] == z) return 0;
                    }

                    if(ttHit){
                        if(tt.depth >= depth && (
                            (tt.bound == 0 && tt.score >= C::beta[depth]) ||
                            (tt.bound == 1 && tt.score <= C::alpha[depth]) ||
                            (tt.bound == 2 && C::alpha[depth] + 1 == C::beta[depth])
                        )) {C::skips++; return tt.score;}
                    }
                    
                    C::reps[C::fullmove] = z;

                    long long eval, best = -2147483647ll - depth;
                    ::move moves[8], bestmove;

                    int cnt = 0;
                    const ull king = King<white>(brd);
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Queens<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Rooks<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Bishops<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Knights<!white>(brd));
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, Pawns<!white>(brd));

                    int captures = cnt;
                    _kingmoves<white>(C::template Kingmove<status, QuiesceMoves>, moves, cnt, king, kingatk, 0xffffffffffffffffull);
                    sort(moves + captures, moves + cnt, greater());

                    if (ttHit) for(int i=0;i<cnt;i++){
                        if (tt.m == moves[i]) {
                            moves[i] = moves[0];
                            moves[0] = tt.m;
                        }
                    }
                    
                    long long beta = C::beta[depth];

                    for(int i=0;i<cnt;i++){
                        if(i != 0 /*&& C::beta[depth] != C::alpha[depth] + 1*/) {
                            C::beta[depth] = C::alpha[depth] + 1;
                            eval = -moves[i](brd, depth);
                            if(eval > C::alpha[depth]) {
                                C::beta[depth] = beta;
                                eval = -moves[i](brd, depth);
                            }
                        } else eval = -moves[i](brd, depth);
                        if(eval > best){
                            best = eval;
                            C::alpha[depth] = max(eval, C::alpha[depth]);
                            bestmove = moves[i];
                            pvtable[depth][depth] = bestmove.ft & 0b11111111111111111;
                            for(int j=depth-1;j>-1;j--) pvtable[depth][j] = pvtable[depth-1][j];
                            if(eval >= C::beta[depth]){
                                if(i >= captures) butterfly[white][square(king)][moves[i].ft & 0b111111] += depth * depth - butterfly[status.wMove][square(king)][moves[i].ft & 0b111111] * depth * depth / 16384;
                                C::cuts++;
                                break;
                            }
                        }
                    }

                    tt.hash = z;
                    tt.m = C::alpha[depth] > oldAlpha ? bestmove : tt.m;
                    tt.score = best;
                    tt.depth = depth;
                    tt.bound = best >= C::beta[depth] ? 0 : (C::alpha[depth] == oldAlpha ? 1 : 2);
                    
                    return best;
                }
            }
        }
    };
}