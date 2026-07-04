#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include "chess.hpp"

using namespace std;
using namespace chess;

// --- Types and Globals ---
struct TTEntry {
    uint64_t key;
    Move bestMove;
    int score, depth, bound;
};
static const uint64_t TT_SIZE_LOG = 23;
static const uint64_t TT_SIZE = 1ULL << TT_SIZE_LOG;
static vector<TTEntry> TT(TT_SIZE);
static int historyTable[2][7][64] = {};
Board board;
Move rootBestMove;

chrono::steady_clock::time_point searchStart;
int TIME_MS = 500;  // Example fixed time
bool timeOut;

// --- Engine Logic ---

inline TTEntry& probe(uint64_t key) {
    return TT[key & (TT_SIZE - 1)];
}

int evaluate(const Board &b) {
    int eval = 0;
    // TODO: Add material, PST, piece mobility, king safety, pawn structure, etc.
    return (b.sideToMove() == Color::WHITE ? eval : -eval);
}

int quiescence(int alpha, int beta) {
    int stand = evaluate(board);
    if (stand >= beta) return beta;
    alpha = max(alpha, stand);

    Movelist caps; movegen::legalmoves<movegen::MoveGenType::CAPTURE>(caps, board);
    for (Move m : caps) {
        board.makeMove(m);
        int score = -quiescence(-beta, -alpha);
        board.unmakeMove(m);
        if (score >= beta) return beta;
        alpha = max(alpha, score);
    }
    return alpha;
}

Move orderMoveList(const vector<Move> &moves, Move ttMove, vector<int> &scores) {
    int best = 0;
    for (int i = 1; i < moves.size(); i++)
        if (scores[i] > scores[best]) best = i;
    Move chosen = moves[best];
    swap(scores[best], scores[0]);
    return chosen;
}

int negamax(int alpha, int beta, int depth, bool isRoot = false) {
    if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - searchStart).count() > TIME_MS)
        return timeOut = true, 0;

    uint64_t key = board.hash();
    auto &entry = probe(key);
    if (entry.key == key && entry.depth >= depth) {
        if (entry.bound == 0 && entry.score <= alpha) return entry.score;
        if (entry.bound == 1 && entry.score >= beta) return entry.score;
        if (entry.bound == 2) return entry.score;
    }

    if (depth == 0)
        return quiescence(alpha, beta);

    auto moves = board.legal_moves();
    vector<int> scores(moves.size(), 0);
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i] == entry.bestMove) scores[i] += 1000000;
        if (board.isCapture(moves[i])) {
            Piece cap = board.at(moves[i].to());
            scores[i] += 10000 * static_cast<int>(cap.type());
        } else {
            scores[i] += historyTable[board.sideToMove() == Color::WHITE][static_cast<int>(moves[i].from().pieceType())][moves[i].to().index()];
        }
    }

    Move bestMove = Move::NO_MOVE;
    int bestScore = numeric_limits<int>::min();
    int alphaOrig = alpha;

    while (!moves.empty()) {
        Move m = orderMoveList(moves, entry.bestMove, scores);
        board.makeMove(m);
        int score = -negamax(-beta, -alpha, depth - 1);
        board.unmakeMove(m);
        if (timeOut) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }

        alpha = max(alpha, score);
        if (alpha >= beta) {
            if (!board.isCapture(m))
                historyTable[board.sideToMove()==Color::WHITE][static_cast<int>(m.from().pieceType())][m.to().index()] += depth;
            break;
        }
    }

    entry = {key, bestMove, bestScore, depth,
             bestScore <= alphaOrig ? 0 :
             bestScore >= beta       ? 1 : 2};

    if (isRoot && !timeOut) rootBestMove = bestMove;
    return bestScore;
}

// --- UCI Interface ---
Move parse_uci(const std::string &uciMove, const Board &board) {
    auto moves = board.legal_moves();
    for (const Move &move : moves) {
        if (uci::to_string(move) == uciMove)
            return move;
    }
    return Move::NO_MOVE; // fallback
}


void uci_loop() {
    string line;
    while (getline(cin, line)) {
        if (line == "uci") {
            cout << "id name Bot614_Cpp\nid author You\nuciok\n";
        } else if (line == "isready") {
            cout << "readyok\n";
        } else if (line.rfind("position", 0) == 0) {
            board.setFen(constants::STARTPOS);
            auto p = line.find("moves");
            if (p != string::npos) {
                istringstream iss(line.substr(p + 6));
                string mv;
                while (iss >> mv)
                    board.makeMove(parse_uci(mv, board));
            }
        } else if (line.rfind("go", 0) == 0) {
            searchStart = chrono::steady_clock::now();
            timeOut = false;
            int depth = 6;
            for (int d = 1; d <= depth && !timeOut; ++d)
                negamax(-30000, 30000, d, true);
            cout << "bestmove " << uci::to_string(rootBestMove) << "\n";
        } else if (line == "quit") {
            break;
        }
    }
}

int main() {
    iostream::sync_with_stdio(false);
    uci_loop();
    return 0;
}
