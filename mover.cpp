#include"moves.cpp"

void printbb(ull p){
    cout << "+---+---+---+---+---+---+---+---+\n";
    for(int r=7;r>-1;r--){
        for(int f=0;f<8;f++){
            cout << "| " << ((p & (1ull << (r * 8 + f))) ? "X" : " ") << " ";
        } cout << "|\n";
        cout << "+---+---+---+---+---+---+---+---+\n";
    }
}

constexpr bool debug = false;
constexpr bool post = true;
constexpr int dlvl = 0;

class mover {
  public:
	static inline ull enodes, qnodes, cuts, skips;
	static inline int maxdepth, halfmove, fullmove;
	static inline long long alpha[256];
	static inline long long beta[256];
	static inline ull reps[1024];

	static __attribute__((always_inline)) inline void Init(int md) {
		mover::enodes = 0;
		mover::qnodes = 0;
		mover::cuts = 0;
		mover::skips = 0;
		for(int i=0;i<64;i++){
			alpha[i] = -21474836470ll;
			beta[i] = 21474836470ll;
		}
		maxdepth = md;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Kingmove(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		if(to & brd.occupied) halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::king, status.wMove>(brd, from, to, to & Enemy<status.wMove>(brd));
		mGen::enking[depth - 1] = Lookup::King(square(to));
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
		if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.kingMove(), mover>::e(next, depth-1);
		if (depth == maxdepth) addEval(-eval);
        mGen::enking[depth - 1] = mGen::myking[depth];
		if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
		if(to & brd.occupied) halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F, ull rookswitch>
	static long long KingCastle(const board& brd, ull from, ull to, int depth) {
		fullmove++;
		board next = board::moveCastle<status.wMove>(brd, from | to, rookswitch);
		mGen::enking[depth - 1] = Lookup::King(square(to));
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << (from | to) << " " << rookswitch << endl;
		long long eval = F<status.kingMove(), mover>::e(next, depth-1);
        if (depth == maxdepth) addEval(-eval);
        mGen::enking[depth - 1] = mGen::myking[depth];
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << (from | to) << " " << rookswitch << " " << -eval << endl;
		fullmove--;
		return eval;
	}

	template<class status status>
	static void PawnCheck(ull eking, ull to, int depth) {
		constexpr bool white = status.wMove;
		ull pl = Pawn_AttackLeft<white>(to & Pawns_NotLeft());
		ull pr = Pawn_AttackRight<white>(to & Pawns_NotRight());

		if (eking & (pl | pr)) mGen::checks[depth - 1] = to;
	}

	template<class status status>
	static void KnightCheck(ull eking, ull to, int depth) {
		constexpr bool white = status.wMove;

		if (Lookup::Knight(square(eking)) & to) mGen::checks[depth - 1] = to;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Pawnmove(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::pawn, status.wMove, false>(brd, from, to);
		PawnCheck<status>(EnemyKing<status.wMove>(brd), to, depth);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		mGen::checks[depth - 1] = 0xffffffffffffffffull;
		halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Pawnatk(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::pawn, status.wMove, true>(brd, from, to);
		PawnCheck<status>(EnemyKing<status.wMove>(brd), to, depth);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.kingMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		mGen::checks[depth - 1] = 0xffffffffffffffffull;
		halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long PawnEnpassantTake(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		halfmove = fullmove;
		fullmove++;
		board next = board::moveEP<status.wMove>(brd, from, to);
		PawnCheck<status>(EnemyKing<status.wMove>(brd), to, depth);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		mGen::checks[depth - 1] = 0xffffffffffffffffull;
		halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Pawnpush(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		halfmove = fullmove;
		fullmove++;
		board next = board::move <boardPiece::pawn, status.wMove, false>(brd, from, to);

		mList::epSquare = to;
		PawnCheck<status>(EnemyKing<status.wMove>(brd), to, depth);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.pawnPush(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		mGen::checks[depth - 1] = 0xffffffffffffffffull;
		halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F, boardPiece p>
	static long long Pawnpromote(const board& brd, ull from, ull to, int depth) {
		int h = halfmove;
		halfmove = fullmove;
		fullmove++;
		board next = board::movePromote<p, status.wMove>(brd, from, to);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << (int)p << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << (int)p << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Knightmove(const board& brd, ull from, ull to, int depth){
		int h = halfmove;
		if(to & brd.occupied) halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::knight, status.wMove>(brd, from, to, to & Enemy<status.wMove>(brd));
		KnightCheck<status>(EnemyKing<status.wMove>(brd), to, depth);
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		mGen::checks[depth - 1] = 0xffffffffffffffffull;
		if(to & brd.occupied) halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Bishopmove(const board& brd, ull from, ull to, int depth){
		int h = halfmove;
		if(to & brd.occupied) halfmove = fullmove;
		fullmove++;
		board next = board::move <boardPiece::bishop, status.wMove>(brd, from, to, to & Enemy<status.wMove>(brd));
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if (depth == maxdepth) addEval(-eval);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
		if(to & brd.occupied) halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Rookmove(const board& brd, ull from, ull to, int depth){
		int h = halfmove;
		if(to & brd.occupied) halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::rook, status.wMove>(brd, from, to, to & Enemy<status.wMove>(brd));
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval;
		if constexpr (status.canCastle()) {
			if (status.isLeftRook(from)) eval = F<status.rookMoveL(), mover>::e(next, depth-1);
			else if (status.isRightRook(from)) eval = F<status.rookMoveR(), mover>::e(next, depth-1);
			else eval = F<status.silentMove(), mover>::e(next, depth-1);
		} else eval = F<status.silentMove(), mover>::e(next, depth-1);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
        if (depth == maxdepth) addEval(-eval);
		if(to & brd.occupied) halfmove = h;
		fullmove--;
		return eval;
	}

	template<class status status, template<class ::status, class> class F>
	static long long Queenmove(const board& brd, ull from, ull to, int depth){
		int h = halfmove;
		if(to & brd.occupied) halfmove = fullmove;
		fullmove++;
		board next = board::move<boardPiece::queen, status.wMove>(brd, from, to, to & Enemy<status.wMove>(brd));
		alpha[depth-1] = -beta[depth];
		beta[depth-1] = -alpha[depth];
        if constexpr (!post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << endl;
		long long eval = F<status.silentMove(), mover>::e(next, depth-1);
        if (depth == maxdepth) addEval(-eval);
        if constexpr (post && debug) if(depth > dlvl) cout << string(depth, ' ') << __func__ << " " << from << " " << to << " " << -eval << endl;
		if(to & brd.occupied) halfmove = h;
		fullmove--;
		return eval;
	}
};