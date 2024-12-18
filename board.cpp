class status {
    static constexpr ull wNotOccupiedL = 0xEull;
    static constexpr ull wNotAttackedL = 0x1Cull;
    static constexpr ull wNotOccupiedR = 0x60ull;
    static constexpr ull wNotAttackedR = 0x70ull;

    static constexpr ull bNotOccupiedL = 0xEull << 56;
    static constexpr ull bNotAttackedL = 0x1Cull << 56;
    static constexpr ull bNotOccupiedR = 0x60ull << 56;
    static constexpr ull bNotAttackedR = 0x70ull << 56;

    static constexpr ull wRookL = 0x1ull;
    static constexpr ull bRookL = 0x1ull << 56;
    static constexpr ull wRookR = 0x80ull;
    static constexpr ull bRookR = 0x80ull << 56;

  public:
    const bool wMove;
    const bool hasEP;

    const bool wCastleL;
    const bool wCastleR;
    const bool bCastleL;
    const bool bCastleR;

    constexpr status(bool w, bool ep, bool wcl, bool wcr, bool bcl, bool bcr) :
        wMove(w), hasEP(ep), wCastleL(wcl), wCastleR(wcr), bCastleL(bcl), bCastleR(bcr){}

    constexpr status(int pat) :
        wMove((pat & 0b100000) != 0), hasEP((pat & 0b010000) != 0), 
        wCastleL((pat & 0b001000) != 0), wCastleR((pat & 0b000100) != 0), 
        bCastleL((pat & 0b000010) != 0), bCastleR((pat & 0b000001) != 0){}

    // constexpr status(const status&& s) : status(s.wMove, s.hasEP, s.wCastleL, s.wCastleR, s.bCastleL, s.bCastleR){}

    constexpr bool canCastle() const {
        if (wMove) return wCastleL | wCastleR;
        else return bCastleL | bCastleR;
    }

    constexpr bool canCastleLeft() const {
        if (wMove) return wCastleL;
        else return bCastleL;
    }

    constexpr bool canCastleRight() const {
        if (wMove) return wCastleR;
        else return bCastleR;
    }

    constexpr ull rookswitchR() const {
        if (wMove) return 0b10100000ull;
        else return 0b10100000ull << 56;
    }

    constexpr ull rookswitchL() const {
        if (wMove) return 0b00001001ull;
        else return 0b00001001ull << 56;
    }

    inline constexpr bool canCastleLeft(ull attacked, ull occupied, ull rook) const {
        if (wMove && wCastleL){
            if (occupied & wNotOccupiedL) return false;
            if (attacked & wNotAttackedL) return false;
            if (rook & wRookL) return true; 
            return false;
        } else if (bCastleL) {
            if (occupied & bNotOccupiedL) return false;
            if (attacked & bNotAttackedL) return false;
            if (rook & bRookL) return true;
            return false;
        }
        return false;
    }

    inline constexpr bool canCastleRight(ull attacked, ull occupied, ull rook) const {
        if (wMove && wCastleR){
            if (occupied & wNotOccupiedR) return false;
            if (attacked & wNotAttackedR) return false;
            if (rook & wRookR) return true;
            return false;
        } else if (bCastleR) {
            if (occupied & bNotOccupiedR) return false;
            if (attacked & bNotAttackedR) return false; 
            if (rook & bRookR) return true;
            return false;
        }
        return false;
    }

    constexpr bool isLeftRook(ull rook) const {
        if (wMove) return wRookL == rook;
        else return bRookL == rook;
    }
    constexpr bool isRightRook(ull rook) const {
        if (wMove) return wRookR == rook;
        else return bRookR == rook;
    }

    constexpr status pawnPush() const {
        return status(!wMove, true, wCastleL, wCastleR, bCastleL, bCastleR);
    }

    constexpr status kingMove() const {
        if (wMove) return status(!wMove, false, false, false, bCastleL, bCastleR);
        else return status(!wMove, false, wCastleL, wCastleR, false, false);
    }

    constexpr status rookMoveL() const {
        if (wMove) return status(!wMove, false, false, wCastleR, bCastleL, bCastleR);
        else return status(!wMove, false, wCastleL, wCastleR, false, bCastleR);
    }

    constexpr status rookMoveR() const {
        if (wMove) return status(!wMove, false, wCastleL, false, bCastleL, bCastleR);
        else return status(!wMove, false, wCastleL, wCastleR, bCastleL, false);
    }

    constexpr status silentMove() const {
        return status(!wMove, false, wCastleL, wCastleR, bCastleL, bCastleR);
    }

    static constexpr status startpos() {
        return status(true, false, true, true, true, true);
    }
};

enum class boardPiece {
    pawn,
    knight,
    bishop,
    rook,
    queen,
    king,
    empty
};

class board {
    int fen_helper_helper(char p){
        if(p == 'P') return 0;
        if(p == 'N') return 1;
        if(p == 'B') return 2;
        if(p == 'R') return 3;
        if(p == 'Q') return 4;
        if(p == 'K') return 5;
        if(p == 'p') return 6;
        if(p == 'n') return 7;
        if(p == 'b') return 8;
        if(p == 'r') return 9;
        if(p == 'q') return 10;
        if(p == 'k') return 11;
        return -1;
    }

    ull fen_helper(string fen, int b){
        string s;
        stringstream ss(fen);
        ull bbs[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
        
        for (int i=0;i<12;i++) bbs[i] = 0;

        ss >> s;
        int r = 7, f = 0, i = 0;
        for(;i<s.length();i++){
            if(s[i] >= '1' && s[i] <= '8'){
                f += s[i] - '0';
            } else if (s[i] == '/') {
                r--;
                f = 0;
            } else {
                bbs[fen_helper_helper(s[i])] |= 1ull << (r * 8 + f);
                f++;
            }
        }
        return bbs[b];
    }

  public:
    const ull wPawns;
    const ull wKnights;
    const ull wBishops;
    const ull wRooks;
    const ull wQueens;
    const ull wKings;
    const ull bPawns;
    const ull bKnights;
    const ull bBishops;
    const ull bRooks;
    const ull bQueens;
    const ull bKings;
    const ull whites;
    const ull blacks;
    const ull occupied;

    int getPiece(int rank, int file) const {
        ull i = (1ull << (rank * 8 + file));
        if(whites & i){
            if(wPawns & i) return 0;
            if(wKnights & i) return 2;
            if(wBishops & i) return 4;
            if(wRooks & i) return 6;
            if(wQueens & i) return 8;
            if(wKings & i) return 10;
        } else if(blacks & i){
            if(bPawns & i) return 1;
            if(bKnights & i) return 3;
            if(bBishops & i) return 5;
            if(bRooks & i) return 7;
            if(bQueens & i) return 9;
            if(bKings & i) return 11;
        }
        return 12;
    }

    boardPiece getPiece(ull i) const {
        if(whites & i){
            if(wPawns & i) return boardPiece::pawn;
            if(wKnights & i) return boardPiece::knight;
            if(wBishops & i) return boardPiece::bishop;
            if(wRooks & i) return boardPiece::rook;
            if(wQueens & i) return boardPiece::queen;
            if(wKings & i) return boardPiece::king;
        } else if(blacks & i){
            if(bPawns & i) return boardPiece::pawn;
            if(bKnights & i) return boardPiece::knight;
            if(bBishops & i) return boardPiece::bishop;
            if(bRooks & i) return boardPiece::rook;
            if(bQueens & i) return boardPiece::queen;
            if(bKings & i) return boardPiece::king;
        }
        return boardPiece::empty;
    }

    int getMaterial(ull i) const {
        if(wPawns & i) return 1;
        if(wKnights & i) return 3;
        if(wBishops & i) return 3;
        if(wRooks & i) return 5;
        if(wQueens & i) return 9;
        if(wKings & i) return 0;
        if(bPawns & i) return -1;
        if(bKnights & i) return -3;
        if(bBishops & i) return -3;
        if(bRooks & i) return -5;
        if(bQueens & i) return -9;
        if(bKings & i) return 0;
        return 0;
    }

    constexpr board(
        ull bp, ull bn, ull bb, ull br, ull bq, ull bk,
        ull wp, ull wn, ull wb, ull wr, ull wq, ull wk) :
        bPawns(bp), bKnights(bn), bBishops(bb), bRooks(br), bQueens(bq), bKings(bk),
        wPawns(wp), wKnights(wn), wBishops(wb), wRooks(wr), wQueens(wq), wKings(wk),
        blacks(bp | bn | bb | br | bq | bk),
        whites(wp | wn | wb | wr | wq | wk),
        occupied(blacks | whites){}

    board(const string& fen) : board(
        fen_helper(fen, 6), fen_helper(fen, 7), fen_helper(fen, 8), fen_helper(fen, 9), fen_helper(fen, 10), fen_helper(fen, 11), 
        fen_helper(fen, 0), fen_helper(fen, 1), fen_helper(fen, 2), fen_helper(fen, 3), fen_helper(fen, 4), fen_helper(fen, 5)){}

    // board(const board&& b) : board(b.bPawns, b.bKnights, b.bBishops, b.bRooks, b.bQueens, b.bKings,
    //     b.wPawns, b.wKnights, b.wBishops, b.wRooks, b.wQueens, b.wKings){}

    void print() const {
        constexpr char p[13] = {'P', 'p', 'N', 'n', 'B', 'b', 'R', 'r', 'Q', 'q', 'K', 'k', ' '};
        cout << "+---+---+---+---+---+---+---+---+\n";
        for(int r=7;r>-1;r--){
            for(int f=0;f<8;f++){
                cout << "| " << p[getPiece(r, f)] << " ";
            } cout << "|\n";
            cout << "+---+---+---+---+---+---+---+---+\n";
        }
    }

    template<boardPiece piece, bool white>
    __attribute__((always_inline)) static constexpr board movePromote(const board& existing, ull from, ull to){
        const ull rem = ~to;
        const ull bp = existing.bPawns;
        const ull bn = existing.bKnights;
        const ull bb = existing.bBishops;
        const ull br = existing.bRooks;
        const ull bq = existing.bQueens;
        const ull bk = existing.bKings;

        const ull wp = existing.wPawns;
        const ull wn = existing.wKnights;
        const ull wb = existing.wBishops;
        const ull wr = existing.wRooks;
        const ull wq = existing.wQueens;
        const ull wk = existing.wKings;

        if constexpr (white) {
            if constexpr (boardPiece::queen == piece)  return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ from, wn, wb, wr, wq ^ to, wk);
            if constexpr (boardPiece::rook == piece)   return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ from, wn, wb, wr ^ to, wq, wk);
            if constexpr (boardPiece::bishop == piece) return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ from, wn, wb ^ to, wr, wq, wk);
            if constexpr (boardPiece::knight == piece) return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ from, wn ^ to, wb, wr, wq, wk);
        } else {
            if constexpr (boardPiece::queen == piece)  return board(bp ^ from, bn, bb, br, bq ^ to, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
            if constexpr (boardPiece::rook == piece)   return board(bp ^ from, bn, bb, br ^ to, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
            if constexpr (boardPiece::bishop == piece) return board(bp ^ from, bn, bb ^ to, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
            if constexpr (boardPiece::knight == piece) return board(bp ^ from, bn ^ to, bb, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
        }
    }

    template<bool white>
    __attribute__((always_inline)) static constexpr board moveCastle(const board& existing, ull kingswitch, ull rookswitch){
        const ull bp = existing.bPawns;
        const ull bn = existing.bKnights;
        const ull bb = existing.bBishops;
        const ull br = existing.bRooks;
        const ull bq = existing.bQueens;
        const ull bk = existing.bKings;

        const ull wp = existing.wPawns;
        const ull wn = existing.wKnights;
        const ull wb = existing.wBishops;
        const ull wr = existing.wRooks;
        const ull wq = existing.wQueens;
        const ull wk = existing.wKings;

        if constexpr (white) return board(bp, bn, bb, br, bq, bk, wp, wn, wb, wr ^ rookswitch, wq, wk ^ kingswitch);
        else return board(bp, bn, bb, br ^ rookswitch, bq, bk ^ kingswitch, wp, wn, wb, wr, wq, wk);
    }

    template<bool white>
    __attribute__((always_inline)) static constexpr board moveEP(const board& existing, ull from, ull to){
        const ull bp = existing.bPawns;
        const ull bn = existing.bKnights;
        const ull bb = existing.bBishops;
        const ull br = existing.bRooks;
        const ull bq = existing.bQueens;
        const ull bk = existing.bKings;

        const ull wp = existing.wPawns;
        const ull wn = existing.wKnights;
        const ull wb = existing.wBishops;
        const ull wr = existing.wRooks;
        const ull wq = existing.wQueens;
        const ull wk = existing.wKings;
        const ull mov = from | to;
        
        if constexpr (white) {
            const ull rem = ~(to >> 8);
            return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ mov, wn, wb, wr, wq, wk);
        } else {
            const ull rem = ~(to << 8);
            return board(bp ^ mov, bn, bb, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
        }
    }

    template<boardPiece piece, bool white>
    __attribute__((always_inline)) static constexpr board move(const board& existing, ull from, ull to, bool taking){
        if (taking) return move<piece, white, true>(existing, from, to);
        else return move<piece, white, false>(existing, from, to);
    }

    template<boardPiece piece, bool white, bool taking>
    __attribute__((always_inline)) static constexpr board move(const board& existing, ull from, ull to){
        const ull bp = existing.bPawns;
        const ull bn = existing.bKnights;
        const ull bb = existing.bBishops;
        const ull br = existing.bRooks;
        const ull bq = existing.bQueens;
        const ull bk = existing.bKings;

        const ull wp = existing.wPawns;
        const ull wn = existing.wKnights;
        const ull wb = existing.wBishops;
        const ull wr = existing.wRooks;
        const ull wq = existing.wQueens;
        const ull wk = existing.wKings;
        const ull mov = from | to;
        
        if constexpr (taking) {
            const ull rem = ~to;
            if constexpr (white) {
                if constexpr (boardPiece::pawn == piece)    return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp ^ mov, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::knight == piece)  return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp, wn ^ mov, wb, wr, wq, wk);
                if constexpr (boardPiece::bishop == piece)  return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp, wn, wb ^ mov, wr, wq, wk);
                if constexpr (boardPiece::rook == piece)    return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp, wn, wb, wr ^ mov, wq, wk);
                if constexpr (boardPiece::queen == piece)   return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp, wn, wb, wr, wq ^ mov, wk);
                if constexpr (boardPiece::king == piece)    return board(bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, wp, wn, wb, wr, wq, wk ^ mov);
            }
            else {
                if constexpr (boardPiece::pawn == piece)    return board(bp ^ mov, bn, bb, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
                if constexpr (boardPiece::knight == piece)  return board(bp, bn ^ mov, bb, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
                if constexpr (boardPiece::bishop == piece)  return board(bp, bn, bb ^ mov, br, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
                if constexpr (boardPiece::rook == piece)    return board(bp, bn, bb, br ^ mov, bq, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
                if constexpr (boardPiece::queen == piece)   return board(bp, bn, bb, br, bq ^ mov, bk, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
                if constexpr (boardPiece::king == piece)    return board(bp, bn, bb, br, bq, bk ^ mov, wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk);
            }
        } else {
            if constexpr (white) {
                if constexpr (boardPiece::pawn == piece)    return board(bp, bn, bb, br, bq, bk, wp ^ mov, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::knight == piece)  return board(bp, bn, bb, br, bq, bk, wp, wn ^ mov, wb, wr, wq, wk);
                if constexpr (boardPiece::bishop == piece)  return board(bp, bn, bb, br, bq, bk, wp, wn, wb ^ mov, wr, wq, wk);
                if constexpr (boardPiece::rook == piece)    return board(bp, bn, bb, br, bq, bk, wp, wn, wb, wr ^ mov, wq, wk);
                if constexpr (boardPiece::queen == piece)   return board(bp, bn, bb, br, bq, bk, wp, wn, wb, wr, wq ^ mov, wk);
                if constexpr (boardPiece::king == piece)    return board(bp, bn, bb, br, bq, bk, wp, wn, wb, wr, wq, wk ^ mov);
            } else {
                if constexpr (boardPiece::pawn == piece)    return board(bp ^ mov, bn, bb, br, bq, bk, wp, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::knight == piece)  return board(bp, bn ^ mov, bb, br, bq, bk, wp, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::bishop == piece)  return board(bp, bn, bb ^ mov, br, bq, bk, wp, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::rook == piece)    return board(bp, bn, bb, br ^ mov, bq, bk, wp, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::queen == piece)   return board(bp, bn, bb, br, bq ^ mov, bk, wp, wn, wb, wr, wq, wk);
                if constexpr (boardPiece::king == piece)    return board(bp, bn, bb, br, bq, bk ^ mov, wp, wn, wb, wr, wq, wk);
            }
        }
        
    }

    static constexpr board startpos() {
        return board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
};

bool FenInfo(int x, string pos){
    stringstream ss(pos);
    bool b[6] = {0,0,0,0,0,0};
    ss >> pos >> pos;
    b[0] = pos == "w";
    ss >> pos;
    for(char c: pos){
        b[2] |= c == 'q';
        b[3] |= c == 'k';
        b[4] |= c == 'Q';
        b[5] |= c == 'K';
    }
    ss >> pos;
    b[1] = pos != "-";
    return b[x];
}

ull FenEP(string s){
    stringstream ss(s);
    ss >> s >> s >> s >> s;
    if (s == "-") return 0ull;
    if(s[1] == '3') return 1ull << (s[0] - 'a' + 24);
    if(s[1] == '6') return 1ull << (s[0] - 'a' + 32);
    return 0ull;
}

#define PositionToTemplate(func) \
static inline void _##func(string pos, int depth) { \
const bool WH = FenInfo(0, pos);\
const bool EP = FenInfo(1, pos);\
const bool BL = FenInfo(2, pos);\
const bool BR = FenInfo(3, pos);\
const bool WL = FenInfo(4, pos);\
const bool WR = FenInfo(5, pos);\
board brd(pos);\
if ( WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b111111)>(pos, brd, depth); \
if ( WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b111110)>(pos, brd, depth); \
if ( WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b111101)>(pos, brd, depth); \
if ( WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b111100)>(pos, brd, depth); \
if ( WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b111011)>(pos, brd, depth); \
if ( WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b111010)>(pos, brd, depth); \
if ( WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b111001)>(pos, brd, depth); \
if ( WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b111000)>(pos, brd, depth); \
if ( WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b110111)>(pos, brd, depth); \
if ( WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b110110)>(pos, brd, depth); \
if ( WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b110101)>(pos, brd, depth); \
if ( WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b110100)>(pos, brd, depth); \
if ( WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b110011)>(pos, brd, depth); \
if ( WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b110010)>(pos, brd, depth); \
if ( WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b110001)>(pos, brd, depth); \
if ( WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b110000)>(pos, brd, depth); \
if ( WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b101111)>(pos, brd, depth); \
if ( WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b101110)>(pos, brd, depth); \
if ( WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b101101)>(pos, brd, depth); \
if ( WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b101100)>(pos, brd, depth); \
if ( WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b101011)>(pos, brd, depth); \
if ( WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b101010)>(pos, brd, depth); \
if ( WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b101001)>(pos, brd, depth); \
if ( WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b101000)>(pos, brd, depth); \
if ( WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b100111)>(pos, brd, depth); \
if ( WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b100110)>(pos, brd, depth); \
if ( WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b100101)>(pos, brd, depth); \
if ( WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b100100)>(pos, brd, depth); \
if ( WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b100011)>(pos, brd, depth); \
if ( WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b100010)>(pos, brd, depth); \
if ( WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b100001)>(pos, brd, depth); \
if ( WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b100000)>(pos, brd, depth); \
if (!WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b011111)>(pos, brd, depth); \
if (!WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b011110)>(pos, brd, depth); \
if (!WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b011101)>(pos, brd, depth); \
if (!WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b011100)>(pos, brd, depth); \
if (!WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b011011)>(pos, brd, depth); \
if (!WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b011010)>(pos, brd, depth); \
if (!WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b011001)>(pos, brd, depth); \
if (!WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b011000)>(pos, brd, depth); \
if (!WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b010111)>(pos, brd, depth); \
if (!WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b010110)>(pos, brd, depth); \
if (!WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b010101)>(pos, brd, depth); \
if (!WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b010100)>(pos, brd, depth); \
if (!WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b010011)>(pos, brd, depth); \
if (!WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b010010)>(pos, brd, depth); \
if (!WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b010001)>(pos, brd, depth); \
if (!WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b010000)>(pos, brd, depth); \
if (!WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b001111)>(pos, brd, depth); \
if (!WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b001110)>(pos, brd, depth); \
if (!WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b001101)>(pos, brd, depth); \
if (!WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b001100)>(pos, brd, depth); \
if (!WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b001011)>(pos, brd, depth); \
if (!WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b001010)>(pos, brd, depth); \
if (!WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b001001)>(pos, brd, depth); \
if (!WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b001000)>(pos, brd, depth); \
if (!WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b000111)>(pos, brd, depth); \
if (!WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b000110)>(pos, brd, depth); \
if (!WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b000101)>(pos, brd, depth); \
if (!WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b000100)>(pos, brd, depth); \
if (!WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b000011)>(pos, brd, depth); \
if (!WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b000010)>(pos, brd, depth); \
if (!WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b000001)>(pos, brd, depth); \
if (!WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b000000)>(pos, brd, depth); \
return func<status::startpos()>(pos, brd, depth);}

#define StatusToTemplate(func) \
inline void _##func(status s, board& brd, int depth) { \
const bool WH = s.wMove;\
const bool EP = s.hasEP;\
const bool BL = s.bCastleL;\
const bool BR = s.bCastleR;\
const bool WL = s.wCastleL;\
const bool WR = s.wCastleR;\
if ( WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b111111)>(brd, depth); \
if ( WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b111110)>(brd, depth); \
if ( WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b111101)>(brd, depth); \
if ( WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b111100)>(brd, depth); \
if ( WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b111011)>(brd, depth); \
if ( WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b111010)>(brd, depth); \
if ( WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b111001)>(brd, depth); \
if ( WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b111000)>(brd, depth); \
if ( WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b110111)>(brd, depth); \
if ( WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b110110)>(brd, depth); \
if ( WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b110101)>(brd, depth); \
if ( WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b110100)>(brd, depth); \
if ( WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b110011)>(brd, depth); \
if ( WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b110010)>(brd, depth); \
if ( WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b110001)>(brd, depth); \
if ( WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b110000)>(brd, depth); \
if ( WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b101111)>(brd, depth); \
if ( WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b101110)>(brd, depth); \
if ( WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b101101)>(brd, depth); \
if ( WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b101100)>(brd, depth); \
if ( WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b101011)>(brd, depth); \
if ( WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b101010)>(brd, depth); \
if ( WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b101001)>(brd, depth); \
if ( WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b101000)>(brd, depth); \
if ( WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b100111)>(brd, depth); \
if ( WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b100110)>(brd, depth); \
if ( WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b100101)>(brd, depth); \
if ( WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b100100)>(brd, depth); \
if ( WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b100011)>(brd, depth); \
if ( WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b100010)>(brd, depth); \
if ( WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b100001)>(brd, depth); \
if ( WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b100000)>(brd, depth); \
if (!WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b011111)>(brd, depth); \
if (!WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b011110)>(brd, depth); \
if (!WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b011101)>(brd, depth); \
if (!WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b011100)>(brd, depth); \
if (!WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b011011)>(brd, depth); \
if (!WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b011010)>(brd, depth); \
if (!WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b011001)>(brd, depth); \
if (!WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b011000)>(brd, depth); \
if (!WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b010111)>(brd, depth); \
if (!WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b010110)>(brd, depth); \
if (!WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b010101)>(brd, depth); \
if (!WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b010100)>(brd, depth); \
if (!WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b010011)>(brd, depth); \
if (!WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b010010)>(brd, depth); \
if (!WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b010001)>(brd, depth); \
if (!WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b010000)>(brd, depth); \
if (!WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b001111)>(brd, depth); \
if (!WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b001110)>(brd, depth); \
if (!WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b001101)>(brd, depth); \
if (!WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b001100)>(brd, depth); \
if (!WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b001011)>(brd, depth); \
if (!WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b001010)>(brd, depth); \
if (!WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b001001)>(brd, depth); \
if (!WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b001000)>(brd, depth); \
if (!WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b000111)>(brd, depth); \
if (!WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b000110)>(brd, depth); \
if (!WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b000101)>(brd, depth); \
if (!WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b000100)>(brd, depth); \
if (!WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b000011)>(brd, depth); \
if (!WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b000010)>(brd, depth); \
if (!WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b000001)>(brd, depth); \
if (!WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b000000)>(brd, depth); \
return func<status::startpos()>(brd, depth);}

#define EvalToTemplate(func) \
template <bool e=false> \
static inline long long _##func(status s, const board& brd) { \
const bool WH = s.wMove;\
const bool EP = s.hasEP;\
const bool BL = s.bCastleL;\
const bool BR = s.bCastleR;\
const bool WL = s.wCastleL;\
const bool WR = s.wCastleR;\
if ( WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b111111), e>(brd); \
if ( WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b111110), e>(brd); \
if ( WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b111101), e>(brd); \
if ( WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b111100), e>(brd); \
if ( WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b111011), e>(brd); \
if ( WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b111010), e>(brd); \
if ( WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b111001), e>(brd); \
if ( WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b111000), e>(brd); \
if ( WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b110111), e>(brd); \
if ( WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b110110), e>(brd); \
if ( WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b110101), e>(brd); \
if ( WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b110100), e>(brd); \
if ( WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b110011), e>(brd); \
if ( WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b110010), e>(brd); \
if ( WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b110001), e>(brd); \
if ( WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b110000), e>(brd); \
if ( WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b101111), e>(brd); \
if ( WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b101110), e>(brd); \
if ( WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b101101), e>(brd); \
if ( WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b101100), e>(brd); \
if ( WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b101011), e>(brd); \
if ( WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b101010), e>(brd); \
if ( WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b101001), e>(brd); \
if ( WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b101000), e>(brd); \
if ( WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b100111), e>(brd); \
if ( WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b100110), e>(brd); \
if ( WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b100101), e>(brd); \
if ( WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b100100), e>(brd); \
if ( WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b100011), e>(brd); \
if ( WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b100010), e>(brd); \
if ( WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b100001), e>(brd); \
if ( WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b100000), e>(brd); \
if (!WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b011111), e>(brd); \
if (!WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b011110), e>(brd); \
if (!WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b011101), e>(brd); \
if (!WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b011100), e>(brd); \
if (!WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b011011), e>(brd); \
if (!WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b011010), e>(brd); \
if (!WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b011001), e>(brd); \
if (!WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b011000), e>(brd); \
if (!WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b010111), e>(brd); \
if (!WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b010110), e>(brd); \
if (!WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b010101), e>(brd); \
if (!WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b010100), e>(brd); \
if (!WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b010011), e>(brd); \
if (!WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b010010), e>(brd); \
if (!WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b010001), e>(brd); \
if (!WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b010000), e>(brd); \
if (!WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b001111), e>(brd); \
if (!WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b001110), e>(brd); \
if (!WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b001101), e>(brd); \
if (!WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b001100), e>(brd); \
if (!WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b001011), e>(brd); \
if (!WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b001010), e>(brd); \
if (!WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b001001), e>(brd); \
if (!WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b001000), e>(brd); \
if (!WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b000111), e>(brd); \
if (!WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b000110), e>(brd); \
if (!WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b000101), e>(brd); \
if (!WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b000100), e>(brd); \
if (!WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b000011), e>(brd); \
if (!WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b000010), e>(brd); \
if (!WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b000001), e>(brd); \
if (!WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b000000), e>(brd); \
return func<status::startpos(), e>(brd);}

#define BookToTemplate(func) \
inline string _##func(status s, const board& brd) { \
const bool WH = s.wMove;\
const bool EP = s.hasEP;\
const bool BL = s.bCastleL;\
const bool BR = s.bCastleR;\
const bool WL = s.wCastleL;\
const bool WR = s.wCastleR;\
if ( WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b111111)>(brd); \
if ( WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b111110)>(brd); \
if ( WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b111101)>(brd); \
if ( WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b111100)>(brd); \
if ( WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b111011)>(brd); \
if ( WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b111010)>(brd); \
if ( WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b111001)>(brd); \
if ( WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b111000)>(brd); \
if ( WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b110111)>(brd); \
if ( WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b110110)>(brd); \
if ( WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b110101)>(brd); \
if ( WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b110100)>(brd); \
if ( WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b110011)>(brd); \
if ( WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b110010)>(brd); \
if ( WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b110001)>(brd); \
if ( WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b110000)>(brd); \
if ( WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b101111)>(brd); \
if ( WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b101110)>(brd); \
if ( WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b101101)>(brd); \
if ( WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b101100)>(brd); \
if ( WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b101011)>(brd); \
if ( WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b101010)>(brd); \
if ( WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b101001)>(brd); \
if ( WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b101000)>(brd); \
if ( WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b100111)>(brd); \
if ( WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b100110)>(brd); \
if ( WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b100101)>(brd); \
if ( WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b100100)>(brd); \
if ( WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b100011)>(brd); \
if ( WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b100010)>(brd); \
if ( WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b100001)>(brd); \
if ( WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b100000)>(brd); \
if (!WH &&  EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b011111)>(brd); \
if (!WH &&  EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b011110)>(brd); \
if (!WH &&  EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b011101)>(brd); \
if (!WH &&  EP &&  WL &&  WR && !BL && !BR)       return func<status(0b011100)>(brd); \
if (!WH &&  EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b011011)>(brd); \
if (!WH &&  EP &&  WL && !WR &&  BL && !BR)       return func<status(0b011010)>(brd); \
if (!WH &&  EP &&  WL && !WR && !BL &&  BR)       return func<status(0b011001)>(brd); \
if (!WH &&  EP &&  WL && !WR && !BL && !BR)       return func<status(0b011000)>(brd); \
if (!WH &&  EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b010111)>(brd); \
if (!WH &&  EP && !WL &&  WR &&  BL && !BR)       return func<status(0b010110)>(brd); \
if (!WH &&  EP && !WL &&  WR && !BL &&  BR)       return func<status(0b010101)>(brd); \
if (!WH &&  EP && !WL &&  WR && !BL && !BR)       return func<status(0b010100)>(brd); \
if (!WH &&  EP && !WL && !WR &&  BL &&  BR)       return func<status(0b010011)>(brd); \
if (!WH &&  EP && !WL && !WR &&  BL && !BR)       return func<status(0b010010)>(brd); \
if (!WH &&  EP && !WL && !WR && !BL &&  BR)       return func<status(0b010001)>(brd); \
if (!WH &&  EP && !WL && !WR && !BL && !BR)       return func<status(0b010000)>(brd); \
if (!WH && !EP &&  WL &&  WR &&  BL &&  BR)       return func<status(0b001111)>(brd); \
if (!WH && !EP &&  WL &&  WR &&  BL && !BR)       return func<status(0b001110)>(brd); \
if (!WH && !EP &&  WL &&  WR && !BL &&  BR)       return func<status(0b001101)>(brd); \
if (!WH && !EP &&  WL &&  WR && !BL && !BR)       return func<status(0b001100)>(brd); \
if (!WH && !EP &&  WL && !WR &&  BL &&  BR)       return func<status(0b001011)>(brd); \
if (!WH && !EP &&  WL && !WR &&  BL && !BR)       return func<status(0b001010)>(brd); \
if (!WH && !EP &&  WL && !WR && !BL &&  BR)       return func<status(0b001001)>(brd); \
if (!WH && !EP &&  WL && !WR && !BL && !BR)       return func<status(0b001000)>(brd); \
if (!WH && !EP && !WL &&  WR &&  BL &&  BR)       return func<status(0b000111)>(brd); \
if (!WH && !EP && !WL &&  WR &&  BL && !BR)       return func<status(0b000110)>(brd); \
if (!WH && !EP && !WL &&  WR && !BL &&  BR)       return func<status(0b000101)>(brd); \
if (!WH && !EP && !WL &&  WR && !BL && !BR)       return func<status(0b000100)>(brd); \
if (!WH && !EP && !WL && !WR &&  BL &&  BR)       return func<status(0b000011)>(brd); \
if (!WH && !EP && !WL && !WR &&  BL && !BR)       return func<status(0b000010)>(brd); \
if (!WH && !EP && !WL && !WR && !BL &&  BR)       return func<status(0b000001)>(brd); \
if (!WH && !EP && !WL && !WR && !BL && !BR)       return func<status(0b000000)>(brd); \
return func<status::startpos()>(brd);}

//beep boop