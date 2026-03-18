# Orderbook

A limit order book engine written in C++, with a REST API layer and an interactive web-based trading simulator.

<img width="1162" height="515" alt="RECAP Stock Exchange UI" src="https://github.com/user-attachments/assets/73dca5e5-8c6a-4659-b8c6-d9a7aae2c3b3" />

---

## What it does

- **Core engine** — models a real exchange order book: orders are grouped into price levels, matched price-priority / FIFO, and filled or left to rest
- **Matching engine** — incoming orders cross against the opposite side; unfilled remainder is added to the book
- **REST API** — C++ HTTP server exposing the order book over JSON endpoints
- **Trading simulator** — browser UI where you start with $10,000 and practice placing buy/sell limit orders against a live book

## Project structure

```
order.hpp          # Order struct + Side/OrderType enums
priceLevels.hpp    # PriceLevel class — groups orders at the same price, handles FIFO fills
orderbook.hpp      # OrderBook class — manages both sides, matching engine, book snapshot
main.cpp           # CLI demo (add/delete/modify orders, print book)
main_server.cpp    # HTTP server entry point (REST API + serves frontend)
public/index.html  # Self-contained trading UI
include/           # Single-header deps: httplib + nlohmann/json
Makefile
```

---

## Running locally

### Requirements

- **macOS or Linux**
- **g++ with C++17 support** — check with `g++ --version` (comes with Xcode Command Line Tools on Mac)

On macOS, install the compiler if needed:
```bash
xcode-select --install
```

### Clone the repo

```bash
git clone https://github.com/jacobroseseide/orderbook.git
cd orderbook
```

### Option 1 — Interactive trading UI (web frontend)

```bash
make server
./orderbook_server
```

Then open **http://localhost:8080** in your browser.

You'll see a live order book pre-seeded with market-maker orders around $100. Place buy or sell limit orders, watch fills happen in real time, and track your cash and P&L.

### Option 2 — CLI demo

```bash
make
./orderbook
```

Prints a walkthrough of adding orders to both sides of the book, deleting an order, and modifying a quantity.

---

## API endpoints

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/book` | Current bids and asks (sorted, price + qty) |
| `POST` | `/api/order` | Place an order — body: `{"side":"buy","price":100.25,"qty":50}` |
| `DELETE` | `/api/order/:id` | Cancel a resting order by ID |
| `GET` | `/api/portfolio` | Cash, shares, unrealised P&L, trade history |

---

## How the engine works

Orders are stored in a two-sided book (`buy_orders`, `sell_orders`), each a vector of `PriceLevel` objects. A price level holds all orders at a single price as a FIFO queue.

When a new order arrives, `matchAndFill` walks the opposite side in price-priority order — best ask first for buys, best bid first for sells — consuming shares from each level until the order is filled or no more eligible levels remain. Any unfilled remainder is inserted into the resting book.

```
Buy order @ $100.50
  → sort asks ascending
  → $100.25 ask: consume 200 shares  ✓
  → $100.50 ask: consume remaining   ✓
  → fully filled, nothing rests
```
