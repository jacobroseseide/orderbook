#include <vector>
#include <iostream>
#include "order.hpp"


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

};


// BUY: 100 shares @ 50.25
// BUY: 50 shares @ 50.00
// SELL: 75 shares @ 51.00
