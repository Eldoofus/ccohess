#include"senjo/ChessEngine.h"
#include"senjo/UCIAdapter.h"
#include"senjo/Output.h"
#include"mover.cpp"

#include"polyglot.cpp"

constexpr bool engineDebug = true;

class ccohess : public senjo::ChessEngine {
  public:
    bool initialized = false, inbook = true;
    board* b = new board(board::startpos()); // disgusting workaround
    status* stat;
    ull EP;
    inline static vector<string> moves;
    inline static vector<long long> evals;

    string getEngineName() const {
        return "ccohess";
    }
    
    string getEngineVersion() const {
        return "0.0";
    }

    string getAuthorName() const {
        return "Eldoofus, ccohho, Betterthanyou!";
    }

    list<senjo::EngineOption> getOptions() const {
        return list<senjo::EngineOption>();
    }

    bool setEngineOption(const string& optionName, const string& optionValue){
        return false;
    }

    void initialize(){
        initialized = true;
        inbook = true;
    }

    bool isInitialized() const {
        return initialized;
    }

    bool setPosition(const string& fen, string* remain = nullptr){
        b = new board(fen);
        stat = new status(FenInfo(0, fen), FenInfo(1, fen), FenInfo(4, fen), FenInfo(5, fen), FenInfo(2, fen), FenInfo(3, fen));
        EP = FenEP(fen);
        evaluator::_eval<true>(*stat, *b);
        mover::halfmove = -1;
        mover::fullmove = 0;
        //b->print();
        return true;
    }

    template<class status status>
    string rep(const board& brd){
        mover::reps[mover::fullmove] = Lookup::zobrist<status>(brd, EP);
        return "";
    }
    BookToTemplate(rep);

    bool makeMove(const string& move){
        ull from = 1ull << (move[0] - 'a' + (move[1] - '1') * 8);
        ull to = 1ull << (move[2] - 'a' + (move[3] - '1') * 8);

        boardPiece p = b->getPiece(from);
        bool t = to & (stat->wMove ? b->blacks : b->whites);
        board* oldb = b;
        status* olds = stat;
        // cout << "board before move: " << move << endl;
        // b->print();
        // cout << "white to move: " << stat->wMove << endl;
        // cout << "board after move: " << move << endl;
        _rep(*stat, *b);
        if(p == boardPiece::pawn){
            mover::halfmove = mover::fullmove;
            if(move.length() == 5){
                if(stat->wMove){
                    if(move[4] == 'n') b = new board(board::movePromote<boardPiece::knight, true>(*b, from, to));
                    if(move[4] == 'b') b = new board(board::movePromote<boardPiece::bishop, true>(*b, from, to));
                    if(move[4] == 'r') b = new board(board::movePromote<boardPiece::rook, true>(*b, from, to));
                    if(move[4] == 'q') b = new board(board::movePromote<boardPiece::queen, true>(*b, from, to));
                } else {
                    if(move[4] == 'n') b = new board(board::movePromote<boardPiece::knight, false>(*b, from, to));
                    if(move[4] == 'b') b = new board(board::movePromote<boardPiece::bishop, false>(*b, from, to));
                    if(move[4] == 'r') b = new board(board::movePromote<boardPiece::rook, false>(*b, from, to));
                    if(move[4] == 'q') b = new board(board::movePromote<boardPiece::queen, false>(*b, from, to));
                }
                stat = new status(stat->silentMove());
            } else if((to >> 16) == from || (to << 16) == from){
                if(stat->wMove) b = new board(board::move<boardPiece::pawn, true, false>(*b, from, to));
                else b = new board(board::move<boardPiece::pawn, false, false>(*b, from, to));
                EP = to;
                stat = new status(stat->pawnPush());
            } else if(stat->hasEP){
                if(stat->wMove){
                    if(to == (EP << 8)) b = new board(board::moveEP<true>(*b, from, to));
                    else b = new board(board::move<boardPiece::pawn, true>(*b, from, to, t));
                } else {
                    if(to == (EP >> 8)) b = new board(board::moveEP<false>(*b, from, to));
                    else b = new board(board::move<boardPiece::pawn, false>(*b, from, to, t));
                }
                stat = new status(stat->silentMove());
            } else {
                if(stat->wMove) b = new board(board::move<boardPiece::pawn, true>(*b, from, to, t));
                else b = new board(board::move<boardPiece::pawn, false>(*b, from, to, t));
                stat = new status(stat->silentMove());
            }
        } else if(p == boardPiece::knight){
            if(t) mover::halfmove = mover::fullmove;
            if(stat->wMove) b = new board(board::move<boardPiece::knight, true>(*b, from, to, t));
            else b = new board(board::move<boardPiece::knight, false>(*b, from, to, t));
            stat = new status(stat->silentMove());
        } else if(p == boardPiece::bishop){
            if(t) mover::halfmove = mover::fullmove;
            if(stat->wMove) b = new board(board::move<boardPiece::bishop, true>(*b, from, to, t));
            else b = new board(board::move<boardPiece::bishop, false>(*b, from, to, t));
            stat = new status(stat->silentMove());
        } else if(p == boardPiece::rook){
            if(t) mover::halfmove = mover::fullmove;
            if(stat->wMove) b = new board(board::move<boardPiece::rook, true>(*b, from, to, t));
            else b = new board(board::move<boardPiece::rook, false>(*b, from, to, t));
            if(stat->canCastle()){
                if(stat->isLeftRook(from)) stat = new status(stat->rookMoveL());
                else if(stat->isRightRook(from)) stat = new status(stat->rookMoveR());
                else stat = new status(stat->silentMove());
            } else stat = new status(stat->silentMove());
        } else if(p == boardPiece::queen){
            if(t) mover::halfmove = mover::fullmove;
            if(stat->wMove) b = new board(board::move<boardPiece::queen, true>(*b, from, to, t));
            else b = new board(board::move<boardPiece::queen, false>(*b, from, to, t));
            stat = new status(stat->silentMove());
        } else if(p == boardPiece::king){
            if(t) mover::halfmove = mover::fullmove;
            if((to << 2) == from){
                if(stat->wMove) b = new board(board::moveCastle<true>(*b, from | to, stat->rookswitchL()));
                else b = new board(board::moveCastle<false>(*b, from | to, stat->rookswitchL()));
            } else if((to >> 2) == from){
                if(stat->wMove) b = new board(board::moveCastle<true>(*b, from | to, stat->rookswitchR()));
                else b = new board(board::moveCastle<false>(*b, from | to, stat->rookswitchR()));
            } else {
                if(stat->wMove) b = new board(board::move<boardPiece::king, true>(*b, from, to, t));
                else b = new board(board::move<boardPiece::king, false>(*b, from, to, t));
            }
            stat = new status(stat->kingMove());
        }

        // b->print();
        // cout << "white to move: " << stat->wMove << endl;
        // cout << "has EP: " << stat->hasEP << endl;
        // cout << "EP: " << EP << endl;
        mover::fullmove++;
        evaluator::_eval<true>(*stat, *b);
        delete oldb;
        delete olds;
        return true;
    }

    string getFEN() const { return ""; }
    void printBoard() const { return; }
    bool whiteToMove() const { return true; }
    void clearSearchData(){ return; }
    void ponderHit(){ return; }
    void setDebug(const bool flag){ return; }
    bool isDebugOn() const { return false; }
    bool isSearching(){ return false; }
    void stopSearching(){ return; }
    bool stopRequested() const { return false; }
    void waitForSearchFinish(){}
    uint64_t perft(const int depth){ return 0ull; }

    template<class status status>
    long long search(const board& brd, const int& depth, const long long& alpha, const long long& beta){
        // cout << "has EP: " << stat->hasEP << endl;
        // cout << "EP: " << EP << endl;
        mList::init<status>(brd, depth + 25);
        mList::initEP(EP);
        mover::Init(depth + 25);
        mover::alpha[depth+26] = max(alpha, -21474836470ll);
        mover::beta[depth+26] = min(beta, 21474836470ll);
        return mList::EnumerateMoves<status, mover>::e(brd, depth + 25);
        //cout << "zobrist: " << format("{:#x}", Lookup::zobrist<status>(brd, EP)) << endl;
    }
    StatusToTemplate(search);

    template<class status status>
    string book(const board& brd){
        return polyglot::get(Lookup::zobrist<status>(brd, EP));
    }
    BookToTemplate(book);
    
    auto iterDeepen(ull time, int& depth, long long& eval, int& bestmove){
        using namespace chrono;
        long long delta, alpha, beta;
        ull e;
        searchtime = time;
        start = high_resolution_clock::now();
        eval = _search(*stat, *b, 1, -21474836470ll, 21474836470ll);
        for(;depth<229;depth++){
            try {
                mover::Init<true>(depth + 25);
                delta = 9 + eval * eval / 16384;
                alpha = eval - alpha;
                beta = eval + beta;
                while(true){
                    eval = _search(*stat, *b, depth, alpha, beta);
                    if (eval <= alpha){
                        alpha -= delta;
                        beta = (alpha + 3 * beta) / 4;
                    } else if (eval >= beta){
                        beta += delta;
                    } else break;
                    delta += delta / 3;
                }
                ::end = high_resolution_clock::now();
                e = duration_cast<milliseconds>(::end - ::start).count();
                cout << "Reached depth " << depth << ": " << (eval >= 0 ? "+" : "") << eval << endl;
                bestmove = pvtable[depth+25][depth+25];
                if (depth > 4 && e >= time / 20) return e;
            } catch (int x) {
                depth--;
                cout << "Aborting Search\n";
                return e;
            }
        }
        for(int i=229;bestmove != 0 && i > -1;i--) bestmove = pvtable[i+25][i+25];
        return e;
    }

    string moveString(int m){
        if(!m) return "";
        string s = "";
        s += ((m >> 6 & 0b111) + 'a');
        s += ((m >> 9 & 0b111) + '1');
        s += ((m & 0b111) + 'a');
        s += ((m >> 3 & 0b111) + '1');
        if(m >> 12) s += ((m >> 12) + '`');
        return s;
    }

    string go(const senjo::GoParams& params, string* ponder = nullptr){
        if(inbook){
            string s = _book(*stat, *b);
            if(s == "") {
                inbook = false;
                cout << "Out of book, switching to analysis\n";
            } else {
                cout << "Book move: " << s << endl;
                return s;
            }
        }
        cout << mover::fullmove << " " << mover::halfmove << " " << mover::fullmove - mover::halfmove << endl;
        int depth = 2;
        long long eval;
        int bestmove;
        auto elapsed = iterDeepen((stat->wMove ? params.wtime : params.btime) + params.movetime * 10, depth, eval, bestmove);
        cout << "elapsed: " << elapsed << "ms (depth " << depth << ")\n";
        // if(moves.empty()) {
        //     b->print();
        //     cout << "No Moves!\n";
        //     return "";
        // }
        // cout << "moves:";
        // for (string s : moves) cout << " " << s;
        // cout << "\nevals:";
        // for (long long d : evals) cout << " " << d;
        // cout << endl;
        // long long be = -21474836470ll;
        // int bi = 0;
        // for(int i=0;i<moves.size();i++) if(evals[i] > be){
        //     be = evals[i];
        //     bi = i;
        // }
        // cout << "best move: " << moves.at(bi) << endl;
        //for(;depth>-1 && !pvtable[depth+25][depth+25];depth--) cout << "skipping depth " << depth << endl;
        cout << "Search Eval: " << eval << endl;
        cout << "PV line: ";
        for(int i=depth+25;i>-1;i--){
            if (pvtable[depth+25][i]) cout << moveString(pvtable[depth+25][i]) << " ";
        } cout << endl;
        cout << "total nodes: " << mover::enodes + mover::qnodes << " (" << mover::enodes << " + " << mover::qnodes << ")" << endl;
        cout << "beta cutoffs: " << mover::cuts << " (" << (double)mover::cuts / (mover::enodes + mover::qnodes) * 100 << "%)\n";
        cout << "TT skips: " << mover::skips << " (" << (double)mover::skips / (mover::enodes + mover::qnodes) * 100 << "%)\n";
        return moveString(bestmove);
    }

    senjo::SearchStats getSearchStats() const {
        return senjo::SearchStats();
    }
};


void addMove(ull from, ull to, char promo){
    //cout << from << ' ' << to << endl;
    int f = square(from), t = square(to);
    string s = "";
    s += 'a' + f % 8;
    s += '1' + f / 8;
    s += 'a' + t % 8;
    s += '1' + t / 8;
    s += promo;
    ccohess::moves.push_back(s);
}

void addEval(long long e){
    //cout << str << ": " << e << endl;
    ccohess::evals.push_back(e);
}

int main() {
    try {
        srand(time(0));
        cout << "Beep Boop\n";
        ccohess engine;
        senjo::UCIAdapter adapter(engine);
        adapter.doCommand("position startpos");
        
        string line;
        line.reserve(16384);
        
        ofstream o;
        o.open("log.txt", ios_base::app);
        o << "Beep Boop\n";

        while (getline(cin, line)) {
            //cout << line << ", length: " << line.length();
            o << line << endl;
            if (line == "position startpos") continue;
            if (!adapter.doCommand(line)) break;
            if (line == "ucinewgame") adapter.doCommand("position startpos");
        }
        o.close();
        return 0;
    } catch (const exception& e) {
        senjo::Output() << "ERROR: " << e.what();
        return 1;
    }
}

// int main(){
    // string s; int n;
    // cout << "b: ";
    // getline(cin, s);
    // cout << "n: ";
    // cin >> n;
    // _PerfT(s, n);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 2);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 3);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 4);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 5);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 6);
    // cout << mover::nodes << endl;
    // PerfT<s>("", b, 7);
    // cout << mover::nodes << endl;
// }
