#include "orderbook.hpp"
#include "include/httplib.h"
#include "include/nlohmann/json.hpp"
#include <sstream>
#include <iomanip>
#include <mutex>
#include <atomic>
#include <ctime>

using json = nlohmann::json;

// ── portfolio state ────────────────────────────────────────────────────────────

struct Trade {
    std::string side;       // "buy" | "sell"
    uint32_t    qty;
    double      price;
    std::string timestamp;
};

// format current time as HH:MM:SS
static std::string now_str() {
    std::time_t t = std::time(nullptr);
    std::tm *tm = std::localtime(&t);
    char buf[9];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    return buf;
}

struct Portfolio {
    double             cash   = 10000.0;
    int                shares = 0;
    double             cost_basis = 0.0; // total spent acquiring current shares
    std::vector<Trade> trades;

    // apply a fill and update portfolio; returns false if funds/shares insufficient
    bool apply(const std::string &side, uint32_t qty, double avg_price) {
        if (side == "buy") {
            double cost = qty * avg_price;
            if (cost > cash) return false;
            cash       -= cost;
            cost_basis += cost;
            shares     += static_cast<int>(qty);
        } else {
            if (static_cast<int>(qty) > shares) return false;
            double proceeds = qty * avg_price;
            // adjust cost_basis proportionally
            if (shares > 0)
                cost_basis -= cost_basis * (static_cast<double>(qty) / shares);
            cash   += proceeds;
            shares -= static_cast<int>(qty);
            if (shares == 0) cost_basis = 0.0;
        }
        trades.push_back({ side, qty, avg_price, now_str() });
        return true;
    }

    double pnl() const {
        // unrealised P&L based on last trade price; 0 if no position
        if (shares == 0 || trades.empty()) return 0.0;
        double last_price = trades.back().price;
        return shares * last_price - cost_basis;
    }
};

// ── global state (protected by mutex) ─────────────────────────────────────────

static OrderBook   g_book;
static Portfolio   g_portfolio;
static std::mutex  g_mutex;
static std::atomic<uint64_t> g_next_user_id{1};

// ── CORS helper ────────────────────────────────────────────────────────────────

static void set_cors(httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// ── main ───────────────────────────────────────────────────────────────────────

int main() {
    // seed the book with market-maker orders around $100.00
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        g_book.seedMarketMakerOrders(100.0, 6, 150);
    }

    httplib::Server svr;

    // serve the public/ directory as static files
    svr.set_mount_point("/", "./public");

    // ── GET /api/book ──────────────────────────────────────────────────────────
    // returns current sorted bids and asks
    svr.Get("/api/book", [](const httplib::Request &, httplib::Response &res) {
        std::lock_guard<std::mutex> lk(g_mutex);

        auto bids = g_book.getBids();
        auto asks = g_book.getAsks();

        json j;
        j["bids"] = json::array();
        j["asks"] = json::array();

        for (auto &b : bids)
            j["bids"].push_back({ {"price", b.price}, {"qty", b.quantity} });
        for (auto &a : asks)
            j["asks"].push_back({ {"price", a.price}, {"qty", a.quantity} });

        set_cors(res);
        res.set_content(j.dump(), "application/json");
    });

    // ── GET /api/portfolio ─────────────────────────────────────────────────────
    svr.Get("/api/portfolio", [](const httplib::Request &, httplib::Response &res) {
        std::lock_guard<std::mutex> lk(g_mutex);

        json j;
        j["cash"]   = std::round(g_portfolio.cash   * 100.0) / 100.0;
        j["shares"] = g_portfolio.shares;
        j["pnl"]    = std::round(g_portfolio.pnl()  * 100.0) / 100.0;

        j["trades"] = json::array();
        for (auto &t : g_portfolio.trades)
            j["trades"].push_back({
                {"side",      t.side},
                {"qty",       t.qty},
                {"price",     std::round(t.price * 100.0) / 100.0},
                {"timestamp", t.timestamp}
            });

        set_cors(res);
        res.set_content(j.dump(), "application/json");
    });

    // ── POST /api/order ────────────────────────────────────────────────────────
    // body: { "side": "buy"|"sell", "price": 100.25, "qty": 50 }
    svr.Post("/api/order", [](const httplib::Request &req, httplib::Response &res) {
        std::lock_guard<std::mutex> lk(g_mutex);

        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"invalid JSON\"}", "application/json");
            return;
        }

        std::string side_str = body.value("side",  "buy");
        double      price    = body.value("price", 0.0);
        uint32_t    qty      = body.value("qty",   0u);

        if (qty == 0 || price <= 0) {
            res.status = 400;
            res.set_content("{\"error\":\"qty and price must be positive\"}", "application/json");
            return;
        }

        Side side = (side_str == "sell") ? Side::sell : Side::buy;
        uint64_t id = g_next_user_id++;
        Order o { id, side, OrderType::limit, price, qty };

        // check portfolio has enough to cover the order before matching
        if (side == Side::buy && price * qty > g_portfolio.cash) {
            res.status = 400;
            res.set_content("{\"error\":\"insufficient cash\"}", "application/json");
            return;
        }
        if (side == Side::sell && static_cast<int>(qty) > g_portfolio.shares) {
            res.status = 400;
            res.set_content("{\"error\":\"insufficient shares\"}", "application/json");
            return;
        }

        FillResult fill = g_book.matchAndFill(o);

        json j;
        j["order_id"]   = id;
        j["filled_qty"] = fill.filled_qty;
        j["avg_price"]  = std::round(fill.avg_price * 100.0) / 100.0;
        j["was_filled"] = fill.was_filled;
        j["status"]     = fill.filled_qty == qty ? "fully_filled"
                        : fill.filled_qty > 0    ? "partially_filled"
                        :                          "resting";

        if (fill.was_filled)
            g_portfolio.apply(side_str, fill.filled_qty, fill.avg_price);

        set_cors(res);
        res.set_content(j.dump(), "application/json");
    });

    // ── DELETE /api/order/:id ──────────────────────────────────────────────────
    svr.Delete(R"(/api/order/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        std::lock_guard<std::mutex> lk(g_mutex);

        uint64_t id = std::stoull(req.matches[1]);
        bool ok = g_book.deleteOrder(id);

        json j;
        j["deleted"] = ok;
        if (!ok) j["error"] = "order not found";

        set_cors(res);
        res.status = ok ? 200 : 404;
        res.set_content(j.dump(), "application/json");
    });

    // preflight
    svr.Options(".*", [](const httplib::Request &, httplib::Response &res) {
        set_cors(res);
        res.status = 204;
    });

    std::cout << "RECAP Stock Exchange running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
