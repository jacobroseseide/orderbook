#pragma once
#include <vector>
#include <iostream>
#include "order.hpp"
#include <algorithm>
#include "priceLevels.hpp"

// Private vector for buy orders, vector for sell orders

class OrderBook
{
private:
    using PriceLevelVec = std::vector<PriceLevel>;


    PriceLevelVec buy_orders;
    PriceLevelVec sell_orders;

public:


// void addOrder(Order o):
//     1. Pick the right vector (buy_orders or sell_orders based on o.side)
//     2. Loop through the price levels
//     3. If you find a level where getPrice() == o.price:
//          - Add the order to that level
//          - Return
//     4. If no matching level found:
//          - Create a new PriceLevel with that price
//          - Add the order to the new level
//          - Push the new level to the vector




    void addOrder(Order o) {

        // for adding to buy order side
        if (o.side == Side::buy) {
            for (PriceLevel &p : buy_orders) {
                if (p.getPrice() == o.price) {
                    p.addOrder(o);
                    return;
                }
            }
        // If we get here, no matching price level was found
        PriceLevel newLevel(o.price); // create new price level
        newLevel.addOrder(o); // add the order to that specific price level
        buy_orders.push_back(newLevel); // add new price level to buy_orders pricelevel vector
        
        } else { // must be sell order

            // TO DO: Make this more consice, create addOrderList method, then use ? for addOrder method
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

    void printBook() {

        // sort buy_orders vector from greatest to least (descending)
        std::sort(buy_orders.begin(), buy_orders.end(), [](const Order &a, const Order &b)
                  { return a.price > b.price; });

        std::cout << "BUY ORDER BOOK:\n";

        // then print out the sorted buy orders
        for (const Order &o : buy_orders) {
            std::cout << "BUY " << o.quantity << " at " << o.price << std::endl;
        }

        std::cout << "--------------------------\n";

        // sort sell_orders vector from least to greatest (ascending)
        std::sort(sell_orders.begin(), sell_orders.end(), [](const Order &a, const Order &b) { 
            return a.price < b.price; 
        });

        std::cout << "SELL ORDER BOOK:\n";

        // then print out the sorted sell orders
        for (const Order &o : sell_orders) {
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
        for (size_t i = 0; i < buy_orders.size(); i++) {
            if (buy_orders[i].id == id) { // found the id
                buy_orders.erase(buy_orders.begin() + i);
                return true;
            }
        }
        // if not in buy orders, check sell orders
        for (size_t i = 0; i < sell_orders.size(); i++) {
            if (sell_orders[i].id == id) { // found the id
                sell_orders.erase(sell_orders.begin() + i);
                return true;
            }
        }
        return false; // return false if given id is not in either buy or sell order vectors
    }


    bool modifyOrderList(uint64_t id, uint32_t new_quantity, PriceLevelVec &v) {
         // edge case if new quantity user wants is 0 (same as deleting order)
        if (new_quantity == 0){
            deleteOrder(id);
            std::cout << "successfully deleted, user requested new quantity be 0 shares\n";
            return true;
        }


        // check buy order book for specific id
        for (size_t i = 0; i < v.size(); i++) {

            if (v[i].id == id) { // first: find the right order

                if (new_quantity > v[i].quantity) { // then: validate
                    return false; // can't add shares
                }

                if (new_quantity == v[i].quantity) {
                    std::cout << "user requested to modify shares to amount they previously had, nothing changed\n";
                    return true;
                }

                v[i].quantity = new_quantity; // finally: modify
                return true;
            }
        }
        return false;

    }


    // function to change number of shares in specific order
    // ONLY ALLOWED TO REQUEST LESS SHARES THAN IN INITIAL ORDER, CANNOT ADD SHARES
    bool modifyOrder(uint64_t id, uint32_t new_quantity, bool isBuy){
        return isBuy ? modifyOrderList(id, new_quantity, buy_orders) : modifyOrderList(id, new_quantity, sell_orders);
    }

};

