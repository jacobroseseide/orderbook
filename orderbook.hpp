#pragma once
#include <vector>
#include <iostream>
#include "order.hpp"
#include <algorithm>


// Private vector for buy orders, vector for sell orders

class OrderBook {
private: 
    std::vector<Order> buy_orders;
    std::vector<Order> sell_orders;

public:
    void addOrder(Order o) {
        if (o.side == Side::buy) {
            buy_orders.push_back(o);
        } else {
            sell_orders.push_back(o);
        }
    }

    void printBook() {

        // sort buy_orders vector from greatest to least (descending)
        std::sort(buy_orders.begin(), buy_orders.end(), [](const Order& a, const Order& b) {
            return a.price > b.price;
        });


        std::cout << "BUY ORDER BOOK:\n";

        // then print out the sorted buy orders
        for (const Order& o: buy_orders) {
            std::cout << "BUY " << o.quantity << " at " << o.price << std::endl;
        }

        std::cout << "--------------------------\n";

        // sort sell_orders vector from least to greatest (ascending)
        std::sort(sell_orders.begin(), sell_orders.end(), [](const Order& a, const Order& b) {
            return a.price < b.price;
        });

        std::cout << "SELL ORDER BOOK:\n";
        
        // then print out the sorted sell orders
        for (const Order& o : sell_orders) {
            std::cout << "SELL " << o.quantity << " at " << o.price << std::endl;
        }

    }



    // given an order ID, we need to:
    // 1. find the order with the unique id
        // 1a. start with buy order book, if found, remove it
        // 1b. if not in buy, go to sell order book, if found, remove it
    // 2. if no matching ID, return false

    // for now return a bool (true if found and removed, false if not found)
    // can make it so it returns an order in V2
    bool deleteOrder(uint64_t id) {

        // check buy orders for id
        for(size_t i=0; i<buy_orders.size(); i++) {
            if (buy_orders[i].id == id) { // found the id
                buy_orders.erase(buy_orders.begin() + i);
                return true;
            }
        }

        // if not in buy orders, check sell orders
        for(size_t i=0; i<sell_orders.size(); i++) {
            if (sell_orders[i].id == id) { // found the id
                sell_orders.erase(sell_orders.begin() + i);
                return true;
            }
        }

        return false; // return false if given id is not in either buy or sell order vectors

    }

};


// BUY: 100 shares @ 50.25
// BUY: 50 shares @ 50.00
// SELL: 75 shares @ 51.00
