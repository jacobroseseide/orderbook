#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include "order.hpp"
#include "priceLevels.hpp"

// Represents a matched fill: how many shares were filled and at what average price
struct FillResult {
    uint32_t filled_qty;
    double   avg_price;
    bool     was_filled; // true if any shares were matched
};

// A snapshot of one side of the book for serialization / display
struct BookLevel {
    double   price;
    uint32_t quantity;
};

class OrderBook
{
private:
    using PriceLevelVec = std::vector<PriceLevel>;

    PriceLevelVec buy_orders;
    PriceLevelVec sell_orders;

    // next auto-incrementing id for market-maker seed orders
    uint64_t next_id = 1000;

public:

    // add an order to the correct side of the book, merging into existing price level if one exists
    void addOrder(Order o) {

        if (o.side == Side::buy) {
            for (PriceLevel &p : buy_orders) {
                if (p.getPrice() == o.price) {
                    p.addOrder(o);
                    return;
                }
            }
            PriceLevel newLevel(o.price);
            newLevel.addOrder(o);
            buy_orders.push_back(newLevel);

        } else {
            for (PriceLevel &p : sell_orders) {
                if (p.getPrice() == o.price) {
                    p.addOrder(o);
                    return;
                }
            }
            PriceLevel newLevel(o.price);
            newLevel.addOrder(o);
            sell_orders.push_back(newLevel);
        }
    }

    // print the book to stdout sorted best-price first on each side
    void printBook() {
        std::sort(buy_orders.begin(), buy_orders.end(), [](const PriceLevel &a, const PriceLevel &b)
                  { return a.getPrice() > b.getPrice(); });

        std::cout << "BUY ORDER BOOK:\n";
        for (const PriceLevel &p : buy_orders) {
            std::cout << p.getPrice() << ": " << p.getTotalQuantity() << std::endl;
        }

        std::cout << "--------------------------\n";

        std::sort(sell_orders.begin(), sell_orders.end(), [](const PriceLevel &a, const PriceLevel &b)
            { return a.getPrice() < b.getPrice(); });

        std::cout << "SELL ORDER BOOK:\n";
        for (const PriceLevel &p : sell_orders) {
            std::cout << p.getPrice() << ": " << p.getTotalQuantity() << std::endl;
        }
    }

    // delete an order by id from whichever side holds it; removes the price level if it becomes empty
    bool deleteOrder(uint64_t id) {
        for (auto it = buy_orders.begin(); it != buy_orders.end(); ++it) {
            if (it->deleteOrder(id)) {
                if (it->isEmpty()) buy_orders.erase(it);
                return true;
            }
        }
        for (auto it = sell_orders.begin(); it != sell_orders.end(); ++it) {
            if (it->deleteOrder(id)) {
                if (it->isEmpty()) sell_orders.erase(it);
                return true;
            }
        }
        return false;
    }

    // helper: modify an order within a specific side's price-level vector
    bool modifyOrderList(uint64_t id, uint32_t new_quantity, PriceLevelVec &v) {
        if (new_quantity == 0) {
            deleteOrder(id);
            std::cout << "successfully deleted — user requested new quantity be 0 shares\n";
            return true;
        }

        for (PriceLevel &level : v) {
            uint32_t total = level.getTotalQuantity();
            if (level.modifyOrder(id, new_quantity)) {
                // validate: only allow reducing quantity
                if (new_quantity > total) {
                    // undo — restore original (modifyOrder already set it, so set it back)
                    level.modifyOrder(id, total);
                    return false; // can't add shares
                }
                if (new_quantity == total) {
                    std::cout << "user requested to modify shares to amount they previously had — nothing changed\n";
                }
                return true;
            }
        }
        return false;
    }

    // reduce the quantity of an existing order; cannot increase shares
    bool modifyOrder(uint64_t id, uint32_t new_quantity, bool isBuy) {
        return isBuy ? modifyOrderList(id, new_quantity, buy_orders)
                     : modifyOrderList(id, new_quantity, sell_orders);
    }

    // return the best (highest) bid price, or -1 if no bids
    double getBestBid() const {
        if (buy_orders.empty()) return -1.0;
        double best = buy_orders[0].getPrice();
        for (const PriceLevel &p : buy_orders)
            if (p.getPrice() > best) best = p.getPrice();
        return best;
    }

    // return the best (lowest) ask price, or -1 if no asks
    double getBestAsk() const {
        if (sell_orders.empty()) return -1.0;
        double best = sell_orders[0].getPrice();
        for (const PriceLevel &p : sell_orders)
            if (p.getPrice() < best) best = p.getPrice();
        return best;
    }

    // attempt to match an incoming order against the resting opposite side.
    // fills as many shares as possible at eligible price levels (price-priority, FIFO within level).
    // any unfilled remainder is added to the book.
    FillResult matchAndFill(Order o) {
        uint32_t remaining = o.quantity;
        double   total_cost = 0.0;
        uint32_t total_filled = 0;

        if (o.side == Side::buy) {
            // sort asks ascending so we hit the cheapest first
            std::sort(sell_orders.begin(), sell_orders.end(), [](const PriceLevel &a, const PriceLevel &b)
                      { return a.getPrice() < b.getPrice(); });

            for (auto it = sell_orders.begin(); it != sell_orders.end() && remaining > 0; ) {
                if (it->getPrice() > o.price) break; // no more eligible asks
                uint32_t take = std::min(remaining, it->getTotalQuantity());
                uint32_t consumed = it->consume(take);
                total_cost  += consumed * it->getPrice();
                total_filled += consumed;
                remaining   -= consumed;
                if (it->isEmpty()) it = sell_orders.erase(it);
                else ++it;
            }
        } else {
            // sort bids descending so we hit the highest first
            std::sort(buy_orders.begin(), buy_orders.end(), [](const PriceLevel &a, const PriceLevel &b)
                      { return a.getPrice() > b.getPrice(); });

            for (auto it = buy_orders.begin(); it != buy_orders.end() && remaining > 0; ) {
                if (it->getPrice() < o.price) break; // no more eligible bids
                uint32_t take = std::min(remaining, it->getTotalQuantity());
                uint32_t consumed = it->consume(take);
                total_cost  += consumed * it->getPrice();
                total_filled += consumed;
                remaining   -= consumed;
                if (it->isEmpty()) it = buy_orders.erase(it);
                else ++it;
            }
        }

        // add any unfilled remainder to the resting book
        if (remaining > 0) {
            o.quantity = remaining;
            addOrder(o);
        }

        double avg = (total_filled > 0) ? total_cost / total_filled : 0.0;
        return { total_filled, avg, total_filled > 0 };
    }

    // return sorted snapshots of both sides for API / display use
    std::vector<BookLevel> getAsks() const {
        std::vector<BookLevel> levels;
        for (const PriceLevel &p : sell_orders)
            levels.push_back({ p.getPrice(), p.getTotalQuantity() });
        std::sort(levels.begin(), levels.end(), [](const BookLevel &a, const BookLevel &b)
                  { return a.price < b.price; });
        return levels;
    }

    std::vector<BookLevel> getBids() const {
        std::vector<BookLevel> levels;
        for (const PriceLevel &p : buy_orders)
            levels.push_back({ p.getPrice(), p.getTotalQuantity() });
        std::sort(levels.begin(), levels.end(), [](const BookLevel &a, const BookLevel &b)
                  { return a.price > b.price; });
        return levels;
    }

    // seed the book with realistic-looking market-maker resting orders around a mid price
    void seedMarketMakerOrders(double mid, int levels = 6, uint32_t base_qty = 150) {
        double tick = 0.25;
        for (int i = 1; i <= levels; i++) {
            double ask_price = mid + i * tick;
            double bid_price = mid - i * tick;
            uint32_t qty = base_qty + (i % 3) * 50;

            Order ask { next_id++, Side::sell, OrderType::limit, ask_price, qty };
            Order bid { next_id++, Side::buy,  OrderType::limit, bid_price, qty };
            addOrder(ask);
            addOrder(bid);
        }
    }
};
