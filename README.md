# ♟️ Chess Engine

A classical **C++ Chess Engine** implementing efficient adversarial game-tree search using **Minimax**, **Alpha-Beta Pruning**, **Iterative Deepening**, and **Quiescence Search**. The engine follows the **Universal Chess Interface (UCI)** protocol and can be used with standard chess GUIs.

## Algorithms & Features

- Minimax Search
- Alpha-Beta Pruning
- Iterative Deepening
- Quiescence Search
- Transposition Tables
- Move Ordering
- Killer & History Heuristics
- Piece-Square Tables
- Material & Pawn Structure Evaluation
- King Safety Evaluation
- Opening Book


## Run

Start the engine:

```bash
engine.exe
```

Example UCI commands:

```text
uci
isready
position startpos
go
```

Example output:

```text
bestmove e2e4
```

## Playing Against the Engine

Use any UCI-compatible GUI such as:

- Arena Chess
- CuteChess
- Banksia GUI

Load `engine.exe` as a UCI engine and start a new game.
