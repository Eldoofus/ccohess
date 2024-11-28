#include<bits/stdc++.h>
#include<x86intrin.h>
#include"map.cpp"
using namespace std;

typedef unsigned long long ull;

void printbb(ull p){
    cout << "+---+---+---+---+---+---+---+---+\n";
    for(int r=7;r>-1;r--){
        for(int f=0;f<8;f++){
            cout << "| " << ((p & (1ull << (r * 8 + f))) ? "X" : " ") << " ";
        } cout << "|\n";
        cout << "+---+---+---+---+---+---+---+---+\n";
    }
}

class board {
  public:
    ull bbs[14]; // a1 to f8
    bool side; // 0: White, 1: Black
    char castle; // KQkq
    ull enpassant; // rank, file
    int halfmove, fullmove;

    enum piecetype {
        nPawn,
        nKnight,
        nBishop,
        nRook,
        nQueen,
        nKing,
        nEmpty
    };

    inline ull& wPawns(){ return bbs[nPawn * 2]; }
    inline ull& wKnights(){ return bbs[nKnight * 2]; }
    inline ull& wBishops(){ return bbs[nBishop * 2]; }
    inline ull& wRooks(){ return bbs[nRook * 2]; }
    inline ull& wQueens(){ return bbs[nQueen * 2]; }
    inline ull& wKings(){ return bbs[nKing * 2]; }
    inline ull& bPawns(){ return bbs[nPawn * 2 + 1]; }
    inline ull& bKnights(){ return bbs[nKnight * 2 + 1]; }
    inline ull& bBishops(){ return bbs[nBishop * 2 + 1]; }
    inline ull& bRooks(){ return bbs[nRook * 2 + 1]; }
    inline ull& bQueens(){ return bbs[nQueen * 2 + 1]; }
    inline ull& bKings(){ return bbs[nKing * 2 + 1]; }
    inline ull& wPieces(){ return bbs[12]; }
    inline ull& bPieces(){ return bbs[13]; }

    int getPiece(int rank, int file){
        ull i = (1ull << (rank * 8 + file));
        if(wPieces() & i){
            if(wPawns() & i) return 0;
            if(wKnights() & i) return 2;
            if(wBishops() & i) return 4;
            if(wRooks() & i) return 6;
            if(wQueens() & i) return 8;
            if(wKings() & i) return 10;
        } else if(bPieces() & i){
            if(bPawns() & i) return 1;
            if(bKnights() & i) return 3;
            if(bBishops() & i) return 5;
            if(bRooks() & i) return 7;
            if(bQueens() & i) return 9;
            if(bKings() & i) return 11;
        }
        return 12;
    }

    board(string fen){
        string s;
        stringstream ss(fen);
        
        for (int i=0;i<14;i++) bbs[i] = 0;

        ss >> s; // position data -> bitboards
        int r = 7, f = 0, i = 0;
        for(;i<s.length();i++){
            //cout << s[i];
            if(s[i] >= '1' && s[i] <= '8'){
                f += s[i] - '0';
            } else if (s[i] == '/') {
                r--;
                f = 0;
            } else {
                bbs[fen_helper(s[i])] |= 1ull << (r * 8 + f);
                if(s[i] >= 'a') bbs[13] |= 1ull << (r * 8 + f);
                else bbs[12] |= 1ull << (r * 8 + f);
                f++;
            }
        }
        //cout << endl;

        ss >> s; // side to play
        side = s[0] == 'b';

        ss >> s; // castling rights
        castle = 0;
        for(i=0;i<s.length();i++){
            if(s[i] == 'K') castle |= 0b1000;
            if(s[i] == 'Q') castle |= 0b100;
            if(s[i] == 'k') castle |= 0b10;
            if(s[i] == 'q') castle |= 0b1;
        }

        ss >> s; // en passant
        enpassant = 1ull << ((s[1] - '0') * 8 + (s[0] - 'a'));

        ss >> halfmove >> fullmove;
    }

    void print(){
        constexpr char p[13] = {'P', 'p', 'N', 'n', 'B', 'b', 'R', 'r', 'Q', 'q', 'K', 'k', ' '};
        cout << "+---+---+---+---+---+---+---+---+\n";
        for(int r=7;r>-1;r--){
            for(int f=0;f<8;f++){
                cout << "| " << p[getPiece(r, f)] << " ";
            } cout << "|\n";
            cout << "+---+---+---+---+---+---+---+---+\n";
        }
    }

  private:
    int fen_helper(char p){
        if(p == 'P') return 0;
        if(p == 'N') return 2;
        if(p == 'B') return 4;
        if(p == 'R') return 6;
        if(p == 'Q') return 8;
        if(p == 'K') return 10;
        if(p == 'p') return 1;
        if(p == 'n') return 3;
        if(p == 'b') return 5;
        if(p == 'r') return 7;
        if(p == 'q') return 9;
        if(p == 'k') return 11;
        return -1;
    }
};

class LookupTables{
  public:
    static ull clear_rank(int rank){ return ~mask_rank(rank); }
    //0x80 => 0b10000000
    static ull clear_file(int file) { return ~mask_file(file); }
    
    static ull mask_rank(int rank){
      return 0xFFull << ((rank-1) * 8);
    }
    
    static ull mask_file(int file){
      return 0x0101010101010101ull << (file-1);
    }
};

struct move {
    ull from, to;
    board::piecetype type;
};

class MoveGeneration{
  public:
    static ull king(ull loc, ull my_pieces){
        ull clear_file_1 = loc & LookupTables::clear_file(1);
        ull clear_file_8 = loc & LookupTables::clear_file(8);
        ull NW = clear_file_1 << 7;
        ull N = loc << 8;
        ull NE = clear_file_8 << 9;
        ull E = clear_file_8 << 1;
        ull SE = clear_file_8 >> 7;
        ull S = loc >> 8;
        ull SW = clear_file_1 >> 9;
        ull W = clear_file_1 >> 1;
        return (NW | N | NE | E | SE | S | SW | W) & ~my_pieces;
    }

    static ull knight(ull loc, ull my_pieces){
        ull clear_file_1 = loc & LookupTables::clear_file(1);
        ull clear_file_2 = loc & LookupTables::clear_file(2);
        ull clear_file_7 = loc & LookupTables::clear_file(7);
        ull clear_file_8 = loc & LookupTables::clear_file(8);
        ull NWW = (clear_file_1 & clear_file_2) << 6;
        ull NNW = (clear_file_1) << 15;
        ull NNE = (clear_file_8) << 17;
        ull NEE = (clear_file_7 & clear_file_8) << 10;
        ull SEE = (clear_file_7 & clear_file_8) >> 6;
        ull SSE = (clear_file_8) >> 15;
        ull SSW = (clear_file_1) >> 17;
        ull SWW = (clear_file_1 & clear_file_2) >> 10;
        return (NWW | NNW | NNE | NEE | SEE | SSE | SSW | SWW) & ~my_pieces;
    }

    static ull white_pawn(ull loc, ull my_pieces, ull opp_pieces, ull en_passant){
        ull all_pieces = my_pieces | opp_pieces;
        ull default_step = (loc << 8) & ~all_pieces;
        ull two_steps = ((default_step & LookupTables::mask_rank(3)) << 8) & ~all_pieces;
        ull NW = ((loc & LookupTables::clear_file(1)) << 7) & (opp_pieces | en_passant);
        ull NE = ((loc & LookupTables::clear_file(8)) << 9) & (opp_pieces | en_passant);
        return (default_step | two_steps | NW | NE);
    }

    static ull black_pawn(ull loc, ull my_pieces, ull opp_pieces, ull en_passant){
        ull all_pieces = my_pieces | opp_pieces;
        ull default_step = (loc >> 8) & ~all_pieces;
        ull two_steps = ((default_step & LookupTables::mask_rank(6)) >> 8) & ~all_pieces;
        ull SW = ((loc & LookupTables::clear_file(1)) >> 9) & (opp_pieces | en_passant);
        ull SE = ((loc & LookupTables::clear_file(8)) >> 7) & (opp_pieces | en_passant);
        return (default_step | two_steps | SW | SE);
    }

    static ull rook(ull loc, ull my_pieces, ull opp_pieces){
        return rMap[_tzcnt_u64(loc) * 4096 + _pext_u64(my_pieces | opp_pieces, rMask[_tzcnt_u64(loc)])] & ~my_pieces;
    }

    static ull bishop(ull loc, ull my_pieces, ull opp_pieces){
        return bMap[_tzcnt_u64(loc) * 512 + _pext_u64(my_pieces | opp_pieces, bMask[_tzcnt_u64(loc)])] & ~my_pieces;
    }

    static ull queen(ull loc, ull my_pieces, ull opp_pieces){
        return rook(loc, my_pieces, opp_pieces) | bishop(loc, my_pieces, opp_pieces);
    }
};//morning morning morning morning morning eldoofus eldoofus eldoofus mornng 

int main(){
    // board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // // for (int i=0;i<14;i++){
    // //     cout << bitset<64>(b.bbs[i]) << endl;
    // // }
    // b.print();
    printbb(MoveGeneration::black_pawn((1ull << (4 + 6 * 8)), 0ull, 0ull, 0ull));
    printbb(MoveGeneration::black_pawn((1ull << (2 + 6 * 8)), 0ull, 0ull, 0ull));
    printbb(MoveGeneration::bishop((1ull << (4 + 4 * 8)), 0ull, 0ull));
    printbb(MoveGeneration::queen((1ull << (4 + 4 * 8)), 0ull, 0ull));
    // printbb(LookupTables::clear_file(1));
    // printbb(LookupTables::clear_file(8));
    // printbb(LookupTables::clear_rank(1));
}
